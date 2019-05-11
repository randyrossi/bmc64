//
// vicescreen.cpp
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

#include "vicescreen.h"
#include "viceoptions.h"
#include <circle/devicenameservice.h>
#include <circle/synchronize.h>
#include <circle/sysconfig.h>
#include <circle/util.h>

#define ROTORS 4

enum TScreenState {
  ScreenStateStart,
  ScreenStateEscape,
  ScreenStateBracket,
  ScreenStateNumber1,
  ScreenStateQuestionMark,
  ScreenStateSemicolon,
  ScreenStateNumber2,
  ScreenStateNumber3
};

CViceScreenDevice::CViceScreenDevice(unsigned nWidth, unsigned nHeight,
                                     boolean bVirtual)
    : m_nInitWidth(nWidth), m_nInitHeight(nHeight), m_bVirtual(bVirtual),
      m_pFrameBuffer(0), m_pBuffer(0), m_nState(ScreenStateStart),
      m_nScrollStart(0), m_nCursorX(0), m_nCursorY(0), m_bCursorOn(TRUE),
      m_Color(NORMAL_COLOR), m_bInsertOn(FALSE), m_bUpdated(FALSE)
#ifdef REALTIME
      ,
      m_SpinLock(TASK_LEVEL)
#endif
{
}

CViceScreenDevice::~CViceScreenDevice(void) {
  if (m_bVirtual) {
    delete[] m_pBuffer;
  }
  m_pBuffer = 0;

  delete m_pFrameBuffer;
  m_pFrameBuffer = 0;
}

boolean CViceScreenDevice::Initialize(void) {
  if (!m_bVirtual) {
    // Give a virtual height of x2 the physical height for our
    // double buffering technique.
    m_pFrameBuffer = new CBcmFrameBuffer(m_nInitWidth, m_nInitHeight, DEPTH,
                                         m_nInitWidth, m_nInitHeight * 2);
#if DEPTH == 8
    m_pFrameBuffer->SetPalette(NORMAL_COLOR, NORMAL_COLOR16);
    m_pFrameBuffer->SetPalette(HIGH_COLOR, HIGH_COLOR16);
    m_pFrameBuffer->SetPalette(HALF_COLOR, HALF_COLOR16);
#endif
    if (!m_pFrameBuffer->Initialize()) {
      return FALSE;
    }

    if (m_pFrameBuffer->GetDepth() != DEPTH) {
      return FALSE;
    }

    m_pBuffer = (TScreenColor *)m_pFrameBuffer->GetBuffer();
    m_nSize = m_pFrameBuffer->GetSize();
    m_nPitch = m_pFrameBuffer->GetPitch();
    m_nWidth = m_pFrameBuffer->GetWidth();
    m_nHeight = m_pFrameBuffer->GetHeight();

    // Ensure that each row is word-aligned so that we can safely use
    // memcpyblk()
    if (m_nPitch % sizeof(u32) != 0) {
      return FALSE;
    }
    m_nPitch /= sizeof(TScreenColor);
  } else {
    m_nWidth = m_nInitWidth;
    m_nHeight = m_nInitHeight;
    m_nSize = m_nWidth * m_nHeight * sizeof(TScreenColor);
    m_nPitch = m_nWidth;

    m_pBuffer = new TScreenColor[m_nWidth * m_nHeight];
  }

  m_nUsedHeight =
      m_nHeight / m_CharGen.GetCharHeight() * m_CharGen.GetCharHeight();
  m_nScrollEnd = m_nUsedHeight;

  CursorHome();
  ClearDisplayEnd();
  InvertCursor();

  CDeviceNameService::Get()->AddDevice("tty1", this, FALSE);

  return TRUE;
}

unsigned CViceScreenDevice::GetWidth(void) const { return m_nWidth; }

unsigned CViceScreenDevice::GetHeight(void) const { return m_nHeight; }

unsigned CViceScreenDevice::GetColumns(void) const {
  return m_nWidth / m_CharGen.GetCharWidth();
}

unsigned CViceScreenDevice::GetRows(void) const {
  return m_nUsedHeight / m_CharGen.GetCharHeight();
}

