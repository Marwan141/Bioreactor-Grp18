#include <Wire.h>

#define SLAVE_ADDR 9
#define I2C_SDA 21
#define I2C_SCL 22

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);
}


void loop() {
  delay(100);
  Wire.requestFrom(SLAVE_ADDR, 6); // Request 18 bytes from slave
  String receivedString = "";
  while (Wire.available()) {
    char c = Wire.read(); // Read a character from the I2C buffer
    Serial.print(c);      // Print the received character to Serial Monitor
    receivedString += c;

  }

  Serial.println(); // Add a new line for better readability

  delay(1000);
}