
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
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

#ifndef PLUS4EMU_GLDISP_HPP
#define PLUS4EMU_GLDISP_HPP

#include "plus4emu.hpp"
#include "system.hpp"
#include "display.hpp"
#include "fldisp.hpp"

#include <FL/Fl_Gl_Window.H>

namespace Plus4Emu {

  class OpenGLDisplay : public Fl_Gl_Window, public FLTKDisplay_ {
   protected:
    bool compileShader(int shaderMode_);
    void deleteShader();
    bool enableShader();
    void disableShader();
    void initializeGLDisplay();
    void setColormap_quality3(const VideoDisplay::DisplayParameters& dp);
    void applyDisplayParameters(const VideoDisplay::DisplayParameters& dp);
    void decodeLine_quality0(uint16_t *outBuf,
                             Message_LineData **lineBuffers_, size_t lineNum);
    void decodeLine_quality3(uint32_t *outBuf,
                             Message_LineData **lineBuffers_, int lineNum,
                             const VideoDisplayColormap<uint32_t>& colormap);
    void drawFrame_quality0(Message_LineData **lineBuffers_,
                            double x0, double y0, double x1, double y1,
                            bool oddFrame_);
    void drawFrame_quality1(Message_LineData **lineBuffers_,
                            double x0, double y0, double x1, double y1,
                            bool oddFrame_);
    void drawFrame_quality2(Message_LineData **lineBuffers_,
                            double x0, double y0, double x1, double y1,
                            bool oddFrame_);
    void drawFrame_quality3(Message_LineData **lineBuffers_,
                            double x0, double y0, double x1, double y1,
                            bool oddFrame_);
    void copyFrameToRingBuffer();
    static void fltkIdleCallback(void *userData_);
    void displayFrame();
    // ----------------
    VideoDisplayColormap<uint16_t>  colormap16;
    VideoDisplayColormap<uint32_t>  colormap32_0;
    VideoDisplayColormap<uint32_t>  colormap32_1;
    bool          *linesChanged;
    // 768x14 subtexture in 16-bit R5G6B5, or 32-bit R8G8B8 or Y8U8V8 format
    unsigned char *textureSpace;
    uint16_t      *textureBuffer16;
    uint32_t      *textureBuffer32;
    unsigned long textureID;
    uint8_t       forceUpdateLineCnt;
    uint8_t       forceUpdateLineMask;
    bool          redrawFlag;
    bool          yuvTextureMode;
    bool          prvFrameWasOdd;
    int           lastLineNum;
    Timer         noInputTimer;
    Timer         forceUpdateTimer;
    Timer         displayFrameRateTimer;
    Timer         inputFrameRateTimer;
    double        displayFrameRate;
    double        inputFrameRate;
    Message_LineData  **frameRingBuffer[4];
    double        ringBufferReadPos;
    int           ringBufferWritePos;
    int           shaderMode;   // 0: no shader, 1: PAL, 2: NTSC
    unsigned long shaderHandle;
    unsigned long programHandle;
    std::string   shaderSources[2];
   public:
    OpenGLDisplay(int xx = 0, int yy = 0, int ww = 768, int hh = 576,
                  const char *lbl = (char *) 0, bool isDoubleBuffered = false);
    virtual ~OpenGLDisplay();
#ifndef ENABLE_GL_SHADERS
    virtual void setDisplayParameters(const DisplayParameters& dp);
#endif
    /*!
     * Read and process messages sent by the child thread. Returns true if
     * redraw() needs to be called to update the display.
     */
    virtual bool checkEvents();
   protected:
    virtual void draw();
   public:
    virtual int handle(int event);
    /*!
     * Load GLSL fragment shader for PAL emulation from 'fileName'.
     * If the name is empty or NULL, the default shader will be used.
     */
    void loadShaderSource(const char *fileName, bool isNTSC);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_GLDISP_HPP

