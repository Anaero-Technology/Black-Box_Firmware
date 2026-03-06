#pragma once
#include "esp_mac.h"

class Network_Handler {
    public:
        void store_mac_address();
    private:
        char mac_address[20];
};