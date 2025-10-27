// Updated 03/06/2025
// Turbine sweep pitches for test on 03/06/2025

// ]INPUT] Pitches to sweep through: 1100 - 1500 possible
int positions[] = {1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1480};

#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 250;
int settingLoggingInterval = 5000;
int pitchPositionHoldTime = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// Encoder variables
int ENCODER_PIN = 2;
double rpm = 0;
double eventsPerRevolution = 2 * 48;
volatile int counter = 0;

int RPM_THRESHOLD = 1000;
int ACTUATOR_PIN = 5;
const int LOAD_PIN = 6;

int i = 0;

const int load_increment = 5;
const int load_start = 0;
const int load_stop = 1;

int load_setting = load_start;
bool skipFirstMeasurement = false;  // Flag to skip the first RPM reading after pitch change

struct SensorData {
  float voltage;
  float current;
  float power;
  float rpm;
  float pitch;
  float load_setting;
};

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(LOAD_PIN, OUTPUT);
  analogWrite(LOAD_PIN, load_setting);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // Linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  linActNewPos(positions[i]);
  delay(pitchPositionHoldTime);
  previousSettingTime = millis();
}

void loop() {
  currentTime = millis();

  if (millis() - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / dataLoggingInterval / eventsPerRevolution;
    
    if (!skipFirstMeasurement) {  // Only log valid measurements
      SensorData data;
      data.voltage = ina260.readBusVoltage() / 1000.0;
      data.current = ina260.readCurrent() / 1000.0;
      data.power = ina260.readPower() / 1000.0;
      data.rpm = rpm;
      data.pitch = positions[i];
      data.load_setting = load_setting;

      Serial.write((uint8_t*)&data, sizeof(data));
    } else {
      skipFirstMeasurement = false;  // Allow normal logging from the next period
    }

    counter = 0;
    previousTime = millis();
  }

  if (millis() - previousSettingTime > settingLoggingInterval) {
    load_setting += load_increment;
    if (load_setting > load_stop) {
      load_setting = load_start;
      i++;

      if (i == sizeof(positions) / sizeof(int)) {
        SensorData data;
        data.voltage = -1;
        data.current = -1;
        data.power = -1;
        data.rpm = -1;
        data.pitch = -1;
        data.load_setting = -1;
        Serial.write((uint8_t*)&data, sizeof(data));
      } else {
        linActNewPos(positions[i]);
        delay(5000);
        skipFirstMeasurement = true;  // Skip the first RPM reading after a pitch change
      }
    }
    analogWrite(LOAD_PIN, load_setting);
    previousSettingTime = millis();
  }
}

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
  long timer = 0;
  boolean state = 1;
  for (int i = 0; i < 10000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state = !state;
      digitalWrite(ACTUATOR_PIN, state);
    }
  }
}
