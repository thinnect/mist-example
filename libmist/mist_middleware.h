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
 * Called when mist middleware executes control or queries state / samples for data.
 *
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
 * Get the mistmiddleware library version.
 *
 * @param major - Pointer to store major version, may be NULL.
 * @param minor - Pointer to store minor version, may be NULL.
 * @param patch - Pointer to store patch version, may be NULL.
 * @return \0 terminated string representation of the version.
 */
const char * mist_middleware_version(uint8_t * major, uint8_t * minor, uint8_t * patch);

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
 * Start the middleware - register receivers, start thread.
 *
 * Middleware must be initialized before calling start.
 *
 * Handlers should be registered before calling start, otherwise stored
 * rules could be deleted when their handler is not registered and they are
 * loaded from storage.
 *
 * @return MIST_SUCCESS if started successfully.
 */
mist_error_t mist_middleware_start();

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
