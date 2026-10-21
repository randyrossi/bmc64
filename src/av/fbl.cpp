//
// fbl.cpp
//
// See fbl.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "fbl.h"

#include "screen.h"
#include "../v3d/crt.h"
#include "../v3d/gpu.h"

#include <circle/logger.h>
#include <circle/synchronize.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char FromFbl[] = "fbl";

#define RGB565(r, g, b) (((r) >> 3) << 11 | ((g) >> 2) << 5 | (b) >> 3)
#define ARGB(a, r, g, b) \
  ((uint32_t)((uint8_t)(a) << 24 | (uint8_t)(r) << 16 | (uint8_t)(g) << 8 | (uint8_t)(b)))

// BMC64's default palettes (src/fbl.cpp): the first 16 colours, and for
// layers with transparency index 16 fully transparent.
static const uint16_t pal_565[16] = {
    RGB565(0x00, 0x00, 0x00), RGB565(0xFF, 0xFF, 0xFF), RGB565(0xFF, 0x00, 0x00),
    RGB565(0x70, 0xa4, 0xb2), RGB565(0x6f, 0x3d, 0x86), RGB565(0x58, 0x8d, 0x43),
    RGB565(0x35, 0x28, 0x79), RGB565(0xb8, 0xc7, 0x6f), RGB565(0x6f, 0x4f, 0x25),
    RGB565(0x43, 0x39, 0x00), RGB565(0x9a, 0x67, 0x59), RGB565(0x44, 0x44, 0x44),
    RGB565(0x6c, 0x6c, 0x6c), RGB565(0x9a, 0xd2, 0x84), RGB565(0x6c, 0x5e, 0xb5),
    RGB565(0x95, 0x95, 0x95),
};

static uint32_t Argb565(uint16_t c) {
  unsigned r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return ARGB(0xFF, (r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2));
}

Pi45Screen *FrameBufferLayer::screen_ = 0;
v3d::Crt *FrameBufferLayer::crt_ = 0;
bool FrameBufferLayer::smooth_ = true;
FrameBufferLayer *FrameBufferLayer::layers_[8];
unsigned FrameBufferLayer::num_layers_ = 0;

FrameBufferLayer::FrameBufferLayer()
    : pixels_(nullptr), fb_width_(0), fb_height_(0), fb_pitch_(0), bytes_per_pixel_(1),
      layer_(0), transparency_(false), hstretch_(1.6), vstretch_(1.0), hintstr_(0),
      vintstr_(0), use_hintstr_(0), use_vintstr_(0), valign_(0), vpadding_(0), halign_(0),
      hpadding_(0), h_center_offset_(0), v_center_offset_(0), leftPadding_(0),
      rightPadding_(0), topPadding_(0), bottomPadding_(0), display_width_(0),
      display_height_(0), src_x_(0), src_y_(0), src_w_(0), src_h_(0), dst_x_(0), dst_y_(0),
      dst_w_(0), dst_h_(0), showing_(false), allocated_(false), buffer_pitch_(0),
      buffer_words_(0), front_(0), uses_shader_(false), curvature_(false),
      bilinear_interpolation_(false), gpu_pitch_(0), gpu_words_(0), gpu_w_(0), gpu_h_(0),
      gpu_ready_(false) {
  for (int i = 0; i < 256; i++) {
    pal_565_[i] = i < 16 ? pal_565[i] : 0;
    pal_argb_[i] = i < 16 ? Argb565(pal_565[i]) : 0;
  }
  buffer_[0] = buffer_[1] = 0;
  gpu_buffer_[0] = gpu_buffer_[1] = 0;
  if (num_layers_ < sizeof layers_ / sizeof layers_[0]) {
    layers_[num_layers_++] = this;
  }
}

FrameBufferLayer::~FrameBufferLayer() {
  if (showing_) {
    Hide();
  }
  if (allocated_) {
    Free();
  }
  for (unsigned i = 0; i < num_layers_; i++) {
    if (layers_[i] == this) {
      layers_[i] = layers_[--num_layers_];
      break;
    }
  }
}

