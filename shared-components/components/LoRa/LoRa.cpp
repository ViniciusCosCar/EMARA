
#include "LoRa.hpp"
#include "M5_SX127X.h"
#include <SPI.h>

#define CS_PIN 33
#define RST_PIN 25
#define IRQ_PIN 36

#define LORA_MISO 38
#define LORA_MOSI 23
#define LORA_SCLK 18

#define LORA_FREQ 868E6
#define LORA_SF 12
#define LORA_BW 125E3
#define LORA_TX_POWER 17

bool LoRa_driver_init(void)
{
        SPI.begin(LORA_SCLK, LORA_MISO, LORA_MOSI, -1);

        LoRa.setSPI(&SPI);

        LoRa.setPins(CS_PIN, RST_PIN, IRQ_PIN);

        LoRa.setTxPower(LORA_TX_POWER);

        LoRa.setSignalBandwidth(LORA_BW);

        LoRa.setSpreadingFactor(LORA_SF);

        if (!LoRa.begin(LORA_FREQ)) return false;

        return true;
}

// send pacekt
// note: don't forget to put it back on RX mode afterwards!!!
size_t LoRa_driver_send(const uint8_t* data, size_t len)
{
        if (data == nullptr || len == 0) return 0;

        // enter idle mode and start LoRa packet
        // note: change mode if not 1
        if (!LoRa.beginPacket()) return 0;

        size_t size = LoRa.write(data, len);

        if (size != len)
        {
                LoRa.endPacket();
                return 0;
        }
        // enter TX mode
        if (!LoRa.endPacket()) return 0;

        return size;
}

// partially blocking reading function
size_t LoRa_driver_receive(uint8_t* buffer, size_t max_len)
{
        if (!LoRa.parsePacket()) return 0;

        size_t BytesRead = 0;

        while (LoRa.available() && BytesRead < max_len)
        {
                buffer[BytesRead] = LoRa.read();
                BytesRead++;
        }

        return BytesRead;
}

// register callback to be called when a packet is received
void LoRa_on_rx(void (*callback)(int)) { LoRa.onReceive(callback); }
