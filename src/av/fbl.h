//
// fbl.h
//
// The Pi 4's and Pi 5's FrameBufferLayer: the same interface as BMC64's
// dispmanx and OpenGL ES one (src/fbl.h, which includes this on those
// boards), on the HVS (src/av/screen.h). Each layer that is showing is a
// plane: its pixels (8-bit indexed, or RGB565) are turned into 32-bit
// colour through its palette by the ARM, into one of two buffers, and the
// HVS scales the plane to its place, sharp or smooth, blending layers with
// transparency by their pixels' alpha. Higher layers are on top.
//
// With the shader on, the 3D GPU draws the layer through crt-pi at the
// size it is shown (src/v3d/crt.h), and the HVS shows that unscaled.
//
// Sources:
//  - BMC64, src/fbl.h and src/fbl.cpp: the interface, what each call does,
//    the default palettes and how the destination rectangle is worked out
//    from the stretch, alignment, padding and centre offset.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_AV_FBL_H_
#define BMC64_AV_FBL_H_

#include <stdint.h>

class Pi45Screen;
namespace v3d {
class Crt;
}

class FrameBufferLayer {
public:
  FrameBufferLayer();
  virtual ~FrameBufferLayer();

  // Sets the layer for this frame buffer. Must be called before Allocate.
  void SetLayer(int layer);
  int GetLayer(void);

  // Tells this fb to use a palette with transparency. Must be called
  // before Allocate. If transparency is used, palette calls must use the
  // 32 bit ARGB format. Otherwise, the RGB565 format is used.
  void SetTransparency(bool transparency);

  // pixel mode: 0=8-bit-indexed, 1=RGB565
  // pitch represents bytes per line
  int Allocate(int pixelmode, uint8_t **pixels, int width, int height, int *pitch);

  // Allocate again with the same parameters (the shader on or off).
  int ReAllocate(bool shader_enable);

  void Free();
  void Clear();

  void *GetPixels();

  // The pixels hold a complete frame: to the buffer shown next if
  // TO_OFFSCREEN, else to the one on the screen.
  void FrameReady(int to_offscreen);

  void Show();
  void Hide();

  void SetPalette(uint8_t index, uint16_t rgb565);
  void SetPalette(uint8_t index, uint32_t argb);
  void UpdatePalette();

  void SetSrcRect(int x, int y, int w, int h);
  void SetStretch(double hstretch, double vstretch, int hintstr, int vintstr, int use_hintstr,
                  int use_vintstr);
  void SetCenterOffset(int cx, int cy);
  void SetVerticalAlignment(int alignment, int padding);
  void SetHorizontalAlignment(int alignment, int padding);
  void SetPadding(double leftPadding, double rightPadding, double topPadding,
                  double bottomPadding);

  void GetDimensions(int *display_w, int *display_h, int *fb_w, int *fb_h, int *src_w,
                     int *src_h, int *dst_w, int *dst_h);

  // Takes the HVS (and the 3D GPU), at start up on core 0.
  static void Initialize();
  // On the Pi 3 this sets up OpenGL ES; nothing to do here.
  static void OGLInit();

  bool UsesShader();
  bool Showing();
  void SetUsesShader(bool enable);
  void SetShaderParams(bool curvature, float curvature_x, float curvature_y, int mask,
                       float mask_brightness, bool gamma, bool fake_gamma, bool scanlines,
                       bool multisample, float scanline_weight, float scanline_gap_brightness,
                       float bloom_factor, float input_gamma, float output_gamma, bool sharper,
                       bool bilinear_interpolation);

  // Shows the frames made ready off screen for FB1 (and FB2) from the next
  // frame; with SYNC, waits until they are on the screen.
  static void SwapResources(bool sync, FrameBufferLayer *fb1, FrameBufferLayer *fb2);
  static void SetInterpolation(int enable);

  // Changes the video mode to run at FPS frames a second (see
  // Pi45Screen::MatchRate()) and lays the layers out again.
  static void MatchRate(double fps, unsigned audio_fs, unsigned lines);

private:
  void FreeInternal(bool keepPixels);
  void BuildLut(void);
  void Convert(uint32_t *buffer);
  void Layout(void);
  bool DrawnByGpu(void) const;
  bool PrepareGpu(void);
  // Puts the layers showing on the screen.
  static void Commit(bool wait);

  uint8_t *pixels_;
  int fb_width_;
  int fb_height_;
  int fb_pitch_;
  int bytes_per_pixel_;
  int layer_;
  bool transparency_;

  double hstretch_;
  double vstretch_;
  int hintstr_;
  int vintstr_;
  int use_hintstr_;
  int use_vintstr_;
  int valign_;   // -1 top, 0 centre, 1 bottom
  int vpadding_;
  int halign_;   // -1 left, 0 centre, 1 right
  int hpadding_;
  int h_center_offset_;
  int v_center_offset_;
  double leftPadding_;
  double rightPadding_;
  double topPadding_;
  double bottomPadding_;

  int display_width_;
  int display_height_;
  int src_x_, src_y_, src_w_, src_h_;
  int dst_x_, dst_y_, dst_w_, dst_h_;

  bool showing_;
  bool allocated_;

  uint16_t pal_565_[256];
  uint32_t pal_argb_[256];
  uint32_t lut_[256];   // what each index (or, for RGB565, nothing) becomes

  // The two buffers the HVS shows in turn, BUFFER_PITCH bytes a line.
  uint32_t *buffer_[2];
  unsigned buffer_pitch_;
  unsigned buffer_words_;
  int front_;   // the one on the screen

  // The 3D GPU's picture (with the shader on): DST_W x DST_H, or less.
  bool uses_shader_;
  bool curvature_;
  bool bilinear_interpolation_;
  uint32_t *gpu_buffer_[2];
  unsigned gpu_pitch_;
  unsigned gpu_words_;
  unsigned gpu_w_, gpu_h_;
  bool gpu_ready_;

  static Pi45Screen *screen_;
  static v3d::Crt *crt_;
  static bool smooth_;
  static FrameBufferLayer *layers_[8];
  static unsigned num_layers_;
};

#endif
