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
#include "platform_eui.h"

#include "retargetserial.h"
#include "retargetspi.h"

#include "SignatureArea.h"
#include "DeviceSignature.h"

#include "basic_rtos_logger_setup.h"
#include "basic_rtos_filesystem_setup.h"

#include "cmsis_os2_ext.h"

#include "DeviceSignature.h"

#include "time_rtc.h"
#include "watchdog.h"

#include "radio.h"
#ifdef INCLUDE_BEATSTACK
#include "license_rcvr.h"
#include "beatstack.h"
#include "basic_rtos_beatstack_timesync.h"
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

#include "basic_rtos_threads_stats.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"
#include "sys_panic.h"


#ifdef INCLUDE_OTA
#include "basic_rtos_ota_setup.h"
#endif

#define USER_FILE_SYS_NR 0

#define DEVICE_ANNOUNCEMENT_PERIOD_S 300

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

ieee_eui64_t g_eui; // Global node EUI

static comms_layer_t * m_radio_comm = NULL;

#ifdef INCLUDE_BEATSTACK
static comms_layer_t * m_beat_comm = NULL;
#endif

static comms_receiver_t m_receiver_bc_data;

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
    m_beat_comm = beatstack_create(node_addr,               \
                                   m_radio_comm,            \
                                   basic_nw_time_changed,   \
                                   USER_FILE_SYS_NR);
    if (NULL == m_beat_comm)
    {
        err1("bs start"); // TODO remove once sys_panic learns to log
        osDelay(10000);
        sys_panic("bs start");
    }
    // Also set Global address of the device for the MH layer
    eui64_set(&(m_beat_comm->eui), eui); // TODO this should have an API

    radio = m_beat_comm;

    static comms_sleep_controller_t rctrl;
    static lic_rcvr_t lic;
    if (false == license_rcvr_init(m_radio_comm, &lic, &rctrl))
    {
        err1("!license_rcvr");
    }

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

static void main_loop ()
{
    // Switch to a thread-safe logger
    basic_rtos_logger_setup();

    am_addr_t node_addr = NODE_AM_ADDR;
    // Initialize node signature - get address and EUI64

    if (SIG_GOOD != sigInit())
    {
        warn1("sig");
    }

    if ((0 != node_addr)&&(0xFFFF != node_addr)) // Override sig if NODE_AM_ADDR set
    {
        uint8_t eui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, node_addr >> 8, node_addr};
        eui64_set(&g_eui, eui);
    }
    else
    {
        node_addr = sigGetNodeId();
        sigGetEui64(g_eui.data);
    }
    infob1("ADDR:%" PRIX16 " EUI64:", g_eui.data, sizeof(g_eui.data), node_addr);

    uint8_t mcu_eui[IEEE_EUI64_LENGTH];
    platform_eui(mcu_eui);
    infob1("MCU EUI64:", mcu_eui, IEEE_EUI64_LENGTH);

    time_rtc_init();

    // Initialize SPI flash filesystem
    basic_rtos_filesystem_setup();
    debug1("fs rdy");

    // initialize radio for application use
    comms_layer_t *radio = radio_setup(node_addr, g_eui.data);
    if (NULL == radio)
    {
        err1("radio"); // TODO remove once sys_panic learns to log
        osDelay(10000);
        sys_panic("radio");
    }

    #ifdef INCLUDE_OTA
    bool feed_watchdog = false;
    #ifdef INCLUDE_BEATSTACK
        basic_rtos_ota_setup(m_beat_comm, m_radio_comm, true, &feed_watchdog);
    #else
        basic_rtos_ota_setup(NULL, m_radio_comm, true, &feed_watchdog);
    #endif
    #endif
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

    // Initialize the mist-example applications that register handlers
    #ifdef EXAMPLE_MIST_LIGHT_CONTROL
        #pragma message "EXAMPLE_MIST_LIGHT_CONTROL"
        if ( ! mist_mod_lighting_init())
        {
            err1("light init");
        }
    #endif// EXAMPLE_MIST_LIGHT_CONTROL

    #ifdef EXAMPLE_MIST_MOVEMENT
        #pragma message "EXAMPLE_MIST_MOVEMENT"
        if ( ! mist_mod_movement_init())
        {
            err1("mov init");
        }
    #endif//EXAMPLE_MIST_MOVEMENT

    #ifdef EXAMPLE_MIST_BUTTON
        #pragma message "EXAMPLE_MIST_BUTTON"
        if ( ! mist_mod_button_init())
        {
            err1("button init");
        }
    #endif//EXAMPLE_MIST_BUTTON

    #ifdef EXAMPLE_MIST_LUX
        #pragma message "EXAMPLE_MIST_LUX"
        if ( ! mist_mod_lux_init())
        {
            err1("lux init");
        }
    #endif//EXAMPLE_MIST_LUX

    #ifdef EXAMPLE_MIST_ARRAY_PAR
        #pragma message "EXAMPLE_MIST_ARRAY_PAR"
        if ( ! mist_mod_array_init(true))
        {
            err1("arr p init");
        }
    #endif//EXAMPLE_MIST_ARRAY_PAR

    #ifdef EXAMPLE_MIST_ARRAY_SEQ
        #pragma message "EXAMPLE_MIST_ARRAY_SEQ"
        if ( ! mist_mod_array_init(false))
        {
            err1("arr s init");
        }
    #endif//EXAMPLE_MIST_ARRAY_SEQ

    // All registrations should be done now, start middleware
    mist_error_t merr = mist_middleware_start();
    if (MIST_SUCCESS == merr)
    {
        debug1("mist rdy");
    }
    else
    {
        err1("merr %d", merr);
    }

    // Loop forever, printing uptime
    for (;;)
    {
        basic_rtos_threads_stats();
        info1("uptime: %u unix_time: %u", (unsigned int)osCounterGetSecond(), (unsigned int)time(NULL));
        osDelay(60000);
    }
}

int main ()
{
    PLATFORM_Init();

    // Disable watchdog in the example application.
    // Enable and feed watchdog in release applications.
    watchdog_disable();

    // LEDs
    PLATFORM_LedsInit();

    // Button
    PLATFORM_ButtonPinInit();

    // Configure debug output
    basic_noos_logger_setup();

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
    const osThreadAttr_t thread_attr = {.name = "main", .stack_size = 1536};
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
