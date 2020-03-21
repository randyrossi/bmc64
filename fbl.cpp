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

// Notes: This class started out being just a wrapper around the
// dispmanx API to get frame buffer pixels to the screen using just
// an element, src rect and dest rect.  It now includes code that
// essentially tries to get the same pixels to the screen but
// through an gles shader instead.  So when uses_shader_ flag is true,
// (for the emulated machine's display, for example) gles is used.
// Otherwise, it's not (for the UI or overlays, for example).

#include "fbl.h"

#include <stdio.h>
#include <circle/bcmframebuffer.h>

#include "crt_pi_idx.h"
#include "crt_pi_rgb.h"

#ifndef ALIGN_UP
#define ALIGN_UP(x,y)  ((x + (y)-1) & ~((y)-1))
#endif

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

static const char sNoInt[] = "scaling_kernel 0 0 0 0 0 0 0 0 1 1 1 1 255 255 255 255 255 255 255 255 1 1 1 1 0 0 0 0 0 0 0 0   1";

static char config_scaling_kernel[1024];

bool FrameBufferLayer::initialized_ = false;
DISPMANX_DISPLAY_HANDLE_T FrameBufferLayer::dispman_display_;
EGLDisplay FrameBufferLayer::egl_display_;
EGLContext FrameBufferLayer::egl_context_;
static char* fshader_txt_;
static char* vshader_txt_;

// For testing.
//#define LOAD_SHADER_FROM_FILE

#ifdef LOAD_SHADER_FROM_FILE
static char *file_shader_txt_;
#endif

/*
static void check(const char* msg) {
	int g = glGetError();
	if (g != 0) {
		FILE *fp = fopen("errors.txt","w");
		fprintf (fp,"%s %d\n",msg, g);
		fclose(fp);
		assert(false);
	}
}
*/

FrameBufferLayer::FrameBufferLayer() :
		pixels_(nullptr), dispman_element_(0),egl_config_(nullptr),egl_surface_(nullptr),
        fb_width_(0), fb_height_(0), fb_pitch_(0), layer_(0), transparency_(false),
        hstretch_(1.6), vstretch_(1.0), hintstr_(0), vintstr_(0),
        use_hintstr_(0), use_vintstr_(0),
        valign_(0), vpadding_(0), halign_(0), hpadding_(0),
        h_center_offset_(0), v_center_offset_(0),
        rnum_(0), leftPadding_(0), rightPadding_(0), topPadding_(0),
        bottomPadding_(0),
        display_width_(0), display_height_(0),
        src_x_(0), src_y_(0), src_w_(0), src_h_(0),
        dst_x_(0), dst_y_(0), dst_w_(0), dst_h_(0),
        showing_(false), allocated_(false),
        mode_(VC_IMAGE_8BPP), bytes_per_pixel_(1), uses_shader_(false),
        shader_init_(false),
        vshader_(-1), fshader_(-1), shader_program_(-1),
        vbo_(-1),
        attr_vertex_(-1), attr_texcoord_(-1),
        texture_sampler_(-1), palette_sampler_(-1),
        tex_(-1), pal_(-1), mvp_(0),
        input_size_(0), output_size_(0), texture_size_(0), texel_size_(0),
        need_cpu_crop_(true), cropped_pixels_(0),
        curvature_(false) {
  alpha_.flags = DISPMANX_FLAGS_ALPHA_FROM_SOURCE;
  alpha_.opacity = 255;
  alpha_.mask = 0;

  memcpy (pal_565_, pal_565, sizeof(pal_565));
  memcpy (pal_argb_, pal_argb, sizeof(pal_argb));
}

FrameBufferLayer::~FrameBufferLayer() {
  if (showing_) {
    Hide();
  }
  if (allocated_) {
    Free();
  }
}

void FrameBufferLayer::OGLInit() {
  EGLBoolean result;

  egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  assert(egl_display_ != EGL_NO_DISPLAY);
  result = eglInitialize(egl_display_, NULL, NULL);
  assert(EGL_FALSE != result);
  result = eglBindAPI(EGL_OPENGL_ES_API);
  assert(EGL_FALSE != result);
}

