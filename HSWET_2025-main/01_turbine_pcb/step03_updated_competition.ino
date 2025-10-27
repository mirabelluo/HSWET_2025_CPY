#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 500;
int settingLoggingInterval = 5000;
int pitchPositionHoldTime = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// [INPUT] encoder variables
int ENCODER_PIN = 2;
int ACTUATOR_PIN = 5;
const int LOAD_PIN = 7;
int SAFETY_PIN = 6;
int LOAD_CONTROL = 8;
int WALL_CONNECT = 9;

int positions[] = {1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500};
int i = 0;

double rpm = 0;
double eventsPerRevolution = 2 * 48;
volatile int counter = 0;

int RPM_THRESHOLD = 1000;

const int load_increment = 5;
const int load_start = 0;
const int load_stop = 1;
int load_setting = load_start;

int RELAY_NORMAL = 3;
int RELAY_SAFETY = 2;
int relay_delay = 50;

// Rated power variables
int maxRpm = 20; //this will be ~20 below the rpm that we were able to achive at the correct pitch at 11m/s
const int minPitch = 1100; //the selected power generation pitch
const int featheredPitch = 1500; //the pitch where blades are feathered

// [INPUT] RPM control loop variables. _HIGH and _LOW can be different, to be determined through experiments
// _HIGH means RPM is too high. the "10" changes the pitch accordingly.
// _LOW means RPM is too low. the "8" changes the pitch accordingly.
// Smaller values means more optimized power outputs, but more potential for error/slow adjustment
const int RPM_MARGIN_HIGH = 10; 
const int RPM_MARGIN_LOW = 8;

// [INPUT] These are probably fine. But the pitches changes by 2 and 1, respectively, for control loop
// After these look good, you should stop changing them.
const int PITCH_INCREMENT_DOWN = 2;
const int PITCH_INCREMENT_UP = 1; 

int currentPitchDeviation = 0;

// 0 for normal operation, 1 for emergency/stopped positions
int prev_state_emergency = 1;
volatile int state_emergency = 1;
int prev_backup = 1;

struct SensorData {
    float timestamp;
    float voltage;
    float current;
    float power;
    float rpm;
    float pitch;
    float load_setting;
  };

// Actuator control variables
bool movingActuator = false;
long actuatorStartTime = 0;
int actuatorTargetInterval = 0;
bool actuatorState = false;

// put your setup code here, to run once:
void setup() {
  Serial.begin(115200);

  // Serial.print("RPM_MARGINS:");
  // Serial.print(RPM_MARGIN_HIGH);
  // Serial.print(",");
  // Serial.println(RPM_MARGIN_LOW);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);
  
  pinMode(LOAD_PIN, OUTPUT);
  analogWrite(LOAD_PIN, load_setting);

  pinMode(SAFETY_PIN, INPUT_PULLUP);
  pinMode(LOAD_CONTROL, OUTPUT);
  pinMode(WALL_CONNECT, OUTPUT);

  // Set up the INA260 power sensor chip
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // Linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  linActNewPos(positions[i]);

  while (!Serial) { delay(10); }
  previousSettingTime = millis();

  // pinMode(SAFETY_PIN, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(SAFETY_PIN), brake, RISING);
  // attachInterrupt(digitalPinToInterrupt(SAFETY_PIN), restart, FALLING);
  // // pinMode(WALL_CONNECT, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(WALL_CONNECT), brake_pcc, RISING);

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_SAFETY, LOW);
  

  // // linear actuator
  // pinMode(ACTUATOR_PIN, OUTPUT);

  // // start at feathered position
  // brake_tasks();
}

