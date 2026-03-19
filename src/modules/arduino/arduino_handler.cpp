#include <modules/arduino/arduino_handler.h>
#include <modules/state/state_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/sd_card/sd_handler.h>

extern State_Handler state;
extern RTC_Handler rtc;
extern SD_Handler sd_card;

void Arduino_Handler::begin() {
    Serial2.begin(ARDUINO_BAUD, SERIAL_8N1, ARDUINO_PINS[0], ARDUINO_PINS[1]);
    resetting = false;
    sent_clear = false;
    waiting_for_resume = false;
}

void Arduino_Handler::receive_characters() {
    //Repeat until there are no more characters - prioritises the arduino (may need to change to if)
    while (Serial2.available()){
        //Read the character
        char new_char = Serial2.read();
        //Serial.write(c);
        //If currently running an experiment
        if (state.get_logging()) {
            //If the character is a new line - end of message
            if (new_char == '\n'){
                //Add null terminator
                incoming_message[message_position] = '\0';
                //Convert from one message to multiple parts in 2D array and check if it matches a type
                split_message();
                check_message();
                message_position = 0;
            } else {
                //If the message is not too long
                if (message_position < MESSAGE_MAX){
                    //Only useful characters allowed
                    if (new_char > 31 && new_char < char(123)){
                        //Add the character and increment the position
                        incoming_message[message_position] = new_char;
                        message_position = message_position + 1;
                    }
                }
            }
        }
    }
}

void Arduino_Handler::split_message() {
    for (int index = 0; index < MESSAGE_NUMBER_PARTS_MAX; index = index + 1) {
        message_sections[index][MESSAGE_PART_LENGTH_MAX] = '\0';
    }

    char * message_part;
    message_part = strtok(incoming_message, " ");

    int current_part = 0;
    for (int part = 0; part < MESSAGE_NUMBER_PARTS_MAX; part = part + 1) {
        if (message_part && strlen(message_part) > 0) {
            strncpy(message_sections[current_part], message_part, MESSAGE_PART_LENGTH_MAX);
            current_part = current_part + 1;
        }
        message_part = strtok(NULL, " ");
    }
}

void Arduino_Handler::check_message() {
    bool found = false;
    for (int index = 0; index < NUMBER_COMMANDS && !found; index = index + 1) {
        if (strcmp(message_sections[command_list[index].position], command_list[index].word) == 0) {
            found = true;
            (this->*command_list[index].function)();
        }
    }
}

void Arduino_Handler::clear_incoming_characters() {
    while (Serial2.available()){
        Serial2.read();
    }
}

void Arduino_Handler::reset_message() {
    data_buffer[0] = '\0';
    output_buffer[0] = '\0';
}

void Arduino_Handler::start_reset() {
    resetting = true;
    reset_start_time = millis();
    sent_clear = false;
    waiting_for_resume = false;
}

void Arduino_Handler::request_hourly() {
    waiting_for_hourly = true;
    Serial2.println("PAUSE_DATA");
}

void Arduino_Handler::request_download() {
    waiting_for_download = true;
    Serial2.println("PAUSE_DATA");
}

void Arduino_Handler::ping_received() {
    if (resetting) {
        if (!sent_clear){
            //Send the clear message
            Serial2.write("SD_CLEAR\n");
            sent_clear = true;
        }
    } else {
        //If waiting to resume data after a download
        if (waiting_for_resume){
            //Resume the information from the arduino
            Serial2.write("RESUME_DATA\n");
            waiting_for_resume = false;
        }
    }
}

void Arduino_Handler::data_received() {
    data_buffer[0] = '\0';
    bool data_present = false;
    //Iterate through the parts - ignore the first two
    for (int part = 4; part < 7 /*partMax*/; part = part + 1){
        if (strlen(message_sections[part]) > 0) {
            if (data_present) {
                strcat(data_buffer, " ");
            }
            strcat(data_buffer, message_sections[part]);
            data_present = true;
        }
    }

    //If there was actually data in the message
    if (data_present){
        uint32_t rtc_event_time = rtc.get_seconds_since_epoch() - rtc.get_experiment_start();
        uint32_t arduino_event_number = strtol(message_sections[2], NULL, 10);
        uint32_t arduino_event_time = strtol(message_sections[3], NULL, 10);
        if (re_downloading){
        rtc_event_time = arduino_event_time;
        }

        bool asking_again = false;
        
        if (state.get_event_number() != arduino_event_number){
            if (re_downloading){
                state.set_event_number(arduino_event_number);
            } else {
                if (state.get_event_number() == -1 || arduino_event_number < state.get_event_number()){
                    state.set_event_number(arduino_event_number);
                } else {
                //Store the end point
                last_received_event_number = arduino_event_number;
                //Request data dump from previous tip (give last one recieved)
                Serial2.write("DUMP_DATA_FROM ");
                Serial2.print(state.get_event_number());
                Serial2.write("\n");
                
                Serial.write("Re-Requesting tips from ");
                Serial.print(state.get_event_number());
                Serial.write("\n");

                asking_again = true;
                re_downloading = true;
                }
            }
        }

        if (!asking_again){
        //Write the line to the file
        output_data(rtc_event_time);
        }
    }
}