void FrameBufferLayer::CreateTexture() {
  // These values make sense but I'm not sure what the difference between
  // inputSize and textureSize is.  Perhaps it has something to do with
  // the fact these shaders are meant to be chained together over different
  // passes and the next shader needs to know what the output size of the
  // previous was?
  float inputSize[2] = { (float) src_w_, (float)src_h_ };
  float outputSize[2] = { (float) dst_w_, (float)dst_h_ };

  int tx = need_cpu_crop_ ? src_w_ : fb_pitch_ / bytes_per_pixel_;
  int ty = need_cpu_crop_ ? src_h_ : fb_height_;
  float textureSize[2] = { (float) tx, (float) ty };
  float texelSize[2] = { 1.0f / (float) tx, 1.0f / (float) ty };

  glUniform2fv(input_size_, 1, inputSize);
  glUniform2fv(output_size_, 1, outputSize);
  glUniform2fv(texture_size_, 1, textureSize);
  glUniform2fv(texel_size_, 1, texelSize);

  glBindTexture(GL_TEXTURE_2D,tex_);
  if (mode_ == VC_IMAGE_8BPP) {
     glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,
        need_cpu_crop_ ? src_w_ : fb_pitch_ / bytes_per_pixel_,
        need_cpu_crop_ ? src_h_ : fb_height_,
        0,GL_LUMINANCE,GL_UNSIGNED_BYTE, 0);
  } else {
     glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,
        need_cpu_crop_ ? src_w_ : fb_pitch_ / bytes_per_pixel_,
        need_cpu_crop_ ? src_h_ : fb_height_,
        0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5, 0);
  }

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void FrameBufferLayer::ReCreateTexture() {
  if (shader_init_) {
	  CreateTexture();
  }
}

void FrameBufferLayer::ConcatShaderDefines(char *dst) {
  char scratch[64];
  if (curvature_) {
	  strcat(dst,"#define CURVATURE\n");
  }

  sprintf (scratch, "#define CURVATURE_X %f\n", curvature_x_);
  strcat(dst, scratch);

  sprintf (scratch, "#define CURVATURE_Y %f\n", curvature_y_);
  strcat(dst, scratch);

  sprintf (scratch, "#define MASK_TYPE %d\n", mask_);
  strcat(dst, scratch);

  sprintf (scratch, "#define MASK_BRIGHTNESS %f\n", mask_brightness_);
  strcat(dst, scratch);

  if (gamma_) {
	  strcat (dst, "#define GAMMA\n");
	  if (fake_gamma_) {
		  strcat (dst, "#define FAKE_GAMMA\n");
	  }
  }

  if (scanlines_) {
	  strcat (dst, "#define SCANLINES\n");
  }

  if (multisample_) {
	  strcat (dst, "#define MULTISAMPLE\n");
  }

  sprintf (scratch, "#define SCANLINE_WEIGHT %f\n", scanline_weight_);
  strcat(dst, scratch);

  sprintf (scratch, "#define SCANLINE_GAP_BRIGHTNESS %f\n", scanline_gap_brightness_);
  strcat(dst, scratch);

  sprintf (scratch, "#define BLOOM_FACTOR %f\n", bloom_factor_);
  strcat(dst, scratch);

  sprintf (scratch, "#define INPUT_GAMMA %f\n", input_gamma_);
  strcat(dst, scratch);

  sprintf (scratch, "#define OUTPUT_GAMMA %f\n", output_gamma_);
  strcat(dst, scratch);

  if (sharper_) {
	  strcat (dst, "#define SHARPER\n");
  }
  if (bilinear_interpolation_) {
	  strcat (dst, "#define BILINEAR_INTERPOLATION\n");
  }
}

