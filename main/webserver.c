/**
 * @file webserver.c
 * @brief HTTP Web Server Implementation for Motor Controller
 * @author Motor Controller Team
 * @date 2025
 * 
 * This module implements a comprehensive HTTP web server that provides a
 * web-based interface for motor control and management. It serves HTML pages,
 * handles REST API endpoints, and manages all user interactions through the
 * web interface. The server supports motor control, schedule management,
 * device settings, and system monitoring.
 * 
 * @defgroup Web_Server Web Server Module
 * @{
 */

#include "global.h"
#include "webserver.h"
#include "motor_control.h"
#include "schedule.h"
#include "storage.h"
#include "wifi_ap.h"
#include "time_util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_http_server.h"

/** @brief Logging tag for web server operations */
static const char *TAG = "WEBSERVER";

/**
 * @brief Main dashboard page handler
 * 
 * Serves the primary web interface dashboard with motor status, device information,
 * upcoming schedules, and irrigation history. Dynamically generates HTML with
 * current system state and provides interactive motor controls.
 * 
 * @param req HTTP request handle
 * @return ESP_OK on success, ESP_FAIL on error
 * 
 * @note Allocates up to 16KB for HTML generation
 * @note Automatically refreshes every 30 seconds via JavaScript
 */
esp_err_t root_handler(httpd_req_t *req) {
    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));
    
    // Allocate larger buffer for the enhanced HTML
    char *html = malloc(16384);
    if (!html) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    // Build the basic HTML structure
    int html_len = snprintf(html, 16384,
        "<!DOCTYPE html><html>"
        "<head><title>Motor Controller</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body { font-family: Arial, sans-serif; margin: 0; background: #e8f5e9; color: #333; }"
        ".container { max-width: 800px; margin: 20px auto; background: white; padding: 20px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.2); }"
        "h1 { color: #2e7d32; text-align: center; margin-bottom: 20px; font-size: 28px; }"
        "h3 { color: #2e7d32; margin-bottom: 15px; font-size: 20px; }"
        ".logo { display: block; margin: 0 auto 20px; width: 100px; height: auto; }"
        ".info-section { background: #c8e6c9; padding: 15px; border-radius: 10px; margin-bottom: 20px; text-align: center; }"
        ".status { font-size: 18px; text-align: center; padding: 15px; border-radius: 10px; margin-bottom: 20px; }"
        ".status.on { background: #a5d6a7; color: #1b5e20; }"
        ".status.off { background: #ef9a9a; color: #b71c1c; }"
        ".schedules-info { background: #fff3cd; padding: 15px; border-radius: 10px; margin-bottom: 20px; }"
        ".history-section { background: #d1c4e9; padding: 15px; border-radius: 10px; margin-bottom: 20px; }"
        ".button { background: #4CAF50; color: white; padding: 12px 24px; border: none; border-radius: 25px; cursor: pointer; font-size: 16px; margin: 10px; }"
        ".button.off { background: #f44336; }"
        ".button:hover { opacity: 0.9; }"
        ".nav-buttons { text-align: center; margin-top: 20px; }"
        "table { width: 100%%; border-collapse: collapse; margin-top: 10px; }"
        "th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }"
        "th { background-color: #e8f5e9; color: #2e7d32; font-weight: bold; }"
        "tr:hover { background-color: #f5f5f5; }"
        ".table-container { max-height: 300px; overflow-y: auto; border: 1px solid #ddd; border-radius: 5px; }"
        "</style></head>"
        "<body>"
        "<div class='container'>"
        "<img src='/logo' alt='Logo' class='logo'>"
        "<h1>Motor Scheduler</h1>"
        "<div class='info-section'>"
        "<strong>Device:</strong> %s<br>"
        "<strong>Location:</strong> %s<br>"
        "<strong>Current Time:</strong> %s"
        "</div>"
        "<div class='status %s'>"
        "Motor Status: <strong>%s</strong>"
        "</div>"
        "<div style='text-align: center;'>"
        "<button class='button' onclick='controlMotor(\"on\")'>Turn ON</button>"
        "<button class='button off' onclick='controlMotor(\"off\")'>Turn OFF</button>"
        "</div>",
        device_name, device_location, time_str,
        motor_state ? "on" : "off",
        motor_state ? "ON" : "OFF"
    );
    
    // Add Next Schedules section
    html_len += snprintf(html + html_len, 16384 - html_len,
        "<div class='schedules-info'>"
        "<h3>Next Schedules</h3>"
    );
    
    // Find next scheduled event
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    int current_hour = timeinfo.tm_hour;
    int current_minute = timeinfo.tm_min;
    int current_total = current_hour * 60 + current_minute;
    
    int next_schedule_minutes = 24 * 60; // Default to tomorrow
    char next_schedule_text[256] = "";
    bool found_next = false;
    
    for (int i = 0; i < schedule_count && i < MAX_SCHEDULES; i++) {
        if (schedules[i].enabled) {
            int schedule_total = schedules[i].start_hour * 60 + schedules[i].start_minute;
            if (schedule_total > current_total && schedule_total < next_schedule_minutes) {
                next_schedule_minutes = schedule_total;
                snprintf(next_schedule_text, sizeof(next_schedule_text),
                    "%s at %02d:%02d (Duration: %02d:%02d - %02d:%02d)",
                    schedules[i].name,
                    schedules[i].start_hour, schedules[i].start_minute,
                    schedules[i].start_hour, schedules[i].start_minute,
                    schedules[i].end_hour, schedules[i].end_minute
                );
                found_next = true;
            }
        }
    }
    
    if (!found_next) {
        html_len += snprintf(html + html_len, 16384 - html_len,
            "<p>No upcoming schedules today</p>"
        );
    } else {
        html_len += snprintf(html + html_len, 16384 - html_len,
            "<p>%s</p>", next_schedule_text
        );
    }
    
    html_len += snprintf(html + html_len, 16384 - html_len, "</div>");
    
    // Add Schedule History section
    html_len += snprintf(html + html_len, 16384 - html_len,
        "<div class='history-section'>"
        "<h3>Irrigation History</h3>"
    );
    
    if (history_count == 0) {
        html_len += snprintf(html + html_len, 16384 - html_len,
            "<p>No irrigation history</p>"
        );
    } else {
        html_len += snprintf(html + html_len, 16384 - html_len,
            "<div class='table-container'>"
            "<table>"
            "<tr><th>Name</th><th>Start Time</th><th>End Time</th><th>Duration</th></tr>"
        );
        
        // Display history in reverse order (newest first)
        for (int i = MAX_HISTORY - 1; i >= 0; i--) {
            if (schedule_history[i].start_time[0] != '\0') {
                html_len += snprintf(html + html_len, 16384 - html_len,
                    "<tr>"
                    "<td>%s</td>"
                    "<td>%s</td>",
                    schedule_history[i].name,
                    schedule_history[i].start_time
                );
                
                if (schedule_history[i].end_time[0] != '\0') {
                    // Calculate duration
                    struct tm start_tm = {0}, end_tm = {0};
                    
                    // Parse start time
                    sscanf(schedule_history[i].start_time, "%d-%d-%d %d:%d",
                           &start_tm.tm_year, &start_tm.tm_mon, &start_tm.tm_mday,
                           &start_tm.tm_hour, &start_tm.tm_min);
                    start_tm.tm_year -= 1900;
                    start_tm.tm_mon -= 1;
                    
                    // Parse end time
                    sscanf(schedule_history[i].end_time, "%d-%d-%d %d:%d",
                           &end_tm.tm_year, &end_tm.tm_mon, &end_tm.tm_mday,
                           &end_tm.tm_hour, &end_tm.tm_min);
                    end_tm.tm_year -= 1900;
                    end_tm.tm_mon -= 1;
                    
                    time_t start_time = mktime(&start_tm);
                    time_t end_time = mktime(&end_tm);
                    
                    if (start_time != -1 && end_time != -1 && end_time > start_time) {
                        int duration_minutes = (end_time - start_time) / 60;
                        html_len += snprintf(html + html_len, 16384 - html_len,
                            "<td>%s</td>"
                            "<td>%d mins</td>",
                            schedule_history[i].end_time,
                            duration_minutes
                        );
                    } else {
                        html_len += snprintf(html + html_len, 16384 - html_len,
                            "<td>%s</td>"
                            "<td>Invalid Duration</td>",
                            schedule_history[i].end_time
                        );
                    }
                } else if (schedule_history[i].active) {
                    html_len += snprintf(html + html_len, 16384 - html_len,
                        "<td>In Progress</td>"
                        "<td>-</td>"
                    );
                } else {
                    html_len += snprintf(html + html_len, 16384 - html_len,
                        "<td>-</td>"
                        "<td>-</td>"
                    );
                }
                
                html_len += snprintf(html + html_len, 16384 - html_len, "</tr>");
            }
        }
        
        html_len += snprintf(html + html_len, 16384 - html_len,
            "</table></div>"
        );
    }
    
    html_len += snprintf(html + html_len, 16384 - html_len, "</div>");
    
    // Add navigation buttons and closing tags
    html_len += snprintf(html + html_len, 16384 - html_len,
        "<div class='nav-buttons'>"
        "<button class='button' onclick=\"location.href='/schedules'\">Manage Schedules</button>"
        "<button class='button' onclick=\"location.href='/time'\">Time Settings</button>"
        "<button class='button' onclick=\"location.href='/settings'\">Device Settings</button>"
        "</div>"
        "</div>"
        "<script>"
        "function controlMotor(action) {"
        "  fetch('/motor/' + action)"
        "  .then(response => response.json())"
        "  .then(data => {"
        "    setTimeout(() => { location.reload(); }, 500);"
        "  });"
        "}"
        "setInterval(() => location.reload(), 30000);"
        "</script>"
        "</body></html>"
    );
    
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    free(html);
    return ESP_OK;
}

