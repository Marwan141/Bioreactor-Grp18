#include <Wire.h>

#define SLAVE_ADDR 9


void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
}

void loop() {
 
  delay(100);
}

void requestEvent() {
  Wire.write("hello "); // respond with message
}