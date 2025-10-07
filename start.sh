#!/bin/sh

./bin/vision -s -c 2 &
VISION_PID=$!

./video-stream-to-file.py 10 &
VID1_PID=$!
./video-stream-to-file.py 11 &
VID2_PID=$!

wait $VISION_PID

kill $VID1_PID $VID2_PID
