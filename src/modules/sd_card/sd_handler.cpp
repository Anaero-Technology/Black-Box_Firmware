#include <modules/sd_card/sd_handler.h>
#include <modules/state/state_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/serial/serial_handler.h>

extern State_Handler state;
extern RTC_Handler rtc;
extern Serial_Handler serial;

SD_Handler::SD_Handler(){};

bool SD_Handler::begin() {
    bool startedSD = SD.begin();
    uint32_t cardType = CARD_NONE;
    if (startedSD) {
        cardType = SD.cardType();
    }
    if (!startedSD || cardType == CARD_NONE) {
        return false;
    }

    sd_working = true;
    if (!SD.exists(SETTINGS_ROOT)) {
        SD.mkdir(SETTINGS_ROOT);
    }
    if (!SD.exists(DATA_ROOT)) {
        SD.mkdir(DATA_ROOT);
    }

    if (read_file(SETUP_FILE)) {
        char *token;
        token = strtok(file_buffer, " ");
        //If the first character is a 1 - the experiment is running
        if (strcmp(token, "1") == 0) {
            token = strtok(NULL, " ");
            if (strlen(token) < 37) {
                strcpy(logging_file, token);
            }
            state.set_logging(true);
        } else {
            state.set_logging(false);
        }
    } else {
        if (!set_setup(false)) {
            sd_working = false;
        }
    }

    //If the file specified does not exist
    if (state.get_logging() && !SD.exists(logging_file)) {
        //Create the file and close it
        data_file = SD.open(logging_file, FILE_WRITE);
        data_file.close();
    }

    if (state.get_logging() && read_file(TIP_FILE)) {
        int event = strtoul(file_buffer, nullptr, 10);
        if (event < 1) {
            state.set_event_number(1);
            set_tip_file();
        } else {
            state.set_event_number(event);
        }
    } else {
        state.set_event_number(1);
        set_tip_file();
    }

    //If there is a file storing the name of the device
    if (!read_name()) {
        //Default name
        set_name("Unnamed");
    }

    if (state.get_logging()) {
        rtc.set_experiment_start(get_time_file());
    }

    return sd_working;
}

bool SD_Handler::read_file(const char* PATH) {
    if (!SD.exists(PATH)) {
        return false;
    }
    File file = SD.open(PATH, FILE_READ);
    int current_data_position = 0;
    while (file.available() && current_data_position < FILE_BUFFER_SIZE) {
        char c = file.read();
        file_buffer[current_data_position] = c;
        current_data_position = current_data_position + 1;
    }
    file_buffer[current_data_position] = '\0';
    file.close();
    return true;
}

bool SD_Handler::set_setup(bool col){
    /*Change the stored state of the data collection*/
    if (sd_working){
        char message[36];
        //If it is supposed to be running
        if (col){
            //Change the message to the number one
            strcpy(message, "1 ");
            strcat(message, logging_file);
        } else {
            strcpy(message, "0");
        }
        data_file = SD.open(SETUP_FILE, FILE_WRITE);
        data_file.print(message);
        data_file.close();
        return true;
    }
    return false;
}

bool SD_Handler::set_name(const char *NAME) {
    if (sd_working && strlen(NAME) < 33) {
        data_file = SD.open(NAME_FILE, FILE_WRITE);
        data_file.println(NAME);
        data_file.close();
        strcpy(device_name, NAME);
        return true;
    }
    return false;
}

bool SD_Handler::read_name() {
    if (sd_working && read_file(NAME_FILE)) {
        if (strlen(file_buffer) < 33) {
            strcpy(device_name, file_buffer);
            return true;
        }
    }
    return false;
}

void SD_Handler::set_time_file(unsigned long time) {
    if (sd_working && time > 0) {
        data_file = SD.open(TIME_FILE, FILE_WRITE);
        data_file.print(time);
        data_file.close();
    }
}

unsigned long SD_Handler::get_time_file() {
    if (sd_working && read_file(TIME_FILE)) {
        return strtol(file_buffer, nullptr, 10);
    }
    return 0;
}

void SD_Handler::set_tip_file() {
    if (sd_working){
        data_file = SD.open(TIP_FILE, FILE_WRITE);
        data_file.print(state.get_event_number());
        data_file.close();
    }
}

