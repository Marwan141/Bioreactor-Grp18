#include <Wire.h>

#define SLAVE_ADDR 12
#define MASTER_ADDR 11
// #define I2C_SDA smth
// #define I2C_SCL smth

int my_delay = 1000;
String current_string, subsystem;

void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void loop() {

  if (subsystem == "T") {
    
    Serial.println("TEMPERATURE");
    // T SP 257 SP
    int one, two, small;
    one = (int) (current_string[2]) - 48;
    two = (int) (current_string[3]) - 48;
    small = (int) (current_string[4]) - 48;
    
    float val_temperature = (one * 10) + (two) + (small * 0.1);
    Serial.println(val_temperature);

  } else if (subsystem == "P") {
    
    Serial.println("PERCENTAGE HYDROGEN");

    // P SP 50 SP SP
    int big, small;
    big = (int) (current_string[2]) - 48;
    small = (int) (current_string[3]) - 48;
    
    float val_pH = (big) + (small * 0.1);
    Serial.println(val_pH);

  } else if (subsystem == "R") {
    
    Serial.println("RPM");

    // R SP 1125
    int one, two, three, four;
    one = (int) (current_string[2]) - 48;
    two = (int) (current_string[3]) - 48;
    three = (int) (current_string[4]) - 48;
    four = (int) (current_string[5]) - 48;
    
    int val_RPM = (one * 1000) + (two * 100) + (three * 10) + (four);
    Serial.println(val_RPM);

  } else {
    Serial.println("OTHER");
  }

  delay(my_delay);

}

void requestEvent() {
  // Wire.write("T 257 "); // respond with message
  // Wire.write("P 57  ");
  Wire.write("R 1125");
}

void receiveEvent(int howMany) {

  current_string = "";
  int counter = 0;

  while (Wire.available()) {

    char c = Wire.read();
    
    if (counter == 0) {
      subsystem = c;
    }

    current_string += c;
    counter++;

  }

}
