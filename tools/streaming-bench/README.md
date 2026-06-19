# streaming-bench

Microbenchmarks for the per-frame CPU cost of the optional RTP H.264 livestream
(`USE_RTP_STREAM`, see `src/app/rtpstreamer.cpp`). These isolate the hot
operations so you can measure them **on the target machine** without building
the full ssl-vision app.

It measures:

1. **memcpy** of a 5 MP frame — the cost the streamer adds to the *capture*
   thread (the deep copy in `RTPStreamer::sendFrame`).
2. **swscale RGB→NV12** — the cost on the *encoder* thread, across output
   resolutions (5 MP / 1080p / 720p) and scaler flags (point / fast_bilinear /
   bilinear).
3. **H.264 encode** — tries `h264_qsv`, `h264_nvenc`, and the `libx264`
   software fallback, at each resolution.

## Build & run

```sh
make
./streaming_bench
```

Requires libav dev headers (Debian/Ubuntu:
`apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev`).

## Interpreting the results

- The **capture/detection** thread only pays the memcpy (1) — everything else
  runs on a separate encoder thread.
- On a **2-core** host (e.g. NUC7i7BNH) the encoder thread competes for cores
  with detection. If `h264_qsv` shows a real number, hardware encode works and
  the encode is nearly free on the CPU; if it's `-1`, the stream falls back to
  `libx264` (software) — watch that swscale + libx264 don't saturate a core.
- swscale still reads the full 5 MP input regardless of output size, so there
  is a floor (~the 720p/point number) that only the GPU path (route-b) removes.

These were the basis for the latency/CPU figures discussed during development;
re-run here to get figures for your actual hardware rather than a dev box.
