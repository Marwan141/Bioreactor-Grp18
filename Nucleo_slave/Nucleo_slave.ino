#include <Wire.h>

#define SLAVE_ADDR 12
#define MASTER_ADDR 11
// #define I2C_SDA smth
// #define I2C_SCL smth

int my_delay = 1000;
String current_string, subsystem;

const int stirring_low_threshold = 200;
const int stirring_high_threshold = 300;
unsigned long stirring_pulse_count = 0;
unsigned long stirring_start_time = 0;
unsigned long stirring_period = 0;

int stirring_RPM = 0;
String to_send = "ABCDEF";

void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  analogWrite(9, 255);
}

// val_pH/val_... needs to be sent to actuators
void loop() {

  if (subsystem == "T") {
    
    Serial.println("TEMPERATURE");
    // T SP 257 SP
    int one, two, small;
    one = (int) (current_string[2] - 48);
    two = (int) (current_string[3] - 48);
    small = (int) (current_string[4] - 48);

    float val_temperature = (one * 10) + (two) + (small * 0.1); //remind raghav
    Serial.println(val_temperature);

    current_string = "";
    subsystem = "";

  } else if (subsystem == "P") {
    
    Serial.println("PERCENTAGE HYDROGEN");

    // P SP 50 SP SP
    int big, small;
    big = (int) (current_string[2]) - 48;
    small = (int) (current_string[3]) - 48;
    
    float val_pH = (big) + (small * 0.1);
    Serial.println(val_pH);

    current_string = "";
    subsystem = "";

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

    current_string = "";
    subsystem = "";

  } else {
    Serial.println("OTHER");
  }

  delay(my_delay);

}

void requestEvent() {
  // Wire.write("T 257 "); // respond with message
  // Wire.write("P 57  ");

  


  int stirring_sensor_val = analogRead(A0);

  if (stirring_sensor_val > stirring_high_threshold && stirring_start_time == 0) {
    stirring_start_time = micros();
  }

  if (stirring_sensor_val < stirring_low_threshold && stirring_start_time != 0) {
    stirring_period = micros() - stirring_start_time;
    stirring_pulse_count++;
    stirring_start_time = 0; // Reset stirring_start_time for the next pulse
  }

  if (millis() % 1000 == 0) {
    float stirring_freq = 1000000.0 / stirring_period;
    float stirring_RPS = stirring_freq / 2.0;
    float stirring_rad_per_min = 2 * PI * stirring_RPS * 60.0;

    // Serial.print("stirring_freq: ");
    // Serial.print(stirring_freq);
    // Serial.print("Hz, RPS:");
    // Serial.print(stirring_RPS);
    // Serial.print(", Stirring Speed: ");
    // Serial.print(stirring_rad_per_min);
    // Serial.println(" rad/min");

    stirring_RPM = (int) (stirring_RPS * 60);
    to_send = "R ";

    if (stirring_RPM <= 999) {
      to_send += 0;
    }
    if (stirring_RPM <= 99) {
      to_send += 0;
    }
    if (stirring_RPM <= 9) {
      to_send += 0;
    }

    to_send += stirring_RPM;
    Serial.println(to_send);

    if (to_send != "ABCDEF") {
      for (int i = 0; i < 7; i++) {
        Wire.write(to_send[i]);
      }
    }

    to_send = "ABCDEF";

    stirring_pulse_count = 0;
    stirring_period = 0; // Reset stirring_period for the next calculation
  }
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

  // Serial.println("HELLO");
  Serial.println(current_string);

}
