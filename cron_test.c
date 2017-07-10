#include "cron.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    schedule_t s;

    char *spec_a[] = {
        "@annually",
        "@yearly",
        "@monthly",
        "@weekly",
        "@daily",
        "@midnight",
        "@hourly",
        "@every 2",
        "@every 3m",
        "@every 5m",
        "@every 15m",
        "@every 30m",
        "0 1,3,21,5-20/2 */5 1,10 *",
        "1 /2 */5 1,10 *",
        "0 0 * * 7",
        "/5 * * * *",
        "* * * * *",
        "*/5 * * * *",
        "0/5 * * * *",
        0,
    };

    char *spec_b[] = {
        "10 0 1 0 0",
        "60 0 1 * *",
        "2 1 0 * *",
        0,
    };

    int i = 0;
    time_t t = 1499676600, pt, nt;
    struct tm st;
    char ts[20];

    //time(&t);

    for (; spec_a[i] != 0; ++i)
    {
        if (cron_parse(spec_a[i], s) == 0) {
            printf("%20s: %016llx %016llx %016llx %016llx %016llx\n",
                   spec_a[i], s[0], s[1], s[2], s[3], s[4]);
            pt = cron_prev(s, t);
            localtime_r(&pt, &st);
            strftime(ts, 20, "%F %T", &st);
            printf("prev: %ld %s\n", pt, ts);
            nt = cron_next(s, t);
            localtime_r(&nt, &st);
            strftime(ts, 20, "%F %T", &st);
            printf("next: %ld %s\n", nt, ts);
        } else {
            printf("%20s: INVALID SPEC\n",
                   spec_a[i]);
        }
    }

    for (i = 0; spec_b[i] != 0; ++i)
    {
        time_t d = cron_duration(spec_b[i]);
        printf("%20s: %ld\n",
               spec_b[i], d);
    }
    return 0;
}
