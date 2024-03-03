#pragma once

// look for original descriptions in CRC library

uint8_t crc8(const uint8_t *addr, uint8_t len);
bool check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc = 0);
uint16_t crc16(const uint8_t* input, uint16_t len, uint16_t crc = 0);
