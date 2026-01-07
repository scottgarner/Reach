#!/bin/bash

PORTS="
  /dev/reach-port-1-1
  /dev/reach-port-1-2
  /dev/reach-port-3-1
  /dev/reach-port-3-2
"

for device in $PORTS; do
  if [ -e "$device" ]; then
    echo "Updating $device..."
    screen -S temp -dm $device 115200
    screen -S temp -p 0 -X stuff "B"
    sleep 1
    pio run -t upload --upload-port $device
    echo "Done."
  else
    echo "Skipping $device (not connected)"
  fi
done

echo "All boards updated!"
