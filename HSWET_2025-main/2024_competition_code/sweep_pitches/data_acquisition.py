import serial
import numpy as np 
import struct
import datetime
import time
import os
import sys

# Open serial connection
ser = serial.Serial('COM10', 115200, timeout = 0.1)  # Adjust 'COMx' based on your Arduino's serial port

# Function to reset the Arduino
def reset_arduino(serial_port):
    serial_port.setDTR(False)
    time.sleep(0.1)
    serial_port.setDTR(True)


def scientific_to_float(scientific_str):
    try:
        float_val = float(scientific_str)
        formatted_float = f"{float_val:.3f}"
        return formatted_float
    except ValueError:
        return None

#manually set the windspeed
windspeed = 10
current_datetime = datetime.datetime.now()
timestamp = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")
filename = f"data_{str(windspeed)}ms_{timestamp}.csv"

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
ser.reset_input_buffer()
try: 
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
            voltage_str = "{:>10.3f}".format(voltage)
            current_str = "{:>10.3f}".format(current)
            rpm_str = "{:>10.0f}".format(rpm)
            pitch_str = "{:>10.0f}".format(pitch)
            load_setting_str = "{:>10.0f}".format(load_setting)
            print("Voltage: " + voltage_str, end="")
            print(", Current: " + current_str, end="")
            print(", RPMs: " + rpm_str, end="")
            print(", Pitch: " + pitch_str, end="")
            print(", Load setting: " + load_setting_str)
    raise KeyboardInterrupt



except KeyboardInterrupt:
    voltages = np.float64(voltages)
    currents = np.float64(currents)
    rpms = np.array(rpms)
    pitches = np.array(pitches)
    load_settings = np.array(load_settings)
    windspeeds = [windspeed] * len(voltages)
    resistances = voltages / currents
    powers = voltages * currents
    combined_array = np.column_stack((windspeeds, voltages, currents, resistances, powers, rpms, pitches, load_settings))
    # Save the combined array to a CSV file
    np.savetxt(filename, combined_array, delimiter=',', header='Windspeeds, Voltages, Currents, Resistances, Powers, RPMs, Pitches, Load settings', comments='')

    try:
        sys.exit(130)
    except SystemExit:
        os._exit(130)