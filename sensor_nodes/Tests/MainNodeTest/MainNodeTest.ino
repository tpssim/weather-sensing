#include <Wire.h>
#include <WireData.h>
#include <stdint.h>
#define I2C_ADDRESS 0x0A

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);
  Serial.println("Setup done");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void onI2CReceive(uint8_t byteCount){
  Serial.print("Received: ");
  uint8_t received;
  wireReadData(received);
  Serial.println(received);
}

void onI2CRequest(){
  Serial.println("Got a request, whohoo!!");
  float testfloat = 3.1415f;
  wireWriteData(testfloat);
}
