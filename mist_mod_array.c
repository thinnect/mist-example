/**
 * Basic example for sending multiple values of the same type.
 *
 * Two different scenarios are possible:
 * 1) A sequence of events - for example 10 temperature readings taken
 *    every 60 seconds.
 * 2) A number of readings of the same type taken from different data channels
 *    - for example readings taken from 10 different temperature sensors at the
 *    same time.
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

#include "yxktime.h"

#include "loglevels.h"
#define __MODUUL__ "m_m_a"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_array & BASE_LOG_LEVEL)
#include "log.h"

#define DATA_ARRAY_ELEMENTS 10

static bool m_parallel;

static mist_module_t m_example_array_module;

static uint16_t format_sequential_array (uint8_t buffer[], uint16_t length)
{
	ml_encoder_t enc;
	if(ML_SUCCESS == MLE_initialize(&enc, buffer, length))
	{
		// Values: 10.0, 20.0, 30.0, 40.0. 50.0, 60.0, 70.0, 80.0, 90.0, 1000.0
		//static int16_t values[DATA_ARRAY_ELEMENTS];
		static int16_t values[DATA_ARRAY_ELEMENTS] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

		// Timestamp offsets from the dt_timestamp_utc listed below for each
		// element of the values array
		static int16_t times[] = {-60,-120,-180,-240,-300,-360,-420,-480,-540,-600};

		uint8_t index1 = MLE_appendOV(&enc, dt_data, dt_temperature_C);
		uint8_t index2 = MLE_appendOS(&enc, dt_value, index1);

		// Append the array object, array element size is determined from buffer
		// size and array object value (number of elements)
		MLE_appendOSVB(&enc, dt_array, index2, sizeof(values)/sizeof(int16_t), (uint8_t*)values, sizeof(values));

		// All the values in the array have been multiplied by 10, so tell
		// the receiver to divide by 10
		MLE_appendOSV(&enc, dt_exp, index2, -1);

		// Production timestamp array, the values are sequential, timestamps
		// offset from dt_timestamp_utc
		uint8_t index3 = MLE_appendOS(&enc, dt_production_start, index1);
		MLE_appendOSVB(&enc, dt_array, index3, sizeof(times)/sizeof(int16_t), (uint8_t*)times, sizeof(times));

		// Add an UTC timestamp with the epoch at 2000-01-01
		MLE_appendOSV(&enc, dt_timestamp_utc, index1, 604401880); // time_yxk(time(NULL)));
		return MLE_finalize(&enc);
	}
	return 0;
}

static uint16_t format_parallel_array (uint8_t buffer[], uint16_t length)
{
	ml_encoder_t enc;
	if(ML_SUCCESS == MLE_initialize(&enc, buffer, length))
	{
		// Values: 10.0, 20.0, 30.0, 40.0. 50.0, 60.0, 70.0, 80.0, 90.0, 1000.0
		static int16_t values[DATA_ARRAY_ELEMENTS] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

		uint8_t index1 = MLE_appendOV(&enc, dt_data, dt_temperature_C);
		uint8_t index2 = MLE_appendOS(&enc, dt_value, index1);

		// Append the array object, array element size is determined from buffer
		// size and array object value (number of elements)
		MLE_appendOSVB(&enc, dt_array, index2, sizeof(values)/sizeof(int16_t), (uint8_t*)values, sizeof(values));

		// All the values in the array have been multiplied by 10, so tell
		// the receiver to divide by 10
		MLE_appendOSV(&enc, dt_exp, index2, -1);

		// The array contains data elements with the same timestamp from
		// different channels (sensors)
		MLE_appendOS(&enc, dt_parallel, index1);
		// A parallel array is interpreted by the gateway and split into
		// different channels for the JSON output format. The data types end up
		// being dt_temperature_C_0, dt_temperature_C_1 ...
		// This transformation is currently not reversible

		// Add an UTC timestamp with the epoch at 2000-01-01
		MLE_appendOSV(&enc, dt_timestamp_utc, index1, 604401880); // time_yxk(time(NULL)));
		return MLE_finalize(&enc);
	}
	return 0;
}

static mist_error_t array_data_func (
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype)
	{
		*otype = MIST_ITEM_MOTEXML;
		if (m_parallel)
		{
			*output_length = format_parallel_array(output, output_size);
		}
		else
		{
			*output_length = format_sequential_array(output, output_size);
		}
		return MIST_SUCCESS;
	}
	return MIST_FAIL;
}

bool mist_mod_array_init (bool parallel)
{
	m_parallel = parallel;

	// Register the example sensor
	m_example_array_module.data_type = dt_temperature_C;
	m_example_array_module.function = array_data_func;
	memset(m_example_array_module.uuid, 0, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_example_array_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
		return false;
	}
	return true;
}
