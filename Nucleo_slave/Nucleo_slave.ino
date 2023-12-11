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

const float Kv=800;
const float Too=0.25;

const float wn=4;
const float zeta=1;
int val_RPM;
// Calculate the required PI controller parameters
const float wo=1/Too;
// corresponding motor response frequency in rad/s
const float Kp=(2*zeta*wn/wo-1)/Kv;
const float KI=wn*wn/Kv/wo;
//And finally we need to declare the remaining constants and variables:
const byte encoderpin=A0, motorpin=D10;
float setspeed; // Desired (set) motor speed
long currtime, prevtime, pulseT, prevpulseT, prevprevpulseT, T1, T2;
float measspeed, meanmeasspeed, freq, error, KIinterror, deltaT;
bool ctrl;
int Vmotor, onoff;

//pH Subsystem
int pHArray[ArrayLenth];    // Array to store pH sensor readings
int pHArrayIndex = 0;
unsigned long lastSampleTime = 0;
unsigned long lastPumpRunTime = 0;
bool isPumpRunning = false;
float pHValue = 0;
//Temp subSystem
const byte thermistorPin = A1, heaterPin = D7;
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





void setup() {
  //Serial.begin(9600);
  Wire.begin(SLAVE_ADDR); // join I2C bus with slave address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  pinMode(thermistorPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  digitalWrite(PUMP1_PIN, LOW);  // Ensure Pump 1 is off
  digitalWrite(PUMP2_PIN, LOW);  // Ensure Pump 2 is off
  Serial.begin(9600);

    // put your setup code here, to run once:
   pinMode(encoderpin, INPUT);
  // One of the interrupt pins (D2/D3 on an Uno/Nano)
   pinMode(motorpin, OUTPUT);
  // Motor PWM control signal, to MOSFET
  // ! connected to button, get ride and change setspeed to be based off inp from esp32
  // Attach push-switch to ground, to cycle speeds
   attachInterrupt(digitalPinToInterrupt(encoderpin), freqcount, CHANGE);
   analogWriteResolution(10); // 10-bit PWM -TCCR1A = 0b00000011; for Uno/Nano
   analogWriteFrequency(8000); //8 kHz PWM -TCCR1B = 0b00000001; for Uno/Nano
  analogWrite(motorpin, 0);
  Serial.println("Initialising system...");
  //delay(10000); TRY WITH AND WITHOUT???????

}
void freqcount() {
  pulseT= micros();
  if(pulseT-prevpulseT>6000){
    // Attempt to mitigate sensor false triggers due to PWM current spikes, apparent speeds > 2500 RPM
    freq= 1e6/float(pulseT-prevprevpulseT);
    }
  // Calculate speed sensor frequency
  prevprevpulseT= prevpulseT;
  prevpulseT= pulseT;
 }



int stirring_RPM = 0;
String to_send = "ABCDEF";

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
    
    val_RPM = (one * 1000) + (two * 100) + (three * 10) + (four);
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

   // put your main code here, to run repeatedly:
  currtime = micros();
  deltaT = (currtime-prevtime)*1e-6;
  if(ctrl==1 && currtime-T2>1000000) {ctrl=0;}
  if (ctrl==0 && digitalRead(A2) == 0) {onoff++; ctrl=1; T2=currtime; setspeed=val_RPM/10;} // ! Based off ESP input1!!!
  if (currtime-T1 > 0) {
    prevtime = currtime;
    T1 = T1+10000;
    measspeed = freq*30;
    if (currtime-pulseT>5e5) {measspeed=0; meanmeasspeed=0;}
    error = setspeed-measspeed;
    KIinterror = KIinterror+KI*error*deltaT;
    KIinterror = constrain(KIinterror,0,3);
    Vmotor = round(204*(Kp*error+KIinterror));
    Vmotor = constrain(Vmotor, 0, 1500); // constrain to 600
    Serial.print("Vmotor: "); Serial.println(Vmotor);
    analogWrite(motorpin, Vmotor);
    meanmeasspeed = 0.1*measspeed+0.9*meanmeasspeed;
    //Serial.print(0); Serial.print(","); Serial.print(1500); Serial.print(",");
    Serial.print("meanmeasspeed: "); Serial.println(meanmeasspeed);
    return meanmeasspeed;
}
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