// Combined step02 and step03 Arduino sketch with PID control for load resistance based on measured V/I
// Merged and extended on 2025-05-14

#include <Adafruit_INA260.h>
#include "resistor_lookup.h"

// Calculate lookup table size
static const int resistorLookupSize = sizeof(resistorLookup) / sizeof(resistorLookup[0]);

// ---------------------- USER CONFIG ------------------------
const float r_loads[]      = {2};
const uint8_t NUM_LOADS    = sizeof(r_loads) / sizeof(r_loads[0]);

// PID parameters for resistance control
const double Kp = 0.5;    // proportional gain
const double Ki = 0.1;    // integral gain
const double Kd = 0.05;   // derivative gain
const unsigned long pidIntervalMs = 200;  // PID update interval (ms)

// ---------------------- PIN DEFINITIONS -------------------
const int ENCODER_PIN      = 2;
const int ACTUATOR_PIN     = 5;
const int LOAD_EN_PIN      = 25;  // enable variable-load control
const int SAFETY_BUTTON    = 6;   // emergency stop button
const int WALL_CONNECT     = 7;   // secondary safety output
const int BACKUP_WALL_PWR  = 9;   // backup power sense
const int RELAY_NORMAL     = 3;
const int RELAY_SAFETY     = 2;
const int relay_delay      = 50;

const int NUM_FETS = 13;
const int FETs[NUM_FETS] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};

// ---------------------- GLOBAL VARS -----------------------
Adafruit_INA260 ina260;
volatile unsigned long pulseCount = 0;
const double eventsPerRevolution = 2 * 48.0;
double measuredRPM = 0;
unsigned long lastPidTime = 0;
double prevError = 0, integralErr = 0;
double currentSetR = r_loads[0];
volatile bool emergencyState = false;
int prevBackupState = HIGH;
unsigned long lastRpmCalc = 0;

// Sensor data structure
struct SensorData {
  float voltage;
  float current;
  float power;
  float rpm;
  float load_ohm;
  float measured_ohm;
};

// --------------------------- SETUP --------------------------
void setup() {
  Serial.begin(115200);

  // Encoder for logging
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, RISING);

  pinMode(ACTUATOR_PIN, OUTPUT);
  pinMode(LOAD_EN_PIN, OUTPUT);
  pinMode(SAFETY_BUTTON, INPUT_PULLUP);
  pinMode(WALL_CONNECT, OUTPUT);
  pinMode(BACKUP_WALL_PWR, INPUT_PULLUP);

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, LOW);

  for (int i = 0; i < NUM_FETS; i++) {
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }

  if (!ina260.begin()) {
    Serial.println("Error: INA260 not detected!");
    while (1);
  }

  lastPidTime   = millis();
  lastRpmCalc   = millis();

  // Initialize load
  applyBestResistance(currentSetR);
}

// ----------------------------- LOOP ------------------------
void loop() {
  unsigned long now = millis();

  // Safety check
  emergencyState = (digitalRead(SAFETY_BUTTON) == LOW);
  int backupState = digitalRead(BACKUP_WALL_PWR);
  if (backupState == LOW && prevBackupState == HIGH) {
    Serial.println("Backup power lost: braking");
    brakeTasks();
  }
  prevBackupState = backupState;

  if (emergencyState) {
    brakeTasks();
  }

  // Compute RPM for logging (optional)
  if (now - lastRpmCalc >= 1000) {
    measuredRPM = pulseCount * 60000.0 / (now - lastRpmCalc) / eventsPerRevolution;
    pulseCount = 0;
    lastRpmCalc = now;
  }

  // PID control: match external resistance to measured V/I
  if (now - lastPidTime >= pidIntervalMs && !emergencyState) {
    double voltage = getVoltageRaw() / 1000.0;
    double current = getCurrentRaw() / 1000.0;
    double measuredR = (current > 1e-6) ? (voltage / current) : currentSetR;

    double error = measuredR - currentSetR;
    double dt = (now - lastPidTime) / 1000.0;
    integralErr += error * dt;
    double derivative = (error - prevError) / dt;

    double output = Kp * error + Ki * integralErr + Kd * derivative;
    prevError = error;
    lastPidTime = now;

    currentSetR = constrain(currentSetR + output, r_loads[0], r_loads[NUM_LOADS - 1]);
    applyBestResistance(currentSetR);
  }

  // Data logging every 500 ms
  static unsigned long lastLog = 0;
  if (now - lastLog >= 500) {
    SensorData data;
    data.voltage     = getVoltageRaw() / 1000.0;
    data.current     = getCurrentRaw() / 1000.0;
    data.power       = data.voltage * data.current;
    data.rpm         = measuredRPM;
    data.load_ohm    = currentSetR;
    data.measured_ohm = (data.current > 1e-6) ? (data.voltage / data.current) : data.load_ohm;

    Serial.write((uint8_t*)&data, sizeof(data));
    lastLog = now;
  }

  // Maintain relays
  if (!emergencyState) {
    digitalWrite(RELAY_NORMAL, HIGH);
    digitalWrite(RELAY_SAFETY, LOW);
  }
}

// ---------------------- INTERRUPT -------------------------
void updateEncoder() {
  pulseCount++;
}

// ------------------ VARIABLE LOAD --------------------------
void applyBitstring(uint16_t bitstring) {
  for (int b = 3; b < 16; b++) {
    int idx = b - 3;
    if (idx < NUM_FETS) {
      bool on = (bitstring >> (15 - b)) & 0x01;
      digitalWrite(FETs[idx], on ? HIGH : LOW);
    }
  }
}

void applyBestResistance(float targetR) {
  float bestDiff = 1e6;
  uint16_t bestMask = 0;
  for (int i = 0; i < resistorLookupSize; i++) {
    float diff = abs(targetR - resistorLookup[i].resistance);
    if (diff < bestDiff) {
      bestDiff = diff;
      bestMask = resistorLookup[i].mask;
    }
  }
  applyBitstring(bestMask);
  delay(50);
}

void disconnectLoad() {
  digitalWrite(LOAD_EN_PIN, LOW);
  delay(500);
  Serial.print("Residual current: ");
  Serial.println(ina260.readCurrent() / 1000.0);
}

void brakeTasks() {
  Serial.println("Emergency brake engaged");
  disconnectLoad();
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, HIGH);
}

// ------------------- SENSOR HELPERS ------------------------
int getCurrentRaw() {
  double sum = 0;
  for (int i = 0; i < 4; i++) sum += ina260.readCurrent();
  return int(sum / 4);
}

int getVoltageRaw() {
  double sum = 0;
  for (int i = 0; i < 4; i++) sum += ina260.readBusVoltage();
  return int(sum / 4);
}