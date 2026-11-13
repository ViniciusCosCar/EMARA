#include  "lora_driver.h"

void setup()
{
  Serial.begin(9600);

  if(!lora_driver_init())
    Serial.println("Lora initialization failed");

} 
void loop()
{
}

