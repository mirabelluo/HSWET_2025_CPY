"""
For the given optimized resistor configuration:
    Block 1: [5]                (1 switch)
    Block 2: [18]               (1 switch)
    Block 3: [7, 100, 22, 49, 49, 77, 241, 107, 71, 4]
    
Each block behaves as follows:
  - A block with a single resistor has only one switch.
  - A block with multiple resistors has a block switch plus individual switches.
    However, if the block switch is off, only one canonical configuration is generated
    (e.g. all internal switches set to 0), because the effective resistance is 0 regardless.
    
This script generates all possible overall switch configurations (with duplicates removed)
and computes the overall effective resistance, then stores each pair [[configuration], Req].
The list is sorted by Req. Then, plots the Req values using matplotlib.
"""

import itertools
import numpy as np
import matplotlib.pyplot as plt

# --- Define the Sorted Resistor Values for Each Block ---
block1 = [5]  # single-resistor block
block2 = [18] # single-resistor block
unsorted_block3 = [4, 7, 22, 50, 50, 71, 75, 100, 108, 240]
block3 = sorted(unsorted_block3) 

# --- Effective Resistance for a Single Block ---
def effective_resistance_block(resistor_values, switches):
    """
    Compute the effective resistance of a block given its resistor values and
    switch configuration.
    
    For a block with one resistor:
      - switches is a list of length 1.
        If that value is 1, the effective resistance is the resistor's value;
        if 0, then it is 0 (block is off/shorted).
      
    For a block with multiple resistors:
      - switches is a list of length 1 + number_of_resistors.
        The first element is the block switch.
      - If the block switch is 0, return 0 (ignore individual resistor switches).
      - If the block switch is 1:
            Compute the parallel effective resistance using only the resistors whose
            individual switches are 1. If none are on, return 0.
    """
    if len(resistor_values) == 1:
        return resistor_values[0] if switches[0] == 1 else 0.0
    else:
        if switches[0] == 0:
            return 0.0
        # Block is on: consider individual resistor switches.
        internal_switches = switches[1:]
        sum_recip = 0.0
        for r, s in zip(resistor_values, internal_switches):
            if s == 1:
                sum_recip += 1.0 / r
        return 1.0 / sum_recip if sum_recip != 0.0 else 0.0

# --- Overall Effective Resistance ---
def overall_effective_resistance(config, blocks):
    """
    Given a complete switch configuration (a 1D list) and the list of blocks,
    compute the overall effective resistance of the series-connected blocks.
    """
    idx = 0
    total_req = 0.0
    for block in blocks:
        if len(block) == 1:
            # One switch for a single-resistor block.
            switches = config[idx: idx + 1]
            idx += 1
            total_req += effective_resistance_block(block, switches)
        else:
            n = len(block)
            switches = config[idx: idx + 1 + n]  # block switch + individual resistor switches.
            idx += (1 + n)
            total_req += effective_resistance_block(block, switches)
    return total_req

# --- Configuration Generators for Each Block ---
def generate_block_configurations(block):
    """
    Generates the switch configurations for a given block.
    For a single-resistor block, generate [[0]] and [[1]].
    For a multi-resistor block:
      - Always generate the "off" configuration: [0] + [0]*len(block).
      - For the "on" case (first switch is 1), generate all combinations for the individual resistors.
    Returns a list of lists (each inner list is the switch configuration for the block).
    """
    if len(block) == 1:
        return [[0], [1]]
    else:
        configs = []
        # Off configuration: block switch off => canonical configuration.
        off_config = [0] + [0] * len(block)
        configs.append(off_config)
        # On configurations: block switch on + all combinations of internal switches.
        n = len(block)
        # Generate all combinations for n individual switches.
        for bits in itertools.product([0, 1], repeat=n):
            # Only include "on" configuration if at least one resistor is on.
            if any(bits):
                config = [1] + list(bits)
                configs.append(config)
        return configs

# --- Generate Overall Configurations and Compute Req ---
def generate_all_configurations():
    """
    Generate every overall switch configuration for the three blocks (with no duplicate redundant entries)
    and compute the overall effective resistance.
    Block possibilities:
      - Block 1: 2 possibilities.
      - Block 2: 2 possibilities.
      - Block 3: For a multi-resistor block with 10 resistors, generate 1 (off) + (2^10 - 1) = 1 + 1023 = 1024 possibilities.
    Total combinations: 2 * 2 * 1024 = 4096.
    
    Returns a list of entries [[configuration], Req].
    """
    block1_configs = generate_block_configurations(block1)
    block2_configs = generate_block_configurations(block2)
    block3_configs = generate_block_configurations(block3)
    
    all_configurations = []
    # Iterate over the Cartesian product of configurations for each block.
    for conf1 in block1_configs:
        for conf2 in block2_configs:
            for conf3 in block3_configs:
                overall_conf = conf1 + conf2 + conf3
                Req = overall_effective_resistance(overall_conf, [block1, block2, block3])
                all_configurations.append([overall_conf, Req])
    return all_configurations

