/**
 * Mist middleware management API.
 *
 * Copyright Thinnect Inc. 2020
 * @license <PROPRIETARY>
 */
#ifndef MIST_MIDDLEWARE_H_
#define MIST_MIDDLEWARE_H_

#include <stdint.h>

#include "mist_comm.h"

#include "mist_middleware_types.h"

#define AMID_MIST_SUBS 0xE0
#define AMID_MIST_DATA 0xE1

/**
 * Mist action function prototype.
 *
 * @param params    - array of MoteXML encoded parameters.
 * @param param_len - length of the parameters array
 * @param results   - output array.
 * @param param_len - length of the output array
 * @param params    - array of MoteXML encoded parameters.
 * @param param_len - length of the parameters array
 * @return 0 when call successful.
 */
typedef mist_error_t (*mist_action_function_f)(uint8_t params[], uint16_t param_len,
	                                 uint8_t results[], uint16_t result_len,
	                                 uint16_t * outlen);

/**
 * Mist middleware action module structure. Must be filled for registration.
 *
 * @param data_type - Type of the data provided by the module (dt_types).
 * @param function  - Action fuction to be called to execute control and/or read data.
 */
typedef struct mist_module {
	uint32_t data_type;
	mist_action_function_f function;
} mist_module_t;

/**
 * Initialize the Mist Middleware.
 *
 * @param radio - Pointer to a communications interface to use.
 */
void mist_middleware_init(comms_layer_t * radio);

/**
 * Register an action handler.
 *
 * @param module - pointer to a module for registration.
 *
 * @return 0 if registered successfully.
 */
mist_error_t mist_register_handler(mist_module_t * module);


/**
 * Notify mist middleware about a spontaneous event from an action handler.
 *
 * @param module - pointer to an already registered module generating this event.
 * @param data   - event data.
 * @param length - length of the data.
 *
 * @return 0 when event processed.
 */
mist_error_t mist_spontaneous_event(mist_module_t * module, uint8_t data[], uint16_t length);

#endif//MIST_MIDDLEWARE_H_
