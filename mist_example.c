/**
 * Basic example for setting up a half-simulated lighting controller.
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
#define __MODUUL__ "mistex"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_example & BASE_LOG_LEVEL)
#include "log.h"

static mist_module_t m_lighting_module;
static mist_module_t m_movement_module;

static int32_t m_light_output = -1; // Totally off
static int32_t m_movement_count = 0; // No movement yet

static mist_error_t light_control(
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype) // No input - return current state
	{
		debug1("read light_control");
		*otype = MIST_ITEM_INT32;
		*((int32_t*)output) = m_light_output;
		*output_length = sizeof(int32_t);
		return MIST_SUCCESS;
	}
	else if (MIST_ITEM_INT32 == itype) // Simple control input, return set value
	{
		info1("CONTROL %"PRIi32, *((int32_t*)input));

		m_light_output = *((int32_t*)input);
		if(m_light_output > 100)
		{
			warn1("limit 100");
			m_light_output = 100;
		}
		else if(m_light_output < -1) // -1 is AC cut and 0 is soft-off, where applicable
		{
			warn1("limit -1");
			m_light_output = -1;
		}

		if(m_light_output > 0)
		{
			PLATFORM_LedsSet(PLATFORM_LedsGet() | (1 << 2)); // LED2 on
		}
		else
		{
			PLATFORM_LedsSet(PLATFORM_LedsGet() & ~(1 << 2)); // LED2 off
		}

		*otype = MIST_ITEM_INT32;
		*((int32_t*)output) = m_light_output;
		*output_length = sizeof(int32_t);
		return MIST_SUCCESS;
	}
	else // Some type of complex control, not implemented
	{
		warn1("light_control itype %d", (int)itype);
	}

	return MIST_FAIL;
}

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

void mist_example_init()
{
	mist_error_t result;

	// Register lighting control
	m_lighting_module.data_type = dt_light_control;
	m_lighting_module.function = light_control;
	memset(m_lighting_module.uuid, 0, UUID_LENGTH);

	result = mist_register_handler(&m_lighting_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
	}

	// Register movement sensor
	m_movement_module.data_type = dt_movement_count;
	m_movement_module.function = movement_count;
	memcpy(m_movement_module.uuid, "\xd1\x4a\x6a\x68\x97\x2c\x49\xae\x92\x69\x14\x9a\x27\x5c\x50\xe3", UUID_LENGTH);

	result = mist_register_handler(&m_movement_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
	}

	// Create a thread to poll fake movement detector
	const osThreadAttr_t thread_attr = { .name = "pir" };
	osThreadNew(movement_detector_simulation_thread, NULL, &thread_attr);
}
