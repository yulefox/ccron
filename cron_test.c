#include "cron.h"
#include <stdio.h>

int _main(int argc, char **argv)
{
    schedule_t s;

    char *spec[] = {
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

    int i = 0;
    time_t t;

    time(&t);

    for (; spec[i] != 0; ++i)
    {
        if (cron_parse(spec[i], s) == 0) {
            printf("%20s: %016llx %016llx %016llx %016llx %016llx %016llx\n",
                   spec[i], s[0], s[1], s[2], s[3], s[4], s[5]);
            cron_next(s, t);
            cron_prev(s, t);
        } else {
            printf("%20s: INVALID SPEC\n",
                   spec[i]);
        }
    }
    return 0;
}
