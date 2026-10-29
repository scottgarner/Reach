#!/bin/bash

PORTS="
  /dev/reach-board-1-2
  /dev/reach-board-3-1
  /dev/reach-board-3-2
"

for device in $PORTS; do
  if [ -e "$device" ]; then
    echo "Updating $device..."
    screen -S temp -dm $device 115200
    screen -S temp -p 0 -X stuff "B"
    sleep 1
    screen -S temp -X quit > /dev/null 2>&1
    if pio run -t upload --upload-port $device; then
      echo "Done."
    else
      echo "FAILED: $device"
      failed=1
    fi
  else
    echo "Skipping $device (not connected)"
  fi
done

if [ -n "$failed" ]; then
  echo "Some boards failed to update."
  exit 1
fi

echo "All boards updated!"
