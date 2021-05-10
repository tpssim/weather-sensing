#include <stdint.h>

// DHT temperature and humidity sensor parameters
#include "DHT.h"
#define DHTPIN 7
#define DHTTYPE DHT22

//Need I2C connection with LOLIN Wemos D1 mini ESP8266 board with WiFi
//Using the Wire library for I2C
//SDA to pin A4 on UNO
//SCL to pin A5 on UNO
#include <Wire.h>
#include <WireData.h>
#define I2C_ADDRESS 0x0A

//Init DHT
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


// How many nodes in the XBee network
volatile uint8_t numberOfNodes = 1; // Total nodes (this + connected)
const uint8_t maxNumberOfConnectedNodes = 15; // How many connected nodes there can be

// The node ids of the current nodes in the network.
// Node id 0 is reserved for MainNode (this) so it means unassigned.
uint8_t nodes[maxNumberOfConnectedNodes];

// Store the measured values from nodes here for transmitting
volatile float temperature[maxNumberOfConnectedNodes + 1];
volatile float humidity[maxNumberOfConnectedNodes + 1];
volatile float light[maxNumberOfConnectedNodes + 1];

// All measurements combined into a single array for sending via I2C
volatile float allData[maxNumberOfConnectedNodes * 3 + 3];

// I2C command from master
// 0 - send the number of nodes in the network
// 1 - send all the measurements
volatile uint8_t i2cCmd;
// Index of the value to send when the measured values are requested
volatile uint8_t currentSendValueIndex = 0;
volatile uint8_t currentSendNodeIndex = 0; // The id of the node whose data is being sent


// How many loops to do between measuring again
uint16_t measureCounter = 400;


void measure(){
  // Read the weather values for this node.
  temperature[0] = dht.readTemperature();
  humidity[0] = dht.readHumidity();
  light[0] = (1024 - analogRead(A1)) / 10.0f;

  // Flush the receive buffer so that it is clean for getting the values
  while(Serial.available()){
    Serial.read();
  }
  
  // Get the weather values from every other XBee node in the network
  for(uint8_t i = 1; i < maxNumberOfConnectedNodes; i++){
    // Skip unassigned nodes
    if(nodes[i] == 0){
      continue;
    }
    else{
      Serial.print("CMD_REQDATA");  // Command
      Serial.write(nodes[i]);       // Node id
      Serial.print("THL%");         // Temperature, Humidity, Light and terminator
    
      // Wait a little for response
      uint8_t timeout = 200;
      while(Serial.available() < 48){
        timeout--;
        if(timeout == 0){
          break;
        }
        delay(1);
      }
      
      if(Serial.findUntil("DATA_RESP_T", "%")){
        // Read temperature
        if(Serial.available() > 4){
          floatConverter_t converter;
          converter.raw.b0 = Serial.read();
          converter.raw.b1 = Serial.read();
          converter.raw.b2 = Serial.read();
          converter.raw.b3 = Serial.read();
          temperature[i] = converter.f;
          
          if(Serial.peek() == '%'){
            Serial.read();
          }
        }
      }
      if(Serial.findUntil("DATA_RESP_H", "%")){
        // Read humidity
        if(Serial.available() > 4){
          floatConverter_t converter;
          converter.raw.b0 = Serial.read();
          converter.raw.b1 = Serial.read();
          converter.raw.b2 = Serial.read();
          converter.raw.b3 = Serial.read();
          humidity[i] = converter.f;
          
          if(Serial.peek() == '%'){
            Serial.read();
          }
        }
      }
      if(Serial.findUntil("DATA_RESP_L", "%")){
        // Read light
        if(Serial.available() > 4){
          floatConverter_t converter;
          converter.raw.b0 = Serial.read();
          converter.raw.b1 = Serial.read();
          converter.raw.b2 = Serial.read();
          converter.raw.b3 = Serial.read();
          light[i] = converter.f;

          if(Serial.peek() == '%'){
            Serial.read();
          }
        }
      }
    }
  }
  
  // Update the I2C-sendable array for all nodes
  for(uint8_t node = 0; node < numberOfNodes + 1; node++){
    allData[3 * node]     = temperature[node];
    allData[3 * node + 1] = humidity[node];
    allData[3 * node + 2] = light[node];
  }
  
}

void setup() {

  //Start serial
  Serial.begin(9600);

  //Start I2C as a slave (ESP does not support slave mode)
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);
  
  //Start DHT
  dht.begin();
}

void loop() {

  // Do measurements every 200th loop, about every 10 seconds
  measureCounter--;
  if(measureCounter == 0){
    measure();
    measureCounter = 100;
  }

  // Check if a node wants to join the network
  if(Serial.available() > 10 && Serial.findUntil("NODE_REQJOIN", "%")){
    delay(5);
    // Allow the node to join if it has an id that is not yet used
    if(Serial.available() > 1){
      uint8_t newNodeId = Serial.read();
      // If the join request is terminated correctly and there is no node with same id then make the join
      if(Serial.read() == '%'){

        bool joined = false;
        for(uint8_t i = 1; i < maxNumberOfConnectedNodes; i++){
          if(nodes[i] == 0){
            nodes[i] = newNodeId;
            numberOfNodes++;
            Serial.print("NODE_JOIN_ACCEPT");
            Serial.write(newNodeId);
            Serial.print("%");
            joined = true;
            break;
          }
        }
        if(!joined){
          Serial.print("NODE_JOIN_DENY");
          Serial.write(newNodeId);
          Serial.print("%");
        }
      }
    }
  }
  


  delay(25);
}




void onI2CReceive(uint8_t byteCount){
  // Get the I2C command
  wireReadData(i2cCmd);
}

void onI2CRequest(){
  
  // The command tells which data the master wants
  if(i2cCmd == 0){
    // Send the number of nodes (int)
    wireWriteData(numberOfNodes);
    // Reset the send index as the master is requesting new data.
    currentSendValueIndex = 0;
  }
  else if(i2cCmd == 1){
    // Send the actual measurements
    if(currentSendValueIndex >= maxNumberOfConnectedNodes){
      float zero = 0.0f;
      wireWriteData(zero);
    }
    else{
      wireWriteData(allData[currentSendValueIndex]);
      currentSendValueIndex++;
      // Skip unassigned id:s (0 marks unused, but the first one is reserved for MainNode
      if(currentSendValueIndex > 2 && nodes[currentSendValueIndex / 3] == 0){
        currentSendValueIndex += 3;
      }
    }

  }
  else{
    uint8_t zero = 0;
    wireWriteData(zero);
  }
}
