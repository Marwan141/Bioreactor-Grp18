#include <Wire.h>

#define SLAVE_ADDR 12
#define MASTER_ADDR 11
// #define I2C_SDA smth
// #define I2C_SCL smth

int my_delay = 1000;
String current_string, subsystem;
#define SensorPin A3        // pH meter Analog output to Arduino Analog Input 0
#define PUMP1_PIN D6         // Ac pump control pin
#define PUMP2_PIN D9        // Al pump control pin
#define ArrayLenth 40       // Number of samples for averaging
#define objectivePH 10    // Target pH value
#define tolerance 1.0       // Tolerance for pH adjustment
#define pumpRunTime 1000    // Time to run the pump (in milliseconds)
#define mixDelay 5000       // Time to wait after running the pump (in milliseconds)

//pH Subsystem
int pHArray[ArrayLenth];    // Array to store pH sensor readings
int pHArrayIndex = 0;
unsigned long lastSampleTime = 0;
unsigned long lastPumpRunTime = 0;
bool isPumpRunning = false;
float pHValue = 0;
//Temp subSystem
const byte thermistorPin = A1, heaterPin = D7;
const byte tempReadPin = A0;
const float g = 0.0909, c = -22.2682;
float T, Tset, Taim, analogTempReading;
//Stirring Subsystem
const int stirring_low_threshold = 200;
const int stirring_high_threshold = 300;
unsigned long stirring_pulse_count = 0;
unsigned long stirring_start_time = 0;
unsigned long stirring_period = 0;
int counter2;
int pwmValue = 120; // PWM value for stirring speed control




int stirring_RPM = 0;
String to_send = "ABCDEF";

void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  pinMode(thermistorPin, INPUT);
  pinMode(tempReadPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  digitalWrite(PUMP1_PIN, LOW);  // Ensure Pump 1 is off
  digitalWrite(PUMP2_PIN, LOW);  // Ensure Pump 2 is off
  Serial.begin(9600);
  pinMode(10, OUTPUT);
  analogWrite(10, pwmValue); // Set initial PWM value
  Serial.println("Initialising system...");

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
    Tset = val_temperature;
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
    
  }

  delay(my_delay);
  

}

float adjustTemp(float analogReading) {
  T = (g * analogReading) + c;
  //Tset = digitalRead(tempReadPin);
  Taim = Tset - 1.8; // -2 from esp32 value to prevent overshoot.

  // BANG BANG CONTROL
  if (T < Taim) {
    Serial.println("Heater on");
    digitalWrite(heaterPin, HIGH);
  } else {
    Serial.println("Heater off");
    digitalWrite(heaterPin, LOW);
  }
  return T;
}

int adjustStirring(int stirringValue){

   if (stirringValue > stirring_high_threshold && stirring_start_time == 0) {
    stirring_start_time = micros();
  }

  if (stirringValue < stirring_low_threshold && stirring_start_time != 0) {
    stirring_period = micros() - stirring_start_time;
    stirring_pulse_count++;
    stirring_start_time = 0; // Reset stirring_start_time for the next pulse
  }

  if (millis() % 1000 == 0) {
    float frequency = 1000000.0 / stirring_period;
    float rps = frequency / 2.0;
    float radPerMin = 2 * PI * rps * 60.0;
    stirring_pulse_count = 0;
    stirring_period = 0; 
    stirring_RPM = (int) (rps * 60);
   
  }
  return stirring_RPM;
}

