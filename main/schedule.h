/**
 * @file schedule.h
 * @brief Schedule management interface and function declarations
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

/** @defgroup Schedule_Management Schedule Management Functions
 *  @brief Functions for managing motor schedules and execution history
 *  @{
 */

/**
 * @brief Add an entry to the schedule execution history
 * 
 * Records when a schedule action (ON/OFF) occurs, maintaining a history
 * of motor operations. Handles both start and end events for schedules.
 * 
 * @param schedule_name Name of the schedule being executed
 * @param action Action performed ("ON" or "OFF")
 * 
 * @note The function automatically manages history array bounds and
 *       shifts older entries when the maximum is reached.
 */
void add_to_history(const char* schedule_name, const char* action);

/**
 * @brief Check and execute scheduled motor operations
 * 
 * Examines all enabled schedules and executes any that match the current
 * time. Handles both start and end times for schedules, and manages
 * one-time vs. daily repeating schedules.
 * 
 * @note This function should be called periodically (recommended every
 *       30 seconds) to ensure schedules are executed on time.
 */
void check_schedules(void);

/** @} */