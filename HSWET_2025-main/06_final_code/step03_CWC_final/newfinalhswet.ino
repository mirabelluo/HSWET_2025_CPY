// Updated 04/17/2025
// Turbine automated power collection code
// With control loop but no safety system yet
// Updated 5/14/2025
// its just mirabel code excpet no variable load and also constant pitch value

#include <Adafruit_INA260.h>
#include "resistor_lookup.h"
Adafruit_INA260 ina260 = Adafruit_INA260();
int r_loads[] = {2, 5, 10, 15, 20, 25, 30}; 
int dataLoggingInterval = 250;
int settingLoggingInterval = 5000;
int pitchPositionHoldTime = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;
int positions[] = {1100, 1150, 1180, 1200, 1250, 1300, 1350, 1400, 1450}; 
unsigned long previousDataTime = 0;
int i = 0;  // pitch index
int j = 0;  // r_load index
int state_emergency = 0;
int previous_emergency = 0;
const int settingHoldTime = 5000;

bool skipFirstMeasurement = false;

// [DON'T CHANGE] Encoder Variables for RPM measurements
const int ENCODER_PIN = 2;
const int ACTUATOR_PIN = 5;

// [TODO] R_LOAD and safety system stuff: not implemented yet
const int R_LOAD_PIN = 25; 
const int SAFETY_BUTTON = 6;
const int WALL_CONNECT = 7;


double rpm = 0;
double eventsPerRevolution = 2*48;
volatile int counter = 0;

const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};


// Rated power variables [TODO]
int maxRpm = 20; //this will be ~20 below the rpm that we were able to achive at the correct pitch at 11m/s

const int minPitch = 1150; //the selected power generation pitch
const int featheredPitch = 1150; //the pitch where blades are feathered

// [INPUT] RPM control loop variables. _HIGH and _LOW can be different, to be determined through experiments
// _HIGH means RPM is too high. the "10" changes the pitch accordingly.
// _LOW means RPM is too low. the "8" changes the pitch accordingly.
// Smaller values means more optimized power outputs, but more potential for error/slow adjustment
const int RPM_MARGIN_HIGH = 10; 
const int RPM_MARGIN_LOW = 8;

// [INPUT] [TODO] TEST THESE DURING TUNNEL TESTS
// These are probably fine. But the pitches changes by 2 and 1, respectively, for control loop
// After these look good, you should stop changing them.
const int PITCH_INCREMENT_DOWN = 2;
const int PITCH_INCREMENT_UP = 1; 

int currentPitchDeviation = 0;

// 0 for normal operation, 1 for emergency/stopped positions
int prev_state_emergency = 1;

int load_setting = 0;
int prev_backup = 1;

struct SensorData {
    float voltage;
    float current;
    float power;
    float rpm;
    float pitch;
    float load_setting; // need to add resistanceMeasured too
    float r_measured;
  };

// put your setup code here, to run once:
void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);
  
  pinMode(R_LOAD_PIN, OUTPUT);
  analogWrite(R_LOAD_PIN, load_setting);

  // Set up the INA260 power sensor chip
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  pinMode(SAFETY_BUTTON, INPUT_PULLUP);

  
  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);

  // start at feathered position
  brake_tasks();
    Serial.begin(115200);
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(R_LOAD_PIN, OUTPUT);
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
  //applyBestResistance(r_loads[j]);
  // ------------------------------------------------------

  linActNewPos(positions[1]);
  delay(settingHoldTime);
  previousDataTime = millis();
  previousSettingTime = millis();
}

void loop() {
    unsigned long now = millis();

  if (now - previousDataTime >= dataLoggingInterval) {
    rpm = counter * 60000.0 / dataLoggingInterval / eventsPerRevolution;

    if (!skipFirstMeasurement) {
      SensorData data;
      data.voltage = ina260.readBusVoltage() / 1000.0;
      data.current = ina260.readCurrent() / 1000.0;
      data.power = ina260.readPower() / 1000.0;
      data.rpm = rpm;
      data.pitch = positions[1];
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
        return;
      }

      // THIS IS THE NEW RESISTANCE CODE. SEE HELPER FUNCTION WITH CTRL + F -------------
      //applyBestResistance(r_loads[j]);
      // --------------------------------------------------------------------------------
    }

    linActNewPos(positions[1]);
    skipFirstMeasurement = true;
    previousSettingTime = millis();
  }
}


// ------------------------------------------------------------
// [HELPER FUNCTIONS] These are not to be changed unless you know what you're doing

// [NEW THIS YEAR] NEED TO ADD VARIABLE LOAD STUFF. LAST YEAR THERE"S NO VARIABLE LOAD.
// ------------------------------------------------------------


// -------------- INSERT ALL THE setR() function stuff here ----------- // ALSO ADD THE R_TARGET.CPP header files

void updateEncoder() {
  counter++;
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

void brake() {
  state_emergency = 1;
}


// [TODO] MIRABEL UPDATE THIS
void brake_tasks() {
  Serial.println("Brake tasks");
  linActNewPos(featheredPitch);
  delay(5000);

}

void restart() {
  state_emergency = 0;
}


// [TODO] MIRABEL UPDATE THIS FOR SAFETY ----------------------
void restart_tasks() {
    Serial.println("Restart tasks");
  linActNewPos(1150);
  delay(15000);
}
// ------------------------------------------------------------

void pitchNewDeviation(int deviation) {
  if (deviation < 0) {
    deviation = 0;
  }
  linActNewPos(1150);
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



// ----------------- NEW: VARIABLE LOAD ---------------------------



void applyBitstring(uint16_t bitstring) {
  for (int i = 3; i < 16; ++i) {
    int index = i - 3;
    bool state = ((bitstring >> (15 - i)) & 0x01);
    if (index >= 0 && index < NUM_FETS)
      digitalWrite(FETs[index], state ? HIGH : LOW);
  }
}


// ----------------- [BEGIN] NEW VARIABLE LOAD ---------------------------

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
  
}
// ------------------ [END] VARIABLE LOAD ------------------

// ------------------ NEW: SAFEETY -----------------------------

void disconnectLoad() {
  digitalWrite(R_LOAD_PIN, LOW);
  delay(1000);
  Serial.print("Residual current: ");
  Serial.println(ina260.readCurrent() / 1000.0);
}

void setLoadControl(int counter, int safety_sig) {
  bool RPM;

 
  if (counter >= 50) {
    RPM = 1;
  } else {
    RPM = 0;
  }

  if (RPM & safety_sig) {

    digitalWrite(R_LOAD_PIN, HIGH);
  } else {

    disconnectLoad();
  }
}
// -----------------------------------------------------------