void FrameBufferLayer::ShaderInit() {

	// Used to for size to reserve enough space at
	// beginning of shader for dynamic defines.
	static const char* header_template =
           "#define FRAGMENT                    "
           "#define CURVATURE                   "
           "#define CURVATURE_X 0.10            "
           "#define CURVATURE_Y 0.25            "
           "#define MASK 1                      "
           "#define MASK_BRIGHTNESS 0.70        "
           "#define GAMMA                       "
           "#define FAKE_GAMMA                  "
           "#define SCANLINES                   "
           "#define MULTISAMPLE                 "
           "#define SCANLINE_WEIGHT 6.0         "
           "#define SCANLINE_GAP_BRIGHTNESS 0.12"
           "#define BLOOM_FACTOR 1.5            "
           "#define INPUT_GAMMA 2.4             "
           "#define OUTPUT_GAMMA 2.2            "
           "#define SHARPER                     "
           "#define BILINEAR_INTERPOLATION      ";

  // orthographic projection matrix
  static const GLfloat mvp_ortho[16] = { 2.0f,  0.0f,  0.0f,  0.0f,
                                         0.0f,  2.0f,  0.0f,  0.0f,
                                         0.0f,  0.0f, -1.0f,  0.0f,
                                         -1.0f, -1.0f,  0.0f,  1.0f };

  if (shader_init_) {
      return;
  }

  const char *shader_txt;
  int len;

#ifdef LOAD_SHADER_FROM_FILE
  if (!file_shader_txt_) {
     FILE *f;
     if (mode_ == VC_IMAGE_8BPP) {
        // Use indexed texture version
        f = fopen("crt-pi-idx.gls", "r");
     } else {
        // Use rgb texture version
        f = fopen("crt-pi-rgb.gls", "r");
     }
     fseek(f, 0, SEEK_END);
     len = ftell(f);
     fseek(f, 0, SEEK_SET);
     // Never freed.
     file_shader_txt_ = (char*) malloc(len + 1);
     fread(file_shader_txt_, 1, len, f);
     fclose(f);
     file_shader_txt_[len] = 0;
  }
  shader_txt = file_shader_txt_;
#else
  // Use statically linked shader txt.
  if (mode_ == VC_IMAGE_8BPP) {
     shader_txt = idx_shader;
  } else {
     shader_txt = rgb_shader;
  }
#endif

  len = strlen(shader_txt);
  if (vshader_txt_) {
     free(vshader_txt_);
  }
  char vheader[] = "#define VERTEX\n";
  vshader_txt_ = (char*) malloc(len + 1 +
                                strlen(vheader) +
                                strlen(header_template));
  vshader_txt_[0] = '\0';
  strcpy(vshader_txt_, vheader);
  ConcatShaderDefines(vshader_txt_);
  strcat(vshader_txt_, shader_txt);

  if (fshader_txt_) {
     free(fshader_txt_);
  }
  char fheader[] = "#define FRAGMENT\n";
  fshader_txt_ = (char*) malloc(len + 1 +
                                strlen(fheader) +
                                strlen(header_template));
  fshader_txt_[0] = '\0';
  strcpy(fshader_txt_, fheader);
  ConcatShaderDefines(fshader_txt_);
  strcat(fshader_txt_, shader_txt);

  const GLchar *vshader_source = (const GLchar*) vshader_txt_;
  vshader_ = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader_, 1, &vshader_source, 0);
  glCompileShader(vshader_);
  //check("glCompileShader");

//  char log[1024];
//  strcpy(log,"");
//  glGetShaderInfoLog(vshader_,sizeof log,NULL,log);
//  FILE *fp = fopen("shader1.log","w");
//  fprintf(fp,"%s\n",log);
//  fclose(fp);

  const GLchar *fshader_source = (const GLchar*) fshader_txt_;
  fshader_ = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader_, 1, &fshader_source, 0);
  glCompileShader(fshader_);
  //check("glCompileShader2");

//  glGetShaderInfoLog(fshader_,sizeof log,NULL,log);
//  fp = fopen("shader2.log","w");
//  fprintf(fp,"%s\n",log);
//  fclose(fp);

  shader_program_ = glCreateProgram();
  glAttachShader(shader_program_, vshader_);
  glAttachShader(shader_program_, fshader_);
  glLinkProgram(shader_program_);
  //check("linkProgram");

