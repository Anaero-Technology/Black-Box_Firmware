#pragma once

struct Command {
    char word[33];
    void (*function)();
    int parameters;
};

class Serial_Handler {
    public:
        void begin();
        void receive_characters();
        void split_message();
        void check_message();
        bool wait_for_next(int timeout);

    private:
        static void info_request();
        static void type_request();
        static void start_request();
        static void stop_request();
        static void files_request();
        static void delete_request();
        static void download_request();
        static void download_from_request();
        static void get_time_request();
        static void set_time_request();
        static void set_name_request();
        static void get_hourly_request();

        const int SERIAL_BAUD = 115200;

        char incoming_message[97] = "";
        char message_part[97] = "";
        int message_position = 0;
        const int MESSAGE_MAX = 96;
        int parameters_present = 0;

        static char message_sections[3][33];

        Command test = {"hello", nullptr, 1};

        Command command_list[12] = {
            {"info", info_request, 0},
            {"type", type_request, 0},
            {"start", start_request, 1},
            {"stop", stop_request, 0},
            {"files", files_request, 0},
            {"delete", delete_request, 0},
            {"download", download_request, 0},
            {"downloadFrom", download_from_request, 0},
            {"getTime", get_time_request, 0},
            {"setTime", set_time_request, 0},
            {"setName", set_name_request, 0},
            {"getHourly", get_hourly_request, 0}
        };

        const int NUMBER_COMMANDS = 12;
};