/**
 * @file storage.h
 * @brief Non-volatile storage interface for persistent data management
 * @author Motor Controller Team
 * @date 2025
 */

#pragma once

#include <sys/time.h>
#include "time_util.h"
#include "types.h"

/** @defgroup Storage_Management Storage Management Functions
 *  @brief Functions for persistent storage of settings, schedules, and history
 *  @{
 */

/**
 * @brief Initialize the Non-Volatile Storage (NVS) system
 * 
 * Sets up the NVS partition and opens a handle for storing motor controller
 * data. Handles partition initialization and error recovery.
 * 
 * @note Must be called before any other storage functions
 */
void init_nvs(void);

/**
 * @brief Load device settings and schedules from NVS
 * 
 * Restores device configuration including:
 * - Device name and location
 * - Schedule count and all schedule data
 * - System preferences
 * 
 * Sets default values if no stored data is found.
 */
void load_settings(void);

/**
 * @brief Save current device settings and schedules to NVS
 * 
 * Persists all current configuration data:
 * - Device name and location
 * - All configured schedules
 * - Schedule count
 * 
 * Automatically commits changes to flash storage.
 */
void save_settings(void);

/**
 * @brief Save schedule execution history to NVS
 * 
 * Persists the current schedule execution history array,
 * allowing history to survive system reboots.
 */
void save_history_to_flash(void);

/**
 * @brief Load schedule execution history from NVS
 * 
 * Restores previously saved schedule execution history,
 * maintaining continuity across system reboots.
 */
void load_history_from_flash(void);

/**
 * @brief Save current system time to NVS
 * 
 * Stores the current time for restoration after reboot when
 * network time synchronization may not be immediately available.
 */
void save_time_to_flash(void);

/**
 * @brief Restore system time from NVS
 * 
 * Attempts to restore previously saved time if network
 * synchronization is not available. Used as fallback time source.
 */
void restore_time_from_flash(void);

/** @} */