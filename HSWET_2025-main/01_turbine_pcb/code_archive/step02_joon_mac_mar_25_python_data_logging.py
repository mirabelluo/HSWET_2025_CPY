import serial
import numpy as np 
import struct
import datetime
import time
import os
def list_serial_ports():
    """Lists available serial ports on Mac."""
    import glob
    return glob.glob('/dev/tty.usbmodem*') + glob.glob('/dev/tty.usbserial*')

# Detect serial port
available_ports = list_serial_ports()
if not available_ports:
    raise Exception("No serial ports found. Ensure your Arduino is connected.")
else:
    print(f"Available ports: {available_ports}")
    ser_port = available_ports[0]  # Use the first detected port

# Open serial connection
ser = serial.Serial(ser_port, 115200, timeout=0.1)

def reset_arduino(serial_port):
    serial_port.setDTR(False)
    time.sleep(0.1)
    serial_port.setDTR(True)

# [USER INPUT]
windspeed = 200
R_load = 10
date_string = "03-09-2025"

windspeed_str = "{:02d}_{:02d}".format(int(windspeed), round((windspeed % 1) * 100))
r_load_str = "{:01d}_{:01d}".format(int(R_load), round((R_load % 1) * 100))

current_datetime = datetime.datetime.now()
timestamp = current_datetime.strftime("%Y-%m-%d_%H-%M-%S")

output_folder = f"data_logged/{date_string}"
os.makedirs(output_folder, exist_ok=True)

filename = f'{output_folder}/windspeed_{windspeed_str}_m_per_s_rload_{r_load_str}_{timestamp}.csv'

voltages, currents, powers, rpms, pitches, load_settings = [], [], [], [], [], []
reset_arduino(ser)
print(f"SUCCESS : Begin testing at {windspeed} m/s. Logging into {filename}...")
ser.reset_input_buffer()

while True:
    struct_format = 'ffffff'
    size = struct.calcsize(struct_format)
    data = ser.read(size)
    if len(data) == size:
        unpacked_data = struct.unpack(struct_format, data)
        voltage, current, power, rpm, pitch, load_setting = unpacked_data
        if voltage < 0 and current < 0 and rpm < 0 and pitch < 0 and load_setting < 0:
            break  # Stop if all values are negative

        voltages.append(voltage)
        currents.append(current)
        powers.append(power)
        rpms.append(rpm)
        pitches.append(pitch)
        load_settings.append(load_setting)

        print(f"Voltage: {voltage:.2f} V, Current: {current:.2f} A, Power: {power:.2f} W, "
              f"RPM: {rpm:.0f}, Pitch: {pitch:.0f}, Load Setting: {load_setting:.0f}")

voltages, currents, powers = np.array(voltages), np.array(currents), np.array(powers)
rpms, pitches, load_settings = np.array(rpms), np.array(pitches), np.array(load_settings)
windspeeds = np.full(len(voltages), windspeed)
resistances = np.divide(voltages, currents, where=currents!=0)  # Avoid division by zero

combined_array = np.column_stack((windspeeds, pitches, voltages, currents, resistances, powers, rpms, load_settings))
combined_array = combined_array[:-1]

# Save CSV with two decimal precision
np.savetxt(filename, combined_array, delimiter=',', 
           header='Windspeed (m/s), Pitch, Voltage (V), Current (A), Resistance (ohm), Power (W), RPM, Load Setting', 
           comments='', fmt='%.2f')
print(f"SUCCESS : {filename} is saved for windspeed = {windspeed} m/s")