/**
 * @brief System status API endpoint
 * 
 * Returns JSON-formatted system status information including motor state,
 * device configuration, current time, schedule count, and system uptime.
 * Used by web interface for status updates and monitoring.
 * 
 * @param req HTTP request handle
 * @return ESP_OK on success, ESP_FAIL on memory allocation failure
 * 
 * @note Response Content-Type: application/json
 */

esp_err_t status_handler(httpd_req_t *req) {
    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));
    
    char *json = malloc(512);
    if (!json) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    snprintf(json, 512,
        "{"
        "\"motor_state\":\"%s\","
        "\"device_name\":\"%s\","
        "\"location\":\"%s\","
        "\"current_time\":\"%s\","
        "\"schedule_count\":%ld,"
        "\"uptime_ms\":%lu"
        "}",
        motor_state ? "ON" : "OFF",
        device_name,
        device_location,
        time_str,
        (long)schedule_count,
        (unsigned long)(esp_timer_get_time() / 1000)
    );
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    return ESP_OK;
}

/**
 * @brief Initialize and start the HTTP web server
 * 
 * Sets up the HTTP server with all necessary URI handlers for the motor controller
 * web interface. Registers handlers for dashboard, motor control, schedule management,
 * settings, and API endpoints. Configures server with appropriate settings for
 * embedded system use.
 * 
 * @return ESP_OK on successful server start, ESP_FAIL on error
 * 
 * @note Server runs on port 80 (HTTP default)
 * @note Supports up to 20 URI handlers
 * @note LRU purge enabled for memory management
 */