void SD_Handler::get_memory_data() {
/*Find how much memory is present on the board and how much has been used and output this information via serial*/
    if (sd_working){
        //Get the total number of bytes available to the file system
        double total_bytes = SD.totalBytes();
        //Get the number of bytes being used to store information
        double used_bytes = SD.usedBytes();
        //Send message
        Serial.print("memory ");
        Serial.print(total_bytes);
        Serial.print(" ");
        Serial.println(used_bytes);
    }
}

void SD_Handler::list_files() {
    /*Outputs a list of files one line at a time preceded by the keyword: file*/
    if (sd_working){
        get_memory_data();
        char sizeBuff[33];
        Serial.write("file start\n");
        //Open the root directory
        File root = SD.open(DATA_ROOT);
        //Open the first file
        File current_file = root.openNextFile();
        while (current_file){
            //If there is a file
            if (current_file and !current_file.isDirectory()){
                Serial.print("file ");
                Serial.print(current_file.name());
                Serial.print(" ");
                Serial.println(current_file.size());
            }
            //Open the next file
            current_file = root.openNextFile();
        }
        //Close the root location
        root.close();
    }
    //Send signal to indicate that it has finished (whether it was able to send files or not)
    Serial.write("done files\n");
}

int SD_Handler::set_logging_file(const char * FILE_NAME) {
    /*Set the file name to write data to*/
    char path[38] = "";
    strcpy(path, DATA_ROOT);
    strcat(path, FILE_NAME);
    if (sd_working){
        if (SD.exists(path)){
            return 2;
        } else {
            //Create the file
            File newFile = SD.open(path, FILE_WRITE);
            newFile.close();
            strcpy(logging_file, path);
            //Set file - can start
            return 0;
        }
    }
    //Cannot start, the file system is not working
    return 1;
}

void SD_Handler::reset_logging_file() {
    logging_file[0] = '\0';
}

void SD_Handler::reset_tip_memory_file() {
    data_file = SD.open(TIP_MEMORY_FILE, FILE_WRITE);
    data_file.close();
}

void SD_Handler::write_tip_memory(unsigned long location) {
    char locationBuffer[11];
    ultoa(location, locationBuffer, 10);
    int sizeLength = strlen(locationBuffer);
    if (!SD.exists(TIP_MEMORY_FILE)) {
        reset_tip_memory_file();
    }
    data_file = SD.open(TIP_MEMORY_FILE, FILE_APPEND);
    for (int i = 0; i < 10 - sizeLength; i = i + 1){
        data_file.print("0"); 
    }
    data_file.print(locationBuffer);
    data_file.print('\n');
    data_file.close();
}

unsigned long SD_Handler::get_tip_memory_location(unsigned long eventNumber) {
    unsigned long file_pos = (eventNumber - 1) * 11;
    File memory_file = SD.open(TIP_MEMORY_FILE, FILE_READ);
    bool failed = false;
    if (memory_file.size() > file_pos + 10){
        memory_file.seek(file_pos);
        char place_chars[11];
        for (int i = 0; i < 10; i = i + 1){
            if (memory_file.available()){
                place_chars[i] = memory_file.read();
            } else {
                failed = true;
            }
        }
        place_chars[10] = '\0';
        memory_file.close();

        if (!failed){
            unsigned long position = strtoul(place_chars, nullptr, 10);
            return position;
        }
    }

    return 0;
}

