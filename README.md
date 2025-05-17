# Hints by Hertz

A Hertzbleed-like attack against CPA-Kyber without Compress/Decompress and CCA-NTTRU, accompanying the paper

**Hints from Hertz: Dynamic Frequency Scaling Side-Channel Analysis of Number Theoretic Transform in Lattice-Based KEMs**


**NB:** This source code is CPU-specific.

## Tested Setup
We follow the same setup as Hertzbleed attack：

✅ Tested: Intel i7-9700 CPU, Ubuntu 20.04  
⚠️ Not guaranteed to work on VMs, other CPUs, custom hardware, or different OS.

### Requirements

- Python 3
- Sage 10.2
- seaborn (Python package)
- stress-ng (install via `sudo apt install stress-ng`)



## Materials

This repository contains the following materials:

`01-leakage-channel-workloads` : the same as the one in [Hertbleed](https://github.com/FPSG-UIUC/hertzbleed), the code  to test if the scaling of P-states leaks information in certain CPU.

`02-Kyber-pureNTT` contains the code that we used to perform the frequency measuremnt over NTT.

`02b-Kyber-pureNTT-time` contains the code that we used to perform timing mesurement over NTT.

`03-CPA-Kyber` contains the code that we used to perform the Kyber attack over the AVX2-implementation.

`03-Kyber-KnownSK_C0C0_128pair_verification` contains the code that we used to verify our attack is valid over multiple pairs.

`04-CCA-NTTRU` contains the code that we used to perform the NTTRU attack over the AVX2-implementation.

`05-ToyExample` contains the code of our ToyExample , Kyber-256 and the code that we used to perform the frequency measurement and the script to finish an end-to-end attack.

`06-Bit_Estimation`  contains the code that we  integrate the hints to DDGR AND estimate the security loss.

`util`:  same as [Hertbleed](https://github.com/FPSG-UIUC/hertzbleed), contains scripts that can be used to run all the leakage model experiments and parse their results.
