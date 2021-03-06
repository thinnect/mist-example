/**
 * Mist middleware basic types.
 *
 * Copyright Thinnect Inc. 2020
 * @license <PROPRIETARY>
 */
#ifndef MIST_MIDDLEWARE_TYPES_H_
#define MIST_MIDDLEWARE_TYPES_H_

#include <stdint.h>

/**
 * Mist success-fail type. 0 for success.
 */
typedef enum mist_error
{
	MIST_ERR_PRIORITY = -20, // Internal type, TODO fix naming scheme
	MIST_ETIMEOUT     = -12,
	MIST_ERETRY       =  -7,
	MIST_EBUSY        =  -5,
	MIST_ESIZE        =  -2,
	MIST_FAIL         =  -1,
	MIST_SUCCESS      =   0
} mist_error_t;

/**
 * Local subscription id: 0 <= lid < SUBS_LOCALID_NONE
 */
typedef uint8_t local_subs_id_t;
#define SUBS_LOCALID_NONE 0xFF

/**
 * Data stream id: 0 <= sid < SUBS_STREAM_NONE
 */
typedef uint8_t subs_stream_id_t;
#define SUBS_STREAM_NONE 0xFF

/**
 * Subscription id: 0 < cid < 0xFFFFFFFF
 */
typedef uint32_t csubs_id_t;

/**
 * Subscription slot id: 0 < slid < ?
 */
typedef uint8_t csubs_slot_t;

/**
 * Data delivery channel sequence id: 0 < seq < ?
 */
typedef uint32_t cseq_id_t;

#endif//MIST_MIDDLEWARE_TYPES_H_