// static
void FrameBufferLayer::Initialize() {
  if (screen_ != 0) {
    return;
  }
  screen_ = new Pi45Screen;
  if (!screen_->Initialize()) {
    return;
  }

  // The 3D GPU draws the CRT shader; without it the shader is left off.
  v3d::Gpu *gpu = new v3d::Gpu;
  v3d::Renderer *renderer = new v3d::Renderer;
  v3d::Crt *crt = new v3d::Crt;
  if (gpu->Initialize() && gpu->ClearTest() && renderer->Initialize(gpu, 4096, 2160) &&
      crt->Initialize(gpu, renderer, 1024, 640)) {
    crt_ = crt;
  } else {
    CLogger::Get()->Write(FromFbl, LogWarning, "The 3D GPU isn't working: no CRT shader");
  }
}

// static
void FrameBufferLayer::OGLInit() {}

void FrameBufferLayer::SetLayer(int layer) { layer_ = layer; }

int FrameBufferLayer::GetLayer() { return layer_; }

bool FrameBufferLayer::UsesShader() { return uses_shader_; }

bool FrameBufferLayer::Showing() { return showing_; }

void FrameBufferLayer::SetTransparency(bool transparency) { transparency_ = transparency; }

void FrameBufferLayer::SetUsesShader(bool enable) { uses_shader_ = enable; }

int FrameBufferLayer::Allocate(int pixelmode, uint8_t **pixels, int width, int height,
                               int *pitch) {
  if (allocated_) {
    FreeInternal(pixels == nullptr);
  }
  allocated_ = true;
  bytes_per_pixel_ = pixelmode == 1 ? 2 : 1;

  if (pixels) {
    fb_width_ = width;
    fb_height_ = height;
    fb_pitch_ = (width * bytes_per_pixel_ + 31) & ~31;
    pixels_ = (uint8_t *)malloc((size_t)fb_pitch_ * height);
    memset(pixels_, 0, (size_t)fb_pitch_ * height);
    *pixels = pixels_;
    if (pitch) {
      *pitch = fb_pitch_;
    }
    dst_x_ = 0;
    dst_y_ = 0;
    dst_w_ = width;
    dst_h_ = height;
    src_x_ = 0;
    src_y_ = 0;
    src_w_ = width;
    src_h_ = height;
  }

  display_width_ = screen_ != 0 ? (int)screen_->Width() : 0;
  display_height_ = screen_ != 0 ? (int)screen_->Height() : 0;

  // Two buffers for the whole layer (lines on 64-byte boundaries), so the
  // part shown can change without allocating again.
  buffer_pitch_ = ((unsigned)fb_width_ + 15) & ~15u;
  buffer_words_ = buffer_pitch_ * (unsigned)fb_height_;
  buffer_pitch_ *= 4;
  if (buffer_[0] == 0 && screen_ != 0) {
    for (int i = 0; i < 2; i++) {
      buffer_[i] = screen_->Allocate((size_t)buffer_words_ * 4);
      if (buffer_[i] == 0) {
        CLogger::Get()->Write(FromFbl, LogError, "Not enough memory for layer %d (%dx%d)",
                              layer_, fb_width_, fb_height_);
      }
    }
  }
  front_ = 0;
  BuildLut();
  return 0;
}

int FrameBufferLayer::ReAllocate(bool shader_enable) {
  if (uses_shader_ == shader_enable) {
    return 0;
  }
  bool was_showing = showing_;
  if (was_showing) {
    Hide();
  }
  uses_shader_ = shader_enable;
  if (was_showing) {
    Show();
  }
  return 0;
}

void FrameBufferLayer::Clear() {
  if (pixels_) {
    memset(pixels_, 0, (size_t)fb_height_ * fb_pitch_);
  }
}

void FrameBufferLayer::FreeInternal(bool keepPixels) {
  if (!allocated_) {
    return;
  }
  if (showing_) {
    Hide();
  }
  // What was on the screen is no longer read once the list without it is.
  if (screen_ != 0) {
    screen_->WaitForFlip();
    for (int i = 0; i < 2; i++) {
      screen_->Free(buffer_[i]);
      buffer_[i] = 0;
      screen_->Free(gpu_buffer_[i]);
      gpu_buffer_[i] = 0;
    }
  }
  gpu_words_ = 0;
  gpu_ready_ = false;
  if (!keepPixels) {
    free(pixels_);
    pixels_ = nullptr;
    fb_width_ = fb_height_ = fb_pitch_ = 0;
  }
  allocated_ = false;
}

void FrameBufferLayer::Free() { FreeInternal(false); }

void *FrameBufferLayer::GetPixels() { return pixels_; }

void FrameBufferLayer::SetPalette(uint8_t index, uint16_t rgb565) { pal_565_[index] = rgb565; }

