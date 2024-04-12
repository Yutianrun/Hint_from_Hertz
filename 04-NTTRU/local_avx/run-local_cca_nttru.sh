#!/usr/bin/env bash

TOTAL_PHYSICAL_CORES=`grep '^core id' /proc/cpuinfo | sort -u | wc -l`
TOTAL_LOGICAL_CORES=`grep '^core id' /proc/cpuinfo | wc -l`

# Load MSR module
sudo modprobe msr


# for i in {1,101,601,1001,1201,1501,1751,2001,2051,3201,4501,5051,5501,6001,6751,7001,7251}  
# do

# Setup
samples=10000	# 20 seconds
outer=20
# date=`date +"%m%d-%H%M"`
thread=300

echo "This script will take about $(($(($samples/1000+10))*$outer*100/60+10)) minitues. Reduce 'outer' if you want a shorter run."
# Prepare
# sudo rm -rf out
# mkdir out
# sudo rm -rf input.txt

#right z 942 6684 7359
# for guess_z in {45,110,123,213,557,579,711,791,867,942,1091,1515,1797,1863,1893,2863,2967,3055,3106,3249,3445,4093,4189,4403,4597,4655,5086,5116,5329,5406,5534,5544,5598,5833,5854,5881,5919,6100,6239,6289,6327,6549,6684,6882,6985,7053,7186,7359,7547,7614}  
# for guess_z in {45,123,213,557,579,711,791,942,1515,1863,1893,2967,3055,3106,3249,4189,4403,4655,5086,5116,5598,5854,5881,5919,6239,6684,6882,6985,7186,7359}
# do
# echo $guess_z 0  $thread >> input.txt
# done




# 生成指定数量的随机数字并排序
generate_and_sort_numbers() {
  local numbers=()
  # 生成17个不等于942、6684、7359的随机数字
  for i in {1..97}; do
    local rand_num
    while true; do
      rand_num=$((RANDOM % 7680 + 1))
      # 检查数字是否已经存在于数组中，以及是否等于指定的三个数字
      if [[ ! " ${numbers[@]} " =~ " $rand_num " ]] && [ $rand_num -ne 942 ] && [ $rand_num -ne 6684 ] && [ $rand_num -ne 7359 ]; then
        break
      fi
    done
    numbers+=( $rand_num )
  done

  # 确保包含指定的三个数字  
  numbers+=(942 6684 7359)

  # 对数组进行排序
  sorted_numbers=($(echo "${numbers[@]}" | tr ' ' '\n' | sort -n))

  for j in {0..99}; do
    echo ${sorted_numbers[j]} 0  $thread >> input.txt
  done
  
}

# 主循环，执行十次
for ((k=1; k<=2; k++)); do
  date=`date +"%m%d-%H%M"`
  sudo rm -rf out
  mkdir out
  sudo rm -rf input.txt
  generate_and_sort_numbers


  #   Warm Up 

  stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m

  #   Run

  sudo ./bin/driver_kem ${samples} ${outer}
  cp -r out/ data/out-${date}

  sleep 60
done



# Warm Up
# stress-ng -q --cpu $TOTAL_LOGICAL_CORES -t 10m
# Run
# sudo ./bin/driver_kem ${samples} ${outer}
# cp -r out/ data/out-${date}

# sleep 1800


# done








# Unload MSR module
sudo modprobe -r msr