esp_err_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 20; 

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t motor_on = {
            .uri       = "/motor/on",
            .method    = HTTP_GET,
            .handler   = motor_on_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &motor_on);

        httpd_uri_t motor_off = {
            .uri       = "/motor/off",
            .method    = HTTP_GET,
            .handler   = motor_off_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &motor_off);

        httpd_uri_t status = {
            .uri       = "/status",
            .method    = HTTP_GET,
            .handler   = status_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &status);

        httpd_uri_t logo = {
            .uri       = "/logo",
            .method    = HTTP_GET,
            .handler   = logo_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &logo);

        httpd_uri_t time = {
            .uri       = "/time",
            .method    = HTTP_GET,
            .handler   = time_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &time);

        httpd_uri_t manual_time = {
            .uri       = "/manual_time",
            .method    = HTTP_POST,
            .handler   = manual_time_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &manual_time);

        httpd_uri_t schedules = {
            .uri       = "/schedules",
            .method    = HTTP_GET,
            .handler   = schedules_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &schedules);

        httpd_uri_t add_schedule = {
            .uri       = "/add_schedule",
            .method    = HTTP_POST,
            .handler   = add_schedule_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &add_schedule);

        httpd_uri_t delete_schedule = {
            .uri       = "/delete_schedule",
            .method    = HTTP_POST,
            .handler   = delete_schedule_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &delete_schedule);

        httpd_uri_t settings = {
            .uri       = "/settings",
            .method    = HTTP_GET,
            .handler   = settings_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &settings);
        esp_err_t err = httpd_register_uri_handler(server, &settings);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register /settings handler: %s", esp_err_to_name(err));
        }

        httpd_uri_t save_settings_uri = {
            .uri       = "/save_settings",
            .method    = HTTP_POST,
            .handler   = save_settings_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &save_settings_uri);

        return ESP_OK;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return ESP_FAIL;
}

