# Python script to generate lookup table entries for parallel resistor combinations.
import math

resistor_values = [3.0, 11.0, 40.0, 45.0, 59.0, 115.0, 158.0, 236.0]
#resistor_values = [3.0, 10.0, 21.0, 55.0, 74.0, 198.0, 239.0, 297.0, 434.0, 479.0]             #10 1024
#resistor_values = [3.0, 11.0, 24.0, 56.0, 145.0, 224.0, 262.0, 310.0, 332.0, 349.0, 465.0]     #11 2048
#resistor_values = [3.0, 9.0, 19.0, 61.0, 63.0, 115.0, 261.0, 310.0, 387.0, 413.0, 445.0, 500.0]#12 4096

print("resistor_values =", resistor_values)

# List to hold tuples of (mask, effective resistance)
combinations = []

# Calculate effective resistance for each mask (0x00 to 0xFF)
for mask in range(pow(2, len(resistor_values))):
    if mask == 0:
        effective = float('inf')  # Open circuit when no resistor is selected.
    else:
        sum_reciprocal = 0.0
        for i in range(len(resistor_values)):
            if mask & (1 << i):
                sum_reciprocal += 1.0 / resistor_values[i]
        effective = 1.0 / sum_reciprocal
    combinations.append((mask, effective))

# Sort combinations by effective resistance (ascending order)
combinations.sort(key=lambda x: x[1])

# Print sorted lookup table entries in the desired format
for mask, effective in combinations:
    if math.isinf(effective):
        eff_str = "INFINITY"
    else:
        eff_str = f"{effective:f}f"
    print("  {0x%04X, %s}," % (mask, eff_str))
