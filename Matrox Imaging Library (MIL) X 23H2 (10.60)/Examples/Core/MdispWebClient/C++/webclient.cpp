/***************************************************************************************/
/*
 * File name: MWebClient.cpp
 *
 * Synopsis:  This program shows how to use web publishing.
 *
 *
 *
 * Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include "webclient.h"
static bool sDisConnect = false;
 /* Disconnect hook handler*/
MIL_INT MFTYPE DisconnectHookHandler(MIL_INT /*HookType*/, MIL_ID /*EventId*/, void* /*UserData*/)
   {
   sDisConnect = true;
   return M_NULL;
   }

/* Update hook handler*/
/* Get object data, call user function */
MIL_INT MFTYPE UpdateHookHandler(MIL_INT /*HookType*/, MIL_ID EventId,void* UserData)
   {
   MIL_ID ObjId = M_NULL;
   MilWeb::MobjGetHookInfo(EventId, M_OBJECT_ID, &ObjId);
   if(ObjId)
      {
      MIL_INT64 ObjectType;
      MilWeb::MobjInquire(ObjId,M_OBJECT_TYPE,&ObjectType);
      if(ObjectType == M_MESSAGE_MAILBOX)
         {
         MIL_INT MsgLength = M_NULL;
         MilWeb::MobjInquire(ObjId, M_MESSAGE_LENGTH, &MsgLength);
         if(MsgLength > 0 )
            {
            MIL_UINT8* MsgData = new MIL_UINT8[(size_t)MsgLength];
            MIL_INT64  MsgTag;
            MIL_INT64  MsgStatus;
            memset(MsgData, 0, (size_t)MsgLength);
            // Got message image data
            MilWeb::MobjMessageRead(ObjId, MsgData, MsgLength,NULL,&MsgTag, &MsgStatus,M_DEFAULT);
            DisplayMessage(MsgData, (MIL_INT)MsgLength, MsgTag, UserData);
            delete [] MsgData;
            }
         }
      else if (ObjectType == M_DISPLAY)
         {
         MIL_INT SizeByte = M_NULL;
         MilWeb::MdispInquire(ObjId, M_SIZE_BYTE, &SizeByte);
         MIL_UINT8 *Data = M_NULL;
         MIL_INT SizeX = 0, SizeY = 0, PitchByte = 0;
         MilWeb::MdispInquire(ObjId, M_SIZE_X, &SizeX);
         MilWeb::MdispInquire(ObjId, M_SIZE_Y, &SizeY);
         MilWeb::MdispInquire(ObjId, M_PITCH_BYTE, &PitchByte);
         MilWeb::MdispInquire(ObjId,  M_IMAGE_HOST_ADDRESS, &Data);
         if(SizeX > 0 && SizeY > 0 && PitchByte > 0 && Data)
            {
            // Got display image data
            DisplayImage(Data, SizeByte, SizeX, SizeY, PitchByte, UserData);
            }
         }
      }
   return M_NULL;
   }


/* Open new connection */
/* and connect to published objects */
MIL_ID StartConnection(void *UserData, MIL_CONST_TEXT_PTR Url)
   {
   MIL_ID AppId  = M_NULL;
   MIL_ID DispId = M_NULL;
   MIL_ID MsgId  = M_NULL;
      
   MilWeb::MappOpenConnection(Url, M_DEFAULT, M_DEFAULT, &AppId);  
   if(AppId)
      { 
      //MilWeb::MappControl(AppId, M_ERROR, M_PRINT_DISABLE);
      MilWeb::MappInquireConnection(AppId, M_WEB_PUBLISHED_NAME, MIL_TEXT("Message"), M_DEFAULT, &MsgId);
      MilWeb::MappInquireConnection(AppId, M_WEB_PUBLISHED_NAME, MIL_TEXT("Display"), M_DEFAULT, &DispId);
      MilWeb::MappHookFunction(AppId, M_DISCONNECT, DisconnectHookHandler, NULL);
      MilWeb::MdispControl(DispId, M_WEB_PUBLISHED_FORMAT, M_BGR32);
      MilWeb::MobjHookFunction(MsgId, M_UPDATE_WEB,  UpdateHookHandler, UserData);
      MilWeb::MobjHookFunction(DispId, M_UPDATE_WEB, UpdateHookHandler, UserData);
      }
   return AppId;
   }

/* Close connection */
void EndConnection(MIL_ID AppId)
   {
   if(AppId && !sDisConnect)	
     MilWeb::MappCloseConnection(AppId);
   }

