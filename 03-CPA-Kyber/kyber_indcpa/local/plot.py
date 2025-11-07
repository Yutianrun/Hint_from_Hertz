#!/usr/bin/env python3

import argparse
import glob
import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from tqdm import tqdm


def parse_file(fn):
    """Parse energy and frequency data from file"""
    energy = []
    freq = []
    with open(fn) as f:
        for line in f:
            a, b = line.strip('\x00').split()
            energy.append(float(a))
            freq.append(int(b))
    return np.array(energy), np.array(freq)


def plot_frequency_scatter(df):
    """Create frequency scatter plot with matplotlib"""
    fig, ax = plt.subplots(figsize=(15, 6))

    # Sort data by z value
    df_sorted = df.sort_values(by="z")

    # Plot main line
    ax.plot(df_sorted['z'], df_sorted['frequency_mean'],
            color='black', linewidth=2, zorder=1)

    # Plot all scatter points
    ax.scatter(df_sorted['z'], df_sorted['frequency_mean'],
              color='blue', s=200, alpha=0.7,
              edgecolors='DarkSlateGrey', linewidths=2, zorder=2,
              label=r'$(C_0,C_1,0,...)$')

    # Highlight special points (z=1760 and z=2923)
    special_1760 = df[df['z']==1760]
    special_2923 = df[df['z']==2923]

    if not special_1760.empty:
        ax.scatter(special_1760['z'], special_1760['frequency_mean'],
                  color='red', s=500,
                  edgecolors='DarkSlateGrey', linewidths=2, zorder=3,
                  label=r'$(C_2,0,0,...) /(0,C_3,0,...)$')

    if not special_2923.empty:
        ax.scatter(special_2923['z'], special_2923['frequency_mean'],
                  color='red', s=500,
                  edgecolors='DarkSlateGrey', linewidths=2, zorder=3)

    # Set labels and title
    ax.set_xlabel("value of z", fontsize=30)
    ax.set_ylabel('Frequency (GHz)', fontsize=30)

    # Set legend
    legend = ax.legend(loc='upper left', fontsize=25,
                      frameon=True, fancybox=False, shadow=False)
    legend.get_frame().set_facecolor('LightSteelBlue')
    legend.get_frame().set_edgecolor('Black')
    legend.get_frame().set_linewidth(1)

    # Set tick parameters
    ax.tick_params(axis='both', which='major', labelsize=20)

    # Adjust layout
    ax.set_xlim(0, max(df_sorted['z']) * 1.05)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    return fig, ax


def main():
    # Parse arguments
    parser = argparse.ArgumentParser(description='Plot frequency analysis from AVX data files')
    parser.add_argument('directory', help='Directory containing avx_* data files')
    args = parser.parse_args()

    in_dir = args.directory

    if not os.path.exists(in_dir):
        print(f"Error: Directory '{in_dir}' does not exist")
        return

    # Find all data files
    out_files = sorted(glob.glob(in_dir + "/avx_*"))
    if not out_files:
        print(f"Error: No avx_* files found in directory '{in_dir}'")
        return

    print(f"Processing {len(out_files)} files...")

    # Process frequency data
    freq_label_dict = {}

    # Process files with progress bar
    for f in tqdm(out_files, desc="Processing files"):
        label = "_".join(f.split("/")[-1].split(".")[0].split("_")[1:-1])
        energy_trace, freq_trace = parse_file(f)
        freq_label_dict.setdefault(label, []).extend(freq_trace)

    print("\nAnalyzing frequency data...")

    # Calculate frequency statistics
    freq_mean_data = []

    for label, trace in tqdm(freq_label_dict.items(), desc="Analyzing data"):
        samples_mean = np.mean(trace)
        samples_std = np.std(trace)

        # Filter outliers
        samples_filtered = []
        for sample in trace:
            if abs(sample - samples_mean) <= 6 * samples_std:
                samples_filtered.append(sample / 1000000)  # Convert to GHz

        # Extract z value from label
        label_list = label.split("_")
        if len(label_list) >= 2:
            z_val = int(label_list[1])
        else:
            z_val = int(label_list[0])

        if samples_filtered:
            freq_mean_data.append({
                'z': z_val,
                'frequency_mean': np.mean(samples_filtered),
                'std': np.std(samples_filtered),
                'num': len(samples_filtered)
            })

    if not freq_mean_data:
        print("Error: No valid frequency data found")
        return

    # Create DataFrame and plot
    df = pd.DataFrame(freq_mean_data)

    print(f"Creating plot with {len(df)} data points...")

    # Create output directory
    os.makedirs('plot', exist_ok=True)

    # Create plot
    fig, ax = plot_frequency_scatter(df)

    # Save as PNG
    output_file = "plot/frequency_plot.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    plt.close()

    print(f"Plot saved as: {output_file}")

    # Calculate and print statistics
    special_freqs = []
    normal_freqs = []

    for _, row in df.iterrows():
        if row['z'] in [1760, 2923]:
            special_freqs.append(row['frequency_mean'])
        else:
            normal_freqs.append(row['frequency_mean'])

    print("\n--- Results ---")
    if special_freqs:
        print(f"Low hamming weight cases average frequency: {np.mean(special_freqs):.3f} GHz")
    if normal_freqs:
        print(f"Normal hamming weight cases average frequency: {np.mean(normal_freqs):.3f} GHz")
    print(f"Total data points: {len(df)}")


if __name__ == "__main__":
    main()