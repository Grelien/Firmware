/**
 * @file motor_control.h
 * @brief Motor control interface and function declarations
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once

#include <stdbool.h>

/** @defgroup Motor_Control Motor Control Functions
 *  @brief Functions for controlling the motor hardware
 *  @{
 */

/**
 * @brief Current motor state variable
 * @note Actual declaration is in global.h
 */
extern bool motor_state;

/**
 * @brief Set motor control pins to floating state
 * 
 * This function configures the motor control pin as input with floating
 * state, effectively disconnecting the motor. This is used for safety
 * when the motor should be OFF.
 */
void set_pins_to_float(void);

/**
 * @brief Turn the motor ON
 * 
 * Configures the motor control pin as output and sets it LOW to activate
 * the motor. Updates the global motor_state variable and logs the action.
 */
void motor_on(void);

/**
 * @brief Turn the motor OFF
 * 
 * Sets the motor control pin to floating state and updates the global
 * motor_state variable. Logs the action for debugging purposes.
 */
void motor_off(void);

/** @} */