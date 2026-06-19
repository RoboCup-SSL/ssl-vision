/*
     Copyright 2024 Felix Weinmann (original, Apache-2.0; part of TIGERs-Mannheim
     vision-processor: src/rtpstreamer.cpp)

     Adapted for ssl-vision in 2026: the original consumed an NV12 buffer that
     was produced on the GPU (OpenCL). ssl-vision has only CPU RawImages, so
     this version pulls frames from a CPU RawImage and converts them to NV12 on
     the encoder thread with libswscale before handing them to the H.264
     encoder. The encoder selection, RTP muxing and drop-newest handoff follow
     the original. Original licensed under the Apache License, Version 2.0:
       http://www.apache.org/licenses/LICENSE-2.0
*/
#ifdef RTP_STREAM

#include "rtpstreamer.h"

#include <cstring>
#include <chrono>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

// Map ssl-vision's internal pixel formats to libav input formats for swscale.
// Returns AV_PIX_FMT_NONE for formats we don't stream (e.g. raw Bayer).
static enum AVPixelFormat toAVPixFmt(ColorFormat f) {
  switch (f) {
    case COLOR_RGB8:        return AV_PIX_FMT_RGB24;
    case COLOR_RGBA8:       return AV_PIX_FMT_RGBA;
    case COLOR_YUV422_UYVY: return AV_PIX_FMT_UYVY422;
    case COLOR_YUV422_YUYV: return AV_PIX_FMT_YUYV422;
    case COLOR_MONO8:       return AV_PIX_FMT_GRAY8;
    default:                return AV_PIX_FMT_NONE;
  }
}

static int scalerToSwsFlag(RTPScaler s) {
  switch (s) {
    case RTP_SCALE_POINT:    return SWS_POINT;
    case RTP_SCALE_BILINEAR: return SWS_BILINEAR;
    case RTP_SCALE_FAST_BILINEAR:
    default:                 return SWS_FAST_BILINEAR;
  }
}

RTPStreamer::RTPStreamer(bool active, std::string uri, int framerate,
                         int outWidth, int outHeight, RTPScaler scaler)
    : active(active),
      uri(std::move(uri)),
      framerate(framerate > 0 ? framerate : 30),
      frametime_us(1000000 / (framerate > 0 ? framerate : 30)),
      cfgOutWidth(outWidth),
      cfgOutHeight(outHeight),
      swsFlags(scalerToSwsFlag(scaler)) {
  if (active)
    encoder = std::thread(&RTPStreamer::encoderRun, this);
}

RTPStreamer::~RTPStreamer() {
  stopEncoding = true;
  {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueSignal.notify_one();
  }
  if (encoder.joinable())
    encoder.join();
  freeResources();
}

void RTPStreamer::sendFrame(const RawImage &image) {
  if (!active)
    return;
  if (toAVPixFmt(image.getColorFormat()) == AV_PIX_FMT_NONE)
    return;  // unsupported source format, silently skip

  // Drop frames arriving faster than the target rate before paying for a copy.
  // (first call: now - epoch is huge, so it always passes)
  auto now = std::chrono::steady_clock::now();
  if (now - lastAccept < std::chrono::microseconds(frametime_us))
    return;
  lastAccept = now;

  std::unique_lock<std::mutex> lock(queueMutex);
  const int n = image.getNumBytes();
  queueData.resize(n);
  std::memcpy(queueData.data(), image.getData(), n);
  qWidth = image.getWidth();
  qHeight = image.getHeight();
  qFormat = image.getColorFormat();
  hasFrame = true;
  queueSignal.notify_one();
}

