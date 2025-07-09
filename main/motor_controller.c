/**
 * @file motor_controller.c
 * @mainpage Motor Controller Firmware
 * @brief Main application entry point and system initialization.
 *
 * This file contains the initialization of the system, WiFi setup,
 * HTTP server launch, and control loop for the motor controller firmware.
 *
 * @author Eranga Wijebandara
 * @date 2025
 */



// Add this include
#include "global.h"
#include "schedule.h"
#include "storage.h"
#include "wifi_ap.h"
#include "webserver.h"

/** @brief Logging tag for main application */
static const char *TAG = "MAIN";

/** @defgroup Global_Variables Global Variable Definitions
 *  @brief Actual definitions of global system variables
 *  @{
 */
bool motor_state = false;                                   /**< Current motor state */
int32_t schedule_count = 0;                                /**< Number of active schedules */
int32_t history_count = 0;                                 /**< Number of history entries */
char device_name[MAX_NAME_LEN] = "Motor Controller";       /**< Device name */
char device_location[MAX_NAME_LEN] = "Green House";        /**< Device location */
schedule_t schedules[MAX_SCHEDULES];                       /**< Schedule array */
schedule_history_t schedule_history[MAX_HISTORY];         /**< History array */
saved_time_t last_known_time;                              /**< Saved time data */
httpd_handle_t server = NULL;                              /**< HTTP server handle */
nvs_handle_t g_nvs_handle;                                 /**< NVS storage handle */
/** @} */

/**
 * @brief Background task for schedule checking and time management
 * 
 * This FreeRTOS task runs continuously to:
 * - Check and execute schedules every 30 seconds
 * - Save current time to flash every 5 minutes
 * - Maintain system timing for automated operations
 * 
 * @param pvParameters Task parameters (unused)
 */
void schedule_check_task(void *pvParameters) {
    static uint32_t last_check = 0;
    static uint32_t last_time_save = 0;
    
    while (1) {
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS; // FIXED
        
        // Check schedules every 30 seconds
        if (current_time - last_check >= 30000) {
            check_schedules();
            last_check = current_time;
        }
        
        // Save time every 5 minutes
        if (current_time - last_time_save >= 300000) {
            save_time_to_flash();
            last_time_save = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds
    }
}

/**
 * @brief Main application entry point
 * 
 * Initializes all system components in the following order:
 * 1. GPIO configuration for motor control
 * 2. Non-volatile storage (NVS)
 * 3. Load saved settings and time
 * 4. WiFi Access Point setup
 * 5. SNTP time synchronization
 * 6. Web server startup
 * 7. Motor indication sequence
 * 8. Background task creation
 * 
 * The system provides visual feedback through LED blinking during startup.
 */
void app_main(void) {
    ESP_LOGI(TAG, "Starting Motor Controller...");
    
    // Initialize GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = MOTOR_PIN_SEL,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    gpio_set_level(MOTOR_PIN, 1); // Start with motor OFF
    
    // Initialize NVS
    init_nvs();
    
    // Load settings and history
    load_settings();
    restore_time_from_flash();
    load_history_from_flash();
    
    // Initialize WiFi
    wifi_init_softap();
    
    // Initialize time
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    
    // Set timezone
    setenv("TZ", "UTC-5:30", 1);
    tzset();
    
    // Start web server
    start_webserver();
    
    // Motor startup indication (3 blinks)
    for (int i = 0; i < 3; i++) {
        gpio_set_level(MOTOR_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level(MOTOR_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // Create schedule check task
    xTaskCreate(schedule_check_task, "schedule_check", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Motor Controller started successfully!");
}