void FrameBufferLayer::SetPalette(uint8_t index, uint32_t argb) { pal_argb_[index] = argb; }

void FrameBufferLayer::BuildLut(void) {
  for (int i = 0; i < 256; i++) {
    lut_[i] = transparency_ ? pal_argb_[i] : Argb565(pal_565_[i]);
  }
  if (crt_ != 0 && uses_shader_ && !transparency_) {
    for (int i = 0; i < 256; i++) {
      crt_->SetPalette(i, lut_[i] & 0xFFFFFF);
    }
  }
}

void FrameBufferLayer::UpdatePalette() {
  if (!allocated_) {
    return;
  }
  BuildLut();
  // dispmanx recolours what is on the screen; here it is drawn again.
  if (showing_) {
    FrameReady(0);
  }
}

// Turns the part of the pixels shown into colour in BUFFER, from its top
// left corner.
void FrameBufferLayer::Convert(uint32_t *buffer) {
  if (buffer == 0 || pixels_ == 0) {
    return;
  }
  unsigned pitch = buffer_pitch_ / 4;
  for (int y = 0; y < src_h_; y++) {
    uint32_t *out = buffer + (unsigned)y * pitch;
    if (bytes_per_pixel_ == 1) {
      const uint8_t *in = pixels_ + (size_t)(src_y_ + y) * fb_pitch_ + src_x_;
      for (int x = 0; x < src_w_; x++) {
        out[x] = lut_[in[x]];
      }
    } else {
      const uint16_t *in =
          (const uint16_t *)(pixels_ + (size_t)(src_y_ + y) * fb_pitch_) + src_x_;
      for (int x = 0; x < src_w_; x++) {
        out[x] = Argb565(in[x]);
      }
    }
  }
  CleanAndInvalidateDataCacheRange((uintptr)buffer, (size_t)src_h_ * buffer_pitch_);
}

bool FrameBufferLayer::DrawnByGpu(void) const {
  return uses_shader_ && crt_ != 0 && !transparency_ && gpu_ready_;
}

// Sets the GPU up to draw this layer at its size on the screen, with
// buffers for it. False if it can't (the HVS then scales the plain pixels).
bool FrameBufferLayer::PrepareGpu(void) {
  gpu_ready_ = false;
  if (!uses_shader_ || crt_ == 0 || transparency_ || screen_ == 0 ||
      dst_w_ <= 0 || dst_h_ <= 0) {
    return false;
  }
  // At most 1080 lines; the HVS scales a bigger picture up from that.
  unsigned divide = ((unsigned)dst_h_ + 1079) / 1080;
  gpu_w_ = (unsigned)dst_w_ / divide;
  gpu_h_ = (unsigned)dst_h_ / divide;
  unsigned pitch = ((gpu_w_ + 15) & ~15u) * 4;
  unsigned words = pitch / 4 * gpu_h_;
  if (words > gpu_words_) {
    screen_->WaitForFlip();
    for (int i = 0; i < 2; i++) {
      screen_->Free(gpu_buffer_[i]);
      gpu_buffer_[i] = screen_->Allocate((size_t)words * 4);
    }
    if (gpu_buffer_[0] == 0 || gpu_buffer_[1] == 0 ||
        !v3d::Gpu::Reachable(gpu_buffer_[0], (size_t)words * 4) ||
        !v3d::Gpu::Reachable(gpu_buffer_[1], (size_t)words * 4)) {
      CLogger::Get()->Write(FromFbl, LogWarning, "No memory the GPU can draw the CRT shader in");
      for (int i = 0; i < 2; i++) {
        screen_->Free(gpu_buffer_[i]);
        gpu_buffer_[i] = 0;
      }
      gpu_words_ = 0;
      return false;
    }
    gpu_words_ = words;
  }
  gpu_pitch_ = pitch;
  crt_->SetMode(true, bilinear_interpolation_, curvature_);
  if (!crt_->Configure((unsigned)fb_pitch_, src_x_, src_y_, src_w_, src_h_, gpu_w_, gpu_h_,
                       (unsigned)bytes_per_pixel_)) {
    return false;
  }
  for (int i = 0; i < 256; i++) {
    crt_->SetPalette(i, lut_[i] & 0xFFFFFF);
  }
  gpu_ready_ = true;
  return true;
}

