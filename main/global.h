/**
 * @file global.h
 * @brief Global definitions, constants, and variable declarations for Motor Controller
 * @author Motor Controller Team
 * @date 2025
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "types.h"

/** @brief Utility macro to find minimum of two values */
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/** @defgroup WiFi_Config WiFi Access Point Configuration
 *  @brief WiFi network settings for the motor controller
 *  @{
 */
#define AP_SSID "Motor Controller"  /**< WiFi network name */
#define AP_PASSWORD "12345679"      /**< WiFi network password */
#define AP_MAX_STA_CONN 4           /**< Maximum simultaneous connections */
/** @} */

/** @defgroup GPIO_Config GPIO Pin Configuration
 *  @brief Hardware pin assignments
 *  @{
 */
#define MOTOR_PIN GPIO_NUM_2        /**< GPIO pin connected to motor control */
#define MOTOR_PIN_SEL (1ULL << MOTOR_PIN) /**< Pin selection mask for GPIO configuration */
/** @} */

/** @defgroup Time_Config Time and Timezone Configuration
 *  @brief Time synchronization settings
 *  @{
 */
#define GMT_OFFSET_SEC 19800        /**< GMT+5:30 for Sri Lanka (in seconds) */
#define DAYLIGHT_OFFSET_SEC 0       /**< Daylight saving time offset */
/** @} */

/** @defgroup Global_Variables Global Variable Declarations
 *  @brief External declarations for global system variables
 *  @{
 */
extern bool motor_state;                                    /**< Current motor state (true=ON, false=OFF) */
extern int32_t schedule_count;                             /**< Number of configured schedules */
extern int32_t history_count;                              /**< Number of history entries */
extern char device_name[MAX_NAME_LEN];                     /**< User-configurable device name */
extern char device_location[MAX_NAME_LEN];                 /**< User-configurable device location */
extern schedule_t schedules[MAX_SCHEDULES];                /**< Array of configured schedules */
extern schedule_history_t schedule_history[MAX_HISTORY];   /**< Array of schedule execution history */
extern saved_time_t last_known_time;                       /**< Last known time for persistence */
extern httpd_handle_t server;                              /**< HTTP server handle */
extern nvs_handle_t g_nvs_handle;                          /**< NVS storage handle */
/** @} */

/** @defgroup Logo_Data Embedded Logo Data
 *  @brief Binary data for web interface logo
 *  @{
 */
extern const uint8_t logo_data[] asm("_binary_logo_png_start"); /**< Start of embedded logo data */
extern const uint8_t logo_end[] asm("_binary_logo_png_end");    /**< End of embedded logo data */
/** @} */

#endif // GLOBAL_H