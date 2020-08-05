/**
 * Setup for a minimal Mist application.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include "platform.h"

#include "retargetserial.h"
#include "retargetspi.h"

#include "SignatureArea.h"
#include "DeviceSignature.h"

#include "loggers_ext.h"
#include "logger_ldma.h"

#include "cmsis_os2_ext.h"

#include "DeviceSignature.h"

#include "radio.h"
#ifdef INCLUDE_BEATSTACK
#include "beatstack.h"
#endif

// emdrv components
#include "sleep.h"
#include "dmadrv.h"

#include "fs.h"
#include "spi_flash.h"

#include "announcement_app.h"

#include "mist_comm_am_addrdisco.h"

#include "mist_middleware.h"
#include "mist_example.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"
#include "sys_panic.h"

#define FS_SPIFFS_DATA_PARTITION 2

#define DEVICE_ANNOUNCEMENT_PERIOD_S 300

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

static fs_driver_t m_spi_fs_driver;

ieee_eui64_t g_eui; // Global node EUI

static comms_layer_t * m_radio_comm = NULL;

#ifdef INCLUDE_BEATSTACK
static comms_layer_t * m_beat_comm = NULL;
#endif

static void radio_start_done (comms_layer_t * comms, comms_status_t status, void * user)
{
    debug("started %d", status);
}

// Perform basic radio setup
static comms_layer_t * radio_setup (am_addr_t node_addr, uint8_t eui[IEEE_EUI64_LENGTH])
{
    comms_layer_t * radio = NULL;

    m_radio_comm = radio_init(DEFAULT_RADIO_CHANNEL, DEFAULT_PAN_ID, node_addr);
    if (NULL == m_radio_comm)
    {
        return NULL;
    }

    eui64_set(&(m_radio_comm->eui), eui); // TODO this should have an API

#ifdef INCLUDE_BEATSTACK
    info1("Starting multi-hop");
    m_beat_comm = beatstack_create(node_addr, m_radio_comm);
    if (NULL == m_beat_comm)
    {
        err1("bs start"); // TODO remove once sys_panic learns to log
        osDelay(10000);
        sys_panic("bs start");
    }
    // Also set Global address of the device for the MH layer
    eui64_set(&(m_beat_comm->eui), eui); // TODO this should have an API

    radio = m_beat_comm;
#else
    info1("Starting single-hop");
    radio = m_radio_comm;
#endif

    static am_addrdisco_t disco;
    static comms_addr_cache_t cache;
    // Set up Global address resolution and caching
    comms_am_addrdisco_init(radio, &disco, &cache);

    if (COMMS_SUCCESS != comms_start(radio, radio_start_done, NULL))
    {
        return NULL;
    }

    // Wait for radio to start, could use osTreadFlagWait and set from callback
    while (COMMS_STARTED != comms_status(radio))
    {
        osDelay(1);
    }

    debug1("radio rdy");
    return radio;
}

static void filesystem_setup ()
{
    // SPI for dataflash
    RETARGET_SpiInit();

    // Get dataflash chip ID
    uint8_t jedec[4] = {0};
    RETARGET_SpiTransferHalf(0, "\x9F", 1, jedec, 4);
    info1("jedec %02x%02x%02x%02x", jedec[0], jedec[1], jedec[2], jedec[3]);

    spi_flash_init();
    // spi_flash_mass_erase();

    m_spi_fs_driver.read = spi_flash_read;
    m_spi_fs_driver.write = spi_flash_write;
    m_spi_fs_driver.erase = spi_flash_erase;
    m_spi_fs_driver.size = spi_flash_size;
    m_spi_fs_driver.erase_size = spi_flash_erase_size;
    m_spi_fs_driver.lock = spi_flash_lock;
    m_spi_fs_driver.unlock = spi_flash_unlock;

    fs_init(0, FS_SPIFFS_DATA_PARTITION, &m_spi_fs_driver);

    fs_start();
}


static void main_loop ()
{
    // Switch to a thread-safe logger
    logger_ldma_init();
    log_init(BASE_LOG_LEVEL, &logger_ldma, NULL);

    am_addr_t node_addr = DEFAULT_AM_ADDR;
    // Initialize node signature - get address and EUI64
    if (SIG_GOOD == sigInit())
    {
        node_addr = sigGetNodeId();
        sigGetEui64(g_eui.data);
    }
    else
    {
        uint8_t eui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, node_addr >> 8, node_addr};
        eui64_set(&g_eui, eui);
    }
    infob1("ADDR:%" PRIX16 " EUI64:", g_eui.data, sizeof(g_eui.data), node_addr);

    // Initialize SPI flash filesystem
    filesystem_setup();
    debug1("fs rdy");

    // initialize radio for application use
    comms_layer_t *radio = radio_setup(node_addr, g_eui.data);
    if (NULL == radio)
    {
        err1("radio"); // TODO remove once sys_panic learns to log
        osDelay(10000);
        sys_panic("radio");
    }

    // Start deviceannouncement application ------------------------------------
    if (0 == announcement_app_init(radio, DEVICE_ANNOUNCEMENT_PERIOD_S))
    {
        debug1("annc started");
    }
    else
    {
        err1("annc");
    }

    // Setup mist middleware
    info1("mist middleware %s", mist_middleware_version(NULL, NULL, NULL));
    mist_middleware_init(radio);

    // Initialize the mist-example application, register handlers
    mist_example_init();

    // All registrations should be done now, start middleware
    mist_error_t merr = mist_middleware_start();
    if(MIST_SUCCESS != merr)
    {
        err1("merr %d", merr);
    }
    debug1("mist rdy");

    // Loop forever, printing uptime
    for (;;)
    {
        info1("uptime: %u", (unsigned int)osCounterGetSecond());
        osDelay(60000);
    }
}

uint32_t ident_timestamp () // TODO this should be moved elsewhere
{
    return IDENT_TIMESTAMP;
}

int logger_fwrite_boot (const char *ptr, int len)
{
    fwrite(ptr, len, 1, stdout);
    fflush(stdout);
    return len;
}

int main ()
{
    PLATFORM_Init();

    // LEDs
    PLATFORM_LedsInit();

    // Button
    PLATFORM_ButtonPinInit();

    // Configure debug output
    RETARGET_SerialInit();
    log_init(BASE_LOG_LEVEL, &logger_fwrite_boot, NULL);

    info1("TestMist " VERSION_STR " (%d.%d.%d)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    PLATFORM_RadioInit(); // Radio GPIO/PRS - LNA on some MGM12P

    // Initialize OS kernel
    osKernelInitialize();

    // Initialize sleep management
    SLEEP_Init(NULL, NULL);
    //SLEEP_SleepBlockBegin(sleepEM2);

    // Initialize DMADRV
    DMADRV_Init();

    // Create a thread
    const osThreadAttr_t thread_attr = {.name = "main"};
    osThreadNew(main_loop, NULL, &thread_attr);

    if (osKernelReady == osKernelGetState())
    {
        osKernelStart();
    }
    else
    {
        err1("!osKernelReady");
    }

    for (;;)
        ;
}
