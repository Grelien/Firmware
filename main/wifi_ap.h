/**
 * @file wifi_ap.h
 * @brief WiFi Access Point setup and configuration
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once

/** @defgroup WiFi_AP WiFi Access Point Functions
 *  @brief Functions for setting up and managing WiFi Access Point mode
 *  @{
 */

/**
 * @brief Initialize WiFi in Access Point mode
 * 
 * Configures the ESP32 as a WiFi Access Point with predefined credentials,
 * allowing direct connection without requiring an existing network.
 * 
 * Configuration details:
 * - SSID: "Motor Controller"
 * - Password: "12345679"
 * - Channel: 1
 * - Max connections: 4
 * - Security: WPA/WPA2-PSK
 * 
 * @note Creates a network at IP 192.168.4.1 for web interface access
 */
void wifi_init_softap(void);

/** @} */