esp_err_t logo_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/png");
    httpd_resp_send(req, (const char *)logo_data, logo_end - logo_data);
    return ESP_OK;
}
esp_err_t motor_on_handler(httpd_req_t *req) {
    motor_on();
    add_to_history("Manual Control", "ON");
    
    // Return JSON response
    char *json = malloc(64);
    if (!json) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    snprintf(json, 64, "{\"status\":\"success\",\"action\":\"on\"}");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    return ESP_OK;
}

esp_err_t motor_off_handler(httpd_req_t *req) {
    motor_off();
    add_to_history("Manual Control", "OFF");
    
    // Return JSON response
    char *json = malloc(64);
    if (!json) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    snprintf(json, 64, "{\"status\":\"success\",\"action\":\"off\"}");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    free(json);
    return ESP_OK;
}
esp_err_t time_handler(httpd_req_t *req) {
    char *html = malloc(4096);
    if (!html) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));

    int html_len = snprintf(html, 4096,
        "<!DOCTYPE html><html><head><title>Time Settings</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body { font-family: Arial; margin: 15px; background: #f5f5f5; }"
        ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }"
        ".form-group { margin: 15px 0; }"
        "label { display: block; margin-bottom: 5px; font-weight: bold; }"
        "input, select { width: 100%%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }"
        ".button { background: #4CAF50; color: white; padding: 12px 24px; border: none; border-radius: 5px; cursor: pointer; margin: 10px 5px; }"
        ".button:hover { background: #45a049; }"
        ".back { background: #6c757d; }"
        ".back:hover { background: #5a6268; }"
        "</style></head>"
        "<body><div class='container'>"
        "<h2>Time Settings</h2>"
        "<p><strong>Current Time:</strong> %s</p>"
        "<h3>Manual Time Set</h3>"
        "<form action='/manual_time' method='post'>"
        "<div class='form-group'>"
        "<label>Date:</label><input type='date' name='date' required>"
        "</div>"
        "<div class='form-group'>"
        "<label>Time:</label><input type='time' name='time' required>"
        "</div>"
        "<button type='submit' class='button'>Set Time</button>"
        "</form>"
        "<div style='margin-top:30px;'>"
        "<button class='button back' onclick=\"window.location.href='/'\">Back to Home</button>"
        "</div>"
        "</div></body></html>",
        time_str
    );

    httpd_resp_send(req, html, html_len);
    free(html);
    return ESP_OK;
}

