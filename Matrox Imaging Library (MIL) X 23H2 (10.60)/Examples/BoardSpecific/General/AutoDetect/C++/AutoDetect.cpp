/********************************************************************************/
/* 
* File name: autodetect.cpp 
*
* Synopsis:  This program shows how to detect Matrox Orion HD or 
*            Matrox Clarity UHD input sources using the M_MINIMAL 
*            flag with MdigAlloc.
*
*            When a digitizer is allocated with M_MINIMAL it can only be
*            used for input source detection, it cannot be used for grabbing.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#define M_MIL_USE_OS_SCREEN_FUNCTIONS 1

#include <mil.h>

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("AutoDetect\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program shows how to detect\n")\
      MIL_TEXT("input sources using the M_MINIMAL flag with MdigAlloc. \n\n")\
      MIL_TEXT("When a digitizer is allocated with M_MINIMAL it can only be\n")\
      MIL_TEXT("used for input source detection, it cannot be used for grabbing.\n\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, digitizer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\r"));
   MosScreenRefresh();
   MosGetch();   
   }

const MIL_STRING gDCFName = MIL_TEXT("AutoDetect.dcf");
const MIL_DOUBLE gCameraPresentWaitTime = 5.0;

MIL_INT MFTYPE DigHookCameraPresent(MIL_INT hookType, MIL_ID eventId, void *userData);
void PrintDigitizerInfo(MIL_ID MilDigitizer);
bool SystemSupportsAutoDetect(MIL_ID MilSystem);

typedef struct 
   {
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_INT DevNumber;
   MIL_ID MilDisplay;
   MIL_ID MilImageDisp;

   bool   IsCameraPresent;
   bool   IsGrabbing;
   MIL_DOUBLE CheckforCameraPresentWaitTime;

   } DIG_INFO, *PDIG_INFO ;


int MosMain(void)
   { 
   MIL_ID MilApplication,                   /* Application identifier.  */
          MilSystem;                        /* System identifier.       */
   MIL_STRING DCFFormat;
   MIL_INT i;
   MIL_INT NumberOfGrabsInProgress = 0;

   MosScreenInit();

   // First step: allocate Mil system.
   MappAlloc(M_DEFAULT, &MilApplication);
   MsysAlloc(M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);

   if ( !SystemSupportsAutoDetect(MilSystem) )
      {
      MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
      MosScreenRelease();
      return 1;
      }

   MosScreenResize(44, 80);

   MIL_INT lNumberOfDigitizer;
   MsysInquire(MilSystem, M_DIGITIZER_NUM, &lNumberOfDigitizer);

   PDIG_INFO DigInfo = new DIG_INFO[lNumberOfDigitizer];
   memset(DigInfo, 0, sizeof(DIG_INFO) * lNumberOfDigitizer);
   
   PrintHeader();

   MosScreenClear();
   
   // Second step: allocate displays and minimum digitizers.
   MosPrintf(MIL_TEXT("Allocating digitizers... "));
   for(i = 0; i < lNumberOfDigitizer; i++)
      {
      PDIG_INFO pDigInfo = &DigInfo[i];
      pDigInfo->MilSystem = MilSystem;
      pDigInfo->DevNumber = i;
      pDigInfo->IsGrabbing = false;
      pDigInfo->IsCameraPresent = false;

      MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &pDigInfo->MilDisplay);
      MdigAlloc(MilSystem, DigInfo[i].DevNumber, gDCFName.c_str(), M_MINIMAL, &pDigInfo->MilDigitizer);
      MdigHookFunction(DigInfo[i].MilDigitizer, M_CAMERA_PRESENT, DigHookCameraPresent, pDigInfo);

      if(MdigInquire(pDigInfo->MilDigitizer, M_CAMERA_PRESENT, M_NULL) == M_TRUE)
         pDigInfo->IsCameraPresent = true;
      MosPrintf(MIL_TEXT("."));
      }
   MosPrintf(MIL_TEXT(" done\n\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------------------------\n"));
   MosScreenRefresh();
   MappControl(M_ERROR, M_PRINT_DISABLE);

   // Third step: loop all the digitizers and print their information.
   while(1)
      {
      MosSleep(500);

      MIL_INT KeyPressed = 0;
      if(MosKbhit())
         KeyPressed = MosGetch();

      if(KeyPressed == 'q')
         break;

      MosScreenSetPosition(0,6);

      // Print the input source of each digitizer.
      for(i = 0; i < lNumberOfDigitizer; i++)
         {
         PDIG_INFO pDigInfo = &DigInfo[i];

         MosPrintf(MIL_TEXT("\nDigitizer %d: "), (int)pDigInfo->DevNumber);

         if(pDigInfo->IsCameraPresent)
            {
            MosPrintf(MIL_TEXT("Input source is present. %s"), pDigInfo->IsGrabbing?MIL_TEXT("Live grab in progress.\n"):
                                                                                    MIL_TEXT("                      \n"));
            PrintDigitizerInfo(pDigInfo->MilDigitizer);
            }
         else
            {
            MosPrintf(MIL_TEXT("Input source not present.                                        \n"));
            MosPrintf(MIL_TEXT("                                                                              \n"));
            MosPrintf(MIL_TEXT("                                                                              \n"));
            MosScreenRefresh();
            }

         // The camera present hook was called, check the camera present status after a few seconds
         // to be sure it is stable. Prevents fast plug-unplug issues.
         if(pDigInfo->CheckforCameraPresentWaitTime)
            {
            MIL_DOUBLE TimeRead;
            MappTimer(M_TIMER_READ + M_GLOBAL, &TimeRead);
            if(TimeRead - pDigInfo->CheckforCameraPresentWaitTime > gCameraPresentWaitTime)
               {
               pDigInfo->CheckforCameraPresentWaitTime = 0;
               if(MdigInquire(pDigInfo->MilDigitizer, M_CAMERA_PRESENT, M_NULL))
                  pDigInfo->IsCameraPresent = true;
               else
                  pDigInfo->IsCameraPresent = false;
               }
            }

         // Stop the live grab if the camera is disconnected.
         bool StopGrabbing = (pDigInfo->IsGrabbing) && (!pDigInfo->IsCameraPresent);

         // The user pressed the digitizer number.
         if((KeyPressed == i + '0') || StopGrabbing)
            {
            if(pDigInfo->IsGrabbing || StopGrabbing)
               {
               // A grab is in progress, stop it.
               MdigHalt(pDigInfo->MilDigitizer);
               MbufFree(DigInfo[i].MilImageDisp);
               MdigFree(pDigInfo->MilDigitizer);

               // Allocated the digitizer with the M_MINIMAL flag. This digitizer cannot be used for grabbing.
               MdigAlloc(pDigInfo->MilSystem, pDigInfo->DevNumber, gDCFName.c_str(), M_MINIMAL, &pDigInfo->MilDigitizer);
               MdigHookFunction(pDigInfo->MilDigitizer, M_CAMERA_PRESENT, DigHookCameraPresent, pDigInfo);
               pDigInfo->IsGrabbing = false;
               NumberOfGrabsInProgress--;
               }
            else if(pDigInfo->IsCameraPresent)
               {
               MIL_INT SizeX, SizeY;
               MdigInquire(pDigInfo->MilDigitizer, M_FORMAT_DETECTED, DCFFormat);
               MdigFree(pDigInfo->MilDigitizer);

               MdigAlloc(pDigInfo->MilSystem, pDigInfo->DevNumber, DCFFormat.c_str(), M_DEFAULT, &pDigInfo->MilDigitizer);
               if(pDigInfo->MilDigitizer)
                  {
                  MdigHookFunction(pDigInfo->MilDigitizer, M_CAMERA_PRESENT, DigHookCameraPresent, pDigInfo);

                  MIL_STRING_STREAM WindowTitle;
                  WindowTitle << MIL_TEXT("Dev: ") << pDigInfo->DevNumber << MIL_TEXT(" DCF: ") << DCFFormat;
                  MdispControl(pDigInfo->MilDisplay, M_TITLE, WindowTitle.str().c_str());

                  MdigInquire(pDigInfo->MilDigitizer, M_SIZE_X, &SizeX);
                  MdigInquire(pDigInfo->MilDigitizer, M_SIZE_Y, &SizeY);
                  MbufAllocColor(pDigInfo->MilSystem, 3, SizeX, SizeY, 8, M_IMAGE + M_GRAB + M_DISP, &pDigInfo->MilImageDisp);
                  MbufClear(pDigInfo->MilImageDisp, 0);
                  MdispSelect(pDigInfo->MilDisplay, pDigInfo->MilImageDisp);
                  MdigGrabContinuous(pDigInfo->MilDigitizer, pDigInfo->MilImageDisp);
                  pDigInfo->IsGrabbing = true;
                  NumberOfGrabsInProgress++;
                  }
               else
                  {
                  MdigAlloc(pDigInfo->MilSystem, pDigInfo->DevNumber, gDCFName.c_str(), M_MINIMAL, &pDigInfo->MilDigitizer);
                  MdigHookFunction(pDigInfo->MilDigitizer, M_CAMERA_PRESENT, DigHookCameraPresent, pDigInfo);
                  pDigInfo->IsGrabbing = false;
                  }
               }
            }
         }

      MosPrintf(MIL_TEXT("\n---------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press the digitizer number (0-%d) to start or stop a live grab.  \n"), (int)lNumberOfDigitizer-1);
      MosPrintf(MIL_TEXT("Press 'q' to quit.                                               \n"));
      MosPrintf(MIL_TEXT("                                                                 \n"));
      MosScreenRefresh();
      }

   // Last step: free all allocations and exit.
   for(i = 0; i < lNumberOfDigitizer; i++)
      {
      if(DigInfo[i].MilImageDisp)
         MbufFree(DigInfo[i].MilImageDisp);

      if(DigInfo[i].MilDisplay)
         MdispFree(DigInfo[i].MilDisplay);

      if(DigInfo[i].MilDigitizer)
         MdigFree(DigInfo[i].MilDigitizer);
      }

   delete [] DigInfo;

   MsysFree(MilSystem);
   MappFree(MilApplication);
   MosScreenRelease();

   return 0;
   }


