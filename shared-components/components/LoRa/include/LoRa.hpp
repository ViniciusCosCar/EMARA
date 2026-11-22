#pragma once

#include <stddef.h>
#include <stdint.h>

bool LoRa_driver_init(void);

size_t LoRa_driver_send(const uint8_t* data, size_t len);
size_t LoRa_driver_receive(uint8_t* buffer, size_t max_len);

void LoRa_on_rx(void (*callback)(int));
