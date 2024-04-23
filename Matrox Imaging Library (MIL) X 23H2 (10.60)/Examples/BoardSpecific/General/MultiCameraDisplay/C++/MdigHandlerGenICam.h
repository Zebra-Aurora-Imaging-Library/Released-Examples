/************************************************************************
*
* File name    :  MdigHandlerGenICam.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*************************************************************************/

#ifndef __mdighandlergenicam_h__
#define __mdighandlergenicam_h__

class CMILHandlerGenICam : public CMILDigitizerHandler
   {
   public:
      CMILHandlerGenICam(MIL_ID MilSystemId, MIL_INT DevNum) : CMILDigitizerHandler(MilSystemId, DevNum)
         {
         }
      virtual ~CMILHandlerGenICam() {};

      virtual const MIL_STRING& GetInputDescriptionBrief()
         {
         if (_inputDescriptionBrief.size() == 0)
            MdigInquire(_milDigitizerId, M_CAMERA_MODEL, _inputDescriptionBrief);

         return _inputDescriptionBrief;
         }
   };

#endif
