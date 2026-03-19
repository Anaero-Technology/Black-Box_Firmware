#include <modules/serial/serial_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/arduino/arduino_handler.h>
#include <modules/network/network_handler.h>
#include <modules/state/state_handler.h>
#include <modules/sd_card/sd_handler.h>

extern RTC_Handler rtc;
extern Arduino_Handler arduino;
extern Network_Handler network;
extern State_Handler state;
extern SD_Handler sd_card;

void Serial_Handler::begin() {
    Serial.begin(SERIAL_BAUD);
}

void Serial_Handler::receive_characters() {
    //If there is data to be read from the laptop end
    while (Serial.available()){
        char new_char = Serial.read();
        //If the end of the current message has not been reached
        if (new_char == '\n'){
            incoming_message[message_position] = '\0';
            split_message();
            check_message();
            message_position = 0;
        } else {
            //If the message is not too long
            if (message_position < MESSAGE_MAX){
                //Add the character to the message
                incoming_message[message_position] = new_char;
                message_position = message_position + 1;
                if (message_position > 96){
                    message_position = 96;
                }
            }
        }
    }
}

void Serial_Handler::split_message() {
    message_sections[0][32] = '\0';
    message_sections[1][32] = '\0';
    message_sections[2][32] = '\0';

    char * message_part;
    message_part = strtok(incoming_message, " ");
    if (strlen(message_part) > 0) {
        strncpy(message_sections[0], message_part, 31);
    } else {
        message_sections[0][0] = '\0';
    }
    //ERROR HERE!
    message_part = strtok(NULL, " ");
    if (message_part && strlen(message_part) > 0) {
        strncpy(message_sections[1], message_part, 31);
    } else {
        message_sections[1][0] = '\0';
    }
    //TO HERE
    message_part = strtok(NULL, " ");
    if (message_part && strlen(message_part) > 0) {
        strncpy(message_sections[2], message_part, 31);
    } else {
        message_sections[2][0] = '\0';
    }

    parameters_present = 0;
    if (strlen(message_sections[1]) > 0) {
        parameters_present = 1;
        if (strlen(message_sections[2]) > 0) {
            parameters_present = 2;
        }
    }
}

void Serial_Handler::check_message() {
    bool found = false;
    for (int index = 0; index < NUMBER_COMMANDS && !found; index = index + 1) {
        if (strcmp(message_sections[0], command_list[index].word) == 0) {
            found = true;
            if (parameters_present >= command_list[index].parameters) {
                (this->*command_list[index].function)();
            }
        }
    }
}

bool Serial_Handler::wait_for_next(int timeout) {
    char wait_message[64];
    int wait_index = 0;
    bool waiting = true;
    uint32_t lastTime = rtc.get_seconds_since_epoch();
    //Repeat until no longer waiting for a response
    while (waiting) {
        //Get the current time
        uint32_t currentTime = rtc.get_seconds_since_epoch();
        //If the timeout is exceeded
        if (currentTime - lastTime > timeout){
            //Stop waiting - failed, terminate the data transfer
            waiting = false;
        }

        //While there is data to read from the arduino (can safely be ignored)
        arduino.clear_incoming_characters();

        //While there are characters to read
        while (Serial.available()){
            char msg_char = Serial.read();
            //If it is a new line
            if (msg_char == '\n'){
                wait_message[wait_index] = '\0';
                //If the message is 'next' - to indicate that it is ready for the next item
                if (strcmp(wait_message, "next") == 0){
                    return false;
                }
                //Reset message and index
                wait_message[0] = '\0';
                wait_index = 0;
            //If it is any other character and within the message length
            }else if (wait_index < 63){
                //Add the character
                wait_message[wait_index] = msg_char;
                //Add termination character and increment counter
                wait_message[wait_index + 1] = '\0';
                wait_index = wait_index + 1;
            }
        }
    }
    
    return true;
}

void Serial_Handler::info_request() {
    if (state.get_logging()){
       //Is collecting and the file name
       Serial.print("info 1 ");
       Serial.print(sd_card.get_logging_file());
       Serial.print(" ");
     }else{
       //Not collecting
       Serial.print("info 0 none ");
     }
     Serial.print(sd_card.get_name());
     Serial.print(" black-box ");
     Serial.println(network.get_mac_address());
}

