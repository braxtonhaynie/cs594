#include <esp_now.h>
#include <WiFi.h>
#include "ICM_20948.h"
#include <Wire.h>

#define SERIAL_PORT Serial

#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 1
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x70, 0xB8, 0xF6, 0x4F, 0x85, 0x04};
//  0x70, 0xB8, 0xF6, 0x4D, 0xEF, 0x40
// Define variables to store BME280 readings to be sent
float accX;
float accY;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  float accX;
  float accY;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message IMUReadings;

// Create a struct_message to hold incoming sensor readings
struct_message incomingMessage;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

void setup() {
  // Init Serial Monitor
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT)
  {
  };

  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);

  bool initialized = false;
  while (!initialized)
  {
    myICM.begin(WIRE_PORT, AD0_VAL);
    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    }
    else
    {
      initialized = true;
    }
  }
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  // esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  getReadings();
 
  // Set values to send
  IMUReadings.accX = accX;
  IMUReadings.accY = accY;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &IMUReadings, sizeof(IMUReadings));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  SERIAL_PORT.print("X = ");
  SERIAL_PORT.print(accX);
  SERIAL_PORT.print(" Y = ");
  SERIAL_PORT.println(accY);
  delay(200);
}

float prevX = -1;
float prevY = -1;
void getReadings(){
  myICM.getAGMT();
  if (abs(myICM.accX() - prevX) < 100) accX = prevX;
  accX += 40;
  if (abs(myICM.accY() - prevY) < 100) accY = prevY;
  accY += 10;
  prevX = myICM.accX();
  prevY = myICM.accY();
}
