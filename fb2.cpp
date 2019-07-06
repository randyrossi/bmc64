//
// fb2.cpp
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

#include "fb2.h"

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif

#define IMG_TYPE VC_IMAGE_RGB565

bool FrameBuffer2::init_ = false;

FrameBuffer2::FrameBuffer2(int width, int height) : 
        buf_num_(0), width_(width), height_(height) {

  int ret;
  DISPMANX_MODEINFO_T dispman_info;
  pitch_ = ALIGN_UP(width * 2, 32);
  //int aligned_height = ALIGN_UP(height, 16);
  uint32_t vc_image_ptr;

  init();

  ret = vc_dispmanx_display_get_info(dispman_display_, &dispman_info);
  assert(ret == 0);

  pixels_ = calloc(1, pitch_ * height);

  dispman_resource_[0] = vc_dispmanx_resource_create(IMG_TYPE,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  assert(dispman_resource_[0]);
  dispman_resource_[1] = vc_dispmanx_resource_create(IMG_TYPE,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  assert(dispman_resource_[1]);
  vc_dispmanx_rect_set( &dst_rect_, 0, 0, width, height);

  ret = vc_dispmanx_resource_write_data(dispman_resource_[0],
                                        IMG_TYPE,
                                        pitch_,
                                        pixels_,
                                        &dst_rect_ );
  assert(ret == 0);

  ret = vc_dispmanx_resource_write_data(dispman_resource_[1],
                                        IMG_TYPE,
                                        pitch_,
                                        pixels_,
                                        &dst_rect_ );
  assert( ret == 0 );

  // Define source rect
  vc_dispmanx_rect_set(&src_rect_, 0, 0, width << 16, height << 16 );

  // Stretch to full width/height of our display
  vc_dispmanx_rect_set(&dst_rect_, 0,
                                  0,
                                  dispman_info.width,
                                  dispman_info.height );

}

FrameBuffer2::~FrameBuffer2(void) {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  dispman_update = vc_dispmanx_update_start(0);
  ret = vc_dispmanx_element_remove(dispman_update, dispman_element_);
  assert(ret == 0);
  ret = vc_dispmanx_update_submit_sync(dispman_update);
  assert(ret == 0);
  ret = vc_dispmanx_resource_delete(dispman_resource_[0]);
  assert(ret == 0);
  ret = vc_dispmanx_resource_delete(dispman_resource_[1]);
  assert(ret == 0);
}

void FrameBuffer2::Show() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  dispman_update = vc_dispmanx_update_start(0);
  assert( dispman_update );

  dispman_element_ = vc_dispmanx_element_add(dispman_update,
                                            dispman_display_,
                                            1,               // layer
                                            &dst_rect_,
                                            dispman_resource_[0],
                                            &src_rect_,
                                            DISPMANX_PROTECTION_NONE,
                                            &alpha_,
                                            NULL,             // clamp
                                            DISPMANX_NO_ROTATE);

  ret = vc_dispmanx_update_submit_sync(dispman_update);
  assert( ret == 0 );
}

void FrameBuffer2::Hide() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  dispman_update = vc_dispmanx_update_start(0);
  ret = vc_dispmanx_element_remove(dispman_update, dispman_element_);
  assert(ret == 0);
  ret = vc_dispmanx_update_submit_sync(dispman_update);
  assert(ret == 0);
}

void FrameBuffer2::init() {
  if (init_)
     return;

  alpha_.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
  alpha_.opacity = 120;
  alpha_.mask = 0;

  bcm_host_init();

  dispman_display_ = vc_dispmanx_display_open(0); 

  init_ = true;
}

void* FrameBuffer2::GetPixels() {
  return pixels_;
}

void FrameBuffer2::SwapBuffers() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  int offscreen_buf_num = 1 - buf_num_;

  vc_dispmanx_rect_set( &dst_rect_, 0, 0, width_, height_);

  ret = vc_dispmanx_resource_write_data(dispman_resource_[offscreen_buf_num],
                                        IMG_TYPE,
                                        pitch_,
                                        pixels_,
                                        &dst_rect_);
  assert(ret == 0);

  dispman_update = vc_dispmanx_update_start(0);
  vc_dispmanx_element_change_source(dispman_update, dispman_element_,
                                    dispman_resource_[offscreen_buf_num]);
  ret = vc_dispmanx_update_submit_sync(dispman_update);
  assert(ret == 0);

  buf_num_ = offscreen_buf_num;
}
