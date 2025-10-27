/**
 * @file step01_three_channel_load_resistance_calculator.ino
 * @brief Resistance optimization program for a 3-channel parallel load system.
 *
 * This Arduino program is designed to optimize load resistance in a 3-channel system. 
 * It measures incoming analog voltage, calculates the equivalent resistance for each 
 * possible combination of channel states, and determines the best configuration 
 * to minimize resistance while ensuring current remains below a maximum threshold.
 *
 * ## Features
 * - Reads incoming analog voltage using an analog pin.
 * - Applies linear scaling to convert the analog value to a float voltage.
 * - Manages 3 parallel resistive channels, each with an ON/OFF state encoded as 0 or 1.
 * - Sweeps through all 2^3 = 8 combinations of channel states and records the best combination.
 * - Measures and outputs the optimized equivalent resistance, voltage, and current.
 * - Loops through the selected combination, toggling the respective pins HIGH or LOW.
 * - Outputs diagnostic data through the serial monitor.
 *
 * ## Future Improvements
 * - Calibration for accurate measurements.
 * - Debugging for edge cases where no paths are active.
 * - Expansion to handle 8 channels for actual implementation (2^8 = 256 combinations).
 *
 * ## Author
 * Hopkins Student Wind Energy Team, JHU
 * Updated 11/14/2024 by John Oak
 *
 * ## Connections
 * - Analog input voltage: Pin A0 (`analogPin`).
 * - Digital pins for resistive paths: Pins 2, 3, and 4 (`resistorPins`).
 * - Resistor values: 15 Ω, 15.3 Ω, 3.3 Ω (`resistances` array).
 *
 * ## Usage
 * - Upload the program to the Arduino.
 * - Connect resistive loads to the specified pins.
 * - Monitor serial output for the best resistance configuration and measured data.
 */

const int analogPin = A0;              // Analog pin for reading incoming voltage
const float maxVoltage = 48.0;         // Maximum incoming voltage
const int resistorPins[3] = {2, 3, 4}; // Digital pins for controlling resistive paths
const float resistances[3] = {15, 15.3, 3.3}; // Resistance values for each path (ohms)
const float maxCurrent = 2.5;          // Maximum allowable current (amps)

void setup() {
  Serial.begin(9600);

  // Initialize all gates to OFF
  for (int i = 0; i < 3; i++) {
    pinMode(resistorPins[i], OUTPUT);
    digitalWrite(resistorPins[i], LOW);
  }
}

void loop() {
  // Step 1: Read incoming analog voltage and convert to real voltage
  int analogValue = analogRead(analogPin);
  float voltage = (analogValue / 982.0) * maxVoltage;

  Serial.print("AnalogRead: ");
  Serial.println(analogValue);

  // Step 2: Determine the best combination of paths
  int bestCombination = 0;
  float lowestResistance = 0;

  for (int i = 1; i < 64; i++) { // Sweep through all 2^6 combinations
    float equivalentResistance = calculateParallelResistance(i);
    float current = voltage / equivalentResistance;

    // Update the best combination based on resistance and current threshold
    if (current <= maxCurrent && equivalentResistance < lowestResistance) {
      lowestResistance = equivalentResistance;
      bestCombination = i;
    }
  }

  // Step 3: Activate paths for the best combination
  int tempBest = bestCombination;

  for (int i = 0; i < 3; i++) {
    int remBest = tempBest % 2;
    tempBest /= 2;

    if (remBest == 1) {
      digitalWrite(resistorPins[i], HIGH); // Open path
    } else {
      digitalWrite(resistorPins[i], LOW);  // Close path
    }
  }

  // Output results to the serial monitor
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.print(" V, ");
  Serial.print("Equivalent Resistance: ");
  Serial.print(lowestResistance);
  Serial.print(" Ohms, ");
  Serial.print("Current: ");
  Serial.print(voltage / lowestResistance);
  Serial.println(" A");

  delay(500); // Delay for stability
}

/**
 * @brief Calculates the equivalent resistance for a given combination of channels.
 * @param combination An integer representing the ON/OFF state of each channel in binary.
 * @return The equivalent resistance of the active channels.
 */
float calculateParallelResistance(int combination) {
  float reciprocalSum = 0.0;
  int tempCombination = combination;

  for (int i = 0; i < 3; i++) {
    int rem = tempCombination % 2;
    tempCombination /= 2;

    if (rem == 1) {
      reciprocalSum += 1.0 / resistances[i];
    }
  }

  if (reciprocalSum == 0.0) {
    return 1.0; // Prevent division by zero
  }

  return 1.0 / reciprocalSum;
}
