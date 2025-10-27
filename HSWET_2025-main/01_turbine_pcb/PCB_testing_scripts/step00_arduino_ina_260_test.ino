// Updated 03/06/2025
// Step00 test: Test the INA260 sensor
// Keeps looping and printing power measurements

#include <Wire.h>
#include <Adafruit_INA260.h>

// Create INA260 sensor object
Adafruit_INA260 ina260;

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Wait for serial monitor to open

    Serial.println("INA260 Voltage, Current, and Power Measurement");

    if (!ina260.begin()) {
        Serial.println("Failed to find INA260 chip!");
        while (1); // Halt execution if sensor is not found
    }
}

void loop() {
    float voltage = ina260.readBusVoltage() / 1000.0;  // Read voltage in Volts
    float current = ina260.readCurrent() / 1000.0;     // Read current in Amperes
    float power = ina260.readPower() / 1000.0;        // Read power in Watts

    // Print the values to the Serial Monitor
    Serial.print("Voltage: "); Serial.print(voltage); Serial.print(" V, ");
    Serial.print("Current: "); Serial.print(current); Serial.print(" A, ");
    Serial.print("Power: "); Serial.print(power); Serial.println(" W");

    delay(1000); // Delay 1 second before next reading
}
