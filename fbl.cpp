//
// fbl.cpp
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

#include "fbl.h"

#include <stdio.h>

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif

// Always use an indexed mode.
#define IMG_TYPE VC_IMAGE_8BPP
#define BYTES_PER_PIXEL 1

#define RGB565(r,g,b) (((r)>>3)<<11 | ((g)>>2)<<5 | (b)>>3)
#define ARGB(a,r,g,b) ((uint32_t)((uint8_t)(a)<<24 | (uint8_t)(r)<<16 | (uint8_t)(g)<<8 | (uint8_t)(b)))

// Default palette used for canvases with no transparency
static uint16_t pal_565[256] = {
  RGB565(0x00, 0x00, 0x00),
  RGB565(0xFF, 0xFF, 0xFF),
  RGB565(0xFF, 0x00, 0x00),
  RGB565(0x70, 0xa4, 0xb2),
  RGB565(0x6f, 0x3d, 0x86),
  RGB565(0x58, 0x8d, 0x43),
  RGB565(0x35, 0x28, 0x79),
  RGB565(0xb8, 0xc7, 0x6f),
  RGB565(0x6f, 0x4f, 0x25),
  RGB565(0x43, 0x39, 0x00),
  RGB565(0x9a, 0x67, 0x59),
  RGB565(0x44, 0x44, 0x44),
  RGB565(0x6c, 0x6c, 0x6c),
  RGB565(0x9a, 0xd2, 0x84),
  RGB565(0x6c, 0x5e, 0xb5),
  RGB565(0x95, 0x95, 0x95),
};

// Default palette used for canvases with transparency. This palette
// is identical to pal_565 except we include an additional
// entry for a fully transparent pixel (index 16).
static uint32_t pal_argb[256] = {
  // First 16 colors are opaque
  ARGB(0xFF, 0x00, 0x00, 0x00),
  ARGB(0xFF, 0xFF, 0xFF, 0xFF),
  ARGB(0xFF, 0xFF, 0x00, 0x00),
  ARGB(0xFF, 0x70, 0xa4, 0xb2),
  ARGB(0xFF, 0x6f, 0x3d, 0x86),
  ARGB(0xFF, 0x58, 0x8d, 0x43),
  ARGB(0xFF, 0x35, 0x28, 0x79),
  ARGB(0xFF, 0xb8, 0xc7, 0x6f),
  ARGB(0xFF, 0x6f, 0x4f, 0x25),
  ARGB(0xFF, 0x43, 0x39, 0x00),
  ARGB(0xFF, 0x9a, 0x67, 0x59),
  ARGB(0xFF, 0x44, 0x44, 0x44),
  ARGB(0xFF, 0x6c, 0x6c, 0x6c),
  ARGB(0xFF, 0x9a, 0xd2, 0x84),
  ARGB(0xFF, 0x6c, 0x5e, 0xb5),
  ARGB(0xFF, 0x95, 0x95, 0x95),

  // Use index 16 for fully transparent pixels
  ARGB(0x00, 0x00, 0x00, 0x00),
};

bool FrameBufferLayer::initialized_ = false;
DISPMANX_DISPLAY_HANDLE_T FrameBufferLayer::dispman_display_;

FrameBufferLayer::FrameBufferLayer() :
        width_(0), height_(0), pitch_(0), layer_(0), transparency_(false),
        aspect_(1.6), valign_(0), vpadding_(0), halign_(0), hpadding_(0),
        h_center_offset_(0), v_center_offset_(0),
        rnum_(0), leftPadding_(0), rightPadding_(0), topPadding_(0),
        bottomPadding_(0), showing_(false), allocated_(false) {
  alpha_.flags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE;
  alpha_.opacity = 255;
  alpha_.mask = 0;

  memcpy (pal_565_, pal_565, sizeof(pal_565));
  memcpy (pal_argb_, pal_argb, sizeof(pal_565));
}

FrameBufferLayer::~FrameBufferLayer() {
  if (showing_) {
    Hide();
  }
  if (allocated_) {
    Free();
  }
}

// static
void FrameBufferLayer::Initialize() {
  if (initialized_)
     return;

  bcm_host_init();

  dispman_display_ = vc_dispmanx_display_open(0);

  initialized_ = true;
}