//  GLint status;
//  glGetProgramiv (shader_program_, GL_LINK_STATUS, &status);
//  if (status != GL_TRUE) {
//	  glGetProgramInfoLog(shader_program_,sizeof log,NULL,log);
//	  fp = fopen("program.log","w");
//	  fprintf(fp,"%s\n",log);
//	  fclose(fp);
//  }

  glUseProgram (shader_program_);

  attr_vertex_ = glGetAttribLocation(shader_program_, "VertexCoord");
  attr_texcoord_ = glGetAttribLocation(shader_program_, "TexCoord");
  texture_sampler_ = glGetUniformLocation(shader_program_, "Texture");
  if (mode_ == VC_IMAGE_8BPP) {
     palette_sampler_ = glGetUniformLocation(shader_program_, "Palette");
  }
  input_size_ = glGetUniformLocation (shader_program_, "InputSize");
  output_size_= glGetUniformLocation (shader_program_, "OutputSize");
  texture_size_ = glGetUniformLocation (shader_program_, "TextureSize");
  texel_size_ = glGetUniformLocation (shader_program_, "TexelSize");

  mvp_ = glGetUniformLocation (shader_program_, "MVPMatrix");
  if (mvp_ > -1) {
	  glUniformMatrix4fv(mvp_, 1, GL_FALSE, mvp_ortho);
  }

  // This texture is the indexed bitmap data so we use LUMINANCE which
  // will show up as coordinate x in the shader. Then we use the palette
  // 256x1 texture (below) to lookup the color.
  glGenTextures(1, &tex_);
  CreateTexture();

  if (mode_ == VC_IMAGE_8BPP) {
     glGenTextures(1, &pal_);
     glBindTexture(GL_TEXTURE_2D,pal_);
     glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,256,1,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5, pal_565_);
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  // Use full screen viewport.
  glViewport (0, 0, display_width_, display_height_);

  // For our vertex and texture coordinates.
  glGenBuffers(1, &vbo_);

  shader_init_ = true;
}

void FrameBufferLayer::ShaderDestroy() {
  if (shader_init_) {
    glDeleteBuffers(1, &vbo_);
    glDeleteTextures(1, &tex_);
    glUseProgram (0);
    glDetachShader(shader_program_, vshader_);
    glDetachShader(shader_program_, fshader_);
    glDeleteProgram(shader_program_);
    glDeleteShader(vshader_);
    glDeleteShader(fshader_);
    shader_init_ = false;
  }
}

void FrameBufferLayer::ShaderUpdate() {
  // Update any shader inputs that may need to change because of
  // new values coming in after a Show().

  // Futz with the vertex coordinates so we draw into a quad
  // that matches our calculated destination rect.  These
  // need to be updates every time they can change from a
  // call to Show()

  // Bottom left
  tex_coords_[0] = (float)dst_x_ / (float)display_width_;
  tex_coords_[1] = (float)dst_y_ / (float)display_height_;

  // Bottom right
  tex_coords_[2] = (float)(dst_x_+dst_w_) / (float)display_width_;
  tex_coords_[3] = (float)dst_y_ / (float)display_height_;

  // Top left
  tex_coords_[4] = (float)dst_x_ / (float)display_width_;
  tex_coords_[5] = (float)(dst_y_ + dst_h_) / (float)display_height_;

  // Top right
  tex_coords_[6] = (float)(dst_x_ + dst_w_) / (float)display_width_;
  tex_coords_[7] = (float)(dst_y_ + dst_h_) / (float)display_height_;

  if (!need_cpu_crop_) {
    // Now futz with the texture coordinates (for triangle strip)
    // to crop out only the portion of the frame buffer we want to see.
    // When curvature is requested, we can't do this because the
    // shader seems to want the texture to be only visible pixels
    // for its curvature calculation.

    // Top left
    tex_coords_[8] = (float)src_x_ / (float)(fb_pitch_ / bytes_per_pixel_);
    tex_coords_[9] = (float)(src_y_ + src_h_) / (float)fb_height_;

    // Top right
    tex_coords_[10] = (float)(src_x_ + src_w_) / (float)(fb_pitch_ / bytes_per_pixel_);
    tex_coords_[11] = (float)(src_y_ + src_h_) / (float)fb_height_;

    // Bottom left
    tex_coords_[12] = (float)src_x_ / (float)(fb_pitch_ / bytes_per_pixel_);
    tex_coords_[13] = (float)src_y_ / (float)fb_height_;

    // Bottom right
    tex_coords_[14] = (float)(src_x_+src_w_) / (float)(fb_pitch_ / bytes_per_pixel_);
    tex_coords_[15] = (float)src_y_ / (float)fb_height_;
  } else {
    tex_coords_[8] = 0.0f;
    tex_coords_[9] = 1.0f;
    tex_coords_[10] = 1.0f;
    tex_coords_[11] = 1.0f;
    tex_coords_[12] = 0.0f;
    tex_coords_[13] = 0.0f;
    tex_coords_[14] = 1.0f;
    tex_coords_[15] = 0.0f;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 16, tex_coords_, GL_STATIC_DRAW);

  glEnableVertexAttribArray(attr_texcoord_);
  glVertexAttribPointer(attr_texcoord_, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof (float) * 8));

  glEnableVertexAttribArray(attr_vertex_);
  glVertexAttribPointer(attr_vertex_, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glUseProgram (0);
}

