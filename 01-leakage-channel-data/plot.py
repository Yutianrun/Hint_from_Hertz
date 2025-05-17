#!/usr/bin/env python3
# filepath: /home/yu/workspace/Hint_from_Hertz/01-leakage-channel-data/plot.py
import argparse
import glob
import os
import matplotlib.ticker as ticker
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from multiprocessing import Process, Manager

def parse_file(fn):
    with open(fn) as f:
        return np.array([int(line.strip()) for line in f])

def plot_pdf(datas, labels):
    for data, label in zip(datas, labels):
        sns.distplot(data, label=label, hist=True, kde=True, bins=25)

def detect_drop(trace):
    best_x, max_diff = 0, 0
    for x in range(int(len(trace) * 0.25), int(len(trace) * 0.85)):
        mean_before = np.mean(trace[:x])
        mean_after = np.mean(trace[x:])
        if mean_before - mean_after > max_diff:
            max_diff = mean_before - mean_after
            best_x = x
    return best_x

def f_pool(secret_bit, i, trace, drop_idxs, pre_drop_freq, post_drop_freq):
    trace = trace[500:]  # Exclude first few samples
    drop_idx = detect_drop(trace)
    
    if drop_idx == 0:
        return
        
    pre_drop_avg = np.mean(trace[:drop_idx])
    post_drop_avg = np.mean(trace[drop_idx:])
    
    # Save frequency drop data
    drop_idxs.append(drop_idx)
    pre_drop_freq.append(pre_drop_avg)
    post_drop_freq.append(post_drop_avg)

def main():
    # Create output directory
    os.makedirs('plot', exist_ok=True)

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--raw', action='store_true', help='Plot raw frequency traces')
    parser.add_argument('--steady', action='store_true', help='Plot steady state frequencies')
    parser.add_argument('--drops', action='store_true', help='Plot frequency drops')
    parser.add_argument('folder', help='Directory containing frequency data files')
    args = parser.parse_args()

    # Data structures
    freq_label_dict = {}    # For steady state
    freq_label_trace = {}   # For drop analysis

    # Read and process data files
    out_files = sorted(glob.glob(args.folder + "/freq_*"))
    for f in out_files:
        label = "_".join(f.split("/")[-1].split(".")[0].split("_")[1:-1])
        rept_idx = f.split("/")[-1].split(".")[0].split("_")[-1]
        freq_trace = parse_file(f)

        # Plot raw frequency trace if needed
        if args.raw:
            plt.figure(figsize=(10, 3))
            freq_trace_raw = [i/10**6 for i in freq_trace]
            plt.plot(freq_trace_raw)
            plt.xlabel('Time (ms)')
            plt.ylabel('Frequency (GHz)')
            plt.savefig(f"./plot/freq_{label}_{rept_idx}.pdf", bbox_inches='tight')
            plt.close()

        # Store data for steady state analysis
        if args.steady:
            freq_label_dict.setdefault(label, []).extend(freq_trace)

        # Store data for drop analysis
        if args.drops:
            freq_label_trace.setdefault(label, []).append(freq_trace)

    # Process steady state data
    if args.steady:
        print("\nFrequency Mean")
        
        datas = []
        labels = []
        weights = []
        
        for label, trace in freq_label_dict.items():
            # Calculate statistics
            samples_mean = np.mean(trace)
            samples_std = np.std(trace)
            
            print(f"{label:>15} ({len(trace)} samples): {samples_mean:.0f} +- {samples_std:.0f} KHz (min {min(trace)} max {max(trace)})")
            
            # Filter outliers and normalize to GHz
            samples_filtered = [s/1000000 for s in trace if abs(s - samples_mean) <= 6 * samples_std]
            
            datas.append(samples_filtered)
            labels.append(f"hw={label}")
            weights.append(np.ones_like(samples_filtered)/float(len(samples_filtered)))
        
        # Create histogram
        plt.figure(figsize=(6, 4))
        bins = np.linspace(min([min(d) for d in datas]), max([max(d) for d in datas]), 20)
        plt.hist(datas, alpha=0.5, bins=bins, weights=weights, label=labels, align="left", density=True)
        plt.xlabel('Frequency (GHz)')
        plt.ylabel('Probability density')
        plt.legend(fontsize=8)
        plt.tight_layout()
        plt.savefig("./plot/hist-freq.pdf", dpi=300)
        plt.close()

    # Process drop data
    if args.drops:
        results = []
        datas = []
        labels = []

        for secret_bit, all_traces_for_bit in freq_label_trace.items():
            # Process each trace in parallel
            manager = Manager()
            drop_idxs = manager.list()
            pre_drop_freq = manager.list()
            post_drop_freq = manager.list()
            
            processes = []
            for i, trace in enumerate(all_traces_for_bit[5:]):  # Skip first few for warmup
                p = Process(target=f_pool, args=(secret_bit, i, trace, drop_idxs, pre_drop_freq, post_drop_freq))
                processes.append(p)
                p.start()
            
            for p in processes:
                p.join()
            
            if len(drop_idxs) > 0:
                # Calculate statistics
                avg_drop_idx = np.mean(drop_idxs)
                avg_drop_std = np.std(drop_idxs)
                avg_pre = np.mean(pre_drop_freq)
                avg_pre_std = np.std(pre_drop_freq)
                avg_post = np.mean(post_drop_freq)
                avg_post_std = np.std(post_drop_freq)
                
                results.append(f"[+] Case {secret_bit} ({len(drop_idxs)} samples): drops at index {avg_drop_idx:.3f} +- {avg_drop_std:.3f}; " +
                              f"min at {min(drop_idxs)} max at {max(drop_idxs)}; " +
                              f"drops from ({avg_pre:.3f} +- {avg_pre_std:.3f} KHz) to ({avg_post:.3f} +- {avg_post_std:.3f} KHz)")
                
                # Filter outliers
                samples_mean = np.mean(drop_idxs)
                samples_std = np.std(drop_idxs)
                samples_filtered = [s for s in drop_idxs if abs(s - samples_mean) <= 1 * samples_std]
                
                datas.append(np.array(samples_filtered)/1000)
                labels.append(f"hw={secret_bit}")
        
        # Print results
        if results:
            print("\nFrequency Drops")
            for result in results:
                print(result)
            
            # Plot drop distribution
            plt.figure(figsize=(6, 4))
            plot_pdf(datas, labels)
            plt.xlabel('Seconds before steady state')
            plt.ylabel('Probability density')
            plt.legend(fontsize=8)
            plt.tight_layout()
            plt.savefig("./plot/hist-drop-idx.pdf", dpi=300)
            plt.close()

if __name__ == "__main__":
    main()