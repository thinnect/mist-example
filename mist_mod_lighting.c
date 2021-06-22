/**
 * Basic example for setting up a half-simulated lighting controller.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */

#include "mist_middleware.h"

#include "dt_types.h"
#include "MLE.h"
#include "MLD.h"
#include "MLI.h"

#include "platform.h"

#include "cmsis_os2_ext.h"

#include <inttypes.h>
#include <string.h>

#include "loglevels.h"
#define __MODUUL__ "m_m_lc"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_lighting & BASE_LOG_LEVEL)
#include "log.h"

#define LUX_LC_PID_KP 0.1
#define LUX_LC_PID_KI 0.01
#define LUX_LC_PID_KD 0.01

static mist_module_t m_lighting_module;

static int32_t m_light_output = -1; // Totally off

// Simple control
static bool m_pid_initialized = false;
static int32_t m_pid_integral;
static int32_t m_pid_last_error;
static uint32_t m_last_control_s;


/**
 * Timestamps have acquired some quirks over time, so this function looks at
 * several options and tries to return a local second timestamp that should
 * reference osCounterGetSecond().
 *
 * @param idx - index where to look for timestamp.
 * @param data - data message to look in.
 * @param length - length of the data message.
 * @param ts - location to store the extracted timestamp.
 * @return true if a timestamp was found.
 */
static bool get_timestamp_seconds (uint8_t idx, const uint8_t data[], uint16_t length, uint32_t * p_ts)
{
	ml_object_t object;

	// Local timestamp in seconds
	if ((MLD_findOS(dt_timestamp_local_s, idx, data, length, &object))
	  &&(object.valueIsPresent))
	{
		*p_ts = object.value;
		return true;
	}

	// Local timestamp in milliseconds
	if ((MLD_findOS(dt_timestamp_local_ms, idx, data, length, &object))
	  &&(object.valueIsPresent))
	{
		*p_ts = object.value / 1000;
		return true;
	}

	// "Legacy" timestamp in milliseconds
	if ((MLD_findOS(dt_timestamp_ms, idx, data, length, &object))
	  &&(object.valueIsPresent))
	{
		*p_ts = object.value / 1000;
		return true;
	}

	return false;
}


/**
 * Look for light-sensor target value under the input object.
 * @param p_value - memory for storing the light-sensor value.
 * @param data - data buffer.
 * @param len_data - length of the data in the buffer.
 * @return true when found and value stored in p_value.
 */
static bool get_target_lightsensor_value (
    int32_t * p_value,
    const uint8_t data[], uint16_t len_data)
{
	ml_object_t object; // Generic helper object

	if (0 == MLD_findOS(dt_input, 0, data, len_data, &object))
	{
		return false;
	}
	if (0 == MLD_findOS(dt_value, object.index, data, len_data, &object))
	{
		return false;
	}
	if (object.valueIsPresent) // If it has a value, it is something else
	{
		return false;
	}
	if (0 == MLD_findOS(dt_illuminance, object.index, data, len_data, &object))
	{
		return false;
	}
	if (0 == MLD_findOS(dt_light_lx, object.index, data, len_data, &object)
	 ||( ! object.valueIsPresent))
	{
		return false;
	}
	*p_value = object.value;
	return true;
}


/**
 * Look for light-sensor value in the included data.
 * @param p_value - memory for storing the light-sensor value.
 * @param p_timestamp - memory for storing the timestamp of the light-sensor value.
 * @param data - data buffer.
 * @param len_data - length of the data in the buffer.
 * @return true when found and value stored in p_value.
 */
static bool get_current_lightsensor_value (
    int32_t * p_value, uint32_t * p_timestamp,
    const uint8_t state_data[], uint16_t len_state_data)
{
	ml_object_t object; // Generic helper object

	// Figure out if we have current lux data and how old it is
	// (at least initially provides only a single piece of data over all sensors - the newest)
	uint8_t idx_data = MLD_findOS(dt_data, 0, state_data, len_state_data, &object);
	if ( ! object.valueIsPresent)
	{
		idx_data = MLD_findOSV(dt_data, idx_data, dt_light_lx, state_data, len_state_data, &object);
		if (0 == idx_data)
		{
			err1("no data");
			return false;
		}
	}
	else if (dt_light_lx != object.value)
	{
		err1("wrong data");
		return false;
	}
	if ((0 == MLD_findOS(dt_value, idx_data, state_data, len_state_data, &object))
		||( ! object.valueIsPresent))
	{
		return false;
	}
	*p_value = object.value;

	return get_timestamp_seconds(idx_data, state_data, len_state_data, p_timestamp);
}


/**
 * Look for control values passed with data messages. Like jog-dials sending
 * color-temperature or override values. These appear in states where permitted
 * and if available.
 * @param subtype - the type to look for, dt_output_pct, dt_color_temperature, dt_rgbw etc.
 * @param p_value - memory for returning the value when found.
 * @param data - data buffer.
 * @param len_data - length of the data in the buffer.
 * @return true when found and value stored in p_value.
 */
static bool get_subdata_value (
    int32_t * p_value,
	uint32_t subtype,
    const uint8_t data[], uint16_t len_data)
{
	// Find a root data object in the input, don't care what the data itself is,
	// we care if it has any extra elements under it that we have been asked for.
	ml_object_t oroot;
	ml_iterator_t iter;
	MLI_initialize(&iter, data, len_data);
	while (0 != MLI_nextWithSubject(&iter, 0, &oroot))
	{
		if (dt_data == oroot.type)
		{
			ml_object_t osub;
			if (0 != MLD_findOSV(dt_data, oroot.index, subtype, data, len_data, &osub))
			{
				ml_object_t oval;
				if (MLD_findOS(dt_value, osub.index, data, len_data, &oval)&&(oval.valueIsPresent))
				{
					*p_value = oval.value;
					return true;
				}
			}
		}
	}

	return false;
}


