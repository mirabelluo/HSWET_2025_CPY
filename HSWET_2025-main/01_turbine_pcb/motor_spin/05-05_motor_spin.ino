// Motor X Pins
#define EN_PIN_X    9   // Enable 
#define STEP_PIN_X  8   // Step
#define DIR_PIN_X   7   // Direction

int step_delay = 500;          // Microseconds between steps
unsigned long switch_interval = 2000;  // Switch direction every 2 seconds
unsigned long last_switch_time = 0;
bool direction = LOW;          // Start with CCW

void setup() {
  Serial.begin(9600);
  Serial.println("Motor test: switching direction every 2 seconds");

  pinMode(EN_PIN_X, OUTPUT);
  pinMode(STEP_PIN_X, OUTPUT);
  pinMode(DIR_PIN_X, OUTPUT);

  digitalWrite(EN_PIN_X, LOW);  // Enable motor driver
  digitalWrite(DIR_PIN_X, direction); // Set initial direction
  last_switch_time = millis();
}

void loop() {
  // Switch direction every 2 seconds
  if (millis() - last_switch_time >= switch_interval) {
    direction = !direction;
    digitalWrite(DIR_PIN_X, direction);
    Serial.print("Direction switched to: ");
    Serial.println(direction == HIGH ? "HIGH (CW)" : "LOW (CCW)");
    last_switch_time = millis();
  }

  // Step the motor
  digitalWrite(STEP_PIN_X, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(STEP_PIN_X, LOW);
  delayMicroseconds(step_delay);
}
