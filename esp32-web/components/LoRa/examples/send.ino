#include "LoRa.hpp"

void setup()
{
  Serial.begin(9600);

  if(!lora_driver_init())
    Serial.println("Lora initialization failed");
  
  else
    Serial.println("Lora initialization was a sucess");
} 

void loop()
{
  const char message[] = "Hello";
  size_t size = lora_driver_send((const uint8_t *) message, sizeof(message));
  if(size > 0 )
  {
    Serial.println("The message was sent");
  }
  delay(2000);
}


