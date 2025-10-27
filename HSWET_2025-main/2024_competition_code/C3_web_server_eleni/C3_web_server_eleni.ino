#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "index_html.h"
#include <math.h>

#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();


// ----------------- Set up the web server -----------------
/* How to configure your mobile hotspot
1. Look up "mobile hotspot" on your Window laptop's search bar
2. Set the "band" to 2.4 GHz
3. Name the ssid and password to something that matches the Arduino code here
4. Turn on the mobile hotspot every time you want to use the web server.
*/
const char *ssid = "Eleni";
const char *password = "12345678";

// -------- [FEEL FREE TO CHANGE] Set up the pins ----------
// Digital output state indicators
const int PIN_LED_STOP = 18;   // [GOOD] Emergency Stop
const int PIN_LED_RUN = 19;    // [GOOD] Experiment start light

//ELENI
//*******************************************
const int PIN_ENCODER = 7;
const int PIN_ACTUATOR = 6;
int dataLoggingInterval = 500;
float rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2*48;
volatile int counter = 0;
// in mA
int current = 0;
// in V
float voltage = 0;

//rated power stuff
bool rated_power = false;
int maxRpm = 1000;
int minPitch = 1250; //the pitch that we had last time we pitched belowe 11.3 m/s
int RPM_MARGIN_HIGH = 10; // smaller means better score but more potential for error/slow adjustment
int RPM_MARGIN_LOW = 10;
int PITCH_INCREMENT_DOWN = 2;
int PITCH_INCREMENT_UP = 1; 
int currentPitchDeviation = 0; 
// *********************************************

// This could be smaller if possible (for web page storage)
const int buffer_size = 2048; 

// ----------------- Set up variables -----------------

// initialize booleans and integer variables
bool STOPPED = false, RUNNING = false;

float load_current_setpoint = 0;
float load_current_measured = 0;

int pitch = 1630;

int StateSafety = 0, StateBackup = 0;
long SensorUpdate = 0;

// the XML array size needs to be bigger that your maximum expected size. 
// 2048 is way too big for this example


// ------------ Web server starts here -----------------
WebServer server(80);

// Set your Static IP address
IPAddress local_IP(192, 168, 137, 204);
// Set your Gateway IP address
IPAddress gateway(192, 168, 137, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

void setup(void) {
  Serial.begin(115200);

  // Set up all the pins
  // ELENI
  pinMode(PIN_ENCODER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER), updateEncoder, CHANGE);
  pinMode(PIN_ACTUATOR, OUTPUT);
  linActNewPos(pitch);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    //while (1);
  }

  // Might not need this
  disableCore0WDT();
  // disableCore1WDT();

  // ------ Wifi conncecion status is displayed in serial monitor ---
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  // ------ Attach functions to their tags --------------
  server.on("/", HandleRoot); // this sends the website
  server.on("/xml", SendXML);

  server.on("/BUTTON_STOP", ProcessStopButton);
  server.on("/BUTTON_RUN", ProcessRunButton);

  server.on("/PWR_OPTI", SetPwrOptiMode);
  server.on("/DURABILITY", SetDurabilityMode);
  server.on("/RATED_PWR", SetRatedPwrMode);

  server.on("/UPDATE_LOAD_CURRENT", SendLoadCurrent);
  server.on("/UPDATE_PITCH", UpdatePitch);

  // ----------- Error handling + start server ------------
  server.onNotFound(HandleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// -------------- Main loop -----------------
void loop(void) {
  if ((millis() - SensorUpdate) >= dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-SensorUpdate) / eventsPerRevolution;

    voltage = getVoltage() / 1000.0;
    current = getCurrent();
    Serial.print("Voltage: ");
    Serial.print(voltage);
    Serial.print(", RPMs: ");
    Serial.print(rpm);   
    Serial.print(", Current: ");
    Serial.println(current); 

    counter=0;
    SensorUpdate = millis();
    
    if (rated_power == true) {
      if (rpm > maxRpm + RPM_MARGIN_HIGH) {
        currentPitchDeviation += max(PITCH_INCREMENT_DOWN, int(abs(rpm-maxRpm)/25.0));
        pitchNewDeviation();
        Serial.print("RPMs are too high, increased pitch to: ");
        Serial.println(minPitch + currentPitchDeviation);
      }
      if (rpm < maxRpm - RPM_MARGIN_LOW) {
        currentPitchDeviation -= max(PITCH_INCREMENT_UP, int(abs(rpm-maxRpm)/30.0));
        pitchNewDeviation();
        Serial.print("RPMs are too low, decreased pitch to: ");
        Serial.println(minPitch + currentPitchDeviation);
      }
    }
  }
  server.handleClient();
}

// Sends the webpage
void HandleRoot() {
  Serial.println("Handling root");
  server.send(200, "text/html", PAGE_MAIN);
}

// Hanldes file not found error
void HandleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Stop button toggles
void ProcessStopButton() {
  STOPPED = !STOPPED;
  digitalWrite(PIN_LED_STOP, STOPPED);
  Serial.print("Button 0 "); Serial.println(STOPPED);
  server.send(200, "text/plain", ""); 
}

