/**
 * @file motor_control.c
 * @brief Motor control implementation with safety features
 * @author Motor Controller Team
 * @date 2025
 */

#include "motor_control.h"
#include "global.h"
#include "driver/gpio.h"
#include "esp_log.h"

/** @brief Logging tag for motor control operations */
static const char *TAG = "MOTOR_CONTROL";

/** @brief Local definition of motor control pin */
#define MOTOR_PIN GPIO_NUM_2

/**
 * @brief Set motor control pins to floating state for safety
 * 
 * Configures the motor control pin as input with floating state,
 * effectively disconnecting the motor control signal. This ensures
 * the motor is safely OFF and not consuming power.
 */
void set_pins_to_float(void) {
    gpio_set_direction(MOTOR_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(MOTOR_PIN, GPIO_FLOATING);
}

/**
 * @brief Turn the motor ON
 * 
 * Activates the motor by:
 * 1. Setting the control pin as output
 * 2. Setting pin LOW (active low control)
 * 3. Updating global motor state
 * 4. Logging the action
 */
void motor_on(void) {
    gpio_set_direction(MOTOR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR_PIN, 0); // Turn ON (LOW)
    motor_state = true;
    ESP_LOGI(TAG, "Motor turned ON");
}

/**
 * @brief Turn the motor OFF safely
 * 
 * Deactivates the motor by:
 * 1. Setting pins to floating state for safety
 * 2. Updating global motor state
 * 3. Logging the action
 * 
 * Uses floating pins instead of HIGH signal for additional safety.
 */
void motor_off(void) {
    set_pins_to_float();
    motor_state = false;
    ESP_LOGI(TAG, "Motor turned OFF");
}
