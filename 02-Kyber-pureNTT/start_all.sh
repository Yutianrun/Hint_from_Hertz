#!/usr/bin/env bash

# Start or stop all NTT frequency measurements
# Usage: sudo ./start_all.sh [stop|help]

# Show help
if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "Usage: sudo ./start_all.sh [COMMAND]"
    echo ""
    echo "Start or stop all NTT frequency measurements"
    echo ""
    echo "Commands:"
    echo "  (no argument)    Start all 4 tests in background"
    echo "  stop             Stop all running measurements and cleanup"
    echo "  help             Show this help message"
    echo ""
    echo "Examples:"
    echo "  sudo ./start_all.sh         # Start measurements"
    echo "  sudo ./start_all.sh stop    # Stop and cleanup"
    exit 0
fi

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   echo "Error: This script must be run with sudo"
   echo "Usage: sudo ./start_all.sh [stop|help]"
   exit 1
fi

# Stop mode
if [ "$1" = "stop" ]; then
    echo "Stopping all measurements..."

    # Kill all child processes first
    for pid in $(pgrep -f "run_all.sh"); do
        pkill -9 -P "$pid" 2>/dev/null
        kill -9 "$pid" 2>/dev/null
    done

    # Kill measurement processes
    killall -9 stress-ng 2>/dev/null
    sleep 1
    killall -9 driver_ref driver_avx 2>/dev/null
    pkill -9 -f "run.sh" 2>/dev/null

    # Cleanup temporary data
    rm -rf data/tmp/* 2>/dev/null

    echo "All processes stopped"
    exit 0
fi

# Start mode
echo "Starting all NTT frequency measurements in background..."
echo ""

# Run in background
./run_all.sh > run_all.log 2>&1 &

PID=$!
echo "Process started with PID: $PID"
echo ""
echo "To monitor progress:"
echo "  tail -f run_all.log"
echo ""
echo "To stop measurements:"
echo "  sudo ./start_all.sh stop"
echo ""
