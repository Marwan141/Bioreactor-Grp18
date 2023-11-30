#include <Wire.h>

#define SLAVE_ADDR 12
#define MASTER_ADDR 11
// #define I2C_SDA smth
// #define I2C_SCL smth

int my_delay = 1000;

void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {
  delay(my_delay);
}

void requestEvent() {
  // Wire.write("T 257 "); // respond with message
  // Wire.write("P 57  ");
  Wire.write("R 1125");
}

void receiveEvent(int howMany) {

  while (Wire.available()) {
    char c = Wire.read();
    Serial.print(c);
  }
  Serial.println();

}