// Run button toggles
void ProcessRunButton() {
  Serial.println("Exp run button press");
  RUNNING = !RUNNING;
  digitalWrite(PIN_LED_RUN, RUNNING);
  Serial.print("Experiment Run Status: "); Serial.println(RUNNING);
  server.send(200, "text/plain", ""); 
}

// Set power optimization mode high
void SetPwrOptiMode() {
  Serial.println("Set to power optimization mode");
  rated_power = false;
  server.send(200, "text/plain", ""); 
}

// Set rated power mode high
void SetRatedPwrMode() {
  Serial.println("Set to rated power mode");
  rated_power = true;
  server.send(200, "text/plain", ""); 
}

// Set durability mode high
void SetDurabilityMode() {
  Serial.println("Set to durability mode");
  rated_power = false;
  server.send(200, "text/plain", ""); 
}

// [TODO] [INTERFACE WITH E-LOAD CARD ADC]
void SendLoadCurrent() {
  Serial.println("Sending new load current: ");
  String input_str = server.arg("VALUE"); // This grabs the new current value
  load_current_setpoint = input_str.toInt() / 1000.0;
  Serial.print("Update E-Load Current: "); Serial.println(load_current_setpoint);

  server.send(200, "text/plain", ""); 
}

void UpdatePitch() {
  Serial.println("Sending new pitch: ");
  String input_str = server.arg("VALUE"); // This grabs the new current value
  pitch = input_str.toInt();
  Serial.print("Updated Pitch to "); Serial.println(pitch);
  
  linActNewPos(pitch);

  server.send(200, "text/plain", ""); 
}
/*
void getMeasuredCurrentFromELoad() {
  // [TODO]: talk to the E-load to get real-time measured current/voltage value
  Serial.println("THIS IS A STUB.");
}*/

// Send the XML data to HTML in index_html.h. Encloses the data in XML tags
void SendXML() {
  char XML[buffer_size];
  char buf[60];

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");
  // send Wind Speed (float)
  sprintf(buf, "<wind-speed>%d.%d</wind-speed>\n", (int) (voltage), abs((int) (voltage * 10)  - ((int) (voltage) * 10)));
  strcat(XML, buf);
  // send RPM (float)
  sprintf(buf, "<rpm>%d.%d</rpm>\n", (int) (rpm), abs((int) (rpm * 10)  - ((int) (rpm) * 10)));
  strcat(XML, buf);

  // send POWER (float)
  sprintf(buf, "<pwr>%d.%d</pwr>\n", (int) (current), abs((int) (current * 10)  - ((int) (current) * 10)));
  strcat(XML, buf);

  // [TODO] Send E-Load current measured (float)
  sprintf(buf, "<load-current-measured>%d</load-current-measured>\n", pitch);
  strcat(XML, buf);

  //sprintf(buf, "<pitch-measured>%d</pitch-current-measured>\n", pitch);
  //strcat(XML, buf);

  // Send STOP button status
  if (STOPPED) {
    strcat(XML, "<STOP>1</STOP>\n");
  }
  else {
    strcat(XML, "<STOP>0</STOP>\n");
  }

  // Send EXP button status
  if (RUNNING) {
    strcat(XML, "<RUN>1</RUN>\n");
  }
  else {
    strcat(XML, "<RUN>0</RUN>\n");
  }

  // Send safety state
  sprintf(buf, "<SAFETY>%d</SAFETY>\n", StateSafety);
  strcat(XML, buf);

  // Send backup state
  sprintf(buf, "<BACKUP>%d</BACKUP>\n", StateBackup);
  strcat(XML, buf);

  strcat(XML, "</Data>\n");
  server.send(200, "text/xml", XML);
}

// Hardware functions

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
  long timer = 0;
  boolean state = 1;
  for (int i = 0; i < 100000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state = !state;
      digitalWrite(PIN_ACTUATOR, state);
    }
  }
  digitalWrite(PIN_ACTUATOR, LOW);
}

void pitchNewDeviation() {
  if (currentPitchDeviation < 0) {
    currentPitchDeviation = 0;
  }
  if (minPitch + currentPitchDeviation > 1500) {
    currentPitchDeviation = 1500-minPitch;
  }
  linActNewPos(minPitch + currentPitchDeviation);
  pitch = minPitch + currentPitchDeviation;
}

int getCurrent() {
  double I = 0;
  double samples = 4;
  double samples2 = 25;
  for (int i = 0; i < samples2; i++) {
    for (int i = 0; i < samples; i++) {
      I += ina260.readCurrent();
    } 
  }
  return (int)(I / samples / samples2);
}

int getVoltage() {
  double V = 0;
  double samples = 4;
  double samples2 = 50;
  for (int i = 0; i < samples2; i++) {
    for (int i = 0; i < samples; i++) {
      V += ina260.readBusVoltage(); 
    } 
  }
  return (int)(V / samples / samples2);
}