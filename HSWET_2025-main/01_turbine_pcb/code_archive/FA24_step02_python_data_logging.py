# Updated 12/10/2024
# Close the Arduino IDE first to free up the port
# Run the Python file to collect data
# filename = f'{output_folder}/windspeed_{windspeed_str}_m_per_s_{timestamp}.csv"

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
windspeed = 11
R_load = 25.3

date_string = "12-13-2024"
#manually set the windspeed during testing

windspeed_str = "{:02d}_{:02d}".format(int(windspeed), int((windspeed % 1) * 100))  # Format windspeed
r_load_str = "{:01d}_{:01d}".format(int(R_load), int((R_load % 1) * 100))  # Format R_load

current_datetime = datetime.datetime.now()
timestamp = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")
# filename = f"data_{str(windspeed)}ms_{timestamp}.csv"

output_folder = f"data_logged/{date_string}"
if not os.path.exists(output_folder):
    os.makedirs(output_folder)

# filename = f'{output_folder}/windspeed_{windspeed_str}_m_per_s_{timestamp}.csv'
filename = f'{output_folder}/windspeed_{windspeed_str}_m_per_s_rload_{r_load_str}_{timestamp}.csv'

voltages = []
currents = []
rpms = []
pitches = []
load_settings = []

voltage = 0
current = 0
rpm = 0
pitch = 0
load_setting = 0

reset_arduino(ser)
print(f"SUCCESS : Begin testing at {windspeed} m/s. Logging into {filename}...")
ser.reset_input_buffer()
while (voltage >= 0 or current >= 0 or rpm >= 0 or pitch >= 0 or load_setting >= 0):
    # Read the size of the struct from Arduino
    struct_format = 'fffff'
    size = struct.calcsize(struct_format)
    data = ser.read(size)

    if len(data) == size:
        # Unpack the received data into a tuple
        unpacked_data = struct.unpack(struct_format, data)
        voltage, current, rpm, pitch, load_setting = unpacked_data
        voltages.append(voltage)
        currents.append(current)
        rpms.append(rpm)
        pitches.append(pitch)
        load_settings.append(load_setting)
        voltage_str = "{:>10.2f}".format(voltage)
        current_str = "{:>10.2f}".format(current)
        rpm_str = "{:>10.0f}".format(rpm)
        pitch_str = "{:>10.0f}".format(pitch)
        load_setting_str = "{:>10.0f}".format(load_setting)
        print("Voltage (V): " + voltage_str, end="")
        print(", Current (A): " + current_str, end="")
        print(", RPM: " + rpm_str, end="")
        print(", Pitch: " + pitch_str, end="")
        print(", Load setting: " + load_setting_str)

voltages = np.float64(voltages)
currents = np.float64(currents)
rpms = np.array(rpms)
pitches = np.array(pitches)
load_settings = np.array(load_settings)
windspeeds = [windspeed] * len(voltages)
resistances = voltages / currents
powers = voltages * currents
# Remove the last row from the combined array
combined_array = np.column_stack((windspeeds, pitches, voltages, currents, resistances, powers, rpms, load_settings))
combined_array = combined_array[:-1]

# Save the combined array to a CSV file
np.savetxt(filename, combined_array, delimiter=',', header='Windspeed (m/s), Pitch, Voltage (V), Current (A), Resistance (ohm), Power (W), RPM, Load Setting', comments='')
print(f"SUCCESS : {filename} is saved for windspeed = {windspeed} m/s")
