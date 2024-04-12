import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

import argparse
import seaborn as sns
import glob
import os
import matplotlib.ticker as ticker
import matplotlib.pyplot as plt
import numpy as np
from multiprocessing import Process
import multiprocessing
sns.set_theme()

def parse_file(fn):
    energy = []
    freq = []
    with open(fn) as f:
        for line in f:
            a, b = line.strip().split()
            energy.append(float(a))
            freq.append(int(b))

    return np.array(energy), np.array(freq)


# Density Plot and Histogram
# https://towardsdatascience.com/histograms-and-density-plots-in-python-f6bda88f5ac0
def plot_pdf(datas, labels):
    for data, label in zip(datas, labels):
        sns.histplot(data, label=label,  kde=True, bins=25, stat="density")


def detect_drop(trace):
    best_x = 0
    max_diff = 0
    for x in range(int(len(trace) * 0.25), int(len(trace) * 0.75)):  # FIXME: may need to restrict range
        mean_before = np.mean(trace[:x])
        mean_after = np.mean(trace[x:])
        if mean_before - mean_after > max_diff:
            max_diff = mean_before - mean_after
            best_x = x

    return best_x


def f_pool(secret_bit, i, trace, drop_idxs, pre_drop_freq, post_drop_freq):
    trace = trace[500:]     # Exclude first few samples

    # Save index of the drop
    drop_idx = detect_drop(trace)

    # Exclude weird runs (and plot for debug)
    excluded = 0

    # Compute mean before and after the drop
    pre_drop_avg = np.mean(trace[:drop_idx])
    post_drop_avg = np.mean(trace[drop_idx:])

    # Exclude runs that had a weird post-drop average
    # FIXME: Change this threshold for your CPU
    threshold = 4450000
    if post_drop_avg > threshold:
        print("Excluded", secret_bit, i, "because frequency was higher than", threshold, "after the drop. You may need to change the threshold variable")
        excluded = 1

    # FIXME: Change to True to plot drop in traces
    if (False):
        plt.figure(figsize=(20, 3.8))
        freq_x_axis = []
        for j in range(len(trace)):
            freq_x_axis.append(j / 1000)
        plt.plot(freq_x_axis, trace)
        plt.axvline(x=drop_idx / 1000, color='red')
        plt.savefig("./plot/freq_%s_%d.png" % (secret_bit, i))
        plt.clf()
        plt.close()

    # Plot traces that were excluded
    if excluded == 1:
        plt.figure(figsize=(20, 3.8))
        plt.plot(trace)
        plt.axvline(x=drop_idx, color='red')
        plt.savefig("./plot/freq_%s_%d_EXCLUDED.png" % (secret_bit, i))
        plt.clf()
        plt.close()
        return

    # Count all the other runs
    drop_idxs.append(drop_idx)
    pre_drop_freq.append(pre_drop_avg)
    post_drop_freq.append(post_drop_avg)


