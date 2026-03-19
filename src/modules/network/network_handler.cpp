#include <modules/network/network_handler.h>

extern RTC_Handler rtc;

void Network_Handler::store_mac_address() {
  unsigned char mac[6] = {};
  esp_efuse_mac_get_default(mac);
  if (esp_read_mac(mac, ESP_MAC_ETH) == ESP_OK) {
    sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  } else {
    sprintf(mac_address, "--:--:--:--:--:--");
  }
}

char* Network_Handler::get_mac_address() {
    return mac_address;
}

bool Network_Handler::set_wifi_info(const char* SSID, const char* PASS) {
    if (strlen(SSID) < 99) {
        strcpy(wifi_ssid, SSID);
    } else {
        return false;
    }
    if (strlen(PASS) < 99) {
        strcpy(wifi_password, PASS);
    } else {
        return false;
    }
    return true;
}

bool Network_Handler::connect_to_wifi() {
    if (connected_to_wifi) {
        WiFi.disconnect();
    }
    WiFi.begin(wifi_ssid, wifi_password);
    bool connected = false;
    bool timed_out = false;
    unsigned long start = millis();
    while (!connected && !timed_out) {
        if (WiFi.status() == WL_CONNECTED) {
            connected = true;
        }
        if (rtc.check_elapsed(start, millis(), connection_timeout)) {
            timed_out = true;
        }
    }

    if (!connected) {
        WiFi.disconnect();
    }

    connected_to_wifi = connected;

    return connected_to_wifi;
}

bool Network_Handler::set_host_address(const char* ADDRESS) {
    if (strlen(ADDRESS) < 99) {
        strcpy(host_address, ADDRESS);
        return true;
    }
    return false;
}

bool Network_Handler::send_message(const char* DATA) {
    if (connected_to_wifi && strlen(DATA) < 99) {
        char full_data[230];
        strcpy(full_data, host_address);
        //Delimeter
        strcat(full_data, mac_address);
        //Delimeter
        strcat(full_data, DATA);
        HTTPClient http;
        http.begin(full_data);
        int http_response = http.GET();
        http.end();
        if (http_response < 0) {
            return false;
        }
        return true;
    }
    return false;
}

bool Network_Handler::is_wifi_connected() {
    return connected_to_wifi;
}