/**
 * Basic example for setting up a light sensor.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */

#include "mist_middleware.h"

#include "dt_types.h"
#include "MLE.h"

#include "platform.h"

#include "retargeti2c.h"

#include <inttypes.h>
#include <string.h>

#include "loglevels.h"
#define __MODUUL__ "m_m_lx"
#define __LOG_LEVEL__ (LOG_LEVEL_mist_mod_illuminance & BASE_LOG_LEVEL)
#include "log.h"

// d22721b7-c5cc-4d69-bfe7-50f11aaab845
#define UUID_LIGHT_LX_RESOURCE ((uint8_t*)"\xd2\x27\x21\xb7\xc5\xcc\x4d\x69\xbf\xe7\x50\xf1\x1a\xaa\xb8\x45")

static mist_module_t m_illuminance_module;

static int32_t m_light_lx = 0;

#define VEML6030_ADDR 0x10
//#define VEML6030_ADDR 0x48

/**
 * Really basic lightsensor read implementation for VEML6030.
 */
static int32_t veml6030_read ()
{
	int32_t lux = -1;

	// platform_i2c_request(I2C0, osWaitForever); // TODO: I2C bus must first be acuired.
	RETARGET_I2CInit();

	// gain = 0b11 (1/4), it = 100ms
	if (0 == RETARGET_I2CWrite(VEML6030_ADDR, 0x00, (uint8_t *)"\x00\x11", 2))
	{
		debug2("initd");
		osDelay(1000);

		uint8_t buffer[2];
		if (0 == RETARGET_I2CRead(VEML6030_ADDR, 0x04, buffer, sizeof(buffer)))
		{
			lux = (buffer[0] | ((uint16_t)buffer[1] << 8));
			lux = lux * 2304;
			lux = lux / 10000;
			debug1("rd %"PRIu32, lux);
		}
		else
		{
			err1("i2c_r");
		}
	}
	else
	{
		err1("i2c_w");
	}

	RETARGET_I2CDeinit();
	// platform_i2c_release(I2C0); // TODO: and subsequently released

	return lux;
}


static mist_error_t light_sensor(
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length)
{
	if (MIST_ITEM_NULL == itype) // No input - return current state
	{
		debug1("read");
		*otype = MIST_ITEM_INT32;
		*((int32_t*)output) = m_light_lx;
		*output_length = sizeof(int32_t);
		return MIST_SUCCESS;
	}

	return MIST_FAIL; // Control actions not supported by movement sensor
}

// Fake a motion detector by polling the platform button (assuming there is something behind it)
static void light_sensor_thread(void * arg)
{
	for (;;)
	{
		osDelay(10000);
		m_light_lx = veml6030_read();
		info1("light %"PRIi32, m_light_lx);

		mist_error_t r = mist_spontaneous_event(&m_illuminance_module, MIST_ITEM_INT32, &m_light_lx, sizeof(int32_t));
		if (MIST_SUCCESS != r)
		{
			warn1("mov spnt evt %d", (int)r); // Something went wrong
		}
	}
}

void mist_mod_lux_init()
{
	// Register movement sensor
	m_illuminance_module.data_type = dt_light_lx;
	m_illuminance_module.function = light_sensor;
	memcpy(m_illuminance_module.uuid, UUID_LIGHT_LX_RESOURCE, UUID_LENGTH);

	mist_error_t result = mist_register_handler(&m_illuminance_module);
	if (MIST_SUCCESS != result)
	{
		err1("mist reg %d", result);
	}

	const osThreadAttr_t thread_attr = { .name = "lux", .stack_size = 2048 };
	osThreadNew(light_sensor_thread, NULL, &thread_attr);
}