def main():

    # Prepare output directory
    try:
        os.makedirs('plot')
    except:
        pass

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--raw', action='store_true')
    parser.add_argument('--steady', action='store_true')
    parser.add_argument('--drops', action='store_true')
    parser.add_argument('--energy', action='store_true')
    parser.add_argument('folder')
    args = parser.parse_args()
    in_dir = args.folder
    plot_raw_freq = args.raw
    plot_steady = args.steady
    plot_drops = args.drops
    plot_energy = args.energy

    # For steady state experiment
    freq_label_dict = {}
    energy_label_dict = {}
    freq_mean={}
    # For drop idxs experiment
    freq_label_trace = {}
    energy_label_trace = {}
    # Read data
    #    ./out/freq_%s_%04d.out
    out_files = sorted(glob.glob(in_dir + "/freq_*"))
    for f in out_files:
        label = "_".join(f.split("/")[-1].split(".")[0].split("_")[1:-1])
        rept_idx = f.split("/")[-1].split(".")[0].split("_")[-1]
        energy_trace, freq_trace = parse_file(f)

        # Plot raw frequency trace if needed (useful for debug)
        if (plot_raw_freq):
            plt.figure(figsize=(20, 3.8))
            plt.plot(freq_trace)
            plt.savefig("./plot/freq_%s_%s.png" % (label, rept_idx))
            plt.clf()
            plt.close()
    
        # For steady state experiment
        if plot_steady:
            freq_label_dict.setdefault(label, []).extend(freq_trace)
            freq_mean.setdefault(label, []).append(np.mean(freq_trace))

        # For drop idxs experiment
        if plot_drops:
            freq_label_trace.setdefault(label, []).append(freq_trace)

        if plot_energy:
            energy_label_dict.setdefault(label, []).extend(energy_trace)

    ##############################################################
    # Parse energy data
    if plot_energy:
        print("\nEnergy Consumption Mean")

        # Prepare sike plot
        power = {}
        power[0] = []
        power[1] = []
        energy_data = []
        labels = []
        lable_category = [r'$\hat{r}_0:[C_0,C_1,\cdots]$',r"$\hat{r}_1:[C_2,0,\cdots]$",r"error",r"$\hat{r}_0\!:\![C_0,\cdots,0,C_1]\!$",r"$\hat{r}_0\!:\![C_0;0;,\cdots,C_2,0]\!$",]
        # Parse energy data
        for label, trace in energy_label_dict.items():
            trace = [i for i in trace if i>0 and i < 1]
            # Exclude negative samples (due to counter overflow)
            samples_positive = [x for x in trace if x > 0]

            # Get mean/std for each selector
            samples_mean = np.mean(samples_positive)
            samples_std = np.std(samples_positive)

            # Save mean/std
            print("%15s (%6d samples): %.15fW +- %.15f J" % (label, len(trace), samples_mean, samples_std))

            # Filter outliers (for the plot only)
            samples_filtered = []  # 2 std
            for sample in samples_positive:
                if abs(sample - samples_mean) <= 3 * samples_std:
                    power_temp = sample / 0.001      # 0.001 is to convert to power since we sample energy every 1ms
                    samples_filtered.append(power_temp)

            # Process sike plot
            label_list = label.split("_")
            curr = int(label_list[0])
            # datas.append(samples_filtered)
            labels.append(lable_category[int(label)])
            power[curr].extend(samples_filtered)

        # Print mean
        print()

        # print("miss trigger: %.15f +- %.15f W" % (np.mean(power[0]), np.std(power[0])))     
        # print("hit trigger : %.15f +- %.15f W" % (np.mean(power[1]), np.std(power[1])))  


        # key = [r"$miss trigger$", r"$hit trigger$"]
        key = labels
        # value = [power[0], power[1]]
        value = power.values()
        plt.figure(figsize=(3, 2))  # 6.4, 4.8
        plot_pdf(value, key)
        # plt.xlim(55, 75)
        plt.xlabel('Power consumption (W)')
        plt.ylabel('Probability density')
        plt.legend(fontsize=7)
        plt.tight_layout(pad=0.1)
        plt.savefig("./plot/ms-local-hist-energy.pdf", dpi=300)
        plt.clf()



    ###########################################################

    if plot_steady:
        print("\nFrequency Mean")

        # Prepare plot
        minimum = 100000
        maximum = 0
        datas = []
        labels = []
        weights = []
        lable_category = [r'$p_0:[C_1,C_2,\cdots]$',r"$p_1:[C_3,0,\cdots]$",r"$C_1;0;0;C_3$",r"$C_1;0;C_2;0$",]
        # lable_category = ["Normal Hamming Weight Case", "Low Hamming Weight Case",r"$C_1;0;0;C_3$",r"$C_1;0;C_2;0$",]
        # Parse data
        for label, trace in freq_label_dict.items():

            # Get mean/std for each selector
            samples_mean = np.mean(trace)
            samples_std = np.std(trace)

            # Print mean
            print("%15s (%d samples): %d +- %d KHz (min %d max %d)" % (label, len(trace), samples_mean, samples_std, min(trace), max(trace)))
            print("%d means:"%(len(freq_mean[label])), freq_mean[label], "\n The average mean is %d The std of %d mean %d Khz"%(np.mean(freq_mean[label]) ,len(freq_mean[label]),np.std(freq_mean[label])))

            # Filter outliers (for the plot only)
            samples_filtered = []
            for sample in trace:
                if abs(sample - samples_mean) <= 4 * samples_std:
                    samples_filtered.append(sample / 1000000 + 0.05)    # the + 0.05 is because the hist takes [4.2, 4.3) as range and we want 4.299999 in 4.3

            # Store data for bins
            minimum = min(round(min(samples_filtered), 1), minimum)
            maximum = max(round(max(samples_filtered), 1), maximum)

            # Store data for bars
            datas.append(samples_filtered)
 
            labels.append(lable_category[int(label)])
            # labels=[r"$C_1;C_2$(random)",r"$C_3;0~~$(random)"]
            weights.append(np.ones_like(samples_filtered)/float(len(samples_filtered)))

        # Plot all data
        plt.figure(figsize=(4, 3))
        bins = np.arange(minimum, maximum, 0.1)    # FIXME: adjust range
        _, bins, _ = plt.hist(datas, alpha=0.5, bins=bins, weights=weights, label=labels, align="left", density=True)
        plt.gca().xaxis.set_major_locator(ticker.MultipleLocator(0.1))
        plt.xlabel('Frequency (GHz)')
        plt.ylabel('Probability density')
        plt.legend(fontsize=7)
        plt.tight_layout(pad=0.1)
        plt.savefig("./plot/hist-freq.pdf", dpi=300)
        plt.clf()

        import plotly.graph_objects as go

        import plotly.io as pio


        fig = go.Figure()
        Case_category = ['Normal', 'Low']
        fig.update_layout(title='Measured Avrage Frequency Distribution',
                   yaxis_title='Hamming Weight Case',
                   xaxis_title='Average Frequency(Ghz))',
                   )

        pio.kaleido.scope.mathjax = None
        pio.full_figure_for_development(fig, warn=False)
        # Use x instead of y argument for horizontal plot

        for label, label_mean in freq_mean.items():
            fig.add_trace(go.Violin(x=np.array(label_mean, float)/1000000, box_visible=True,
                            meanline_visible=True,y0 = Case_category[int(label)]))


        fig.write_image('./plot/violn.pdf',engine="kaleido")

    ###########################################################

    if plot_drops:

        results = []
        datas = []
        labels = []

        for secret_bit, all_traces_for_bit in freq_label_trace.items():

            # Process each trace separately
            processes = []
            manager = multiprocessing.Manager()
            drop_idxs = manager.list()
            pre_drop_freq = manager.list()
            post_drop_freq = manager.list()
            for i, trace in enumerate(all_traces_for_bit[5:]):  # Exclude first 5 for warmup
                p = Process(target=f_pool, args=(secret_bit, i, trace, drop_idxs, pre_drop_freq, post_drop_freq))
                processes.append(p)
                p.start()

            # Wait for classifiers to end
            for p in processes:
                p.join()

            # Filter outliers (for the plot)
            samples_filtered = []
            samples_mean = np.mean(drop_idxs)
            samples_std = np.std(drop_idxs)
            for sample in drop_idxs:
                if abs(sample - samples_mean) <= 2 * samples_std:
                    samples_filtered.append(sample)

            # Compute mean of indices and means
            if len(drop_idxs) > 0:
                avg_drop_idx, avg_drop_std = np.mean(drop_idxs), np.std(drop_idxs)
                avg_pre_drop_freq, avg_pre_drop_std = np.mean(pre_drop_freq), np.std(pre_drop_freq)
                avg_post_drop_freq, avg_post_drop_std = np.mean(post_drop_freq), np.std(post_drop_freq)
                results.append("[+] Case %s (%03d samples): drops at index (%.03f +- %.03f); min at %d max at %d; drops from (%.03f +- %.03f KHz) to (%.03f +- %.03f KHz)"
                               % (secret_bit, len(drop_idxs), avg_drop_idx, avg_drop_std, min(drop_idxs), max(drop_idxs), avg_pre_drop_freq, avg_pre_drop_std, avg_post_drop_freq, avg_post_drop_std))

                datas.append(np.array(samples_filtered)/1000)
                labels.append("hw=" + str(secret_bit))

        # Print results
        if len(results) > 0:
            print("\nFrequency")
            for result in results:
                print(result)

        # Plot histogram
        plt.figure(figsize=(3, 2))
        plot_pdf(datas, labels)
        plt.xlabel('Seconds before steady state')
        plt.ylabel('Probability density')
        plt.legend(fontsize=7)
        plt.tight_layout(pad=0.1)
        plt.savefig("./plot/hist-drop-idx.pdf", dpi=300)
        plt.clf()


if __name__ == "__main__":
    main()
