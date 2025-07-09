/**
 * @file types.h
 * @brief Type definitions and constants for the Motor Controller system
 * @author Motor Controller Team
 * @date 2025
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>

/** @defgroup Configuration Configuration Constants
 *  @brief System configuration parameters
 *  @{
 */
#define MAX_SCHEDULES 10    /**< Maximum number of schedules that can be stored */
#define MAX_HISTORY 30      /**< Maximum number of history entries to maintain */
#define MAX_NAME_LEN 32     /**< Maximum length for device and schedule names */
/** @} */

/**
 * @brief Schedule configuration structure
 * 
 * This structure defines a motor schedule with start/end times,
 * repeat settings, and identification information.
 */
typedef struct {
    bool enabled;           /**< Whether this schedule is active */
    int hour;              /**< Legacy hour field (deprecated) */
    int minute;            /**< Legacy minute field (deprecated) */
    bool action;           /**< Legacy action field (deprecated) */
    int start_hour;        /**< Schedule start hour (0-23) */
    int start_minute;      /**< Schedule start minute (0-59) */
    int end_hour;          /**< Schedule end hour (0-23) */
    int end_minute;        /**< Schedule end minute (0-59) */
    bool repeat_daily;     /**< Whether to repeat this schedule daily */
    char name[MAX_NAME_LEN]; /**< Human-readable name for this schedule */
} schedule_t;

/**
 * @brief Schedule execution history entry
 * 
 * Records when a schedule was executed, including start and end times.
 */
typedef struct {
    char name[MAX_NAME_LEN]; /**< Name of the schedule that was executed */
    char start_time[20];     /**< ISO format start time (YYYY-MM-DD HH:MM) */
    char end_time[20];       /**< ISO format end time (YYYY-MM-DD HH:MM) */
    bool active;             /**< Whether this schedule execution is currently active */
} schedule_history_t;

/**
 * @brief Saved time structure for persistent storage
 * 
 * Used to store and restore system time across reboots when
 * network time synchronization is not available.
 */
typedef struct {
    int32_t year;          /**< Year (e.g., 2025) */
    int32_t month;         /**< Month (1-12) */
    int32_t day;           /**< Day of month (1-31) */
    int32_t hour;          /**< Hour (0-23) */
    int32_t minute;        /**< Minute (0-59) */
    int32_t second;        /**< Second (0-59) */
    bool is_valid;         /**< Whether the stored time is valid */
} saved_time_t;

#endif // TYPES_H
