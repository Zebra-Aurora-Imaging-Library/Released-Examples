/***************************************************************************************/
/*
* File name: MdigHandler.cpp
*
* Synopsis: The digitizer handler is used to manage MIL digitizers, buffers and
*           displays.
*           It handles the start and stop of the grab using MdigProcess.
*           The grabbed buffers are sent to the associated display in the callback function.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include <iomanip>
#include <algorithm>

using namespace std;
#include "MdigHandler.h"

// utility function to round up an integer to the next multiple.
template<typename T>
T roundUp(T numToRound, size_t multiple)
   {
   if (multiple == 0)
      return numToRound;

   T remainder = numToRound % multiple;
   if (remainder == 0)
      return numToRound;

   return numToRound + T(multiple) - remainder;
   }

#if M_MIL_UNICODE_API
#include <codecvt>
#endif

// wrapper function to convert std::string to MIL_STRING.
MIL_STRING str2Mstr(const std::string& str)
{
#if M_MIL_UNICODE_API
   std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.from_bytes(str);
#else
	return str;
#endif
}

// wrapper function to convert MIL_STRING to std::string.
std::string Mstr2str(const MIL_STRING& milstr)
{
#if M_MIL_UNICODE_API
   std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
	return converterX.to_bytes(milstr);
#else
	return milstr;
#endif
}

// Function to allocate a MIL digitizer with the default DCF.
// returns true on success.
bool CMILDigitizerHandler::DigAlloc()
   {
   _pixelFormatString.clear();

   MdigAlloc(_milSystemId, _digDevNum, GetDCFName().c_str(), M_DEFAULT, &_milDigitizerId);
   if (_milDigitizerId)
      {
      MdigInquire(_milDigitizerId, M_SIZE_BAND, &_sizeBand);
      MdigInquire(_milDigitizerId, M_SIZE_X, &_sizeX);
      MdigInquire(_milDigitizerId, M_SIZE_Y, &_sizeY);

      if (MdigInquire(_milDigitizerId, M_CAMERA_PRESENT, M_NULL) == M_NO)
         {
         MdigFree(_milDigitizerId);
         _milDigitizerId = M_NULL;
         _inputDescription.clear();
         _inputDescriptionBrief.clear();
         }

      // Disable MdigProcess grab monitor since disconnecting a camera will result in an
      // error message.
      MdigControl(_milDigitizerId, M_PROCESS_GRAB_MONITOR, M_DISABLE);

      if (_pdisplay && (_tileId == 0))
         {
         _tileId = _pdisplay->TileAlloc(int(_sizeX), int(_sizeY));
         std::stringstream tilestr;
         tilestr << GetInputDescriptionBrief().c_str();
         _pdisplay->TileIdentificationString(_tileId, tilestr.str().c_str());
         }
      }

   return  (_milDigitizerId ? true : false);
   }

// Function to free an allocated digitizer with all the associated buffers.
void CMILDigitizerHandler::DigFree()
   {
   _pixelFormatString.clear();

   // Free tiles.
   if (_pdisplay && _tileId)
      _pdisplay->TileFree(_tileId);
   _tileId = 0;

   if (_milDigitizerId)
      {
      if (_isGrabbing)
         StopGrab();
      MdigControl(_milDigitizerId, M_GC_FEATURE_BROWSER, M_CLOSE);
      MdigFree(_milDigitizerId);
      }

   // Free buffers.
   FreeBuffers();

   _inputDescription.clear();
   _inputDescriptionBrief.clear();
   _milDigitizerId = M_NULL;
   _pdisplay = NULL;
   }

// Function to allocate a MIL buffer for direct display grabbing.
void CMILDigitizerHandler::AllocateBuffer(eBUFFER_MAPPING mapping, PIXEL_FORMAT pixelFormat, MIL_INT sizeBand, MIL_INT dynamicSizeByte, MIL_ID &milBuffer, int &gpuBuffer)
   {
   MIL_INT64 Attribute = 0;
   MIL_INT sizeX = _sizeX;
   MIL_INT sizeY = _sizeY;

   // find equivalent MIL attribute from pixelFormat.
   switch (pixelFormat)
      {
         case eMono8: Attribute = 0; break;
         case eYUV422: Attribute = M_YUV16 + M_PACKED; break;
         case eYUV422_10p: Attribute = M_DYNAMIC; break;
         case eRGB24_planar: Attribute = M_RGB24 + M_PLANAR; break;
         case eBGR32: Attribute = M_BGR32 + M_PACKED; break;
         case eBGRa10p: Attribute = M_DYNAMIC; break;
         case eYUV411_8p: Attribute = M_DYNAMIC; break;
         default: break;
      }

   if ((Attribute & M_DYNAMIC) && dynamicSizeByte == 0)
      {
      MosPrintf(MIL_TEXT("Buffer allocation error when allocate DYNAMIC buffer on dig num %d. \n"), _digDevNum);
      return;
      }

   // MIL buffer mapped on GPU.
   if (mapping == eMIL_BUFFER_MAPPED_ON_A_GPU_BUFFER)
      {
      void *pHostAddress[3] = { NULL, NULL, NULL };
      int pitchByte = 0;

      gpuBuffer = _pdisplay->BufAlloc(int(sizeX), int(sizeY), pixelFormat, &pitchByte, pHostAddress);
      if (gpuBuffer)
         {
         // Map MIL buffer on GPU buffer.
         MbufCreateColor(_milSystemId, sizeBand, sizeX, sizeY, 8, M_IMAGE + M_PROC + M_GRAB + M_PAGED + Attribute, M_HOST_ADDRESS + M_PITCH_BYTE, pitchByte, pHostAddress, &milBuffer);
         if (milBuffer == M_NULL)
            {
            // Cannot map MIL buffer so free gpu buffer.
            _pdisplay->BufFree(gpuBuffer);
            gpuBuffer = 0;
            }
         }
      }
   else if (mapping == eGPU_BUFFER_MAPPED_ON_A_MIL_BUFFER)
      {
      // Allocate MIL buffer aligned on 4K memory because GPUs are more efficient this way.
      auto pitchPixel = sizeX;
      if (dynamicSizeByte)
         {
         MbufAlloc1d(_milSystemId, dynamicSizeByte, 8, M_IMAGE + M_GRAB + M_DYNAMIC, &milBuffer);
         }
      else
         {
         MIL_INT _4KAlignement = (M_ALIGNMENT_RESERVED_BITS & 0xA);

         MbufCreateColor(_milSystemId, sizeBand, sizeX, sizeY, 8, M_IMAGE + M_PROC + M_GRAB + Attribute, M_ALLOCATION + M_PITCH + _4KAlignement, pitchPixel, NULL, &milBuffer);
         if (milBuffer == M_NULL)
            {
            // if allocation failed... try again with pitch multiple of 128 (some GPUs are more restrictive then others).
            pitchPixel = MIL_INT(roundUp(sizeX, 128));
            MbufCreateColor(_milSystemId, sizeBand, sizeX, sizeY, 8, M_IMAGE + M_PROC + M_GRAB + Attribute, M_ALLOCATION + M_PITCH + _4KAlignement, pitchPixel, NULL, &milBuffer);
            }
         }

      // Now map a GPU buffer on top of the MIL buffer.
      if (milBuffer)
         {
         void *pHostAddress[3] = { NULL, NULL, NULL };
         MIL_INT pitchByte = 0;

         // Inquire host address
         MbufInquire(milBuffer, M_HOST_ADDRESS, pHostAddress);

         if (pixelFormat == eYUV411_8p)
            {
            // In 2 band YUV420 the second plane starts at the next 4k address.
            pitchByte = roundUp(int(_sizeX), 128);
            auto plane1address = ((MIL_INT64)pHostAddress[0]) + (pitchByte * sizeY);
            pHostAddress[1] = (void *)roundUp(plane1address, 0x1000);
            }

         // If the host address is null then we must get the address for each plane.
         if (pHostAddress[0] == NULL)
            for (int ii = 0; ii < sizeBand; ii++)
               {
               MIL_ID bandID;
               MIL_INT bandValue[3] = { M_RED, M_GREEN, M_BLUE };
               MbufChildColor(milBuffer, bandValue[ii], &bandID);
               MbufInquire(bandID, M_HOST_ADDRESS, &pHostAddress[ii]);
               MbufFree(bandID);
               }

         MbufInquire(milBuffer, M_PITCH_BYTE, &pitchByte);

         // now create the GPU buffer with our host address.
         gpuBuffer = _pdisplay->BufCreate(int(sizeX), int(sizeY), pixelFormat, int(pitchByte), pHostAddress);
         }
      }
   else if ((mapping == eMIL_BUFFER_HOST) || (mapping == eMIL_BUFFER_ON_BOARD))
      {
      bool isOnBoard = (mapping == eMIL_BUFFER_ON_BOARD) ? true : false;
      if (dynamicSizeByte)
         MbufAlloc1d(_milSystemId, dynamicSizeByte, 8, M_IMAGE + M_GRAB + M_DYNAMIC + (isOnBoard?M_ON_BOARD:0), &milBuffer);
      else
         MbufAllocColor(_milSystemId, sizeBand, sizeX, sizeY, 8, M_IMAGE + M_GRAB + Attribute + (isOnBoard ? M_ON_BOARD : M_PROC), &milBuffer);
      }
   }

// Function to allocate grab buffers with their associated display buffer.
// The buffers are either:
//  1: allocated by the GPU and then MIL buffers are mapped on them.
//  2: allocated by MIL and GPU buffers mapped on the MIL buffers.
//  3: allocated in frame grabber memory and copied in a GPU buffer (optimized for encoding).
void CMILDigitizerHandler::AllocateBuffers()
   {
   // Free allocated buffers (if any).
   FreeBuffers();

   // Alloc buffers.
   MIL_INT dynamicBufferSizeByte = 0;

   // sizeX must be multiple of 4;
   _sizeX = roundUp(int(_sizeX), 4);

   auto grabPixelFormat = _pixelFormat;
   // if the pixel format is not supported, change it!
      {
      auto pixelFormats = SupportedPixelFormats();
      auto ispixelFormatSupported = (std::find(pixelFormats.begin(), pixelFormats.end(), _pixelFormat) != pixelFormats.end());
      if (!ispixelFormatSupported)
         grabPixelFormat = *pixelFormats.begin();
      }

   auto sizeBand = _sizeBand;

   // If mono digitizer, force mono buffers.
   if (sizeBand == 1)
      grabPixelFormat = eMono8;

   if (grabPixelFormat == eMono8)
      sizeBand = 1;

   // Set M_PFNC_TARGET_FORMAT for PFNC buffers when calculate memory size.
   switch (grabPixelFormat)
      {
         case eYUV411_8p: // YUV420 8-bit packed.
            {
            MdigControl(GetDigId(), M_PFNC_TARGET_FORMAT, PFNC_YCbCr411_8);
            int pitchByte = roundUp(int(_sizeX), 128);
            dynamicBufferSizeByte = (pitchByte * _sizeY) + ((pitchByte *2) * (_sizeY / 2)) + 0x1000; // the uv plane must start on a multiple of 4k.
            }
            break;

         case eYUV422_10p: // YUV422 10-bit packed.
            {
            MdigControl(GetDigId(), M_PFNC_TARGET_FORMAT, PFNC_YCbCr422_10p);
            MIL_INT stride = ((_sizeX + 47) / 48) * 128;
            int pitchByte = roundUp(int(stride), 128);
            dynamicBufferSizeByte = pitchByte * _sizeY * 2;
            }
            break;

         case eBGRa10p: // BGR 10-bit packed.
            {
            MdigControl(GetDigId(), M_PFNC_TARGET_FORMAT, PFNC_BGRa10p);
            int pitchByte = roundUp(int(_sizeX), 128);
            dynamicBufferSizeByte = pitchByte * _sizeY * 4;
            }
            break;
      }

   int bufSize = 0;
   if (IsEncoding())
      bufSize = _bufferingSizeWhenEncoding;
   else
      bufSize = _bufferingSizeWhenGrabbing;

   // Allocate the buffers for direct grabbing on display, processing and encoding.
   for (int i = 0; i < bufSize; i++)
      {
      BUFFER buf;
      int gpuBuffer = 0;
      MIL_ID milBuffer = M_NULL;

      // case 1: For PFNC (M_DYNAMIC) buffers, we must first allocate the MIL buffer and then map the GPU buffer on it.
      if (dynamicBufferSizeByte)
         AllocateBuffer(eGPU_BUFFER_MAPPED_ON_A_MIL_BUFFER, grabPixelFormat, 1, dynamicBufferSizeByte, milBuffer, gpuBuffer);
      else
         {
         // Does the grabber support grabbing in paged memory? If yes the buffers are allocated in GPU memory and we create a MIL buffer on them.
         // case 2: MIL buffers mapped over GPU.
         if (IsGrabInPagedMemorySupported() && _pdisplay->isAllocBufferSupported())
            AllocateBuffer(eMIL_BUFFER_MAPPED_ON_A_GPU_BUFFER, grabPixelFormat, sizeBand, 0, milBuffer, gpuBuffer);

         // case 3: If previous case failed then try allocating GPU buffers mapped over MIL.
         if (milBuffer == M_NULL)
            AllocateBuffer(eGPU_BUFFER_MAPPED_ON_A_MIL_BUFFER, grabPixelFormat, sizeBand, 0, milBuffer, gpuBuffer);
         }

      // If the buffer was allocated, allocate another one to be used when we activate the processing or the encoding.
      if (milBuffer)
         {
         buf.dispid = gpuBuffer;
         buf.tileId = (int)_digDevNum;
         buf.pixelFormat = grabPixelFormat;
         buf.milGrabBufferForProcessing = M_NULL;
         buf.milGrabBufferMappedOnDisplay = milBuffer;
         MbufClear(buf.milGrabBufferMappedOnDisplay, M_COLOR_DARK_BLUE);

         // Allocate a grab buffer to be used for processing (planar is the most efficient format for processing).
         MbufAllocColor(_milSystemId, sizeBand, _sizeX, _sizeY, 8, M_IMAGE + M_GRAB + M_PROC, &buf.milGrabBufferForProcessing);
         MbufClear(buf.milGrabBufferForProcessing, M_COLOR_DARK_BLUE);

         // Allocate a grab buffer to be used when encoding.
         if (IsEncoding())
            {
            int gpuBufferNotUsed;
            if(_seqHandler.isH264Board())
               AllocateBuffer(eMIL_BUFFER_ON_BOARD, grabPixelFormat, sizeBand, dynamicBufferSizeByte, buf.milGrabBufferForEncoding, gpuBufferNotUsed);
            else
               AllocateBuffer(eMIL_BUFFER_HOST, grabPixelFormat, sizeBand, dynamicBufferSizeByte, buf.milGrabBufferForEncoding, gpuBufferNotUsed);
            }

         _allocatedBuffers.push_back(buf);
         }
      else
         {
         MosPrintf(MIL_TEXT("Buffer allocation error on dig num %d.\n"), _digDevNum);
         break;
         }
      }
   }

// Set the display to be used for the grabbed images.
void CMILDigitizerHandler::SetDisplay(IMilDisplayEx *pdispOpenGL)
   {
   // Free current tile if display changes.
   if (_pdisplay && _tileId && _pdisplay != pdispOpenGL)
      {
      _pdisplay->TileFree(_tileId);
      _tileId = 0;
      }

   _pdisplay = pdispOpenGL;
   if (_pdisplay && _tileId == 0)
      {
      _tileId = _pdisplay->TileAlloc(int(_sizeX), int(_sizeY));
      std::stringstream tilestr;
      tilestr << GetInputDescriptionBrief().c_str();
      _pdisplay->TileIdentificationString(_tileId, tilestr.str().c_str());

      SetOverlayText(GetInputDescriptionBrief());
      }
   }

// The functions frees all the grab and display buffers.
void CMILDigitizerHandler::FreeBuffers()
   {
   // First stop the grab.
   if (_isGrabbing)
      StopGrab();

   // Free buffers;
   for (auto iterBuf = _allocatedBuffers.begin(); iterBuf != _allocatedBuffers.end(); ++iterBuf)
      {
      if (_pdisplay)
         _pdisplay->BufFree(iterBuf->dispid);
      if (iterBuf->milGrabBufferForProcessing)
         MbufFree(iterBuf->milGrabBufferForProcessing);
      if (iterBuf->milGrabBufferForEncoding)
         MbufFree(iterBuf->milGrabBufferForEncoding);
      MbufFree(iterBuf->milGrabBufferMappedOnDisplay);
      }
   _allocatedBuffers.clear();
   }

// The function start the grab using MdigProcess.
void CMILDigitizerHandler::StartGrab()
   {
   _pixelFormatString.clear();
   _milDigProcessBuffers.clear();
   _frameCountTotal = 0;
   _frameRateCurrent = 0.0;
   _startTime = 0;
   _skipNextDisplay = false;

   // Cannot grab if dig is not allocated.
   if ((_milDigitizerId == M_NULL) || (_pdisplay == nullptr))
      return;

   // if buffers are not allocated then allocate them.
   if (_allocatedBuffers.size() == 0)
      AllocateBuffers();

   if (_allocatedBuffers.size())
      {
      _milDigProcessBuffers.clear();
      _milDigProcessBufferMap.clear();

      for (auto iterBuf = _allocatedBuffers.begin(); iterBuf != _allocatedBuffers.end(); ++iterBuf)
         {
         MIL_ID grabBuffer = M_NULL;

         // If we do not have a processing or encoding buffer, disable them.
         if (iterBuf->milGrabBufferForProcessing == M_NULL)
            _processing = false;
         if(iterBuf->milGrabBufferForEncoding == M_NULL)
            _encoding = false;

         if (MbufInquire(iterBuf->milGrabBufferMappedOnDisplay, M_EXTENDED_ATTRIBUTE, M_NULL) & M_DYNAMIC)
            _processing = false; // cannot process in a M_DYNAMIC buffer.

         if (IsEncoding())
            grabBuffer = iterBuf->milGrabBufferForEncoding;
         else if (IsProcessing())
            grabBuffer = iterBuf->milGrabBufferForProcessing;
         else
            grabBuffer = iterBuf->milGrabBufferMappedOnDisplay;

         _milDigProcessBuffers.push_back(grabBuffer);

         // this map is used to retrieve the buffer structure in the dig process callback.
#if !M_MIL_USE_LINUX         
         _milDigProcessBufferMap[grabBuffer] = iterBuf._Ptr;
#else
         _milDigProcessBufferMap[grabBuffer]= iterBuf.base();
#endif
         }

      // Start encoding engine.
      if (IsEncoding())
         {
         MIL_DOUBLE frameRate;
         MdigInquire(_milDigitizerId, M_SELECTED_FRAME_RATE, &frameRate);
         _seqHandler.SetFrameRate(frameRate);
         _seqHandler.Start(GetInputDescription(), _milDigProcessBuffers[0]);
         }

      _isGrabbing = true;
      MdigProcess(_milDigitizerId, &_milDigProcessBuffers[0], MIL_INT(_milDigProcessBuffers.size()),
         M_START, M_DEFAULT, MILGrabCallbackFunction, this);
      }
   }

// This function stops the grab.
void CMILDigitizerHandler::StopGrab()
   {
   _isGrabbing = false;

   if (_milDigProcessBuffers.size())
      MdigProcess(_milDigitizerId, &_milDigProcessBuffers[0], MIL_INT(_milDigProcessBuffers.size()),
         M_STOP, M_DEFAULT, MILGrabCallbackFunction, this);
   
   // Stop encoding engine (it is ok to stop it even if it was not started).
   _seqHandler.Stop();
   
   _milDigProcessBuffers.clear();
   }

// This function sets text on the overlay of the tile of the digitizer.
void CMILDigitizerHandler::SetOverlayText(const MIL_STRING& itext)
   {
   if (_tileId)
      _pdisplay->SetText(_tileId, Mstr2str(itext).c_str(), 10, 18);
   }

// This function returns the text on the overlay of the tile of the digitizer.
MIL_STRING CMILDigitizerHandler::GetOverlayText() const
   {

   char text[255] = { 0 };
   _pdisplay->GetTile(_tileId, NULL, NULL, text, sizeof(text), NULL, NULL, NULL, NULL);
   return str2Mstr(text);
   }

// The function returns a MIL_STRING containing the number of grabbed frames with the frame rate.
const MIL_STRING& CMILDigitizerHandler::GetGrabStats()
   {
   if (_allocatedBuffers.size() == 0)
      {
      _statText = MIL_TEXT("Not enough memory to allocate grab buffers.");
      }
   else
      {
      MIL_STRING_STREAM ss;
      ss << _frameCountTotal << MIL_TEXT(" frames at ") << std::setprecision(4) << _frameRateCurrent << MIL_TEXT(" fps. ");
      _statText = ss.str();
      }
   return _statText;
   }

// This function updates a MIL_STRING with the buffer format and color space.
// The color space of YUV buffers are inquired after a grab is done
// because the camera sets it.
void CMILDigitizerHandler::UpdateBufferPixelFormat(BUFFER &buf)
   {
   // We can only inquire the buffer pixel once the grab is started. The color space is set on the first grab.
   if (_pixelFormatString.size() == 0 && _frameCountTotal > 0)
      {
      MIL_ID bufId = buf.milGrabBufferMappedOnDisplay;
      MIL_INT64 Format = 0;

      // Get pixel format name.

      _pixelFormatString = str2Mstr(GetPixelFormatName((PfncFormat)buf.pixelFormat));
      _bufferColorSpaceFormat = eCSC_FULL;

      // Inquire color space of YUV16 buffers (this is set after grab is done).
      MbufInquire(bufId, M_EXTENDED_FORMAT, &Format);
      if (M_IS_FORMAT_YUV(Format) || buf.pixelFormat == PFNC_YCbCr422_10p || buf.pixelFormat == PFNC_YCbCr411_8)
         {
         MIL_INT CbCRRange = 0;
         MbufInquire(bufId, M_YCBCR_RANGE, &CbCRRange);

         MIL_STRING_STREAM ss;
         if (CbCRRange == M_YCBCR_SD)
            {
            _bufferColorSpaceFormat = eCSC_ITU_601;
            ss << _pixelFormatString << MIL_TEXT(" ITU-601");
            }
         else if (CbCRRange == M_YCBCR_HD)
            {
            _bufferColorSpaceFormat = eCSC_ITU_709;
            ss << _pixelFormatString << MIL_TEXT(" ITU-709");
            }
         else if (CbCRRange == M_YCBCR_UHD) // UHD
            {
            _bufferColorSpaceFormat = eCSC_ITU_2020;
            ss << _pixelFormatString << MIL_TEXT(" ITU-2020");
            }
         else
            {
            ss << _pixelFormatString;
            _bufferColorSpaceFormat = eCSC_FULL;
            }

         _pixelFormatString = ss.str();

         // now set the color space in the display buffer.
         for (auto iterBuffer = _allocatedBuffers.begin(); iterBuffer != _allocatedBuffers.end(); ++iterBuffer)
            _pdisplay->BufSetColorSpace(iterBuffer->dispid, _bufferColorSpaceFormat);
         }
      }
   return;
   }

// Stops the grab, reallocate all the buffers and restart the grab.
void CMILDigitizerHandler::RestartGrab()
   {
   // get current tile position on display
   bool tile_isMainTile = false;
   int  tile_posX, tile_posY, tile_sizeX, tile_sizeY;
   tile_posX = tile_posY = tile_sizeX = tile_sizeY = 0;

   auto currentRenderSource = _pdisplay->GetRenderSource();
   if (currentRenderSource == eRenderFromGrabCallBack)
      _pdisplay->SetRenderSource(eRenderFromThread);

   if (_tileId)
      _pdisplay->GetTile(_tileId, NULL, &tile_isMainTile, NULL, 0, &tile_posX, &tile_posY, &tile_sizeX, &tile_sizeY);

   StopGrab();
   FreeBuffers();
   AllocateBuffers();

   if (_tileId)
      _pdisplay->SetTile(_tileId, true, tile_isMainTile, NULL, tile_posX, tile_posY, tile_sizeX, tile_sizeY);
   StartGrab();

   if (currentRenderSource == eRenderFromGrabCallBack)
      _pdisplay->SetRenderSource(eRenderFromGrabCallBack);
   }

// Changes the grab pixel format. If a grab is in progress, it will stop-it, reallocate the buffers and restart grabbing.
void CMILDigitizerHandler::SetPixelFormat(PIXEL_FORMAT pixelFormat)
   {
   _pixelFormat = pixelFormat;
   RestartGrab();
   }

// Activates image processing.
void CMILDigitizerHandler::SetProcessing(bool activate)
   {
   _processing = activate;
   RestartGrab();
   }

// Activates encoding.
void CMILDigitizerHandler::SetEncoding(bool activate)
   {
   _encoding = activate;
   RestartGrab();
   }

// Returns a MIL_STRING containing a description of camera.
const MIL_STRING& CMILDigitizerHandler::GetInputDescription()
   {
   if (_milDigitizerId && _inputDescription.size() == 0)
      {
      MIL_INT sizeX;
      MIL_INT sizeY;
      MIL_INT scanMode;
      MIL_DOUBLE FrameRate;

      auto brief = GetInputDescriptionBrief();

      MdigInquire(_milDigitizerId, M_SIZE_X, &sizeX);
      MdigInquire(_milDigitizerId, M_SIZE_Y, &sizeY);
      MdigInquire(_milDigitizerId, M_SCAN_MODE, &scanMode);
      MdigInquire(_milDigitizerId, M_SELECTED_FRAME_RATE, &FrameRate);
      FrameRate += 0.01;

      MIL_STRING_STREAM ss;
      ss << GetInputDescriptionBrief() << MIL_TEXT(" ") << sizeX << MIL_TEXT("x") << sizeY << (scanMode == M_INTERLACE ? MIL_TEXT("i") : MIL_TEXT("p")) << std::setprecision(4) << FrameRate;
      _inputDescription = ss.str();
      }
   return _inputDescription;
   }


// MdigProcess call back function.
// This function is called at each grabbed frame.
// It is responsible to update the display and encode the image if needed.
MIL_INT MFTYPE CMILDigitizerHandler::MILGrabCallbackFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   CMILDigitizerHandler *pThis = (CMILDigitizerHandler *)HookDataPtr;
   pThis->GrabCallbackFunction(HookType, HookId);
   return 0;
   }
void CMILDigitizerHandler::GrabCallbackFunction(MIL_INT HookType, MIL_ID HookId)
   {
   if (HookType == M_MODIFIED_BUFFER)
      {
      // If the grab is stopping then do not update the display.
      if (!_isGrabbing)
         return;

      // Retrieve the MIL_ID of the grabbed buffer.
      MIL_ID ModifiedBufferId;
      MIL_DOUBLE grab_hw_timestamp = 0.0;
      MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);
      MdigGetHookInfo(HookId, M_TIME_STAMP, &grab_hw_timestamp);
      if (ModifiedBufferId)
         {
         // calculate frameRate at every 120 frames.
         const auto frameRateCount = 120;
         if (_frameCountTotal % frameRateCount == 0)
            {
            if (_startTime)
               {
               auto deltaTime = grab_hw_timestamp - _startTime;
               _frameRateCurrent = frameRateCount / deltaTime;
               }
            _startTime = grab_hw_timestamp;
            }

         // Get the buf structure containing all the buffer information.
         auto buf_iter = _milDigProcessBufferMap.find(ModifiedBufferId);
         if (buf_iter != _milDigProcessBufferMap.end())
            {
            auto &buf = *buf_iter->second;

            // If we have an associated display buffer, update the display.
            if (buf.dispid)
               {
               // Get and update the pixel format and color-space on the first grabbed frame (the color-space is not knowned before).
               if (_frameCountTotal == 1)
                  UpdateBufferPixelFormat(buf);

               _frameCountTotal++;
               if (!_skipNextDisplay)
                  {
                  // Perform image process if activated. 
                  // When doing image processing instead of grabbing directly on the display buffer, we grab in a MIL buffer then
                  // the destination of the processing is the display buffer.
                  if (IsProcessing())
                     MimArith(ModifiedBufferId, M_NULL, buf.milGrabBufferMappedOnDisplay, M_NOT);
                  else if (IsEncoding())
                     MbufCopy(ModifiedBufferId, buf.milGrabBufferMappedOnDisplay);

                  _pdisplay->UpdateDisplay(_tileId, buf.dispid, grab_hw_timestamp);
                  }
               // is encoding activated?
               if (IsEncoding() && _frameCountTotal > 30)
                  _seqHandler.Feed(ModifiedBufferId);
               }
            }

          //To reduce latency we need to drop frames when the internal buffering is growing.
          //The internal buffering is the difference between the total number of buffers and
          //the number of pending buffers.
         if (_skipNextDisplay == false)
            {
            MIL_INT buffering_size_total = MdigInquire(_milDigitizerId, M_PROCESS_TOTAL_BUFFER_NUM, M_NULL);
            MIL_INT buffering_size = MdigInquire(_milDigitizerId, M_PROCESS_PENDING_GRAB_NUM, M_NULL);

            MIL_INT currentGrabbedFrames = buffering_size_total - buffering_size;
            if (currentGrabbedFrames > getFrameBufferingLatency())
               _skipNextDisplay = true;
            }
         else
            _skipNextDisplay = false;
         }
      }
   return;
   }
