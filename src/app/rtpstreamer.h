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

#include "rawimage.h"
#include "colors.h"

// Opaque libav forward declarations to keep the header light.
typedef struct AVCodecContext AVCodecContext;
typedef struct AVFormatContext AVFormatContext;
typedef struct AVStream AVStream;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;
struct SwsContext;

/*!
  \class  RTPStreamer
  \brief  Encodes captured frames to H.264 (hardware encoder if available) and
          sends them out as an RTP stream. A dedicated encoder thread is fed by
          a single-slot, drop-newest queue so the capture thread is never
          blocked by encoding or the network.
*/
class RTPStreamer {
public:
  RTPStreamer(bool active, std::string uri, int framerate = 30);
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

  // Geometry/format the encoder is currently configured for.
  int width = 0;
  int height = 0;
  ColorFormat srcFormat = COLOR_UNDEFINED;

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

  AVCodecContext* codecCtx = nullptr;
  AVFormatContext* fmtCtx = nullptr;
  AVStream* stream = nullptr;
  AVFrame* frame = nullptr;
  AVPacket* pkt = nullptr;
  SwsContext* sws = nullptr;
};

#endif // RTP_STREAM
#endif // RTPSTREAMER_H
