#include <modules/arduino/arduino_handler.h>
#include <modules/state/state_handler.h>

extern State_Handler state;

void Arduino_Handler::begin() {
    Serial2.begin(ARDUINO_BAUD, SERIAL_8N1, ARDUINO_PINS[0], ARDUINO_PINS[1]);
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

    strcpy(message_part, strtok(incoming_message, " "));
    for (int part = 0; part < MESSAGE_NUMBER_PARTS_MAX; part = part + 1) {
        strncpy(message_sections[part], message_part, MESSAGE_PART_LENGTH_MAX);
        strcpy(message_part, strtok(NULL, " "));
    }
}

void Arduino_Handler::check_message() {
    bool found = false;
    for (int index = 0; index < NUMBER_COMMANDS && !found; index = index + 1) {
        if (strcmp(message_sections[command_list[index].position], command_list[index].word) == 0) {
            found = true;
            command_list[index].function();
        }
    }
}

void Arduino_Handler::clear_incoming_characters() {
    while (Serial2.available()){
        Serial2.read();
    }
}

void Arduino_Handler::reset_message() {
    //collectionBuffer[0] = '\0';
    //collectionBufferPosition = 0;
}

void Arduino_Handler::start_reset() {
    resetting = true;
    reset_start_time = millis();
    sent_clear = false;
    waiting_for_resume = false;
}

void Arduino_Handler::ping_received() {

}

void Arduino_Handler::data_received() {
    //Flags to indicate if anthing was added and if anything has been added since the last part
    bool addedAnything = false;
    bool addedSince = false;
    //Iterate through the parts - ignore the first two
    for (int part = 4; part < 7 /*partMax*/; part = part + 1){
      //This part is not finished yet
      bool sectionDone = false;
      //If any text was added last part (prevents extra spaces in case of blank parts)
      if (addedSince){
        //Add a space and reset flag
        collectionBuffer[collectionBufferPosition] = ' ';
        collectionBufferPosition = collectionBufferPosition + 1;
        addedSince = false;
      }
      //Iterate through the characters
      for (int charPos = 0; charPos < partLenMax + 1; charPos = charPos + 1){
        //Get the character
        char character = currentMessage[part][charPos];
        //If this is the end of the part
        if (character == '\0'){
          //No need to add the remainder of the section
          sectionDone = true;
        }
        //If the section is sill being added
        if (!sectionDone){
          //Add the character to the buffer
          collectionBuffer[collectionBufferPosition] = currentMessage[part][charPos];
          collectionBufferPosition = collectionBufferPosition + 1;
          //Something has been added so set flags
          addedAnything = true;
          addedSince = true;
        }
      }
    }

    //If there was actually data in the message
    if (addedAnything){
      //Add a null to indicate this is the end of the buffer for now (do not increment counter so that next message starts here)
      collectionBuffer[collectionBufferPosition] = '\0';
      uint32_t eventTime = getSecondsSince() - experimentStartTime;
      uint32_t arduinoEventNumber = strtol(currentMessage[2], NULL, 10);
      uint32_t arduinoEventTime = strtol(currentMessage[3], NULL, 10);
      if (reRequesting){
        eventTime = arduinoEventTime;
      }

      bool askingAgain = false;
      
      if (eventNumber != arduinoEventNumber){
        if (reRequesting){
          eventNumber = arduinoEventNumber;
        }else{
          if (eventNumber == -1 || arduinoEventNumber < eventNumber){
            eventNumber = arduinoEventNumber;
          }else{
            //Store the end point
            arduinoLastEventNumber = arduinoEventNumber;
            //Request data dump from previous tip (give last one recieved)
            Serial2.write("DUMP_DATA_FROM ");
            Serial2.print(eventNumber);
            Serial2.write("\n");
            Serial.write("Re-Requesting tips from ");
            Serial.print(eventNumber);
            Serial.write("\n");
            askingAgain = true;
            reRequesting = true;
          }
        }
      }

      if (!askingAgain){
        //Write the line to the file
        outputCollectionBuffer(eventTime);
      }
    }
}

void Arduino_Handler::ready_received() {

}

void Arduino_Handler::done_received() {

}

