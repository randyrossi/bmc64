
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "display.hpp"
#include "ted.hpp"

#include <cmath>

static const float phaseShiftTable[18] = {
     0.0f,   33.0f,  327.0f,    0.0f,   33.0f,  327.0f,    0.0f,    0.0f,
     0.0f,   33.0f,   33.0f,    0.0f,    0.0f,  327.0f,  327.0f,    0.0f,
     0.0f,    0.0f
};

static const float yScaleTable[18] = {
     1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    0.0f,
     1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,
     1.0f,    1.0f
};

static const float uScaleTable[18] = {
     1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    0.0f,    0.0f,
     1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,    1.0f,
     1.0f,    1.0f
};

static const float vScaleTable[18] = {
     1.0f,    1.0f,    1.0f,   -1.0f,   -1.0f,   -1.0f,    0.0f,    0.0f,
     1.0f,    1.0f,   -1.0f,   -1.0f,    1.0f,    1.0f,   -1.0f,   -1.0f,
     1.0f,   -1.0f
};

static const bool ntscModeTable[18] = {
  false, false,  true, false, false,  true, false, false,
  false, false, false, false,  true,  true,  true,  true,
   true,  true
};

static const unsigned short colormapIndexTable[32] = {
  0x0000, 0x0200, 0x0300, 0x0500, 0x0300, 0x0500, 0x0000, 0x0200,
  0x0800, 0x0D00, 0x0B00, 0x0E00, 0x0B00, 0x0E00, 0x0800, 0x0D00,
  0x0100, 0x1000, 0x0100, 0x1000, 0x0400, 0x1100, 0x0400, 0x1100,
  0x0900, 0x0C00, 0x0900, 0x0C00, 0x0A00, 0x0F00, 0x0A00, 0x0F00
};

namespace Plus4Emu {

  void VideoDisplay::DisplayParameters::defaultIndexToYUVFunc(
      uint8_t color, bool isNTSC, float& y, float& u, float& v)
  {
    (void) isNTSC;
    y = float(color) / 255.0f;
    u = 0.0f;
    v = 0.0f;
  }

  void VideoDisplay::DisplayParameters::copyDisplayParameters(
      const DisplayParameters& src)
  {
    displayQuality = (src.displayQuality > 0 ?
                      (src.displayQuality < 3 ? src.displayQuality : 3)
                      : 0);
    bufferingMode = (src.bufferingMode > 0 ?
                     (src.bufferingMode < 2 ? src.bufferingMode : 2) : 0);
    ntscMode = src.ntscMode;
    if (src.indexToYUVFunc)
      indexToYUVFunc = src.indexToYUVFunc;
    else
      indexToYUVFunc = &defaultIndexToYUVFunc;
    brightness = (src.brightness > -0.5f ?
                  (src.brightness < 0.5f ? src.brightness : 0.5f)
                  : -0.5f);
    contrast = (src.contrast > 0.5f ?
                (src.contrast < 2.0f ? src.contrast : 2.0f)
                : 0.5f);
    gamma = (src.gamma > 0.25f ?
             (src.gamma < 4.0f ? src.gamma : 4.0f)
             : 0.25f);
    hueShift = (src.hueShift > -180.0f ?
                (src.hueShift < 180.0f ? src.hueShift : 180.0f)
                : -180.0f);
    saturation = (src.saturation > 0.0f ?
                  (src.saturation < 2.0f ? src.saturation : 2.0f)
                  : 0.0f);
    redBrightness = (src.redBrightness > -0.5f ?
                     (src.redBrightness < 0.5f ? src.redBrightness : 0.5f)
                     : -0.5f);
    redContrast = (src.redContrast > 0.5f ?
                   (src.redContrast < 2.0f ? src.redContrast : 2.0f)
                   : 0.5f);
    redGamma = (src.redGamma > 0.25f ?
                (src.redGamma < 4.0f ? src.redGamma : 4.0f)
                : 0.25f);
    greenBrightness = (src.greenBrightness > -0.5f ?
                       (src.greenBrightness < 0.5f ? src.greenBrightness : 0.5f)
                       : -0.5f);
    greenContrast = (src.greenContrast > 0.5f ?
                     (src.greenContrast < 2.0f ? src.greenContrast : 2.0f)
                     : 0.5f);
    greenGamma = (src.greenGamma > 0.25f ?
                  (src.greenGamma < 4.0f ? src.greenGamma : 4.0f)
                  : 0.25f);
    blueBrightness = (src.blueBrightness > -0.5f ?
                      (src.blueBrightness < 0.5f ? src.blueBrightness : 0.5f)
                      : -0.5f);
    blueContrast = (src.blueContrast > 0.5f ?
                    (src.blueContrast < 2.0f ? src.blueContrast : 2.0f)
                    : 0.5f);
    blueGamma = (src.blueGamma > 0.25f ?
                 (src.blueGamma < 4.0f ? src.blueGamma : 4.0f)
                 : 0.25f);
    palPhaseError = (src.palPhaseError > -30.0f ?
                     (src.palPhaseError < 30.0f ? src.palPhaseError : 30.0f)
                     : -30.0f);
    lineShade = (src.lineShade > 0.0f ?
                 (src.lineShade < 1.0f ? src.lineShade : 1.0f) : 0.0f);
    blendScale = (src.blendScale > 0.5f ?
                  (src.blendScale < 2.0f ? src.blendScale : 2.0f) : 0.5f);
    motionBlur = (src.motionBlur > 0.0f ?
                  (src.motionBlur < 0.95f ? src.motionBlur : 0.95f) : 0.0f);
    pixelAspectRatio = (src.pixelAspectRatio > 0.5f ?
                        (src.pixelAspectRatio < 2.0f ?
                         src.pixelAspectRatio : 2.0f) : 0.5f);
    shaderSourcePAL = src.shaderSourcePAL;
    shaderSourceNTSC = src.shaderSourceNTSC;
  }