TScreenStatus CViceScreenDevice::GetStatus(void) {
  TScreenStatus Status;

  Status.pContent = m_pBuffer;
  Status.nSize = m_nSize;
  Status.nState = m_nState;
  Status.nScrollStart = m_nScrollStart;
  Status.nScrollEnd = m_nScrollEnd;
  Status.nCursorX = m_nCursorX;
  Status.nCursorY = m_nCursorY;
  Status.bCursorOn = m_bCursorOn;
  Status.Color = m_Color;
  Status.bInsertOn = m_bInsertOn;
  Status.nParam1 = m_nParam1;
  Status.nParam2 = m_nParam2;
  Status.bUpdated = m_bUpdated;

  return Status;
}

boolean CViceScreenDevice::SetStatus(const TScreenStatus &Status) {
  if (m_nSize != Status.nSize || m_nPitch != m_nWidth) {
    return FALSE;
  }

  m_SpinLock.Acquire();

  if (m_bUpdated || Status.bUpdated) {
    m_SpinLock.Release();

    return FALSE;
  }

  memcpyblk(m_pBuffer, Status.pContent, m_nSize);

  m_nState = Status.nState;
  m_nScrollStart = Status.nScrollStart;
  m_nScrollEnd = Status.nScrollEnd;
  m_nCursorX = Status.nCursorX;
  m_nCursorY = Status.nCursorY;
  m_bCursorOn = Status.bCursorOn;
  m_Color = Status.Color;
  m_bInsertOn = Status.bInsertOn;
  m_nParam1 = Status.nParam1;
  m_nParam2 = Status.nParam2;

  m_SpinLock.Release();

  DataMemBarrier();

  return TRUE;
}

int CViceScreenDevice::Write(const void *pBuffer, unsigned nCount) {
  m_SpinLock.Acquire();

  m_bUpdated = TRUE;

  InvertCursor();

  const char *pChar = (const char *)pBuffer;
  int nResult = 0;

  while (nCount--) {
    Write(*pChar++);

    nResult++;
  }

  InvertCursor();

  m_bUpdated = FALSE;

  m_SpinLock.Release();

  DataMemBarrier();

  return nResult;
}