void RTPStreamer::allocResources() {
  if (codecCtx != nullptr)
    return;

  // Output resolution: configured value, or the source resolution if unset.
  outWidth = (cfgOutWidth > 0) ? cfgOutWidth : width;
  outHeight = (cfgOutHeight > 0) ? cfgOutHeight : height;

  const AVCodec* codec = nullptr;
  // h264_vaapi is intentionally omitted: it requires a hardware frames context
  // (AV_PIX_FMT_VAAPI surfaces), which this software-NV12 path does not set up.
  // h264_nvenc / h264_qsv both accept system-memory NV12 frames directly.
  std::vector<const char*> codecNames{"h264_nvenc", "h264_qsv", "libx264"};
  for (const auto &codecName : codecNames) {
    codec = avcodec_find_encoder_by_name(codecName);
    if (codec == nullptr)
      continue;

    codecCtx = avcodec_alloc_context3(codec);
    codecCtx->bit_rate = 3500000;
    codecCtx->width = outWidth;
    codecCtx->height = outHeight;
    codecCtx->time_base.num = 1;
    codecCtx->time_base.den = framerate;
    codecCtx->gop_size = framerate;
    codecCtx->max_b_frames = 0;
    codecCtx->pix_fmt = AV_PIX_FMT_NV12;
    codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

    if (strcmp(codecName, "h264_qsv") == 0) {
      av_opt_set(codecCtx->priv_data, "preset", "veryfast", 0);
    }
    if (strcmp(codecName, "libx264") == 0) {
      av_opt_set(codecCtx->priv_data, "preset", "ultrafast", 0);
      av_opt_set(codecCtx->priv_data, "tune", "zerolatency", 0);
    }

    if (avcodec_open2(codecCtx, codec, nullptr) == 0)
      break;

    avcodec_free_context(&codecCtx);
    codecCtx = nullptr;
  }

  if (codecCtx == nullptr) {
    std::cerr << "[RTPStreamer] Failed to find a usable H.264 encoder." << std::endl;
    return;
  }
  std::cout << "[RTPStreamer] Using codec: " << codec->long_name
            << " (" << width << "x" << height << " -> " << outWidth << "x" << outHeight
            << " @ " << framerate << "fps -> " << uri << ")" << std::endl;

  const AVOutputFormat* ofmt = av_guess_format("rtp", nullptr, nullptr);
  avformat_alloc_output_context2(&fmtCtx, ofmt, ofmt->name, uri.c_str());
  if (avio_open(&fmtCtx->pb, uri.c_str(), AVIO_FLAG_WRITE) < 0) {
    std::cerr << "[RTPStreamer] Failed to open output: " << uri << std::endl;
  }

  stream = avformat_new_stream(fmtCtx, codec);
  avcodec_parameters_from_context(stream->codecpar, codecCtx);
  stream->time_base = codecCtx->time_base;

  if (avformat_write_header(fmtCtx, nullptr) < 0) {
    std::cerr << "[RTPStreamer] Failed to write RTP header." << std::endl;
  }

  frame = av_frame_alloc();
  frame->format = AV_PIX_FMT_NV12;
  frame->width = outWidth;
  frame->height = outHeight;
  av_frame_get_buffer(frame, 32);

  pkt = av_packet_alloc();

  sws = sws_getContext(width, height, toAVPixFmt(srcFormat),
                       outWidth, outHeight, AV_PIX_FMT_NV12,
                       swsFlags, nullptr, nullptr, nullptr);
}

void RTPStreamer::freeResources() {
  if (codecCtx != nullptr) {
    avcodec_send_frame(codecCtx, nullptr);  // flush
    avcodec_free_context(&codecCtx);
  }
  if (fmtCtx != nullptr) {
    if (fmtCtx->pb != nullptr)
      avio_close(fmtCtx->pb);
    avformat_free_context(fmtCtx);
    fmtCtx = nullptr;
  }
  if (frame != nullptr)
    av_frame_free(&frame);
  if (pkt != nullptr)
    av_packet_free(&pkt);
  if (sws != nullptr) {
    sws_freeContext(sws);
    sws = nullptr;
  }
  stream = nullptr;
}

void RTPStreamer::encoderRun() {
  while (!stopEncoding) {
    std::vector<unsigned char> local;
    int lw = 0, lh = 0;
    ColorFormat lf = COLOR_UNDEFINED;
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      queueSignal.wait(lock, [&]() { return hasFrame || stopEncoding; });
      if (stopEncoding) {
        freeResources();
        return;
      }
      local.swap(queueData);
      lw = qWidth;
      lh = qHeight;
      lf = qFormat;
      hasFrame = false;
    }

    if (lw != width || lh != height || lf != srcFormat) {
      freeResources();
      width = lw;
      height = lh;
      srcFormat = lf;
    }

    allocResources();
    if (codecCtx == nullptr || sws == nullptr || lh <= 0)
      continue;

    auto startTime = std::chrono::high_resolution_clock::now();

    av_frame_make_writable(frame);
    const uint8_t* srcSlice[4] = {local.data(), nullptr, nullptr, nullptr};
    int srcStride[4] = {static_cast<int>(local.size() / lh), 0, 0, 0};
    sws_scale(sws, srcSlice, srcStride, 0, lh, frame->data, frame->linesize);

    frame->pts = currentFrameId++;
    avcodec_send_frame(codecCtx, frame);

    int status = avcodec_receive_packet(codecCtx, pkt);
    if (status == 0) {
      av_packet_rescale_ts(pkt, codecCtx->time_base, stream->time_base);
      av_interleaved_write_frame(fmtCtx, pkt);
      av_packet_unref(pkt);
    } else if (status != AVERROR(EAGAIN)) {
      std::cerr << "[RTPStreamer] Encoder error: " << status << std::endl;
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
    auto target = std::chrono::microseconds(frametime_us);
    if (elapsed < target)
      std::this_thread::sleep_for(target - elapsed);
  }
}

#endif  // RTP_STREAM
