#include "cron.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int BOUND_MIN = 0;
static const int BOUND_MAX = 1;
static const int RANGE_MAX = 60;
static const long long STAR_BIT = 1L << 63;

static int bounds[TO_MAX][2] = {
    {0, 59}, // seconds
    {0, 59}, // minutes
    {0, 23}, // hours
    {1, 31}, // dom
    {1, 12}, // months
    {0, 6},  // dow
};

static long long get_bits(int min, int max, int step) 
{
    long long bits = 0;

    if (step == 1) {
        return ~(-1L << (max + 1)) & (-1L << min);
    }

    int i = min;
    for (; i <= max; i += step) {
        bits |= 1L << i;
    }
    return bits;
}

static long long all_bits(int bound[2]) 
{
    return get_bits(bound[BOUND_MIN], bound[BOUND_MAX], 1) | STAR_BIT;
}

static int every(char *spec, schedule_t s)
{
    int step = 0;
    sscanf(spec, "@every %dm", &step);
    if (step <= 0 || step > 30) {
        return -1;
    }
    s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
    s[TO_MINUTE] = get_bits(bounds[TO_MINUTE][BOUND_MIN], bounds[TO_MINUTE][BOUND_MAX], step);
    s[TO_HOUR] = all_bits(bounds[TO_HOUR]);
    s[TO_DOM] = all_bits(bounds[TO_DOM]);
    s[TO_MONTH] = all_bits(bounds[TO_MONTH]);
    s[TO_DOW] = all_bits(bounds[TO_DOW]);
    return 0;
}

static int day_matched(schedule_t s, const struct tm *st)
{
    int dom_matched = ((1L << st->tm_mday) & s[TO_DOM]) > 0;
    int dow_matched = ((1L << st->tm_wday) & s[TO_DOW]) > 0;

    if ((s[TO_DOM] & STAR_BIT) || (s[TO_DOW] & STAR_BIT)) {
        return (dom_matched && dow_matched);
    }
    return (dom_matched || dow_matched);
}

static int parse_descriptor(char *spec, schedule_t s)
{
    if (strcmp(spec, "@yearly") == 0 || strcmp(spec, "@annually") == 0) {
        s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
        s[TO_MINUTE] = 1 << bounds[TO_MINUTE][BOUND_MIN];
        s[TO_HOUR]   = 1 << bounds[TO_HOUR][BOUND_MIN];
        s[TO_DOM]    = 1 << bounds[TO_DOM][BOUND_MIN];
        s[TO_MONTH]  = 1 << bounds[TO_MONTH][BOUND_MIN];
        s[TO_DOW]    = all_bits(bounds[TO_DOW]);
        return 0;
    }
    if (strcmp(spec, "@monthly") == 0) {
        s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
        s[TO_MINUTE] = 1 << bounds[TO_MINUTE][BOUND_MIN];
        s[TO_HOUR]   = 1 << bounds[TO_HOUR][BOUND_MIN];
        s[TO_DOM]    = 1 << bounds[TO_DOM][BOUND_MIN];
        s[TO_MONTH]  = all_bits(bounds[TO_MONTH]);
        s[TO_DOW]    = all_bits(bounds[TO_DOW]);
        return 0;
    }
    if (strcmp(spec, "@weekly") == 0) {
        s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
        s[TO_MINUTE] = 1 << bounds[TO_MINUTE][BOUND_MIN];
        s[TO_HOUR]   = 1 << bounds[TO_HOUR][BOUND_MIN];
        s[TO_DOM]    = all_bits(bounds[TO_DOM]);
        s[TO_MONTH]  = all_bits(bounds[TO_MONTH]);
        s[TO_DOW]    = 1 << bounds[TO_DOW][BOUND_MIN];
        return 0;
    }
    if (strcmp(spec, "@daily") == 0 || strcmp(spec, "@midnight") == 0) {
        s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
        s[TO_MINUTE] = 1 << bounds[TO_MINUTE][BOUND_MIN];
        s[TO_HOUR]   = 1 << bounds[TO_HOUR][BOUND_MIN];
        s[TO_DOM]    = all_bits(bounds[TO_DOM]);
        s[TO_MONTH]  = all_bits(bounds[TO_MONTH]);
        s[TO_DOW]    = all_bits(bounds[TO_DOW]);
        return 0;
    }
    if (strcmp(spec, "@hourly") == 0) {
        s[TO_SECOND] = 1 << bounds[TO_SECOND][BOUND_MIN];
        s[TO_MINUTE] = 1 << bounds[TO_MINUTE][BOUND_MIN];
        s[TO_HOUR]   = all_bits(bounds[TO_HOUR]);
        s[TO_DOM]    = all_bits(bounds[TO_DOM]);
        s[TO_MONTH]  = all_bits(bounds[TO_MONTH]);
        s[TO_DOW]    = all_bits(bounds[TO_DOW]);
        return 0;
    }
    return every(spec, s);
}

