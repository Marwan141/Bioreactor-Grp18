#include <Wire.h>

#define SLAVE_ADDR 9

int my_delay = 1000;

void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
}

void loop() {
  delay(my_delay);
  Wire.onRequest(requestEvent); // register event
}

void requestEvent() {
  // Wire.write("T 257 "); // respond with message
  // Wire.write("P 57  ");
  Wire.write("R 1125");
}