#!/bin/sh

./bin/vision -s -c 1 &
./video_stream_file_capture.py 10 &
