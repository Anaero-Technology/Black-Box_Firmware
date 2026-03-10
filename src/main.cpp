#include <modules/sd_card/sd_handler.h>
#include <modules/network/network_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/serial/serial_handler.h>
#include <modules/arduino/arduino_handler.h>
#include <modules/state/state_handler.h>

/*#include "modules/sd_card/sd_handler.h"
#include "modules/network/network_handler.h"
#include "modules/state/state_handler.h"
#include "modules/rtc/rtc_handler.h"
#include "modules/serial/serial_handler.h"
#include "modules/arduino/arduino_handler.h"*/

SD_Handler sd_card;
Network_Handler network;
State_Handler state;
RTC_Handler rtc;
Serial_Handler serial;
Arduino_Handler arduino;

bool started_successfully = true;

void arduino_messages() {

}

void serial_messages() {
    
}

void setup() {
    serial.begin();
    arduino.begin();
    
    if (!sd_card.begin()) {
        Serial.println("Failed to start SD");
        started_successfully = false;
    }
    if (!rtc.begin()) {
        Serial.println("Failed to start RTC");
        started_successfully = false;
    }
    network.store_mac_address();

    if (!started_successfully) {
        while (1);
    }
}

void loop() {
    arduino_messages();
    serial_messages();
}



/* OLD CODE BELOW */

// void resetTipCounters(){
//   //Reset the counters for each channel so a new hour can begin
//   for (int i = 0; i < 15; i = i + 1){
//     tipCounts[i] = 0;
//   }
// }

// void updateHourTime(){
//   if(collecting){
//     uint32_t currentTime = millis();
//     uint32_t elapsed = currentTime - hourStarted;
//     if (currentTime < hourStarted){
//       elapsed = (ULONGMAX - hourStarted) + currentTime;
//     }
//     if (elapsed >= hourLength){
//       outputTipCounters();
//       if(filesWorking){
//         saveHourTips();
//       }
//       resetTipCounters();
//       hourStarted = hourStarted + hourLength;
//     }
//   }
// }

// void outputTipCounters(){
//   Serial.write("counts ");
//   for (int i = 0; i < 15; i = i + 1){
//     Serial.print(tipCounts[i]);
//     Serial.write(" ");  
//   }
//   Serial.write("\n");
// }

// void saveHourTips(){
//   if(!SD.exists(hourlyTipFile)){
//     File hourlyFile = SD.open(hourlyTipFile, FILE_WRITE);
//     hourlyFile.close();
//   }
//   File hourlyFile = SD.open(hourlyTipFile, FILE_APPEND);
//   for(int i = 0; i < 15; i = i + 1){
//     hourlyFile.print(tipCounts[i]);
//     if(i != 14){
//       hourlyFile.print(' ');
//     }
//   }
//   hourlyFile.print('\n');
//   hourlyFile.close();
// }

// void clearHourTips(){
//   File hourlyFile = SD.open(hourlyTipFile, FILE_WRITE);
//   hourlyFile.close();
// }