/**
 * Retrieve the input value when control data received as MIST_ITEM_MOTEXML.
 * @param p_value - memory for returning the value.
 * @param data - data buffer.
 * @param len_data - length of the data in the buffer.
 * @return true when found and value stored in p_value.
 */
static bool get_input_value(int32_t * p_value, uint8_t input[], uint16_t input_length)
{
	ml_object_t object;
	if (MLD_findOS(dt_input, 0, input, input_length, &object))
	{
		if ((MLD_findOS(dt_value, object.index, input, input_length, &object))
			&&(object.valueIsPresent))
		{
			*p_value = object.value;
			return true;
		}
	}
	return false;
}


/**
 * Set the control value of the simulated luminaire.
 * @param value - desired control value.
 */
static void set_value (int32_t value)
{
	if(value > 100)
	{
		warn1("limit 100");
		value = 100;
	}
	else if(value < -1) // -1 is AC cut and 0 is soft-off, where applicable
	{
		warn1("limit -1");
		value = -1;
	}

	m_light_output = value;
	m_last_control_s = osCounterGetSecond();

	info1("CONTROL %"PRIi32, m_light_output);
	if(m_light_output > 0)
	{
		PLATFORM_LedsSet(PLATFORM_LedsGet() | (1 << 2)); // LED2 on
	}
	else
	{
		PLATFORM_LedsSet(PLATFORM_LedsGet() & ~(1 << 2)); // LED2 off
	}
}


/**
 * Example implementation of a light control function.
 * @param itype         - type of input, can be complex MoteXML data,
 *                        an int32 or NULL for reading current state into output.
 * @param input         - action function input, according to itype, NULL for read.
 * @param input_length  - length of the input data.
 * @param otype         - type of returned output, can be comlex MoteXML data or and int32.
 * @param output        - buffer for data output.
 * @param output_size   - size of the output array.
 * @param output_length - length of the data stored in the output array by the called function.
 * @return MIST_SUCCESS when call successful and otype value has been set.
 */
static mist_error_t light_control (
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype) // No input - return current state
	{
		debug1("read light_control");
	}
	else if (MIST_ITEM_INT32 == itype) // Simple control input, return set value
	{
		set_value(*((int32_t*)input));
		m_pid_initialized = false; // Direct set, forget regulator state
	}
	else if (MIST_ITEM_MOTEXML == itype) // Maybe it is dynamic illuminance control
	{
		int32_t target_lx;
		int32_t current_lx;
		uint32_t timestamp_lx;
		if ((get_target_lightsensor_value(&target_lx, input, input_length))
		  &&(get_current_lightsensor_value(&current_lx, &timestamp_lx, input, input_length)))
		{
			if (timestamp_lx > m_last_control_s)
			{
				int32_t error = target_lx - current_lx;
				int32_t derivative = 0;
				if ( ! m_pid_initialized)
				{
					m_pid_initialized = true;
					m_pid_integral = 0;
				}
				else
				{
					derivative = m_pid_last_error - error;
				}

				if ((m_pid_last_error < 0) == (error < 0))
				{
					m_pid_integral += error;
				}
				else
				{
					m_pid_integral = 0; // clear integral component when error sign changes
				}

				m_pid_last_error = error;

				set_value(m_light_output
				        + LUX_LC_PID_KP*error
				        + LUX_LC_PID_KI*m_pid_integral
				        + LUX_LC_PID_KD*derivative);
			}
		}
		else
		{
			// A case with many control options, probably hardware/platform
			// dependent in practice. In this example we try to parse some
			// options and reach at least some control value.
			bool have_value = false;
			int32_t value;
			// Look for default value in input
			if (get_input_value(&value, input, input_length))
			{
				have_value = true;
			}

			int32_t subval;
			// Look for pct in received data (buttons and other input devices)
			if (get_subdata_value(&subval, dt_output_pct, input, input_length))
			{
				// We replace the default value with the override
				value = subval;
				have_value = true;
			}

			int32_t color_temp = 0;
			// Look for colortemp in received data (buttons and other input devices)
			get_subdata_value(&color_temp, dt_color_temperature, input, input_length);

			uint32_t rgbw = 0; // Transported as int32_t, but we know it's 4 bytes
			// Look for RGBW in value (buttons and other input devices)
			if (get_subdata_value((int32_t*)&rgbw, dt_rgbw, input, input_length))
			{
				// We replace the default or override value with the white channel,
				// it is up to the actual implementations to handle multiple inputs,
				// for example tread the value as overal brightness and the RGBW
				// information as a form of color temperature.
				value = (uint8_t)rgbw; // Take the white channel
				have_value = true;
			}

			if (have_value)
			{
				// For the example we just print the color temp and RGBW values,
				// it is up to the real implementations use them.
				info1("CTEMP: %d RGBW: %08X", color_temp, rgbw);
				set_value(value);
			}
			else
			{
				err1("no ctrl value");
				return MIST_FAIL;
			}
		}
	}
	else // Some unknown type of input ???
	{
		err1("light_control itype %d", (int)itype);
		return MIST_FAIL;
	}

	*otype = MIST_ITEM_INT32;
	*((int32_t*)output) = m_light_output;
	*output_length = sizeof(int32_t);
	return MIST_SUCCESS;
}


/**
 * Register a handler for light control with the Mist middleware.
 * @return true when successfully registered.
 */
bool mist_mod_lighting_init ()
{
	// Register lighting control
	m_lighting_module.data_type = dt_light_control;
	m_lighting_module.function = light_control;
	memset(m_lighting_module.uuid, 0, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_lighting_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
		return false;
	}
	return true;
}
