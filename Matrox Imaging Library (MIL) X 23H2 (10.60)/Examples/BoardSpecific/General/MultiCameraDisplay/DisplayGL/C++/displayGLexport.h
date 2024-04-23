/************************************************************************
*
* File name    :  displayGLexport.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*************************************************************************/

#ifndef __displayexport_h__
#define __displayexport_h__
#if !M_MIL_USE_LINUX
#ifdef DISPLAY_LIBRARY_EXPORTS
#define DISPLAYAPI __declspec(dllexport)
#else
#define DISPLAYAPI __declspec(dllimport)
#endif
#else
#define DISPLAYAPI
#endif

#define PFNC_Mono8                               0x01080001 /* Monochrome 8-bit */
#define PFNC_YUV422_8                            0x02100032 /* YUV 4:2:2 8-bit */
#define PFNC_YCbCr422_10p                        0x02140087 /* YCbCr 4:2:2 10-bit packed */
#define PFNC_RGB8_Planar                         0x02180021 /* Red-Green-Blue 8-bit planar */
#define PFNC_BGRa8                               0x02200017 /* Blue-Green-Red-alpha 8-bit */
#define PFNC_BGRa10p                             0x0228004D /* Blue-Green-Red-alpha 10-bit packed */
#define PFNC_YCbCr411_8                          0x020C005A /* YCbCr 4:1:1 8-bit */

enum PIXEL_FORMAT { eMono8 = PFNC_Mono8, eYUV422 = PFNC_YUV422_8, eYUV422_10p = PFNC_YCbCr422_10p, eRGB24_planar = PFNC_RGB8_Planar, eBGR32 = PFNC_BGRa8, eBGRa10p = PFNC_BGRa10p, eYUV411_8p = PFNC_YCbCr411_8};
enum PIXEL_COLOR_SPACE { eCSC_FULL = 0, eCSC_ITU_601, eCSC_ITU_709, eCSC_ITU_2020 };
enum TILE_PATTERN { eAUTO, eNEXTPATTERN, eONLY_MAIN, eSIDE_BY_SIDE_BOTTOM, eSIDE_BY_SIDE_MOSAIC, eCUSTOM,  eLAST };
enum RENDER_SOURCE { eRenderFromThread, eRenderFromGrabCallBack };

struct IMilDisplayEx
{
typedef unsigned int GLuint;

   public:

      // Delete this object.
      virtual void Release() = 0;

      // Buffer functions.
      virtual int BufAlloc(int sizeX, int sizeY, PIXEL_FORMAT pixelFormat, int *pitchbyte, void **pOutHostAddress) = 0;
      virtual int BufCreate(int sizeX, int sizeY, PIXEL_FORMAT pixelFormat, int pitchByte, void **pInHostAddress) = 0;
      virtual void BufSetColorSpace(int bufId, PIXEL_COLOR_SPACE pixelCSC) = 0;
      virtual void BufFree(int bufId) = 0;

      // tile functions.
      virtual int TileAlloc(int iSizeX, int iSizeY) = 0;
      virtual void TileFree(int tileId) = 0;
      virtual void TileIdentificationString(int tileIdx, const char *identificationString) = 0;
      virtual void SetText(int tileIdx, const char * text, int posX, int posY) = 0;
      virtual void SetTile(int tileIdx, bool visible, bool setAsMainTile, const char *text, int _tileStartPosX, int _tileStartPosY, int _tileSizeX, int _tileSizeY) = 0;
      virtual void GetTile(int tileIdx, bool *pvisible, bool *pisMainTile, char *ptextTitle, int textSize, int *ptileStartPosX, int *ptileStartPosY, int *ptileSizeX, int *ptileSizeY) = 0;

      virtual void RearrangeTiles(TILE_PATTERN tp = eNEXTPATTERN) = 0;

      // display functions

      // Update the buffer id on the tileId. Returns true if this tile is the main display.
      virtual bool UpdateDisplay(int tileId, int bufId, double grab_hw_timestamp_insec) = 0;

      virtual void OpenWindow() = 0;
      virtual void CloseWindow() = 0;
      virtual int GetMonitorCount() = 0;
      virtual const char *GetMonitorName(int index) = 0;
      virtual bool SetWindowMonitor(int monitorIndex) = 0;
      virtual bool IsWindowClosing() = 0;
      virtual void SetScalingFitToWindow(bool FitToWindow) = 0;
      virtual bool GetScalingFitToWindow() = 0;

      virtual bool Render() = 0;
      virtual bool PollEvents() = 0;

      virtual void SetRenderSource(RENDER_SOURCE renderSource) = 0;
      virtual RENDER_SOURCE GetRenderSource() = 0;

      virtual bool isAllocBufferSupported() = 0;
      virtual bool isCreateBufferSupported() = 0;

      virtual bool LatenciesGet(const char **iLatencySrc, const char **iLatencyDest, double &curavgLatency_ms,
         int &latencyFrameCount, int &dropFrames, double &avgLatency_ms) = 0;
};


#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus
#if !M_MIL_USE_LINUX
EXTERN_C   DISPLAYAPI IMilDisplayEx*  __stdcall GetMilDisplayEx(const char *title, int sizeX, int sizeY);
EXTERN_C   DISPLAYAPI void  __stdcall ReleaseMilDisplayEx(IMilDisplayEx* handle);
#else
EXTERN_C  IMilDisplayEx* GetMilDisplayEx(const char *title, int sizeX, int sizeY);
EXTERN_C  void  ReleaseMilDisplayEx(IMilDisplayEx* handle);
#endif





#endif
