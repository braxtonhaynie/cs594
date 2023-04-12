#include <esp_now.h>
#include <WiFi.h>
#include "ICM_20948.h" 
#include <ESP32Servo.h>
#include <Wire.h>

#define SERIAL_PORT Serial
#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 1
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x70, 0xB8, 0xF6, 0x4D, 0xEF, 0x40};
// 0x70, 0xB8, 0xF6, 0x4F, 0x85, 0x04

// Define variables to store incoming readings
float recv_accX;
float recv_accY;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
  float accX;
  float accY;
} struct_message;

// Create a struct_message to hold incoming sensor readings
struct_message incomingMessage;
esp_now_peer_info_t peerInfo;

Servo servo_xp;
Servo servo_xn;
Servo servo_yp;
Servo servo_yn;
bool ready = false;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Bytes received: ");
  Serial.println(len);
  recv_accX = map(incomingMessage.accX, -900, 850, 0, 180);
  recv_accY = map(incomingMessage.accY, -820, 840, 0, 180);
  ready = true;
}

void setup() {
  // Init Serial Monitor
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT)
  {
  };

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    SERIAL_PORT.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  // esp_now_register_send_cb(OnDataSent);
  
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
  esp_now_register_recv_cb(OnDataRecv);
  
  // initialize servos
  servo_xp.attach(25);
  servo_xp.write(90);
  servo_yn.attach(26);
  servo_yn.write(90);
  servo_xn.attach(27);
  servo_xn.write(90);
  servo_yp.attach(13);
  servo_yp.write(90);
}

/*
void servo_write_x(float x) {
  if (x >= 85 && x <= 95) {
    servo_xp.write(90);
    servo_xn.write(15);
  }
  if (x <= 90) {
    if (x < 0) x = 0;
    servo_xp.write(90);
    servo_xn.write(15);
    Serial.print("in servo write if ");
    Serial.println(x);
  }
  else {
    if (x > 180) x = 180;
    Serial.print("in servo write else ");
    Serial.println(x - 90);
    x = x - 80;
    servo_xn.write(x);
    servo_xp.write(90);
  }
}
void servo_write_y(float y) {
  if (y <= 90) {
    if (y < 0) y = 0;
    servo_yn.write(90);
    servo_yp.write(90);
  }
  else {
    if (y > 180) y = 180;
    servo_yp.write(y / 180 * 90);
    servo_yn.write(90);
  }
}
*/
void servo_write_x(float x) {
  if (x >= 85 && x <= 95) {
    servo_xp.write(90);
    servo_xn.write(90);
  }
  if (x <= 90) {
    if (x < 0) x = 0;
    servo_xp.write(x);
    servo_xn.write(90);
  }
  else {
    if (x > 180) x = 180;
    servo_xn.write(x);
    servo_xp.write(90);
  }
}
void servo_write_y(float y) {
  if (y <= 90) {
    if (y < 0) y = 0;
    servo_yn.write(y);
    servo_yp.write(90);
  }
  else {
    if (y > 180) y = 180;
    servo_yp.write(y);
    servo_yn.write(90);
  }
}

void loop() {
  SERIAL_PORT.print("X = ");
  SERIAL_PORT.print(recv_accX);
  SERIAL_PORT.print(" Y = ");
  SERIAL_PORT.print(recv_accY);
  if (ready) {
    servo_write_x(recv_accX);
    servo_write_y(recv_accY);
    ready = false;
  }
  delay(200);
}