import numpy as np

def linearize_data(data, tolerance=0.87, use_max_step=True):
    """
    Filters data points to yield a linearized set based on an ideal step size.
    
    Parameters:
        data (list): Input data where each element is of the form [[notimportant], R].
        tolerance (float): Relative tolerance for including a step.
        use_max_step (bool): Whether to use the maximum step as the ideal step. 
                             If False, the median step is used.
    Returns:
        list: The filtered data following the ideal step size.
    """
    # Extract the resistor values.
    resistor_values = np.array([item[1] for item in data])
    
    # Compute differences between consecutive resistor values.
    steps = np.diff(resistor_values)
    
    # Choose the ideal step size.
    if use_max_step:
        ideal_step = np.max(steps)
    else:
        ideal_step = np.median(steps)
    
    # Initialize the filtered data list with the first point.
    filtered_data = [data[0]]
    last_value = resistor_values[0]
    
    # Iterate over the remaining data.
    for i in range(1, len(resistor_values)):
        current_value = resistor_values[i]
        step = current_value - last_value
        
        # If the step is within the accepted tolerance, include this data point.
        if abs(step - ideal_step) <= tolerance * ideal_step:
            filtered_data.append(data[i])
            last_value = current_value
            
    return filtered_data


if __name__ == '__main__':
    all_configs = generate_all_configurations()
    print(f"Generated {len(all_configs)} configurations.")
        
    # filter out any configurations with Req == 0
    all_configs = [cfg for cfg in all_configs if cfg[1] != 0.0]
    # Ignore values over 50
    all_configs = [cfg for cfg in all_configs if cfg[1] <= 40.0]
    
    print(f"Configurations after filtering out Req == 0: {len(all_configs)}")
    
    # Sort configurations by overall effective resistance.
    all_configs_sorted = sorted(all_configs, key=lambda x: x[1])
    
    # Extract Req values for plotting
    Req_values = [entry[1] for entry in all_configs_sorted]
    
    # Assuming Req_values is a sorted list or numpy array of Req values.
    Req_values = np.array(Req_values)  # e.g., obtained from your sorted configurations

    # Compute differences between adjacent Req values
    diffs = np.diff(Req_values)

    # Find the largest step size (maximum difference) and its index
    max_diff = np.max(diffs)
    max_index = np.argmax(diffs)

    print(f"The largest step is: {max_diff:.3f} Ohms")
    print(f"This gap occurs between configuration indices {max_index} and {max_index + 1}, " \
          f"with Req values: {Req_values[max_index]:.3f} Ohms and {Req_values[max_index + 1]:.3f} Ohms")
    
    lin_data = linearize_data(all_configs_sorted)
    Req_values_lin = np.array([entry[1] for entry in lin_data])
    
    print(f"Linearized data points: {len(Req_values_lin)}")    
    print(f"The average gap in the linearized data is: {np.average(np.diff(Req_values_lin))}")
    
    # Define the output log file name.
    output_filename = "rFinal_out.log"

    with open(output_filename, "w") as log_file:
        # Iterate over each configuration and its corresponding resistance.
        for config, resistance in lin_data:
            # Convert the binary list to an integer, assuming the first bit is the MSB.
            hex_val = 0
            for bit in config:
                hex_val = (hex_val << 1) | bit

            # Format the integer as a hexadecimal value (zero-padded to at least 4 digits)
            # and the resistance as a float with six decimal places and an appended 'f'.
            log_file.write("  {0x%04X, %s},\n" % (hex_val, resistance))

    print(f"Linearized data has been written to {output_filename}")
    
    # Create a figure with two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(18, 8), sharex=False)

    # First subplot: Sorted Configurations
    ax1.plot(Req_values, marker='o', linestyle='-', markersize=3)
    ax1.set_xlabel("Configuration Index")
    ax1.set_ylabel("Equivalent Resistance (Ohms)")
    ax1.set_title("Equivalent Resistance for Raw Configurations")
    ax1.grid(True)

    # Second subplot: Linearized Data
    ax2.plot(Req_values_lin, marker='o', linestyle='-', markersize=3)
    ax2.set_xlabel("Configuration Index (Linearized)")
    ax2.set_ylabel("Equivalent Resistance (Ohms)")
    ax2.set_title("Equivalent Resistance for Linearized Data")
    ax2.grid(True)

    # Adjust layout for better spacing between subplots
    plt.tight_layout()
    plt.show()