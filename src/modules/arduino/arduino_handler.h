#pragma once
#include <Arduino.h>

class Arduino_Handler {
    public:
        void begin();
        void receive_characters();
        void split_message();
        void check_message();
        void clear_incoming_characters();
    private:
        const int ARDUINO_BAUD = 57600;
        const int ARDUINO_PINS[2] = { 16, 17 };

};