/**
 * Init function and parameters for a mist examples.
 *
 * Copyright Thinnect Inc. 2020
 * @license MIT
 */
#ifndef MIST_EXAMPLE_H_
#define MIST_EXAMPLE_H_

#include <stdbool.h>

bool mist_mod_lighting_init();
bool mist_mod_movement_init();
bool mist_mod_button_init();
bool mist_mod_lux_init();
bool mist_mod_array_init(bool parallel);

#endif//MIST_EXAMPLE_H_
