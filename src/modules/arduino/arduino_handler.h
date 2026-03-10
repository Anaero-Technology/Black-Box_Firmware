#pragma once
#include <Arduino.h>
struct Arduino_Command {
    char word[33];
    int position;
    void (*function)();
};

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

        static void ping_received();
        static void data_received();
        static void ready_received();
        static void done_received();
        static void data_paused_received();
        static void update_received();

        const int ARDUINO_BAUD = 57600;
        const int ARDUINO_PINS[2] = { 16, 17 };
        bool resetting = false;
        unsigned long reset_start_time = 0UL;
        bool sent_clear = false;
        bool waiting_for_resume = false;
        char incoming_message[160];
        char message_part[160];
        int message_position = 0;
        int MESSAGE_MAX = 159;
        const int MESSAGE_PART_LENGTH_MAX = 19;
        const int MESSAGE_NUMBER_PARTS_MAX = 8;

        static char message_sections[8][20];

        Arduino_Command command_list[12] = {
            {"PING", 1, ping_received},
            {"DATA", 1, data_received},
            {"READY", 0, ready_received},
            {"DONE", 0, done_received},
            {"DATA_PAUSED", 0, data_paused_received},
            {"UPDATE", 0, update_received}
        };

        const int NUMBER_COMMANDS = 6;
};