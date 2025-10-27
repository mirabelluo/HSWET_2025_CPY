#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

// encoder variables
int ENCODER_PIN = 5;
double rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2*48;
volatile int counter = 0;

int ACTUATOR_PIN = 2;

int dataLoggingInterval = 500;
long currentTime = 0;
long previousTime = 0;

int SB = 4;
int BACKUP_P = 5;

int RELAY_NORMAL = 3;
int RELAY_SAFETY = 2;
int relay_delay = 50;

//rated power variables
//TODO: change maxRpm
int maxRpm = 20; //this will be ~20 below the rpm that we were able to achive at the correct pitch at 11m/s
//TODO: change minPitch 1100 - 1500
int minPitch = 1185; //the selected power generation pitch
int featheredPitch = 1500; //the pitch where blades are feathered
int RPM_MARGIN_HIGH = 10; // smaller means better score but more potential for error/slow adjustment
int RPM_MARGIN_LOW = 10;
int PITCH_INCREMENT_DOWN = 2;
int PITCH_INCREMENT_UP = 1; 
int currentPitchDeviation = 0;

// 0 for normal, 1 for safety
int prev_state = 1;
volatile int state = 1;

int prev_backup = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(SB, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(SB), brake, RISING);
  //attachInterrupt(digitalPinToInterrupt(SB), restart, FALLING);
  pinMode(BACKUP_P, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(BACKUP_P), brake_pcc, RISING);

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  digitalWrite(RELAY_NORMAL, LOW);
  digitalWrite(RELAY_SAFETY, HIGH);
  delay(relay_delay);
  digitalWrite(RELAY_SAFETY, LOW);
  
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);

  // start at feathered position
  brake_tasks();
}

void loop() {
  //check if safety button has been pressed an update state
  if (digitalRead(SB) == HIGH) {
    state = 1;
  } else {
    state = 0;
  }

  // if backup low, clear the flag
  if (digitalRead(BACKUP_P) == LOW) {
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
  
  if (state == 1 && prev_state != 1){
    Serial.println("SB pressed");
    brake_tasks();
    prev_state = state;
  } else if (state == 0 && prev_state != 0) {
    Serial.println("Restarting");
    restart_tasks();
    prev_state = state;
  }
  
  // put your main code here, to run repeatedly:
  currentTime = millis();
  if (currentTime - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    int voltage = getVoltage() / 1000.0;
    // TODO: if voltage  above 45, pitch increase minPitch by 50
    int current = getCurrent();
    Serial.print("State: ");
    Serial.print(prev_state);
    Serial.print(", Voltage: ");
    Serial.print(voltage);
    Serial.print(", RPMs: ");
    Serial.print(rpm);   
    Serial.print(", SB: ");
    Serial.print(digitalRead(SB));  
    Serial.print(", Backup: ");
    Serial.print(digitalRead(BACKUP_P));  
    Serial.print(", Current: ");
    Serial.println(current); 

    counter = 0;
    previousTime = millis();
    if (prev_state == 0) {
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
  state = 1;
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
  state = 0;
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
