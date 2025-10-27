#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();

int dataLoggingInterval = 500;
int settingLoggingInterval = 5000;
long previousSettingTime = 0;
long currentTime = 0;
long previousTime = 0;

// encoder variables
int ENCODER_PIN = 2;
double rpm = 0;
// 2*external encoder setting
double eventsPerRevolution = 2*48;
volatile int counter = 0;

int RPM_THRESHOLD = 1000;

int ACTUATOR_PIN = 5;

const int LOAD_PIN = 6;

int i = 0;
int positions[] = {1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500};
//int positions[] = {1150, 1175, 1200, 1225, 1250};
const int load_increment = 5;
// put 5 less than what you want
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

void setup() {
  Serial.begin(115200);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), updateEncoder, CHANGE);

  pinMode(LOAD_PIN, OUTPUT);
  analogWrite(LOAD_PIN, load_setting);

  if (!ina260.begin()) {
    Serial.println("Couldn't find INA260 chip");
    while (1);
  }

  // linear actuator
  pinMode(ACTUATOR_PIN, OUTPUT);
  // range is 1100(all the way in)-1500(all the way out)
  linActNewPos(positions[i]);
  delay(5000);
  //linActNewPos(1100);
  while (!Serial) { delay(10); }
  previousSettingTime = millis();
}

void loop() {
  currentTime = millis();

  if (millis() - previousTime > dataLoggingInterval) {
    rpm = counter * 60000.0 / dataLoggingInterval / eventsPerRevolution;
    SensorData data;
    data.voltage = ina260.readBusVoltage() / 1000.0;
    data.current = ina260.readCurrent() / 1000.0;
    data.rpm = rpm;
    data.pitch = positions[i];
    data.load_setting = load_setting;

    // Sending the struct through serial
    Serial.write((uint8_t*)&data, sizeof(data));
    /*Serial.print("Voltage: ");
    Serial.print(data.voltage);
    Serial.print(", Current: ");
    Serial.print(data.current);
    Serial.print(", RPM: ");
    Serial.print(data.rpm);
    Serial.print(", Pitch: ");
    Serial.print(data.pitch);
    Serial.print(", Load: ");
    Serial.println(load_setting);*/
    
    counter = 0;
    previousTime = millis();
  }
  
  if (millis() - previousSettingTime > settingLoggingInterval) {
    load_setting += load_increment;
    if (load_setting > load_stop) {
      load_setting = load_start;
      // also increment pitch
      i++;
      // check if done with all pitch values
      if (i == sizeof(positions)/sizeof(int)) {
        // done with current windspeed
        // set all values to negative to indicate being done
        SensorData data;
        data.voltage = -1;
        data.current = -1;
        data.rpm = -1;
        data.pitch = -1;
        data.load_setting = -1;
    
        // Sending the struct through serial
        Serial.write((uint8_t*)&data, sizeof(data));
      }
      linActNewPos(positions[i]);
      delay(5000);
    }
    analogWrite(LOAD_PIN, load_setting);
    previousSettingTime = millis();
  }
}

void updateEncoder() {
  counter++;
}

void linActNewPos(int interval) {
  long timer = 0;
  boolean state = 1;
  for (int i = 0; i < 10000; i++) {
    if (micros() > timer) {
      timer = micros() + interval;
      state = !state;
      digitalWrite(ACTUATOR_PIN, state);
    }
  }
}
