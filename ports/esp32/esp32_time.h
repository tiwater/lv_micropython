#ifndef __ESP32_TIME_H
#define __ESP32_TIME_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

bool time_setzone(int8_t tz);
int8_t time_getzone(void);

bool time_setstamp(time_t ts);
time_t time_getstamp(void);

static inline time_t time_offset_sec_by_tz(void)
{
    return time_getzone() * (60 * 60);
}

#endif // __ESP32_TIME_H
