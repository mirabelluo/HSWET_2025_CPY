// Updated 2025-05-12
// Turbine sweep through r_loads and pitch positions
// Have to include resistoer_lookup.h in the Arduino IDE
#include <Adafruit_INA260.h>
#include "resistor_lookup.h"

Adafruit_INA260 ina260 = Adafruit_INA260();

// [USER INPUT] Pitches and R-loads to sweep -------------------

// Resistive loads (ohm values)
int r_loads[] = {2, 5, 10, 15, 20, 25, 30};  
// Pitch positions (unit-less, full range is 1100 - 1500)
int positions[] = {1100, 1150, 1180, 1200, 1250, 1300, 1350, 1400, 1450}; 

// -------------------------------------------------------------


// ------------------------ CONST DEC ----------------------------
const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};

const int ENCODER_PIN = 2;
const int ACTUATOR_PIN = 5;
const int LOAD_PIN = 25;
const int SAFETY_BUTTON = 6;
const int WALL_CONNECT = 7;

const int featheredPitch = 1500;
const int minPitch = 1100;

const int dataLoggingInterval = 250;
const int settingHoldTime = 5000;
const float eventsPerRevolution = 2 * 48;
// -------------------------------------------------------------

// --------------------- GLOBAL VAR DEC --------------------------
volatile int counter = 0;
double rpm = 0;

unsigned long previousDataTime = 0;
unsigned long previousSettingTime = 0;
bool skipFirstMeasurement = false;

// safety
int state_emergency = 0;
int previous_emergency = 0;

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
// --------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(LOAD_PIN, OUTPUT);
  pinMode(SAFETY_BUTTON, INPUT_PULLUP);
  pinMode(ACTUATOR_PIN, OUTPUT);
  pinMode(WALL_CONNECT, OUTPUT);

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

  // ------------------- SAFETY ------------------------------------
  int safetySignal = digitalRead(SAFETY_BUTTON); // read safety pin
  if ((safetySignal) == HIGH) {
    state_emergency = 0; // everything is connect, proceed
  } else {
    state_emergency = 1; // button has been pressed 
  }

  if (state_emergency == 0 && previous_emergency != 0) {
    Serial.println("Restart tasks");
    linActNewPos(minPitch);
    previous_emergency = state_emergency;
  } else if ( state_emergency == 1 && previous_emergency != 1) {
    Serial.println("Brake tasks");
    linActNewPos(featheredPitch);
    previous_emergency = state_emergency;
  }

  setLoadControl(counter, safetySignal);
  // ----------------------------------------------------------------

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

<<<<<<< Updated upstream:06_final_code/step02_CWC_ctrl_box/step02_CWC_ctrl_box.ino
=======

// ----------------- NEW: VARIABLE LOAD ---------------------------

bool usesBadFET(uint16_t mask) {
  return (mask & BAD_FET_MASK) != 0;
}
>>>>>>> Stashed changes:06_final_code/step02_CWC_ctrl_box/step01_sweep_pitches_w_rload.ino

void applyBitstring(uint16_t bitstring) {
  for (int i = 3; i < 16; ++i) {
    int index = i - 3;
    bool state = ((bitstring >> (15 - i)) & 0x01);
    if (index >= 0 && index < NUM_FETS)
      digitalWrite(FETs[index], state ? HIGH : LOW);
  }
}

<<<<<<< Updated upstream:06_final_code/step02_CWC_ctrl_box/step02_CWC_ctrl_box.ino
// ----------------- [BEGIN] NEW VARIABLE LOAD ---------------------------
=======
>>>>>>> Stashed changes:06_final_code/step02_CWC_ctrl_box/step01_sweep_pitches_w_rload.ino
void applyBestResistance(float targetR) {
  float best_diff = 1e9;
  uint16_t best_mask = 0;

  for (int k = 0; k < 543; ++k) {
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
// ------------------ [END] VARIABLE LOAD ------------------

// ------------------ NEW: SAFEETY -----------------------------

void disconnectLoad() {
  digitalWrite(LOAD_PIN, LOW);
  delay(1000);
  Serial.print("Residual current: ");
  Serial.println(ina260.readCurrent() / 1000.0);
}

void setLoadControl(int counter, int safety_sig) {
  bool RPM;

  //if (ina260.readBusVoltage()/1000.0 > 3) {
  // RPM THRESHOLD - TO CHANGE
  if (counter >= 50) {
    RPM = 1;
  } else {
    RPM = 0;
  }

  if (RPM & safety_sig) {
    digitalWrite(WALL_CONNECT, LOW);
    digitalWrite(LOAD_PIN, HIGH);
  } else {
    digitalWrite(WALL_CONNECT, HIGH);
    disconnectLoad();
  }
}
// -----------------------------------------------------------
