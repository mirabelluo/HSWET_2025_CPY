import numpy as np
import matplotlib.pyplot as plt

# --- Wind Speed Generator ---
def generate_wind_speed(t):
    base_speed = 8 + 2 * np.sin(0.01 * t)
    gusts = np.random.normal(0, 0.5)
    return max(base_speed + gusts, 0.1)

# --- Turbine Generator Model ---
def turbine_generator_model(wind_speed):
    rotor_speed = wind_speed
    V_oc = 2.0 * rotor_speed  # Open-circuit voltage scales with wind speed

    # Internal resistance fluctuates nonlinearly between 1 and 40 Ohms
    R_internal = 1 + 39 * (np.sin(0.02 * rotor_speed + np.random.normal(0, 0.2)) * 0.5 + 0.5)
    return V_oc, R_internal

# --- Black Box Measurement Function ---
def measure_blackbox(V_oc, R_internal, R_external):
    total_R = R_internal + R_external
    I = V_oc / total_R
    V = I * R_external  # Voltage across the external load
    return V, I

# --- PID Controller Class ---
class PID:
    def __init__(self, kp, ki, kd):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.prev_error = 0
        self.integral = 0

    def update(self, error, dt):
        self.integral += error * dt
        derivative = (error - self.prev_error) / dt
        self.prev_error = error
        return self.kp * error + self.ki * self.integral + self.kd * derivative

# --- Simulation Parameters ---
pid = PID(kp=0.5, ki=0.05, kd=0.02)
R_ext = 5.0
delta = 0.5
timesteps = 300

# --- Data Logging ---
times = []
wind_speeds = []
Vocs = []
Rints = []
Rext_vals = []
Rest_est = []

# --- Simulation Loop ---
for t in range(1, timesteps):
    wind_speed = generate_wind_speed(t)
    V_oc, R_true = turbine_generator_model(wind_speed)

    # Take two measurements with slightly different external resistances
    V1, I1 = measure_blackbox(V_oc, R_true, R_ext)
    V2, I2 = measure_blackbox(V_oc, R_true, R_ext + delta)

    # Estimate internal resistance from blackbox measurements
    if abs(I2 - I1) > 1e-6:
        R_est = (V1 - V2) / (I2 - I1)
    else:
        R_est = R_true  # fallback in case of divide-by-zero

    # Update external resistance using PID
    error = R_est - R_ext
    R_ext += pid.update(error, dt=1)
    R_ext = max(1.0, min(40.0, R_ext))  # Clamp to 1-40Ω

    # Log data
    times.append(t)
    wind_speeds.append(wind_speed)
    Vocs.append(V_oc)
    Rints.append(R_true)
    Rext_vals.append(R_ext)
    Rest_est.append(R_est)

# --- Plotting Results ---
plt.figure(figsize=(12, 6))

plt.subplot(2, 1, 1)
plt.plot(times, wind_speeds, label="Wind Speed (m/s)")
plt.plot(times, Vocs, label="Voc (V)")
plt.ylabel("Wind / Voltage")
plt.legend()
plt.grid(True)

plt.subplot(2, 1, 2)
plt.plot(times, Rints, label="True R_internal")
plt.plot(times, Rest_est, label="Estimated R_internal (from I, V)", linestyle='dashed')
plt.plot(times, Rext_vals, label="External R (Controlled)")
plt.xlabel("Time")
plt.ylabel("Resistance (Ohms)")
plt.title("Real-Time Resistance Matching Using Blackbox Current & Voltage")
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.show()
