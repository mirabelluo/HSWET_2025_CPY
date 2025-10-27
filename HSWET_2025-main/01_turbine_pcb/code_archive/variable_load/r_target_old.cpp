
#include "r_target.h"
#include "resistor_lookup.h" 

#include <Arduino.h>
#include <Adafruit_INA260.h>
Adafruit_INA260 ina260 = Adafruit_INA260();


void initialize() {
    if (!ina260.begin(0x50)) {
        Serial.println("Couldn't find INA260 chip");
        while (1); // Halt if sensor isn't found
    }

}


void initialR(int setR) {

  float best_resistance;
  uint16_t bitstring;
  int time = 0;
  int count = 0;
  int possible_R = 543;
  float maxVoltage = 48.0;                                                                 // MAX INCOMING VOLTAGE
  const int FETs[13] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};                       // gate enabler digital pins
  const float Resistances[13] = { 22, 22, 0, 22, 22, 22, 39, 39, 75, 75, 100, 100, 239 };  // Resistance values for group 1
  int num_nmos = 13;
  for (int i = 0; i < num_nmos; i++) {  //all gates off
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }
    // find closest resistance to setR
    float minDiff = 10000000.0;
    for (int k = 0; k < possible_R; k++) {
      float diff = abs(setR - resistorLookup[k].resistance);
      if (diff < minDiff) {
        minDiff = diff;
        best_resistance = resistorLookup[k].resistance;
        bitstring = resistorLookup[k].mask;
      }

    }
    

    // output said resistance
    for (int i = 3; i < 16; ++i) {
      if (i == 3 || i == 4 || i == 5) {
        if ((bitstring >> (15 - i)) & 0x01) {
          digitalWrite(FETs[i - 3], HIGH);
        } else {
          digitalWrite(FETs[i - 3], LOW);
        }
      }
      else if ((bitstring >> (15 - i)) & 0x01) {
        digitalWrite(FETs[i - 3], HIGH);
      }

      else {
        digitalWrite(FETs[i - 3], LOW);
      }
    }



    return;


}




//dV/dI = internal resistance of generator
float calculate_targetRes(int interval, int r_old) {
    float v1 = ina260.readBusVoltage();
    float i1 = ina260.readCurrent();
    delay(interval);
    float v2 = ina260.readBusVoltage();
    float i2 = ina260.readCurrent();

    return abs(v1/i1);

    if (i2 == i1 || v2 == v1) {
        return r_old;
    }
    else {
        return abs(v1/i1);
        //return abs((v2 - v1)/(i2 - i1));
    }
}

//gradually approach new resistance, use constant K to change smoothness
float updateResistance(float r_current, float r_target, float K) {
    float error = r_target - r_current;
    return (r_current + K * error);
}




void setResistance(float targetR, int interval, int settlecount, float k) {
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
  float best_resistance;
  uint16_t bitstring;
  float setR = targetR;
  int time = 0;
  int count = 0;
  int possible_R = 543;
  float maxVoltage = 48.0;                                                                 // MAX INCOMING VOLTAGE
  const int FETs[13] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};                       // gate enabler digital pins
  const float Resistances[13] = { 22, 22, 0, 22, 22, 22, 39, 39, 75, 75, 100, 100, 239 };  // Resistance values for group 1
  int num_nmos = 13;
  for (int i = 0; i < num_nmos; i++) {  //all gates off
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }
  initialize(); //uncomment for actual circuit

  while (true) {

    // find closest resistance to setR
    float minDiff = 10000000.0;
    for (int k = 0; k < possible_R; k++) {
      float diff = abs(setR - resistorLookup[k].resistance);
      if (diff < minDiff) {
        minDiff = diff;
        best_resistance = resistorLookup[k].resistance;
        bitstring = resistorLookup[k].mask;
      }

    }
    

    // output said resistance
    for (int i = 3; i < 16; ++i) {
      if (i == 3 || i == 4 || i == 5) {
        if ((bitstring >> (15 - i)) & 0x01) {
          digitalWrite(FETs[i - 3], HIGH);
        } else {
          digitalWrite(FETs[i - 3], LOW);
        }
      }
      else if ((bitstring >> (15 - i)) & 0x01) {
        digitalWrite(FETs[i - 3], HIGH);
      }

      else {
        digitalWrite(FETs[i - 3], LOW);
      }
    }



    // reality check
    float mV = ina260.readBusVoltage()/1000;
    float mI = ina260.readCurrent()/1000;
    float aR = abs(mV/mI);
    Serial.println(aR);

    // adjust to match targetR
    if (abs(aR - targetR) > 0.2 && mV < 100 && mI < 100 && mV > .01 && mI > .001) {  
      setR = setR + k *(targetR - aR);  
    } 
    else {
      count += 1;
    }

    
    if (count >= settlecount) {
      return;;
    }
    delay(interval);


  }



}



void disconnectLoad() {
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  delay(1000);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
  Serial.println(ina260.readCurrent()/1000);
}



