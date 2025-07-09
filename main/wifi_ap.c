/**
 * @file wifi_ap.c
 * @brief WiFi Access Point Configuration and Management
 * @author Motor Controller Team
 * @date 2025
 * 
 * This module implements WiFi Access Point functionality for the motor controller.
 * It creates a standalone WiFi network that allows devices to connect directly
 * to the ESP32 without requiring an existing WiFi infrastructure. The AP mode
 * enables web interface access for motor control and configuration.
 * 
 * @defgroup WiFi_Management WiFi Access Point Management
 * @{
 */

#include "wifi_ap.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <string.h>

/** @brief WiFi network name (SSID) */
#define AP_SSID "Motor Controller"
/** @brief WiFi network password */
#define AP_PASSWORD "12345679"
/** @brief Maximum simultaneous connections */
#define AP_MAX_STA_CONN 4

/** @brief Logging tag for WiFi operations */
static const char *TAG = "WIFI_AP";

/**
 * @brief Initialize and start WiFi Access Point
 * 
 * Configures the ESP32 as a WiFi Access Point with predefined network settings.
 * Creates a standalone WiFi network that other devices can connect to for
 * accessing the web interface. Handles network interface initialization,
 * WiFi configuration, and AP startup.
 * 
 * @note Network details:
 *       - SSID: "Motor Controller"
 *       - Password: "12345679"  
 *       - IP Address: 192.168.4.1
 *       - Channel: 1
 *       - Max Connections: 4
 *       
 * @note This function blocks until WiFi AP is successfully started
 */
void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = 1,
            .password = AP_PASSWORD,
            .max_connection = AP_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    
    if (strlen(AP_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi Access Point started. SSID:%s password:%s", AP_SSID, AP_PASSWORD);
}

/** @} */ // End of WiFi_Management group