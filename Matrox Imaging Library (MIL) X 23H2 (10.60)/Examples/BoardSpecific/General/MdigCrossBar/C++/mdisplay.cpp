/********************************************************************************/
/*
* File name: mdisplay.cpp
*
* Synopsis:  This class manages a display output.
*
*
*            A thread is created for each display.
*            When a DisplayBuffer() is called,
*            the buffer is inserted in a FIFO that
*            is consumed by the created thread.
*
*            To calculate the latency between the output and a input,
*            a black pixel containing the display index is inserted on the output image
*            at offset 0,0. The black pixel is sent for 10 frames then a white pixel is sent.
*            When the white pixel is grabbed, the time difference is taken which is the latency.
*
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <windows.h> 
#include <oleauto.h>
#include <commdlg.h>
#include <mil.h>
#include <queue>
#include <list>
#include "d3d9.h"
#include "md3ddisplayeffect.h"
#include "mdisplay.h"
using namespace std;

// Private.
// Initialisation.
void CDisplay::Init()
   {
   m_Index = 0;
   m_SizeX = 0;
   m_SizeY = 0;
   memset(m_SourceSizeX, 0, sizeof(m_SourceSizeX));
   memset(m_SourceSizeY, 0, sizeof(m_SourceSizeY));
   m_DisplayFrameRate = 0.0;
   m_DisplayCount = 0;
   m_DisplayOverlayCount = 0;
   m_FrameSkip = 0;
   m_DisplayStartTime = 0;

   m_pD3DDevice = NULL;
   m_ThreadId = M_NULL;
   memset(m_pDst, 0, sizeof(m_pDst));
   memset(m_pDstOverlay, 0, sizeof(m_pDstOverlay));
   m_DisplayOverlayLastBuffer = NULL;
   m_DisplayOverlayEnable = false;

   m_pD3DFont = NULL;
   //m_pSwapChain = NULL;

   memset(m_SourceId, 0, sizeof(m_SourceId));
   m_Latency.Init();

   m_EnableD3DEffect = false;
   m_D3DEffect.Init();
   m_DrawDisplayInfo = DRAW_DISPLAYINFO;
   m_Exit = false;

   m_IsAllocated = false;
   }

// Public. 
// Allocates the display.
bool CDisplay::Allocate(MIL_ID iMilSystem, MIL_INT iIndex, LPDIRECT3D9EX pD3D)
   {
   bool Success = false;

   Init();
   m_DisplayModeEx.Size = sizeof(D3DDISPLAYMODEEX);
   HRESULT hr = pD3D->GetAdapterDisplayModeEx((UINT)iIndex + 1, &m_DisplayModeEx, NULL);

   if (hr == D3D_OK)
      {
      m_SizeX = m_DisplayModeEx.Width;
      m_SizeY = m_DisplayModeEx.Height;
      m_Index = iIndex;

      D3DPRESENT_PARAMETERS d3dpp;
      memset(&d3dpp, 0, sizeof(d3dpp));

      d3dpp.Windowed = FALSE;
      d3dpp.SwapEffect = D3DSWAPEFFECT_FLIPEX;

      d3dpp.BackBufferWidth = m_DisplayModeEx.Width;
      d3dpp.BackBufferHeight = m_DisplayModeEx.Height;
      d3dpp.BackBufferFormat = m_DisplayModeEx.Format;
      d3dpp.FullScreen_RefreshRateInHz = m_DisplayModeEx.RefreshRate;
      d3dpp.BackBufferCount = MAX_DISPLAY_BUFFERING + 1;
      d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
      d3dpp.hDeviceWindow = GetDesktopWindow();

      hr = pD3D->CreateDeviceEx((UINT)(iIndex + 1), D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_ENABLE_PRESENTSTATS | D3DCREATE_MULTITHREADED, &d3dpp, &m_DisplayModeEx, &m_pD3DDevice);
      if (hr == D3D_OK)
         {
         D3DXCreateFont(m_pD3DDevice,  // D3D device
            (INT)m_SizeY / 50,          // Height
            (UINT)m_SizeX / 160,        // Width
            FW_BOLD,                   // Weight
            1,                         // MipLevels, 0 = autogen mipmaps
            FALSE,                     // Italic
            DEFAULT_CHARSET,           // CharSet
            OUT_DEFAULT_PRECIS,        // OutputPrecision
            DEFAULT_QUALITY,           // Quality
            DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
            MIL_TEXT("Arial"),         // pFaceName
            &m_pD3DFont);              // ppFont

         m_pD3DDevice->GetSwapChain(0, &m_pSwapChain);

         m_pSwapChain->QueryInterface(IID_IDirect3DSwapChain9Ex, reinterpret_cast<void **>(&m_pSwapChainEx));

         InitializeCriticalSection(&m_CSLock);
         InitializeCriticalSection(&m_ReallocationLock);
         MthrAlloc(iMilSystem, M_THREAD, M_DEFAULT, &DispUpdateThread, (void *)this, &m_ThreadId);

         MosPrintf(MIL_TEXT("Allocating display %d (%lld x %lld @ %lldHz %s)\n"),
            (int)iIndex,
            (long long)m_DisplayModeEx.Width,
            (long long)m_DisplayModeEx.Height,
            (long long)m_DisplayModeEx.RefreshRate,
            (m_DisplayModeEx.ScanLineOrdering == D3DSCANLINEORDERING_INTERLACED) ? MIL_TEXT("interlaced") : MIL_TEXT("progressive"));

         m_pD3DDevice->SetMaximumFrameLatency(2);
         }

      // Is everything allocated.
      if (m_pD3DDevice && m_pD3DFont && m_pSwapChain && m_ThreadId)
         Success = true;
      }

   m_IsAllocated = Success;

   return Success;
   }

// Private.
// Frees everything.
void CDisplay::Free()
   {
   m_Exit = true;

   if (m_IsAllocated)
      {
      MthrWait(m_ThreadId, M_THREAD_END_WAIT, M_NULL);

      MthrFree(m_ThreadId);
      m_pD3DDevice->Release();
      for (MIL_INT i = 0; i < MAX_DISPLAY_BUFFERING; i++)
         {
         if (m_pDst[i])
            m_pDst[i]->Release();
         if (m_pDstOverlay[i])
            m_pDstOverlay[i]->Release();
         }

      m_pD3DFont->Release();
      DeleteCriticalSection(&m_CSLock);
      DeleteCriticalSection(&m_ReallocationLock);
      m_D3DEffect.Free();
      Init();
      m_IsAllocated = false;
      }
   }

// Public.
// Sets the display input source ID, reallocates internal buffers matching the input size.
void CDisplay::SetDisplaySource(MIL_INT64 iSourceId, MIL_INT iSizeX,
   MIL_INT iSizeY)
   {
   EnterCriticalSection(&m_ReallocationLock);
   EnterCriticalSection(&m_CSLock);

   // Empty fifos.
   while (!m_DisplayQueue.empty())
      m_DisplayQueue.pop();

   for (MIL_INT i = 0; i < MAX_DISPLAY_BUFFERING; i++)
      {
      if (m_pDst[i])
         {
         m_pDst[i]->Release();
         m_pDst[i] = NULL;
         }
      }

   for (MIL_INT i = 0; i < MAX_DISPLAY_BUFFERING; i++)
      {
      if (iSourceId && iSizeX && iSizeY)
         {
         m_SourceSizeX[0] = iSizeX;
         m_SourceSizeY[0] = iSizeY;
         m_pD3DDevice->CreateOffscreenPlainSurface((UINT)m_SourceSizeX[0], (UINT)m_SourceSizeY[0], D3DFMT_YUY2, D3DPOOL_DEFAULT, &m_pDst[i], NULL);
         }
      }
   m_SourceId[0] = iSourceId;
   ResetStatistic();
   LPDIRECT3DSURFACE9 pBackBuffer;
   m_pSwapChainEx->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);

   if (m_pDst[0])
      {
      D3DLOCKED_RECT LockRect;
      LockRect.pBits = 0;
      while (LockRect.pBits == 0)
         m_pDst[0]->LockRect(&LockRect, NULL, 0);

      memset(LockRect.pBits, 127, LockRect.Pitch * m_SourceSizeY[0]);
      m_pDst[0]->UnlockRect();
      }

   m_pD3DDevice->StretchRect(m_pDst[0], NULL, pBackBuffer, NULL, D3DTEXF_NONE);
   m_pD3DDevice->PresentEx(0, 0, 0, 0, 0);
   LeaveCriticalSection(&m_CSLock);
   LeaveCriticalSection(&m_ReallocationLock);
   }

// Public.
// Sets the display overlay input source ID, reallocates internal buffers matching the input size.
void CDisplay::SetDisplayOverlaySource(MIL_INT64 iSourceId, MIL_INT iSizeX, MIL_INT iSizeY)
   {
   EnterCriticalSection(&m_ReallocationLock);
   EnterCriticalSection(&m_CSLock);
   m_DisplayOverlayEnable = false;

   // Empty fifos.
   while (!m_DisplayOverlayQueue.empty())
      m_DisplayOverlayQueue.pop();

   for (MIL_INT i = 0; i < MAX_DISPLAY_BUFFERING; i++)
      {
      if (m_pDstOverlay[i])
         {
         m_pDstOverlay[i]->Release();
         m_pDstOverlay[i] = NULL;
         }
      }

   for (MIL_INT i = 0; i < MAX_DISPLAY_BUFFERING; i++)
      {
      if (iSourceId && iSizeX && iSizeY)
         {
         m_DisplayOverlayEnable = true;
         m_SourceSizeX[1] = iSizeX;
         m_SourceSizeY[1] = iSizeY;
         m_pD3DDevice->CreateOffscreenPlainSurface((UINT)m_SourceSizeX[1], (UINT)m_SourceSizeY[1], D3DFMT_YUY2, D3DPOOL_DEFAULT, &m_pDstOverlay[i], NULL);
         }
      }

   m_SourceId[1] = iSourceId;
   m_DisplayOverlayLastBuffer = NULL;

   LeaveCriticalSection(&m_CSLock);
   LeaveCriticalSection(&m_ReallocationLock);
   }

// Public.
// Reset the statistics.
void CDisplay::ResetStatistic()
   {
   m_DisplayFrameRate = 0;
   m_DisplayCount = 0;
   m_DisplayOverlayCount = 0;
   m_FrameSkip = 0;
   }

// Public.
// Returns the display statistics.
void CDisplay::GetStatistic(MIL_DOUBLE *FrameRate, MIL_INT *FrameCount, MIL_INT *FramesSkipped)
   {
   if (FrameRate)
      *FrameRate = m_DisplayFrameRate;
   if (FrameCount)
      *FrameCount = m_DisplayCount;
   if (*FramesSkipped)
      *FramesSkipped = m_FrameSkip;
   }

// Public.
// This function update the display with the buffer.
bool CDisplay::DisplayBuffer(MIL_ID iBuffer)
   {
   EnterCriticalSection(&m_CSLock);
   while (m_DisplayQueue.size() >= MAX_DISPLAY_BUFFERING)
      {
      m_FrameSkip++;
      m_DisplayQueue.pop();
      }
   m_DisplayQueue.push(iBuffer);
   LeaveCriticalSection(&m_CSLock);

   return true;
   }

// Public.
// This function update the overlay with the buffer.
bool CDisplay::DisplayOverlayBuffer(MIL_ID iBuffer)
   {
   // Update the overlay at half the rate.
   if (m_DisplayOverlayEnable)
      {
      m_DisplayOverlayCount++;
      if (m_DisplayOverlayCount % 2 == 0)
         return false;
      }

   EnterCriticalSection(&m_CSLock);
   while (m_DisplayOverlayQueue.size() >= MAX_DISPLAY_BUFFERING)
      m_DisplayOverlayQueue.pop();
   m_DisplayOverlayQueue.push(iBuffer);
   LeaveCriticalSection(&m_CSLock);

   return true;
   }

// Public.
// Calculates the latency. The unmodified grab buffer is used to read the latency tag.
bool CDisplay::UpdateLatency(MIL_ID iBuffer)
   {
   bool IsBufferFromThisDisplay = false;

   // Calculate latency.
   if (m_Latency.Enable && (m_Latency.State == eLATENCY_LATCH_WAITING_FOR_TAG))
      {
      MIL_UINT32 PixelValue = 0;
      MbufGet2d(iBuffer, 0, 0, 1, 1, &PixelValue);

      MIL_INT TagValue = ((PixelValue >> 8) & 0xffff);
      MIL_INT DisplayIndex = ((PixelValue)& 0xff) - 10;

      // Is buffer from this display.
      if (m_Index == DisplayIndex)
         IsBufferFromThisDisplay = true;

      // White image, now lets measure the latency.
      if ((TagValue == 0xf0f0) && IsBufferFromThisDisplay)
         m_Latency.State = eLATENCY_READ_LATENCY;
      }

   return IsBufferFromThisDisplay;
   }

// Public.
// Enable/disable latency calculation.
void CDisplay::Latency(bool State)
   {
   m_Latency.Init();
   m_Latency.Enable = State;
   }

// Public.
// Returns if latency calculation is enabled.
bool CDisplay::Latency()
   {
   return m_Latency.Enable;
   }

// Public.
// Returns latency results.
bool CDisplay::GetLatency(MIL_DOUBLE *Cur, MIL_DOUBLE *Min, MIL_DOUBLE *Max, MIL_DOUBLE *Average, MIL_INT *CurInFrames, MIL_INT *Count)
   {
   *Cur = 0.0;
   *Min = 0.0;
   *Max = 0.0;
   *Average = 0.0;
   *CurInFrames = 0;
   *Count = 0;

   if (m_Latency.Enable)
      {
      if (Cur)
         *Cur = m_Latency.Cur;
      if (Min)
         *Min = m_Latency.Min;
      if (Max)
         *Max = m_Latency.Max;
      if (Average && m_Latency.AverageCount)
         {
         *Average = (m_Latency.Average / (MIL_DOUBLE)m_Latency.AverageCount);
         }
      if (CurInFrames)
         *CurInFrames = m_Latency.LatencyInFrames;
      if (Count)
         *Count = m_Latency.AverageCount;
      }

   return m_Latency.Enable;
   }

// Public.
// Enable/disable Direct3D effects.
void CDisplay::D3DEffect(bool State)
   {
   m_EnableD3DEffect = State;
   }

// Public.
// Returns if Direct3D effects is enabled.
bool CDisplay::D3DEffect()
   {
   return m_EnableD3DEffect;
   }

// Private.
// Copies the image from the host to the board for display.
void CDisplay::UpdateDisplay(CDisplay *pDisp, MIL_ID SourceBuf, MIL_ID OverlayBuf)
   {
   unsigned char *pSrcData = 0;
   MIL_INT SrcSizeX, SrcSizeY, SrcPitchByte;
   D3DLOCKED_RECT LockRect;
   MIL_ID SrcBuffers[2] = { SourceBuf, OverlayBuf };
   MIL_INT BufferIndex = pDisp->m_DisplayCount%MAX_DISPLAY_BUFFERING;

   if (!pDisp->m_SourceId)
      return;

   MIL_DOUBLE lCurTime = 0;
   MappTimer(M_TIMER_READ + M_GLOBAL, &lCurTime);

   if (pDisp->m_DisplayCount == 0)
      pDisp->m_DisplayStartTime = lCurTime;

   LPDIRECT3DSURFACE9 pDest[2] = { pDisp->m_pDst[BufferIndex], pDisp->m_pDstOverlay[BufferIndex] };

   if (!pDest[0])
      return;

   pDisp->m_DisplayCount++;
   m_DisplayFrameRate = (pDisp->m_DisplayCount) / (lCurTime - pDisp->m_DisplayStartTime);

   // Copy the buffers (main image and overlay) into on-board video memory.
   for (MIL_INT i = 0; (i < 2) && SrcBuffers[i] && pDest[i]; i++)
      {
      MbufInquire(SrcBuffers[i], M_HOST_ADDRESS, &pSrcData);
      MbufInquire(SrcBuffers[i], M_SIZE_X, &SrcSizeX);
      MbufInquire(SrcBuffers[i], M_SIZE_Y, &SrcSizeY);
      MbufInquire(SrcBuffers[i], M_PITCH_BYTE, &SrcPitchByte);

      LockRect.pBits = 0;

      // Buffer is YUV16
      if ((SrcSizeX == pDisp->m_SourceSizeX[i]) && (SrcSizeY == pDisp->m_SourceSizeY[i]))
         {
         while (LockRect.pBits == 0)
            {
            pDest[i]->LockRect(&LockRect, NULL, 0);
            }

         if (SrcPitchByte == LockRect.Pitch)
            {
            // When the pitchs are the same, use one memcpy.
            memcpy(LockRect.pBits, pSrcData, LockRect.Pitch * pDisp->m_SourceSizeY[i]);
            }
         else
            {
            // The pitchs are not the same, use memcpy per line.
            for (MIL_INT Line = 0; Line < SrcSizeY; Line++)
               {
               unsigned char *pdestline = (unsigned char *)(((unsigned char *)LockRect.pBits) + (LockRect.Pitch * Line));
               unsigned char *psrcline = pSrcData + (SrcPitchByte * Line);
               memcpy(pdestline, psrcline, SrcSizeX * 2);
               }
            }

         pDest[i]->UnlockRect();
         }
      }

   LPDIRECT3DSURFACE9 pBackBuffer;
   pDisp->m_pSwapChainEx->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
   if (pDisp->m_EnableD3DEffect)
      {
      DX9Processing(&pDisp->m_D3DEffect, pDisp->m_pD3DDevice, pDest[0], pBackBuffer);
      }
   else
      {
      pDisp->m_pD3DDevice->StretchRect(pDest[0], NULL, pBackBuffer, NULL, D3DTEXF_NONE);
      }

   // Draw text.
   if (pDisp->m_DrawDisplayInfo)
      {
      MIL_STRING_STREAM Buf;
      Buf << MIL_TEXT("Display:") << pDisp->m_Index << MIL_TEXT(" ");
      RECT rc;
      SetRect(&rc, 10, 5, 0, 0);
      m_pD3DDevice->BeginScene();
      pDisp->m_pD3DFont->DrawText(NULL, Buf.str().c_str(), -1, &rc, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
      m_pD3DDevice->EndScene();
      }

   if (pDisp->m_DisplayOverlayEnable)
      {
      RECT rect;
      rect.right = (LONG)pDisp->SizeX() / 3;
      rect.bottom = (LONG)pDisp->SizeY() / 3;
      rect.top = 90;
      rect.left = 50;

      // If no overlay buffer is present, use the last one.
      if (OverlayBuf == NULL)
         pDest[1] = m_DisplayOverlayLastBuffer;

      pDisp->m_pD3DDevice->StretchRect(pDest[1], NULL, pBackBuffer, &rect, D3DTEXF_NONE);
      m_DisplayOverlayLastBuffer = pDest[1];
      }

   pDisp->m_pD3DDevice->PresentEx(0, 0, 0, 0, D3DPRESENT_DONOTWAIT);

   // Calculate latency.
   if (pDisp->m_Latency.Enable)
      {
      RECT rect;
      rect.bottom = 1;
      rect.top = 0;
      rect.left = 0;
      rect.right = 1;

      if (pDisp->m_Latency.State == eLATENCY_DISABLE)
         {
         pDisp->m_Latency.StartCount = 10;
         pDisp->m_Latency.StartTime = 0.0;
         pDisp->m_Latency.State = eLATENCY_COUNTING;
         pDisp->m_Latency.LatencyInFramesCounter = 1;
         }
      else if (pDisp->m_Latency.State == eLATENCY_COUNTING)
         {
         // Now output 10 black images. The display index value is the R value
         // of the RGB output.
         pDisp->m_Latency.StartCount--;
         pDisp->m_pD3DDevice->ColorFill(pBackBuffer, &rect, D3DCOLOR_XRGB(10 + pDisp->m_Index, 10, 10));
         if (pDisp->m_Latency.StartCount == 0)
            pDisp->m_Latency.State = eLATENCY_LATCHING_TAG_IMAGE;
         }
      else if (pDisp->m_Latency.State == eLATENCY_LATCHING_TAG_IMAGE)
         {
         // Now output one white image;
         pDisp->m_pD3DDevice->ColorFill(pBackBuffer, &rect, D3DCOLOR_XRGB(10 + pDisp->m_Index, 240, 240));
         if (pDisp->m_Latency.StartTime == 0.0)
            pDisp->m_Latency.StartTime = lCurTime;
         pDisp->m_Latency.State = eLATENCY_LATCH_WAITING_FOR_TAG;
         pDisp->m_Latency.LatencyInFramesCounter = 1;
         }
      else if (pDisp->m_Latency.State == eLATENCY_LATCH_WAITING_FOR_TAG)
         {
         // Output one white image;
         pDisp->m_pD3DDevice->ColorFill(pBackBuffer, &rect, D3DCOLOR_XRGB(10 + pDisp->m_Index, 240, 240));
         pDisp->m_Latency.LatencyInFramesCounter++;
         }
      else if (pDisp->m_Latency.State == eLATENCY_READ_LATENCY)
         {
         // Now we read the latency.
         pDisp->m_Latency.EndTime = lCurTime;
         MIL_DOUBLE lLatency = pDisp->m_Latency.EndTime - pDisp->m_Latency.StartTime;

         if ((pDisp->m_Latency.Min == 0) || (lLatency < pDisp->m_Latency.Min))
            pDisp->m_Latency.Min = lLatency;

         if (lLatency > pDisp->m_Latency.Max)
            pDisp->m_Latency.Max = lLatency;

         pDisp->m_Latency.Cur = lLatency;
         pDisp->m_Latency.Average += lLatency;
         pDisp->m_Latency.AverageCount++;

         pDisp->m_Latency.LatencyInFrames = pDisp->m_Latency.LatencyInFramesCounter;
         pDisp->m_Latency.State = eLATENCY_DISABLE;
         }
      }



   }
