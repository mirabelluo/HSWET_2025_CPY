
#ifndef R_TARGET_H
#define R_TARGET_H

#include <Arduino.h>
#include <Adafruit_INA260.h>

// External INA260 sensor instance
extern Adafruit_INA260 ina260;



// Sensor data struct
struct SensorData {
  float voltage;
  float current;
  float power;
  float rpm;
  float pitch;
  float load_setting;
};

// Function declarations
void initialize();
float calculate_targetRes(int interval, int r_old);
float updateResistance(float r_old, float r_new, float K);
void setResistance(float targetR, int interval, int settlecount, float k, float tolerance);
void initialR(int setR);
void disconnectLoad();




#endif
