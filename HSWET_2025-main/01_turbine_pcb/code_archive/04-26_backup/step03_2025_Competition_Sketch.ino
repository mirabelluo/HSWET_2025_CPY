// Updated 04/17/2025
// Turbine automated power collection code
// With control loop but no safety system yet
// Updated 04/17/2025
// Authors: Delancy, Joon, Christian

#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 250;
int settingLoggingInterval = 5000;
int pitchPositionHoldTime = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// [DON't CHANGE] Encoder Variables for RPM measurements
const int ENCODER_PIN = 2;
double rpm = 0;
double eventsPerRevolution = 2*48;
volatile int counter = 0;

const int ACTUATOR_PIN = 5;

// [TODO] R_LOAD and safety system stuff: not implemented yet
const int R_LOAD_PIN = 6; 
const int SAFETY_BUTTON = 7;
const int BACKUP_WALL_PWR = 9;

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
    float voltage;
    float current;
    float power;
    float rpm;
    float pitch;
    float load_setting;
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
  //attachInterrupt(digitalPinToInterrupt(SAFETY_BUTTON), brake, RISING);
  //attachInterrupt(digitalPinToInterrupt(SAFETY_BUTTON), restart, FALLING);
  pinMode(BACKUP_WALL_PWR, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(BACKUP_WALL_PWR), brake_pcc, RISING);

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
  if (digitalRead(SAFETY_BUTTON) == HIGH) {
    state_emergency = 1;
  } else {
    state_emergency = 0;
  }

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
    Serial.println("SAFETY_BUTTON pressed");
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
    // TODO: if voltage  above 45, pitch increase minPitch by 50
    int current = getCurrent();
    Serial.print("state_emergency: ");
    Serial.print(prev_state_emergency);
    Serial.print(", Voltage: ");
    Serial.print(voltage);
    Serial.print(", RPMs: ");
    Serial.print(rpm);   
    Serial.print(", SAFETY_BUTTON: ");
    Serial.print(digitalRead(SAFETY_BUTTON));  
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


// [HELPER FUNCTIONS] These are not to be changed unless you know what you're doing
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
    Serial.println("Restart tasks");
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
