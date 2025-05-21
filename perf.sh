#!/bin/bash

# Install 
# sudo apt install time
# pip install psutil
# sudo apt install nvidia-smi
#  Run 
# chmod +x monitor_exec.sh
# ./monitor_exec.sh


# also 
# sudo apt install gnome-system-monitor
# sudo apt install ksysguard
# hyperfine # run time statistics
# This script monitors the CPU and memory usage of a specified executable



# ========== CONFIG ==========
EXECUTABLE="./build/mytest"     # Set your executable path
LOG_PREFIX="report_$(date +%Y%m%d_%H%M%S)"
CPU_MEM_LOG="${LOG_PREFIX}_cpu_mem_usage.log"
GPU_LOG="${LOG_PREFIX}_gpu_usage.log"
RUNTIME_LOG="${LOG_PREFIX}_runtime_summary.log"
# ============================

echo "Running executable: $EXECUTABLE"
echo "Logging to $CPU_MEM_LOG, $GPU_LOG, $RUNTIME_LOG"

# Launch executable with timing
/usr/bin/time -v $EXECUTABLE > ${LOG_PREFIX}_stdout.log 2> $RUNTIME_LOG &
PID=$!

# Monitor CPU and memory usage
echo "timestamp,cpu_percent,mem_percent" > $CPU_MEM_LOG
echo "timestamp,gpu_util,memory_used" > $GPU_LOG

while ps -p $PID > /dev/null 2>&1; do
    timestamp=$(date +%Y-%m-%dT%H:%M:%S)
    cpu=$(ps -p $PID -o %cpu= | xargs)
    mem=$(ps -p $PID -o %mem= | xargs)
    echo "$timestamp,$cpu,$mem" >> $CPU_MEM_LOG

    # GPU (NVIDIA only)
    if command -v nvidia-smi &> /dev/null; then
        gpu_data=$(nvidia-smi --query-gpu=utilization.gpu,memory.used --format=csv,noheader,nounits | head -n 1)
        echo "$timestamp,$gpu_data" >> $GPU_LOG
    fi

    sleep 1
done

echo "Execution complete. Logs saved with prefix: $LOG_PREFIX"
