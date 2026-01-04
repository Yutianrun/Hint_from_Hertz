import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

import argparse
import seaborn as sns
import glob
import os
import matplotlib.ticker as ticker
import matplotlib.pyplot as plt
import numpy as np
import gc


def parse_file(fn):
    energy = []
    freq = []
    # print(fn)
    with open(fn) as f:
        for line in f:
            a, b = line.strip('\x00').split()
            energy.append(float(a))
            freq.append(int(b))

    return np.array(energy), np.array(freq)


# Density Plot and Histogram
# https://towardsdatascience.com/histograms-and-density-plots-in-python-f6bda88f5ac0
def plot_pdf(datas, labels):
    for data, label in zip(datas, labels):
        sns.distplot(data, label=label, hist=True, kde=True, bins=int(180/5))


def main():

    # Prepare output directory
    try:
        os.makedirs('plot')
    except:
        pass

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--raw', action='store_true')
    parser.add_argument('--freq', action='store_true')
    parser.add_argument('--energy', action='store_true')
    parser.add_argument('--hist', action='store_true')
    parser.add_argument('folder')
    args = parser.parse_args()
    in_dir = args.folder
    plot_raw_freq = args.raw
    plot_freq = args.freq
    plot_energy = args.energy
    plot_histograms = args.hist

    # For histograms
    energy_label_dict = {}
    freq_label_dict = {}

    # Read data
    #    ./out/all_%s_%04d.out
    out_files = sorted(glob.glob(in_dir + "/avx_*"))
    for f in out_files:
        label = "_".join(f.split("/")[-1].split(".")[0].split("_")[1:-1])
        rept_idx = f.split("/")[-1].split(".")[0].split("_")[-1]
        energy_trace, freq_trace = parse_file(f)

        if plot_energy:
            energy_label_dict.setdefault(label, []).extend(energy_trace)

        if plot_freq:
            freq_label_dict.setdefault(label, []).extend(freq_trace)

        # FIXME: Change to True to plot frequency
        if (plot_raw_freq):
            plt.figure(figsize=(20, 3.8))
            plt.plot(freq_trace)
            plt.savefig("./plot/freq_%s_%s.png" % (label, rept_idx))
            plt.clf()
            plt.close()

    # Parse energy data
    if plot_energy:
        print("\nEnergy Consumption Mean")

        # Prepare sike plot
        power = {}
        power[0] = []
        power[1] = []

        # Parse energy data
        for label, trace in energy_label_dict.items():
            trace = [i for i in trace if i>0 and i < 1]
            # Exclude negative samples (due to counter overflow)
            samples_positive = [x for x in trace if x > 0]

            # Get mean/std for each selector
            samples_mean = np.mean(samples_positive)
            samples_std = np.std(samples_positive)

            # Save mean/std
            print("%15s (%6d samples): %.15f +- %.15f J" % (label, len(trace), samples_mean, samples_std))

            # Filter outliers (for the plot only)
            samples_filtered = []  # 2 std
            for sample in samples_positive:
                if abs(sample - samples_mean) <= 3 * samples_std:
                    power_temp = sample / 0.001      # 0.001 is to convert to power since we sample energy every 1ms
                    samples_filtered.append(power_temp)

            # Process sike plot
            label_list = label.split("_")
            curr = int(label_list[0])
            power[curr].extend(samples_filtered)

        # Print mean
        print()

        print("miss trigger: %.15f +- %.15f W" % (np.mean(power[0]), np.std(power[0])))     
        print("hit trigger : %.15f +- %.15f W" % (np.mean(power[1]), np.std(power[1])))  

        if plot_histograms:
            key = [r"$miss trigger$", r"$hit trigger$"]
            value = [power[0], power[1]]
            plt.figure(figsize=(3, 2))  # 6.4, 4.8
            plot_pdf(value, key)
            plt.xlabel('Power consumption (W)')
            plt.ylabel('Probability density')
            plt.legend(fontsize=7)
            plt.tight_layout(pad=0.1)
            plt.savefig("./plot/ms-local-hist-energy.pdf", dpi=300)
            plt.clf()

    # Parse frequency data
    freq_mean_data=[]
    if plot_freq:
        print("\nFrequency Mean")

        # Prepare sike plot
        minimum = 100000
        maximum = 0
        freq = {}
        # freq[0] = []
        # freq[1] = []
        weight = {}
        weight[0] = []
        weight[1] = []

        # Parse data
        for label, trace in freq_label_dict.items():
            # trace = [i for i in trace if i>3850000 and i < 4100000]
            # Get mean/std for each selector
            samples_mean = np.mean(trace)
            samples_std = np.std(trace)

            # Save mean/std
            print("%15s (%6d samples): %d +- %d KHz (min %d max %d)" % (label, len(trace), samples_mean, samples_std, min(trace), max(trace)))

            # Filter outliers (for the plot only)

            samples_filtered = []
            for sample in trace:
                if abs(sample - samples_mean) <= 6* samples_std:
                    samples_filtered.append(sample / 1000000)

            # Store data for bins
            minimum = min(round(min(samples_filtered), 1), minimum)
            maximum = max(round(max(samples_filtered), 1), maximum)

            # Process sike plot
            label_list = label.split("_")
            curr = int(label_list[0])
            freq.setdefault(curr,[])
            freq[curr].extend(samples_filtered)
            print("%d trigger : %.15f +- %.15f KHz (*)" % (curr, np.mean(freq[curr]), np.std(freq[curr]))) 
            
            weight.setdefault(curr,[])
            weight[curr].extend(np.ones_like(samples_filtered)/float(len(samples_filtered)))
            # print("%d trigger : %.15f +- %.15f KHz (*)" % (curr, np.mean(freq[curr]), np.std(freq[curr])))
        # Print mean

        for curr in freq.keys():
            freq_mean_data.append({'z':curr,'frequency_mean':np.mean(freq[curr]), 'std':np.std(freq[curr]), 'num':len(freq[curr])})
        # print()
        # print("miss trigger: %.15f +- %.15f KHz (*)" % (np.mean(freq[0]), np.std(freq[0])))     # These means are shifted by 0.05
        # print("hit trigger : %.15f +- %.15f KHz (*)" % (np.mean(freq[1]), np.std(freq[1])))     # These means are shifted by 0.05

        import plotly.graph_objects as go
        import pandas as pd
        df = pd.DataFrame(freq_mean_data)

        print(df)
        df = df.sort_values(by="z")
        fig = go.Figure()

        
        fig.add_trace(go.Scatter(x=df['z'], y=df['frequency_mean'],
                        mode='lines',
                        line_color='black',
                        showlegend=False,
                        ))

        # fig.update_traces(textposition="bottom right",showlegend=False)
        fig.add_trace(go.Scatter(x=df['z'], y=df['frequency_mean'],
                        mode='markers', name=r'$\huge (C_0,0,...,C_1,C_2,...)$', marker=dict(color='blue', size=20, opacity=0.7, line=dict(width=2,
                                            color='DarkSlateGrey'))))

        # seed rand={9} for keypair , right z is  1760/2923
        # fig.add_trace(go.Scatter(x=[1760,2923],y=[float(df[df['z']==1760]['frequency_mean']),float(df[df['z']==2923]['frequency_mean'])],
        #                 mode='markers', name=r'$\huge (C_2,0,0,...) /(0,C_3,0,...)$',  
        #                 marker=dict(color='red', size=30, line=dict(width=2,color='DarkSlateGrey',))))

        # fig.add_trace(go.Scatter(x=[1490],y=[float(df[df['z']==1490]['frequency_mean']),float(df[df['z']==1490]['frequency_mean'])],
        #                 mode='markers', name=r'All Zero',  
        #                 marker=dict(color='green', size=30, line=dict(width=2,color='DarkSlateGrey',))))

        fig.add_trace(go.Scatter(x=[1387],y=[float(df[df['z']==1387]['frequency_mean'])],
                        mode='markers',  name=r'$\huge (C_0,0,...,C_3,0,...)$',
                        marker=dict(color='red', size=20, line=dict(width=2,color='DarkSlateGrey',))))          
        
        fig.update_traces(textposition="bottom right")
        fig.update_layout(
                    xaxis_title="value of"+ r"$\huge z'$",
                    # yaxis_title='Frequency Means (GHz)',
                    yaxis_title='Mean Execution Frequency (GHz)',
                    legend_font_size=25,
                    margin=dict(l=0,r=0,b=0,t=60)
                    )

        fig.update_layout(legend=dict(
                            x=0.01,
                            y=0.99,
                            # traceorder="reversed",
                            title_font_family="Times New Roman",
                            font=dict(
                                family="Courier",
                                size=50,
                                color="black"
                            ),
                            bgcolor="LightSteelBlue",
                            bordercolor="Black",
                            borderwidth=1,
                            entrywidth=200
            )
        )
        # title='Average Frequency when enumerating different z',
        fig.update_layout(width = 1500,height = 600, font_size = 30, legend_font_size=35,title_font_size= 20, font_color='black',font_family='Times New Roman')
        import plotly.io as pio
        # pio.kaleido.scope.mathjax = None
        # pio.full_figure_for_development(fig, warn=False)
        fig.write_image("plot/avx_mean_{}_2.pdf".format(len(df)),engine="kaleido")
        import time
        time.sleep(2)
        # pio.full_figure_for_development(fig, warn=False)
        fig.write_image("plot/avx_mean_{}_2.pdf".format(len(df)),engine="kaleido")
        print("low hamming weight case average frequency :",np.mean([df[df['z']==1387]['frequency_mean']]))
        print("nomral hamming weight case average frequency :",np.mean(df.loc[(df.z !=1387),['frequency_mean']]['frequency_mean']))
        # fig = go.Figure()
        # # Use x instead of y argument for horizontal plot
        # fig.add_trace(go.Box(x=))
        # fig.add_trace(go.Box(x=x1))

if __name__ == "__main__":
    

    main()
