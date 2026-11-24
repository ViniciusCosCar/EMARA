// web system handler .hpp

#pragma once

#include "esp_bit_defs.h"
#include "freertos/idf_additions.h"
#include "packet.hpp"

using namespace Packet;

// task handle
extern TaskHandle_t system_handle;
extern void system_handler(void*);

// exposed packet data buffer
extern uint8_t pk_data[MAX_PACKET_SIZE];

//
// interrupt source bits
//
// success
#define TRAJECTORY_DEFINED 0x01
#define LORA_PACKET_RECEIVED 0x02
// failure: can be ored with a failure code
#define LORA_PACKET_RECEIVED_BUT_ERROR 0x03

// failure codes
#define FAILURE_RC_OVERFLOW (0x01 << 4)
#define FAILURE_RC_UNKNOWN (0x02 << 4)
