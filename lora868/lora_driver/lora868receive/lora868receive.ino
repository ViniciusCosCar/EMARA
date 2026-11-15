#include  "lora_driver.h"

uint8_t buffer[256];

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
  int received = lora_driver_receive(buffer,sizeof(buffer));
  
  if(received > 0)
  {
    Serial.write(buffer,received);
    Serial.println();
  }
}
