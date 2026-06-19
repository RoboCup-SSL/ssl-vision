/*
     Copyright 2024 Felix Weinmann (original, Apache-2.0; part of TIGERs-Mannheim
     vision-processor: src/rtpstreamer.cpp)

     Adapted for ssl-vision in 2026:
       - CPU path: pulls a CPU RawImage and converts RGB/YUV -> NV12 with
         libswscale, then encodes with h264_qsv/nvenc/libx264.
       - VAAPI path (route-b): converts RGB -> BGR0 on the CPU, then uploads to
         the iGPU and does the colour-convert/downscale (scale_vaapi) and H.264
         encode (h264_vaapi) entirely on the GPU, so the heavy swscale RGB->NV12
         and the encode leave the CPU.
     Original licensed under the Apache License, Version 2.0:
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
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

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
                         int outWidth, int outHeight, RTPScaler scaler, bool hwaccel)
    : active(active),
      uri(std::move(uri)),
      framerate(framerate > 0 ? framerate : 30),
      frametime_us(1000000 / (framerate > 0 ? framerate : 30)),
      cfgOutWidth(outWidth),
      cfgOutHeight(outHeight),
      swsFlags(scalerToSwsFlag(scaler)),
      hwaccel(hwaccel),
      useHw(hwaccel) {
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

// Shared RTP output setup once codecCtx is open.
static bool setupOutputImpl(const std::string &uri, AVCodecContext *codecCtx,
                            AVFormatContext **fmtCtxOut, AVStream **streamOut,
                            const AVCodec *codec) {
  const AVOutputFormat* ofmt = av_guess_format("rtp", nullptr, nullptr);
  avformat_alloc_output_context2(fmtCtxOut, ofmt, ofmt->name, uri.c_str());
  if (avio_open(&(*fmtCtxOut)->pb, uri.c_str(), AVIO_FLAG_WRITE) < 0) {
    std::cerr << "[RTPStreamer] Failed to open output: " << uri << std::endl;
    return false;
  }
  *streamOut = avformat_new_stream(*fmtCtxOut, codec);
  avcodec_parameters_from_context((*streamOut)->codecpar, codecCtx);
  (*streamOut)->time_base = codecCtx->time_base;
  if (avformat_write_header(*fmtCtxOut, nullptr) < 0) {
    std::cerr << "[RTPStreamer] Failed to write RTP header." << std::endl;
    return false;
  }
  return true;
}

void RTPStreamer::allocResourcesSw() {
  const AVCodec* codec = nullptr;
  // h264_vaapi is omitted here (needs a hw frames context); that is the VAAPI
  // path. h264_nvenc / h264_qsv accept system-memory NV12 frames directly.
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
    if (strcmp(codecName, "h264_qsv") == 0)
      av_opt_set(codecCtx->priv_data, "preset", "veryfast", 0);
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

  frame = av_frame_alloc();
  frame->format = AV_PIX_FMT_NV12;
  frame->width = outWidth;
  frame->height = outHeight;
  av_frame_get_buffer(frame, 32);
  pkt = av_packet_alloc();
  sws = sws_getContext(width, height, toAVPixFmt(srcFormat),
                       outWidth, outHeight, AV_PIX_FMT_NV12,
                       swsFlags, nullptr, nullptr, nullptr);

  if (!setupOutputImpl(uri, codecCtx, &fmtCtx, &stream, codec))
    return;
  std::cout << "[RTPStreamer] Using codec: " << codec->long_name
            << " [CPU swscale] (" << width << "x" << height << " -> "
            << outWidth << "x" << outHeight << " @ " << framerate << "fps -> "
            << uri << ")" << std::endl;
}

bool RTPStreamer::allocResourcesHw() {
  if (av_hwdevice_ctx_create(&hwDeviceRef, AV_HWDEVICE_TYPE_VAAPI,
                             "/dev/dri/renderD128", nullptr, 0) < 0) {
    std::cerr << "[RTPStreamer] Failed to create VAAPI device." << std::endl;
    return false;
  }
  const AVCodec* codec = avcodec_find_encoder_by_name("h264_vaapi");
  if (!codec) {
    std::cerr << "[RTPStreamer] h264_vaapi encoder not available." << std::endl;
    return false;
  }

  // CPU only repacks RGB -> BGR0 (4-byte, VAAPI-uploadable). No YUV maths, no
  // scaling here: the GPU does CSC + downscale in scale_vaapi.
  sws = sws_getContext(width, height, toAVPixFmt(srcFormat),
                       width, height, AV_PIX_FMT_BGR0,
                       SWS_POINT, nullptr, nullptr, nullptr);
  swFrame = av_frame_alloc();
  swFrame->format = AV_PIX_FMT_BGR0;
  swFrame->width = width;
  swFrame->height = height;
  av_frame_get_buffer(swFrame, 32);
  hwFrame = av_frame_alloc();

  // Filter graph: buffer(BGR0) -> hwupload -> scale_vaapi(format=nv12) -> sink
  fgraph = avfilter_graph_alloc();
  char args[256];
  snprintf(args, sizeof(args),
           "video_size=%dx%d:pix_fmt=%d:time_base=1/%d:pixel_aspect=1/1",
           width, height, (int)AV_PIX_FMT_BGR0, framerate);
  if (avfilter_graph_create_filter(&fsrc, avfilter_get_by_name("buffer"), "in",
                                   args, nullptr, fgraph) < 0 ||
      avfilter_graph_create_filter(&fsink, avfilter_get_by_name("buffersink"), "out",
                                   nullptr, nullptr, fgraph) < 0) {
    std::cerr << "[RTPStreamer] Failed to create buffer/buffersink." << std::endl;
    return false;
  }

  char desc[256];
  snprintf(desc, sizeof(desc), "hwupload,scale_vaapi=w=%d:h=%d:format=nv12",
           outWidth, outHeight);
  AVFilterInOut* outputs = avfilter_inout_alloc();
  AVFilterInOut* inputs = avfilter_inout_alloc();
  outputs->name = av_strdup("in");  outputs->filter_ctx = fsrc;  outputs->pad_idx = 0; outputs->next = nullptr;
  inputs->name  = av_strdup("out"); inputs->filter_ctx = fsink; inputs->pad_idx = 0; inputs->next = nullptr;
  int rc = avfilter_graph_parse_ptr(fgraph, desc, &inputs, &outputs, nullptr);
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);
  if (rc < 0) {
    std::cerr << "[RTPStreamer] Failed to parse VAAPI filter graph." << std::endl;
    return false;
  }
  // hwupload / scale_vaapi need the VAAPI device.
  for (unsigned i = 0; i < fgraph->nb_filters; i++)
    fgraph->filters[i]->hw_device_ctx = av_buffer_ref(hwDeviceRef);
  if (avfilter_graph_config(fgraph, nullptr) < 0) {
    std::cerr << "[RTPStreamer] Failed to configure VAAPI filter graph." << std::endl;
    return false;
  }

  codecCtx = avcodec_alloc_context3(codec);
  codecCtx->bit_rate = 3500000;
  codecCtx->width = outWidth;
  codecCtx->height = outHeight;
  codecCtx->time_base.num = 1;
  codecCtx->time_base.den = framerate;
  codecCtx->gop_size = framerate;
  codecCtx->max_b_frames = 0;
  codecCtx->pix_fmt = AV_PIX_FMT_VAAPI;
  codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  // The encoder's hw frames come from the filter graph's output (buffersink).
  AVBufferRef* framesCtx = av_buffersink_get_hw_frames_ctx(fsink);
  if (!framesCtx) {
    std::cerr << "[RTPStreamer] No hw_frames_ctx on filter output." << std::endl;
    return false;
  }
  codecCtx->hw_frames_ctx = av_buffer_ref(framesCtx);
  if (avcodec_open2(codecCtx, codec, nullptr) != 0) {
    std::cerr << "[RTPStreamer] Failed to open h264_vaapi." << std::endl;
    avcodec_free_context(&codecCtx);
    codecCtx = nullptr;
    return false;
  }
  pkt = av_packet_alloc();

  if (!setupOutputImpl(uri, codecCtx, &fmtCtx, &stream, codec))
    return false;
  std::cout << "[RTPStreamer] Using codec: h264_vaapi [GPU VAAPI] ("
            << width << "x" << height << " -> " << outWidth << "x" << outHeight
            << " @ " << framerate << "fps -> " << uri << ")" << std::endl;
  return true;
}

void RTPStreamer::allocResources() {
  if (codecCtx != nullptr)
    return;
  outWidth = (cfgOutWidth > 0) ? cfgOutWidth : width;
  outHeight = (cfgOutHeight > 0) ? cfgOutHeight : height;

  if (useHw) {
    if (!allocResourcesHw()) {
      std::cerr << "[RTPStreamer] VAAPI setup failed; falling back to CPU encode."
                << std::endl;
      freeResources();
      useHw = false;
      allocResourcesSw();
    }
  } else {
    allocResourcesSw();
  }
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
  if (frame != nullptr)   av_frame_free(&frame);
  if (swFrame != nullptr) av_frame_free(&swFrame);
  if (hwFrame != nullptr) av_frame_free(&hwFrame);
  if (pkt != nullptr)     av_packet_free(&pkt);
  if (sws != nullptr) { sws_freeContext(sws); sws = nullptr; }
  if (fgraph != nullptr) { avfilter_graph_free(&fgraph); fgraph = nullptr; }
  fsrc = nullptr;
  fsink = nullptr;
  if (hwDeviceRef != nullptr) av_buffer_unref(&hwDeviceRef);
  stream = nullptr;
}

bool RTPStreamer::encodeAndSend() {
  int status = avcodec_receive_packet(codecCtx, pkt);
  while (status == 0) {
    av_packet_rescale_ts(pkt, codecCtx->time_base, stream->time_base);
    av_interleaved_write_frame(fmtCtx, pkt);
    av_packet_unref(pkt);
    status = avcodec_receive_packet(codecCtx, pkt);
  }
  if (status != AVERROR(EAGAIN) && status != AVERROR_EOF) {
    std::cerr << "[RTPStreamer] Encoder error: " << status << std::endl;
    return false;
  }
  return true;
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

    const uint8_t* srcSlice[4] = {local.data(), nullptr, nullptr, nullptr};
    int srcStride[4] = {static_cast<int>(local.size() / lh), 0, 0, 0};

    if (useHw) {
      av_frame_make_writable(swFrame);
      sws_scale(sws, srcSlice, srcStride, 0, lh, swFrame->data, swFrame->linesize);
      swFrame->pts = currentFrameId++;
      if (av_buffersrc_add_frame_flags(fsrc, swFrame, AV_BUFFERSRC_FLAG_KEEP_REF) >= 0) {
        while (av_buffersink_get_frame(fsink, hwFrame) >= 0) {
          avcodec_send_frame(codecCtx, hwFrame);
          av_frame_unref(hwFrame);
          encodeAndSend();
        }
      }
    } else {
      av_frame_make_writable(frame);
      sws_scale(sws, srcSlice, srcStride, 0, lh, frame->data, frame->linesize);
      frame->pts = currentFrameId++;
      avcodec_send_frame(codecCtx, frame);
      encodeAndSend();
    }

    auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
    auto target = std::chrono::microseconds(frametime_us);
    if (elapsed < target)
      std::this_thread::sleep_for(target - elapsed);
  }
}

#endif  // RTP_STREAM
