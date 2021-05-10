#include <stdint.h>

// DHT temperature and humidity sensor parameters
#include "DHT.h"
#define DHTPIN 7
#define DHTTYPE DHT22

// Init DHT
DHT dht(DHTPIN, DHTTYPE);


// For sending floats over serial without string conversion
typedef union {
  float f;
  struct{
    uint8_t b0 : 8;
    uint8_t b1 : 8;
    uint8_t b2 : 8;
    uint8_t b3 : 8;
  } raw;
} floatConverter_t;

// ID of this node. Must be different for each node! Cannot be 0.
const uint8_t nodeId = 2;

bool joined = false;

// Measured values buffered for sending.
volatile float temperature;
volatile float humidity;
volatile float light;

// How many loops to do between measuring again
uint8_t measureCounter = 200;




void join_network(){
  Serial.print("NODE_REQJOIN");
  Serial.write(nodeId);
  Serial.print("%");
  
  delay(20);
  if(Serial.available() > 10 && Serial.findUntil("NODE_JOIN_ACCEPT", "%")){
    joined = true;
  }
  // Empty the input buffer
  while(Serial.available()){
    Serial.read();
  }
}







void setup() {
  Serial.begin(9600);
  dht.begin();

  // Wait a bit on startup and then try to join network
  delay(2000);
  Serial.print("NODE_REQJOIN");
  Serial.write(nodeId);
  Serial.print("%");
  
  delay(20);
  if(Serial.available() && Serial.findUntil("NODE_JOIN_ACCEPT", "%")){
    joined = true;
  }
  // Empty the input buffer
  while(Serial.available()){
    Serial.read();
  }
  
  
}



void measure(){
  // Update the weather values every 10 seconds
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  light = (1024 - analogRead(A1)) / 10.0f;
}




void loop() {
  
   
  // Try to join once a minute if joining is denied
  if(!joined){
    delay(60000);
    join_network();
  }
  else{

    // Do measurements every 200th loop, about every 10 seconds
    measureCounter--;
    if(measureCounter == 0){
      measure();
      measureCounter = 200;
    }

    
    if(Serial.available()){
      // Check if there is the data request command CMD_REQDATA<node id>[T][H][L]%
      // For example, a request for node 10 for temperature and humidity would be
      // CMD_REQDATA<10>TH% where <10> is a byte whose value is 10
      if(Serial.available() > 10 && Serial.findUntil("CMD_REQDATA", "%")){
  
        delay(10);
  
        // Get the id of node from which the master wants readings
        uint8_t requestNodeId = Serial.read();
        // If the request is for this node, send the requested data.
        if(requestNodeId == nodeId){
          while(Serial.available()){
            char char_in = Serial.read();
            floatConverter_t converter;
            converter.f = 0.0f;
  
            // '%' terminates the command 
            if(char_in == '%'){
              break;
            }
            
            // T -> send temperature
            else if(char_in == 'T'){
              converter.f = temperature;
              // Send the data.
              Serial.print("DATA_RESP_T");
              Serial.write(converter.raw.b0);
              Serial.write(converter.raw.b1);
              Serial.write(converter.raw.b2);
              Serial.write(converter.raw.b3);
              Serial.print("%");
            }
            // H -> send humidity
            else if(char_in == 'H'){
              converter.f = humidity;
              // Send the data.
              Serial.print("DATA_RESP_H");
              Serial.write(converter.raw.b0);
              Serial.write(converter.raw.b1);
              Serial.write(converter.raw.b2);
              Serial.write(converter.raw.b3);
              Serial.print("%");
            }
            // L -> send light
            else if(char_in == 'L'){
              converter.f = light;
              // Send the data.
              Serial.print("DATA_RESP_L");
              Serial.write(converter.raw.b0);
              Serial.write(converter.raw.b1);
              Serial.write(converter.raw.b2);
              Serial.write(converter.raw.b3);
              Serial.print("%");
            }
            delay(20);
          }
        }
      }
    }

    delay(50);
  }
  
  
  
}