void SD_Handler::download_file(const char *FILE_NAME, unsigned long start_position) {
    char path[45] = "";
    strcpy(path, DATA_ROOT);
    strcat(path, FILE_NAME);
    /*Send the file as a download via serial then resume the Arduino communication*/
    if (SD.exists(path)){
        char lastChar;
        //Open the file to send
        data_file = SD.open(path, FILE_READ);
        //If it is within the file - seek that end point
        if (start_position > 0 && start_position < data_file.size()) {
            data_file.seek(start_position);
        }
        //Get the number of characters in the file
        int char_number = data_file.available();
        //Send message to indicate that the file is about to be transferred, with the file name and size
        Serial.print("download start ");
        Serial.print(FILE_NAME);
        Serial.print(" ");
        Serial.println(char_number);

        bool terminated = false;
        //If there is data to send
        if (char_number > 0){
            //keyword to indicate a line is being sent
            Serial.write("download ");
        }
        //Iterate through each character index
        for (int cha = 0; cha < char_number && !terminated; cha = cha + 1){
            //Read the character
            char next_char = data_file.read();
            //Write the character
            Serial.write(next_char);
            //If this is the end of a line
            if (next_char == '\n' && cha != char_number - 1){
                //Message received to indicate that it is ready for the next item
                terminated = serial.wait_for_next(5);
                //If the download did not time out
                if (!terminated){
                    //Keyword to indicate the next line
                    Serial.write("download ");
                }
            }
            //Store the final character that was sent
            lastChar = next_char;
        }

        //If the file did not terminate with a newline send one to terminate the command (and there was something to send)
        if (lastChar != '\n' && char_number > 0 && !terminated){
            Serial.write("\n");
        }

        //Close the file
        data_file.close();
        if (!terminated){
            //Send message to indicate that file has been downloaded fully
            Serial.write("download stop\n");
        } else {
            //Send message to indicate that something went wrong
            Serial.write("download failed\n");
        }

    } else {
        //Message to indicate that the requested file does not exist
        Serial.write("failed download nofile\n");
    }
}

bool SD_Handler::delete_file(const char* FILE_NAME) {
    char path[45] = "";
    strcpy(path, DATA_ROOT);
    strcat(path, FILE_NAME);
    if (SD.exists(path)) {
        SD.remove(path);
        return true;
    }
    return false;
}

char* SD_Handler::get_logging_file() {
    for (int char_index = 0; char_index < 33; char_index = char_index + 1) {
        output_file_name[char_index] = '\0';
    }
    for (int ch = 5; ch < 38; ch = ch + 1) {
        output_file_name[ch - 5] = logging_file[ch];
    }
    return output_file_name;
}

char* SD_Handler::get_name() {
    return device_name;
}

void SD_Handler::reset_hourly_tips_file() {
    File data_file = SD.open(HOURLY_TIP_FILE, FILE_WRITE);
    data_file.close();
}

void SD_Handler::save_hourly_tips() {
    if(!SD.exists(HOURLY_TIP_FILE)) {
        reset_hourly_tips_file();
    }
    data_file = SD.open(HOURLY_TIP_FILE, FILE_APPEND);
    for(int index = 1; index < 16; index = index + 1){
        data_file.print(state.get_tip_counter(index));
        if(index != 14){
            data_file.print(' ');
        }
    }
    data_file.println("");
    data_file.close();
}

void SD_Handler::send_hourly_tips(){
    if (sd_working) {
        Serial.write("tipfile start\n");
        if(SD.exists(HOURLY_TIP_FILE)){
            File hourlyFile = SD.open(HOURLY_TIP_FILE, FILE_READ);
            bool newLine = true;
            while(hourlyFile.available()){
                if(newLine){
                    Serial.print("tipfile ");
                    newLine = false;
                }
                char c = hourlyFile.read();
                Serial.write(c);
                if(c == '\n'){
                    newLine = true;
                }
            }
            if(!newLine){
                Serial.println("");
            }
        }
        Serial.println("tipfile done");
    } else {
        Serial.println("getHourly failed nofiles");
    }
}

void SD_Handler::reset_stored_download_file() {
    stored_download_file[0] = '\0';
}

void SD_Handler::set_download_file(const char* FILE_NAME, unsigned long position) {
    strcpy(stored_download_file, FILE_NAME);
    download_start = position;
}

void SD_Handler::download_prepared_file() {
    if (strlen(stored_download_file) > 0) {
        download_file(stored_download_file, download_start);
    }
}

bool SD_Handler::write_data(const char* NEW_DATA) {
    if (sd_working) {
        if (strlen(logging_file) > 0) {
            if (!SD.exists(logging_file)) {
                data_file = SD.open(logging_file, FILE_WRITE);
                data_file.close();
            }
            data_file = SD.open(logging_file, FILE_APPEND);
            data_file.print(NEW_DATA);
            data_file.close();
            return true;
        }
    }
    return false;
}

unsigned long SD_Handler::get_logging_file_size() {
    if (sd_working && SD.exists(logging_file)) {
        data_file = SD.open(logging_file, FILE_READ);
        unsigned long size = data_file.size();
        data_file.close();
        return size;
    }
    return 0UL;
}