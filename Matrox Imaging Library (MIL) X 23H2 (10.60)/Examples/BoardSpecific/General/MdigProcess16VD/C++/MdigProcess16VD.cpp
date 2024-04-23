/***********************************************************************************/
/*
 * File name: MDigProcessMultiple.cpp
 *
 * Synopsis:  This program shows how to use multiple digitizers to do acquisition
 *            and displaying.
 *            The main starts an independent processing job for each digitizer
 *            (one per camera) and then waits for a key to be pressed to stop them.
 *
 *            This example is based on MDigProcess.cpp
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#include <math.h>
#include <io.h>
#include <windows.h>


// Maximum number of digitizer and displays.
#define MAX_DIGITIZER_NUM   16
#define MAX_DISPLAYS        4

#define BUFFERING_SIZE_MAX  4

// User's processing function prototype and hook parameter structure.
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

typedef struct
   {
   MIL_INT Enabled;
   MIL_INT Index;
   MIL_INT SizeX;
   MIL_INT SizeY;
   } DISPLAY_DEVICE_PARAM;

typedef struct
   {
   MIL_INT DeviceNum;
   MIL_ID  MilSystem;
   MIL_ID  MilDigitizer;
   MIL_ID  MilDisplay;
   MIL_ID  MilImageDisp;
   MIL_ID  MilImageDispChild;

   MIL_ID  MilGrabBufferList[BUFFERING_SIZE_MAX];
   MIL_INT MilGrabBufferListSize;

   MIL_INT IsCameraPresent;
   MIL_INT ProcessedImageCount;
   MIL_INT SizeBand;
   MIL_INT SizeX;
   MIL_INT SizeY;
   } DIG_PARAM;

int MosMain(void)
   {
   MIL_ID  MilApplication = M_NULL;
   MIL_ID  MilSystem = M_NULL;
   MIL_ID  MilDisplayCur = M_NULL;
   MIL_ID  MilImageDispCur = M_NULL;
   MIL_ID  MilImageDisp = M_NULL;
   DIG_PARAM DigParam[MAX_DIGITIZER_NUM] = { 0 };
   MIL_ID  MilDisplay[MAX_DIGITIZER_NUM] = { 0 };
   DISPLAY_DEVICE_PARAM DisplayParam[MAX_DIGITIZER_NUM] = {0};
   DISPLAY_DEVICE DisplayDevice = {0};
   DEVMODE DisplayDeviceMode = {0};
   MIL_INT32 NumberOfDigitizersToUse = MAX_DIGITIZER_NUM;
   MIL_INT NumberOfDisplays = 0;
   MIL_INT WindowCount = 0;
   MIL_INT NbrOfXWindows = 0;
   MIL_INT NbrOfYWindows = 0;
   MIL_INT DisplayPosIndex = 0;
   MIL_INT DispIndex = 0;
   MIL_INT DevNum = 0;
   MIL_INT32 i = 0;

   ///////////////////////////////////
   // Allocate application and system.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);

   ////////////////////////////////////////////////////
   // Inquire the number of display devices (monitors).
   DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
   DisplayDeviceMode.dmSize = sizeof(DEVMODE);
   DevNum = 0;
   for(i = 0; i< NumberOfDigitizersToUse; i++)
      {
      if(EnumDisplayDevices(NULL, i, &DisplayDevice, 0) &&
         DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
         {
         EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS,
                             &DisplayDeviceMode);
         NumberOfDisplays++;
         DisplayParam[DevNum].Enabled = 1;
         DisplayParam[DevNum].Index = i;
         DisplayParam[DevNum].SizeX = DisplayDeviceMode.dmPelsWidth;
         DisplayParam[DevNum].SizeY = DisplayDeviceMode.dmPelsHeight;
         DevNum++;
         }
      }

   ////////////////////////////////////////////////////////////////////////////////////////
   // Inquire the number of digitizers to use and the number of displays to display them in.
   NumberOfDisplays = min(MAX_DISPLAYS, NumberOfDisplays);
   NumberOfDigitizersToUse = (MIL_INT32)min(MAX_DIGITIZER_NUM,
                                 MsysInquire(MilSystem, M_DIGITIZER_NUM, M_NULL));

   MosPrintf(MIL_TEXT("Enter the number of digitizers to use (max: %d): \n"),
             (int)NumberOfDigitizersToUse);
   scanf("%d", &NumberOfDigitizersToUse);
   NumberOfDigitizersToUse = min(NumberOfDigitizersToUse, NumberOfDigitizersToUse);
   NumberOfDisplays = min(NumberOfDisplays, NumberOfDigitizersToUse);

   if((NumberOfDisplays > 1) && (NumberOfDigitizersToUse > 1))
      {
      MIL_INT32 lNbrOfDisplays = 0;
      MosPrintf(MIL_TEXT("Enter the number of display devices (monitors) ")
                MIL_TEXT("to use (max: %d): \n"),
                (int)NumberOfDisplays);
      scanf("%d", &lNbrOfDisplays);
      NumberOfDisplays = min(lNbrOfDisplays, NumberOfDisplays);
      }

  ////////////////////////////////////////////////////////////////////////////
  // Loop through each digitizer to allocate grab buffers and associated display.
  for(DevNum = 0; DevNum < NumberOfDigitizersToUse; DevNum++)
      {
      DIG_PARAM *pDig = &DigParam[DevNum];
      DISPLAY_DEVICE_PARAM *pDisp = &DisplayParam[DispIndex];

      // Allocate digitizer.
      MdigAlloc(MilSystem, DevNum, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &pDig->MilDigitizer);

      // Inquire digitizer settings.
      pDig->MilSystem = MilSystem;
      pDig->DeviceNum = DevNum;
      pDig->SizeBand = MdigInquire(pDig->MilDigitizer, M_SIZE_BAND, M_NULL);
      pDig->SizeX = MdigInquire(pDig->MilDigitizer, M_SIZE_X, M_NULL);
      pDig->SizeY = MdigInquire(pDig->MilDigitizer, M_SIZE_Y, M_NULL);

      // Allocate grab buffers for this digitizer.
      pDig->MilGrabBufferListSize = BUFFERING_SIZE_MAX;
      for(i = 0; i < pDig->MilGrabBufferListSize; i++)
         {
         MbufAllocColor(MilSystem, 3, pDig->SizeX, pDig->SizeY,
                        8 + M_UNSIGNED, M_IMAGE + M_GRAB + M_YUV16 + M_PACKED + M_ON_BOARD,
                        &pDig->MilGrabBufferList[i]);
         if(!pDig->MilGrabBufferList[i])
            {
            MosPrintf(MIL_TEXT("Unable to allocate grab buffers.\n"));
            MosPrintf(MIL_TEXT("Please reduce buffering size.\n"));
            MosGetchar();
            return 0;
            }
         }

      //////////////////////////////////////////
      // Allocate a MilDisplay for each monitor.
      if(DevNum >= WindowCount)
         {
         MIL_INT MaxNbrOfWindowsPerDisplay;
         MdispAlloc(MilSystem, DispIndex, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplayCur);

         // Calculate the number of digitizers to display in each Window.
         if(DispIndex+1 == NumberOfDisplays)
            MaxNbrOfWindowsPerDisplay = NumberOfDigitizersToUse - DevNum;
         else
            MaxNbrOfWindowsPerDisplay = NumberOfDigitizersToUse / NumberOfDisplays;

         NbrOfXWindows = (MIL_INT) max(pDisp->SizeX / pDig->SizeX,
                                       sqrt((double)MaxNbrOfWindowsPerDisplay) + 0.99);
         NbrOfXWindows = min(NbrOfXWindows, MaxNbrOfWindowsPerDisplay);
         NbrOfYWindows = (MIL_INT) ((MaxNbrOfWindowsPerDisplay /
                                    (double)NbrOfXWindows) + 0.99);
         WindowCount = WindowCount + (NbrOfXWindows * NbrOfYWindows);

         MbufAllocColor(MilSystem, pDig->SizeBand, pDig->SizeX * NbrOfXWindows,
                           pDig->SizeY * NbrOfYWindows, 8 + M_UNSIGNED,
                           M_IMAGE + M_DISP + (pDig->SizeBand == 3?
                              M_BGR32 + M_PACKED:0) + M_NON_PAGED,
                           &MilImageDispCur);

         // Clear and select the display buffer.
         MbufClear(MilImageDispCur, M_COLOR_BLACK);
         MdispSelect(MilDisplayCur, MilImageDispCur);

         DisplayPosIndex = 0;
         DispIndex++;
         }

      pDig->MilDisplay = MilDisplayCur;
      pDig->MilImageDisp = MilImageDispCur;

      // Allocate a child display buffer for this digitizer.
      // The grab buffer will be copied into this buffer.
      MIL_INT XPos, YPos;
      XPos = (DisplayPosIndex%NbrOfXWindows) * pDig->SizeX;
      YPos = (DisplayPosIndex/NbrOfXWindows) * pDig->SizeY;
      pDig->MilImageDispChild = MbufChild2d(pDig->MilImageDisp, XPos, YPos, pDig->SizeX,
                                            pDig->SizeY, M_NULL);


      MosPrintf(MIL_TEXT("Allocating digitizer device %2d on display device %2d. \n"),
                DevNum, DispIndex);
      
      DisplayPosIndex++;
      }

   ///////////////////////////////////////
   // Start MdigProcess on each digitizer.
   MosPrintf(MIL_TEXT("\n\nStarting MdigProcess on %d digitizer(s).\n"),
             (int)NumberOfDigitizersToUse);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(DevNum = 0; DevNum < NumberOfDigitizersToUse; DevNum++)
      {
      DIG_PARAM *pDig = &DigParam[DevNum];
      if(MdigInquire(pDig->MilDigitizer, M_CAMERA_PRESENT, M_NULL))
         pDig->IsCameraPresent = TRUE;
      else
         pDig->IsCameraPresent = FALSE;

      MdigProcess(pDig->MilDigitizer, pDig->MilGrabBufferList, pDig->MilGrabBufferListSize,
                  M_START, M_DEFAULT, ProcessingFunction, pDig);
      }

   /////////////////////////
   // Print grab statistics.
   while(!MosKbhit())
      {
      COORD Coord;
      Coord.X = 0;
      Coord.Y = (SHORT) (8 + NumberOfDigitizersToUse);

      // At each second.
      MosSleep(1000);
      SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Coord);
      for(DevNum = 0; DevNum < NumberOfDigitizersToUse; DevNum++)
         {
         double  FrameRate = 0;
         MIL_INT FramesMissed = 0;
         DIG_PARAM *pDig = &DigParam[DevNum];

         // Inquire and print MdigProcess statistics.
         MdigInquire(pDig->MilDigitizer, M_PROCESS_FRAME_RATE, &FrameRate);
         MdigInquire(pDig->MilDigitizer, M_PROCESS_FRAME_MISSED, &FramesMissed);
         MosPrintf(MIL_TEXT("Dig #: %2d, %6d frames grabbed at %5.2f (f/s)."), DevNum,
                   pDig->ProcessedImageCount, FrameRate);

         // Print if a camera is present.
         if(pDig->IsCameraPresent)
            {
            if(FramesMissed > 1)
               MosPrintf(MIL_TEXT("%6d frames were missed.   "), FramesMissed);
            else if(FramesMissed == 1)
               MosPrintf(MIL_TEXT("1 frame was missed.       "));
            }
         else
            {
            MosPrintf(MIL_TEXT(" No camera is present.     "));
            }
         MosPrintf(MIL_TEXT("\n"));
         }
      }

   MosGetchar();
   MosPrintf(MIL_TEXT("\n\nExiting...\n"));

   // Stop MdigProcess for each digitizer.
   for(DevNum = 0; DevNum < NumberOfDigitizersToUse; DevNum++)
       {
       DIG_PARAM *pDig = &DigParam[DevNum];
       MdigProcess(pDig->MilDigitizer, pDig->MilGrabBufferList, pDig->MilGrabBufferListSize,
                   M_STOP, M_DEFAULT, ProcessingFunction, pDig);
       }

   //////////////////////////////
   // Free allocated MIL objects.
   for(DevNum = 0; DevNum < NumberOfDigitizersToUse; DevNum++)
       {
       for (i = 0; i < DigParam[DevNum].MilGrabBufferListSize; i++)
          {
          if(DigParam[DevNum].MilGrabBufferList[i])
             {
             MbufFree(DigParam[DevNum].MilGrabBufferList[i]);
             DigParam[DevNum].MilGrabBufferList[i] = M_NULL;
             }
          }
       if(DigParam[DevNum].MilImageDispChild)
         MbufFree(DigParam[DevNum].MilImageDispChild);

       if(MilImageDisp != DigParam[DevNum].MilImageDisp)
         {
         MilImageDisp = DigParam[DevNum].MilImageDisp;
         MbufFree(MilImageDisp);
         MdispFree(DigParam[DevNum].MilDisplay);
         }

       MdigFree(DigParam[DevNum].MilDigitizer);
       }

   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

////////////////////////////////////////////////////////////////////
// MdigProcess processing function called every time a grab is done.
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   DIG_PARAM *pDig = (DIG_PARAM *)HookDataPtr;
   MIL_ID ModifiedBufferId;

   // Get grab buffer.
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);
   pDig->ProcessedImageCount++;

   // Inquire if a camera is present, if so copy the grab buffer to the display.
   if(!MdigInquire(pDig->MilDigitizer, M_CAMERA_PRESENT, NULL))
      {
      pDig->IsCameraPresent = FALSE;
      MgraText(M_DEFAULT, pDig->MilImageDispChild, 20, 20,
               MIL_TEXT("Sorry, no camera is present"));
      }
   else
      {
      pDig->IsCameraPresent = TRUE;
      MbufCopy(ModifiedBufferId, pDig->MilImageDispChild);
      }

   return 0;
   }
