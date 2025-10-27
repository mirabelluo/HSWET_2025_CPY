#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 500;
int settingLoggingInterval = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// [INPUT] encoder variables
int ENCODER_PIN = 2;
int ACTUATOR_PIN = 5;
const int LOAD_PIN = 6;
int SAFETY_PIN = 11;
int LOAD_CONTROL = 8;
int WALL_CONNECT = 9;
//int positions[] = {1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500};

int positions[] = {1900, 1800, 1600, 1500, 1400, 1300, 1250};

double rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2 * 48;
volatile int counter = 0;

int RPM_THRESHOLD = 1000;

int i = 0;
const int load_increment = 5;
const int load_start = 0;
const int load_stop = 1;

int load_setting = load_start;

struct SensorData {
  float voltage;
  float current;
  float rpm;
  float pitch;
  float load_setting;
};

// Actuator control variables
bool movingActuator = false;
long actuatorStartTime = 0;
int actuatorTargetInterval = 0;
bool actuatorState = false;

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(LOAD_PIN, OUTPUT);
  analogWrite(LOAD_PIN, load_setting);

  pinMode(SAFETY_PIN, INPUT_PULLUP);
  pinMode(LOAD_CONTROL, OUTPUT);
  pinMode(WALL_CONNECT, OUTPUT);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // Linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  linActStartNewPos(positions[i]);

  while (!Serial) { delay(10); }
  previousSettingTime = millis();
}

void loop() {
  currentTime = millis();

  // Log data at the specified interval
  if (currentTime - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / dataLoggingInterval / eventsPerRevolution;
    SensorData data;
    data.voltage = ina260.readBusVoltage() / 1000.0;
    data.current = ina260.readCurrent() / 1000.0;
    data.rpm = rpm;
    data.pitch = positions[i];
    data.load_setting = load_setting;

    Serial.write((uint8_t*)&data, sizeof(data));

    counter = 0; // Reset counter
    previousTime = currentTime;
  }

    // Read the safety pin
    int safetySignal = digitalRead(SAFETY_PIN);

    // Adjust pitch based on the safety signal
    if (safetySignal == HIGH) { // Pitch forward
      if (i < sizeof(positions) / sizeof(int) - 1) {
        i++;
        linActStartNewPos(positions[i]);
      }
    } else { // Pitch in
      if (i > 1) {
        i--;
        linActStartNewPos(positions[i]);
      }
    }

  setLoadControl(counter, safetySignal);

  // Adjust load and pitch at the specified interval
  if (currentTime - previousSettingTime > settingLoggingInterval) {
    load_setting += load_increment;
    if (load_setting > load_stop) {
      load_setting = load_start;
      i++;
      if (i == sizeof(positions) / sizeof(int)) {
        // Send end-of-cycle signal
        SensorData data = {-1, -1, -1, -1, -1};
        Serial.write((uint8_t*)&data, sizeof(data));
        i = 0; // Reset pitch index for continuous testing
      }
      linActStartNewPos(positions[i]); // Start moving the actuator
    }

    analogWrite(LOAD_PIN, load_setting);
    previousSettingTime = currentTime;
  }

  // Handle non-blocking actuator movement
  linActHandleMovement();
}

void updateEncoder() {
  counter++;
}

// Initiate non-blocking actuator movement
void linActStartNewPos(int interval) {
  movingActuator = true;
  actuatorStartTime = micros();
  actuatorTargetInterval = interval;
  actuatorState = true;
  digitalWrite(ACTUATOR_PIN, actuatorState);
}

// Handle actuator movement in a non-blocking way
void linActHandleMovement() {
  if (movingActuator) {
    if (micros() - actuatorStartTime >= actuatorTargetInterval) {
      actuatorStartTime += actuatorTargetInterval;
      actuatorState = !actuatorState;
      digitalWrite(ACTUATOR_PIN, actuatorState);
    }
    if (micros() - actuatorStartTime >= 5000000) { // Stop after 5 seconds
      movingActuator = false;
      digitalWrite(ACTUATOR_PIN, LOW);
    }
  }
}

// Control LOAD_CONTROL Pin
void setLoadControl(int counter, int safety_sig) {
  
  bool RPM;
  if (counter >= 15) { // modify later
  //if (ina260.readBusVoltage() / 1000.0 > 3) {
    RPM = 1;
  } else {
    RPM = 0;
  } 

  if (RPM & safety_sig) {
    digitalWrite(LOAD_CONTROL, HIGH);
    digitalWrite(WALL_CONNECT, LOW);
  } else {
    digitalWrite(LOAD_CONTROL, LOW);
    digitalWrite(WALL_CONNECT, HIGH);
  }
}
