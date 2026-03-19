#include <modules/sd_card/sd_handler.h>
#include <modules/network/network_handler.h>
#include <modules/rtc/rtc_handler.h>
#include <modules/serial/serial_handler.h>
#include <modules/arduino/arduino_handler.h>
#include <modules/state/state_handler.h>

SD_Handler sd_card;
Network_Handler network;
State_Handler state;
RTC_Handler rtc;
Serial_Handler serial;
Arduino_Handler arduino;

bool started_successfully = true;

void arduino_messages() {
    arduino.check_timeout();
    arduino.receive_characters();
}

void serial_messages() {
    serial.receive_characters();
}

void update_hours() {
    if (rtc.update_hour_time()) {
        state.output_tip_counters();
        sd_card.save_hourly_tips();
        state.reset_tip_counters();
    }
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
    Serial.println("Loop Starting");
}

void loop() {
    arduino_messages();
    serial_messages();

    update_hours();
}