int FrameBufferLayer::Allocate(uint8_t **pixels, int width, int height, int *pitch) {
  int ret;
  DISPMANX_MODEINFO_T dispman_info;
  uint32_t vc_image_ptr;

  if (allocated_) {
     Free();
  }

  allocated_ = true;

  *pitch = pitch_ = ALIGN_UP(width, 32);

  width_ = width;
  height_ = height;

  ret = vc_dispmanx_display_get_info(dispman_display_, &dispman_info);
  assert(ret == 0);

  display_width_ = dispman_info.width;
  display_height_ = dispman_info.height;

  *pixels = pixels_ = (uint8_t*) malloc(pitch_ * height * BYTES_PER_PIXEL);

  // Allocate the VC resources along with the frame buffer

  dispman_resource_[0] = vc_dispmanx_resource_create(IMG_TYPE,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  dispman_resource_[1] = vc_dispmanx_resource_create(IMG_TYPE,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  assert(dispman_resource_[0]);
  assert(dispman_resource_[1]);

  // Install the default palette
  UpdatePalette();

  vc_dispmanx_rect_set( &copy_dst_rect_, 0, 0, width, height);

  assert(ret == 0);

  // Default our src region to the entire buffer.
  src_x_ = 0;
  src_y_ = 0;
  src_w_ = width;
  src_h_ = height;

  return 0;
}

void FrameBufferLayer::Clear() {
  assert (allocated_);

  memset(pixels_, 0, height_ * pitch_ * BYTES_PER_PIXEL);
}

void FrameBufferLayer::Free() {
  int ret;

  if (!allocated_) return;

  width_ = 0;
  height_ = 0;
  pitch_ = 0;
  free(pixels_);

  ret = vc_dispmanx_resource_delete(dispman_resource_[0]);
  assert(ret == 0);
  ret = vc_dispmanx_resource_delete(dispman_resource_[1]);
  assert(ret == 0);

  allocated_ = false;
}

void FrameBufferLayer::Show() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  if (showing_) return;

  // Define source rect
  vc_dispmanx_rect_set(&src_rect_,
     src_x_ << 16, src_y_ << 16, src_w_ << 16, src_h_ << 16);

  int dst_w;
  int dst_h;
  assert (aspect_ != 0);

  int lpad_abs = display_width_ * leftPadding_;
  int rpad_abs = display_width_ * rightPadding_;
  int tpad_abs = display_height_ * topPadding_;
  int bpad_abs = display_height_ * bottomPadding_;

  int avail_width = display_width_ - lpad_abs - rpad_abs;
  int avail_height = display_height_ - tpad_abs - bpad_abs;

  if (aspect_ < 0) {
     // Stretch horizontally to fill width and then set height based on
     // aspect ratio.
     dst_w = avail_width;
     dst_h = (double)avail_width / -aspect_;
     if (dst_h > avail_height) {
        dst_h = avail_height;
     }
  } else {
     // Stretch vertically to fill height and then set width based on
     // aspect ratio.
     dst_h = avail_height;
     dst_w = (double)avail_height * aspect_;
     if (dst_w > avail_width) {
        dst_w = avail_width;
     }
  }

  // Resulting image is centered
  int oy;
  switch (valign_) {
     case 0:
        // Center
        oy = (avail_height - dst_h) / 2 + v_center_offset_;
        break;
     case -1:
        // Top
        oy = vpadding_;
        break;
     case 1:
        // Bottom
        oy = avail_height - dst_h - vpadding_;
        break;
     default:
        oy = 0;
        break;
  }

  int ox;
  switch (halign_) {
     case 0:
        // Center
        ox = (avail_width - dst_w) / 2 + h_center_offset_;
        break;
     case -1:
        // Left
        ox = hpadding_;
        break;
     case 1:
        // Right
        ox = avail_width - dst_w - hpadding_;
        break;
     default:
        ox = 0;
        break;
  }

  vc_dispmanx_rect_set(&scale_dst_rect_,
                       ox + lpad_abs,
                       oy + tpad_abs,
                       dst_w,
                       dst_h);

  dispman_update = vc_dispmanx_update_start(0);
  assert( dispman_update );

  rnum_ = 0;
  dispman_element_ = vc_dispmanx_element_add(dispman_update,
                                            dispman_display_,
                                            layer_, // layer
                                            &scale_dst_rect_,
                                            dispman_resource_[rnum_],
                                            &src_rect_,
                                            DISPMANX_PROTECTION_NONE,
                                            &alpha_,
                                            NULL,             // clamp
                                            DISPMANX_NO_ROTATE);

  ret = vc_dispmanx_update_submit(dispman_update, NULL, NULL);
  assert( ret == 0 );
  showing_ = true;
}

void FrameBufferLayer::Hide() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  if (!showing_) return;

  dispman_update = vc_dispmanx_update_start(0);
  ret = vc_dispmanx_element_remove(dispman_update, dispman_element_);
  assert(ret == 0);
  ret = vc_dispmanx_update_submit(dispman_update, NULL, NULL);
  assert(ret == 0);
  showing_ = false;
}

