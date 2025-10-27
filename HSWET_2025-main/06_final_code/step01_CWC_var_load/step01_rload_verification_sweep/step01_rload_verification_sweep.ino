#include "r_target.h"
#include "resistor_lookup.h"

const int FETs[13] = {38, 36, 34, 53, 51, 49, 47, 45, 9, 10, 11, 12, 13};                       // gate enabler digital pins

void setup() {
  Serial.begin(9600);
  
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);

  initialize();

  // This is a quick test to print the resistance values and their corresponding one-hot bitstrings
  // To make sure that all the resistors are working correctly
  // This function def is in r_target.cpp
  testAllResistors(0.5);
  // ----------------------------------------------------------------------
  disconnectLoad();
  delay(999999099);
}

void loop() {
  // do nothing
}

void printTestedValuesAndBitstrings() {
  Serial.println("Index\tResistance (Ohm)\tOne-hot Bitstring");

  for (int i = 0; i < 543; ++i) {
    float resistance = resistorLookup[i].resistance;
    uint16_t mask = resistorLookup[i].mask;

    // Print index and resistance
    Serial.print(i);
    Serial.print("\t");
    Serial.print(resistance, 6);
    Serial.print("\t\t");

    // Convert 13-bit one-hot bitstring (FETs[0] = LSB)
    for (int b = 12; b >= 0; --b) {
      Serial.print((mask >> b) & 0x01);
    }
    Serial.println();
  }
}
