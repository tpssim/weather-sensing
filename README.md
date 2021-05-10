# Weather sensing using XBee and Arduino
Hands-on project for Communication Technologies and Security in IoT course
A simple demonstration on building an XBee network with some sensors and a web server to upload the results to.

!!! There are absolutely no security measures in place. Do not use the code as-is for anything but testing. !!!

## Basic architecture
The weather sensor network is divided into three main parts:
  - Nodes. These measure different environmental variables such as temperature and humidity.
  - Main node. It keeps track of the nodes attached to the network, collects all data measured by the nodes and transmits the data to internet.
  - Web service to store the measurements and make them available over the internet.

## Running the web server
First time only:
```bash
npm install
```
Starting the server in development mode:
```bash
npm run dev
```
The server operates on port 3001 by default.

## Setting up the nodes
First the XBee modules have to be configured for the network to operate correctly. For this project X-CTU was used for configuration, and the Arduino UNOs along with the XBee shields were used as a serial to USB converter. There are many online resources on how to configure XBee networks using these.

The XBee network configuration needs to be as follows:
  -The main node sends data to all of the sensor nodes.
  -The sensor nodes send data only to the main node.

### Main node
The main node is responsible for collecting all the data measured by the nodes in the sensor network. Within this project, it consist of an Arduino Uno with an
XBee shield and a ESP8266 based microcontroller (LOLIN Wemos D1 Mini) for enabling internet connectivity. The two communicate using I2C.

In addition to handling measurement data from sensor nodes, the main node in this project has also the functionality of a sensor node. It has the same sensors attached to it.

#### Building the main node:
In the reference build the main node consists of:
  -Arduino UNO
  -XBee shield
  -XBee radio module
  -LOLIN Wemos D1 Mini (an ESP8266 based microcontroller with WiFi support)
  -A Pololu logic level shifter for using I2C between the Arduino (5V) and the Wemos (3.3V).
  -DHT22 temperature and humidity sensor
  -An analog light sensor
  
1. Connect the XBee radio module to the Arduino using the shield.
2. Connect the Arduino I2C pins (SDA to A4 and SCL to A5 on UNO) to two different ports of the logic level shifter, to the high voltage side.
3. Connect corresponding I2C pins in the Wemos D1 Mini to the correct ports on the low voltage side of the logic level shifter. Connect SDA to D4 and SCL to D3, but these are configurable in the code unlike on the Arduino.
4. Connect the light sensor analog output to A1 on the Arduino (can be changed in software).
5. Connect the data out pin of DHT to D7 on the Arduino (can be changed in software).
6. Wire the ground and VCC connections to all the components. Short together the grounds on the Arduino and Wemos so that I2C can function correctly. Supply the logic level shifter with 5V from Arduino (high voltage supply) and 3.3V from the Wemos (low voltage supply).

#### Building a sensor node
In the reference build, a sensor node consists of:
  -Arduino UNO
  -XBee shield
  -XBee radio module
  -DHT22 temperature and humidity sensor
  -An analog light sensor
  
1. Connect the XBee radio module to the Arduino using the shield.
2. Connect the light sensor analog output to A1 on the Arduino.
3. Connect the data out pin of DHT to D7 on the Arduino.
4. Connect voltage supplies for the light sensor and DHT.
5. Sensor nodes have a node id defined in the Node.ino source file. That has to be different for all of the nodes, so change it before uploading the code to multiple nodes.
