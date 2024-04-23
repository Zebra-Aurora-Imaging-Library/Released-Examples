/********************************************************************************/
/* 
* File name: MDigCrossbar
*
* Synopsis:  This program implements a crossbar switch where
*            any video input can be sent to any video output.
*        
*            Features include:
*               - Picture-in-picture overlay.
*               - Automatic calculation of output to input latency.
*               - No tearing video output.
*               - Low latency video output.
*               - Moving vertical line.
*               - Microsoft Direct3D warp effects.
* 
*            Note:
*               - Displays are allocated in full-screen mode.
*                 You must have at least 2 display outputs.
*
*               - In order to measure the latency, a video DVI-D output 
*                 must be connected to an input.
*
*               - To compile this project you must have Microsoft DirectX SDK
*                 installed.
*                 http://www.microsoft.com/en-us/download/details.aspx?id=6812
* 
*                 Possible 'S1023' error when you install the DirectX SDK:
*                 This is a known issue and is explained in the Microsoft support knowledge base.
*                 This error can be ignored.
*                 For more information, refer to: http://support.microsoft.com/kb/2728613
*
*               - It is recommended to disable Windows Aero.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <windows.h> 
#include <mil.h>
#include <vector>
#include <queue>
#include <list>
#include <d3d9.h>
#include "md3ddisplayeffect.h"
#include "mdisplay.h"
#include "mdigitizer.h"

using namespace std;

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MDigCrossbar\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program implements a crossbar switch where\n")\
             MIL_TEXT("any video input can be sent to any video output. \n\n")\
             MIL_TEXT("Features include:\n")\
             MIL_TEXT("   - Picture-in-picture overlay.\n")\
             MIL_TEXT("   - Automatic calculation of output to input latency.\n")\
             MIL_TEXT("   - No tearing video output.\n")\
             MIL_TEXT("   - Low latency video output.\n")\
             MIL_TEXT("   - Moving vertical line.\n")\
             MIL_TEXT("   - Microsoft Direct3D warp effects.\n")\
             MIL_TEXT("Note:\n")\
             MIL_TEXT("   - Displays are allocated in full-screen mode.\n")\
             MIL_TEXT("     You must have at least 2 display outputs.\n")\
             MIL_TEXT("   - In order to measure the latency, a video DVI-D output\n")\
             MIL_TEXT("     must be connected to an input.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, digitizer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\r"));
   MosGetch();
   }


#define MAX_SYSTEMS 6


typedef struct _AUTODETECT
   {
   MIL_INT      DigDeviceNumber;
   MIL_STRING    DCFName;
   } AUTODETECT;

AUTODETECT DCF_SCAN[] = {
   {M_DEV0, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV1, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV2, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV3, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV4, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV5, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV6, MIL_TEXT("AutoDetect.dcf") },
   {M_DEV7, MIL_TEXT("AutoDetect.dcf") },
   };
size_t DCF_SCAN_COUNT = (sizeof(DCF_SCAN) / sizeof(AUTODETECT));

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, 
                                  void* HookDataPtr);

bool SystemSupportsCrossBar(MIL_ID MilSystem);

int MosMain(void)
   { 
   MIL_ID MilApplication;
   MIL_ID MilSystem[MAX_SYSTEMS] = { M_NULL, };
   MIL_INT SystemIndex = 0;
   MIL_STRING SystemDescriptor;
   MIL_INT InputCount = 0;

   vector<CDigitizer *>  DigitizerList;
   vector<CDisplay *>     DisplayList;
   vector<CDisplay *>::iterator pDispIterator;
   vector<CDigitizer *>::iterator pDigIterator;

   LPDIRECT3D9EX g_pD3D = NULL;
   Direct3DCreate9Ex(D3D_SDK_VERSION, &g_pD3D);

   if (!g_pD3D)
      {
      MosPrintf(MIL_TEXT("Cannot allocated LPDIRECT3D9EX object."));
      MosGetch();
      exit(1);
      }

   ///////////////////////////////////
   // Allocate systems and digitizers.
   ///////////////////////////////////
   MappAlloc(M_DEFAULT, &MilApplication);
   MappControl(M_ERROR, M_PRINT_DISABLE);
   MgraFont(M_DEFAULT, M_FONT_DEFAULT_MEDIUM);
   MgraControl(M_DEFAULT,  M_BACKGROUND_MODE, M_TRANSPARENT);

  MsysAlloc(M_SYSTEM_DEFAULT, M_DEV0, M_DEFAULT, &MilSystem[0]);
   if ( !SystemSupportsCrossBar(MilSystem[0]) )
      {
      MappFreeDefault(MilApplication, MilSystem[0], M_NULL, M_NULL, M_NULL);
      return 1;
      }
   else
      {
      MsysInquire(MilSystem[0], M_SYSTEM_DESCRIPTOR, SystemDescriptor);
      MsysFree(MilSystem[0]);
      }

   PrintHeader();

   InputCount = 0;
   for(SystemIndex = 0; SystemIndex < MAX_SYSTEMS; SystemIndex++)
      {
      MsysAlloc(M_SYSTEM_DEFAULT, SystemIndex, M_DEFAULT, &MilSystem[SystemIndex]);
      if(MilSystem[SystemIndex] == M_NULL)
         break;

      // Start camera detection and print detected inputs.
      MosPrintf(MIL_TEXT("\n"));
      MosPrintf(MIL_TEXT("----------------------------------------------------------\n"));
      MosPrintf(MIL_TEXT("Searching for input sources on Matrox %s device %d.\n"), SystemDescriptor.c_str(), (int)SystemIndex);
      MosPrintf(MIL_TEXT("----------------------------------------------------------\n\n"));

      for(size_t ScanElement = 0; ScanElement < DCF_SCAN_COUNT; ScanElement++)
         {
         CDigitizer *pDig = new CDigitizer(MilSystem[SystemIndex], DCF_SCAN[ScanElement].DigDeviceNumber, DCF_SCAN[ScanElement].DCFName, InputCount);
         if(pDig->Digitizer())
            {
            InputCount++;
            DigitizerList.push_back(pDig);

            MosPrintf(MIL_TEXT(" Dig %d:  \n"), (int) DCF_SCAN[ScanElement].DigDeviceNumber);
            pDig->PrintDigitizerInfo();
            }
         else
            {
            delete pDig;
            MosPrintf(MIL_TEXT(" Dig %d: No input detected. \n"), (int) DCF_SCAN[ScanElement].DigDeviceNumber);
            }
         }
      }
   MappControl(M_ERROR, M_PRINT_ENABLE);

   if(DigitizerList.empty())
      {
      MosPrintf(MIL_TEXT("\n\nNo input(s) detected, exiting.\n"));
      goto end;
      }

   ///////////////////////////////////
   // Allocate displays.
   ///////////////////////////////////
   MosPrintf(MIL_TEXT("\n\n"));


   MosPrintf(MIL_TEXT("------------------------------------\n")); 
   MosPrintf(MIL_TEXT("Allocating displays.\n"));
   MosPrintf(MIL_TEXT("------------------------------------\n\n")); 
   for(UINT i = 0; i < 32; i++)
      {
      CDigitizer *pDig = NULL;
      MIL_INT DigSizeX = M_NULL;
      MIL_INT DigSizeY = M_NULL;

      if(i < DigitizerList.size())
         pDig = DigitizerList.at(i);

      if(pDig)
         {
         DigSizeX = pDig->SizeX();
         DigSizeY = pDig->SizeY();
         }

      CDisplay *pDisp = new CDisplay(MilSystem[0], i, g_pD3D);
      if(pDisp->IsAllocated())
         {
         DisplayList.push_back(pDisp);
         pDisp->SetDisplaySource((MIL_INT64)pDig, DigSizeX, DigSizeY);
         }
      else
         {
         delete pDisp;
         }
      }

   HWND hForegroundWindow = GetForegroundWindow();
   ::InvalidateRect(hForegroundWindow, NULL, TRUE);
   UpdateWindow(hForegroundWindow);

   MosPrintf(MIL_TEXT("\nPress enter to start.\n"));
   MosGetchar();

   ///////////////////////////////////
   // Start grabbing.
   ///////////////////////////////////
   if(DigitizerList.size())
      {
      MosPrintf(MIL_TEXT("\nStart grabbing from detected inputs:\n"));
      MosPrintf(MIL_TEXT("------------------------------------\n")); 
      }
   for (pDigIterator = DigitizerList.begin(); pDigIterator!= DigitizerList.end(); pDigIterator++)
      {
      CDigitizer *pDig = *pDigIterator;
      pDig->SetUserData(&DisplayList);
      Sleep(100);
      pDig->Start(ProcessingFunction);
      }

   Sleep(1000);
   SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

   /////////////////////////////////////////
   // Print commands, status and statistics.
   /////////////////////////////////////////

   MIL_INT Key = 0;
   while( Key != 'q' )
      {
      MIL_INT ProcessFrameCount  = 0;
      MIL_INT ProcessFrameMissed  = 0;
      MIL_DOUBLE ProcessFrameRate = 0;
      bool FoundDviDigitalSource = false;

      Sleep(1000);
      system("cls");

      MosPrintf(MIL_TEXT("MdigCrossbar.\n"));
      MosPrintf(MIL_TEXT("-------------\n\n"));
      MosPrintf(MIL_TEXT("Commands:\n"));
      MosPrintf(MIL_TEXT("---------\n"));
      MosPrintf(MIL_TEXT(" Press:\n"));
      MosPrintf(MIL_TEXT("  <0 to %d> to display the selected input on all outputs.\n"), (int) DigitizerList.size() - 1);
      MosPrintf(MIL_TEXT("  <s> to display a selected input on a selected output.\n"));
      MosPrintf(MIL_TEXT("  <o> to overlay a selected input on a selected output.\n"));
      MosPrintf(MIL_TEXT("  <h> to display one input per output.\n"));
      MosPrintf(MIL_TEXT("  <r> to toggle between 30fps and 60fps.\n"));
      MosPrintf(MIL_TEXT("  <d> to enable/disable Direct3D effects.\n"));
      MosPrintf(MIL_TEXT("  <m> to enable/disable drawing a moving line.\n"));
      MosPrintf(MIL_TEXT("  <l> to calculate the latency between the output and the input.\n"));
      MosPrintf(MIL_TEXT("  <q> to quit.\n"));
      MosPrintf(MIL_TEXT("\n"));

      Key = 0;
      if(MosKbhit())
         Key = MosGetch();

      MosPrintf(MIL_TEXT("Digitizers grabbing:\n"));
      MosPrintf(MIL_TEXT("--------------------\n"));

      // Print digitizer statistics.
      for (pDigIterator = DigitizerList.begin(); pDigIterator!= DigitizerList.end(); pDigIterator++)
         {
         CDigitizer *pDig = *pDigIterator;
         pDig->GetStatistics(&ProcessFrameCount, &ProcessFrameRate, &ProcessFrameMissed);

         MosPrintf(MIL_TEXT("%4d (%4d x %4d @ %.1f Hz), Frame: %5d (missed: %2d). "),
            pDig->Index(), pDig->SizeX(), pDig->SizeY(), ProcessFrameRate, ProcessFrameCount, ProcessFrameMissed);

         CDisplay *pDispLatencySource = (CDisplay *) pDig->GetSourceId();
         if(pDispLatencySource && pDispLatencySource->Latency())
            {
            MIL_DOUBLE Cur, Min, Max, Average;
            MIL_INT    CurInFrames, Count;
            pDispLatencySource->GetLatency(&Cur, &Min, &Max, &Average, &CurInFrames, &Count);

            MosPrintf(MIL_TEXT("Latency from disp %lld (in ms): Cur:%.1f (%lld frames), Min:%.1f, Max:%.1f, Avg:%.1f, Count:%lld "), (long long)pDispLatencySource->Index(),
               Cur*1000.0, (long long)CurInFrames, Min*1000.0, Max*1000.0, Average*1000.0, (long long)Count);
            }

         MosPrintf(MIL_TEXT("\n"));

         // Toggle between 30fps and 60fps.
         if(Key == 'r')
            {
            if(pDig->GrabDecimation() == 1)
               pDig->GrabDecimation(2);
            else
               pDig->GrabDecimation(1);
            }

         // Disable/disable drawing a moving line.
         if(Key == 'm')
            pDig->MovingLine(!pDig->MovingLine());
         }

      // Print display statistics.
      MosPrintf(MIL_TEXT("\n"));
      MosPrintf(MIL_TEXT("Displays:\n"));
      MosPrintf(MIL_TEXT("---------\n"));
      for (pDispIterator = DisplayList.begin(); pDispIterator!= DisplayList.end(); pDispIterator++)
         {
         MIL_INT DisplayCount, DisplaySkipCount;
         MIL_DOUBLE DisplayRefreshRate;
         CDisplay *pDisp   = *pDispIterator;
         pDisp->GetStatistic(&DisplayRefreshRate, &DisplayCount, &DisplaySkipCount);
         CDigitizer *pDig = (CDigitizer *) pDisp->GetDisplaySource();

         MosPrintf(MIL_TEXT("%4d (%4d x %4d @ %.1f Hz) "), pDisp->Index(), pDisp->SizeX(), pDisp->SizeY(), DisplayRefreshRate);
         if(pDig)
            {
            MosPrintf(MIL_TEXT("displaying from dig %d, Frame: %5lld (skip: %3lld). "),
               (int)pDig->Index(), (long long)DisplayCount, (long long)DisplaySkipCount);
            }
         else
            MosPrintf(MIL_TEXT("not displaying."));

         MosPrintf(MIL_TEXT("\n"));

         // Display the selected input on all outputs.
         if(Key >= '0' && Key <= '7')
            {
            MIL_UINT Index = Key - '0';
            if(Index < DigitizerList.size() && Index >= 0)
               {
               CDigitizer *pDig = DigitizerList.at(Index);
               pDisp->SetDisplaySource((MIL_INT64)pDig, pDig->SizeX(), pDig->SizeY());
               }
            }

         // Display one input per output.
         if(Key == 'h')
            {
            if((MIL_UINT)pDisp->Index() < DigitizerList.size())
               {
               CDigitizer *pDig = DigitizerList.at(pDisp->Index());
               pDisp->SetDisplaySource((MIL_INT64)pDig, pDig->SizeX(), pDig->SizeY());
               }
            else
               {
               pDisp->SetDisplaySource(0, 0, 0);
               }
            }
         if(Key == 'r')
            pDisp->ResetStatistic();

         // Enable/disable Direct3D effects.
         if(Key == 'd')
            pDisp->D3DEffect(!pDisp->D3DEffect());

         // Calculate the latency between the output and the input.
         if(Key == 'l')
            {
            MIL_INT        InputMode;

            for (pDigIterator = DigitizerList.begin(); pDigIterator!= DigitizerList.end(); pDigIterator++)
               {
               CDigitizer *pDig = *pDigIterator;
               MdigInquire(pDig->Digitizer(), M_INPUT_MODE, &InputMode);
               if(InputMode == M_DIGITAL)
                  {
                  FoundDviDigitalSource = true;
                  break;
                  }
               }

            if(FoundDviDigitalSource)
               pDisp->Latency(!pDisp->Latency());
            }
         }

      if((Key == 'l') && (FoundDviDigitalSource == false))
         MosPrintf(MIL_TEXT("\nCan not calculate the latency, no DVI-D input sources detected."));

      // Display/Overlay a selected input on a selected output.
      if(Key == 's' || Key == 'o')
         {
         CDigitizer *pDig = NULL;
         CDisplay *pDisp  = NULL;
         bool IsSelect = (Key == 's')? true: false;

         MIL_INT DispIndex = 0;
         MIL_INT DigIndex = 0;

         MosPrintf(MIL_TEXT("\nSelect display number (0 to %d): \n"),  (int) (DisplayList.size() ? DisplayList.size() - 1 : 0));
         Key = MosGetch();
         DispIndex = Key - '0';

         if((UINT)DispIndex < DisplayList.size() && DispIndex >= 0)
            {
            pDisp = DisplayList.at(DispIndex);

            MosPrintf(MIL_TEXT("Select digitizer number to be outputed on display %d: (0 to %d): \n"), (int)DispIndex, (int) DigitizerList.size() - 1);
            Key = MosGetch();
            DigIndex = Key - '0';

            if((UINT)DigIndex < DigitizerList.size() && DigIndex >= 0)
               pDig = DigitizerList.at(DigIndex);

            if(IsSelect)
               {
               if(pDig)
                  pDisp->SetDisplaySource((MIL_INT64)pDig, pDig->SizeX(), pDig->SizeY());
               else
                  pDisp->SetDisplaySource(0 , 0, 0);
               }
            else
               {
               if(pDig)
                  pDisp->SetDisplayOverlaySource((MIL_INT64)pDig, pDig->SizeX(), pDig->SizeY());
               else
                  pDisp->SetDisplayOverlaySource(0 , 0, 0);
                }

            MosPrintf(MIL_TEXT("\n"));
            }
         }
      MosPrintf(MIL_TEXT("\n"));
      }   

end:

   /////////////////////////////////////////
   // Exiting.
   /////////////////////////////////////////
   MosPrintf(MIL_TEXT("Stopping grabs.\n"));
   for(pDigIterator = DigitizerList.begin(); pDigIterator!= DigitizerList.end(); pDigIterator++)
      {
      CDigitizer *pDig = *pDigIterator;
      pDig->Stop();
      }

   MosPrintf(MIL_TEXT("Freeing displays.\n"));
   while(!DisplayList.empty())
      {
      CDisplay *pDisp  = DisplayList.back();
      DisplayList.pop_back();
      delete pDisp;
      }

   MosPrintf(MIL_TEXT("Freeing digitizers.\n"));
   while(!DigitizerList.empty())
      {
      CDigitizer *pDig  = DigitizerList.back();
      DigitizerList.pop_back();
      delete pDig;
      }

   MosPrintf(MIL_TEXT("Freeing systems.\n"));
   for(SystemIndex = 0; SystemIndex < MAX_SYSTEMS; SystemIndex++)
      {
      if(MilSystem[SystemIndex])
         MsysFree(MilSystem[SystemIndex]);
      }

   MappFree(MilApplication);
   g_pD3D->Release();

   ::InvalidateRect(hForegroundWindow, NULL, TRUE);
   return 0;
   }


//////////////////////////
// Grab callback function.
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   CDigitizer *pDig = (CDigitizer *)HookDataPtr;
   CDisplay   *pDisp   = NULL;
   vector<CDisplay *> *pDisplayList;

   MIL_ID ModifiedBufferId;
   pDisplayList = (vector<CDisplay *> *)pDig->GetUserData();

   if(pDisplayList->empty())
      return 0;

   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);
   
   MIL_ID DestBuffer = pDig->GetWorkBuffer();

   MbufCopy(ModifiedBufferId, DestBuffer);

   // Loop all displays to find the displays that output this source.
   vector<CDisplay *>::iterator pDispit;
   for (pDispit = pDisplayList->begin(); pDispit!= pDisplayList->end(); pDispit++)
      {
      CDisplay *pDisp = *pDispit;

      if(pDisp->UpdateLatency(ModifiedBufferId))
         pDig->SetSourceId((MIL_INT64)pDisp);

      // We found a display that outputs this source. Insert buffer in the display queue.
      if(pDisp->GetDisplaySource() == (MIL_INT64)pDig)
         pDisp->DisplayBuffer(DestBuffer);

      // We found a display that outputs on the overlay this source. 
      // Insert buffer in the overlay display queue.
      if(pDisp->GetDisplayOverlaySource() == (MIL_INT64)pDig)
         pDisp->DisplayOverlayBuffer(DestBuffer);
      }
   return 0;
   }
/* Verify whether this example can run on the selected system. */
bool SystemSupportsCrossBar(MIL_ID MilSystem)
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
   MosGetch();
   return false;
   }

