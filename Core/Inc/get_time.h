#ifndef GET_TIME_H
#define GET_TIME_H

#include "main.h"
#include "stdint.h"

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint16_t year;
}TimeDate_t;

TimeDate_t get_time(void);

#endif /* GET_TIME_H */