// Add this function definition before start_webserver
esp_err_t save_settings_handler(httpd_req_t *req) {
    char content[256];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to receive settings data");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[ret] = '\0';
    
    ESP_LOGI(TAG, "Received settings data: %s", content);

    char new_name[MAX_NAME_LEN] = {0};
    char new_location[MAX_NAME_LEN] = {0};
    bool settings_changed = false;
    
    // Parse form data
    char *saveptr;
    char *token = strtok_r(content, "&", &saveptr);
    while (token) {
        if (strncmp(token, "device_name=", 12) == 0) {
            url_decode(new_name, token + 12);
            settings_changed = true;
        } else if (strncmp(token, "location=", 9) == 0) {
            url_decode(new_location, token + 9);
            settings_changed = true;
        }
        token = strtok_r(NULL, "&", &saveptr);
    }

    if (settings_changed) {
        if (new_name[0] != '\0') {
            strncpy(device_name, new_name, sizeof(device_name) - 1);
        }
        if (new_location[0] != '\0') {
            strncpy(device_location, new_location, sizeof(device_location) - 1);
        }
        save_settings();
        ESP_LOGI(TAG, "Settings updated - Name: %s, Location: %s", device_name, device_location);
    }

    // Send success response and redirect
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/settings");
    httpd_resp_send(req, NULL, 0);
    
    return ESP_OK;
}

