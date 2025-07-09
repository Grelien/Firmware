/**
 * @file storage.c
 * @brief Persistent Storage Implementation using NVS
 * @author Motor Controller Project
 * @date 2024
 * 
 * This module implements persistent storage functionality using ESP32's NVS
 * (Non-Volatile Storage) system. It handles storage and retrieval of device
 * settings, schedules, history, and time information. All data is automatically
 * saved to flash memory and persists across device reboots.
 * 
 * @defgroup Storage Storage Management
 * @{
 */

#include "global.h"
#include "storage.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "time.h"

static const char *TAG = "STORAGE"; ///< Logging tag for storage module

/**
 * @brief Initialize NVS storage system
 * 
 * Initializes the ESP32 NVS flash storage and opens a handle for the motor
 * scheduler namespace. If NVS is corrupted or incompatible, it will be erased
 * and reinitialized. This function must be called before any other storage
 * operations.
 * 
 * @note This function will cause a system restart if NVS initialization fails
 * @see load_settings(), save_settings()
 */
void init_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ret = nvs_open("motorscheduler", NVS_READWRITE, &g_nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(ret));
    }
}

/**
 * @brief Load device settings and schedules from NVS
 * 
 * Retrieves all stored device settings including device name, location, and
 * all scheduled motor operations from NVS flash storage. If data is corrupted
 * or missing, appropriate defaults are used. Schedule count is validated to
 * prevent buffer overflows.
 * 
 * @note Should be called during system initialization after init_nvs()
 * @see save_settings(), init_nvs()
 */
void load_settings(void) {
    ESP_LOGI(TAG, "Loading settings...");
    
    // Load device settings
    size_t required_size = sizeof(device_name);
    nvs_get_str(g_nvs_handle, "device_name", device_name, &required_size);
    required_size = sizeof(device_location);
    nvs_get_str(g_nvs_handle, "device_loc", device_location, &required_size);
    
    // Load schedule count
    nvs_get_i32(g_nvs_handle, "sched_count", &schedule_count);
    if (schedule_count < 0 || schedule_count > MAX_SCHEDULES) {
        schedule_count = 0;
    }
    
    // Load schedules
    for (int i = 0; i < schedule_count && i < MAX_SCHEDULES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "sched_%d", i);
        size_t required_size = sizeof(schedule_t);
        esp_err_t err = nvs_get_blob(g_nvs_handle, key, &schedules[i], &required_size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error loading schedule %d: %s", i, esp_err_to_name(err));
            schedules[i].enabled = false;
        }
    }
    
    ESP_LOGI(TAG, "Settings loaded. Schedule count: %d", schedule_count);
}

/**
 * @brief Save device settings and schedules to NVS
 * 
 * Stores all current device settings including device name, location, and
 * all motor schedules to NVS flash storage. Data is committed to ensure
 * immediate persistence. Individual schedule save failures are logged but
 * don't prevent other schedules from being saved.
 * 
 * @note This function should be called whenever settings are modified
 * @see load_settings(), save_history_to_flash()
 */
void save_settings(void) {
    ESP_LOGI(TAG, "Saving settings...");
    
    // Save device name and location
    nvs_set_str(g_nvs_handle, "device_name", device_name);
    nvs_set_str(g_nvs_handle, "device_location", device_location);
    
    // Save schedule count
    nvs_set_i32(g_nvs_handle, "sched_count", schedule_count);
    
    // Save all schedules
    for (int i = 0; i < schedule_count && i < MAX_SCHEDULES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "sched_%d", i);
        esp_err_t err = nvs_set_blob(g_nvs_handle, key, &schedules[i], sizeof(schedule_t));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error saving schedule %d: %s", i, esp_err_to_name(err));
        }
    }
    
    nvs_commit(g_nvs_handle);
    ESP_LOGI(TAG, "Settings saved successfully");
}

// Save irrigation history to NVS
void save_history_to_flash(void) {
    ESP_LOGI(TAG, "Saving irrigation history...");
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        char key[16];
        snprintf(key, sizeof(key), "hist_%d", i);
        nvs_set_blob(g_nvs_handle, key, &schedule_history[i], sizeof(schedule_history_t));
    }
    nvs_set_i32(g_nvs_handle, "hist_count", history_count);
    nvs_commit(g_nvs_handle);
    ESP_LOGI(TAG, "Irrigation history saved successfully");
}

// Load irrigation history from NVS
void load_history_from_flash(void) {
    ESP_LOGI(TAG, "Loading irrigation history...");

    nvs_get_i32(g_nvs_handle, "hist_count", &history_count);
    if (history_count < 0 || history_count > MAX_HISTORY) {
        history_count = 0;
    }

    for (int i = 0; i < MAX_HISTORY; i++) {
        char key[16];
        snprintf(key, sizeof(key), "hist_%d", i);
        size_t required_size = sizeof(schedule_history_t);
        esp_err_t err = nvs_get_blob(g_nvs_handle, key, &schedule_history[i], &required_size);
        if (err != ESP_OK) {
            memset(&schedule_history[i], 0, sizeof(schedule_history_t));
        }
    }

    ESP_LOGI(TAG, "Irrigation history loaded successfully");
}

void save_time_to_flash(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    nvs_set_i32(g_nvs_handle, "time_year", timeinfo.tm_year + 1900);
    nvs_set_i32(g_nvs_handle, "time_month", timeinfo.tm_mon + 1);
    nvs_set_i32(g_nvs_handle, "time_day", timeinfo.tm_mday);
    nvs_set_i32(g_nvs_handle, "time_hour", timeinfo.tm_hour);
    nvs_set_i32(g_nvs_handle, "time_minute", timeinfo.tm_min);
    nvs_set_i32(g_nvs_handle, "time_second", timeinfo.tm_sec);
    nvs_set_u8(g_nvs_handle, "time_valid", 1);
    nvs_commit(g_nvs_handle);
    ESP_LOGI(TAG, "Time saved to flash");
}

void restore_time_from_flash(void) {
    uint8_t is_valid = 0;
    nvs_get_u8(g_nvs_handle, "time_valid", &is_valid);
    
    if (is_valid) {
        struct tm tm = {0};
        nvs_get_i32(g_nvs_handle, "time_year", &last_known_time.year);
        nvs_get_i32(g_nvs_handle, "time_month", &last_known_time.month);
        nvs_get_i32(g_nvs_handle, "time_day", &last_known_time.day);
        nvs_get_i32(g_nvs_handle, "time_hour", &last_known_time.hour);
        nvs_get_i32(g_nvs_handle, "time_minute", &last_known_time.minute);
        nvs_get_i32(g_nvs_handle, "time_second", &last_known_time.second);
        
        tm.tm_year = last_known_time.year - 1900;
        tm.tm_mon = last_known_time.month - 1;
        tm.tm_mday = last_known_time.day;
        tm.tm_hour = last_known_time.hour;
        tm.tm_min = last_known_time.minute;
        tm.tm_sec = last_known_time.second;
        
        time_t t = mktime(&tm);
        struct timeval now = { .tv_sec = t };
        settimeofday(&now, NULL);
        
        char time_str[64];
        get_current_time_str(time_str, sizeof(time_str));
        ESP_LOGI(TAG, "Time restored from flash: %s", time_str);
    }
}