void Arduino_Handler::data_paused_received() {
    //If a download has been scheduled
    if (awaitingDownload || awaitingHourly){
      //Get the pause time for both devices
      arduinoPauseTime = strtol(currentMessage[1], NULL, 10);
      espPauseTime = getSecondsSince();
      if (awaitingDownload){
        //Perform the download
        downloadFile();
      }else{
        sendHourTips();
      }
    }
    //Reset the download setup
    awaitingDownload = false;
    awaitingHourly = false;
    fileToDownload[0] = '\0';
    //Resume the arduino
    awaitingResume = true;
}

void Arduino_Handler::update_received() {
    //Serial.write("Added restored data\n");
    uint32_t eventTime = espPauseTime + (strtol(currentMessage[2], NULL, 10) - arduinoPauseTime) - experimentStartTime;
    //uint32_t eventTime = strtoul(currentMessage[2], NULL, 10);// - experimentStartTime;
    //Flags to indicate if anthing was added and if anything has been added since the last part
    bool addedAnything = false;
    bool addedSince = false;
    //Iterate through the parts - ignore the first two
    for (int part = 3; part < 6 /*partMax*/; part = part + 1){
      //This part is not finished yet
      bool sectionDone = false;
      //If any text was added last part (prevents extra spaces in case of blank parts)
      if (addedSince){
        //Add a space and reset flag
        collectionBuffer[collectionBufferPosition] = ' ';
        collectionBufferPosition = collectionBufferPosition + 1;
        addedSince = false;
      }
      //Iterate through the characters
      for (int charPos = 0; charPos < partLenMax + 1; charPos = charPos + 1){
        //Get the character
        char character = currentMessage[part][charPos];
        //If this is the end of the part
        if (character == '\0'){
          //No need to add the remainder of the section
          sectionDone = true;
        }
        //If the section is sill being added
        if (!sectionDone){
          //Add the character to the buffer
          collectionBuffer[collectionBufferPosition] = currentMessage[part][charPos];
          collectionBufferPosition = collectionBufferPosition + 1;
          //Something has been added so set flags
          addedAnything = true;
          addedSince = true;
        }
      }
    }

    //If there was actually data in the message
    if (addedAnything){
      //Add a null to indicate this is the end of the buffer for now (do not increment counter so that next message starts here)
      //Debug added U to indicate update line
      //collectionBuffer[collectionBufferPosition] = 'u';
      //collectionBufferPosition = collectionBufferPosition + 1;
      uint32_t arduinoEventNumber = atoi(currentMessage[1]);
      if (eventNumber != arduinoEventNumber){
        eventNumber = arduinoEventNumber;
      }
      collectionBuffer[collectionBufferPosition] = '\0';
      //Write the line to the file
      outputCollectionBuffer(eventTime);
    }
}

/* OLD CODE BELOW - MUST BE REFORMATTED AND IMPROVED*/

void arduinoMessageReceived(){
  //If needing to reset - for the start sequence
  if (resettingArduino){
    Serial.write("Testing Reset\n");
    //If the message just received was a ping and the clear has not yet been sent
    if (!sentClear && strcmp(currentMessage[1], "PING") == 0){
      //Send the clear message
      Serial2.write("SD_CLEAR\n");
      sentClear = true;
      Serial.write("Sent clear request\n");
    }

    if (sentClear){
      Serial.write("Waiting for clear response...\n");
    }

    //If the message indicates that the arduino is ready to reset
    if (strcmp(currentMessage[0], "READY") == 0){
      //Send the confirmation message
      Serial2.write("CONFIRM\n");
      Serial.write("Sent clear confirmation\n");
    }

    //If the message indicates that the reset is complete
    if (strcmp(currentMessage[0], "DONE") == 0){
      //Resetting is done
      resettingArduino = false;
      sentClear = false;
      //Start the experiment
      collecting = true;
      hourStarted = millis();
      Serial.write("done start\n");
      Serial2.write("LOGGING_ON\n");
    }
  }

  if (reRequesting && ((strcmp(currentMessage[0], "DONE") == 0) || (eventNumber > arduinoLastEventNumber && arduinoLastEventNumber > 0))){
    reRequesting = false;
    Serial.write("Stopped re-requesting\n");
  }

  //If waiting to resume data after a download
  if (awaitingResume){
    //If this message is a ping
    if (strcmp(currentMessage[1], "PING") == 0){
      //Resume the information from the arduino
      Serial2.write("RESUME_DATA\n");
      awaitingResume = false;
    }
  }
}

