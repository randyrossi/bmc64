//
// fb2.h
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

class FrameBuffer2 {
public:
  FrameBuffer2();
  virtual ~FrameBuffer2();

  // Sets the layer for this frame buffer. Must be called before Allocate.
  void SetLayer(int layer);

  // Tells this fb to use a palette with transparency.
  // Must be called before Allocate.  If transparency is used, palette
  // calls must use the 32 bit ARGB format.  Otherwise, the RGB565 format
  // is used.
  void SetTransparency(bool transparency);

  int Allocate(uint8_t **pixels, int width, int height, int *pitch);
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

  // The desired X:Y ratio of scaled image (i.e. 16:9 = 1.777)
  // If positive, the src region height is scaled up to the height of the
  // frame buffer and the width is determined by height * aspect.
  // If negative, the src region width is scaled up to the width of the
  // frame buffer and the height is determiend by width / aspect.
  void SetAspect(double aspect);

  // alignment can be -1 = ALIGN TOP, 0 = CENTER, 1 = ALIGN BOTTOM
  // padding applies to TOP or BOTTOM only
  void SetVerticalAlignment(int alignment, int padding);

  // alignment can be -1 = ALIGN LEFT, 0 = CENTER, 1 = ALIGN RIGHT
  // padding applies to LEFT or RIGHT only
  void SetHorizontalAlignment(int alignment, int padding);

  // initializes the bcm_host interface
  static void Initialize();

  // make off screen resources for fb1 (and optionally fb2) visible
  // then swap destination resources in prep for next frame
  static void SwapResources(FrameBuffer2* fb1, FrameBuffer2* fb2);

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
  double aspect_;
  // -1 = top, 0 = center, 1 = bottom
  int valign_;
  // -1 = left, 0 = center, 1 = right
  int halign_;
  // Represents the resource currently visible
  int rnum_;

  int display_width_;
  int display_height_;

  int src_x_;
  int src_y_;
  int src_w_;
  int src_h_;

  // Computed from aspect_, defines the region within our display's
  // bounds for the scaled buffer
  int dst_x_;
  int dst_y_;
  int dst_w_;
  int dst_h_;

  bool showing_;
  bool allocated_;
};

#endif
