/************************************************************************
*
* File name    :  MdigHandler.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*************************************************************************/

#ifndef __mdighandler_h__
#define __mdighandler_h__

#include <vector>
#include <map>
#include <list>

#include "../DisplayGL/C++/displayGLexport.h"
#include "MseqHandler.h"

class CMILDigitizerHandler
   {
   public:
      CMILDigitizerHandler(MIL_ID MilSystemId, MIL_INT DevNum) :
         _milSystemId(MilSystemId), _digDevNum(DevNum), _milDigitizerId(M_NULL), _pdisplay(NULL),
         _seqHandler(MilSystemId),
        _pixelFormat(eYUV422)
         {
         _bufferColorSpaceFormat = eCSC_FULL;
         _isGrabbing = false;
         _bufferingSizeWhenGrabbing = 3;
         _bufferingSizeWhenEncoding = 6;
         _frameCountTotal = 0;
         _frameRateCurrent = 0.0;
         _processing = false;
         _directGrabInDisplay = false;
         _encoding = false;
         _tileId = 0;
         _skipNextDisplay = false;
         }

      virtual ~CMILDigitizerHandler()
         {
         DigFree();
         }

      // Allocation.
      bool DigAlloc();
      void DigFree();
      void AllocateBuffers();
      void FreeBuffers();

      virtual MIL_STRING GetDCFName() const { return MIL_STRING(MIL_TEXT("M_DEFAULT")); }

      // Supported options.
      virtual bool IsGrabInPagedMemorySupported() { return false; }

      // Display.
      void SetDisplay(IMilDisplayEx *pdispOpenGL);
      IMilDisplayEx *GetDisplay() const { return _pdisplay; }
      void SetOverlayText(const MIL_STRING& text);
      MIL_STRING GetOverlayText() const;

      // Get info.
      MIL_INT GetDevNum() const { return _digDevNum; }
      MIL_ID GetDigId() const { return _milDigitizerId; }
      MIL_ID GetSysId() const { return _milSystemId; }
      MIL_INT SizeX() const { return _sizeX; }
      MIL_INT SizeY() const { return _sizeY; }

      // Pixel formats
      void SetPixelFormat(PIXEL_FORMAT pixelFormat);
      PIXEL_FORMAT GetPixelFormat() const { return _pixelFormat; }
      MIL_STRING GetPixelFormatString() const { return _pixelFormatString; }
      virtual std::list<PIXEL_FORMAT> SupportedPixelFormats() const
         {
         if (_encoding)
            {
            return _seqHandler.SupportedPixelFormats();
            }
         else
            {
            static const PIXEL_FORMAT pf[] = { eMono8, eYUV422, eRGB24_planar, eBGR32 };
            std::list<PIXEL_FORMAT> pixelFormats(pf, pf + sizeof(pf) / sizeof(pf[0]));
            return pixelFormats;
            }
         }

      // Processing and encoding
      void SetProcessing(bool activate);
      const bool IsProcessing() const { return _processing; }
      void SetEncoding(bool activate);
      const bool IsEncoding() const { return _encoding; }

      // Grab
      void StartGrab();
      void StopGrab();
      void RestartGrab(); // stops, reallocate buffers and restart grab.
      bool IsGrabbing() { return _milDigProcessBuffers.size() > 0 ? true : false; }

      // Grab statistics
      virtual const MIL_STRING& GetInputDescription();
      virtual const MIL_STRING& GetInputDescriptionBrief()
         {
         MIL_STRING_STREAM mss;
         mss << MIL_STRING(MIL_TEXT("Camera")) << _digDevNum;
         _inputDescriptionBrief = mss.str();
         return _inputDescriptionBrief;
         }
      const MIL_STRING &GetGrabStats();

      // Grab call back function
      static MIL_INT MFTYPE MILGrabCallbackFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
      void GrabCallbackFunction(MIL_INT HookType, MIL_ID HookId);

      // for sorting
      bool operator < (const CMILDigitizerHandler& str) const
         {
         return (_inputDescriptionBrief < str._inputDescriptionBrief);
         }

   protected:

      typedef struct _BUFFER
         {
         _BUFFER()
            {
            milGrabBufferMappedOnDisplay = M_NULL;
            milGrabBufferForProcessing = M_NULL;
            milGrabBufferForEncoding = M_NULL;
            dispid = 0;
            tileId = 0;
            pixelFormat = eYUV422;
            }

         int tileId; // tile Id on the display
         int dispid; // Id of the display buffer on the display.
         PIXEL_FORMAT pixelFormat;

         // We have 2 possible grab buffers. If we are not processing, then we can grab directly in the display buffer which is milGrabBufferMappedOnDisplay.
         // If we are doing some processing, we grab in milGrabBufferForProcessing and the destination of the processing is the display buffer: milGrabBufferMappedOnDisplay.
         MIL_ID milGrabBufferMappedOnDisplay; // Id of the MIL buffer mapped on the display buffer, used when we grab directly on the display.
         MIL_ID milGrabBufferForProcessing; // Id of the MIL grab buffer used for processing operations.
         MIL_ID milGrabBufferForEncoding; // Id of the MIL grab buffer used for encoding operations.
         } BUFFER;

      // System, Digitizer and Encoding
      MIL_ID _milSystemId;
      MIL_ID _milDigitizerId;
      MIL_INT _digDevNum;
      MIL_INT _sizeBand, _sizeX, _sizeY;

      // Display
      IMilDisplayEx *_pdisplay;
      int  _tileId; // ID if the tile where the buffers are copied too.
      bool _skipNextDisplay; // skip the next display of the grabbed image to reduce latency.

      // Buffers
      int _bufferingSizeWhenGrabbing;
      int _bufferingSizeWhenEncoding;
      std::vector<BUFFER> _allocatedBuffers;
      std::map<MIL_ID, BUFFER *> _milDigProcessBufferMap;  // this map is used to retrieve the buffer structure in the dig process callback.
      std::vector<MIL_ID> _milDigProcessBuffers;
      PIXEL_FORMAT _pixelFormat; // contains the pixel format to be used to allocate the grab buffers.
      MIL_STRING _pixelFormatString;
      PIXEL_COLOR_SPACE _bufferColorSpaceFormat;

      enum eBUFFER_MAPPING { eGPU_BUFFER_MAPPED_ON_A_MIL_BUFFER, eMIL_BUFFER_MAPPED_ON_A_GPU_BUFFER, eMIL_BUFFER_HOST, eMIL_BUFFER_ON_BOARD };
      void AllocateBuffer(eBUFFER_MAPPING mapping, PIXEL_FORMAT pixelFormat, MIL_INT sizeBand, MIL_INT dynamicSizeByte, MIL_ID &milBuffer, int &gpuBuffer);
      void UpdateBufferPixelFormat(BUFFER &buf);

      // Grab
      // Returns the frame grabber latency (in frame count). Some frame grabbers might require more then 1 frame period to grab the frame depending on the size and frame rate of the source.
      virtual MIL_INT getFrameBufferingLatency() { return 1; }

      // Processing
      bool _directGrabInDisplay; // true when grabbing directly in the display buffer (false when processing activated).
      bool _processing; // processing to do when grabbing.
      bool _encoding; // activate encoding.

      // Encoding
      CseqHandler _seqHandler;

      // Statistics
      bool   _isGrabbing;
      MIL_DOUBLE _startTime;
      MIL_INT _frameCountTotal;
      MIL_DOUBLE _frameRateCurrent;
      MIL_STRING _statText;
      MIL_STRING _inputDescription;
      MIL_STRING _inputDescriptionBrief;

   };

#include "MdigHandlerClarityUHD.h"
#include "MdigHandlerGenICam.h"
class CmilDigitizerFactory
   {
   public:
      CMILDigitizerHandler *AllocateMILDigHandler(MIL_ID milsystem, MIL_INT devNum)
         {
         MIL_INT systemType = 0;

         try
            {
            MsysInquire(milsystem, M_SYSTEM_TYPE, &systemType);
            switch (systemType)
               {
                  case M_SYSTEM_GIGE_VISION_TYPE:
                  case M_SYSTEM_USB3_VISION_TYPE:
                  case M_SYSTEM_RADIENTCXP_TYPE:
                  case M_SYSTEM_RAPIXOCXP_TYPE:
                  case M_SYSTEM_GENTL_TYPE:
                     return new CMILHandlerGenICam(milsystem, M_DEV0 + devNum);
                  case M_SYSTEM_CLARITY_UHD_TYPE:
                     return new CMILClarityHandler(milsystem, M_DEV0 + devNum);
                  default:
                     return new CMILDigitizerHandler(milsystem, M_DEV0 + devNum);
               }
            }
         catch (std::bad_alloc)
            {
            return NULL;
            }
         }
   };

#endif