void CViceScreenDevice::Write(char chChar) {
  switch (m_nState) {
  case ScreenStateStart:
    switch (chChar) {
    case '\b':
      CursorLeft();
      break;

    case '\t':
      Tabulator();
      break;

    case '\n':
      NewLine();
      break;

    case '\r':
      CarriageReturn();
      break;

    case '\x1b':
      m_nState = ScreenStateEscape;
      break;

    default:
      DisplayChar(chChar);
      break;
    }
    break;

  case ScreenStateEscape:
    switch (chChar) {
    case 'M':
      ReverseScroll();
      m_nState = ScreenStateStart;
      break;

    case '[':
      m_nState = ScreenStateBracket;
      break;

    default:
      m_nState = ScreenStateStart;
      break;
    }
    break;

  case ScreenStateBracket:
    switch (chChar) {
    case '?':
      m_nState = ScreenStateQuestionMark;
      break;

    case 'A':
      CursorUp();
      m_nState = ScreenStateStart;
      break;

    case 'B':
      CursorDown();
      m_nState = ScreenStateStart;
      break;

    case 'C':
      CursorRight();
      m_nState = ScreenStateStart;
      break;

    case 'D':
      CursorLeft();
      m_nState = ScreenStateStart;
      break;

    case 'H':
      CursorHome();
      m_nState = ScreenStateStart;
      break;

    case 'J':
      ClearDisplayEnd();
      m_nState = ScreenStateStart;
      break;

    case 'K':
      ClearLineEnd();
      m_nState = ScreenStateStart;
      break;

    case 'L':
      InsertLines(1);
      m_nState = ScreenStateStart;
      break;

    case 'M':
      DeleteLines(1);
      m_nState = ScreenStateStart;
      break;

    case 'P':
      DeleteChars(1);
      m_nState = ScreenStateStart;
      break;

    default:
      if ('0' <= chChar && chChar <= '9') {
        m_nParam1 = chChar - '0';
        m_nState = ScreenStateNumber1;
      } else {
        m_nState = ScreenStateStart;
      }
      break;
    }
    break;

  case ScreenStateNumber1:
    switch (chChar) {
    case ';':
      m_nState = ScreenStateSemicolon;
      break;

    case 'L':
      InsertLines(m_nParam1);
      m_nState = ScreenStateStart;
      break;

    case 'M':
      DeleteLines(m_nParam1);
      m_nState = ScreenStateStart;
      break;

    case 'P':
      DeleteChars(m_nParam1);
      m_nState = ScreenStateStart;
      break;

    case 'X':
      EraseChars(m_nParam1);
      m_nState = ScreenStateStart;
      break;

    case 'h':
    case 'l':
      if (m_nParam1 == 4) {
        InsertMode(chChar == 'h');
      }
      m_nState = ScreenStateStart;
      break;

    case 'm':
      SetStandoutMode(m_nParam1);
      m_nState = ScreenStateStart;
      break;

    default:
      if ('0' <= chChar && chChar <= '9') {
        m_nParam1 *= 10;
        m_nParam1 += chChar - '0';

        if (m_nParam1 > 99) {
          m_nState = ScreenStateStart;
        }
      } else {
        m_nState = ScreenStateStart;
      }
      break;
    }
    break;

  case ScreenStateSemicolon:
    if ('0' <= chChar && chChar <= '9') {
      m_nParam2 = chChar - '0';
      m_nState = ScreenStateNumber2;
    } else {
      m_nState = ScreenStateStart;
    }
    break;

  case ScreenStateQuestionMark:
    if ('0' <= chChar && chChar <= '9') {
      m_nParam1 = chChar - '0';
      m_nState = ScreenStateNumber3;
    } else {
      m_nState = ScreenStateStart;
    }
    break;

  case ScreenStateNumber2:
    switch (chChar) {
    case 'H':
      CursorMove(m_nParam1, m_nParam2);
      m_nState = ScreenStateStart;
      break;

    case 'r':
      SetScrollRegion(m_nParam1, m_nParam2);
      m_nState = ScreenStateStart;
      break;

    default:
      if ('0' <= chChar && chChar <= '9') {
        m_nParam2 *= 10;
        m_nParam2 += chChar - '0';

        if (m_nParam2 > 199) {
          m_nState = ScreenStateStart;
        }
      } else {
        m_nState = ScreenStateStart;
      }
      break;
    }
    break;

  case ScreenStateNumber3:
    switch (chChar) {
    case 'h':
    case 'l':
      if (m_nParam1 == 25) {
        SetCursorMode(chChar == 'h');
      }
      m_nState = ScreenStateStart;
      break;

    default:
      if ('0' <= chChar && chChar <= '9') {
        m_nParam1 *= 10;
        m_nParam1 += chChar - '0';

        if (m_nParam1 > 99) {
          m_nState = ScreenStateStart;
        }
      } else {
        m_nState = ScreenStateStart;
      }
      break;
    }
    break;

  default:
    m_nState = ScreenStateStart;
    break;
  }
}

void CViceScreenDevice::CarriageReturn(void) { m_nCursorX = 0; }

void CViceScreenDevice::ClearDisplayEnd(void) {
  ClearLineEnd();

  unsigned nPosY = m_nCursorY + m_CharGen.GetCharHeight();
  unsigned nOffset = nPosY * m_nPitch;

  TScreenColor *pBuffer = m_pBuffer + nOffset;
  unsigned nSize = m_nSize / sizeof(TScreenColor) - nOffset;

  while (nSize--) {
    *pBuffer++ = BLACK_COLOR;
  }
}

void CViceScreenDevice::ClearLineEnd(void) {
  for (unsigned nPosX = m_nCursorX; nPosX < m_nWidth;
       nPosX += m_CharGen.GetCharWidth()) {
    EraseChar(nPosX, m_nCursorY);
  }
}

void CViceScreenDevice::CursorDown(void) {
  m_nCursorY += m_CharGen.GetCharHeight();
  if (m_nCursorY >= m_nScrollEnd) {
    Scroll();

    m_nCursorY -= m_CharGen.GetCharHeight();
  }
}

void CViceScreenDevice::CursorHome(void) {
  m_nCursorX = 0;
  m_nCursorY = m_nScrollStart;
}