void Arduino_Handler::ready_received() {
    //If needing to reset - for the start sequence
    if (resetting){
        //Send the confirmation message
        Serial2.write("CONFIRM\n");
        Serial.write("Sent clear confirmation\n");
    }
}

void Arduino_Handler::done_received() {
    if (resetting) {
      //Resetting is done
      resetting = false;
      sent_clear = false;
      //Start the experiment
      state.set_logging(true);
      rtc.store_current_hour_start();
      Serial.write("done start\n");
      Serial2.write("LOGGING_ON\n");
    } else {

        if (re_downloading && (state.get_event_number() > last_received_event_number && last_received_event_number > 0)){
            re_downloading = false;
            Serial.write("Stopped re-requesting\n");
        }
    }
}

void Arduino_Handler::data_paused_received() {
    //If a download has been scheduled
    if (waiting_for_download || waiting_for_hourly){
        //Get the pause time for both devices
        arduino_pause_time = strtoul(message_sections[1], NULL, 10);
        esp32_pause_time = rtc.get_seconds_since_epoch();
        if (waiting_for_download){
            //Perform the download
            sd_card.download_prepared_file();
        }else{
            sd_card.send_hourly_tips();
        }
    }
    //Reset the download setup
    waiting_for_download = false;
    waiting_for_hourly = false;
    sd_card.reset_stored_download_file();
    //Resume the arduino
    waiting_for_resume = true;
}

void Arduino_Handler::update_received() {
    //Serial.write("Added restored data\n");
    uint32_t event_time = esp32_pause_time + (strtoul(message_sections[2], NULL, 10) - arduino_pause_time) - rtc.get_experiment_start();
    //Iterate through the parts - ignore the first two
    data_buffer[0] = '\0';
    bool data_present = false;
    for (int part = 3; part < 6 /*partMax*/; part = part + 1){
        if (strlen(message_sections[part]) > 0) {
            if (data_present) {
                strcat(data_buffer, " ");
            }
            strcat(data_buffer, message_sections[part]);
            data_present = true;
        }
    }

    //If there was actually data in the message
    if (data_present){
        uint32_t arduino_event_number = atoi(message_sections[1]);
        if (state.get_event_number() != arduino_event_number){
            state.set_event_number(arduino_event_number);
        }
        //Write the line to the file
        output_data(event_time);
    }
}

void Arduino_Handler::check_timeout() {
  if (resetting){
    unsigned long current_time = millis();
    unsigned long elapsed = current_time - reset_start_time;
    if (current_time < reset_start_time){
      elapsed = (ULONG_MAX - reset_start_time) + current_time;
    }
    if (elapsed >= RESET_TIMEOUT){
      Serial.write("failed start noarduino\n");
      state.set_logging(false);
      resetting = false;
      sent_clear = false;
      sd_card.reset_logging_file();
      sd_card.get_memory_data();
      sd_card.list_files();
    }
  }
}

void Arduino_Handler::output_data(unsigned long event_time) {
  /*Store the buffer and reset it*/
    if (state.get_logging()){
        output_buffer[0] = '\0';

        int event_number = state.get_event_number();

        char time_stamp_buffer[24];
        strcpy(time_stamp_buffer, rtc.get_time_stamp());

        char event_number_buffer[11];
        itoa(event_number, event_number_buffer, 10);

        char event_time_buffer[11];
        ultoa(event_time, event_time_buffer, 10);

        strcat(output_buffer, event_number_buffer);
        strcat(output_buffer, " ");
        strcat(output_buffer, time_stamp_buffer);
        strcat(output_buffer, " ");
        strcat(output_buffer, event_time_buffer);
        strcat(output_buffer, " ");
        strcat(output_buffer, data_buffer);

        char channel[4];
        int channelPos = 0;
        bool done = false;
        for (int cha = 0; cha < 241 && !done; cha = cha + 1){
            char c = data_buffer[cha];
            if(c != ' '){
                if(channelPos < 3){
                    channel[channelPos] = c;
                    channelPos = channelPos + 1;
                }
            }else{
                done = true;
            }
        }
        channel[channelPos] = '\0';
        int channel_number = atoi(channel);
        state.increment_tip_counter(channel_number);

        unsigned long file_size = sd_card.get_logging_file_size();

        //Open the file for append
        if (sd_card.write_data(output_buffer)) {
            Serial.print("tip ");
            Serial.println(output_buffer);
            state.increment_event_number();
            sd_card.set_tip_file();
            sd_card.write_tip_memory(file_size);
        } else {
            Serial.println("File System Has Failed (Attempting to write line)");
        }
    }
    //Reset the buffer position
    data_buffer[0] = '\0';
    output_buffer[0] = '\0';
}