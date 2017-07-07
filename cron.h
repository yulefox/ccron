#pragma once

#include <time.h>

enum time_option {
    TO_SECOND,
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

int cron_parse(const char *spec, schedule_t s);

time_t cron_prev(schedule_t s, time_t t);

time_t cron_next(schedule_t s, time_t t);