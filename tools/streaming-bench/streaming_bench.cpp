// streaming_bench - measure the per-frame CPU cost of the RTP livestream's
// hot operations (the work in src/app/rtpstreamer.cpp), independent of the
// full ssl-vision build. Run this ON THE TARGET MACHINE (e.g. the NUC) to get
// numbers you can trust for that hardware.
//
//   (1) memcpy of a frame      -> the cost added to the CAPTURE thread (sendFrame)
//   (2) swscale RGB->NV12      -> the per-frame cost on the ENCODER thread
//                                 across output resolutions and scaler flags
//   (3) H.264 encode           -> tries hardware encoders (qsv/nvenc) and the
//                                 libx264 software fallback, reports ms/frame
//
// Build: make   (see Makefile / README.md)
#include <chrono>
#include <cstring>
#include <cstdio>
#include <vector>
extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

using clk = std::chrono::high_resolution_clock;
static double ms(clk::duration d) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count() / 1e6;
}

// Source camera resolution (BFS-U3-51S5C-C full frame).
static const int SRC_W = 2448, SRC_H = 2048;

static double memcpyBench(int N) {
  const size_t n = (size_t)SRC_W * SRC_H * 3;  // RGB8
  std::vector<unsigned char> a(n), b(n);
  std::memset(a.data(), 7, n);
  volatile unsigned long sink = 0;
  auto t0 = clk::now();
  for (int i = 0; i < N; i++) {
    std::memcpy(b.data(), a.data(), n);
    sink += b[(i * 4096) % n];  // defeat dead-code elimination
  }
  auto t1 = clk::now();
  (void)sink;
  return ms(t1 - t0) / N;
}

static double swsBench(int dw, int dh, int flags, int N) {
  std::vector<unsigned char> rgb((size_t)SRC_W * SRC_H * 3);
  std::memset(rgb.data(), 7, rgb.size());
  AVFrame* f = av_frame_alloc();
  f->format = AV_PIX_FMT_NV12; f->width = dw; f->height = dh;
  av_frame_get_buffer(f, 32);
  SwsContext* s = sws_getContext(SRC_W, SRC_H, AV_PIX_FMT_RGB24, dw, dh,
                                 AV_PIX_FMT_NV12, flags, nullptr, nullptr, nullptr);
  const uint8_t* src[4] = {rgb.data(), nullptr, nullptr, nullptr};
  int ss[4] = {SRC_W * 3, 0, 0, 0};
  auto t0 = clk::now();
  for (int i = 0; i < N; i++) sws_scale(s, src, ss, 0, SRC_H, f->data, f->linesize);
  auto t1 = clk::now();
  sws_freeContext(s); av_frame_free(&f);
  return ms(t1 - t0) / N;
}

// Returns ms/frame for a software-NV12-fed encoder, or -1 if unavailable/failed.
// (h264_vaapi is not exercised here: it needs a hardware frames context, which
// this simple bench does not set up. See the route-b implementation for that.)
static double encBench(const char* name, int w, int h, int M) {
  const AVCodec* c = avcodec_find_encoder_by_name(name);
  if (!c) return -1;
  AVCodecContext* cc = avcodec_alloc_context3(c);
  cc->width = w; cc->height = h; cc->time_base = {1, 30};
  cc->pix_fmt = AV_PIX_FMT_NV12; cc->max_b_frames = 0; cc->bit_rate = 3500000;
  cc->gop_size = 30;
  if (strcmp(name, "libx264") == 0) {
    av_opt_set(cc->priv_data, "preset", "ultrafast", 0);
    av_opt_set(cc->priv_data, "tune", "zerolatency", 0);
  }
  if (avcodec_open2(cc, c, nullptr) != 0) { avcodec_free_context(&cc); return -1; }
  AVFrame* f = av_frame_alloc();
  f->format = AV_PIX_FMT_NV12; f->width = w; f->height = h;
  av_frame_get_buffer(f, 32);
  AVPacket* pkt = av_packet_alloc();
  // probe one frame to confirm it really encodes (some HW encoders open but fail here)
  av_frame_make_writable(f); f->pts = 0;
  if (avcodec_send_frame(cc, f) != 0) {
    av_frame_free(&f); av_packet_free(&pkt); avcodec_free_context(&cc); return -1;
  }
  while (avcodec_receive_packet(cc, pkt) == 0) av_packet_unref(pkt);
  auto t0 = clk::now();
  for (int i = 1; i <= M; i++) {
    av_frame_make_writable(f); f->pts = i;
    avcodec_send_frame(cc, f);
    while (avcodec_receive_packet(cc, pkt) == 0) av_packet_unref(pkt);
  }
  auto t1 = clk::now();
  av_frame_free(&f); av_packet_free(&pkt); avcodec_free_context(&cc);
  return ms(t1 - t0) / M;
}

int main() {
  printf("== streaming_bench (source %dx%d) ==\n\n", SRC_W, SRC_H);

  printf("[1] memcpy frame (capture-thread cost): %.2f ms\n\n", memcpyBench(500));

  printf("[2] swscale RGB->NV12 (encoder-thread cost), ms/frame:\n");
  struct { const char* n; int f; } flags[] = {
    {"point", SWS_POINT}, {"fast_bilinear", SWS_FAST_BILINEAR}, {"bilinear", SWS_BILINEAR}};
  struct { const char* n; int w, h; } res[] = {
    {"5MP  ", SRC_W, SRC_H}, {"1080p", 1920, 1080}, {"720p ", 1280, 720}};
  printf("    %-10s %8s %8s %8s\n", "out", flags[0].n, flags[1].n, flags[2].n);
  for (auto& r : res) {
    printf("    %-10s", r.n);
    for (auto& fl : flags) printf(" %8.2f", swsBench(r.w, r.h, fl.f, 200));
    printf("\n");
  }
  printf("\n");

  printf("[3] H.264 encode, ms/frame (-1 = unavailable on this machine):\n");
  const char* encs[] = {"h264_qsv", "h264_nvenc", "libx264"};
  printf("    %-12s %8s %8s %8s\n", "encoder", "5MP", "1080p", "720p");
  for (const char* e : encs) {
    printf("    %-12s %8.2f %8.2f %8.2f\n", e,
           encBench(e, SRC_W, SRC_H, 60), encBench(e, 1920, 1080, 120),
           encBench(e, 1280, 720, 120));
  }
  printf("\nNote: HW encode (qsv/nvenc/vaapi) offloads to the GPU; on a 2-core\n"
         "host the software libx264 numbers are what compete with detection.\n");
  return 0;
}