void checkArduinoTimeout(){
  if (resettingArduino){
    uint32_t currentTime = millis();
    uint32_t elapsed = currentTime - arduinoContactTime;
    if (currentTime < arduinoContactTime){
      elapsed = currentTime;
    }
    if (elapsed >= arduinoTimeoutDuration){
      Serial.write("failed start noarduino\n");
      collecting = false;
      resettingArduino = false;
      sentClear = false;
      fileLocation[0] = '\0';
      configureSetup(false);
      getMemoryData();
      listFiles();
    }
  }
}

void outputCollectionBuffer(uint32_t timeOccurred){
  /*Store the buffer and reset it*/
  //If ther file system is working and a file has been given to save in
  if (filesWorking && collecting && strcmp(fileLocation, "") != 0){
    //If the file does not exist
    if (!SD.exists(fileLocation)){
      //Create the file
      File newFile = SD.open(fileLocation, FILE_WRITE);
      newFile.close();
    }

    //Char array to store the whole message
    char writeBuffer[messageLength + 80];
    writeBuffer[0] = '\0';
    
    //Get the time and convert to cstring
    uint32_t timeSince = timeOccurred;
    char timeBuffer[11];
    itoa(timeSince, timeBuffer, 10);
    //Convert the number of the event to a cstring
    char indexBuffer[11];
    itoa(eventNumber, indexBuffer, 10);

    char timeStampBuffer[25];
    DateTime timeNow = rtc.now();
    //If the time does not match current time - use the time occured (means that tips that are restored from updates do not have incorrect timestamp)
    if (timeOccurred != getSecondsSince() - experimentStartTime){
      timeNow = DateTime(timeOccurred + experimentStartTime);
    }
    int timeParts[6];
    timeParts[5] = timeNow.second();
    timeParts[4] = timeNow.minute();
    timeParts[3] = timeNow.hour();
    timeParts[2] = timeNow.day();
    timeParts[1] = timeNow.month();
    timeParts[0] = timeNow.year();

    char buff[5];
    //Iterate through the different parts of the time
    for (int part = 0; part < 6; part = part + 1){
      //Convert to a c string in the buffer
      itoa(timeParts[part], buff, 10);
      strcat(timeStampBuffer, buff);

      //If this is not the last part and the buffer is not full
      if (part != 5){
        //Add the delimeter between parts
        strcat(timeStampBuffer, ".");
      }
    }

    strcat(writeBuffer, indexBuffer);
    strcat(writeBuffer, " ");
    strcat(writeBuffer, timeStampBuffer);
    strcat(writeBuffer, " ");
    strcat(writeBuffer, timeBuffer);
    strcat(writeBuffer, " ");
    strcat(writeBuffer, collectionBuffer);

    char channel[4];
    int channelPos = 0;
    bool done = false;
    for (int cha = 0; cha < messageLength && !done; cha = cha + 1){
      char c = collectionBuffer[cha];
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
    int channelNumber = atoi(channel);
    if(channelNumber > 0 && channelNumber < 16){
      tipCounts[channelNumber - 1] = tipCounts[channelNumber - 1] + 1;
    }
    
    //Open the file for append
    File appendFile = SD.open(fileLocation, FILE_APPEND);
    unsigned long fileSize = appendFile.size();

    //Add the contents of the buffer (with a new line at the end)
    appendFile.print(writeBuffer);
    appendFile.print("\n");
    //Close the file to update the data
    appendFile.close();
    Serial.write("tip ");
    Serial.write(writeBuffer);
    Serial.write("\n");
    eventNumber = eventNumber + 1;
    configureTipFile();
    writeTipMemory(fileSize);
  }else{
    if (!filesWorking){
      Serial.write("File System Has Failed (Attempting to write line)\n");
    }
  }
  //Reset the buffer position
  collectionBuffer[0] = '\0';
  collectionBufferPosition = 0;
}