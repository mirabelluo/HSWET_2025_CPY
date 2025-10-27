/**
 * @file sweep_pitches.ino
 * @brief Wind turbine pitch testing program for the Hopkins Student Wind Energy Team.
 *
 * This Arduino program is designed to test and log power output of a wind turbine 
 * at various pitch angles. It uses an INA260 power sensor to measure current and voltage, 
 * an encoder to calculate RPM, and a linear actuator to adjust blade pitch. The data is 
 * logged through the serial interface for analysis.
 *
 * ## Features
 * - Initializes and reads from an Adafruit INA260 power sensor for voltage and current.
 * - Reads RPM from an encoder to track turbine rotational speed.
 * - Controls a linear actuator to adjust turbine blade pitch at predefined positions.
 * - Sends data logs (voltage, current, RPM, pitch angle) as a binary struct via serial communication.
 * - Automatically cycles through specified pitch positions and indicates test completion.
 *
 * ## Data Logging
 * The program records:
 * - Voltage (V)
 * - Current (A)
 * - RPM
 * - Pitch angle (microseconds for actuator signal)
 * - Load setting (reserved for future use)
 *
 * ## Setup
 * - INA260 sensor for power measurement.
 * - Encoder connected to `ENCODER_PIN` for RPM calculation.
 * - Linear actuator controlled by PWM on `ACTUATOR_PIN`.
 * - Relays for normal and safety control.
 *
 * ## Author
 * Eleni Daskopoulou, last updated 05/07/2024
 *
 * ## Connections
 * - INA260: I2C pins (SDA, SCL)
 * - Encoder: Digital pin 7 (`ENCODER_PIN`)
 * - Actuator: Digital pin 6 (`ACTUATOR_PIN`)
 * - Relays: Digital pins 2 and 3 (`RELAY_SAFETY` and `RELAY_NORMAL`)
 *
 * ## Usage
 * - Adjust `positions[]` array to specify desired blade pitch positions.
 * - Modify `dataLoggingInterval` and `settingLoggingInterval` for data collection frequency.
 * - Upload and monitor serial output for collected data.
 */

#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 500;
int settingLoggingInterval = 10000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// encoder variables
int ENCODER_PIN = 7;
double rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2*48;
volatile int counter = 0;

int ACTUATOR_PIN = 6;

int RELAY_NORMAL = 3;
int RELAY_SAFETY = 2;
int relay_delay = 50;

int i = 0;

int positions[] = {1200, 1250, 1300, 1350, 1400, 1650};

// int positions[] = {1200, 1450};

struct SensorData {
  float voltage;
  float current;
  float rpm;
  float pitch;
  float load_setting;
};

void setup() {
  Serial.begin(115200);
  
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_SAFETY, LOW);
  digitalWrite(RELAY_NORMAL, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_NORMAL, LOW);

  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  // range is 1100(all the way in)-1500(all the way out)
  linActNewPos(positions[i]);
  delay(7000);
  //linActNewPos(1100);
  while (!Serial) { delay(10); }
  previousSettingTime = millis();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  currentTime = millis();

  if (millis() - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    SensorData data;
    
    data.voltage = getVoltage() / 1000.0;
    
    data.current = getCurrent() / 1000.0;
    data.rpm = rpm;
    data.pitch = positions[i];
    data.load_setting = 0.0;

    // Sending the struct through serial
    Serial.write((uint8_t*)&data, sizeof(data));
    Serial.print("Voltage: ");
    Serial.print(data.voltage);
    Serial.print(", Current: ");
    Serial.print(data.current);
    Serial.print(", RPM: ");
    Serial.print(data.rpm);
    Serial.print(", Pitch: ");
    Serial.println(data.pitch);
    
    counter = 0;
    previousTime = millis();
  }
  
  if (millis() - previousSettingTime > settingLoggingInterval) {
    i++;
    // check if done with all pitch values
    if (i == sizeof(positions)/sizeof(int)) {
      // done with current windspeed
      // set all values to negative to indicate being done
      SensorData data;
      data.voltage = -1;
      data.current = -1;
      data.rpm = -1;
      data.pitch = -1;
      data.load_setting = -1;
  
      // Sending the struct through serial
      Serial.write((uint8_t*)&data, sizeof(data));
    }
    linActNewPos(positions[i]);
    delay(7000);
    previousSettingTime = millis();
  }
}

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
      digitalWrite(ACTUATOR_PIN, state);
    }
  }
  digitalWrite(ACTUATOR_PIN, LOW);
}

int getCurrent() {
  double I = 0;
  double samples = 4;
  double samples2 = 25;
  for (int i = 0; i < samples2; i++) {
    for (int i = 0; i < samples; i++) {
      ina260.readCurrent();
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
