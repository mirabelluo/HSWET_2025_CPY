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

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(SB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SB), brake, RISING);
  pinMode(BACKUP_P, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BACKUP_P), brake, RISING);

  pinMode(RELAY_NORMAL, OUTPUT);
  pinMode(RELAY_SAFETY, OUTPUT);
  //digitalWrite(RELAY_SAFETY, HIGH);
  //delay(300);
  //digitalWrite(RELAY_SAFETY, LOW);
  //delay(5000);
  digitalWrite(RELAY_NORMAL, HIGH);
  delay(300);
  digitalWrite(RELAY_NORMAL, LOW);
  
  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
  //while (1);
  }

  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  // range is 1100(all the way in)-1500(all the way out)
  //adjust accordingly based on new setup
  linActNewPos(1500);
  delay(7000);
}

void loop() {
  currentTime = millis();

  if (millis() - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / (millis()-previousTime) / eventsPerRevolution;
    int voltage = getVoltage() / 1000.0;
    int current = getCurrent();
    Serial.print("Voltage: ");
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
  }
}

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
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
  Serial.print("Need to brake");
  return;
}
