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
typedef struct AVBufferRef AVBufferRef;
typedef struct AVFilterGraph AVFilterGraph;
typedef struct AVFilterContext AVFilterContext;
struct SwsContext;

// Downscale filter for the monitor stream. Cost (5 MP -> 1080p): POINT is
// cheapest but aliases; FAST_BILINEAR is a good speed/quality balance.
enum RTPScaler { RTP_SCALE_FAST_BILINEAR, RTP_SCALE_BILINEAR, RTP_SCALE_POINT };

/*!
  \class  RTPStreamer
  \brief  Encodes captured frames to H.264 (hardware encoder if available) and
          sends them out as an RTP stream. A dedicated encoder thread is fed by
          a single-slot, drop-newest queue so the capture thread is never
          blocked by encoding or the network.
*/
class RTPStreamer {
public:
  // outWidth/outHeight <= 0 means "keep the source resolution" (no downscale).
  // hwaccel=true uses the VAAPI GPU path (RGB->NV12 + H.264 on the iGPU);
  // false uses the CPU path (libswscale + h264_qsv/nvenc/libx264).
  RTPStreamer(bool active, std::string uri, int framerate = 30,
              int outWidth = 0, int outHeight = 0,
              RTPScaler scaler = RTP_SCALE_FAST_BILINEAR, bool hwaccel = false);
  ~RTPStreamer();

  // Copies the frame (RGB8 / RGBA8 / YUV422 / MONO8) into the queue and returns
  // immediately. Older un-encoded frames are dropped. No-op if inactive or if
  // the source colour format is unsupported.
  void sendFrame(const RawImage &image);

private:
  void encoderRun();
  void allocResources();      // dispatches to CPU or VAAPI setup
  void allocResourcesSw();    // CPU path: libswscale -> sw encoder
  bool allocResourcesHw();    // VAAPI path: hwupload -> scale_vaapi -> h264_vaapi
  void freeResources();
  bool encodeAndSend();       // drain encoder -> RTP (shared by both paths)

  const bool active;
  const std::string uri;
  const int framerate;
  const int frametime_us;
  const int cfgOutWidth;   // configured output width  (<=0 => match source)
  const int cfgOutHeight;  // configured output height (<=0 => match source)
  const int swsFlags;      // libswscale algorithm flag
  const bool hwaccel;      // use the VAAPI GPU path

  // Source geometry/format the encoder is currently configured for.
  int width = 0;
  int height = 0;
  ColorFormat srcFormat = COLOR_UNDEFINED;
  // Output (encoded) geometry currently in use.
  int outWidth = 0;
  int outHeight = 0;
  bool useHw = false;  // hwaccel, but cleared at runtime if VAAPI setup fails

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
  AVFrame* frame = nullptr;       // CPU path: NV12 frame fed to the encoder
  AVPacket* pkt = nullptr;
  SwsContext* sws = nullptr;      // CPU path: src -> NV12 (or src -> BGR0 for HW)

  // VAAPI (route-b) path
  AVBufferRef* hwDeviceRef = nullptr;
  AVFilterGraph* fgraph = nullptr;
  AVFilterContext* fsrc = nullptr;   // buffersrc (BGR0 in)
  AVFilterContext* fsink = nullptr;  // buffersink (NV12 VAAPI surface out)
  AVFrame* swFrame = nullptr;        // BGR0 staging frame pushed into the graph
  AVFrame* hwFrame = nullptr;        // VAAPI NV12 surface pulled from the graph
};

#endif // RTP_STREAM
#endif // RTPSTREAMER_H
