/************************************************************************
*
* File name    :  MseqHandler.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*************************************************************************/

#ifndef __mseqhandler_h__
#define __mseqhandler_h__

#include "../DisplayGL/C++/displayGLexport.h"

// wrapper functions to convert MIL_STRING to std::string.
MIL_STRING str2Mstr(const std::string& str);
std::string Mstr2str(const MIL_STRING& milstr);

class CseqHandler
   {
   public:
      CseqHandler(MIL_ID MilSystemId);
      ~CseqHandler();

      void Start(MIL_STRING fileName, MIL_ID BufId);
      void Stop();

      void Set(bool useAutoSettings, MIL_DOUBLE frameRate, MIL_INT level, MIL_INT gop, MIL_INT bitRate, MIL_INT bitRateMax);
      void SetFrameRate(MIL_DOUBLE frameRate) { _frameRate = frameRate; }
      void Feed(MIL_ID buffer);

      const bool isH264Board() const { return _isH264Board; };
      std::list<PIXEL_FORMAT> SupportedPixelFormats() const;
      MIL_INT64 BestBufferFormat(PIXEL_FORMAT pixelFormat);

   private:
      MIL_ID _milSystemId;
      MIL_ID _milSeqId;
      bool _isH264Board;
      MIL_STRING _fileName;

      // H264 settings
      bool _useAutoSettings;
      MIL_DOUBLE _frameRate;
      MIL_INT _level;
      MIL_INT _gop;
      MIL_INT _bitRate;
      MIL_INT _bitRateMax;
   };


#endif
