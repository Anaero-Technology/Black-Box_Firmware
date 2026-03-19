#pragma once
#include <RTClib.h>

class RTC_Handler {
    public:
        RTC_Handler();
        bool begin();
        void store_time();
        void get_unix_time();
        char* get_time_stamp();
        void set_time_seconds(unsigned long seconds);
        uint32_t get_seconds_since_epoch();
        void set_experiment_start(unsigned long new_start);
        unsigned long get_experiment_start();
        void store_current_hour_start();
        bool update_hour_time(); 
        bool check_elapsed(unsigned long start, unsigned long current, unsigned long duration);

    private:
        RTC_DS3231 rtc;
        unsigned long experiment_start = 0UL;
        char time_stamp_output[22];
        const unsigned long HOUR_LENGTH = 60UL * 60UL * 1000UL;
        unsigned long hour_start_time;

};