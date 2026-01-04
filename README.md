# Hints by Hertz

Dynamic frequency scaling (DFS) leakage toolkit that reproduces the Hertzbleed-style attack path on Kyber (CPA) and NTTRU (CCA). The codebase extracts pure NTT kernels, measures turbo-state drift, and feeds that signal into multi-stage key recovery tooling.

## Supported Setup
- Intel Core i7-9700 on Ubuntu 20.04 (same platform as the original Hertzbleed experiment)
- Root access for MSR reads (`sudo` is mandatory for every frequency or leakage capture script)
- Stable cooling / power plan; virtual machines and exotic CPUs are **not** supported

## Software Requirements
- Python 3 with `matplotlib`, `seaborn`, `numpy`, `pandas`, `plotly`
- SageMath 10.2 for algebraic helpers in the Kyber attack
- `stress-ng` (install via `sudo apt install stress-ng`)
- `gcc`, `clang`, and AVX2-capable binutils for building the vectorized drivers

## Repository Layout
| Directory | Core Function |
|-----------|----------------|
| `01-leakage-channel-data` | Hertzbleed harness to confirm that DFS exposes a usable timing/frequency channel on your CPU. |
| `02-Kyber-pureNTT` | Extracted Kyber NTT drivers plus plotting scripts to record per-thread frequency traces; includes `run.sh --help` and `start_all.sh help` for quick automation. |
| `02b-Kyber-pureNTT-time` | Timing-only probe of the same kernels for comparison data. |
| `03-CPA-Kyber` | Full CPA attack on Kyber’s AVX2 IND-CPA implementation, including hierarchical grid search scripts (`grid_search_stage1_stability.sh --help`). |
| `03-Kyber-KnownSK_C0C0_128pair_verification` | Batch verifier for recovered key pairs and plotting utilities. |
| `04-NTTRU` | AVX2 implementation of the NTTRU CCA target plus local leakage drivers. |
| `05-ToyExample` | Minimal Kyber-256 toy setup showing the end-to-end workflow. |
| `06-Bit_Estimation` | Integrates frequency hints into DDGR for security-loss estimation. |
| `kyber`, `util`, `data`, `plot`, `scripts` | Vendor snapshots, helper utilities, shared datasets, plotting output, orchestration scripts, and regression fixtures. |

## Command Hints
- **Always prepend `sudo`** when running any script that touches MSRs (`run.sh`, `start_all.sh`, `run-local_cca_nttru.sh`, grid-search helpers, etc.). Without elevated access the measurements silently fail.
- All runtime logs are centralized under `log/`, mirroring the component path (for example `log/02-Kyber-pureNTT/run_all.log`).
- Every orchestration script exposes a help banner:
  - `02-Kyber-pureNTT/run.sh --help` lists implementation/mode flags.
  - `02-Kyber-pureNTT/start_all.sh help` documents the start/stop lifecycle.
  - `03-CPA-Kyber/kyber_indcpa/local/grid_search_hierarchical/grid_search_stage1_stability.sh --help` summarizes all job-control options.
  - `04-NTTRU/local_avx/run-local_cca_nttru.sh --help` explains the capture pipeline and reminds you to run it with `sudo`.

## Typical Workflow
1. **Calibrate leakage** – run `01-leakage-channel-data` to confirm DFS variation.
2. **Measure pure NTT** – build `02-Kyber-pureNTT` (`make`) then launch `sudo ./run.sh --avx --single` or `sudo ./start_all.sh` to collect traces; inspect `python plot.py <folder> --help` for plotting arguments.
3. **Execute attack** – switch to `03-CPA-Kyber` or `04-NTTRU`, run the grid-search script with the desired profile (`--mini`, `--fast`, `--full`), and watch progress via `--status`.
4. **Validate + visualize** – use the verification and toy example directories to sanity-check recovered key bits and regenerate figures.

Keep the directory tree tidy (`data/out-*`, `plot/*.pdf`) so that fresh runs are easy to compare. Every script prints a help banner on `--help`/`help`, so lean on those prompts whenever you need a refresher on arguments or required privileges.
