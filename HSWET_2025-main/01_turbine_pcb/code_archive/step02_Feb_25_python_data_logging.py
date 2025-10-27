# Updated 03/29/2025
# Close the Arduino IDE first to free up the port
# Run the Python file to collect data
# filename = f'{output_folder}/windspeed_{windspeed_str}_rload_{r_load_str}_{timestamp}.csv'

# Resistor values too large
import serial
import numpy as np 
import struct
import datetime
import time
import os

# Open serial connection
ser = serial.Serial('COM4', 115200, timeout = 0.1)  # Adjust 'COMx' based on your Arduino's serial port

# Function to reset the Arduino
def reset_arduino(serial_port):
    serial_port.setDTR(False)
    time.sleep(0.1)
    serial_port.setDTR(True)

# [USER INPUT]
windspeed = 6.8
R_load = 10
date_string = "03-29-2025"

#manually set the windspeed during testing
windspeed_str = "{:02d}_{:02d}".format(int(windspeed), round((windspeed % 1) * 100))
r_load_str = "{:01d}_{:01d}".format(int(R_load), round((R_load % 1) * 100))  # Format R_load

current_datetime = datetime.datetime.now()
timestamp = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")

output_folder = f"data_logged/{date_string}"
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

filename = f'{output_folder}/windspeed_{windspeed_str}_rload_{r_load_str}_{timestamp}.csv'

voltages = []
currents = []
powers = []
rpms = []
pitches = []
load_settings = []

voltage = 0
current = 0
power = 0
rpm = 0
pitch = 0
load_setting = 0

reset_arduino(ser)
print(f"SUCCESS : Begin testing at {windspeed} m/s. Logging into {filename}...")
ser.reset_input_buffer()
while (voltage >= 0 or current >= 0 or rpm >= 0 or pitch >= 0 or load_setting >= 0):
    # Read the size of the struct from Arduino
    struct_format = 'ffffff'
    size = struct.calcsize(struct_format)
    data = ser.read(size)

    if len(data) == size:
        # Unpack the received data into a tuple
        unpacked_data = struct.unpack(struct_format, data)
        voltage, current, power, rpm, pitch, load_setting = unpacked_data

        voltages.append(voltage)
        currents.append(current)
        powers.append(power)
        rpms.append(rpm)
        pitches.append(pitch)
        load_settings.append(load_setting)

        voltage_str = "{:>6.2f}".format(voltage)
        current_str = "{:>6.2f}".format(current)
        power_str = "{:>6.2f}".format(power)
        rpm_str = "{:>6.0f}".format(rpm)
        pitch_str = "{:>6.0f}".format(pitch)
        load_setting_str = "{:>6.0f}".format(load_setting)

        print("Voltage (V): " + voltage_str, end="")
        print(", Current (A): " + current_str, end="")
        print(", Power (W): " + power_str, end="")
        print(", RPM: " + rpm_str, end="")
        print(", Pitch: " + pitch_str, end="")
        print(", Load setting: " + load_setting_str)

voltages = np.float64(voltages)
currents = np.float64(currents)
powers = np.float64(powers)
# powers = voltages * currents

rpms = np.array(rpms)
pitches = np.array(pitches)
load_settings = np.array(load_settings)
windspeeds = [windspeed] * len(voltages)
resistances = voltages / currents

# Remove the last row from the combined array
combined_array = np.column_stack((windspeeds, pitches, voltages, currents, resistances, powers, rpms, load_settings))
combined_array = combined_array[:-1]

# Save CSV with two decimal precision
np.savetxt(filename, combined_array, delimiter=',', 
           header='Windspeed (m/s), Pitch, Voltage (V), Current (A), Resistance (ohm), Power (W), RPM, Load Setting', 
           comments='', fmt='%.2f')
print(f"SUCCESS : {filename} is saved for windspeed = {windspeed} m/s")