// Add this URL decode function before your handlers
void url_decode(char *dst, const char *src) {
    char *p = dst;
    char code[3];
    while (*src) {
        if (*src == '%') {
            memcpy(code, src + 1, 2);
            code[2] = '\0';
            *p++ = (char)strtol(code, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *p++ = ' ';
            src++;
        } else {
            *p++ = *src++;
        }
    }
    *p = '\0';
}

// Fixed manual_time_handler function
esp_err_t manual_time_handler(httpd_req_t *req) {
    char content[200];
    size_t recv_size = sizeof(content) - 1;
    
    if (req->content_len < recv_size) {
        recv_size = req->content_len;
    }

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to receive POST data");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_size] = '\0';
    
    ESP_LOGI(TAG, "Received POST data: %s", content);

    char date[20] = {0};
    char time_str[20] = {0};  // Renamed from 'time' to 'time_str'
    char decoded_date[20] = {0};
    char decoded_time[20] = {0};
    
    // Parse POST data
    char *date_ptr = strstr(content, "date=");
    char *time_ptr = strstr(content, "time=");
    
    if (date_ptr && time_ptr) {
        // Extract date value
        date_ptr += 5; // Skip "date="
        char *date_end = strchr(date_ptr, '&');
        if (date_end) {
            int date_len = date_end - date_ptr;
            if (date_len < sizeof(date)) {
                strncpy(date, date_ptr, date_len);
                date[date_len] = '\0';
            }
        } else {
            // Date might be at the end
            strncpy(date, date_ptr, sizeof(date) - 1);
        }
        
        // Extract time value
        time_ptr += 5; // Skip "time="
        char *time_end = strchr(time_ptr, '&');
        if (time_end) {
            int time_len = time_end - time_ptr;
            if (time_len < sizeof(time_str)) {  // Using time_str instead of time
                strncpy(time_str, time_ptr, time_len);
                time_str[time_len] = '\0';
            }
        } else {
            // Time might be at the end
            strncpy(time_str, time_ptr, sizeof(time_str) - 1);
        }
        
        // URL decode the values
        url_decode(decoded_date, date);
        url_decode(decoded_time, time_str);
        
        ESP_LOGI(TAG, "Parsed date: %s, time: %s", decoded_date, decoded_time);
        
        // Parse date and time
        int year, month, day, hour, minute;
        if (sscanf(decoded_date, "%d-%d-%d", &year, &month, &day) == 3 &&
            sscanf(decoded_time, "%d:%d", &hour, &minute) == 2) {
            
            ESP_LOGI(TAG, "Parsed values: %04d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
            
            // Validate ranges
            if (year >= 2020 && year <= 2099 &&
                month >= 1 && month <= 12 &&
                day >= 1 && day <= 31 &&
                hour >= 0 && hour <= 23 &&
                minute >= 0 && minute <= 59) {

                struct tm tm = {0};
                tm.tm_year = year - 1900;
                tm.tm_mon = month - 1;
                tm.tm_mday = day;
                tm.tm_hour = hour;
                tm.tm_min = minute;
                tm.tm_sec = 0;
                tm.tm_isdst = -1; // Let system determine DST
                
                time_t t = mktime(&tm);
                if (t != -1) {
                    struct timeval now = { .tv_sec = t };
                    if (settimeofday(&now, NULL) == 0) {
                        // Save time to flash
                        save_time_to_flash();
                        
                        ESP_LOGI(TAG, "Time set successfully: %04d-%02d-%02d %02d:%02d",
                            year, month, day, hour, minute);
                            
                        // Verify the time was set
                        time_t verify_time;
                        struct tm verify_tm;
                        time(&verify_time);
                        localtime_r(&verify_time, &verify_tm);
                        ESP_LOGI(TAG, "Verified time: %04d-%02d-%02d %02d:%02d:%02d",
                            verify_tm.tm_year + 1900, verify_tm.tm_mon + 1, verify_tm.tm_mday,
                            verify_tm.tm_hour, verify_tm.tm_min, verify_tm.tm_sec);
                    } else {
                        ESP_LOGE(TAG, "Failed to set system time");
                    }
                } else {
                    ESP_LOGE(TAG, "Invalid time values for mktime");
                }
            } else {
                ESP_LOGE(TAG, "Time values out of range");
            }
        } else {
            ESP_LOGE(TAG, "Failed to parse date/time values");
        }
    } else {
        ESP_LOGE(TAG, "Date or time parameter not found in POST data");
    }

    // Send a response page instead of just redirecting
    char *response_html = malloc(1024);
    if (response_html) {
        char current_time_str[64];
        get_current_time_str(current_time_str, sizeof(current_time_str));
        
        snprintf(response_html, 1024,
            "<!DOCTYPE html><html><head><title>Time Updated</title>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<meta http-equiv='refresh' content='2;url=/'>"
            "<style>"
            "body { font-family: Arial; margin: 15px; background: #f5f5f5; text-align: center; }"
            ".container { max-width: 500px; margin: 50px auto; background: white; padding: 30px; border-radius: 10px; }"
            ".success { color: #4CAF50; font-size: 18px; margin: 20px 0; }"
            "</style></head>"
            "<body><div class='container'>"
            "<h2>Time Updated Successfully!</h2>"
            "<div class='success'>Current Time: %s</div>"
            "<p>Redirecting to home page...</p>"
            "</div></body></html>",
            current_time_str
        );
        
        httpd_resp_send(req, response_html, HTTPD_RESP_USE_STRLEN);
        free(response_html);
    } else {
        // Fallback redirect
        httpd_resp_set_status(req, "302 Found");
        httpd_resp_set_hdr(req, "Location", "/");
        httpd_resp_send(req, NULL, 0);
    }
    
    return ESP_OK;
}
// Additional handlers would go here...
// (schedules_handler, settings_handler, etc.)

esp_err_t schedules_handler(httpd_req_t *req) {
    char *html = malloc(8192);
    if (!html) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    int html_len = snprintf(html, 8192,
        "<!DOCTYPE html><html><head>"
        "<title>Manage Schedules</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body { font-family: Arial; margin: 15px; background: #f5f5f5; }"
        ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }"
        ".schedule { background: #e8f5e9; padding: 15px; margin: 10px 0; border-radius: 5px; }"
        ".form-group { margin: 15px 0; }"
        "label { display: block; margin-bottom: 5px; font-weight: bold; }"
        "input, select { width: 100%%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; box-sizing: border-box; }"
        ".button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }"
        ".delete { background: #f44336; }"
        ".button:hover { opacity: 0.9; }"
        "</style></head>"
        "<body><div class='container'>"
        "<h2>Manage Schedules</h2>"
        "<div id='current-schedules'>"
        "<h3>Current Schedules</h3>"
    );

    // Display existing schedules
    for (int i = 0; i < schedule_count && i < MAX_SCHEDULES; i++) {
        html_len += snprintf(html + html_len, 8192 - html_len,
            "<div class='schedule'>"
            "<h4>Schedule %d: %s</h4>"
            "<p>Start: %02d:%02d - End: %02d:%02d</p>"
            "<p>Status: %s | Repeat Daily: %s</p>"
            "<form action='/delete_schedule' method='post' style='display:inline'>"
            "<input type='hidden' name='index' value='%d'>"
            "<button type='submit' class='button delete'>Delete</button>"
            "</form>"
            "</div>",
            i + 1, schedules[i].name,
            schedules[i].start_hour, schedules[i].start_minute,
            schedules[i].end_hour, schedules[i].end_minute,
            schedules[i].enabled ? "Enabled" : "Disabled",
            schedules[i].repeat_daily ? "Yes" : "No",
            i
        );
    }

    // Add new schedule form
    html_len += snprintf(html + html_len, 8192 - html_len,
        "<h3>Add New Schedule</h3>"
        "<form action='/add_schedule' method='post' enctype='application/x-www-form-urlencoded'>"  // Added enctype
        "<div class='form-group'>"
        "<label>Name:</label>"
        "<input type='text' name='name' required maxlength='31'>"
        "</div>"
        "<div class='form-group'>"
        "<label>Start Time:</label>"
        "<input type='time' name='start_time' required>"
        "</div>"
        "<div class='form-group'>"
        "<label>End Time:</label>"
        "<input type='time' name='end_time' required>"
        "</div>"
        "<div class='form-group'>"
        "<label><input type='checkbox' name='repeat_daily' value='1'> Repeat Daily</label>"  // Added value
        "</div>"
        "<div class='form-group'>"
        "<label><input type='checkbox' name='enabled' value='1' checked> Enabled</label>"  // Added value
        "</div>"
        "<button type='submit' class='button'>Add Schedule</button>"
        "</form>"
        "<div style='margin-top:20px'>"
        "<button class='button' onclick=\"window.location.href='/'\">Back to Home</button>"
        "</div>"
        "</div></div></body></html>"
    );

    httpd_resp_send(req, html, html_len);
    free(html);
    return ESP_OK;
}

esp_err_t settings_handler(httpd_req_t *req) {
    char *html = malloc(4096);
    if (!html) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    int html_len = snprintf(html, 4096,
        "<!DOCTYPE html><html><head>"
        "<title>Device Settings</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body { font-family: Arial; margin: 15px; background: #e8f5e9; }"
        ".container { max-width: 500px; margin: 20px auto; background: white; "
        "padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }"
        ".form-group { margin: 15px 0; }"
        "label { display: block; margin-bottom: 5px; font-weight: bold; color: #2e7d32; }"
        "input { width: 100%%; padding: 10px; border: 1px solid #ccc; border-radius: 5px; "
        "box-sizing: border-box; transition: border-color 0.3s; }"
        "input:focus { border-color: #4CAF50; outline: none; }"
        ".button { background: #4CAF50; color: white; padding: 12px 24px; border: none; "
        "border-radius: 25px; cursor: pointer; margin: 10px 5px; transition: opacity 0.3s; }"
        ".button:hover { opacity: 0.9; }"
        ".back { background: #757575; }"
        "</style></head>"
        "<body><div class='container'>"
        "<h2 style='color: #2e7d32; text-align: center;'>Device Settings</h2>"
        "<form action='/save_settings' method='post' id='settingsForm'>"
        "<div class='form-group'>"
        "<label>Device Name:</label>"
        "<input type='text' name='device_name' value='%s' maxlength='31' required>"
        "</div>"
        "<div class='form-group'>"
        "<label>Location:</label>"
        "<input type='text' name='location' value='%s' maxlength='31' required>"
        "</div>"
        "<div style='text-align: center;'>"
        "<button type='submit' class='button'>Save Settings</button>"
        "<button type='button' class='button back' onclick=\"window.location.href='/'\">Back to Home</button>"
        "</div>"
        "</form>"
        "</div>"
        "<script>"
        "document.getElementById('settingsForm').addEventListener('submit', function(e) {"
        "  const deviceName = this.elements.device_name.value.trim();"
        "  const location = this.elements.location.value.trim();"
        "  if (!deviceName || !location) {"
        "    e.preventDefault();"
        "    alert('Both Device Name and Location are required!');"
        "  }"
        "});"
        "</script>"
        "</body></html>",
        device_name, device_location
    );

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, html_len);
    free(html);
    return ESP_OK;
}