void FrameBufferLayer::FrameReady(int to_offscreen) {
  if (!allocated_ || screen_ == 0) {
    return;
  }
  int index = to_offscreen ? 1 - front_ : front_;
  // The buffer drawn next may still be read by the HVS until the last
  // flip is on the screen.
  if (to_offscreen) {
    screen_->WaitForFlip();
  }
  if (DrawnByGpu()) {
    v3d::Target target = {gpu_buffer_[index], gpu_w_, gpu_h_, gpu_pitch_};
    if (crt_->Draw(pixels_, target, 0, 0)) {
      return;
    }
    // The GPU failed: the HVS scales the pixels from now on.
    CLogger::Get()->Write(FromFbl, LogWarning, "The GPU failed; CRT shader off");
    gpu_ready_ = false;
    crt_ = 0;
    Convert(buffer_[0]);
    Convert(buffer_[1]);
    Commit(false);
    return;
  }
  Convert(buffer_[index]);
}

// static
void FrameBufferLayer::SwapResources(bool sync, FrameBufferLayer *fb1, FrameBufferLayer *fb2) {
  if (!sync) {
    // The frames went to the buffers on the screen.
    return;
  }
  if (fb1 != 0 && fb1->showing_) {
    fb1->front_ ^= 1;
  }
  if (fb2 != 0 && fb2->showing_) {
    fb2->front_ ^= 1;
  }
  Commit(true);
}

// The destination rectangle, as BMC64's src/fbl.cpp Show() works it out.
void FrameBufferLayer::Layout(void) {
  int dst_w;
  int dst_h;

  int lpad_abs = display_width_ * leftPadding_;
  int rpad_abs = display_width_ * rightPadding_;
  int tpad_abs = display_height_ * topPadding_;
  int bpad_abs = display_height_ * bottomPadding_;

  int avail_width = display_width_ - lpad_abs - rpad_abs;
  int avail_height = display_height_ - tpad_abs - bpad_abs;

  if (hstretch_ < 0) {
    // Stretch horizontally to fill width * vstretch and then set height
    // based on hstretch. This mode doesn't support integer stretch.
    dst_w = avail_width * vstretch_;
    dst_h = avail_width / -hstretch_;
    if (dst_w > avail_width) {
      dst_w = avail_width;
    }
    if (dst_h > avail_height) {
      dst_h = avail_height;
    }
  } else {
    // Stretch vertically to fill height * vstretch and then set width
    // based on hstretch.
    dst_h = avail_height * vstretch_;
    if (use_vintstr_) {
      dst_h = vintstr_;
    }
    dst_w = avail_height * hstretch_;
    if (use_hintstr_) {
      dst_w = hintstr_;
    }
    if (dst_h > avail_height) {
      dst_h = avail_height;
    }
    if (dst_w > avail_width) {
      dst_w = avail_width;
    }
  }

  int oy;
  switch (valign_) {
  case 0:
    oy = (avail_height - dst_h) / 2 + v_center_offset_;
    break;
  case -1:
    oy = vpadding_;
    break;
  case 1:
    oy = avail_height - dst_h - vpadding_;
    break;
  default:
    oy = 0;
    break;
  }

  int ox;
  switch (halign_) {
  case 0:
    ox = (avail_width - dst_w) / 2 + h_center_offset_;
    break;
  case -1:
    ox = hpadding_;
    break;
  case 1:
    ox = avail_width - dst_w - hpadding_;
    break;
  default:
    ox = 0;
    break;
  }

  dst_x_ = ox + lpad_abs;
  dst_y_ = oy + tpad_abs;
  dst_w_ = dst_w;
  dst_h_ = dst_h;
}

