#pragma once
#include "esp_mac.h"
#include "Arduino.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "modules/rtc/rtc_handler.h"

class Network_Handler {
    public:
        void store_mac_address();
        char* get_mac_address();
        bool set_wifi_info(const char* SSID, const char* PASS);
        bool connect_to_wifi();
        bool set_host_address(const char* ADDRESS);
        bool send_message(const char* DATA);
        bool is_wifi_connected();;
    private:
        char mac_address[20];
        char wifi_ssid[100];
        char wifi_password[100];
        bool connected_to_wifi = false;
        char host_address[100];
        unsigned long connection_timeout = 10000UL;
};