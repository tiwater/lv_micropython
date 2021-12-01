#ifndef __ESP32_TIME_H
#define __ESP32_TIME_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

bool mod_settimezone(int8_t tz);
int8_t mod_gettimezone(void);

bool mod_settimestamp(time_t ts);
time_t mod_gettimestamp(void);

#endif // __ESP32_TIME_H