static int str_split(char *src, const char *delim, char **dst)
{
    char *next;
    int count = 0;

    next = strtok(src, delim);
    while (next != NULL) {
        *dst++ = next;
        ++count;
        next = strtok(NULL, delim);
    }
    return count;
}

static long long parse_range(char *range, int bound[2])
{
    long long bits = 0;
    int start = 0, end = 0, step = 1;
    char *range_and_step[2];
    char *low_and_high[2];
    int num = 0;

    num = str_split(range, "/", range_and_step);
    if (num > 1) {
        step = atoi(range_and_step[1]);
    }
    num = str_split(range_and_step[0], "-", low_and_high);
    if (strcmp(low_and_high[0], "*") == 0) {
        start = bound[0];
        end = bound[1];
        bits |= STAR_BIT;
    } else {
        start = atoi(low_and_high[0]);
        if (num > 1)
        { // single digit
            end = atoi(low_and_high[1]);
        }
        else if (step == 1)
        {
            end = start;
        }
        else
        {
            end = bound[1];
        }
        if (start < bound[0])
        {
            start = bound[0];
        }
        if (end > bound[1])
        {
            start = bound[1];
        }
        if (start > end)
        {
            end = start;
        }
    }
    bits |= get_bits(start, end, step);
    return bits;
}

static long long parse_field(char *field, int bound[2])
{
    long long bits = 0;

    char *ranges[RANGE_MAX];
    int count = str_split(field, ",", ranges);
    int i = 0;
    for (; i < count; ++i) {
        bits |= parse_range(ranges[i], bound);
    }
    return bits;
}

time_t cron_duration(const char *spec)
{
    if (spec == NULL || strlen(spec) == 0) {
        printf("empty spec string");
        return -1;
    }

    if (strlen(spec) >= 1024)
    {
        printf("ovenlength spec string");
        return -1;
    }


    int count = str_split(spec_dup, " ", fields);
    if (count != TO_MAX)
    {
        printf("invalid spec string");
        return -1;
    }

    int i = 0;
    for (; i < TO_MAX; ++i)
    {
        s[i] = parse_field(fields[i], bounds[i]);
    }
}

int cron_parse(const char *spec, schedule_t s)
{
    if (spec == NULL || strlen(spec) == 0) {
        printf("empty spec string");
        return -1;
    }

    if (strlen(spec) >= 1024)
    {
        printf("ovenlength spec string");
        return -1;
    }

    char spec_dup[1024];
    strcpy(spec_dup, spec);

    if (spec_dup[0] == '@') {
        return parse_descriptor(spec_dup, s);
    }

    char *fields[TO_MAX];

    int count = str_split(spec_dup, " ", fields);
    if (count != TO_MAX)
    {
        printf("invalid spec string");
        return -1;
    }

    int i = 0;
    for (; i < TO_MAX; ++i)
    {
        s[i] = parse_field(fields[i], bounds[i]);
    }
    return 0;
}

