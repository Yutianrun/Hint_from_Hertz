# Hints by Hertz

A Hertzbleed-style side-channel attack on CPA-Kyber (without Compress/Decompress) and CCA-NTTRU, accompanying the paper:

**Hints from Hertz: Dynamic Frequency Scaling Side-Channel Analysis of Number Theoretic Transform in Lattice-Based KEMs**


**Note:** This code is CPU-specific.

## Tested setup
We use the same setup as the Hertzbleed attack:

✅ Tested: Intel i7-9700, Ubuntu 20.04  
⚠️ Not guaranteed to work on VMs, other CPUs, custom hardware, or other OSes.

### Requirements

- Python 3
- Sage 10.2
- seaborn (Python)
- stress-ng (install via `sudo apt install stress-ng`)



## Materials

This repository contains:

`01-leakage-channel-data`: same as in [Hertzbleed](https://github.com/FPSG-UIUC/hertzbleed); code to test whether P‑state scaling leaks information on a given CPU.

`02-Kyber-pureNTT`: code used to measure frequency during the NTT.

`02b-Kyber-pureNTT-time`: code used to measure timing of the NTT.

`03-CPA-Kyber`: code used to attack Kyber (AVX2 implementation).

`03-Kyber-KnownSK_C0C0_128pair_verification`: code used to verify the attack across multiple pairs.

`04-NTTRU`: code used to attack NTTRU (AVX2 implementation).

`05-ToyExample`: toy example, Kyber-256, and scripts for frequency measurement and an end-to-end attack.

`06-Bit_Estimation`: integrates hints into DDGR and estimates security loss.

`util`: same as [Hertzbleed](https://github.com/FPSG-UIUC/hertzbleed); utilities to run leakage-model experiments and parse results.
