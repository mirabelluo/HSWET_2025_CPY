#include "r_target.h"
#include "resistor_lookup.h"

#include <Arduino.h>
#include <Adafruit_INA260.h>

Adafruit_INA260 ina260 = Adafruit_INA260();

const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};

// === Helper to apply bitmask to FETs ===
void applyBitstring(uint16_t bitstring) {
  for (int i = 3; i < 16; ++i) {
    int index = i - 3;
    bool state = ((bitstring >> (15 - i)) & 0x01);
    if (index >= 0 && index < NUM_FETS)
      digitalWrite(FETs[index], state ? HIGH : LOW);
  }
}

// === INA260 sensor setup ===
void initialize() {
  bool found = ina260.begin(0x40);  // default address
  if (!found) found = ina260.begin(0x41);  // secondary address
  if (!found) {
    Serial.println("Couldn't find INA260 chip at 0x40 or 0x41");
    while (1);  // Halt system
  }
  Serial.println("INA260 initialized.");
}

// === Set an initial resistance ===
void initialR(int setR) {
  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  // Find closest match
  float best_diff = 1e9;
  uint16_t best_mask = 0;
  for (int k = 0; k < 543; ++k) {
    float diff = abs(setR - resistorLookup[k].resistance);
    if (diff < best_diff) {
      best_diff = diff;
      best_mask = resistorLookup[k].mask;
    }
  }

  applyBitstring(best_mask);
}

// === Estimate internal resistance based on dV/dI ===
float calculate_targetRes(int interval, int r_old) {
  float v1 = ina260.readBusVoltage();
  float i1 = ina260.readCurrent();
  delay(interval);
  float v2 = ina260.readBusVoltage();
  float i2 = ina260.readCurrent();

  if (fabs(i2 - i1) < 0.001 || fabs(v2 - v1) < 0.001)
    return r_old;

  return fabs((v2 - v1) / (i2 - i1));
}

// === Gradual update to resistance value ===
float updateResistance(float r_current, float r_target, float K) {
  return r_current + K * (r_target - r_current);
}

// === Dynamically adjust resistance until close to target ===
void setResistance(float targetR, int interval, int settlecount, float k) {
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  float setR = targetR;
  int count = 0;
  int attempts = 0;
  const int maxAttempts = 50;

  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  while (attempts++ < maxAttempts) {
    // Find best resistor match
    float best_diff = 1e9;
    uint16_t best_mask = 0;
    for (int k = 0; k < 543; ++k) {
      float diff = abs(setR - resistorLookup[k].resistance);
      if (diff < best_diff) {
        best_diff = diff;
        best_mask = resistorLookup[k].mask;
      }
    }

    applyBitstring(best_mask);

    float mV = ina260.readBusVoltage() / 1000.0;
    float mI = ina260.readCurrent() / 1000.0;

    // Sanity checks
    if (mV < 0.01 || mI < 0.001 || mV > 100 || mI > 100) {
      Serial.println("Sensor readings invalid — skipping.");
      delay(interval);
      continue;
    }

    float actualR = fabs(mV / mI);
    Serial.print("Target R: "); Serial.print(targetR);
    Serial.print(" | Measured R: "); Serial.print(actualR);
    Serial.print(" | V: "); Serial.print(mV);
    Serial.print(" | I: "); Serial.println(mI);

    if (abs(actualR - targetR) > 0.2) {
      setR = updateResistance(setR, targetR, k);
    } else {
      count++;
    }

    if (count >= settlecount) {
      Serial.println("Target resistance achieved.");
      return;
    }

    delay(interval);
  }

  Serial.println("Warning: Maximum attempts reached before settling.");
}

// === Turn off load ===
void disconnectLoad() {
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  delay(1000);
  Serial.print("Residual current: ");
  Serial.println(ina260.readCurrent() / 1000.0);
}
