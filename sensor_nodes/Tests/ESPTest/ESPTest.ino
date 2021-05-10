#include <Wire.h>
#include <WireData.h>
#include <stdint.h>
#define SDA_PIN D4
#define SCL_PIN D3
#define I2CSlaveAddress 0x0A

volatile uint8_t counter = 0;
volatile float a;
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("Setup done and serial is working...");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  Wire.beginTransmission(I2CSlaveAddress);
  wireWriteData(counter);
  Wire.endTransmission();

  Serial.print("Sent counter value ");
  Serial.print(counter);

  Wire.requestFrom(I2CSlaveAddress, sizeof(a));
  wireReadData(a);
  Serial.print(" and received float ");
  Serial.println(a);
  Serial.println();
  
  counter++;
}
