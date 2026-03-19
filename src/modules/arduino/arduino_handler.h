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
        void check_timeout();
        void request_hourly();
        void request_download();

    private:

        void ping_received();
        void data_received();
        void ready_received();
        void done_received();
        void data_paused_received();
        void update_received();

        void (Arduino_Handler::*ping_pointer)() = &Arduino_Handler::ping_received;
        void (Arduino_Handler::*data_pointer)() = &Arduino_Handler::data_received;
        void (Arduino_Handler::*ready_pointer)() = &Arduino_Handler::ready_received;
        void (Arduino_Handler::*done_pointer)() = &Arduino_Handler::done_received;
        void (Arduino_Handler::*data_paused_pointer)() = &Arduino_Handler::data_paused_received;
        void (Arduino_Handler::*update_pointer)() = &Arduino_Handler::update_received;
        
        void output_data(unsigned long event_time);

        struct Arduino_Command {
            char word[33];
            int position;
            void (Arduino_Handler::*function)();
        };

        const int ARDUINO_BAUD = 57600;
        const int ARDUINO_PINS[2] = { 16, 17 };
        bool resetting = false;
        unsigned long reset_start_time = 0UL;
        bool sent_clear = false;
        bool waiting_for_resume = false;
        char incoming_message[160];
        int message_position = 0;
        int MESSAGE_MAX = 159;
        const int MESSAGE_PART_LENGTH_MAX = 19;
        const int MESSAGE_NUMBER_PARTS_MAX = 8;

        const unsigned long RESET_TIMEOUT = 6000UL;

        char message_sections[8][20];

        char data_buffer[161];
        char output_buffer[241];
        
        bool re_downloading = false;

        int last_received_event_number = -1;

        unsigned long arduino_pause_time = 0UL;
        unsigned long esp32_pause_time = 0UL;

        bool waiting_for_download = false;
        bool waiting_for_hourly = false;

        Arduino_Command command_list[12] = {
            {"PING", 1, ping_pointer},
            {"DATA", 1, data_pointer},
            {"READY", 0, ready_pointer},
            {"DONE", 0, done_pointer},
            {"DATA_PAUSED", 0, data_paused_pointer},
            {"UPDATE", 0, update_pointer}
        };

        const int NUMBER_COMMANDS = 6;
};