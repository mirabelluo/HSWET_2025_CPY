// Updated 2025-05-12
// Turbine sweep through r_loads and pitch positions

#include <Adafruit_INA260.h>
#include "resistor_lookup.h"

Adafruit_INA260 ina260 = Adafruit_INA260();

// [USER INPUT] Pitches and R-loads to sweep -------------------
int r_loads[] = {4, 4.5, 5, 5.5, 6, 7, 8, 9, 10, 15};  // Resistive loads (ohm values)
int positions[] = {1100, 1150, 1180, 1200, 1250, 1300, 1350, 1400, 1450, 1480};
// -------------------------------------------------------------

const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};
const uint16_t BAD_FET_MASK = 0b0000010010000; // excludes FETs 6 and 11 (47 and 12)

const int ENCODER_PIN = 2;
const int ACTUATOR_PIN = 5;
const int LOAD_PIN = 6;

const int dataLoggingInterval = 250;
const int settingHoldTime = 5000;
const float eventsPerRevolution = 2 * 48;

volatile int counter = 0;
double rpm = 0;

unsigned long previousDataTime = 0;
unsigned long previousSettingTime = 0;
bool skipFirstMeasurement = false;

int i = 0;  // pitch index
int j = 0;  // r_load index

bool testComplete = false;

struct SensorData {
  float voltage;
  float current;
  float power;
  float rpm;
  float pitch;
  float load_setting;
  float r_measured;
};

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(LOAD_PIN, OUTPUT);
  pinMode(ACTUATOR_PIN, OUTPUT);

  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // Initial resistance setup -----------------------------
  applyBestResistance(r_loads[j]);
  // ------------------------------------------------------

  linActNewPos(positions[i]);
  delay(settingHoldTime);
  previousDataTime = millis();
  previousSettingTime = millis();
}

void loop() {
  if (testComplete) return;

  unsigned long now = millis();

  if (now - previousDataTime >= dataLoggingInterval) {
    rpm = counter * 60000.0 / dataLoggingInterval / eventsPerRevolution;

    if (!skipFirstMeasurement) {
      SensorData data;
      data.voltage = ina260.readBusVoltage() / 1000.0;
      data.current = ina260.readCurrent() / 1000.0;
      data.power = ina260.readPower() / 1000.0;
      data.rpm = rpm;
      data.pitch = positions[i];
      data.load_setting = r_loads[j];
      data.r_measured = (data.current > 1e-6) ? (data.voltage / data.current) : -1;

      Serial.write((uint8_t*)&data, sizeof(data));

    } else {
      skipFirstMeasurement = false;
    }

    counter = 0;
    previousDataTime = now;
  }

  if (now - previousSettingTime >= settingHoldTime) {
    i++;
    if (i >= sizeof(positions) / sizeof(int)) {
      i = 0;
      j++;

      if (j >= sizeof(r_loads) / sizeof(int)) {
        SensorData data = {-1, -1, -1, -1, -1, -1, -1};
        Serial.write((uint8_t*)&data, sizeof(data));
        testComplete = true;
        return;
      }

      // THIS IS THE NEW RESISTANCE CODE. SEE HELPER FUNCTION WITH CTRL + F -------------
      applyBestResistance(r_loads[j]);
      // --------------------------------------------------------------------------------
    }

    linActNewPos(positions[i]);
    skipFirstMeasurement = true;
    previousSettingTime = millis();
  }
}

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
  long timer = 0;
  bool state = 1;
  for (int i = 0; i < 10000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state = !state;
      digitalWrite(ACTUATOR_PIN, state);
    }
  }
}

bool usesBadFET(uint16_t mask) {
  return (mask & BAD_FET_MASK) != 0;
}

void applyBitstring(uint16_t bitstring) {
  for (int i = 3; i < 16; ++i) {
    int index = i - 3;
    bool state = ((bitstring >> (15 - i)) & 0x01);
    if (index >= 0 && index < NUM_FETS)
      digitalWrite(FETs[index], state ? HIGH : LOW);
  }
}

// ----------------- NEW: VARIABLE LOAD ---------------------------
void applyBestResistance(float targetR) {
  float best_diff = 1e9;
  uint16_t best_mask = 0;

  for (int k = 0; k < 543; ++k) {
    if (usesBadFET(resistorLookup[k].mask)) continue;
    float diff = abs(targetR - resistorLookup[k].resistance);
    if (diff < best_diff) {
      best_diff = diff;
      best_mask = resistorLookup[k].mask;
    }
  }

  applyBitstring(best_mask);
  delay(100);
  // Serial.print("Set R_LOAD to ~");
  // Serial.print(targetR);
  // Serial.println(" ohms");
}
// ------------------ VARIABLE LOAD ENDS HERE ------------------

