#include "Wire.h" // This library allows you to communicate with I2C devices.

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

// Motor 1 (X-direction) Pins
#define EN_PIN_X    9 // Enable 
#define STEP_PIN_X  8 // Step
#define DIR_PIN_X   7 // Direction

// Motor 2 (Y-direction) Pins
#define EN_PIN_Y    6 // Enable 
#define STEP_PIN_Y  5 // Step
#define DIR_PIN_Y   4 // Direction

// Calibration offsets (initially zero)
int16_t ax_offset = 0;
int16_t ay_offset = 0;
int16_t az_offset = 0;

// Calibration duration (5 seconds)
const int calibration_duration = 5000;
unsigned long start_time;

long sum_ax = 0;
long sum_ay = 0;
long sum_az = 0;
int num_samples = 0;

void setup() {
  Serial.begin(9600);

  // Set up motor pins as outputs
  pinMode(EN_PIN_X, OUTPUT);
  pinMode(STEP_PIN_X, OUTPUT);
  pinMode(DIR_PIN_X, OUTPUT);
  digitalWrite(EN_PIN_X, HIGH); // deactivate driver (LOW active)

  pinMode(EN_PIN_Y, OUTPUT);
  pinMode(STEP_PIN_Y, OUTPUT);
  pinMode(DIR_PIN_Y, OUTPUT);
  digitalWrite(EN_PIN_Y, HIGH); // deactivate driver (LOW active)

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  start_time = millis(); // Record start time for calibration
}

void rotateMotor(int steps, bool direction, int speed, int stepPin, int dirPin) {
  // Set the motor direction
  digitalWrite(dirPin, direction);
  
  // Step the motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speed);
  }
}

void loop() {
  // Read accelerometer and gyro data from MPU-6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 7*2, true);

  // Read accelerometer and gyro data
  accelerometer_x = (Wire.read() << 8 | Wire.read());
  accelerometer_y = (Wire.read() << 8 | Wire.read());
  accelerometer_z = (Wire.read() << 8 | Wire.read());
  temperature = Wire.read() << 8 | Wire.read(); 
  gyro_x = Wire.read() << 8 | Wire.read(); 
  gyro_y = Wire.read() << 8 | Wire.read(); 
  gyro_z = Wire.read() << 8 | Wire.read();

  // During the first 5 seconds, collect data for calibration
  if (millis() - start_time <= calibration_duration) {
    sum_ax += accelerometer_x;
    sum_ay += accelerometer_y;
    sum_az += accelerometer_z;
    num_samples++;
    
    Serial.println("Calibrating...");
  }
  else if (num_samples > 0 && ax_offset == 0 && ay_offset == 0 && az_offset == 0) {
    ax_offset = sum_ax / num_samples;
    ay_offset = sum_ay / num_samples;
    az_offset = sum_az / num_samples;
    Serial.println("Calibration complete!");
  }
  
  // Apply the calculated offsets
  accelerometer_x -= ax_offset;
  accelerometer_y -= ay_offset;
  accelerometer_z -= az_offset;

  const int step_delay = 300; // smaller = faster

if (accelerometer_x > 1000) { // Right tilt
  digitalWrite(EN_PIN_X, LOW);               // Enable motor
  digitalWrite(DIR_PIN_X, HIGH);             // Set direction
  digitalWrite(STEP_PIN_X, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(STEP_PIN_X, LOW);
  delayMicroseconds(step_delay);
  Serial.println("Right tilt detected, rotating motor X clockwise.");
} else if (accelerometer_x < -1000) { // Left tilt
  digitalWrite(EN_PIN_X, LOW);
  digitalWrite(DIR_PIN_X, LOW);
  digitalWrite(STEP_PIN_X, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(STEP_PIN_X, LOW);
  delayMicroseconds(step_delay);
  Serial.println("Left tilt detected, rotating motor X counterclockwise.");
} else {
  digitalWrite(EN_PIN_X, HIGH); // Disable motor if no tilt
  Serial.println("No tilt detected, motor X disabled.");
}

if (accelerometer_y > 1000) { // Forward tilt
  digitalWrite(EN_PIN_Y, LOW);
  digitalWrite(DIR_PIN_Y, HIGH);
  digitalWrite(STEP_PIN_Y, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(STEP_PIN_Y, LOW);
  delayMicroseconds(step_delay);
  Serial.println("Forward tilt detected, rotating motor Y clockwise.");

} else if (accelerometer_y < -1000) { // Backward tilt
  digitalWrite(EN_PIN_Y, LOW);
  digitalWrite(DIR_PIN_Y, LOW);
  digitalWrite(STEP_PIN_Y, HIGH);
  delayMicroseconds(step_delay);
  digitalWrite(STEP_PIN_Y, LOW);
  delayMicroseconds(step_delay);
  Serial.println("Backward tilt detected, rotating motor Y counterclockwise.");
} else {
  digitalWrite(EN_PIN_Y, HIGH); // Disable motor
}

}