void* FrameBufferLayer::GetPixels() {
  return pixels_;
}

void FrameBufferLayer::FrameReady(int to_offscreen) {
  int rnum = to_offscreen ? 1 - rnum_ : rnum_;

  // Copy data into either the offscreen resource (if swap) or the
  // on screen resource (if !swap).
  vc_dispmanx_resource_write_data(dispman_resource_[rnum],
                                  IMG_TYPE,
                                  width_,
                                  pixels_,
                                  &copy_dst_rect_);
}

// Private function to change the source of this frame buffer's
// element to the off screen resource and toggle the resource
// index in preparation for the off screen data to be shown.
void FrameBufferLayer::Swap(DISPMANX_UPDATE_HANDLE_T& dispman_update) {
  rnum_ = 1 - rnum_;
  vc_dispmanx_element_change_source(dispman_update,
                                    dispman_element_,
                                    dispman_resource_[rnum_]);
}

// Static
void FrameBufferLayer::SwapResources(FrameBufferLayer* fb1,
                                     FrameBufferLayer* fb2) {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  dispman_update = vc_dispmanx_update_start(0);
  fb1->Swap(dispman_update);
  if (fb2) fb2->Swap(dispman_update);
  ret = vc_dispmanx_update_submit_sync(dispman_update);
  assert(ret == 0);
}

void FrameBufferLayer::SetPalette(uint8_t index, uint16_t rgb565) {
  assert(!transparency_);
  pal_565_[index] = rgb565;
}

void FrameBufferLayer::SetPalette(uint8_t index, uint32_t argb) {
  assert(transparency_);
  pal_argb_[index] = argb;
}

void FrameBufferLayer::UpdatePalette() {
  if (!allocated_) return;

  int ret;
  if (transparency_) {
     ret = vc_dispmanx_resource_set_palette(dispman_resource_[0],
                                            pal_argb_, 0, sizeof pal_argb_);
     ret = vc_dispmanx_resource_set_palette(dispman_resource_[1],
                                            pal_argb_, 0, sizeof pal_argb_);
  } else {
     ret = vc_dispmanx_resource_set_palette(dispman_resource_[0],
                                            pal_565_, 0, sizeof pal_565_);
     ret = vc_dispmanx_resource_set_palette(dispman_resource_[1],
                                            pal_565_, 0, sizeof pal_565_);
  }
  assert( ret == 0 );
}

void FrameBufferLayer::SetLayer(int layer) {
  layer_ = layer;
}

int FrameBufferLayer::GetLayer() {
  return layer_;
}

void FrameBufferLayer::SetTransparency(bool transparency) {
  transparency_ = transparency;
}

void FrameBufferLayer::SetSrcRect(int x, int y, int w, int h) {
  src_x_ = x;
  src_y_ = y;
  src_w_ = w;
  src_h_ = h;
}

// Set horizontal multiplier
void FrameBufferLayer::SetAspect(double aspect) {
  aspect_ = aspect;
}

void FrameBufferLayer::SetVerticalAlignment(int alignment, int padding) {
  valign_ = alignment;
  vpadding_ = padding;
}

void FrameBufferLayer::SetHorizontalAlignment(int alignment, int padding) {
  halign_ = alignment;
  hpadding_ = padding;
}

void FrameBufferLayer::SetPadding(double leftPadding,
                              double rightPadding,
                              double topPadding,
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
