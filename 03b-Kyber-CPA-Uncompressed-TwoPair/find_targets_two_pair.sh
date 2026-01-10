#!/usr/bin/env bash
set -euo pipefail

if [[ ! -x ./bin/check_03_consistency ]]; then
  echo "Missing ./bin/check_03_consistency. Build first:"
  echo "  make -C 03-Kyber-CPA-Uncompressed-TwoPair"
  exit 1
fi

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <z_fixed> [pair_index ...]"
  echo
  echo "Examples:"
  echo "  $0 3165 5 44 53 56 60 66 69 74 97 106"
  echo "  $0 2246            # scans all pairs 1..127"
  exit 1
fi

z_fixed="$1"
shift

pairs=()
if [[ $# -gt 0 ]]; then
  pairs=("$@")
else
  for p in $(seq 1 127); do
    pairs+=("$p")
  done
fi

zero_pairs=()

for p in "${pairs[@]}"; do
  out="$(./bin/check_03_consistency --scan-two-pair "${z_fixed}" "${p}" 2>&1 || true)"
  hits="$(echo "${out}" | rg -o "hits=\\d+" | head -n1 | cut -d= -f2 || true)"
  hits="${hits:-0}"
  zs="$(echo "${out}" | rg -o "half_zero_z=\\d+" | cut -d= -f2 | paste -sd, - || true)"
  zs="${zs:-}"

  if [[ "${hits}" == "0" ]]; then
    zero_pairs+=("${p}")
  fi

  printf "pair=%-3s hits=%s" "${p}" "${hits}"
  if [[ -n "${zs}" ]]; then
    printf " z=%s" "${zs}"
  fi
  printf "\n"
done

if [[ "${#zero_pairs[@]}" -gt 0 ]]; then
  echo
  echo "zero-hit pairs (no half-zero z): ${zero_pairs[*]}"
fi