void FrameBufferLayer::SetUsesShader(bool enabled) {
  assert(!allocated_);
  uses_shader_ = enabled;
}

void FrameBufferLayer::SetShaderParams(
		bool curvature,
		float curvature_x,
		float curvature_y,
		int mask,
		float mask_brightness,
		bool gamma,
		bool fake_gamma,
		bool scanlines,
		bool multisample,
		float scanline_weight,
		float scanline_gap_brightness,
		float bloom_factor,
		float input_gamma,
		float output_gamma,
		bool sharper,
                bool bilinear_interpolation) {
  if (uses_shader_) {
     ShaderDestroy();
  }
  curvature_ = curvature;
  need_cpu_crop_ = curvature;
  curvature_x_ = curvature_x;
  curvature_y_ = curvature_y;
  mask_ = mask;
  mask_brightness_ = mask_brightness;
  gamma_ = gamma;
  fake_gamma_ = fake_gamma;
  scanlines_ = scanlines;
  multisample_ = multisample;
  scanline_weight_ = scanline_weight;
  scanline_gap_brightness_ = scanline_gap_brightness;
  bloom_factor_ = bloom_factor;
  input_gamma_ = input_gamma;
  output_gamma_ = output_gamma;
  sharper_ = sharper;
  bilinear_interpolation_ = bilinear_interpolation;
  Hide();
}

// static
void FrameBufferLayer::Initialize() {
  if (initialized_)
     return;

  bcm_host_init();
  OGLInit();

  dispman_display_ = vc_dispmanx_display_open(0);

  bcm_get_sclker(config_scaling_kernel, sizeof(config_scaling_kernel));
  // We have to remove the '=' or else what we send back
  // won't work.
  for (unsigned int i=0;i<strlen(config_scaling_kernel);i++) {
    if (config_scaling_kernel[i] == '=') {
       config_scaling_kernel[i] = ' ';
       break;
    }
  }

  initialized_ = true;
}

