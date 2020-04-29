/**
 * Compatibility header for some NesC headers.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef NESC_TO_C_COMPAT_H_
#define NESC_TO_C_COMPAT_H_

#include <stdint.h>

#define nx_struct struct
typedef uint8_t nx_uint8_t;
typedef uint16_t nx_uint16_t;
typedef uint32_t nx_uint32_t;
typedef int8_t nx_int8_t;
typedef int16_t nx_int16_t;
typedef int32_t nx_int32_t;

#endif//NESC_TO_C_COMPAT_H_
