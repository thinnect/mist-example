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

#ifndef UUID_LENGTH // It should come from some other header
#define UUID_LENGTH 16
#endif//UUID_LENGTH

typedef enum mist_item_type
{
	MIST_ITEM_NULL,
	MIST_ITEM_MOTEXML,
	MIST_ITEM_INT32,
} mist_item_type_t;

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
typedef mist_error_t (*mist_action_function_f) (
            mist_item_type_t   itype, void * input,  uint16_t input_length,
            mist_item_type_t * otype, void * output, uint16_t output_size, uint16_t * output_length
);

/**
 * Mist middleware action module structure. Must be filled for registration.
 *
 * @param data_type - Type of the data provided by the module (dt_types).
 * @param uuid - When a resource type, fill with resource ID, set to zeros otherwise.
 * @param function  - Action fuction to be called to execute control and/or read data.
 */
typedef struct mist_module {
	uint32_t data_type;
	uint8_t uuid[UUID_LENGTH]; // Canonical byte order!!! // TODO replace with a struct from some base library
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
 * @param module - pointer to a module for registration. Don't free this object.
 *
 * @return MIST_SUCCESS if registered successfully.
 */
mist_error_t mist_register_handler(mist_module_t * module);

/**
 * Notify mist middleware about a spontaneous event from an action handler.
 *
 * @param module - pointer to an already registered module generating this event.
 * @param itype - how the following data pointer should be treated.
 * @param data - event data.
 *     Simple (MIST_ITEM_MOTEXML or automatically created when MIST_ITEM_INT32):
 *        <dt_data value="A_DATA_TYPE">
 *            <dt_value value="AN_INTEGER"/>
 *                <dt_exp value="AN_OPTIONAL_EXPONENT"/>
 *            </dt_value>
 *        </dt_data>
 *     Complex (MIST_ITEM_MOTEXML):
 *        ~ more complex structures may be encoded at the root ~
 * @param length - length of the data.
 *
 * @return MIST_SUCCESS when event processed.
 */
mist_error_t mist_spontaneous_event(mist_module_t * module, mist_item_type_t itype, void * data, uint16_t length);

#endif//MIST_MIDDLEWARE_H_
