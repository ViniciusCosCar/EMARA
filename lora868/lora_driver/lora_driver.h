#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <stdint.h>
#include <stddef.h>

void lora_driver_init(void);

int lora_driver_send(const uint8_t *data, size_t len);

int lora_driver_receive(uint8_t *buffer, size_t max_len);

#endif
