import serial
import numpy as np 
import struct
import datetime
import time
import os

# [USER INPUT]
windspeed = 10
R_load = 'sweep'  # Set to 'sweep' for automatic load setting
r_load_str = "4-15"
date_string = "05-12-2025"
system_type = "Windows"  # Change to "Mac" if using Mac
port_name = "COM6"  # Change to your Arduino's serial port

if system_type == "Mac":
    def list_serial_ports():
        """Lists available serial ports on Mac."""
        import glob
        return glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/tty.usbserial*')

    available_ports = list_serial_ports()
    if not available_ports:
        raise Exception("No serial ports found. Ensure your Arduino is connected.")
    else:
        print(f"Available ports: {available_ports}")
        ser_port = available_ports[0]
    ser = serial.Serial(ser_port, 115200, timeout=0.1)

if system_type == "Windows":
    ser = serial.Serial(port_name, 115200, timeout=0.1)

def reset_arduino(serial_port):
    serial_port.setDTR(False)
    time.sleep(0.1)
    serial_port.setDTR(True)

windspeed_str = "{:02d}_{:02d}".format(int(windspeed), round((windspeed % 1) * 100))

current_datetime = datetime.datetime.now()
timestamp = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")

output_folder = f"data_logged/{date_string}"
os.makedirs(output_folder, exist_ok=True)

filename = f'{output_folder}/windspeed_{windspeed_str}_rload_{r_load_str}_{timestamp}.csv'

voltages, currents, powers, rpms, pitches, load_settings, resistances_measured = [], [], [], [], [], [], []
voltage = 0
current = 0
power = 0
rpm = 0
pitch = 0
load_setting = -1
r_measured = 0

reset_arduino(ser)
print(f"SUCCESS : Begin testing at {windspeed} m/s. Logging into {filename}...")
ser.reset_input_buffer()

try:
    while True:
        # Read the size of the struct from Arduino
        struct_format = 'fffffff'
        size = struct.calcsize(struct_format)
        data = ser.read(size)

        if len(data) == size:
            # Unpack the received data into a tuple
            unpacked_data = struct.unpack(struct_format, data)
            voltage, current, power, rpm, pitch, load_setting, r_measured = unpacked_data

            # Check for end signal (all -1)
            if all(val == -1 for val in unpacked_data):
                print("INFO : End signal received from Arduino. Ending data logging.")
                break

            voltages.append(voltage)
            currents.append(current)
            powers.append(power)
            rpms.append(rpm)
            pitches.append(pitch)
            load_settings.append(load_setting)
            resistances_measured.append(r_measured)

            # Printout
            print(f"Voltage (V): {voltage:6.2f} , Current (A): {current:6.2f} , Power (W): {power:6.2f} , "
                  f"RPM: {rpm:6.0f} , Pitch: {pitch:6.0f} , Load setting: {load_setting:6.0f}, R Measured: {r_measured:6.2f}")

except KeyboardInterrupt:
    print("\nWARNING : KeyboardInterrupt received. Finalizing and saving data...")

finally:
    voltages = np.array(voltages)
    currents = np.array(currents)
    powers = np.array(powers)
    rpms = np.array(rpms)
    pitches = np.array(pitches)
    load_settings = np.array(load_settings)
    r_measured = np.array(resistances_measured)
    windspeeds = np.full(len(voltages), windspeed)

    combined_array = np.column_stack((windspeeds, pitches, voltages, currents, powers, rpms, load_settings, r_measured))
    combined_array = combined_array[:-1]  # Optional: drop final entry if needed

    np.savetxt(filename, combined_array, delimiter=',', 
               header='Windspeed (m/s), Pitch, Voltage (V), Current (A), Power (W), RPM, R Load (Ohms), Resistance (ohm)',
               comments='', fmt='%.2f')
    print(f"SUCCESS : {filename} is saved for windspeed = {windspeed} m/s")
