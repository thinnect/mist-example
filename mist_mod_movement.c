/**
 * Basic example for setting up a half-simulated movement sensor.
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

#include "loglevels.h"
#define __MODUUL__ "m_m_m"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_movement & BASE_LOG_LEVEL)
#include "log.h"

// d14a6a68-972c-49ae-9269-149a275c50e3
#define UUID_MOVEMENT_RESOURCE ((uint8_t*)"\xd1\x4a\x6a\x68\x97\x2c\x49\xae\x92\x69\x14\x9a\x27\x5c\x50\xe3")

static mist_module_t m_movement_module;

static int32_t m_movement_count = 0; // No movement yet

static mist_error_t movement_count(
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype) // No input - return current state
	{
		debug1("read");
		*otype = MIST_ITEM_INT32;
		*((int32_t*)output) = m_movement_count;
		*output_length = sizeof(int32_t);
		return MIST_SUCCESS;
	}

	return MIST_FAIL; // Control actions not supported by movement sensor
}

// Fake a motion detector by polling the platform button (assuming there is something behind it)
static void movement_detector_simulation_thread(void * arg)
{
	for (;;)
	{
		osDelay(1000);
		if (PLATFORM_ButtonGet())
		{
			mist_error_t r = MIST_FAIL;

			info1("movement");
			m_movement_count++;

#ifdef LONG_EXAMPLE_FOR_EVENT_MESSAGE
			uint8_t buffer[80];
			uint16_t length = 0;
			ml_encoder_t enc;
			if(ML_SUCCESS == MLE_initialize(&enc, buffer, sizeof(buffer)))
			{
				uint8_t index1 = MLE_appendO(&enc, dt_data);
				uint8_t index2 = MLE_appendOSV(&enc, dt_data, index1, dt_movement_count);
				MLE_appendOSV(&enc, dt_value, index2, m_movement_count);
				// MLE_appendOSV(&enc, dt_exp, index3, -1);

				// Add an UTC timestamp with the epoch at 2000-01-01
				// MLE_appendOSV(&enc, dt_timestamp_utc, index1, 604401880);
				length = MLE_finalize(&enc);
			}

			if (length > 0)
			{
				r = mist_spontaneous_event(&m_movement_module, MIST_ITEM_MOTEXML, buffer, length);
			}
#else // Use short example of encoding data
			r = mist_spontaneous_event(&m_movement_module, MIST_ITEM_INT32, &m_movement_count, sizeof(int32_t));
#endif//LONG_EXAMPLE_FOR_EVENT_MESSAGE

			if (MIST_SUCCESS != r)
			{
				warn1("mov spnt evt %d", (int)r); // Something went wrong
			}
			osDelay(5000); // Limit event rate
		}
	}
}

bool mist_mod_movement_init()
{
	// Register movement sensor
	m_movement_module.data_type = dt_movement_count;
	m_movement_module.function = movement_count;
	memcpy(m_movement_module.uuid, UUID_MOVEMENT_RESOURCE, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_movement_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
		return false;
	}

	// Create a thread to poll fake movement detector
	const osThreadAttr_t thread_attr = { .name = "pir", .stack_size = 1536 };
	if (NULL == osThreadNew(movement_detector_simulation_thread, NULL, &thread_attr))
	{
		err1("thrd mov");
		return false;
	}
	return true;
}
