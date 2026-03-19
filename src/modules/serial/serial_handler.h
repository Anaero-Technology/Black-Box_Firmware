#pragma once

class Serial_Handler {
    public:
        void begin();
        void receive_characters();
        void split_message();
        void check_message();
        bool wait_for_next(int timeout);

    private:
        void info_request();
        void type_request();
        void start_request();
        void stop_request();
        void files_request();
        void delete_request();
        void download_request();
        void download_from_request();
        void get_time_request();
        void set_time_request();
        void set_name_request();
        void get_hourly_request();

        void (Serial_Handler::*info_pointer)() = &Serial_Handler::info_request;
        void (Serial_Handler::*type_pointer)() = &Serial_Handler::type_request;
        void (Serial_Handler::*start_pointer)() = &Serial_Handler::start_request;
        void (Serial_Handler::*stop_pointer)() = &Serial_Handler::stop_request;
        void (Serial_Handler::*files_pointer)() = &Serial_Handler::files_request;
        void (Serial_Handler::*delete_pointer)() = &Serial_Handler::delete_request;
        void (Serial_Handler::*download_pointer)() = &Serial_Handler::download_request;
        void (Serial_Handler::*download_from_pointer)() = &Serial_Handler::download_from_request;
        void (Serial_Handler::*get_time_pointer)() = &Serial_Handler::get_time_request;
        void (Serial_Handler::*set_time_pointer)() = &Serial_Handler::set_time_request;
        void (Serial_Handler::*set_name_pointer)() = &Serial_Handler::set_name_request;
        void (Serial_Handler::*get_hourly_pointer)() = &Serial_Handler::get_hourly_request;

        void download_file(const char* FILE_NAME, unsigned long file_position);

        const int SERIAL_BAUD = 115200;

        char incoming_message[97] = "";
        int message_position = 0;
        const int MESSAGE_MAX = 96;
        int parameters_present = 0;

        char message_sections[3][33];

        typedef void (Serial_Handler::*command_callback)();
        struct Command {
            const char word[33];
            command_callback function;
            const int parameters;
        };

        Command command_list[12] = {
            {"info", info_pointer, 0},
            {"type", type_pointer, 0},
            {"start", start_pointer, 1},
            {"stop", stop_pointer, 0},
            {"files", files_pointer, 0},
            {"delete", delete_pointer, 1},
            {"download", download_pointer, 1},
            {"downloadFrom", download_from_pointer, 2},
            {"getTime", get_time_pointer, 0},
            {"setTime", set_time_pointer, 1},
            {"setName", set_name_pointer, 1},
            {"getHourly", get_hourly_pointer, 0},
        };

        const int NUMBER_COMMANDS = 12;
};