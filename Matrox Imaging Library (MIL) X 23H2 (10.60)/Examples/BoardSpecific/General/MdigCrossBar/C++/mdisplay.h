/********************************************************************************/
/*
* File name: mdisplay.h
*
* Synopsis:
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <windows.h> 
#include <mil.h>
#include <queue>
#include <list>
#include "d3d9.h"
#include "d3dx9.h"

using namespace std;

#define MAX_DISPLAY_BUFFERING 1

const bool DRAW_DISPLAYINFO = true;

MIL_UINT32 MFTYPE DispUpdateThread(void *TPar);


enum eLatencyState{ eLATENCY_DISABLE, eLATENCY_COUNTING, eLATENCY_LATCHING_TAG_IMAGE, eLATENCY_LATCH_WAITING_FOR_TAG, eLATENCY_READ_LATENCY };
class CLatency
   {
   public:
      CLatency()
         {
         Init();
         }
      void Init()
         {
         // Latency
         Enable = false;
         State = eLATENCY_DISABLE;

         StartTime = 0.0;
         EndTime = 0.0;
         EndTimeGrab = 0.0;
         StartCount = 0;
         LatencyInFramesCounter = 0;
         LatencyInFrames = 0;
         AverageCount = 0;
         Cur = 0.0;
         Min = 0.0;
         Max = 0.0;
         Average = 0.0;
         }

      bool          Enable;
      eLatencyState State;
      MIL_DOUBLE StartTime;
      MIL_DOUBLE EndTimeGrab;
      MIL_DOUBLE EndTime;
      MIL_INT    StartCount;
      MIL_INT    LatencyInFramesCounter;
      MIL_INT    LatencyInFrames;
      MIL_INT    AverageCount;
      MIL_DOUBLE Cur;
      MIL_DOUBLE Min;
      MIL_DOUBLE Max;
      MIL_DOUBLE Average;

   };

class CDisplay
   {
   public:
      CDisplay(MIL_ID iMilSystem, MIL_INT iIndex, LPDIRECT3D9EX pD3D)
         {
         if (!Allocate(iMilSystem, iIndex, pD3D))
            Free();
         }
      ~CDisplay()
         {
         Free();
         }

      void SetDisplaySource(MIL_INT64 iSourceId, MIL_INT iSizeX, MIL_INT iSizeY);
      MIL_INT64 GetDisplaySource() { return m_SourceId[0]; };

      void SetDisplayOverlaySource(MIL_INT64 iSourceId, MIL_INT iSizeX, MIL_INT iSizeY);
      MIL_INT64 GetDisplayOverlaySource() { return m_SourceId[1]; };

      bool DisplayBuffer(MIL_ID iBuffer);
      bool DisplayOverlayBuffer(MIL_ID iBuffer);
      bool IsAllocated() { return m_IsAllocated; }

      MIL_INT Index() { return m_Index; }
      MIL_INT SizeX() { return m_SizeX; }
      MIL_INT SizeY() { return m_SizeY; }

      void ResetStatistic();
      void GetStatistic(MIL_DOUBLE *FrameRate, MIL_INT *FrameCount, MIL_INT *FramesSkipped);

      void Latency(bool State);
      bool Latency();
      bool UpdateLatency(MIL_ID iBuffer);
      bool GetLatency(MIL_DOUBLE *Cur, MIL_DOUBLE *Min, MIL_DOUBLE *Max, MIL_DOUBLE *Average, MIL_INT *CurInFrames, MIL_INT *Count);

      void D3DEffect(bool State);
      bool D3DEffect();

   protected:

      void Init();
      bool Allocate(MIL_ID iMilSystem, MIL_INT iIndex, LPDIRECT3D9EX pD3D);
      void Free();

      static MIL_UINT32 MFTYPE CDisplay::DispUpdateThread(void *TPar)
         {
         CDisplay *pDisp = (CDisplay *)TPar;

         while (pDisp->m_Exit == false)
            {
            MIL_ID WorkBuffer = M_NULL;
            MIL_ID OverlayBuffer = M_NULL;
            EnterCriticalSection(&pDisp->m_CSLock);

            if (!pDisp->m_DisplayQueue.empty())
               {
               WorkBuffer = pDisp->m_DisplayQueue.back();
               pDisp->m_DisplayQueue.pop();
               }

            if (!pDisp->m_DisplayOverlayQueue.empty())
               {
               OverlayBuffer = pDisp->m_DisplayOverlayQueue.back();
               pDisp->m_DisplayOverlayQueue.pop();
               }
            LeaveCriticalSection(&pDisp->m_CSLock);

            // Display it.
            if (WorkBuffer)
               {
               EnterCriticalSection(&pDisp->m_ReallocationLock);
               pDisp->UpdateDisplay(pDisp, WorkBuffer, OverlayBuffer);
               LeaveCriticalSection(&pDisp->m_ReallocationLock);
               }
            else
               {
               // If nothing to display wait for a buffer to be added.
               MosSleep(0);
               }
            };

         return 0;
         }

      void UpdateDisplay(CDisplay *pDisp, MIL_ID SourceBuf, MIL_ID OverlayBuf);


      bool         m_IsAllocated;
      MIL_INT      m_Index;
      MIL_INT      m_SizeX;
      MIL_INT      m_SizeY;

      MIL_INT      m_SourceSizeX[2];
      MIL_INT      m_SourceSizeY[2];

      MIL_DOUBLE   m_DisplayFrameRate;
      MIL_INT      m_DisplayCount;
      MIL_INT      m_DisplayOverlayCount;
      MIL_INT      m_FrameSkip;
      MIL_DOUBLE   m_DisplayStartTime;

      LPDIRECT3DDEVICE9EX     m_pD3DDevice;
      MIL_ID                  m_ThreadId;
      LPDIRECT3DSURFACE9      m_pDst[MAX_DISPLAY_BUFFERING];
      LPDIRECT3DSURFACE9      m_pDstOverlay[MAX_DISPLAY_BUFFERING];
      D3DDISPLAYMODEEX        m_DisplayModeEx;

      ID3DXFont              *m_pD3DFont;

      IDirect3DSwapChain9    *m_pSwapChain;
      IDirect3DSwapChain9Ex  *m_pSwapChainEx;

      // Source to be displayed.
      MIL_INT64 m_SourceId[2];
      CRITICAL_SECTION m_ReallocationLock;

      CLatency m_Latency;

      queue<MIL_ID> m_DisplayQueue;

      bool          m_DisplayOverlayEnable;
      queue<MIL_ID> m_DisplayOverlayQueue;
      LPDIRECT3DSURFACE9 m_DisplayOverlayLastBuffer;

      CRITICAL_SECTION m_CSLock;

      bool       m_EnableD3DEffect;
      D3D_EFFECT m_D3DEffect;
      bool       m_DrawDisplayInfo;

      bool m_Exit;
   };