esp_err_t add_schedule_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Add Schedule Handler Started");
    
    // Check schedule limit
    if (schedule_count >= MAX_SCHEDULES) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Maximum 10 schedules allowed");
        return ESP_FAIL;
    }

    // Get POST data
    char content[512];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to receive content");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[ret] = '\0';
    
    ESP_LOGI(TAG, "Received content: %s", content);

    // Parse form data
    char name[MAX_NAME_LEN] = {0};
    char start_time[10] = {0};
    char end_time[10] = {0};
    bool repeat_daily = false;
    bool valid_data = false;

    // Parse each field using strtok
    char *saveptr;
    char *token = strtok_r(content, "&", &saveptr);
    while (token) {
        if (strncmp(token, "name=", 5) == 0) {
            url_decode(name, token + 5);
        } else if (strncmp(token, "start_time=", 11) == 0) {
            url_decode(start_time, token + 11);
        } else if (strncmp(token, "end_time=", 9) == 0) {
            url_decode(end_time, token + 9);
        } else if (strncmp(token, "repeat_daily=1", 14) == 0) {
            repeat_daily = true;
        }
        token = strtok_r(NULL, "&", &saveptr);
    }

    // Validate input
    if (strlen(name) > 0 && strlen(name) < MAX_NAME_LEN &&
        strlen(start_time) >= 5 && strlen(end_time) >= 5) {
        
        int start_hour, start_minute, end_hour, end_minute;
        
        // Parse times using sscanf
        if (sscanf(start_time, "%d:%d", &start_hour, &start_minute) == 2 &&
            sscanf(end_time, "%d:%d", &end_hour, &end_minute) == 2) {
            
            // Validate time ranges
            if (start_hour >= 0 && start_hour <= 23 && 
                start_minute >= 0 && start_minute <= 59 &&
                end_hour >= 0 && end_hour <= 23 && 
                end_minute >= 0 && end_minute <= 59) {
                
                // Add new schedule
                schedule_t *new_schedule = &schedules[schedule_count];
                memset(new_schedule, 0, sizeof(schedule_t));
                
                strncpy(new_schedule->name, name, sizeof(new_schedule->name) - 1);
                new_schedule->start_hour = start_hour;
                new_schedule->start_minute = start_minute;
                new_schedule->end_hour = end_hour;
                new_schedule->end_minute = end_minute;
                new_schedule->repeat_daily = repeat_daily;
                new_schedule->enabled = true;

                schedule_count++;
                save_settings();

                ESP_LOGI(TAG, "Schedule added: %s (%02d:%02d - %02d:%02d)", 
                    name, start_hour, start_minute, end_hour, end_minute);
                ESP_LOGI(TAG, "New schedule count: %d", schedule_count);
                
                valid_data = true;
            } else {
                ESP_LOGE(TAG, "Invalid time values");
            }
        } else {
            ESP_LOGE(TAG, "Failed to parse time values");
        }
    } else {
        ESP_LOGE(TAG, "Invalid input data");
    }

    if (!valid_data) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid schedule data");
        return ESP_FAIL;
    }

    // Redirect back to schedules page
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/schedules");
    httpd_resp_send(req, NULL, 0);
    
    return ESP_OK;
}

esp_err_t delete_schedule_handler(httpd_req_t *req) {
    char content[32];
    size_t recv_size = MIN(req->content_len, sizeof(content) - 1);
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_size] = '\0';

    int index;
    if (sscanf(content, "index=%d", &index) == 1) {
        if (index >= 0 && index < schedule_count) {
            // Shift remaining schedules
            for (int i = index; i < schedule_count - 1; i++) {
                memcpy(&schedules[i], &schedules[i + 1], sizeof(schedule_t));
            }
            schedule_count--;
            save_settings();
        }
    }

    // Redirect back to schedules page
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/schedules");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** @} */ // End of Web_Server group
