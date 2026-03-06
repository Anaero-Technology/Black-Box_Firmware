#include <modules/network/network_handler.h>


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