  VideoDisplay::DisplayParameters::DisplayParameters()
    : displayQuality(2),
      bufferingMode(0),
      ntscMode(false),
      indexToYUVFunc(&defaultIndexToYUVFunc),
      brightness(0.0f), contrast(1.0f), gamma(1.0f),
      hueShift(0.0f), saturation(1.0f),
      redBrightness(0.0f), redContrast(1.0f), redGamma(1.0f),
      greenBrightness(0.0f), greenContrast(1.0f), greenGamma(1.0f),
      blueBrightness(0.0f), blueContrast(1.0f), blueGamma(1.0f),
      palPhaseError(8.0f),
      lineShade(0.8f),
      blendScale(1.0f),
      motionBlur(0.25f),
      pixelAspectRatio(1.0f)
  {
  }

  VideoDisplay::DisplayParameters::DisplayParameters(
      const DisplayParameters& dp)
  {
    copyDisplayParameters(dp);
  }

  VideoDisplay::DisplayParameters& VideoDisplay::DisplayParameters::operator=(
      const DisplayParameters& dp)
  {
    copyDisplayParameters(dp);
    return (*this);
  }

  void VideoDisplay::DisplayParameters::yuvToRGBWithColorCorrection(
      float& red, float& green, float& blue, float y, float u, float v) const
  {
    float   hueShiftU = float(std::cos(double(hueShift) * 0.01745329252));
    float   hueShiftV = float(std::sin(double(hueShift) * 0.01745329252));
    float   tmpU = ((u * hueShiftU) - (v * hueShiftV)) * saturation;
    float   tmpV = ((v * hueShiftU) + (u * hueShiftV)) * saturation;
    // R = (V / 0.877) + Y
    // B = (U / 0.492) + Y
    // G = (Y - ((R * 0.299) + (B * 0.114))) / 0.587
    float   r = y + (tmpV * float(1.0 / 0.877));
    float   g = y + (tmpU * float(-0.114 / (0.492 * 0.587)))
                  + (tmpV * float(-0.299 / (0.877 * 0.587)));
    float   b = y + (tmpU * float(1.0 / 0.492));
    r = (r - 0.5f) * (contrast * redContrast) + 0.5f;
    g = (g - 0.5f) * (contrast * greenContrast) + 0.5f;
    b = (b - 0.5f) * (contrast * blueContrast) + 0.5f;
    r = r + (brightness + redBrightness);
    g = g + (brightness + greenBrightness);
    b = b + (brightness + blueBrightness);
    if (std::fabs(double((gamma * redGamma) - 1.0f)) > 0.01) {
      r = (r > 0.0f ? r : 0.0f);
      r = float(std::pow(double(r), double(1.0f / (gamma * redGamma))));
    }
    if (std::fabs(double((gamma * greenGamma) - 1.0f)) > 0.01) {
      g = (g > 0.0f ? g : 0.0f);
      g = float(std::pow(double(g), double(1.0f / (gamma * greenGamma))));
    }
    if (std::fabs(double((gamma * blueGamma) - 1.0f)) > 0.01) {
      b = (b > 0.0f ? b : 0.0f);
      b = float(std::pow(double(b), double(1.0f / (gamma * blueGamma))));
    }
    red = r;
    green = g;
    blue = b;
  }

  VideoDisplay::~VideoDisplay()
  {
  }

  void VideoDisplay::limitFrameRate(bool isEnabled)
  {
    (void) isEnabled;
  }

