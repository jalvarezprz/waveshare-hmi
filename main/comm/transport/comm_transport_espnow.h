#pragma once
#include <cstddef>
#include <cstdint>
#include "esp_now.h"

bool comm_espnow_init_sta();                                      // Wi-Fi (STA sin IP) + esp_now_init
bool comm_espnow_add_peer(const uint8_t mac[6], uint8_t channel); // Alta de peer (unencrypted)
bool comm_espnow_send(const uint8_t mac[6], const void* data, size_t len);

// Helpers
bool comm_parse_mac_str(const char* str, uint8_t mac[6]);         // "aa:bb:cc:dd:ee:ff" → bytes
void comm_espnow_get_self_mac(uint8_t mac[6]);                    // STA MAC
