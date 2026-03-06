#pragma once
#include <Arduino.h>

class Arduino_Handler {
    public:
        void begin();
        void receive_characters();
        void split_message();
        void check_message();
        void clear_incoming_characters();
        void reset_message();
        void start_reset();
    private:
        const int ARDUINO_BAUD = 57600;
        const int ARDUINO_PINS[2] = { 16, 17 };
        bool resetting = false;
        unsigned long reset_start_time = 0UL;
        bool sent_clear = false;
        bool waiting_for_resume = false;

};