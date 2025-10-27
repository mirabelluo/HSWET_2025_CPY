#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

// encoder variables
int ENCODER_PIN = 7;
double rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2*48;
volatile int counter = 0;

int ACTUATOR_PIN = 6;

int dataLoggingInterval = 500;
long currentTime = 0;
long previousTime = 0;

int SB = 4;
int BACKUP_P = 5;

int RELAY_NORMAL = 3;
int RELAY_SAFETY = 2;

//rated power variables
//TODO: change maxRpm
int maxRpm = 10; //this will be ~20 below the rpm that we were able to achive at the correct pitch at 11m/s
//TODO: change minPitch
int minPitch = 1300; //the selected power generation pitch
int maxPitch = 1650; //the pitch where blades are feathered
int RPM_MARGIN_HIGH = 20; // smaller means better score but more potential for error/slow adjustment
int RPM_MARGIN_LOW = 20;
int PITCH_INCREMENT_DOWN = 2;
int PITCH_INCREMENT_UP = 1; 
int currentPitchDeviation = 0;

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_SAFETY, LOW);
  digitalWrite(RELAY_NORMAL, HIGH);
  delay(300);
  digitalWrite(RELAY_NORMAL, LOW);
  
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  // range is 1100(all the way in)-1500(all the way out)
  //adjust accordingly based on new setup
  linActNewPos(minPitch);
  delay(7000);
}

void loop() {
  currentTime = millis();
  if (currentTime - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    Serial.print("RPM: ");
    Serial.println(rpm);

    counter = 0;
    previousTime = millis();

    // put your main code here, to run repeatedly:
    if (rpm > maxRpm + RPM_MARGIN_HIGH) {
      currentPitchDeviation += max(PITCH_INCREMENT_DOWN, int(abs(rpm-maxRpm)/25.0));
      pitchNewDeviation();
      Serial.print("RPMs are too high, increased pitch to: ");
      Serial.println(minPitch + currentPitchDeviation);
    }
    if (rpm < maxRpm - RPM_MARGIN_LOW) {
      currentPitchDeviation -= max(PITCH_INCREMENT_UP, int(abs(rpm-maxRpm)/30.0));
      pitchNewDeviation();
      Serial.print("RPMs are too low, decreased pitch to: ");
      Serial.println(minPitch + currentPitchDeviation);
    }
  }
}

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
  if(interval < minPitch) {
    interval = minPitch;
  }
  if (interval > maxPitch) {
    interval = maxPitch;
  }
  long timer = 0;
  boolean state = 1;
  for (int i = 0; i < 100000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state = !state;
      digitalWrite(ACTUATOR_PIN, state);
    }
  }
  digitalWrite(ACTUATOR_PIN, LOW);
}

void pitchNewDeviation() {
  if (currentPitchDeviation < 0) {
    currentPitchDeviation = 0;
  }
  linActNewPos(minPitch + currentPitchDeviation);
}
