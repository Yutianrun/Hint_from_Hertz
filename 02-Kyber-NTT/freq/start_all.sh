#!/usr/bin/env bash

# Start or stop all NTT frequency measurements
# Usage: sudo ./start_all.sh [stop|help]

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/run_all.log"

if [ "$1" = "help" ] || [ "$1" = "-h" ]; then
    echo "Usage: sudo ./start_all.sh [stop]"
    echo "  (no arg)  Start all tests in background"
    echo "  stop      Stop all running tests"
    exit 0
fi

if [[ $EUID -ne 0 ]]; then
   echo "Error: run with sudo"
   exit 1
fi

if [ "$1" = "stop" ]; then
    echo "Stopping..."
    pkill -9 -f "run_all.sh" 2>/dev/null
    killall -9 stress-ng driver_ref driver_avx 2>/dev/null
    rm -rf data/tmp/* 2>/dev/null
    echo "Stopped"
    exit 0
fi

echo "Starting all tests in background..."
./run_all.sh > "$LOG_FILE" 2>&1 &
echo "PID: $!"
echo "Monitor: tail -f $LOG_FILE"
echo "Stop: sudo ./start_all.sh stop"
