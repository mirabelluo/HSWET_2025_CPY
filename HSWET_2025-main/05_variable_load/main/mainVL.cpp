#include "r_target.h"
#include "resistor_lookup.h"

const int FETs[13] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};                       // gate enabler digital pins


void setup() {
  Serial.begin(9600);
  /*
  int num_nmos = 13;
  for (int i = 0; i < num_nmos; i++) {  //all gates off
    pinMode(FETs[i], OUTPUT);
    digitalWrite(FETs[i], LOW);
  }
  initialR(20);
  */
  initialize();
    pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

}

void loop() {

  // targetresistance (ohms), interval (ms), iterations, kp
  setResistance(19, 20, 5, .2);
  Serial.println("Done!");
  delay(1000);
  
  setResistance(35, 20, 5, .2);
  Serial.println("Done!");

  delay(1000);

  disconnectLoad();
  delay(999999099);

}

















