#ifndef RESISTOR_LOOKUP_H
#define RESISTOR_LOOKUP_H

#include <stdint.h>

// Structure holding the combination mask and its effective resistance (in ohms).
struct ResistorCombination {
  uint16_t mask;      // Each bit indicates whether a resistor is connected.
  float resistance;   // Effective parallel resistance in ohms.
};

// Declaration only – no initializer here
extern const ResistorCombination resistorLookup[543];

#endif // RESISTOR_LOOKUP_H