void Serial_Handler::type_request() {
    Serial.println("type black-box");
}

void Serial_Handler::start_request() {
    //If not currently running the experiment
    if (!state.get_logging()){
      //handle file name - check character length and validity (28 characters max) and that it doesn't exist yet
      int success = sd_card.set_logging_file(message_sections[1]);

      if (success == 0){
        bool started = sd_card.set_setup(true);
        if (started){
          rtc.store_time();
          //Reset message input buffer
          arduino.reset_message();
          //Reset counters and file
          state.reset_tip_counters();
          sd_card.reset_hourly_tips_file();
          sd_card.reset_tip_memory_file();
          //Reset the event index and the collection buffer
          state.set_event_number(1);
          sd_card.set_tip_file();
          arduino.start_reset();
          //Flag set to start collecting arduino data
          state.set_logging(true);
          Serial.println("Start working - awaiting reset");
        }else{
          //File system or write failed, send message to indicate this
          Serial.println("failed start nofiles");
        }
      } else {
        if (success == 1) {
            //File already is present on sd card
            Serial.write("failed start nofiles\n");
        } else {
            Serial.println("failed start alreadyexists");
        }
      }
    }else{
      //Message to indicate that the experiment was already running
      Serial.println("already start");
    }
}

void Serial_Handler::stop_request() {
    if (state.get_logging()){
      bool stopped = state.set_logging(false);
      if (stopped){
        //Send signal to indicate that the stop was perfomed successfully
        Serial.write("done stop\n");
        Serial2.write("LOGGING_OFF\n");
      }else{
        //Message to inicate that it did not stop due to a file system issue
        Serial.write("failed stop nofiles\n");
      }
    }else{
      //Message to indicate that the experiment was not running already
      Serial.write("already stop\n");
    }
    //Send data about memory usage
    sd_card.get_memory_data();
}

void Serial_Handler::files_request() {
    //Send back the list of files
    sd_card.list_files();
}

void Serial_Handler::delete_request() {
    //If not currently running
    if (!state.get_logging()){
      //Attempt to delete file
      if (sd_card.delete_file(message_sections[1])){
        //Send signal that file was removed successfully
        Serial.write("done delete\n");
      }else{
        //Send signal that the file did not exist
        Serial.write("failed delete nofile\n");  
      }
    }else{
      //Send signal that the system is currently running
      Serial.write("already start\n");
    }
}

void Serial_Handler::download_file(const char* FILE_NAME, unsigned long file_position) {
    //If currently logging
    if (state.get_logging()){
        sd_card.set_download_file(FILE_NAME, file_position);
        arduino.request_download();
    }else{
        //Start the file download
        sd_card.download_file(FILE_NAME, file_position);
    }
}

void Serial_Handler::download_request() {
    download_file(message_sections[1], 0UL);
}

void Serial_Handler::download_from_request() {
    //Attempt to convert start position - 0 if failed
    unsigned long line_location = strtoul(message_sections[2], NULL, 10);
    unsigned long download_start_point = sd_card.get_tip_memory_location(line_location);
    download_file(message_sections[1], download_start_point);
}

void Serial_Handler::get_time_request() {
    rtc.get_unix_time();
}

void Serial_Handler::set_time_request() {
    if (!state.get_logging()){
      //Convert given value to unsigned long
      unsigned long seconds = strtoul(message_sections[1], nullptr, 10);
      //Set the time from the unix time stamp
      rtc.set_time_seconds(seconds);
      //Send message to indicate successful time change
      Serial.println("done setTime");
    }else{
      //Send message to indicate that the experiment is currently running
      Serial.println("already start");
    }
}

void Serial_Handler::set_name_request() {
    if (sd_card.set_name(message_sections[1])) {
        Serial.println("done setName");
    } else {
        Serial.println("failed setName");
    }
    
}

void Serial_Handler::get_hourly_request() {
    //If currently receiving data
    if (state.get_logging()){
        //Perform a pause first
        arduino.request_hourly();
    } else {
        //Start the file download
        sd_card.send_hourly_tips();
    }
}

