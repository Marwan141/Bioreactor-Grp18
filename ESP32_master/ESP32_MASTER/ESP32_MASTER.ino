#include <WiFi.h>             // WiFi control for ESP32
#include <ThingsBoard.h>      // ThingsBoard SDK
#include "esp_wpa2.h"   //wpa2 library for connections to Enterprise networks
#include <Wire.h>

//#define EAP_IDENTITY "insert your user name@ucl.ac.uk here"                
//#define EAP_PASSWORD "insert your password here"
//const char* ssid = "eduroam";

// WiFi
//
#define WIFI_AP_NAME        "Raghavs-iPhone"
#define WIFI_PASSWORD       "Raghav123"

// Helper macro to calculate array size
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define TOKEN               "rcnig1c5ekp8xk2oq5zi"
#define THINGSBOARD_SERVER  "thingsboard.cloud"

#define SLAVE_ADDR 12
#define MASTER_ADDR 11
#define I2C_SDA 21
#define I2C_SCL 22

WiFiClient espClient;           // Initialize ThingsBoard client
ThingsBoard tb(espClient);      // Initialize ThingsBoard instance
int status = WL_IDLE_STATUS;    // the Wifi radio's status

int quant = 1;                  // Main application loop delay
int updateDelay = 1000;        // Initial update delay.
int lastUpdate  = 0;            // Time of last update.
bool subscribed = false;        // Set to true if application is subscribed for the RPC messages.

float temperature = 27.5;
float pH = 5;
int RPM = 750;

float val_temperature = temperature;
float val_pH = pH;
int val_RPM = RPM;


// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_pH(const RPC_Data &data) {
  Serial.print("set pH: ");

  // Process data
  updateDelay = data;

  // Serial.print("Set new delay: ");
  // Serial.println(updateDelay);
  int big = (int) updateDelay / 10;
  int small = (int) updateDelay % 10;

  Serial.print(big);
  Serial.print('.');
  Serial.print(small);

  Serial.println(' ');

  pH = big + (small * 0.1);
  // P SP 50 SP SP
  // String to_send = "P ";
  Serial.println("HI");
  Serial.println(big);
  Serial.println(small);
  char smth1 = (char) (big);
  char smth2 = (char) (small);
  Serial.println(smth1);
  Serial.println(smth2);


  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write("T 257 ");
  Wire.endTransmission();

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_pH(const RPC_Data &data) {
  Serial.println("get pH");

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_Temp(const RPC_Data &data) {
  Serial.print("set Temp: ");

  // Process data
  updateDelay = data;

  // Serial.print("Set new delay: ");
  // Serial.println(updateDelay);
  int big = (int) updateDelay / 10;
  int small = (int) updateDelay % 10;

  Serial.print(big);
  Serial.print('.');
  Serial.print(small);

  Serial.println(' ');

  temperature = big + (small * 0.1);

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_Temp(const RPC_Data &data) {
  Serial.println("get Temp");

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "setValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response set_RPM(const RPC_Data &data) {
  Serial.print("set RPM: ");

  // Process data
  updateDelay = data;

  RPM = updateDelay;

  // Serial.print("Set new delay: ");
  Serial.print(updateDelay);

  Serial.println(' ');

  return RPC_Response(NULL, updateDelay);
}

// Processes function for RPC call "getValue"
// RPC_Data is a JSON variant, that can be queried using operator[]
// See https://arduinojson.org/v5/api/jsonvariant/subscript/ for more details
RPC_Response get_RPM(const RPC_Data &data) {
  Serial.println("get RPM");

  return RPC_Response(NULL, updateDelay);
}


// RPC handlers
RPC_Callback callbacks[] = {
  { "getpHValue",         get_pH},
  { "setpHValue",         set_pH},
  { "getTempValue",         get_Temp},
  { "setTempValue",         set_Temp},
  { "getRPMValue",         get_RPM},
  { "setRPMValue",         set_RPM},
};


void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}





void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);
  WiFi.begin(WIFI_AP_NAME, WIFI_PASSWORD);
  InitWiFi();
}

void loop() {
  
  long now = millis();
  
  delay(quant);
  
  Wire.requestFrom(SLAVE_ADDR, 6); // Request 6 bytes from slave
  
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

  // Reconnect to WiFi, if needed
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
    return;
  }

  // Reconnect to ThingsBoard, if needed
  if (!tb.connected()) {
    subscribed = false;

    // Connect to the ThingsBoard
    // Serial.print("Connecting to: ");
    // Serial.print(THINGSBOARD_SERVER);
    // Serial.print(" with token ");
    // Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      // return;
    }
  }

  // Subscribe for RPC, if needed
  if (!subscribed) {
    // Serial.println("Subscribing for RPC...");

    // Perform a subscription. All consequent data processing will happen in
    // callbacks as denoted by callbacks[] array.
    if (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) {
      Serial.println("Failed to subscribe for RPC");
      // return;
    }

    // Serial.println("Subscribe done");
    subscribed = true;
  }

  if (now > lastUpdate + updateDelay) {
    // r = (float)random(1000)/1000.0;
    // loopCounter++;

    // Serial.print("Sending data...[");  
    // Serial.print(loopCounter);
    // Serial.print("]: ");
    // Serial.println(r);

    // temperature += 10.1;
    // pH += 0.1;
    // RPM += 10;
    
    // Uploads new telemetry to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 
    // for more details
    // tb.sendTelemetryInt("count", loopCounter);    
    // tb.sendTelemetryFloat("randomVal", r);
    temperature = val_temperature;
    pH = val_pH;
    RPM = val_RPM;
    tb.sendTelemetryFloat("temperature", temperature);
    tb.sendTelemetryFloat("pH", pH);
    tb.sendTelemetryInt("RPM", RPM);
    lastUpdate += updateDelay;
  }

  // Process messages
  tb.loop();

}