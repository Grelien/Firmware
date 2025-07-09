/**
 * @file webserver.h
 * @brief Web server interface and HTTP handler declarations
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once

#include "esp_http_server.h"
#include "types.h"

/** @defgroup Web_Server Web Server Functions
 *  @brief HTTP server setup and request handlers
 *  @{
 */

/**
 * @brief Start the HTTP web server
 * 
 * Initializes and starts the HTTP server with all necessary URI handlers
 * for motor control, schedule management, and system configuration.
 * 
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t start_webserver(void);

/** @} */

/** @defgroup HTTP_Handlers HTTP Request Handlers
 *  @brief Individual handlers for different web interface pages and APIs
 *  @{
 */

/**
 * @brief Handle requests to the root page (/)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t root_handler(httpd_req_t *req);

/**
 * @brief Handle motor ON requests (/motor/on)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t motor_on_handler(httpd_req_t *req);

/**
 * @brief Handle motor OFF requests (/motor/off)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t motor_off_handler(httpd_req_t *req);

/**
 * @brief Handle status API requests (/status)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t status_handler(httpd_req_t *req);

/**
 * @brief Handle time display requests (/time)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t time_handler(httpd_req_t *req);

/**
 * @brief Handle schedule management page requests (/schedules)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t schedules_handler(httpd_req_t *req);

/**
 * @brief Handle add schedule requests (/add_schedule)
 * @param req HTTP request handle containing schedule data
 * @return ESP_OK on success
 */
esp_err_t add_schedule_handler(httpd_req_t *req);

/**
 * @brief Handle delete schedule requests (/delete_schedule)
 * @param req HTTP request handle containing schedule index
 * @return ESP_OK on success
 */
esp_err_t delete_schedule_handler(httpd_req_t *req);

/**
 * @brief Handle settings page requests (/settings)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t settings_handler(httpd_req_t *req);

/**
 * @brief Handle save settings requests (/save_settings)
 * @param req HTTP request handle containing new settings
 * @return ESP_OK on success
 */
esp_err_t save_settings_handler(httpd_req_t *req);

/**
 * @brief Handle manual time setting requests (/manual_time)
 * @param req HTTP request handle containing time data
 * @return ESP_OK on success
 */
esp_err_t manual_time_handler(httpd_req_t *req);

/**
 * @brief Handle logo image requests (/logo.png)
 * @param req HTTP request handle
 * @return ESP_OK on success
 */
esp_err_t logo_handler(httpd_req_t *req);

/** @} */

/** @defgroup Utility_Functions Utility Functions
 *  @brief Helper functions for web server operations
 *  @{
 */

/**
 * @brief URL decode a string in-place
 * 
 * Converts URL-encoded characters (e.g., %20 for space) back to
 * their original form for processing form data.
 * 
 * @param dst Destination buffer for decoded string
 * @param src Source URL-encoded string
 */
void url_decode(char *dst, const char *src);

/** @} */