int FrameBufferLayer::Allocate(int pixelmode, uint8_t **pixels,
                               int width, int height, int *pitch) {
  int ret;
  DISPMANX_MODEINFO_T dispman_info;
  uint32_t vc_image_ptr;

  assert(!allocated_);

  allocated_ = true;

  switch (pixelmode) {
     case 0:
        mode_ = VC_IMAGE_8BPP;
        bytes_per_pixel_ = 1;
        break;
     case 1:
        mode_ = VC_IMAGE_RGB565;
        bytes_per_pixel_ = 2;
        break;
     default:
        mode_ = VC_IMAGE_8BPP;
        bytes_per_pixel_ = 1;
        break;
  }

  // pitch is in bytes
  if (pitch) {
     *pitch = fb_pitch_ = ALIGN_UP(width * bytes_per_pixel_, 32);
  }

  fb_width_ = width;
  fb_height_ = height;

  ret = vc_dispmanx_display_get_info(dispman_display_, &dispman_info);
  assert(ret == 0);

  display_width_ = dispman_info.width;
  display_height_ = dispman_info.height;

  if (pixels) {
     pixels_ = (uint8_t*) malloc(fb_pitch_ * height);
     cropped_pixels_ = (uint8_t*) malloc(fb_pitch_ * fb_height_);
     *pixels = pixels_;
  }

  // Allocate the VC resources along with the frame buffer

  dispman_resource_[0] = vc_dispmanx_resource_create(mode_,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  dispman_resource_[1] = vc_dispmanx_resource_create(mode_,
                                                     width,
                                                     height,
                                                     &vc_image_ptr );
  assert(dispman_resource_[0]);
  assert(dispman_resource_[1]);

  vc_dispmanx_rect_set(&copy_dst_rect_, 0, 0, width, height);

  if (pixels) {
     // Don't clobber these on realloc.
     dst_x_ = 0;
     dst_y_ = 0;
     dst_w_ = width;
     dst_h_ = height;

     src_x_ = 0;
     src_y_ = 0;
     src_w_ = width;
     src_h_ = height;
  }

  EGLBoolean result;
  EGLint num_config;

  static const EGLint context_attributes[] =
  {
	 EGL_CONTEXT_CLIENT_VERSION, 2,
	 EGL_NONE
  };

  static const EGLint attribute_list[] =
  {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_NONE
  };

  if (uses_shader_) {
     result = eglChooseConfig(egl_display_, attribute_list,
                               &egl_config_, 1, &num_config);
     assert(EGL_FALSE != result);
     egl_context_ = eglCreateContext(egl_display_, egl_config_,
                                        EGL_NO_CONTEXT, context_attributes);
     assert(egl_context_ != EGL_NO_CONTEXT);
  }

  return 0;
}

int FrameBufferLayer::ReAllocate(bool shader_enable) {
  assert(allocated_);

  if (uses_shader_ == shader_enable) {
     // No need to realloc if nothing changed;
     return 0;
  }

  // Free layer but keep pixels.
  FreeInternal(true);

  // Change the uses shader flag.
  SetUsesShader(shader_enable);

  int pixelmode = 0;
  if (mode_ == VC_IMAGE_RGB565) pixelmode = 1;

  // Reallocate with same params.
  return Allocate(pixelmode, nullptr, fb_width_, fb_height_, nullptr);
}

void FrameBufferLayer::Clear() {
  assert (allocated_);

  memset(pixels_, 0, fb_height_ * fb_pitch_);
}

// When keepPixels is true, don't clobber any dimensions or delete
// buffers. Only tear down gl and dispmanx resources.
void FrameBufferLayer::FreeInternal(bool keepPixels) {
  int ret;

  if (!allocated_) return;

  if (uses_shader_) {
     ShaderDestroy();
  }
 
  if (showing_) {
     Hide();
  }

  if (uses_shader_) {
     eglDestroyContext(egl_display_, egl_context_);
  }

  if (!keepPixels) {
     fb_width_ = 0;
     fb_height_ = 0;
     fb_pitch_ = 0;
     free(pixels_);
     free(cropped_pixels_);
  }

  ret = vc_dispmanx_resource_delete(dispman_resource_[0]);
  assert(ret == 0);
  ret = vc_dispmanx_resource_delete(dispman_resource_[1]);
  assert(ret == 0);

  allocated_ = false;
}

void FrameBufferLayer::Free() {
  FreeInternal(false);
}

void FrameBufferLayer::Show() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  if (showing_) return;

  int dst_w;
  int dst_h;
  assert (hstretch_ != 0);
  assert (vstretch_ != 0);

  int lpad_abs = display_width_ * leftPadding_;
  int rpad_abs = display_width_ * rightPadding_;
  int tpad_abs = display_height_ * topPadding_;
  int bpad_abs = display_height_ * bottomPadding_;

  int avail_width = display_width_ - lpad_abs - rpad_abs;
  int avail_height = display_height_ - tpad_abs - bpad_abs;

  if (hstretch_ < 0) {
     // Stretch horizontally to fill width * vstretch and then set height
     // based on hstretch.  This mode doesn't support integer stretch.
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


  dst_x_ = ox + lpad_abs;
  dst_y_ = oy + tpad_abs;
  dst_w_ = dst_w;
  dst_h_ = dst_h;

  if (uses_shader_) {
     // When we use opengl + shader the source rect needs to
     // be the dest rect because the shader ends up doing the
     // cropping/scaling. So we don't want any scaling done
     // by the dispman layer.
     vc_dispmanx_rect_set(&src_rect_,
                       dst_x_ << 16,
                       dst_y_ << 16,
                       dst_w_ << 16,
                       dst_h_ << 16);
  } else {
     // When we're using just dispmanx, we isolate and crop
     // the region in the layer we want to scale up to the
     // dest rect.
     vc_dispmanx_rect_set(&src_rect_,
                       src_x_ << 16,
                       src_y_ << 16,
                       src_w_ << 16,
                       src_h_ << 16);
  }

  vc_dispmanx_rect_set(&scale_dst_rect_,
                       dst_x_,
                       dst_y_,
                       dst_w_,
                       dst_h_);

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

  if (uses_shader_) {
    egl_native_window_.element = dispman_element_;
    egl_native_window_.width = display_width_;
    egl_native_window_.height = display_height_;
    egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config_, &egl_native_window_, NULL );
    assert(egl_surface_ != EGL_NO_SURFACE);

    EGLBoolean result;
    result = eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
    assert(EGL_FALSE != result);
    //check("eglMakeCurrent");

    ShaderInit();
    ShaderUpdate();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapInterval(egl_display_, 0);
    eglSwapBuffers(egl_display_, egl_surface_);
  }

  if (mode_ == VC_IMAGE_8BPP) {
    UpdatePalette();
  }

  FrameReady(0);
  showing_ = true;
  SwapResources(false, this, nullptr);
}

