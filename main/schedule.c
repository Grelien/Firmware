/**
 * @file schedule.c
 * @brief Schedule Management Implementation
 * @author Motor Controller Project
 * @date 2024
 * 
 * This module implements the schedule management system for motor control.
 * It handles schedule execution, history tracking, and schedule validation.
 * The system supports up to MAX_SCHEDULES concurrent schedules with daily
 * repeat functionality and maintains a history of schedule executions.
 */

#include "global.h"
#include "schedule.h"
#include "motor_control.h"
#include "storage.h"

static const char *TAG = "SCHEDULE"; ///< Logging tag for schedule module

/**
 * @brief Add a schedule event to history
 * 
 * Records schedule events (start/stop) in the history array with timestamps.
 * Manages history array overflow by shifting older entries. For "ON" events,
 * prevents duplicate active entries for the same schedule. For "OFF" events,
 * updates the end time of matching active entries.
 * 
 * @param schedule_name Name of the schedule that triggered the event
 * @param action Action performed ("ON" or "OFF")
 * 
 * @note History is automatically saved to flash storage after each addition
 * @see save_history_to_flash()
 */
void add_to_history(const char* schedule_name, const char* action) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", &timeinfo);
    
    if (strcmp(action, "ON") == 0) {
        int index = -1;
        for (int i = 0; i < MAX_HISTORY; i++) {
            if (!schedule_history[i].active && schedule_history[i].start_time[0] == '\0') {
                index = i;
                break;
            }
        }
        if (index == -1) {
            // Shift history array
            for (int i = 0; i < MAX_HISTORY - 1; i++) {
                memcpy(&schedule_history[i], &schedule_history[i + 1], sizeof(schedule_history_t));
            }
            index = MAX_HISTORY - 1;
        }
        
        bool already_active = false;
        for (int i = 0; i < MAX_HISTORY; i++) {
            if (schedule_history[i].active && strcmp(schedule_history[i].name, schedule_name) == 0) {
                already_active = true;
                break;
            }
        }
        
        if (!already_active) {
            strncpy(schedule_history[index].name, schedule_name, sizeof(schedule_history[index].name) - 1);
            strncpy(schedule_history[index].start_time, time_str, sizeof(schedule_history[index].start_time) - 1);
            schedule_history[index].end_time[0] = '\0';
            schedule_history[index].active = true;
            if (history_count < MAX_HISTORY) history_count++;
        }
    } else if (strcmp(action, "OFF") == 0) {
        for (int i = 0; i < MAX_HISTORY; i++) {
            if (schedule_history[i].active && strcmp(schedule_history[i].name, schedule_name) == 0) {
                strncpy(schedule_history[i].end_time, time_str, sizeof(schedule_history[i].end_time) - 1);
                schedule_history[i].active = false;
                break;
            }
        }
    }
    save_history_to_flash();
}

/**
 * @brief Check and execute scheduled motor operations
 * 
 * Periodically checks all enabled schedules against current time and executes
 * motor operations when scheduled times are reached. Handles missed events by
 * checking all minutes since last execution. Supports one-time and daily
 * repeat schedules.
 * 
 * The function maintains state to prevent duplicate executions and handles
 * schedule lifecycle (disabling one-time schedules after completion).
 * 
 * @note Should be called regularly (e.g., every minute) from main loop
 * @note First call initializes the checking mechanism without executing schedules
 * @see motor_on(), motor_off(), add_to_history()
 */
void check_schedules(void) {
    static int last_checked_minute = -1;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    int current_minute_of_day = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    // If first run, just set last_checked_minute
    if (last_checked_minute == -1) {
        last_checked_minute = current_minute_of_day;
        return;
    }

    // Check all minutes since last check (handles missed events)
    int minute = (last_checked_minute + 1) % (24 * 60);
    while (minute != (current_minute_of_day + 1) % (24 * 60)) {
        int hour = minute / 60;
        int min = minute % 60;
        for (int i = 0; i < schedule_count && i < MAX_SCHEDULES; i++) {
            if (!schedules[i].enabled) continue;
            if (schedules[i].start_hour == hour && schedules[i].start_minute == min) {
                motor_on();
                add_to_history(schedules[i].name, "ON");
                ESP_LOGI(TAG, "Schedule started: %s", schedules[i].name);
            }
            if (schedules[i].end_hour == hour && schedules[i].end_minute == min) {
                motor_off();
                add_to_history(schedules[i].name, "OFF");
                ESP_LOGI(TAG, "Schedule ended: %s", schedules[i].name);
                if (!schedules[i].repeat_daily) {
                    schedules[i].enabled = false;
                    save_settings();
                }
            }
        }
        minute = (minute + 1) % (24 * 60);
    }
    last_checked_minute = current_minute_of_day;
}