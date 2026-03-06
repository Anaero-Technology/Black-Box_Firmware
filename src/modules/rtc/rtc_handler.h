#pragma once
#include <RTClib.h>

class RTC_Handler {
    public:
        RTC_Handler();
        bool begin();
        void store_time();
        void get_unix_time();
        void get_time_stamp();
        void set_time_seconds(unsigned long seconds);
        uint32_t get_seconds_since_epoch();
        void set_experiment_start(unsigned long new_start);
        unsigned long get_experiment_start();

    private:
        RTC_DS3231 rtc;
        unsigned long experiment_start = 0UL;

};