void requestEvent() {
  
  int stirringValue = analogRead(A0);
  analogTempReading = analogRead(thermistorPin);
  float currentTemp;
  pHControl();
  //Stirring System

  currentTemp = adjustTemp(analogTempReading);
  Serial.println(currentTemp);
  stirring_RPM = adjustStirring(stirringValue);
  Serial.println(stirring_RPM);
  Serial.println(pHValue);
  if (counter2 % 3 == 0){
  currentTemp = currentTemp * 10;
  currentTemp = (int) currentTemp;
  to_send = "T ";
  to_send += currentTemp;
  to_send += " ";
  for (int i; i<6; i++){
    Wire.write(to_send[i]);
  }
  Wire.write(to_send[6]);
  }
  else if (counter2 % 3 == 1){

  

  to_send = "R ";
  if (stirring_RPM >= 10000) {
    stirring_RPM = (int)stirring_RPM / 10;
  }
  // Use sprintf to format the RPM value into the to_send string
 


  // Use sprintf to format the RPM value into the to_send string
 
  if (stirring_RPM >= 1000) {
      to_send += stirring_RPM;
      if (to_send != "ABCDEF") {
        for (int i = 0; i < 6; i++) {
          Wire.write(to_send[i]);
        }
        Wire.write(to_send[6]);
      }
    } else if (stirring_RPM < 1000 && stirring_RPM >= 100) {
      to_send += "0";
      to_send += stirring_RPM;
      if (to_send != "ABCDEF") {
        for (int i = 0; i < 6; i++) {
          Wire.write(to_send[i]);
        }
        Wire.write(to_send[6]);
      }
    } else if (stirring_RPM < 100 && stirring_RPM >= 10) {
      to_send += 0;
      to_send += 0;
      to_send += stirring_RPM;
      if (to_send != "ABCDEF") {
        for (int i = 0; i < 6; i++) {
          Wire.write(to_send[i]);
        }
        Wire.write(to_send[6]);
      }
    } else {
      to_send += 0;
      to_send += 0;
      to_send += 0;
      to_send += stirring_RPM;
      if (to_send != "ABCDEF") {
        for (int i = 0; i < 6; i++) {
          Wire.write(to_send[i]);
        }
        Wire.write(to_send[6]);
      }
    }

   
  }
  else{
    to_send = "P ";
    to_send += (int)(pHValue * 10);
    to_send += "  ";
    for (int i = 0; i < 6; i++) {
          Wire.write(to_send[i]);
        }
        Wire.write(to_send[6]);
  }
  counter2++; 
  to_send = "ABCDEF";
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

  Serial.println(current_string);

}

void pHControl() {
  unsigned long currentTime = millis();
  // Take a new pH reading every 20 milliseconds
  if (!isPumpRunning) {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth) {
      pHArrayIndex = 0;
    }
    lastSampleTime = currentTime;
  }

  // Calculate average pH value
  if (!isPumpRunning) {
    float voltage = calculateAverage(pHArray, ArrayLenth) * 5.0 / 1024.0;
    Serial.println(voltage);
    pHValue = 2.1 * voltage + 1.2;  // Calibrated pH calculation

    // Adjust pH if needed
    if (pHValue > 0 && pHValue < objectivePH - tolerance) {
      // If pH is too low, add alkaline
      digitalWrite(PUMP2_PIN, HIGH);
      lastPumpRunTime = currentTime;
      isPumpRunning = true;
    } else if (pHValue > 0 && pHValue > objectivePH + tolerance) {
      // If pH is too high, add acid
      digitalWrite(PUMP1_PIN, HIGH);
      lastPumpRunTime = currentTime;
      isPumpRunning = true;
    }
  }

  // Turn off pump after running for the set duration
  if (isPumpRunning && currentTime - lastPumpRunTime >= pumpRunTime) {
    digitalWrite(PUMP1_PIN, LOW);
    digitalWrite(PUMP2_PIN, LOW);
    if (!(pHValue < 0)) {
    }
    isPumpRunning = false;
    lastSampleTime = currentTime + mixDelay; // Wait for mixing before next reading
  }
}

double calculateAverage(int arr[], int number) {
  long sum = 0;
  for (int i = 0; i < number; i++) {
    sum += arr[i];
  }
  return (double)sum / number;
}

void sendToSlave() {
  if (to_send != "ABCDEF") {
    Wire.beginTransmission(SLAVE_ADDR);
    for(int i; i<7; i++)
    Wire.write(to_send[i]);
    Wire.endTransmission();
  }
}