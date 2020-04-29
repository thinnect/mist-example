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

#include "retargetserial.h"

#include "cmsis_os2_ext.h"

#include "platform.h"

#include "SignatureArea.h"
#include "DeviceSignature.h"

#include "loggers_ext.h"
#include "logger_fwrite.h"

#include "DeviceSignature.h"

#include "radio.h"

#include "announcement_app.h"

#include "mist_comm_am_addrdisco.h"

#include "mist_middleware.h"

#include "loglevels.h"
#define __MODUUL__ "main"
#define __LOG_LEVEL__ (LOG_LEVEL_main & BASE_LOG_LEVEL)
#include "log.h"

#define DEVICE_ANNOUNCEMENT_PERIOD_S 300

// Include the information header binary
#include "incbin.h"
INCBIN(Header, "header.bin");

ieee_eui64_t g_eui; // Global node EUI

static void radio_start_done (comms_layer_t * comms, comms_status_t status, void * user)
{
    debug("started %d", status);
}

// Perform basic radio setup, register to receive RadioCountToLeds packets
static comms_layer_t* radio_setup (am_addr_t node_addr, uint8_t eui[IEEE_EUI64_LENGTH])
{
    static am_addrdisco_t disco;
    static comms_addr_cache_t cache;

    comms_layer_t * radio = radio_init(DEFAULT_RADIO_CHANNEL, 0x22, node_addr);
    if (NULL == radio)
    {
        return NULL;
    }

    eui64_set(&(radio->eui), eui); // TODO this should have an API

    // Set up Global address resolution and caching
    comms_am_addrdisco_init(radio, &disco, &cache);

    if (COMMS_SUCCESS != comms_start(radio, radio_start_done, NULL))
    {
        return NULL;
    }

    // Wait for radio to start, could use osTreadFlagWait and set from callback
    while(COMMS_STARTED != comms_status(radio))
    {
        osDelay(1);
    }

    debug1("radio rdy");
    return radio;
}

static void main_loop ()
{
    am_addr_t node_addr = DEFAULT_AM_ADDR;

    // Initialize node signature - get address and EUI64
    if (SIG_GOOD == sigInit())
    {
        node_addr = sigGetNodeId();
        sigGetEui64(g_eui.data);
    }
    else
    {
        uint8_t eui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, node_addr>>8, node_addr};
        eui64_set(&g_eui, eui);
    }
    infob1("ADDR:%"PRIX16" EUI64:", g_eui.data, sizeof(g_eui.data), node_addr);

    // initialize radio
    comms_layer_t* radio = radio_setup(node_addr, g_eui.data);
    if (NULL == radio)
    {
        err1("radio");
        for (;;); // panic
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
    mist_middleware_init(radio);

    // TODO registration of action modules

    // Lopp forever, printing uptime
    for (;;)
    {
        info1("uptime: %u", (unsigned int)osCounterGetSecond());
        osDelay(60000);
    }
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

    // Configure debug output
    RETARGET_SerialInit();
    log_init(BASE_LOG_LEVEL, &logger_fwrite_boot, NULL);

    info1("TestMist "VERSION_STR" (%d.%d.%d)", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    PLATFORM_RadioInit(); // Radio GPIO/PRS - LNA on some MGM12P

    // Initialize OS kernel
    osKernelInitialize();

    // Create a thread
    const osThreadAttr_t thread_attr = { .name = "main" };
    osThreadNew(main_loop, NULL, &thread_attr);

    if (osKernelReady == osKernelGetState())
    {
        // Switch to a thread-safe logger
        logger_fwrite_init();
        log_init(BASE_LOG_LEVEL, &logger_fwrite, NULL);

        // Start the kernel
        osKernelStart();
    }
    else
    {
        err1("!osKernelReady");
    }

    for(;;);
}
