#!/usr/bin/env python3

import argparse
import glob
import os

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from tqdm import tqdm


def parse_file(fn: str):
    energy = []
    freq = []
    with open(fn, encoding="latin-1") as f:
        for line in f:
            try:
                a, b = line.strip("\x00").split()
            except ValueError:
                continue
            energy.append(float(a))
            freq.append(int(b))
    return np.array(energy), np.array(freq)


def plot_frequency_scatter(df: pd.DataFrame, special_zs: list[int]):
    fig, ax = plt.subplots(figsize=(15, 6))

    df_sorted = df.sort_values(by="z")

    ax.plot(
        df_sorted["z"],
        df_sorted["frequency_mean"],
        color="black",
        linewidth=2,
        zorder=1,
    )

    ax.scatter(
        df_sorted["z"],
        df_sorted["frequency_mean"],
        color="blue",
        s=120,
        alpha=0.7,
        edgecolors="DarkSlateGrey",
        linewidths=1.5,
        zorder=2,
        label=r"$\hat{c} = (1, 1, z, 0, \ldots)$",
    )

    if special_zs:
        df_special = df[df["z"].isin(special_zs)]
        if not df_special.empty:
            ax.scatter(
                df_special["z"],
                df_special["frequency_mean"],
                color="red",
                s=280,
                edgecolors="DarkSlateGrey",
                linewidths=2,
                zorder=3,
                label=r"$z$ minimizes $\mathrm{HW}(\mathrm{invntt}(\hat{c} \cdot \hat{f}))$",
            )

    ax.set_xlabel("value of z", fontsize=22)
    ax.set_ylabel("Frequency (GHz)", fontsize=22)
    ax.tick_params(axis="both", which="major", labelsize=16)
    ax.grid(True, alpha=0.3)

    legend = ax.legend(loc="upper left", fontsize=16, frameon=True, fancybox=False)
    legend.get_frame().set_facecolor("LightSteelBlue")
    legend.get_frame().set_edgecolor("Black")
    legend.get_frame().set_linewidth(1)

    ax.set_xlim(0, max(df_sorted["z"]) * 1.05)
    plt.tight_layout()
    return fig, ax


def main():
    parser = argparse.ArgumentParser(description="Plot frequency means from avx_* files")
    parser.add_argument("directory", help="Directory containing avx_* data files")
    parser.add_argument(
        "--special",
        nargs="*",
        type=int,
        default=[7359, 942, 6684],
        help="z values to highlight (default: 7359 942 6684)",
    )
    args = parser.parse_args()

    in_dir = args.directory
    special_zs = args.special

    if not os.path.exists(in_dir):
        raise SystemExit(f"Error: Directory '{in_dir}' does not exist")

    out_files = sorted(glob.glob(os.path.join(in_dir, "avx_*")))
    if not out_files:
        raise SystemExit(f"Error: No avx_* files found in directory '{in_dir}'")

    print(f"Processing {len(out_files)} files...")

    freq_label_dict: dict[str, list[int]] = {}

    for f in tqdm(out_files, desc="Processing files"):
        label = "_".join(os.path.basename(f).split(".")[0].split("_")[1:-1])
        _, freq_trace = parse_file(f)
        freq_label_dict.setdefault(label, []).extend(freq_trace.tolist())

    freq_mean_data = []
    for label, trace in tqdm(freq_label_dict.items(), desc="Analyzing data"):
        trace_arr = np.array(trace, dtype=np.int64)
        samples_mean = np.mean(trace_arr)
        samples_std = np.std(trace_arr)

        samples_filtered = []
        for sample in trace_arr:
            if abs(sample - samples_mean) <= 6 * samples_std:
                samples_filtered.append(sample / 1_000_000)  # KHz -> GHz

        # label format: "<z>_<triple_index>_<threads>"
        z_val = int(label.split("_")[0])

        if samples_filtered:
            freq_mean_data.append(
                {
                    "z": z_val,
                    "frequency_mean": float(np.mean(samples_filtered)),
                    "std": float(np.std(samples_filtered)),
                    "num": int(len(samples_filtered)),
                }
            )

    if not freq_mean_data:
        raise SystemExit("Error: No valid frequency data found")

    df = pd.DataFrame(freq_mean_data)

    os.makedirs("plot", exist_ok=True)
    fig, _ = plot_frequency_scatter(df, special_zs)

    output_file = "plot/frequency_plot.png"
    plt.savefig(output_file, dpi=300, bbox_inches="tight")
    plt.close(fig)

    print(f"Plot saved as: {output_file}")

    if special_zs:
        special = df[df["z"].isin(special_zs)]["frequency_mean"].tolist()
        normal = df[~df["z"].isin(special_zs)]["frequency_mean"].tolist()
        if special:
            print(f"near-z mean frequency: {np.mean(special):.3f} GHz")
        if normal:
            print(f"random-z mean frequency: {np.mean(normal):.3f} GHz")


if __name__ == "__main__":
    main()

