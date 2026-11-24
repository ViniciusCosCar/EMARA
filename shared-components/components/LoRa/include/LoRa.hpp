#pragma once

#include <stddef.h>
#include <stdint.h>

// initialize LoRa driver
bool LoRa_init(void);

// send pacekt
size_t LoRa_send_chunk(const uint8_t*, size_t);

// register callback to be called when a packet is received
void LoRa_on_receive(void (*callback)(int));

// read and return byte from FIFO register
int LoRa_read_intbyte();
