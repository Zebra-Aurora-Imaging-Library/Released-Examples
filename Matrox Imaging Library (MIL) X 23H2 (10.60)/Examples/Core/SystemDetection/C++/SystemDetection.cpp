/******************************************************************************
/*
* File name: SystemDetection.cpp
*
* Synopsis:  This program shows how to use the MappInquire(M_INSTALLED_... inquires to detect
*            installed systems types and display their names using SystemDetection.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <algorithm>
#include <iostream>

#define IMAGE_SIZEX 640
#define IMAGE_SIZEY 480

int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_INT NbAvailableSystems;

   MappAlloc(M_DEFAULT, &MilApplication);
   MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_COUNT, &NbAvailableSystems);

   std::vector<MIL_ID> MilSystems;
   std::vector<MIL_ID> MilDisplays;
   std::vector<MIL_ID> MilBuffers;
   MIL_STRING CurrentBoardName;
   MIL_STRING CurrentDisplayableBoardName;
   MIL_INT CurrentSystemType = 0;
   MIL_INT DeviceCount = 0;

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

   MosPrintf(MIL_TEXT("%d system(s) found\n\n\n"), NbAvailableSystems);

   for(MIL_INT i = 0; i < NbAvailableSystems; i++)
      {
      MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_PRINT_NAME + i, CurrentDisplayableBoardName);
      MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_DESCRIPTOR + i, CurrentBoardName);
      MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_TYPE + i, &CurrentSystemType);
      MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_DEVICE_COUNT + i, &DeviceCount);

      MosPrintf(MIL_TEXT("\nSystem Print Name: %-20s"), CurrentDisplayableBoardName.c_str());
      MosPrintf(MIL_TEXT("\nSystem Descriptor: %-20s"), CurrentBoardName.c_str());
      MosPrintf(MIL_TEXT("\nSystem Type      : %-d"), (MIL_INT32)CurrentSystemType);
      MosPrintf(MIL_TEXT("\nSystem Count     : "));

      if(DeviceCount == M_UNKNOWN)
         MosPrintf(MIL_TEXT("unknown\n"));
      else
         MosPrintf(MIL_TEXT("%-d\n"), (MIL_INT32)DeviceCount);

      if(DeviceCount == 0)
         continue;

      MIL_ID SystemId = M_NULL;
      MsysAlloc(M_DEFAULT, CurrentBoardName, M_DEFAULT, M_DEFAULT, &SystemId);
      if(SystemId)
         {
         MIL_ID DisplayId      = M_NULL;
         MIL_ID BufferId       = M_NULL;

         MbufAlloc2d(SystemId, IMAGE_SIZEX, IMAGE_SIZEY, 8 + M_UNSIGNED, M_IMAGE + M_DISP, &BufferId);
         MdispAlloc(SystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &DisplayId);

         MbufClear(BufferId, M_COLOR_BLACK);
         MdispControl(DisplayId, M_TITLE, CurrentDisplayableBoardName);
         MdispSelect(DisplayId, BufferId);

         MIL_STRING_STREAM WriteMessage;
         WriteMessage << MIL_TEXT("Allocated ") << CurrentDisplayableBoardName;
         MgraText(M_DEFAULT, BufferId, IMAGE_SIZEX / 5, IMAGE_SIZEY / 3, WriteMessage.str());

         if(DeviceCount == M_UNKNOWN)
            MosPrintf(MIL_TEXT("%s.\n"), WriteMessage.str().c_str());

         MilBuffers.push_back(BufferId);
         MilDisplays.push_back(DisplayId);
         MilSystems.push_back(SystemId);
         }
      else
         {
         if(DeviceCount == M_UNKNOWN)
            {
            MosPrintf(MIL_TEXT("No %s are present in the system.\n"), CurrentDisplayableBoardName.c_str());
            }
         else
            {
            MIL_STRING ErrorMessageFunction;
            MIL_STRING ErrorMessage;
            MIL_STRING ErrorSubMessage1;
            MIL_STRING_STREAM ErrorStream;

            MappGetError(M_DEFAULT, M_MESSAGE + M_CURRENT_OPCODE, ErrorMessageFunction);
            MappGetError(M_DEFAULT, M_MESSAGE + M_CURRENT, ErrorMessage);
            MappGetError(M_DEFAULT, M_MESSAGE + M_CURRENT_SUB_1, ErrorSubMessage1);

            ErrorStream << MIL_TEXT("Error allocating ") << CurrentDisplayableBoardName << MIL_TEXT(": ") << ErrorMessage << MIL_TEXT(" ") << ErrorSubMessage1 << MIL_TEXT("\n");
            MosPrintf(MIL_TEXT("%s"), ErrorStream.str().c_str());
            }
         }
      }
   MosPrintf(MIL_TEXT("\nPress <Enter> to quit the application\n"));

   MosGetch();

   for(MIL_UINT i = 0; i < MilSystems.size(); ++i)
      {
      MdispFree(MilDisplays[i]);
      MbufFree(MilBuffers[i]);
      MsysFree(MilSystems[i]);
      }

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
   MappFree(MilApplication);

   return 0;
   }