  // --------------------------------------------------------------------------

  template <>
  uint8_t VideoDisplayColormap<uint8_t>::pixelConv(float y, float u, float v)
  {
    double  bestError = 1000000.0;
    int     l = 0;
    for (int i = 0; i < 9; i++) {
      float   y_ = 0.0f;
      float   u_ = 0.0f;
      float   v_ = 0.0f;
      getPlus4PaletteColor((i > 0 ? (((i - 1) << 4) | 1) : 0), y_, u_, v_);
      double  err = double(y_) - double(y);
      err = err * err;
      if (err < bestError) {
        l = i;
        bestError = err;
      }
    }
    // FIXME: this is slow
    bestError = 1000000.0;
    int     bestColor = 0;
    for (int c = 0; c < 256; c++) {
      if (!((l == 0 && (c & 0x0F) == 0) ||
            (l > 0 && (c & 0x0F) != 0 && (((c & 0x70) >> 4) + 1) == l))) {
        continue;
      }
      float   y_ = 0.0f;
      float   u_ = 0.0f;
      float   v_ = 0.0f;
      getPlus4PaletteColor(c, y_, u_, v_);
      double  yErr = double(y_) - double(y);
      double  uErr = double(u_) - double(u);
      double  vErr = double(v_) - double(v);
      double  err = (yErr * yErr) + (uErr * uErr) + (vErr * vErr);
      if (err < bestError) {
        bestColor = c;
        bestError = err;
        if (bestError < 0.000001)
          break;
      }
    }
    return uint8_t(bestColor);
  }

  template <>
  uint16_t VideoDisplayColormap<uint16_t>::pixelConv(float r, float g, float b)
  {
    int     ri = int(r *  992.0f + 16.0f);
    ri = (ri > 16 ? (ri < 1007 ? ri : 1007) : 16);
    int     gi = int(g * 2016.0f + 16.0f);
    gi = (gi > 16 ? (gi < 2031 ? gi : 2031) : 16);
    int     bi = int(b *  992.0f + 16.0f);
    bi = (bi > 16 ? (bi < 1007 ? bi : 1007) : 16);
    int     tmp = (16 - (gi & 31)) - ((16 - (ri & 31)) + (16 - (bi & 31)));
    if (tmp <= -16)
      gi = gi + 16;
    else if (tmp >= 16)
      gi = gi - 16;
    return uint16_t(((ri & 0x03E0) << 6) | (gi & 0x07E0) | (bi >> 5));
  }

  template <>
  uint32_t VideoDisplayColormap<uint32_t>::pixelConv(float r, float g, float b)
  {
    uint32_t  ri, gi, bi;
    ri = uint32_t(int(r >= 0.0f ? (r < 1.0f ? (r * 255.0f + 0.5f) : 255.0f)
                                  : 0.0f));
    gi = uint32_t(int(g >= 0.0f ? (g < 1.0f ? (g * 255.0f + 0.5f) : 255.0f)
                                  : 0.0f));
    bi = uint32_t(int(b >= 0.0f ? (b < 1.0f ? (b * 255.0f + 0.5f) : 255.0f)
                                  : 0.0f));
    return ((bi << 16) | (gi << 8) | ri);
  }

  template <typename T>
  VideoDisplayColormap<T>::VideoDisplayColormap()
  {
    colormapData = new T[256 * 18];
    try {
      colormapTable = new T*[256];
    }
    catch (...) {
      delete[] colormapData;
      throw;
    }
    for (size_t i = 0; i < 0x1200; i++)
      colormapData[i] = T(0);
    for (size_t i = 0; i < 256; i++) {
      T       *p = &(colormapData[0x0700]);
      if (!(i & 0xC0)) {
        if (!(i & 0x20)) {
          p = &(colormapData[0x0600]);
        }
        else {
          p = &(colormapData[colormapIndexTable[i & 0x1F]]);
        }
      }
      colormapTable[i] = p;
    }
  }

  template <typename T>
  VideoDisplayColormap<T>::~VideoDisplayColormap()
  {
    delete[] colormapData;
    delete[] colormapTable;
  }