time_t cron_prev(schedule_t s, time_t t)
{
    int subtracted = 0;
    struct tm st;
    char ts[20];

    localtime_r(&t, &st);

    int year_limit = st.tm_year - 2;

WRAP:
    strftime(ts, 20, "%F %T", &st);

    // year
    if (st.tm_year < year_limit) {
        return 0;
    }

    // month
    while (((1L << (st.tm_mon + 1)) & s[TO_MONTH]) == 0) {
        if (subtracted == 0) {
            ++subtracted;
        }
        st.tm_mday = 1;
        st.tm_hour = 0;
        st.tm_min = 0;
        st.tm_sec = 0;
        t = mktime(&st);
        --t;
        localtime_r(&t, &st);

        if (st.tm_mon == (bounds[TO_MONTH][BOUND_MAX] - 1))
        {
            goto WRAP;
        }
        if (st.tm_mon == -1)
        {
            st.tm_mon = bounds[TO_MONTH][BOUND_MAX] - 1;
            --(st.tm_year);
            t = mktime(&st);
            localtime_r(&t, &st);
            goto WRAP;
        }
    }

    // day of month/week
    while (!day_matched(s, &st))
    {
        int mon = st.tm_mon;
        if (subtracted == 0)
        {
            ++subtracted;
            st.tm_hour = 0;
            st.tm_min = 0;
            st.tm_sec = 0;
            t = mktime(&st);
            --t;
        }
        else
        {
            t -= 86400;
        }
        localtime_r(&t, &st);

        if (st.tm_mon != mon)
        {
            goto WRAP;
        }
    }

    // hour
    while (((1L << st.tm_hour) & s[TO_HOUR]) == 0)
    {
        if (subtracted == 0)
        {
            ++subtracted;
            st.tm_min = 0;
            st.tm_sec = 0;
            t = mktime(&st);
            --t;
        }
        else
        {
            t -= 3600;
        }
        localtime_r(&t, &st);

        if (st.tm_hour == bounds[TO_HOUR][BOUND_MAX])
        {
            goto WRAP;
        }
    }

    // minute
    while (((1L << st.tm_min) & s[TO_MINUTE]) == 0)
    {
        if (subtracted == 0)
        {
            ++subtracted;
            st.tm_sec = 0;
            t = mktime(&st);
            --t;
        }
        else
        {
            t -= 60;
        }
        localtime_r(&t, &st);

        if (st.tm_min == bounds[TO_MINUTE][BOUND_MAX])
        {
            goto WRAP;
        }
    }

    // second
    while (((1L << st.tm_sec) & s[TO_SECOND]) == 0)
    {
        if (subtracted == 0)
        {
            ++subtracted;
        }
        --t;
        localtime_r(&t, &st);

        if (st.tm_sec == bounds[TO_SECOND][BOUND_MAX])
        {
            goto WRAP;
        }
    }

    strftime(ts, 20, "%F %T", &st);
    printf("prev: %s\n", ts);
    return t;
}

time_t cron_next(schedule_t s, time_t t)
{
    int added = 0;
    struct tm st;
    char ts[20];

    localtime_r(&t, &st);

    int year_limit = st.tm_year + 2;

WRAP:
    strftime(ts, 20, "%F %T", &st);

    // year
    if (st.tm_year > year_limit) {
        return 0;
    }

    // month
    while (((1L << (st.tm_mon + 1)) & s[TO_MONTH]) == 0) {
        if (added == 0) {
            ++added;
            st.tm_mday = 1;
            st.tm_hour = 0;
            st.tm_min = 0;
            st.tm_sec = 0;
        }
        ++st.tm_mon;
        t = mktime(&st);
        localtime_r(&t, &st);

        if (st.tm_mon == bounds[TO_MONTH][BOUND_MAX]) {
            st.tm_mon = bounds[TO_MONTH][BOUND_MIN] - 1;
            ++(st.tm_year);
            t = mktime(&st);
            localtime_r(&t, &st);
            goto WRAP;
        }
    }

    // day of month/week
    while (!day_matched(s, &st)) {
        if (added == 0) {
            ++added;
            st.tm_hour = 0;
            st.tm_min = 0;
            st.tm_sec = 0;
            t = mktime(&st);
        }
        t += 86400;
        localtime_r(&t, &st);

        if (st.tm_mday == bounds[TO_DOM][BOUND_MIN]) {
            goto WRAP;
        }
    }

    // hour
    while (((1L << st.tm_hour) & s[TO_HOUR]) == 0) {
        if (added == 0) {
            ++added;
            st.tm_min = 0;
            st.tm_sec = 0;
            t = mktime(&st);
        }
        t += 3600;
        localtime_r(&t, &st);

        if (st.tm_hour == bounds[TO_HOUR][BOUND_MIN]) {
            goto WRAP;
        }
    }

    // minute
    while (((1L << st.tm_min) & s[TO_MINUTE]) == 0) {
        if (added == 0) {
            ++added;
            st.tm_sec = 0;
            t = mktime(&st);
        }
        t += 60;
        localtime_r(&t, &st);

        if (st.tm_min == bounds[TO_MINUTE][BOUND_MIN]) {
            goto WRAP;
        }
    }

    // second
    while (((1L << st.tm_sec) & s[TO_SECOND]) == 0) {
        if (added == 0) {
            ++added;
        }
        ++t;
        localtime_r(&t, &st);

        if (st.tm_sec == bounds[TO_SECOND][BOUND_MIN]) {
            goto WRAP;
        }
    }

    strftime(ts, 20, "%F %T", &st);
    printf("next: %s\n", ts);
    return t;
}
