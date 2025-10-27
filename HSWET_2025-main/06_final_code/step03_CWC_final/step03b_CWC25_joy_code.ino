// Updated 04/17/2025
// Turbine automated power collection code
// With control loop but no safety system yet
// Updated 04/17/2025
// Authors: Delancy, Joon, Christian

#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

#include <Adafruit_INA260.h>
#include "resistor_lookup.h"

int dataLoggingInterval = 250;
int settingLoggingInterval = 5000;
int pitchPositionHoldTime = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// [DON'T CHANGE] Encoder Variables for RPM measurements
const int ENCODER_PIN = 2;
const int ACTUATOR_PIN = 5;

// [TODO] R_LOAD and safety system stuff: not implemented yet
const int LOAD_PIN = 25; // used to be 6
const int WALL_CONNECT = 7; // used to be 9, MEGA also for signaling
const int SAFETY_PIN = 6; // used to be 11

// ------------------------ CONST DEC ----------------------------
const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};



// [CONFIG] Encoder variables
double rpm = 0;
double eventsPerRevolution = 2*48;
volatile int counter = 0;

// Rated power variables [TODO]
int maxRpm = 20; //this will be ~20 below the rpm that we were able to achive at the correct pitch at 11m/s

const int minPitch = 1300; //the selected power generation pitch
const int featheredPitch = 1500; // the pitch where blades are feathered

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
volatile int state_emergency = 1;

// THe three ranges of resistanc
const int R1 = 30;
const int R2 = 20;
const int R3 = 10;

int load_setting = 0; // r_set
int prev_backup = 1;

struct SensorData {
    float voltage;
    float current;
    float power;
    float rpm;
    float pitch;
    float load_setting; // Resistance from variable load. need to add resistanceMeasured too
  };

// put your setup code here, to run once:
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


  // Set up the INA260 power sensor chip
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  
  // set to 30 ohm at the beginning
  load_setting = R1; 
  analogWrite(LOAD_PIN, load_setting);
  applyBestResistance(R1);

  pinMode(SAFETY_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(SAFETY_PIN), brake, RISING);
  //attachInterrupt(digitalPinToInterrupt(SAFETY_PIN), restart, FALLING);
  pinMode(BACKUP_WALL_PWR, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(BACKUP_WALL_PWR), brake_pcc, RISING);


  // THIS IS PIN 25 THIS YEAR - ASK JOHN
  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_SAFETY, LOW);
  
  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);

  // start at feathered position
  brake_tasks();
}

void loop() {
  //check if safety button has been pressed an update state_emergency
  if (digitalRead(SAFETY_PIN) == HIGH) {
    state_emergency = 1;
  } else {
    state_emergency = 0;
  }

    if (testComplete) return;


  // if backup low, clear the flag
  if (digitalRead(BACKUP_WALL_PWR) == LOW) {
    prev_backup = 0;
  } else {
    // if not check if you need to brake
    if (prev_backup == 0) {
      Serial.println("PCC disconnect");
      prev_backup = 1;
      brake_tasks;
      delay(5000);
    }
    // otherwise do nothing
  }
  
  if (state_emergency == 1 && prev_state_emergency != 1){
    Serial.println("SAFETY_PIN pressed");
    brake_tasks();
    prev_state_emergency = state_emergency;
  } else if (state_emergency == 0 && prev_state_emergency != 0) {
    Serial.println("Restarting");
    restart_tasks();
    prev_state_emergency = state_emergency;
  }
  
  // put your main code here, to run repeatedly:
  currentTime = millis();
  if (currentTime - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    
    int voltage = getVoltage() / 1000.0;
    int current = getCurrent();
    
    data.voltage = voltage;
    data.current = current;
    data.power = voltage * current / 1000.0;
    data.rpm = rpm;
    data.pitch = minPitch + currentPitchDeviation;

    
    // here, if we're in the first 2 minutes of the test, set load_setting to 30. 2-4 minutes, load_setting to 20. 4-6 minutes, loas setting to 10.
    // {TODO}

    Serial.print("state_emergency: ");
    Serial.print(prev_state_emergency);
    Serial.print(", Voltage: ");
    Serial.print(voltage);
    Serial.print(", RPMs: ");
    Serial.print(rpm);   
    Serial.print(", SAFETY_PIN: ");
    Serial.print(digitalRead(SAFETY_PIN));  
    Serial.print(", Backup: ");
    Serial.print(digitalRead(BACKUP_WALL_PWR));  
    Serial.print(", Current: ");
    Serial.println(current); 

    // Control loop for RPM adjustment [IMPORTANT]
    counter = 0;
    previousTime = millis();
    if (prev_state_emergency == 0) {
      if (rpm > maxRpm + RPM_MARGIN_HIGH) {
        currentPitchDeviation += max(PITCH_INCREMENT_DOWN, int(abs(rpm-maxRpm)/25.0));
        pitchNewDeviation(currentPitchDeviation);
        Serial.print("RPMs are too high, increased pitch to: ");
        Serial.println(minPitch + currentPitchDeviation);
      }
      if (rpm < maxRpm - RPM_MARGIN_LOW) {
        currentPitchDeviation -= max(PITCH_INCREMENT_UP, int(abs(rpm-maxRpm)/30.0));
        pitchNewDeviation(currentPitchDeviation);
        Serial.print("RPMs are too low, decreased pitch to: ");
        Serial.println(minPitch + currentPitchDeviation);
      }
    }
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

void linActNewPos(int interval) {
  if(interval < minPitch) {
    interval = minPitch;
  }
  if (interval > featheredPitch) {
    interval = featheredPitch;
  }
  long timer = 0;
  boolean state_emergency = 1;
  for (int i = 0; i < 100000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state_emergency = !state_emergency;
      digitalWrite(ACTUATOR_PIN, state_emergency);
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
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_SAFETY, LOW);
}

void restart() {
  state_emergency = 0;
}


// [TODO] MIRABEL UPDATE THIS FOR SAFETY ----------------------
void restart_tasks() {
    Serial.println("Restart tasks");
  digitalWrite(RELAY_SAFETY, LOW);
  digitalWrite(RELAY_NORMAL, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_NORMAL, LOW);
  linActNewPos(minPitch);
  delay(15000);
}

// ------------------------------------------------------------
void pitchNewDeviation(int deviation) {
  if (deviation < 0) {
    deviation = 0;
  }
  linActNewPos(minPitch + deviation);
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

