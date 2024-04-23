/************************************************************************
*
* File name    :  MdigHandlerClarityUHD.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*************************************************************************/

#ifndef __mdighandlerclarityuhd_h__
#define __mdighandlerclarityuhd_h__

class CMILClarityHandler : public CMILDigitizerHandler
   {
   public:
      CMILClarityHandler(MIL_ID MilSystemId, MIL_INT DevNum) : CMILDigitizerHandler(MilSystemId, DevNum)
         {
         _boardType = 0;
         MsysInquire(MilSystemId, M_BOARD_TYPE, &_boardType);
         _pixelFormat = eYUV411_8p;
         }
      virtual ~CMILClarityHandler() {};

      virtual MIL_STRING GetDCFName() const { return MIL_STRING(MIL_TEXT("AutoDetect.dcf")); }

      virtual bool IsGrabInPagedMemorySupported() { return true; }

      virtual std::list<PIXEL_FORMAT> SupportedPixelFormats() const
         {
         if (_encoding)
            {
            return _seqHandler.SupportedPixelFormats();
            }
         else
            {
            static const PIXEL_FORMAT pf[] = { eMono8, eYUV422, eYUV422_10p, eYUV411_8p, eRGB24_planar, eBGR32, eBGRa10p };
            std::list<PIXEL_FORMAT> pixelFormats(pf, pf + sizeof(pf) / sizeof(pf[0]));
            return pixelFormats;
            }
         }

      virtual const MIL_STRING& GetInputDescriptionBrief()
         {
         if (_inputDescriptionBrief.size() == 0)
            {
            bool isAnalog = false;
            if (MdigInquire(GetDigId(), M_INPUT_MODE, M_NULL) == M_ANALOG)
               isAnalog = true;
            switch (_digDevNum)
               {
                  case M_DEV0: _inputDescriptionBrief = MIL_TEXT("HDMI0"); break;
                  case M_DEV1: _inputDescriptionBrief = MIL_TEXT("HDMI1"); break;
                  case M_DEV2: _inputDescriptionBrief = (isAnalog ? MIL_TEXT("ANGL0") : MIL_TEXT("HDMI2")); break;
                  case M_DEV3: _inputDescriptionBrief = (isAnalog ? MIL_TEXT("ANGL1") : MIL_TEXT("HDMI3")); break;
                  case M_DEV4: _inputDescriptionBrief = MIL_TEXT("DP0"); break;
                  case M_DEV5: _inputDescriptionBrief = MIL_TEXT("DP1"); break;
                  case M_DEV6: _inputDescriptionBrief = MIL_TEXT("SDI0"); break;
                  case M_DEV7: _inputDescriptionBrief = MIL_TEXT("SDI1"); break;
               }
            }
         return _inputDescriptionBrief;
         }

   protected:

      // Returns the frame grabber latency (in frame count). Some frame grabbers might require more then 1 frame period to grab the frame depending on the size and frame rate of the source.
      virtual MIL_INT getFrameBufferingLatency()
         {
         if (_allocatedBuffers.size())
            {
            // when grabbing UHD in BGR32 format, the framegrabber needs more then 1 frame time to return the frame.
            if (_sizeY == 2160)
               return 2;
            }
         return 1;
         }

      MIL_INT _boardType;
   };

#endif
