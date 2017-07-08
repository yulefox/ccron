#pragma once

#include <time.h>

enum time_option {
    TO_MINUTE,
    TO_HOUR,
    TO_DOM, // day of month
    TO_MONTH,
    TO_DOW, // day of week
    TO_MAX
};

typedef long long schedule_t[TO_MAX]; 

struct entry_t {
    schedule_t schedule;
    time_t prev;
    time_t next;
};

///
/// Calculate duaration of spec.
/// @param[in] spec format: d h m
/// @return duraion in second, 0 for error.
///
time_t cron_duration(const char *spec);

///
/// Parse spec into schedule.
/// @param[in] spec format: @xxx, or a b c d e f
/// @param[out] s schedule
/// @return 0 for okey, -1 for error.
///
int cron_parse(const char *spec, schedule_t s);

///
/// Get previous time stamp before some time of given schedule.
/// @param[in] s schedule
/// @param[in] t reference time stamp.
/// @return time stamp, 0 for error.
///
time_t cron_prev(schedule_t s, time_t t);

///
/// Get next time stamp after some time of given schedule.
/// @param[in] s schedule
/// @param[in] t reference time stamp.
/// @return time stamp, 0 for error.
///
time_t cron_next(schedule_t s, time_t t);
