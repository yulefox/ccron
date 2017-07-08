#include "cron.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    schedule_t s;

    char *spec_a[] = {
        "@annually",
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
        "0 0 1,3,21,5-20/2 */5 1,10 *",
        "9 1 /2 */5 1,10 *",
        "9 1 3/2 */5 * 3",
        "@yearly",
        0,
    };

    char *spec_b[] = {
        "0 0 10",
        "1 0 60",
        "0 1 2",
        0,
    };

    int i = 0;
    time_t t;

    time(&t);

    for (; spec_a[i] != 0; ++i)
    {
        if (cron_parse(spec_a[i], s) == 0) {
            printf("%20s: %016llx %016llx %016llx %016llx %016llx %016llx\n",
                   spec_a[i], s[0], s[1], s[2], s[3], s[4], s[5]);
            cron_next(s, t);
            cron_prev(s, t);
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
