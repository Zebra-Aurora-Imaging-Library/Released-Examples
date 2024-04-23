/***************************************************************************************/
/*
* File name: MseqHandler.cpp
*
* Synopsis: The sequence handler is used to manage the encoding of streams.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include <ctime>

#include <list>
#include "MseqHandler.h"

CseqHandler::CseqHandler(MIL_ID milSystemId) :
   _milSystemId(milSystemId), _milSeqId(M_NULL)
   {
   _useAutoSettings = true;
   _frameRate = 60.0;
   _level = M_LEVEL_5_1;
   _gop = 90;
   _bitRate = 15000;
   _bitRateMax = 30000;
   _isH264Board = false;

   MIL_INT boardType = 0;
   MsysInquire(milSystemId, M_BOARD_TYPE, &boardType);
   if (boardType & M_H264)
      _isH264Board = true;
   }

CseqHandler::~CseqHandler()
   {
   if (_milSeqId)
      {
      auto seqId = _milSeqId;
      _milSeqId = M_NULL;
      MseqFree(seqId);
      }
   }

// returns the pixel formats that are supported by the encoder.
std::list<PIXEL_FORMAT> CseqHandler::SupportedPixelFormats() const
   {
   if (_isH264Board)
      {
      // In order or preference.
      static const PIXEL_FORMAT pf[] = { eYUV411_8p, eYUV422, eYUV422_10p, eRGB24_planar, eMono8 };
      std::list<PIXEL_FORMAT> pixelFormats(pf, pf + sizeof(pf) / sizeof(pf[0]));
      return pixelFormats;
      }
   else
      {
      static const PIXEL_FORMAT pf[] = { eRGB24_planar, eYUV422, eBGR32, eMono8};
      std::list<PIXEL_FORMAT> pixelFormats(pf, pf + sizeof(pf) / sizeof(pf[0]));
      return pixelFormats;
      }
   }

// Returns the most efficient buffer format 
MIL_INT64 CseqHandler::BestBufferFormat(PIXEL_FORMAT pixelFormat)
   {
   MIL_INT64 bestFormat = 0;
   if (_isH264Board)
      {
      if (pixelFormat == eMono8)
         bestFormat = M_ON_BOARD;
      else  if ((pixelFormat == eYUV422_10p) || (pixelFormat == eYUV411_8p))
         bestFormat = M_DYNAMIC + M_ON_BOARD;
      else
         bestFormat = M_YUV12 + M_PLANAR + M_ON_BOARD;
      }
   else if (pixelFormat == eMono8)
      bestFormat = 0;
   else
      bestFormat = M_RGB24 + M_PLANAR;

   return bestFormat;
   }

// Start the encoding sequence.
void CseqHandler::Start(MIL_STRING fileName, MIL_ID bufSampleId)
   {
   if (_milSeqId == M_NULL)
      MseqAlloc(_milSystemId, M_DEFAULT, M_SEQ_COMPRESS, M_DEFAULT, M_DEFAULT, &_milSeqId);

   Set(_useAutoSettings, _frameRate, _level, _gop, _bitRate, _bitRateMax);

   // get current time to put in file name.
   time_t rawtime;
   struct tm timeinfo;
   char buffer[128];

   time(&rawtime);
#if !M_MIL_USE_LINUX   
   localtime_s(&timeinfo, &rawtime);
#else
   localtime_r(&rawtime, &timeinfo);
#endif
   strftime(buffer, sizeof(buffer), "_%Y_%m_%d_%Hh%Mm%S", &timeinfo);
   MIL_STRING milcurtime = str2Mstr(buffer);

   _fileName = fileName + milcurtime + MIL_TEXT(".mp4");
   MseqDefine(_milSeqId, M_SEQ_OUTPUT(0) + M_SEQ_DEST(0), M_FILE, _fileName.c_str(), M_FILE_FORMAT_MP4);

   MseqControl(_milSeqId, M_CONTEXT, M_BUFFER_SAMPLE, bufSampleId);

   /* Start the encoding process, waits for buffer to be fed for encoding. */
   MseqProcess(_milSeqId, M_START, M_ASYNCHRONOUS);
   }

// Stop the encoding sequence.
void CseqHandler::Stop()
   {
   if (_milSeqId)
      {
      MseqProcess(_milSeqId, M_STOP, M_WAIT);
      MseqFree(_milSeqId);
      _milSeqId = M_NULL;
      }
   }

// Sets the encoding parameters. Must be done before the start.
void CseqHandler::Set(bool useAutoSettings, MIL_DOUBLE frameRate, MIL_INT level, MIL_INT gop, MIL_INT bitRate, MIL_INT bitRateMax)
   {
   _useAutoSettings = useAutoSettings;
   _frameRate = frameRate;
   if (useAutoSettings)
      {
      if (_milSeqId)
         {
         MseqControl(_milSeqId, M_CONTEXT, M_SETTING_AUTO_ADJUSTMENT, M_ENABLE);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_FRAME_RATE, _frameRate);
         }
      }
   else
      {
      _level = level;
      _gop = gop;
      _bitRate = bitRate;
      _bitRateMax = bitRateMax;
      if (_milSeqId)
         {
         MseqControl(_milSeqId, M_CONTEXT, M_SETTING_AUTO_ADJUSTMENT, M_DISABLE);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_PROFILE, M_DEFAULT);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_BIT_RATE_MODE, M_VARIABLE);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_GROUP_OF_PICTURE_SIZE, _gop);

         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_FRAME_RATE, _frameRate);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_FRAME_RATE_MODE, M_VARIABLE);

         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_LEVEL, _level);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_BIT_RATE_MAX, _bitRateMax);
         MseqControl(_milSeqId, M_CONTEXT, M_STREAM_BIT_RATE, _bitRate);
         }
      }
   }

// Feeds a buffer in the sequence. Must be done after the Start().
void CseqHandler::Feed(MIL_ID buffer)
   {
   if (_milSeqId)
      MseqFeed(_milSeqId, buffer, M_DEFAULT);
   }
