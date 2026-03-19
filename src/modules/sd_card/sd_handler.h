#pragma once
#include <FS.h>
#include <SD.h>

class SD_Handler {
    public:
        SD_Handler();
        bool begin();
        bool read_file(const char* PATH);
        bool set_setup(bool col);
        bool set_name(const char *NAME);
        bool read_name();
        void set_time_file(unsigned long time);
        unsigned long get_time_file();
        void set_tip_file();
        void get_memory_data();
        void list_files();
        int set_logging_file(const char * FILE_NAME);
        void reset_logging_file();
        void reset_tip_memory_file();
        void write_tip_memory(unsigned long location);
        unsigned long get_tip_memory_location(unsigned long eventNumber);
        void download_file(const char *FILE_NAME, unsigned long start_position = 0UL);
        bool delete_file(const char* FILE_NAME);
        char* get_logging_file();
        char* get_name();
        void reset_hourly_tips_file();
        void save_hourly_tips();
        void send_hourly_tips();
        void reset_stored_download_file();
        void set_download_file(const char *FILE_NAME, unsigned long position);
        void download_prepared_file();
        bool write_data(const char* NEW_DATA);
        unsigned long get_logging_file_size();

    private:
        const char SETUP_FILE[20] = "/settings/setup.txt";
        const char NAME_FILE[19] = "/settings/name.txt";
        const char TIP_FILE[23] = "/settings/tipcount.txt";
        const char TIME_FILE[19] = "/settings/time.txt";
        const char DATA_ROOT[7] = "/data/";
        const char SETTINGS_ROOT[11] = "/settings/";
        const char TIP_MEMORY_FILE[26] = "/settings/eventmemory.txt";
        const char HOURLY_TIP_FILE[25] = "/settings/hourlytips.txt";
        
        bool sd_working = false;
        char device_name[33] = "Unnamed";
        char logging_file[38] = "";
        int download_start = 0;
        File data_file;

        char stored_download_file[40];

        char file_buffer[100];
        const int FILE_BUFFER_SIZE = 99;

        char output_file_name[33];
};