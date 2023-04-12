/****************************************************************
 * Example1_Basics.ino
 * ICM 20948 Arduino Library Demo
 * Use the default configuration to stream 9-axis IMU data
 * Owen Lyke @ SparkFun Electronics
 * Original Creation Date: April 17 2019
 *
 * Please see License.md for the license information.
 *
 * Distributed as-is; no warranty is given.
 ***************************************************************/
#include "ICM_20948.h" // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
#include <ESP32Servo.h>
#define SERIAL_PORT Serial

#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 1
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object
Servo servo_xp;
Servo servo_xn;
Servo servo_yp;
Servo servo_yn;

void setup()
{

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
  servo_xp.attach(25);
  servo_xp.write(90);
  servo_yn.attach(26);
  servo_yn.write(90);
  servo_xn.attach(27);
  servo_xn.write(90);
  servo_yp.attach(13);
  servo_yp.write(90);
}

float x = 0;
float x_n = 0;
float y = 0;
float y_n = 0;

void servo_write_x() {
  if (x <= 90) {
    servo_xp.write(x);
    servo_xn.write(90);
  }
  else {
    servo_xn.write(x);
    servo_xp.write(90);
  }
}
void servo_write_y() {
  if (y <= 90) {
    servo_yn.write(y);
    servo_yp.write(90);
  }
  else {
    servo_yp.write(y);
    servo_yn.write(90);
  }
}

void loop()
{
  if (myICM.dataReady())
  {
    myICM.getAGMT();
    float tmp = map(myICM.accX(), -1000, 1000, 0, 180);
    if (abs(tmp - x) >= 1.5) { 
      x = tmp;
      Serial.print("x = ");
      Serial.println(x);
      servo_write_x();
    }
    tmp = map(myICM.accY(), -1000, 1000, 0, 180);
    if (abs(tmp - y) >= 1.5) { 
      y = tmp;
      Serial.print("y = ");
      Serial.println(y);
      servo_write_y();
    }
    delay(30);
  }
  else
  {
    SERIAL_PORT.println("Waiting for data");
    delay(500);
  }
}

