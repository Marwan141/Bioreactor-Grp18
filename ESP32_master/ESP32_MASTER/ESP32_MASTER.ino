#include <Wire.h>
#include <string.h>

#define SLAVE_ADDR 9
#define I2C_SDA 21
#define I2C_SCL 22

int my_delay = 1000;

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);
}

// T/R/P xx.x
//       xxxx
//       xx

void loop() {
  
  delay(my_delay);
  Wire.requestFrom(SLAVE_ADDR, 6); // Request 18 bytes from slave
  
  String receivedString = "";
  String subsystem;
  int counter = 0;
  
  while (Wire.available()) {
    
    char c = Wire.read(); // Read a character from the I2C buffer

    if (counter == 0) {
      subsystem = c;
      // Serial.print("c: ");
    }
    // Serial.print(c);      // Print the received character to Serial Monitor
    
    receivedString += c;
    counter++;

  }

  // Serial.println(); // Add a new line for better readability
  // Serial.print("subsystem: ");
  // Serial.print(subsystem);
  // Serial.println();
  // Serial.print("recieved str: ");
  // Serial.print(receivedString);
  // Serial.println();

  if (subsystem == "T") {
    
    Serial.println("TEMPERATURE");
    // T SP 257 SP
    int one, two, small;
    one = (int) (receivedString[2]) - 48;
    two = (int) (receivedString[3]) - 48;
    small = (int) (receivedString[4]) - 48;
    
    float val_temperature = (one * 10) + (two) + (small * 0.1);
    Serial.println(val_temperature);

  } else if (subsystem == "P") {
    
    Serial.println("PERCENTAGE HYDROGEN");

    // P SP 50 SP SP
    int big, small;
    big = (int) (receivedString[2]) - 48;
    small = (int) (receivedString[3]) - 48;
    
    float val_pH = (big) + (small * 0.1);
    Serial.println(val_pH);

  } else if (subsystem == "R") {
    
    Serial.println("RPM");

    // R SP 1125
    int one, two, three, four;
    one = (int) (receivedString[2]) - 48;
    two = (int) (receivedString[3]) - 48;
    three = (int) (receivedString[4]) - 48;
    four = (int) (receivedString[5]) - 48;
    
    int val_RPM = (one * 1000) + (two * 100) + (three * 10) + (four);
    Serial.println(val_RPM);

  } else {
    Serial.println("OTHER");
  }

}