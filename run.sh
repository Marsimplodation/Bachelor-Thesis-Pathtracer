#!/bin/bash

# Start Xephyr with the desired screen resolution on display :3
Xephyr -br -screen 1280x720 :3 &

# Store the Xephyr process ID to kill it later
XEPHYR_PID=$!

# Wait for Xephyr to initialize
sleep 0.2

# Set DISPLAY to :3 and start i3
DISPLAY=:3 i3 &

# Wait for i3 to start
sleep 2

# Launch the vkrenderer application
DISPLAY=:3 build/vkrenderer


# Kill i3 and Xephyr
killall i3
kill $XEPHYR_PID

echo "Xephyr and i3 have been killed."
