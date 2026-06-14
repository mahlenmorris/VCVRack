#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import os

def generate_samples(distribution, bias, num_samples=100000):
    dist_clamped = max(0.0, min(4.0, distribution))
    
    min_range = max(0.0, bias)
    max_range = min(1.0, 1.0 + bias)

    # A primary uniform baseline that correlates the morphs.
    U1 = np.random.uniform(min_range, max_range, size=num_samples)
    
    if dist_clamped <= 1.0:
        v1 = (U1 + np.random.uniform(min_range, max_range, size=num_samples) + np.random.uniform(min_range, max_range, size=num_samples)) / 3.0
        return 0.5 + (v1 - 0.5) * dist_clamped
        
    elif dist_clamped <= 2.0:
        v1 = (U1 + np.random.uniform(min_range, max_range, size=num_samples) + np.random.uniform(min_range, max_range, size=num_samples)) / 3.0
        return v1 + (U1 - v1) * (dist_clamped - 1.0)
        
    elif dist_clamped <= 3.0:
        temp_val = (U1 - 0.5) / (2.0 ** 4.0)
        v3 = (np.sign(temp_val) * np.power(np.abs(temp_val), 1.0 / 5.0)) + 0.5
        return U1 + (v3 - U1) * (dist_clamped - 2.0)
        
    else:
        temp_val = (U1 - 0.5) / (2.0 ** 4.0)
        v3 = (np.sign(temp_val) * np.power(np.abs(temp_val), 1.0 / 5.0)) + 0.5
        v4 = np.where(U1 >= 0.5, 1.0, 0.0)
        return v3 + (v4 - v3) * (dist_clamped - 3.0)

def main():
    # Make sure output directory exists
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'distributions')
    os.makedirs(output_dir, exist_ok=True)
    cpp_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    os.makedirs(cpp_dir, exist_ok=True)
    
    num_bins = 61
    
    all_counts = []

    # We want distribution from 0.0 to 4.0 in steps of 0.1
    for i in range(41):
        dist_val = i / 10.0
        samples = generate_samples(dist_val, 0.0, num_samples=1000000)
        
        # Calculate histogram
        counts, bin_edges = np.histogram(samples, bins=num_bins, range=(0.0, 1.0))
        
        # Normalize
        max_count = np.max(counts)
        if max_count > 0:
            counts = counts / max_count
            
        all_counts.append(counts)

        fig, ax = plt.subplots(figsize=(2, 1), dpi=100) # Small size for UI
        
        # Plot as a bar chart. Defaulting to white with no border.
        ax.bar(bin_edges[:-1], counts, width=(1.0/num_bins), align='edge', color='white', edgecolor='none')
        
        # Remove axes, background, etc.
        ax.axis('off')
        fig.patch.set_alpha(0.0)
        ax.patch.set_alpha(0.0)
        
        # Save as PNG
        filename = f"dist_{dist_val:.1f}.png"
        filepath = os.path.join(output_dir, filename)
        plt.savefig(filepath, transparent=True, bbox_inches='tight', pad_inches=0)
        plt.close(fig)
        print(f"Saved {filepath}")

    for i in range(-5, 6):
        bias = i / 5
        samples = generate_samples(3.4, bias, num_samples=1000000)
        
        # Calculate histogram
        counts, bin_edges = np.histogram(samples, bins=num_bins, range=(0.0, 1.0))
        
        # Normalize
        max_count = np.max(counts)
        if max_count > 0:
            counts = counts / max_count
            
        all_counts.append(counts)

        fig, ax = plt.subplots(figsize=(2, 1), dpi=100) # Small size for UI
        
        # Plot as a bar chart. Defaulting to white with no border.
        ax.bar(bin_edges[:-1], counts, width=(1.0/num_bins), align='edge', color='white', edgecolor='none')
        
        # Remove axes, background, etc.
        ax.axis('off')
        fig.patch.set_alpha(0.0)
        ax.patch.set_alpha(0.0)
        
        # Save as PNG
        filename = f"bias_{bias:.1f}.png"
        filepath = os.path.join(output_dir, filename)
        plt.savefig(filepath, transparent=True, bbox_inches='tight', pad_inches=0)
        plt.close(fig)
        print(f"Saved {filepath}")

    # Generate C++ files
    cpp_header_path = os.path.join(cpp_dir, 'distribution_graphs.h')
    with open(cpp_header_path, 'w') as f:
        f.write('#pragma once\n\n')
        f.write(f'constexpr int NUM_DISTRIBUTION_GRAPHS = {len(all_counts)};\n')
        f.write(f'constexpr int NUM_DISTRIBUTION_BINS = {num_bins};\n\n')
        f.write('extern const float DISTRIBUTION_GRAPHS[NUM_DISTRIBUTION_GRAPHS][NUM_DISTRIBUTION_BINS];\n')
        
    print(f"Saved {cpp_header_path}")

    cpp_source_path = os.path.join(cpp_dir, 'distribution_graphs.cpp')
    with open(cpp_source_path, 'w') as f:
        f.write('#include "distribution_graphs.h"\n\n')
        f.write('const float DISTRIBUTION_GRAPHS[NUM_DISTRIBUTION_GRAPHS][NUM_DISTRIBUTION_BINS] = {\n')
        for i, counts in enumerate(all_counts):
            f.write(f'  // Distribution {i / 10.0:.1f}\n')
            f.write('  {\n    ')
            formatted_counts = [f"{c:.5f}f" for c in counts]
            for j in range(0, len(formatted_counts), 10):
                f.write(', '.join(formatted_counts[j:j+10]))
                if j + 10 < len(formatted_counts):
                    f.write(',\n    ')
            f.write('\n  }')
            if i < len(all_counts) - 1:
                f.write(',')
            f.write('\n')
        f.write('};\n')

    print(f"Saved {cpp_source_path}")

if __name__ == '__main__':
    main()