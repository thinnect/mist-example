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
#define __MODUUL__ "m_m_l"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_lighting & BASE_LOG_LEVEL)
#include "log.h"

static mist_module_t m_lighting_module;

static int32_t m_light_output = -1; // Totally off

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

void mist_mod_lighting_init()
{
	// Register lighting control
	m_lighting_module.data_type = dt_light_control;
	m_lighting_module.function = light_control;
	memset(m_lighting_module.uuid, 0, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_lighting_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
	}
}