void FrameBufferLayer::Show() {
  if (showing_ || !allocated_) {
    return;
  }
  if (screen_ != 0) {
    display_width_ = (int)screen_->Width();
    display_height_ = (int)screen_->Height();
  }
  Layout();
  // The emulator screens' layout, when it changes (debug level).
  static int logged[2][4];
  if ((layer_ == 0 || layer_ == 1) &&
      (logged[layer_][0] != display_height_ || logged[layer_][1] != dst_w_ ||
       logged[layer_][2] != dst_h_ || logged[layer_][3] != dst_x_ + dst_y_ * 8192)) {
    logged[layer_][0] = display_height_;
    logged[layer_][1] = dst_w_;
    logged[layer_][2] = dst_h_;
    logged[layer_][3] = dst_x_ + dst_y_ * 8192;
    CLogger::Get()->Write(FromFbl, LogDebug,
                          "Layer %d on %dx%d: %dx%d at %d,%d (stretch %.3f %.3f, integer %d %d: %dx%d)",
                          layer_, display_width_, display_height_, dst_w_, dst_h_, dst_x_, dst_y_,
                          hstretch_, vstretch_, use_hintstr_, use_vintstr_, hintstr_, vintstr_);
  }
  if (uses_shader_) {
    PrepareGpu();
  }
  BuildLut();
  showing_ = true;
  // Both buffers, so whichever is shown next has the frame.
  FrameReady(0);
  if (DrawnByGpu()) {
    v3d::Target target = {gpu_buffer_[1 - front_], gpu_w_, gpu_h_, gpu_pitch_};
    crt_->Draw(pixels_, target, 0, 0);
  } else {
    Convert(buffer_[1 - front_]);
  }
  Commit(false);
}

void FrameBufferLayer::Hide() {
  if (!showing_) {
    return;
  }
  showing_ = false;
  Commit(false);
}

// static
void FrameBufferLayer::Commit(bool wait) {
  if (screen_ == 0 || !screen_->Ready()) {
    return;
  }
  // The layers showing, lowest first.
  FrameBufferLayer *order[8];
  unsigned count = 0;
  for (unsigned i = 0; i < num_layers_; i++) {
    FrameBufferLayer *l = layers_[i];
    if (l->showing_ && l->allocated_ && l->buffer_[l->front_] != 0) {
      unsigned at = count++;
      while (at > 0 && order[at - 1]->layer_ > l->layer_) {
        order[at] = order[at - 1];
        at--;
      }
      order[at] = l;
    }
  }
  if (count > kms::Hvs::MaxPlanes) {
    count = kms::Hvs::MaxPlanes;
  }

  const int sw = (int)screen_->Width(), sh = (int)screen_->Height();
  kms::Plane planes[kms::Hvs::MaxPlanes];
  unsigned n = 0;
  for (unsigned i = 0; i < count; i++) {
    FrameBufferLayer *l = order[i];
    kms::Plane &p = planes[n];
    memset(&p, 0, sizeof p);
    int x = l->dst_x_, y = l->dst_y_, w = l->dst_w_, h = l->dst_h_;
    int sx = 0, sy = 0, src_w, src_h;
    const uint32_t *pixels;
    unsigned pitch;
    if (l->DrawnByGpu()) {
      pixels = l->gpu_buffer_[l->front_];
      pitch = l->gpu_pitch_;
      src_w = (int)l->gpu_w_;
      src_h = (int)l->gpu_h_;
    } else {
      pixels = l->buffer_[l->front_];
      pitch = l->buffer_pitch_;
      src_w = l->src_w_;
      src_h = l->src_h_;
    }
    // What falls off the screen is cut off, from the source too.
    if (w <= 0 || h <= 0 || src_w <= 0 || src_h <= 0) {
      continue;
    }
    if (x < 0) {
      sx = (-x * src_w) / w;
      w += x;
      x = 0;
    }
    if (y < 0) {
      sy = (-y * src_h) / h;
      h += y;
      y = 0;
    }
    if (x + w > sw) {
      w = sw - x;
    }
    if (y + h > sh) {
      h = sh - y;
    }
    if (w <= 0 || h <= 0) {
      continue;
    }
    int cut_w = (int)((long)w * src_w / (l->dst_w_ > 0 ? l->dst_w_ : 1));
    int cut_h = (int)((long)h * src_h / (l->dst_h_ > 0 ? l->dst_h_ : 1));
    src_w = cut_w > 0 ? cut_w : 1;
    src_h = cut_h > 0 ? cut_h : 1;
    p.address = screen_->HvsAddress((const uint8_t *)pixels + (size_t)sy * pitch + sx * 4);
    p.pitch = pitch;
    p.src_width = (unsigned)src_w;
    p.src_height = (unsigned)src_h;
    p.x = (unsigned)x;
    p.y = (unsigned)y;
    p.width = (unsigned)w;
    p.height = (unsigned)h;
    p.nearest = l->DrawnByGpu() ? true : !smooth_;
    p.alpha = l->transparency_;
    n++;
  }
  if (!screen_->Present(planes, n, wait)) {
    static bool logged = false;
    if (!logged) {
      CLogger::Get()->Write(FromFbl, LogWarning, "The HVS can't show the layers so");
      logged = true;
    }
  }
}

