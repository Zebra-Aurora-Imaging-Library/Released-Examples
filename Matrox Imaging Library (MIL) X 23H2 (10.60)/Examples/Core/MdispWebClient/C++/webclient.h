//
// Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H
#include <milweb.h>
#include <stdio.h>
#include <string.h>

#if M_MIL_USE_WINDOWS
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

typedef struct  MilWebWindow
   {
   HWND Window;
   HDC MemDC;
   HBITMAP hbmp;
   MIL_UINT8 *image_data;
   } MilWebWindow;

#endif
#define MILWEB_URL                 MIL_TEXT("ws://localhost:7681")


void DisplayMessage(MIL_UINT8 *Data, MIL_INT MsgLength, MIL_INT64 MsgTag, void *UserData);
void DisplayImage(MIL_UINT8 *Data, MIL_INT SizeByte, MIL_INT SizeX, MIL_INT SizeY, MIL_INT PitchByte, void *UserData);
MIL_INT MFTYPE UpdateHookHandler(MIL_INT /*HookType*/, MIL_ID EventId,void* UserData);
MIL_ID StartConnection(void *UserData, MIL_CONST_TEXT_PTR Url);
void EndConnection(MIL_ID AppId);

#endif // WEB_CLIENT_H
