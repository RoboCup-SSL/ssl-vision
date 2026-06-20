/*
     Copyright 2024 Felix Weinmann (original, Apache-2.0; part of TIGERs-Mannheim
     vision-processor: src/rtpstreamer.cpp)

     Adapted for ssl-vision (CPU RawImage source + libswscale RGB/YUV -> NV12)
     in 2026. Original licensed under the Apache License, Version 2.0:
       http://www.apache.org/licenses/LICENSE-2.0
*/
#ifndef RTPSTREAMER_H
#define RTPSTREAMER_H
#ifdef RTP_STREAM

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "rawimage.h"
#include "colors.h"

// Opaque libav forward declarations to keep the header light.
typedef struct AVCodecContext AVCodecContext;
typedef struct AVFormatContext AVFormatContext;
typedef struct AVStream AVStream;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;
struct SwsContext;

// Downscale filter for the monitor stream. Cost (5 MP -> 1080p): POINT is
// cheapest but aliases; FAST_BILINEAR is a good speed/quality balance.
enum RTPScaler { RTP_SCALE_FAST_BILINEAR, RTP_SCALE_BILINEAR, RTP_SCALE_POINT };

/*!
  \class  RTPStreamer
  \brief  Encodes captured frames to H.264 (hardware encoder if available) and
          sends them out as an MPEG-TS stream over UDP (multicast). MPEG-TS is
          self-describing, so viewers can open it directly (e.g.
          `ffplay udp://@<group>:<port>`) with no SDP file. A dedicated encoder
          thread is fed by a single-slot, drop-newest queue so the capture
          thread is never blocked by encoding or the network.
*/
class RTPStreamer {
public:
  // outWidth/outHeight <= 0 means "keep the source resolution" (no downscale).
  RTPStreamer(bool active, std::string uri, int framerate = 30,
              int outWidth = 0, int outHeight = 0,
              RTPScaler scaler = RTP_SCALE_FAST_BILINEAR);
  ~RTPStreamer();

  // Copies the frame (RGB8 / RGBA8 / YUV422 / MONO8) into the queue and returns
  // immediately. Older un-encoded frames are dropped. No-op if inactive or if
  // the source colour format is unsupported.
  void sendFrame(const RawImage &image);

private:
  void encoderRun();
  void allocResources();
  void freeResources();

  const bool active;
  const std::string uri;
  const int framerate;
  const int frametime_us;
  const int cfgOutWidth;   // configured output width  (<=0 => match source)
  const int cfgOutHeight;  // configured output height (<=0 => match source)
  const int swsFlags;      // libswscale algorithm flag

  // Source geometry/format the encoder is currently configured for.
  int width = 0;
  int height = 0;
  ColorFormat srcFormat = COLOR_UNDEFINED;
  // Output (encoded) geometry currently in use.
  int outWidth = 0;
  int outHeight = 0;

  bool stopEncoding = false;
  std::thread encoder;

  // Single-slot, drop-newest handoff (deep copy of the source frame bytes).
  std::vector<unsigned char> queueData;
  int qWidth = 0;
  int qHeight = 0;
  ColorFormat qFormat = COLOR_UNDEFINED;
  bool hasFrame = false;
  std::mutex queueMutex;
  std::condition_variable queueSignal;
  long currentFrameId = 0;
  // Input-side rate limit: only accept ~framerate frames/s so the capture
  // thread doesn't memcpy frames that would be dropped anyway. Touched only
  // by the capture thread (in sendFrame).
  std::chrono::steady_clock::time_point lastAccept{};

  AVCodecContext* codecCtx = nullptr;
  AVFormatContext* fmtCtx = nullptr;
  AVStream* stream = nullptr;
  AVFrame* frame = nullptr;
  AVPacket* pkt = nullptr;
  SwsContext* sws = nullptr;
};

#endif // RTP_STREAM
#endif // RTPSTREAMER_H