void FrameBufferLayer::SetSrcRect(int x, int y, int w, int h) {
  if (x < 0) {
    x = 0;
  }
  if (y < 0) {
    y = 0;
  }
  if (x + w > fb_width_) {
    w = fb_width_ - x;
  }
  if (y + h > fb_height_) {
    h = fb_height_ - y;
  }
  src_x_ = x;
  src_y_ = y;
  src_w_ = w;
  src_h_ = h;
}

void FrameBufferLayer::SetStretch(double hstretch, double vstretch, int hintstr, int vintstr,
                                  int use_hintstr, int use_vintstr) {
  if (hstretch == 0 || vstretch == 0) {
    // As BMC64's: a zero component would collapse the destination.
    printf("fbl: SetStretch layer=%d given a zero component; keeping usable value\n", layer_);
    if (hstretch == 0) hstretch = hstretch_ != 0 ? hstretch_ : 1.0;
    if (vstretch == 0) vstretch = vstretch_ != 0 ? vstretch_ : 1.0;
  }
  hstretch_ = hstretch;
  vstretch_ = vstretch;
  hintstr_ = hintstr;
  vintstr_ = vintstr;
  use_hintstr_ = use_hintstr;
  use_vintstr_ = use_vintstr;
}

void FrameBufferLayer::SetVerticalAlignment(int alignment, int padding) {
  valign_ = alignment;
  vpadding_ = padding;
}

void FrameBufferLayer::SetHorizontalAlignment(int alignment, int padding) {
  halign_ = alignment;
  hpadding_ = padding;
}

void FrameBufferLayer::SetPadding(double leftPadding, double rightPadding, double topPadding,
                                  double bottomPadding) {
  leftPadding_ = leftPadding;
  rightPadding_ = rightPadding;
  topPadding_ = topPadding;
  bottomPadding_ = bottomPadding;
}

void FrameBufferLayer::SetCenterOffset(int cx, int cy) {
  h_center_offset_ = cx;
  v_center_offset_ = cy;
}

void FrameBufferLayer::GetDimensions(int *display_w, int *display_h, int *fb_w, int *fb_h,
                                     int *src_w, int *src_h, int *dst_w, int *dst_h) {
  if (screen_ != 0 && screen_->Ready()) {
    display_width_ = (int)screen_->Width();
    display_height_ = (int)screen_->Height();
  }
  *display_w = display_width_;
  *display_h = display_height_;
  *fb_w = fb_width_;
  *fb_h = fb_height_;
  *src_w = src_w_;
  *src_h = src_h_;
  *dst_w = dst_w_;
  *dst_h = dst_h_;
}

void FrameBufferLayer::SetShaderParams(bool curvature, float curvature_x, float curvature_y,
                                       int mask, float mask_brightness, bool gamma,
                                       bool fake_gamma, bool scanlines, bool multisample,
                                       float scanline_weight, float scanline_gap_brightness,
                                       float bloom_factor, float input_gamma,
                                       float output_gamma, bool sharper,
                                       bool bilinear_interpolation) {
  // The GPU's crt-pi has BMC64's default look built in (src/v3d/crt.h);
  // curvature and smoothing can be switched.
  (void)curvature_x;
  (void)curvature_y;
  (void)mask;
  (void)mask_brightness;
  (void)gamma;
  (void)fake_gamma;
  (void)scanlines;
  (void)multisample;
  (void)scanline_weight;
  (void)scanline_gap_brightness;
  (void)bloom_factor;
  (void)input_gamma;
  (void)output_gamma;
  (void)sharper;
  curvature_ = curvature;
  bilinear_interpolation_ = bilinear_interpolation;
  // As BMC64's: takes effect at the next Show().
  Hide();
}

// static
void FrameBufferLayer::SetInterpolation(int enable) {
  smooth_ = enable != 0;
  Commit(false);
}

// static
void FrameBufferLayer::MatchRate(double fps, unsigned audio_fs, unsigned lines) {
  if (screen_ == 0 || !screen_->MatchRate(fps, audio_fs, lines)) {
    return;
  }
  // Laid out again for the new mode.
  for (unsigned i = 0; i < num_layers_; i++) {
    FrameBufferLayer *l = layers_[i];
    if (l->showing_) {
      l->showing_ = false;
      l->Show();
    }
  }
  Commit(false);
}
