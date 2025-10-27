def ltc4367_uv_ov_resistor_calculator(UVTH, OVTH, VOS_UV=0.003, ILEAK=10e-9):
    """
    Calculates R1, R2, and R3 resistor values for setting UV and OV thresholds
    on the LTC4367 based on the datasheet procedure.

    Parameters:
    - UVTH: Desired undervoltage threshold in volts
    - OVTH: Desired overvoltage threshold in volts
    - VOS_UV: Voltage error budget (default: 3 mV)
    - ILEAK: Leakage current (default: 10 nA)

    Returns:
    - R1, R2, R3 in ohms
    """

    # Step 1: Total resistance for UV leg
    R1_plus_R2 = VOS_UV / ILEAK  # ohms

    # Step 2: R3 calculation
    R3 = R1_plus_R2 * (UVTH - 0.5) / 0.5

    # Step 3: R1 and R2 based on OV threshold
    R1 = (R1_plus_R2 + R3) * 0.5 / OVTH
    R2 = R1_plus_R2 - R1

    return R1, R2, R3


# Example usage:
# uv_threshold = 3.5  # volts
# ov_threshold = 18.0  # volts

uv_threshold = 1 # volts
ov_threshold = 46.0  # volts


R1, R2, R3 = ltc4367_uv_ov_resistor_calculator(uv_threshold, ov_threshold)

print(f"UV and OV thresholds: {uv_threshold}V, {ov_threshold}V")
print(f"R1 = {R1/1e3:.2f} kΩ")
print(f"R2 = {R2/1e3:.2f} kΩ")
print(f"R3 = {R3/1e3:.2f} kΩ")
