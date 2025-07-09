/**
 * @file time_util.h
 * @brief Time utility functions for formatting and display
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once
#include <stddef.h>

/** @defgroup Time_Utilities Time Utility Functions
 *  @brief Helper functions for time formatting and display
 *  @{
 */

/**
 * @brief Get current time as formatted string
 * 
 * Formats the current system time into a human-readable string
 * suitable for display in the web interface and logs.
 * 
 * @param buffer Output buffer to store the formatted time string
 * @param buffer_size Size of the output buffer in bytes
 * 
 * @note Format: "YYYY-MM-DD HH:MM:SS Day"
 *       Example: "2025-07-08 14:30:25 Monday"
 */
void get_current_time_str(char* buffer, size_t buffer_size);

/** @} */