import numpy as np
import matplotlib.pyplot as plt
import random
import math

# --- Helper Functions for Partitioning ---

def partitions(n, min_part=1):
    """
    Generate all partitions of n as a list of integers, with parts >= min_part.
    Parts are generated in non-decreasing order.
    """
    if n == 0:
        yield []
    else:
        for i in range(min_part, n+1):
            for tail in partitions(n-i, i):
                yield [i] + tail

# --- Functions to Compute Effective Resistance ---

def compute_block_effective_values(block_values):
    """
    Given a list of resistor values for one parallel block, compute all effective resistances.
    Now we include the case when the block is "off" (mask 0), which is defined as 0 Ω
    (i.e. the block is shorted out).
    For any nonzero mask, the effective resistance is:
         R_eff = 1 / (sum(1/R_i) for each resistor i turned on)
    """
    k = len(block_values)
    eff_values = []
    for mask in range(0, 2**k):  # include mask 0
        if mask == 0:
            # Block is off (shorted): effective resistance is 0 Ω.
            eff_values.append(0.0)
        else:
            sum_recip = 0.0
            for i in range(k):
                if mask & (1 << i):
                    sum_recip += 1.0 / block_values[i]
            eff_values.append(1.0 / sum_recip)
    return np.array(eff_values)

def compute_overall_effective_values(blocks):
    """
    Given a series of blocks (each block is a list of resistor values), compute all overall
    effective resistances. For each block, compute its set of effective resistances (now including
    the off-state, which is 0 Ω), and since blocks are in series, the overall effective resistance is
    the sum (Cartesian sum) of one effective value per block.
    """
    block_effects = [compute_block_effective_values(block) for block in blocks]
    overall = block_effects[0]
    for be in block_effects[1:]:
        overall = np.add.outer(overall, be).flatten()  # Cartesian sum over series blocks
    return overall

# --- Cost Function (Linearity in Target Region) ---

def cost_function_topology(blocks, region_min, region_max, min_effective_count=50, penalty_weight=1.5):
    """
    Compute a cost that measures:
      1. How evenly spaced (linear) the effective resistances are in the target region,
      2. And penalizes if the number of effective resistances in that region is below a desired minimum.
      
    Parameters:
      blocks: List of blocks (each block is a list of resistor values).
      region_min, region_max: The target region (e.g. 2 to 50 ohms).
      min_effective_count: Minimum desired count of effective resistance values in the region.
      penalty_weight: Scaling factor for the penalty if the count is below the threshold.
    """
    eff = compute_overall_effective_values(blocks)
    valid = (eff >= region_min) & (eff <= region_max)
    eff_region = eff[valid]
    count = len(eff_region)
    
    # Penalty for having too few effective values in the target region:
    if count < min_effective_count:
        count_penalty = penalty_weight * (min_effective_count - count) ** 2
    else:
        count_penalty = 0.0
        
    # If there are too few points, return a very high cost.
    if count < 2:
        return 1e6

    eff_sorted = np.sort(eff_region)
    diffs = np.diff(eff_sorted)
    mean_diff = np.mean(diffs)
    var_diffs = np.mean((diffs - mean_diff) ** 2)
    
    # Also penalize if the effective values don't span the full desired range.
    range_coverage = eff_sorted[-1] - eff_sorted[0]
    desired_range = region_max - region_min
    range_penalty = (desired_range - range_coverage) ** 2
    
    total_cost = var_diffs + range_penalty + count_penalty
    return total_cost


# --- Simulated Annealing on Resistor Values for a Given Topology ---

def perturb_blocks(blocks, R_min, R_max):
    """
    Randomly select one resistor in one block and perturb its value by a small integer delta.
    Returns a new deep-copied configuration.
    """
    new_blocks = [list(b) for b in blocks]  # deep copy
    block_index = random.randint(0, len(new_blocks)-1)
    resistor_index = random.randint(0, len(new_blocks[block_index])-1)
    delta = random.randint(-10, 10)
    new_value = new_blocks[block_index][resistor_index] + delta
    new_value = max(R_min, min(R_max, new_value))
    new_blocks[block_index][resistor_index] = round(new_value)
    return new_blocks

def optimize_topology(partition, R_min, R_max, region_min, region_max, iterations=10000):
    """
    For a given topology (partition, e.g. [6,3,3] for N=12), initialize each resistor with a median value
    and optimize (via simulated annealing) the resistor values to minimize the cost function.
    Returns the optimized blocks (list of lists) and the best cost.
    """
    # Initialize each block with all resistor values set to the median value.
    initial_value = (R_min + R_max) // 2
    blocks = []
    for size in partition:
        blocks.append([initial_value] * size)
    
    current_blocks = blocks
    current_cost = cost_function_topology(current_blocks, region_min, region_max)
    best_blocks = current_blocks
    best_cost = current_cost
    T = 1.0
    T_min = 1e-6
    alpha = 0.999
    for it in range(iterations):
        candidate_blocks = perturb_blocks(current_blocks, R_min, R_max)
        candidate_cost = cost_function_topology(candidate_blocks, region_min, region_max)
        if candidate_cost < current_cost or random.random() < math.exp(-(candidate_cost - current_cost) / T):
            current_blocks = candidate_blocks
            current_cost = candidate_cost
            if candidate_cost < best_cost:
                best_cost = candidate_cost
                best_blocks = candidate_blocks
        T = max(T * alpha, T_min)
    return best_blocks, best_cost

# --- Overall Search Over Topologies ---

if __name__ == "__main__":
    # Parameters
    N = 12  # Total number of resistors
    R_min = 1    # Minimum allowed resistor value (ohms)
    R_max = 500  # Maximum allowed resistor value (ohms)
    region_min = 2   # Target effective resistance region: lower bound (ohms)
    region_max = 40.0  # Target effective resistance region: upper bound (ohms)
    iterations = 5000  # Annealing iterations per topology
    
    best_overall_cost = 1e9
    best_overall_config = None
    best_partition = None
    
    print("Searching over candidate topologies (partitions of {}):".format(N))
    # Enumerate candidate partitions (topologies). We limit to partitions with up to 4 blocks.
    for partition in partitions(N, 1):
        if len(partition) > 4:
            continue
        print("Trying partition:", partition)
        optimized_blocks, cost_val = optimize_topology(partition, R_min, R_max, region_min, region_max, iterations)
        print(" Partition:", partition, "Cost:", cost_val, "Optimized blocks:", optimized_blocks)
        if cost_val < best_overall_cost:
            best_overall_cost = cost_val
            best_overall_config = optimized_blocks
            best_partition = partition
            
    print("\nBest overall configuration found:")
    print(" Partition (block sizes):", best_partition)
    print(" Resistor values per block:", best_overall_config)
    print(" Achieved cost:", best_overall_cost)
    
    # Optionally, compute and display the overall effective resistance distribution for the best configuration:
    overall_eff = compute_overall_effective_values(best_overall_config)
    overall_eff_in_region = overall_eff[(overall_eff >= region_min) & (overall_eff <= region_max)]
    overall_eff_sorted = np.sort(overall_eff_in_region)
    print("\nNumber of effective values in target region: {} out of {} total.".format(
          len(overall_eff_sorted), len(overall_eff)))
    print("Effective resistances in target region (first 10):", overall_eff_sorted[:10])
    
    plt.figure(figsize=(10, 6))
    plt.plot(overall_eff_sorted, marker='o', linestyle='-', markersize=3)
    plt.xlabel("Index")
    plt.ylabel("Effective Resistance (Ohms)")
    plt.title("Effective Resistances in Target Region")
    plt.grid(True)
    plt.show()