void CViceScreenDevice::CursorLeft(void) {
  if (m_nCursorX > 0) {
    m_nCursorX -= m_CharGen.GetCharWidth();
  } else {
    if (m_nCursorY > m_nScrollStart) {
      m_nCursorX = m_nWidth - m_CharGen.GetCharWidth();
      m_nCursorY -= m_CharGen.GetCharHeight();
    }
  }
}

void CViceScreenDevice::CursorMove(unsigned nRow, unsigned nColumn) {
  unsigned nPosX = (nColumn - 1) * m_CharGen.GetCharWidth();
  unsigned nPosY = (nRow - 1) * m_CharGen.GetCharHeight();

  if (nPosX < m_nWidth && nPosY >= m_nScrollStart && nPosY < m_nScrollEnd) {
    m_nCursorX = nPosX;
    m_nCursorY = nPosY;
  }
}

void CViceScreenDevice::CursorRight(void) {
  m_nCursorX += m_CharGen.GetCharWidth();
  if (m_nCursorX >= m_nWidth) {
    NewLine();
  }
}

void CViceScreenDevice::CursorUp(void) {
  if (m_nCursorY > m_nScrollStart) {
    m_nCursorY -= m_CharGen.GetCharHeight();
  }
}

void CViceScreenDevice::DeleteChars(unsigned nCount) // TODO
{}

void CViceScreenDevice::DeleteLines(unsigned nCount) // TODO
{}

void CViceScreenDevice::DisplayChar(char chChar) {
  // TODO: Insert mode

  if (' ' <= (unsigned char)chChar) {
    DisplayChar(chChar, m_nCursorX, m_nCursorY, m_Color);

    CursorRight();
  }
}

void CViceScreenDevice::EraseChars(unsigned nCount) {
  if (nCount == 0) {
    return;
  }

  unsigned nEndX = m_nCursorX + nCount * m_CharGen.GetCharWidth();
  if (nEndX > m_nWidth) {
    nEndX = m_nWidth;
  }

  for (unsigned nPosX = m_nCursorX; nPosX < nEndX;
       nPosX += m_CharGen.GetCharWidth()) {
    EraseChar(nPosX, m_nCursorY);
  }
}

void CViceScreenDevice::InsertLines(unsigned nCount) // TODO
{}

void CViceScreenDevice::InsertMode(boolean bBegin) { m_bInsertOn = bBegin; }

void CViceScreenDevice::NewLine(void) {
  CarriageReturn();
  CursorDown();
}

void CViceScreenDevice::ReverseScroll(void) {
  if (m_nCursorX == 0 && m_nCursorY == 0) {
    InsertLines(1);
  }
}

void CViceScreenDevice::SetCursorMode(boolean bVisible) {
  m_bCursorOn = bVisible;
}

void CViceScreenDevice::SetScrollRegion(unsigned nStartRow, unsigned nEndRow) {
  unsigned nScrollStart = (nStartRow - 1) * m_CharGen.GetCharHeight();
  unsigned nScrollEnd = nEndRow * m_CharGen.GetCharHeight();

  if (nScrollStart < m_nUsedHeight && nScrollEnd > 0 &&
      nScrollEnd <= m_nUsedHeight && nScrollStart < nScrollEnd) {
    m_nScrollStart = nScrollStart;
    m_nScrollEnd = nScrollEnd;
  }

  CursorHome();
}

// TODO: standout mode should be useable together with one other mode
void CViceScreenDevice::SetStandoutMode(unsigned nMode) {
  switch (nMode) {
  case 0:
  case 27:
    m_Color = NORMAL_COLOR;
    break;

  case 1:
    m_Color = HIGH_COLOR;
    break;

  case 2:
    m_Color = HALF_COLOR;
    break;

  case 7: // TODO: reverse mode
  default:
    break;
  }
}

void CViceScreenDevice::Tabulator(void) {
  unsigned nTabWidth = m_CharGen.GetCharWidth() * 8;

  m_nCursorX = ((m_nCursorX + nTabWidth) / nTabWidth) * nTabWidth;
  if (m_nCursorX >= m_nWidth) {
    NewLine();
  }
}

