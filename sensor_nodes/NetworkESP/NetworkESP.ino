#include <stdint.h>
#include <Wire.h>
#include <WireData.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define SDA_PIN D4
#define SCL_PIN D3
#define SSID ""                         // Wifi SSID
#define PASSWD ""                       // Wifi passwd
#define ADDR ""                         // Server IP

ESP8266WifiMulti wifiMulti;

// I2C address of the XBee coordinator
const uint8_t I2CSlaveAddress = 0x0A;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(9600);

  wifiMulti.addAP(SSID, PASSWD);
}

void loop() {
  // Get the sensor readings from the XBee coordinator
  // First send the byte that tells the coordinator which data to send
  // 0 - request the number of sensor nodes in the XBee network
  // 1 - request the measurement data from all nodes (data size is 3 floats for every node - temperature, humidity, light)

  uint8_t cmd = 0;
  uint8_t noOfNodes;
  
  // Get the number of devices
  Wire.beginTransmission(I2CSlaveAddress);
  wireWriteData(cmd);
  Wire.endTransmission(false);
  
  Wire.requestFrom(I2CSlaveAddress, sizeof noOfNodes);
  wireReadData(noOfNodes);

  Serial.print("Number of nodes: ");
  Serial.println(noOfNodes);
  
  //Get the measurement data for each node
  cmd = 1;
  Wire.beginTransmission(I2CSlaveAddress);
  wireWriteData(cmd);
  Wire.endTransmission(true);
  // How many values to receive
  uint8_t dataLength = 3 * noOfNodes;
  // For storing the received data
  float receiveData[dataLength];

  // A single value received
  float receiveValue;

  // One value at a time, get all the data
  for(uint8_t i = 0; i < dataLength; i++){
    Wire.requestFrom(I2CSlaveAddress, sizeof receiveValue);
    wireReadData(receiveValue);
    receiveData[i] = receiveValue;
  }

  // Print the values to serial
  // This can be modified to send the values to internet server
  char message[128];
  for(uint8_t i = 0; i < dataLength; i += 3){

    float temperature = receiveData[i];
    float humidity = receiveData[i+1];
    float light = receiveData[i+2];
    snprintf_P(message, sizeof(message), PSTR("Node %i: Temperature %f Humidity %f Light %f"), (i+1)/3, temperature, humidity, light);
    Serial.println(message);

    if(wifiMulti.run() == WL_CONNECTED){
      // Create JSON document
      StaticJosonDocument<128> doc;
      char output[128];

      doc["sensor"] = "node_id";

      JsonObject data = doc.createNestedObject("data");
      data["temperature"] = temperature;
      // data["pressure"] = pressure;
      data["humidity"] = humidity;
      data["ligthlevel"] = light;

      serializeJson(doc, output);

      // Send via http
      HTTPClient http;

      http.begin(IP);
      http.addHeader("Content-Type", "application/json");

      int httpCode = http.POST(output);
      
      if(httpCode == HTTP_CODE_OK){
        Serial.println("Data sent to server.");
      }
      else{
        Serial.print("Failed to send data to server. Code: ");
        Serial.println(httpCode);
      }

      http.end();
    }
  }

  // Get the data once in 10 seconds.
  delay(10000);  
}
