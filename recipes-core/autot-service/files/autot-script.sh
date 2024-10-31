#!/bin/sh

# Example script to monitor a directory for new files and log their names
MONITOR_DIR="/home/autot"
LOG_FILE="/var/log/autot.log"

echo "Monitoring directory: $MONITOR_DIR" >> $LOG_FILE

while true; do
    inotifywait -r -e create "$MONITOR_DIR" >> $LOG_FILE
    echo "New file created in $MONITOR_DIR" >> $LOG_FILE
done
