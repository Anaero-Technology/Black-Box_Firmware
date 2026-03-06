#include <modules/serial/serial_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/arduino/arduino_handler.h>

extern RTC_Handler rtc;
extern Arduino_Handler arduino;

void Serial_Handler::begin() {
    Serial.begin(SERIAL_BAUD);
}

void Serial_Handler::receive_characters() {

}

void Serial_Handler::split_message() {

}

void Serial_Handler::check_message() {
    
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

}

void Serial_Handler::type_request() {

}

void Serial_Handler::start_request() {

}

void Serial_Handler::stop_request() {

}

void Serial_Handler::files_request() {

}

void Serial_Handler::delete_request() {

}

void Serial_Handler::download_request() {

}

void Serial_Handler::download_from_request() {

}

void Serial_Handler::get_time_request() {

}

void Serial_Handler::set_time_request() {

}

void Serial_Handler::set_name_request() {

}

void Serial_Handler::get_hourly_request() {

}


/* OLD CODE BELOW*/

// void readCommandInput(){
//   //If there is data to be read from the laptop end
//   if (Serial.available()){
//     //How many characters are incoming
//     int numberCharacters = Serial.available();
  
//     //Iterate through the characters
//     for(int ch = 0; ch < numberCharacters; ch = ch + 1){
//       //Read the next character
//       char newChar = Serial.read();
//       //If the end of the current message has not been reached
//       if (newChar != '\n'){
//         //If the message is not too long
//         if (commandPos < 96){
//           //Add the character to the message
//           commandMsg[commandPos] = newChar;
//           commandPos = commandPos + 1;
//           if (commandPos > 96){
//             commandPos = 96;
//           }
//         }
//       }else{
//         commandMsg[commandPos] = '\0';
//         //Array to store the message split into three parts (space separated)
//         char msgParts[3][33];
//         msgParts[0][0] = '\0';
//         msgParts[1][0] = '\0';
//         msgParts[2][0] = '\0';
        
//         //Which is the current part
//         int part = 0;
//         //Current character index (within the part)
//         int cha = 0;
//         //Iterate through characters within the message
//         for (int charIndex = 0; charIndex < 97; charIndex = charIndex + 1){
//           //If a space is found and this is the first part and some letters have been added
//           if (commandMsg[charIndex] == ' ' and part < 2 and cha != 0){
//             //Add null to end the part
//             msgParts[part][cha] = '\0';
//             //Move to start of next part
//             cha = 0;
//             part = part + 1;
//           }else{
//             //If the character is in range (truncates extra characters)
//             if (cha < 32){
//               //Add character to message part
//               msgParts[part][cha] = commandMsg[charIndex];
//               cha = cha + 1;
//             }
//           }
//         }

//         //Process the received message
//         handleCommandInput(msgParts);
//         //Reset the input
//         for (int i = 0; i < 97; i = i + 1){
//           commandMsg[i] = '\0';
//         }
//         commandPos = 0;
        
//       }
//     }

//   }
// }

// void handleCommandInput(char msgParts[3][33]){
//   /*Handle the command sent from serial and perform the correct action and respond appropriately*/
//   //If this is a call for the information
//   if (strcmp(msgParts[0], "info") == 0){
//     //Send back information regarding the collection state
//      if (collecting){
//        //Is collecting and the file name
//        Serial.write("info 1 ");
//        Serial.write(fileLocation);
//        Serial.write(" ");
//      }else{
//        //Not collecting
//        Serial.write("info 0 none ");
//      }
//      Serial.write(myName);
//      Serial.write(" black-box ");
//      Serial.write(macAddress);
//      Serial.write("\n");
//      //Send the information regarding the memory usage
//      //getMemoryData();
//   }
//   else if (strcmp(msgParts[0], "type") == 0) {
//     Serial.write("type black-box\n");
//   }
//   //If this is the command to start recieving data
//   else if (strcmp(msgParts[0], "start") == 0){
//     //If not currently running the experiment
//     if (!collecting){
//       //handle file name - check character length and validity (28 characters max) and that it doesn't exist yet
//       bool success = fileNameSet(msgParts[1]);

//       if (success){
//         bool started = configureSetup(true);
//         if (started){
//           configureTime();
//           //Reset message input buffer
//           for (int i = 0; i < partMax; i = i + 1){
//             currentMessage[i][0] = '\0';
//           }
//           //Reset counters and file
//           resetTipCounters();
//           clearHourTips();
//           resetTipMemoryFile();
//           //Reset the event index and the collection buffer
//           eventNumber = 1;
//           configureTipFile();
//           collectionBuffer[0] = '\0';
//           collectionBufferPosition = 0;
//           resettingArduino = true;
//           arduinoContactTime = millis();
//           sentClear = false;
//           awaitingResume = false;
//           //Flag set to start collecting arduino data
//           collecting = true;
//           Serial.write("Start working - awaiting reset\n");
//         }else{
//           //File system or write failed, send message to indicate this
//           Serial.write("failed start nofiles\n");
//         }
//       }
//     }else{
//       //Message to indicate that the experiment was already running
//       Serial.write("already start\n");
//     }
//   }

