/**
 * Basic example for broadcasting button resource messages.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */

#include "mist_middleware.h"

#include "dt_types.h"
#include "MLE.h"

#include "platform.h"

#include <inttypes.h>
#include <string.h>

#include "cmsis_os2_ext.h"

#include "loglevels.h"
#define __MODUUL__ "m_m_b"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_button & BASE_LOG_LEVEL)
#include "log.h"

// 3e6ea866-937e-46b0-a6ae-70c96d91733a
#define UUID_BUTTON_RESOURCE ((uint8_t*)"\x3e\x6e\xa8\x66\x93\x7e\x46\xb0\xa6\xae\x70\xc9\x6d\x91\x73\x3a")

static mist_module_t m_button_module;

static int32_t m_button_state = 0;

static mist_error_t button_state(
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype) // No input - return current state
	{
		debug1("read");
		*otype = MIST_ITEM_INT32;
		*((int32_t*)output) = m_button_state;
		*output_length = sizeof(int32_t);
		return MIST_SUCCESS;
	}

	return MIST_FAIL; // Control actions not supported by button
}

static void send_button_state(int32_t state, uint32_t timestamp)
{
	uint8_t buffer[80];
	uint16_t length = 0;
	ml_encoder_t enc;

	if(ML_SUCCESS == MLE_initialize(&enc, buffer, sizeof(buffer)))
	{
		uint8_t index1 = MLE_appendO(&enc, dt_data);
		uint8_t index2 = MLE_appendOSV(&enc, dt_data, index1, dt_button);
		MLE_appendOSV(&enc, dt_value, index2, m_button_state);

		// Add event timestamp, when rebroadcating, this should remain the same
		MLE_appendOSV(&enc, dt_timestamp_ms, index2, timestamp);

		// Add an UTC timestamp with the epoch at 2000-01-01
		// MLE_appendOSV(&enc, dt_timestamp_utc, index1, 604401880);
		length = MLE_finalize(&enc);
	}

	if (length > 0)
	{
		mist_error_t r = mist_spontaneous_event(&m_button_module, MIST_ITEM_MOTEXML, buffer, length);
		if (MIST_SUCCESS != r)
		{
			warn1("but evt %d", (int)r); // Something went wrong
		}
	}

}

// Periodically send different button states
// This explicitly implements retransmissions, in the future the retransmission
// functionality will be handled by the mist layer again.
// TODO remove retransmit and configure the module with appropriate parameters
static void button_simulation_thread(void * arg)
{
	osDelay(60*1000);
	for (;;)
	{
		m_button_state = (m_button_state + 1) % 4;
		info1("button %"PRIi32, m_button_state);

		uint32_t timestamp = osCounterGetMilli();

		// Send new button state for the first time
		send_button_state(m_button_state, timestamp);

		osDelay(20*1000);

		// Send the same state with the same timestamp again
		send_button_state(m_button_state, timestamp);

		osDelay(40*1000);
	}
}

bool mist_mod_button_init()
{
	// Register button
	m_button_module.data_type = dt_button;
	m_button_module.function = button_state;
	memcpy(m_button_module.uuid, UUID_BUTTON_RESOURCE, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_button_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
		return false;
	}

	// Configure button events to have very little backoff: 0, 1, 3, 7, 10
	mist_configure_spontaneous_event_backoff(&m_button_module, 1, 10, 1);

	// Create a thread to simulate button presses
	const osThreadAttr_t thread_attr = { .name = "but", .stack_size = 1536 };
	if (NULL == osThreadNew(button_simulation_thread, NULL, &thread_attr))
	{
		err1("btn thrd");
		return false;
	}
	return true;
}
