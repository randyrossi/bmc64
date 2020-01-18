//
// fbl.h
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _fb2_h
#define _fb2_h

#include "bcm_host.h"

// A wrapper that manages a single dispmanx layer and
// indexed frame buffer.
class FrameBufferLayer {
public:
  FrameBufferLayer();
  virtual ~FrameBufferLayer();

  // Sets the layer for this frame buffer. Must be called before Allocate.
  void SetLayer(int layer);
  int GetLayer(void);

  // Tells this fb to use a palette with transparency.
  // Must be called before Allocate.  If transparency is used, palette
  // calls must use the 32 bit ARGB format.  Otherwise, the RGB565 format
  // is used.
  void SetTransparency(bool transparency);

  // pixel mode: 0=8-bit-indexed, 1=RGB565
  // pitch represents bytes per line
  int Allocate(int pixelmode, uint8_t **pixels,
               int width, int height, int *pitch);
  void Free();
  void Clear();

  // Get a pointer to raw pixel data for this frame buffer
  void* GetPixels();

  // Indicates raw pixel data has a complete frame. Upload to the
  // offscreen resource unless to_offscreen is 0, in which case
  // the currently visible resource is the destination.
  void FrameReady(int to_offscreen);

  // Show the framebuffer over top the first
  void Show();

  // Hide the framebuffer
  void Hide();

  // Set one color of the indexed palette. Can only be called when
  // transparency is false.
  void SetPalette(uint8_t index, uint16_t rgb565);

  // Set one color of the indexed palette. Can only be called when
  // transparency is true.
  void SetPalette(uint8_t index, uint32_t rgba);

  // Commit current palette to frame buffer
  void UpdatePalette();

  // Sets the region within the frame buffer to scale. Takes effect
  // on the next call to Show()
  void SetSrcRect(int x, int y, int w, int h);

  // The amount to stretch to vertical and horizontal dimensions
  // If hstretch is positive, the src region height is scaled up to the
  // height of the frame buffer * vstretch, the width is then determined by
  // frame buffer height * hstretch.
  // Otherwise, the src region width is scaled up to the width of the
  // frame buffer * vstretch, the height is determined by 
  // frame buffer width / hstretch.
  void SetStretch(double hstretch, double vstretch);

  void SetCenterOffset(int cx, int cy);

  // alignment can be -1 = ALIGN TOP, 0 = CENTER, 1 = ALIGN BOTTOM
  // padding applies to TOP or BOTTOM only.
  void SetVerticalAlignment(int alignment, int padding);

  // alignment can be -1 = ALIGN LEFT, 0 = CENTER, 1 = ALIGN RIGHT
  // padding applies to LEFT or RIGHT only.
  void SetHorizontalAlignment(int alignment, int padding);

  // Takes some space away from the full screen when determining destination rect
  // Used to force a fb into a smaller space (for things like PIP or side-by-side.
  void SetPadding(double leftPadding, double rightPadding, double topPadding, double bottomPadding);

  // initializes the bcm_host interface
  static void Initialize();

  // make off screen resources for fb1 (and optionally fb2) visible
  // then swap destination resources in prep for next frame
  static void SwapResources(FrameBufferLayer* fb1, FrameBufferLayer* fb2);

private:

  void Swap(DISPMANX_UPDATE_HANDLE_T& dispman_update);

  // Raw pixel data. Not VC memory.
  uint8_t* pixels_;

  static DISPMANX_DISPLAY_HANDLE_T dispman_display_;
  DISPMANX_ELEMENT_HANDLE_T dispman_element_;
  DISPMANX_RESOURCE_HANDLE_T dispman_resource_[2];

  VC_RECT_T scale_dst_rect_;
  VC_RECT_T copy_dst_rect_;

  // Defines the region within the frame buffer we are scaling
  VC_RECT_T src_rect_;
  VC_DISPMANX_ALPHA_T alpha_;

  static bool initialized_;

  int width_;
  int height_;
  int pitch_;
  int layer_;
  int transparency_;
  double hstretch_;
  double vstretch_;
  // -1 = top, 0 = center, 1 = bottom
  int valign_;
  int vpadding_;
  // -1 = left, 0 = center, 1 = right
  int halign_;
  int hpadding_;

  int h_center_offset_;
  int v_center_offset_;

  // Represents the resource currently visible
  int rnum_;

  // Used to take away some available display area
  // before deciding what the dest rect coords should be. Expressed
  // as percentage of available area.
  double leftPadding_;
  double rightPadding_;
  double topPadding_;
  double bottomPadding_;

  int display_width_;
  int display_height_;

  int src_x_;
  int src_y_;
  int src_w_;
  int src_h_;

  bool showing_;
  bool allocated_;

  VC_IMAGE_TYPE_T mode_;
  int bytes_per_pixel_;

  uint16_t pal_565_[256];
  uint32_t pal_argb_[256];
};

#endif
