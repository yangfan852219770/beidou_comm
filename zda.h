#ifndef ZDA_H
#define ZDA_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct minmea_date {
    int day;
    int month;
    int year;
} minmea_date;

typedef struct minmea_time {
    int hours;
    int minutes;
    int seconds;
    int microseconds;
}minmea_time;

typedef struct minmea_sentence_zda {
    minmea_time time;
    minmea_date date;
    int hour_offset;
    int minute_offset;
}minmea_sentence_zda;

bool parse_zda_time(minmea_sentence_zda *frame, const uint8_t *buf);

#endif