  template <typename T>
  void VideoDisplayColormap<T>::setDisplayParameters(
      const VideoDisplay::DisplayParameters& displayParameters, bool yuvFormat)
  {
    float   baseColormap[768];
    bool    prvNTSCMode = !(ntscModeTable[0]);
    for (size_t i = 0; i < 0x1200; i++) {
      size_t  j = i & 0xFF;
      size_t  k = i >> 8;
      if (ntscModeTable[k] != prvNTSCMode) {
        prvNTSCMode = ntscModeTable[k];
        for (size_t l = 0; l < 256; l++) {
          float   y = float(int(l)) / 255.0f;
          float   u = 0.0f;
          float   v = 0.0f;
          if (displayParameters.indexToYUVFunc)
            displayParameters.indexToYUVFunc(uint8_t(l), prvNTSCMode, y, u, v);
          baseColormap[l * 3 + 0] = y;
          baseColormap[l * 3 + 1] = u;
          baseColormap[l * 3 + 2] = v;
        }
      }
      float   y = baseColormap[j * 3 + 0];
      float   uTmp = baseColormap[j * 3 + 1];
      float   vTmp = baseColormap[j * 3 + 2];
      if (i & 0x0800) {
        float   c = (uTmp * uTmp) + (vTmp * vTmp);
        if (i < 0x0C00) {
          uTmp = uTmp - 0.127f;         // add PAL burst (135 degrees)
          vTmp = vTmp + 0.127f;
        }
        else {
          uTmp = uTmp - 0.179f;         // add NTSC burst (180 degrees)
        }
        if (c > 0.005f) {
          uTmp *= 0.525f;
          vTmp *= 0.525f;
        }
      }
      float   phaseShift = phaseShiftTable[k] * 0.01745329f;
      float   re = float(std::cos(phaseShift));
      float   im = float(std::sin(phaseShift));
      float   u = uTmp * re - vTmp * im;
      float   v = uTmp * im + vTmp * re;
      u = u * uScaleTable[k];
      v = v * vScaleTable[k];
      if (sizeof(T) != 1) {
        float   r = 0.0f, g = 0.0f, b = 0.0f;
        displayParameters.yuvToRGBWithColorCorrection(r, g, b, y, u, v);
        if (!yuvFormat) {
          r *= yScaleTable[k];
          g *= yScaleTable[k];
          b *= yScaleTable[k];
          colormapData[i] = pixelConv(r, g, b);
        }
        else {
          y = (0.299f * r) + (0.587f * g) + (0.114f * b);
          u = 0.492f * (b - y);
          v = 0.877f * (r - y);
          y *= yScaleTable[k];
          u *= yScaleTable[k];
          v *= yScaleTable[k];
          if (y < 0.0f) {
            u *= (0.25f / (0.25f - y));
            v *= (0.25f / (0.25f - y));
            y = 0.0f;
          }
          else if (y > 1.0f) {
            u *= (0.25f / (y - 0.75f));
            v *= (0.25f / (y - 0.75f));
            y = 1.0f;
          }
          u = (u + 0.435912f) * 1.147020f;
          v = (v + 0.614777f) * 0.813303f;
          colormapData[i] = pixelConv(y, u, v);
        }
      }
      else {
        // 8-bit colormap, no color correction
        y = y * yScaleTable[k];
        u = u * yScaleTable[k];
        v = v * yScaleTable[k];
        colormapData[i] = pixelConv(y, u, v);
      }
    }
  }

  template class VideoDisplayColormap<uint8_t>;
  template class VideoDisplayColormap<uint16_t>;
  template class VideoDisplayColormap<uint32_t>;

  void getPlus4PaletteColor(int c, float& y, float& u, float& v)
  {
    c = c & 0xFF;
    double  p = 0.0;
    double  s = 0.19;
    if (c < 0x80) {
      if ((c & 0x0F) != 0 || c == 0x10) {
        Plus4::TED7360::convertPixelToYUV(uint8_t(c), false, y, u, v);
        return;
      }
      else if (c == 0x00) {
        y = 0.0f;                                       // sync
        u = 0.0f;
        v = 0.0f;
        return;
      }
      else {
        Plus4::TED7360::convertPixelToYUV(0x00, false, y, u, v);
        p = (3.14159265 / 4.0) * double(c >> 4);        // burst
        s = 0.1795;
      }
    }
    else if ((c & 0x0F) == 0x0E) {                      // dark blue / NTSC
      Plus4::TED7360::convertPixelToYUV(uint8_t(c), true, y, u, v);
      return;
    }
    else {
      Plus4::TED7360::convertPixelToYUV(uint8_t((c & 0x70) | 0x01), false,
                                        y, u, v);
      switch (c & 0x0F) {
      case 12:                                          // additional hues
        p = 181.0 * 3.14159265 / 180.0;
        break;
      case 13:
        p = 303.0 * 3.14159265 / 180.0;
        break;
      case 15:
        p = 68.0 * 3.14159265 / 180.0;
        break;
      default:
        p = (3.14159265 / 6.0) * double(c & 0x0F);      // half saturation
        s = s * 0.5;
        break;
      }
    }
    u = float(std::cos(p) * s);
    v = float(std::sin(p) * s);
  }

}       // namespace Plus4Emu