void CViceScreenDevice::Scroll(void) {
  if (ViceOptions::Get()->GetHideConsole()) {
    return;
  }

  unsigned nLines = m_CharGen.GetCharHeight();

  u32 *pTo = (u32 *)(m_pBuffer + m_nScrollStart * m_nPitch);
  u32 *pFrom = (u32 *)(m_pBuffer + (m_nScrollStart + nLines) * m_nPitch);

  unsigned nSize = m_nPitch * (m_nScrollEnd - m_nScrollStart - nLines) *
                   sizeof(TScreenColor);
  if (nSize > 0) {
    unsigned nSizeBlk = nSize & ~0xF;
    memcpyblk(pTo, pFrom, nSizeBlk);

    // Handle framebuffers with row lengths not aligned to 16 bytes
    memcpy((u8 *)pTo + nSizeBlk, (u8 *)pFrom + nSizeBlk, nSize & 0xF);

    pTo += nSize / sizeof(u32);
  }

  nSize = m_nPitch * nLines * sizeof(TScreenColor) / sizeof(u32);
  while (nSize--) {
    *pTo++ = BLACK_COLOR;
  }
}

void CViceScreenDevice::DisplayChar(char chChar, unsigned nPosX, unsigned nPosY,
                                    TScreenColor Color) {
  for (unsigned y = 0; y < m_CharGen.GetCharHeight(); y++) {
    for (unsigned x = 0; x < m_CharGen.GetCharWidth(); x++) {
      SetPixel(nPosX + x, nPosY + y,
               m_CharGen.GetPixel(chChar, x, y) ? Color : BLACK_COLOR);
    }
  }
}

void CViceScreenDevice::EraseChar(unsigned nPosX, unsigned nPosY) {
  for (unsigned y = 0; y < m_CharGen.GetCharHeight(); y++) {
    for (unsigned x = 0; x < m_CharGen.GetCharWidth(); x++) {
      SetPixel(nPosX + x, nPosY + y, BLACK_COLOR);
    }
  }
}

void CViceScreenDevice::InvertCursor(void) {
  if (!m_bCursorOn) {
    return;
  }

  for (unsigned y = m_CharGen.GetUnderline(); y < m_CharGen.GetCharHeight();
       y++) {
    for (unsigned x = 0; x < m_CharGen.GetCharWidth(); x++) {
      if (GetPixel(m_nCursorX + x, m_nCursorY + y) == BLACK_COLOR) {
        SetPixel(m_nCursorX + x, m_nCursorY + y, m_Color);
      } else {
        SetPixel(m_nCursorX + x, m_nCursorY + y, BLACK_COLOR);
      }
    }
  }
}

void CViceScreenDevice::SetPixel(unsigned nPosX, unsigned nPosY,
                                 TScreenColor Color) {
  if (nPosX < m_nWidth && nPosY < m_nHeight) {
    if (!ViceOptions::Get()->GetHideConsole()) {
      m_pBuffer[m_nPitch * nPosY + nPosX] = Color;
    }
  }
}

TScreenColor CViceScreenDevice::GetPixel(unsigned nPosX, unsigned nPosY) {
  if (nPosX < m_nWidth && nPosY < m_nHeight) {
    return m_pBuffer[m_nPitch * nPosY + nPosX];
  }

  return BLACK_COLOR;
}

void CViceScreenDevice::Rotor(unsigned nIndex, unsigned nCount) {
  static const char chChars[] = "-\\|/";

  nIndex %= ROTORS;
  nCount &= 4 - 1;

  unsigned nPosX = m_nWidth - (nIndex + 1) * m_CharGen.GetCharWidth();

  DisplayChar(chChars[nCount], nPosX, 0, HIGH_COLOR);
}

void CViceScreenDevice::SetPalette(u8 index, u16 rgb565) {
  m_pFrameBuffer->SetPalette(index, rgb565);
}

void CViceScreenDevice::UpdatePalette() { m_pFrameBuffer->UpdatePalette(); }

void CViceScreenDevice::SetVirtualOffset(u32 x, u32 y) {
  m_pFrameBuffer->SetVirtualOffset(x, y);
}

void CViceScreenDevice::WaitForVerticalSync() {
  m_pFrameBuffer->WaitForVerticalSync();
}