void loop() {

    // Read the safety pin
  int safetySignal = digitalRead(SAFETY_PIN);

  // Adjust pitch based on the safety signal
  if (safetySignal == HIGH) { // Pitch forward
    if (i < sizeof(positions) / sizeof(int) - 1) {
      i++;
      linActNewPos(positions[i]);
    }
  } else { // Pitch in
    if (i > 0) {
      i--;
      linActNewPos(positions[i]);
    }
  }

  setLoadControl(counter, safetySignal);

  // if backup low, clear the flag
  if (digitalRead(WALL_CONNECT) == LOW) {
    prev_backup = 0;
  } else {
    // if not check if you need to brake
    if (prev_backup == 0) {
      // Serial.println("PCC disconnect");
      prev_backup = 1;
      brake_tasks();
      delay(5000);
    }
    // otherwise do nothing
  }
  
  // if (state_emergency == 1 && prev_state_emergency != 1){
  //   // Serial.println("SAFETY_PIN pressed");
  //   brake_tasks();
  //   prev_state_emergency = state_emergency;
  // } else if (state_emergency == 0 && prev_state_emergency != 0) {
  //   // Serial.println("Restarting");
  //   restart_tasks();
  //   prev_state_emergency = state_emergency;
  // }
  
  // put your main code here, to run repeatedly:
  currentTime = millis();
  if (currentTime - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    float voltage = getVoltage() / 1000.0;
    float current = getCurrent();

    SensorData data;
    data.timestamp = millis() / 1000.0;
    data.voltage = voltage;
    data.current = current;
    data.power = ina260.readPower() / 1000.0;
    data.rpm = rpm;
    data.pitch = currentPitchDeviation;
    data.load_setting = load_setting;

    Serial.write((uint8_t*)&data, sizeof(data));

    counter = 0;
    previousTime = millis();
    if (prev_state_emergency == 0) {
      if (rpm > maxRpm + RPM_MARGIN_HIGH) {
        currentPitchDeviation += max(PITCH_INCREMENT_DOWN, int(abs(rpm-maxRpm)/25.0));
        pitchNewDeviation(currentPitchDeviation);
        // Serial.print("RPMs are too high, increased pitch to: ");
        // Serial.println(minPitch + currentPitchDeviation);
      }
      if (rpm < maxRpm - RPM_MARGIN_LOW) {
        currentPitchDeviation -= max(PITCH_INCREMENT_UP, int(abs(rpm-maxRpm)/30.0));
        pitchNewDeviation(currentPitchDeviation);
        // Serial.print("RPMs are too low, decreased pitch to: ");
        // Serial.println(minPitch + currentPitchDeviation);
      }
    }
  }
  linActHandleMovement();
}

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
  movingActuator = true;
  actuatorStartTime = micros();
  actuatorTargetInterval = interval;
  actuatorState = true;
  digitalWrite(ACTUATOR_PIN, actuatorState);
  // long timer = 0;
  // boolean state_emergency = 1;
  // for (int i = 0; i < 100000; i++) {
  //   if (micros() > timer) {
  //     timer = micros() + interval;
  //     state_emergency = !state_emergency;
  //     digitalWrite(ACTUATOR_PIN, state_emergency);
  //   }
  // }
  // digitalWrite(ACTUATOR_PIN, LOW);
}

float getCurrent() {
  double I = 0;
  double samples = 4;
  double samples2 = 25;
  for (int i = 0; i < samples2; i++) {
    for (int j = 0; j < samples; j++) {
      I += ina260.readCurrent();
    } 
  }
  return (float)(I / samples / samples2);
}

float getVoltage() {
  double V = 0;
  double samples = 4;
  double samples2 = 50;
  for (int i = 0; i < samples2; i++) {
    for (int j = 0; j < samples; j++) {
      V += ina260.readBusVoltage(); 
    } 
  }
  return (float)(V / samples / samples2);
}

void brake() {
  state_emergency = 1;
}

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

void restart_tasks() {
  //  Serial.println("Restart tasks");
  digitalWrite(RELAY_SAFETY, LOW);
  digitalWrite(RELAY_NORMAL, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_NORMAL, LOW);
  linActNewPos(minPitch);
  delay(15000);
}

void pitchNewDeviation(int deviation) {
  if (deviation < 0) {
    deviation = 0;
  }
  linActNewPos(minPitch + deviation);
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
  if (counter >= 50) { // modify later
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