//   //If this is the command to stop receiving data
//   else if (strcmp(msgParts[0], "stop") == 0){
//     if (collecting){
//       bool stopped = configureSetup(false);
//       if (stopped){
//         //Flag set to stop collecting arduino data
//         collecting = false;
//         //Reset the file destination
//         fileLocation[0] = '\0';
//         //Send signal to indicate that the stop was perfomed successfully
//         Serial.write("done stop\n");
//         Serial2.write("LOGGING_OFF\n");
//       }else{
//         //Message to inicate that it did not stop due to a file system issue
//         Serial.write("failed stop nofiles\n");
//       }
//     }else{
//       //Message to indicate that the experiment was not running already
//       Serial.write("already stop\n");
//     }
//     //Send data about memory usage
//     getMemoryData();
//   }

//   //If this is the command to send the file list
//   else if (strcmp(msgParts[0], "files") == 0){
//     //Send back the list of files
//     listFiles();
//   }

//   //If this is the command to delete a file
//   else if (strcmp(msgParts[0], "delete") == 0){
//     //If not currently running
//     if (!collecting){
//       //If there is a file with the given name
//       if (SD.exists(msgParts[1])){
//         //Delete the file
//         SD.remove(msgParts[1]);
//         //Send signal that file was removed successfully
//         Serial.write("done delete\n");
//       }else{
//         //Send signal that the file did not exist
//         Serial.write("failed delete nofile\n");  
//       }
//     }else{
//       //Send signal that the system is currently running
//       Serial.write("already start\n");
//     }
//   }

//   //If the message requests a file download
//   else if (strcmp(msgParts[0], "download") == 0){
//     bool done = false;
//     //Iterate through characters in the file name
//     for (int ch = 0; ch < 33 && !done; ch = ch + 1){
//       //Add the character to the name of the file to be downloaded
//       fileToDownload[ch] = msgParts[1][ch];
//       //Once the end of the name has been reached
//       if (msgParts[1][ch] == '\0'){
//         done = true;
//       }
//     }

//     downloadStartPoint = 0UL;

//     //If currently receiving data
//     if (collecting){
//       //Perform a pause first
//       awaitingDownload = true;
//       Serial2.write("PAUSE_DATA\n");
//     }else{
//       //Start the file download
//       downloadFile();
//     }
    
//   }

//   else if (strcmp(msgParts[0], "downloadFrom") == 0){
//      bool done = false;
//     //Iterate through characters in the file name
//     for (int ch = 0; ch < 33 && !done; ch = ch + 1){
//       //Add the character to the name of the file to be downloaded
//       fileToDownload[ch] = msgParts[1][ch];
//       //Once the end of the name has been reached
//       if (msgParts[1][ch] == '\0'){
//         done = true;
//       }
//     }

//     //Attempt to convert start position - 0 if failed
//     unsigned long lineLocation = strtoul(msgParts[2], NULL, 10);
//     downloadStartPoint = getTipMemoryLocation(lineLocation);

//     //If currently receiving data
//     if (collecting){
//       //Perform a pause first
//       awaitingDownload = true;
//       Serial2.write("PAUSE_DATA\n");
//     }else{
//       //Start the file download
//       downloadFile();
//     }
//   }

//   //If this is a request to get the RTC time
//   else if (strcmp(msgParts[0], "getTime") == 0){
//     getUnixTimeStamp();
//   }

//   //If this is a request to set the RTC Ttime
//   else if (strcmp(msgParts[0], "setTime") == 0){
//     //If not currently running an experiment
//     if (!collecting){
//       //Convert given value to unsigned long
//       unsigned long seconds = strtoul(msgParts[1], nullptr, 10);
//       //Set the time from the unix time stamp
//       setTimeStamp(seconds);

//       //Send message to indicate successful time change
//       Serial.write("done setTime\n");
//     }else{
//       //Send message to indicate that the experiment is currently running
//       Serial.write("already start\n");
//     }
//   }

//   else if (strcmp(msgParts[0], "setName") == 0){
//     bool done = false;
//     int index = 0;
//     int nameIndex = 0;
//     while (index < 32 && !done){
//       char c = msgParts[1][index];
//       index = index + 1;
//       if (c == '\0' || c == '\n'){
//         done = true;
//       }else{
//         myName[nameIndex] = c;
//         nameIndex = nameIndex + 1;
//       }
//     }
//     myName[nameIndex] = '\0';
//     setName();
//     Serial.write("done setName\n");
//   }

//   else if (strcmp(msgParts[0], "getHourly") == 0){
//     if(filesWorking){
//       //If currently receiving data
//       if (collecting){
//         //Perform a pause first
//         awaitingHourly = true;
//         Serial2.write("PAUSE_DATA\n");
//       }else{
//         //Start the file download
//         sendHourTips();
//       }
//     }else{
//       Serial.write("getHourly failed nofiles\n");
//     }
//   }
// }

