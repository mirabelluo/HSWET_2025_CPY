#include "resistor_lookup.h"
#include <math.h>

/*
Example usage of the look up table
*/

void setup() {
  Serial.begin(9600);
}

void loop() {
  float targetResistance = 9.2; // desired effective resistance in ohms
  uint8_t bestMask = 0;
  float bestError = INFINITY;
  
  for (int i = 0; i < 543; i++) { 
    float error = fabs(resistorLookup[i].resistance - targetResistance);
    if (error < bestError) {
      bestError = error;
      bestMask = i;
    }
  }
  
  Serial.print("Best mask: 0x");
  Serial.println(resistorLookup[bestMask].mask, BIN);
  Serial.print("Achieved resistance: ");
  Serial.println(resistorLookup[bestMask].resistance);
  delay(5000);
}
