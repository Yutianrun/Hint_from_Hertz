import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

import argparse
import glob
import os
import matplotlib.ticker as ticker
import matplotlib.pyplot as plt
import numpy as np

def parse_file(fn):
    """Parse frequency data from output file"""
    freq = []
    with open(fn) as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) > 0:
                try:
                    # Get the last part as frequency
                    freq_value = parts[-1]
                    freq.append(int(freq_value))
                except ValueError:
                    # Skip lines that can't be converted to int
                    continue
    return np.array(freq)


def main():
    # Prepare output directory
    os.makedirs('plot', exist_ok=True)

    # Parse arguments
    parser = argparse.ArgumentParser(description='Plot frequency distribution from measurement data')
    parser.add_argument('folder', help='Input folder containing freq_*.out files')
    args = parser.parse_args()

    in_dir = args.folder

    # Extract type and date from folder name (e.g., "data/out-avx-1016-1234" -> "avx", "1016-1234")
    folder_name = os.path.basename(in_dir.rstrip('/'))
    # Try to extract type and date portion from folder name
    type_str = ""
    date_str = ""
    if '-' in folder_name:
        parts = folder_name.split('-')
        # Usually the format is out-{type}-{date}
        if len(parts) >= 2:
            type_str = parts[1]  # avx, ref, etc.
        if len(parts) >= 3:
            date_str = "-".join(parts[2:])

    # Store frequency data by label
    freq_label_dict = {}
    freq_mean = {}

    # Read data from all output files
    out_files = sorted(glob.glob(in_dir + "/freq_*"))
    for f in out_files:
        # Extract label from filename: freq_LABEL_INDEX.out
        label = "_".join(f.split("/")[-1].split(".")[0].split("_")[1:-1])

        # Parse frequency trace
        freq_trace = parse_file(f)

        # Accumulate frequency data for steady state analysis
        freq_label_dict.setdefault(label, []).extend(freq_trace)
        freq_mean.setdefault(label, []).append(np.mean(freq_trace))

    # Analyze and plot frequency distribution
    print("\nFrequency Mean")

    # Prepare plot data
    minimum = 100000
    maximum = 0
    datas = []
    labels = []
    weights = []
    label_category = [
        r'$\hat{r}_0:[C_0,C_1,\cdots]$',
        r"$\hat{r}_1:[C_2,0,\cdots]$",
        r"error",
        r"$\hat{r}_0\!:\![C_0,\cdots,0,C_1]\!$",
        r"$\hat{r}_0\!:\![C_0;0;,\cdots,C_2,0]\!$",
    ]

    # Process each label's frequency data
    for label, trace in freq_label_dict.items():
        # Compute statistics
        samples_mean = np.mean(trace)
        samples_std = np.std(trace)

        # Print statistics
        print("%15s (%d samples): %d +- %d KHz (min %d max %d)" %
              (label, len(trace), samples_mean, samples_std, min(trace), max(trace)))
        print("%d means:" % (len(freq_mean[label])), freq_mean[label],
              "\n The average mean is %d The std of %d mean %d Khz" %
              (np.mean(freq_mean[label]), len(freq_mean[label]), np.std(freq_mean[label])))

        # Filter outliers (within 6 standard deviations)
        samples_filtered = []
        for sample in trace:
            if abs(sample - samples_mean) <= 6 * samples_std:
                # Convert to GHz and add small offset for histogram binning
                samples_filtered.append(sample / 1000000 + 0.05)

        # Update min/max for plot range
        minimum = min(round(min(samples_filtered), 1), minimum)
        maximum = max(round(max(samples_filtered), 1), maximum)

        # Store data for plotting
        datas.append(samples_filtered)
        labels.append(label_category[int(label)])
        weights.append(np.ones_like(samples_filtered) / float(len(samples_filtered)))

    # Create histogram plot
    fig, ax = plt.subplots(1, 1, figsize=(3, 2))
    bins = np.arange(minimum, maximum, 0.1)
    num, bins, bars = ax.hist(datas, alpha=0.5, bins=bins, weights=weights,
                               label=labels, align="left", density=True)

    # Add percentage labels on bars
    for b in bars:
        ax.bar_label(b, labels=["{:.2f}%".format(i * 10) for i in b.datavalues],
                     label_type='edge', fontsize='xx-small')

    # Format plot
    plt.gca().xaxis.set_major_locator(ticker.MultipleLocator(0.1))
    plt.xlabel('Frequency (GHz)')
    plt.ylim(0, 11)
    plt.ylabel('Probability density')
    plt.legend(fontsize=6)
    plt.tight_layout(pad=0.1)

    # Save plot with type and date in filename
    if type_str and date_str:
        output_filename = f"./plot/hist-freq-{type_str}-{date_str}.png"
    elif type_str:
        output_filename = f"./plot/hist-freq-{type_str}.png"
    elif date_str:
        output_filename = f"./plot/hist-freq-{date_str}.png"
    else:
        output_filename = "./plot/hist-freq.png"

    plt.savefig(output_filename, dpi=300)
    plt.clf()
    print(f"\nPlot saved to {output_filename}")


if __name__ == "__main__":
    main()
