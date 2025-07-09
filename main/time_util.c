/**
 * @file time_util.c
 * @brief Time utility functions implementation
 * @author Motor Controller Team
 * @date 2025
 * 
 * This module provides utility functions for time formatting and display.
 * It handles time conversion, formatting for web interface display, and
 * provides standardized time string representations throughout the system.
 * 
 * @defgroup Time_Utilities Time Utility Functions
 * @{
 */

#include "time_util.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Format current system time for display
 * 
 * Retrieves the current system time and formats it into a human-readable
 * string suitable for display in the web interface and system logs.
 * Uses strftime() to create a standardized format.
 * 
 * @param buffer Output buffer to store the formatted time string
 * @param buffer_size Size of the output buffer in bytes
 * 
 * @note Format: "YYYY-MM-DD HH:MM:SS Day"
 *       Example: "2025-07-08 14:30:25 Monday"
 * @note If system time is not set, may show epoch time (1970-01-01)
 */
void get_current_time_str(char* buffer, size_t buffer_size) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S %A", &timeinfo);
}

/** @} */ // End of Time_Utilities group