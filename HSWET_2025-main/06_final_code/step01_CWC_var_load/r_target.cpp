#include "r_target.h"
#include "resistor_lookup.h"

#include <Arduino.h>
#include <Adafruit_INA260.h>

Adafruit_INA260 ina260 = Adafruit_INA260();

const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};
const uint16_t BAD_FET_MASK = 0b0000010010000; // excludes FETs 6 and 11 (47 and 12)

void applyBitstring(uint16_t bitstring) {
  for (int i = 3; i < 16; ++i) {
    int index = i - 3;
    bool state = ((bitstring >> (15 - i)) & 0x01);
    if (index >= 0 && index < NUM_FETS)
      digitalWrite(FETs[index], state ? HIGH : LOW);
  }
}

void initialize() {
  bool found = ina260.begin(0x40);
  if (!found) found = ina260.begin(0x41);
  if (!found) {
    Serial.println("Couldn't find INA260 chip at 0x40 or 0x41");
    while (1);
  }
  Serial.println("INA260 initialized.");
}

float readAverageVoltage(int samples = 5) {
  float sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += ina260.readBusVoltage();
    delay(5);
  }
  return sum / samples / 1000.0;
}

float readAverageCurrent(int samples = 5) {
  float sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += ina260.readCurrent();
    delay(5);
  }
  return sum / samples / 1000.0;
}

bool usesBadFET(uint16_t mask) {
  return (mask & BAD_FET_MASK) != 0;
}

void initialR(int setR) {
  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  float best_diff = 1e9;
  uint16_t best_mask = 0;
  for (int k = 0; k < 543; ++k) {
    if (usesBadFET(resistorLookup[k].mask)) continue;
    float diff = abs(setR - resistorLookup[k].resistance);
    if (diff < best_diff) {
      best_diff = diff;
      best_mask = resistorLookup[k].mask;

    }
  }

  applyBitstring(best_mask);
}

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

float updateResistance(float r_current, float r_target, float K) {
  return r_current + K * (r_target - r_current);
}

void setResistance(float targetR, int interval, int settlecount, float k) {
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  float setR = constrain(targetR, 3.0, 39.0);
  int count = 0;
  int attempts = 0;
  const int maxAttempts = 10;

  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  while (attempts++ < maxAttempts) {
    float best_diff = 1e9;
    uint16_t best_mask = 0;
    for (int k = 0; k < 543; ++k) {
      if (usesBadFET(resistorLookup[k].mask)) continue;
      float diff = abs(setR - resistorLookup[k].resistance);
      if (diff < best_diff) {
        best_diff = diff;
        best_mask = resistorLookup[k].mask;
      }
    }

    applyBitstring(best_mask);
    
    float mV = readAverageVoltage();
    float mI = readAverageCurrent();

    if (mV < 0.01 || mI < 0.001 || mV > 100 || mI > 100) {
      Serial.println("Sensor readings invalid — skipping.");
      delay(interval);
      continue;
    }

    float actualR = fabs(mV / mI);
    Serial.print("Target R: "); Serial.print(targetR);
    Serial.print(" | Measured R: "); Serial.print(actualR);
    Serial.print(" | V: "); Serial.print(mV);
    Serial.print(" | I: "); Serial.print(mI);
    Serial.print(" | FET bitstring: ");
    for (int b = NUM_FETS - 1; b >= 0; --b) {
      Serial.print((best_mask >> b) & 0x01);
    }
    Serial.println();

    if (abs(actualR - targetR) > 0.5) {
      setR = constrain(updateResistance(setR, targetR, k), 3.0, 39.0);
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

void disconnectLoad() {
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  delay(1000);
  Serial.print("Residual current: ");
  Serial.println(ina260.readCurrent() / 1000.0);
}


// NEW FUNCTION to test all resistors. Run this with the black variable power supply 
// (wall plug that John was using) to give some voltages (pretend this power supply is the turbine).
// Then it scanning through all 543 combinations and printing the target resistance, 
// measured resistance, error, and one-hot bitstring.
void testAllResistors(float tolerance = 0.5) {
  Serial.println("Index\tTarget_R\tMeasured_R\tError\tOne-hot Bitstring");

    for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }


  for (int i = 0; i < 543; ++i) {
    float targetR = resistorLookup[i].resistance;
    uint16_t mask = resistorLookup[i].mask;

    // Skip combinations that include bad FETs
    if (usesBadFET(mask)) continue;

    // Apply the bitstring
    applyBitstring(mask);
    delay(50);  // Allow switching to settle

    // Read voltage and current
    float V = readAverageVoltage(5);
    float I = readAverageCurrent(5);

    // Skip if invalid readings
    if (V < 0.01 || I < 0.001 || V > 100 || I > 100) {
      Serial.print(i); Serial.print("\t");
      Serial.print(targetR, 4); Serial.print("\t");
      Serial.print("INVALID\t1\t");
    } else {
      float measuredR = V / I;
      int flag = fabs(measuredR - targetR) > tolerance ? 1 : 0;

      Serial.print(i); Serial.print("\t");
      Serial.print(targetR, 4); Serial.print("\t");
      Serial.print(measuredR, 4); Serial.print("\t");
      Serial.print(flag); Serial.print("\t");
    }

    // Print 13-bit one-hot encoding
    for (int b = 12; b >= 0; --b) {
      Serial.print((mask >> b) & 0x01);
    }
    Serial.println();

    delay(100);  // Prevent rapid cycling
  }

  disconnectLoad();
}

// -------------------------- DO THIS IN ARDUINO IE --------------------------