void FrameBufferLayer::Hide() {
  int ret;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  if (!showing_) return;

  if (uses_shader_) {
    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context_);
    eglDestroySurface(egl_display_, egl_surface_);
  }

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
  if (!uses_shader_) {
      vc_dispmanx_resource_write_data(dispman_resource_[rnum],
                                      mode_,
                                      fb_pitch_,
                                      pixels_,
                                      &copy_dst_rect_);
  } else {
      RenderGL();
  }
}

// Private function to change the source of this frame buffer's
// element to the off screen resource and toggle the resource
// index in preparation for the off screen data to be shown.
void FrameBufferLayer::Swap(DISPMANX_UPDATE_HANDLE_T& dispman_update) {
  if (uses_shader_ || !showing_)
     return;

  rnum_ = 1 - rnum_;

  vc_dispmanx_element_change_source(dispman_update,
                                    dispman_element_,
                                    dispman_resource_[rnum_]);
}

void FrameBufferLayer::SwapGL(bool sync) {
  eglSwapInterval(egl_display_, sync ? 1 : 0);
  eglSwapBuffers(egl_display_, egl_surface_);
}

void FrameBufferLayer::RenderGL() {
    // Our pixels_ framebuffer includes a lot of black border area around
    // the visible pixels we want to see. When shader curvature is needed,
    // we need to provide a texture with only the pixels we actually want
    // to see, otherwise the curvature gets applied incorrectly to the
    // larger area. So we crop on the CPU rather than by texture coords.
    if (need_cpu_crop_) {
       int wid = ALIGN_UP(src_w_ * bytes_per_pixel_, 4);
       uint8_t *src_col = pixels_ + src_x_ * bytes_per_pixel_;
       for (int yy=src_y_; yy < src_y_ + src_h_;yy++) {
          memcpy (cropped_pixels_ + (yy - src_y_) * wid,
                  src_col + yy * fb_pitch_, wid);
       }
    }

    glBindTexture(GL_TEXTURE_2D,tex_);

    if (mode_ == VC_IMAGE_8BPP) {
        glTexSubImage2D(GL_TEXTURE_2D,
        0,
        0,
        0,
        need_cpu_crop_ ? src_w_ : fb_pitch_ / bytes_per_pixel_,
        need_cpu_crop_ ? src_h_ : fb_height_,
        GL_LUMINANCE,
        GL_UNSIGNED_BYTE,
        need_cpu_crop_ ? cropped_pixels_ : pixels_);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D,
        0,
        0,
        0,
        need_cpu_crop_ ? src_w_ : fb_pitch_ / bytes_per_pixel_,
        need_cpu_crop_ ? src_h_ : fb_height_,
        GL_RGB,
        GL_UNSIGNED_SHORT_5_6_5,
        need_cpu_crop_ ? cropped_pixels_ : pixels_);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glUseProgram (shader_program_);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glUniform1i(texture_sampler_, 0);

    if (mode_ == VC_IMAGE_8BPP) {
       glActiveTexture(GL_TEXTURE0 + 1);
       glBindTexture(GL_TEXTURE_2D, pal_);
       glUniform1i(palette_sampler_, 1);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// Static
void FrameBufferLayer::SwapResources(bool sync,
                                     FrameBufferLayer* fb1,
                                     FrameBufferLayer* fb2) {

  // We need to know whether the dispmanx code below
  // is actually going to cause a sync. It turns out if we
  // don't actually change resources on either fb1 or fb2,
  // the start/submit doesn't perform the sync.  So we
  // predict what will happen based on the same conditions
  // in Swap() above. If sync is requested but the code below
  // won't sync, let SwapGL take care of it.
  bool will_sync = !fb1->UsesShader() && fb1->Showing() &&
                         (!fb2 || (fb2 && !fb2->UsesShader() && fb2->Showing()));
  fb1->SwapGL(sync && !will_sync);

  if (sync) {
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    dispman_update = vc_dispmanx_update_start(0);
    fb1->Swap(dispman_update);
    if (fb2) {
       fb2->Swap(dispman_update);
    }
    vc_dispmanx_update_submit_sync(dispman_update);
  }
}

void FrameBufferLayer::SetPalette(uint8_t index, uint16_t rgb565) {
  assert(!transparency_);
  assert (mode_ == VC_IMAGE_8BPP);
  pal_565_[index] = rgb565;
}

void FrameBufferLayer::SetPalette(uint8_t index, uint32_t argb) {
  assert(transparency_);
  assert (mode_ == VC_IMAGE_8BPP);
  pal_argb_[index] = argb;
}

void FrameBufferLayer::UpdatePalette() {
  if (!allocated_) return;
  assert (mode_ == VC_IMAGE_8BPP);

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

  if (uses_shader_ & shader_init_) {
     if (transparency_) {
	  // Not supported yet.
	  assert(false);
     } else {
	  glBindTexture(GL_TEXTURE_2D,pal_);
	  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pal_565_);
	  RenderGL();
          SwapResources(false, this, nullptr);
     }
  }
}

void FrameBufferLayer::SetLayer(int layer) {
  layer_ = layer;
}

int FrameBufferLayer::GetLayer() {
  return layer_;
}

bool FrameBufferLayer::UsesShader() {
	return uses_shader_;
}

bool FrameBufferLayer::Showing() {
	return showing_;
}

void FrameBufferLayer::SetTransparency(bool transparency) {
  assert (mode_ == VC_IMAGE_8BPP);
  transparency_ = transparency;
}

void FrameBufferLayer::SetSrcRect(int x, int y, int w, int h) {
  bool has_changed = x != src_x_ || y != src_y_ || w != src_w_ || h != src_h_;
  src_x_ = x;
  src_y_ = y;
  src_w_ = w;
  src_h_ = h;

  if (has_changed && need_cpu_crop_) {
      // When using cpu crop, we have to resize our texture.
      ReCreateTexture();
  }
}

// Set horizontal/vertical multipliers
void FrameBufferLayer::SetStretch(double hstretch, double vstretch, int hintstr, int vintstr, int use_hintstr, int use_vintstr) {
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

void FrameBufferLayer::GetDimensions(int *display_w, int *display_h,
                                     int *fb_w, int *fb_h,
                                     int *src_w, int *src_h,
                                     int *dst_w, int *dst_h) {
  *display_w = display_width_;
  *display_h = display_height_;
  *fb_w = fb_width_;
  *fb_h = fb_height_;
  *src_w = src_w_;
  *src_h = src_h_;
  *dst_w = dst_w_;
  *dst_h = dst_h_;
}

void FrameBufferLayer::SetInterpolation(int enable) {
  if (enable) {
     bcm_set_sclker(config_scaling_kernel);
  } else {
     bcm_set_sclker(sNoInt);
  }
}