// The camera present hook.
// When called inquire and store the current time. 
// Then MdigInquire of camera present will be done in the main loop after a few seconds.
MIL_INT MFTYPE DigHookCameraPresent(MIL_INT hookType, MIL_ID eventId, void *userData)
   {
   PDIG_INFO pDigInfo = (PDIG_INFO)userData;

   pDigInfo->IsCameraPresent = false;
   MappTimer(M_TIMER_READ + M_GLOBAL, &pDigInfo->CheckforCameraPresentWaitTime);

   return 0;
   }


// Prints to the console the details of the detected input.
void PrintDigitizerInfo(MIL_ID MilDigitizer)
   {
   MIL_STRING      DCFFormat;
   MIL_INT        InputMode;
   MIL_INT        ScanMode;
   MIL_INT        SizeX, SizeY, SizeBand;
   MIL_DOUBLE     FrameRate;
   MIL_INT        DigitizerNumber;

   MdigInquire(MilDigitizer, M_NUMBER, &DigitizerNumber);
   MdigInquire(MilDigitizer, M_SIZE_X, &SizeX);
   MdigInquire(MilDigitizer, M_SIZE_Y, &SizeY);
   MdigInquire(MilDigitizer, M_SIZE_BAND, &SizeBand);
   MdigInquire(MilDigitizer, M_INPUT_MODE, &InputMode);
   MdigInquire(MilDigitizer, M_SCAN_MODE, &ScanMode);
   MdigInquire(MilDigitizer, M_SELECTED_FRAME_RATE, &FrameRate);
   MdigInquire(MilDigitizer, M_FORMAT_DETECTED, DCFFormat);

   MosPrintf(MIL_TEXT("                                                                              \r"));
   MosPrintf(MIL_TEXT("\t%lld x %lld, %d band(s). "), (long long)SizeX, (long long)SizeY, (int)SizeBand);


   switch(InputMode)
      {
      case M_ANALOG: MosPrintf(MIL_TEXT("analog "));break;
      case M_DIGITAL: MosPrintf(MIL_TEXT("digital "));break;
      default: MosPrintf(MIL_TEXT(" "));
      }
   switch(ScanMode)
      {
      case M_PROGRESSIVE: MosPrintf(MIL_TEXT("progressive "));break;
      case M_INTERLACE: MosPrintf(MIL_TEXT("interlaced "));break;
      default: MosPrintf(MIL_TEXT(" "));
      }

   MosPrintf(MIL_TEXT("@ %0.2f fps.\n"), FrameRate);
   MosPrintf(MIL_TEXT("                                                                              \r"));
   MosPrintf(MIL_TEXT("\tDCF: %s.\n"), DCFFormat.c_str());
   MosScreenRefresh();
   }

/* Verify whether this example can run on the selected system. */
bool SystemSupportsAutoDetect(MIL_ID MilSystem)
   {
   MIL_INT SystemType = 0;

   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
   if ( SystemType == M_SYSTEM_ORION_HD_TYPE ||
      SystemType == M_SYSTEM_CLARITY_UHD_TYPE )
      {
      return true;
      }

   MosPrintf(MIL_TEXT("This example program can only be used with the Matrox Driver for:\n")
      MIL_TEXT("Orion HD, Clarity UHD.\n\n"));
   MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
      MIL_TEXT("MIL Config.\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------------------------------")
      MIL_TEXT("----------- \n\n"));
   MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
   MosScreenRefresh();
   MosGetch();
   return false;
   }
