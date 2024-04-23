/***************************************************************************************/
/*
 * File name: MultiCameraDisplay.cpp
 *
 * Synopsis:  This program detects all the cameras attached to all the installed
 *            Matrox systems and starts grabbing from them using MdigProcess().
 *
 *            This example requires a graphic card supporting OpenGL 3.0 or higher.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#define M_MIL_USE_OS_SCREEN_FUNCTIONS 1

#include <mil.h>

 //****************************************************************************
 // Example description.
 //****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("MultiCameraDisplay\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This program detects all the cameras attached to all the installed\n")\
      MIL_TEXT("Matrox systems and starts grabbing from them using MdigProcess().\n\n")\
      MIL_TEXT("Features include:\n")\
      MIL_TEXT("   - Displaying multiple live streams from multiple boards.\n")\
      MIL_TEXT("   - No tearing video output.\n")\
      MIL_TEXT("   - Low latency video output.\n")\
      MIL_TEXT("   - Live camera addition and removal.\n")\
      MIL_TEXT("   - Changing the display between windowed and full screen mode.\n")\
      MIL_TEXT("   - Changing grab buffer pixel formats.\n")\
      MIL_TEXT("   - Activating image processing on a live stream.\n")\
      MIL_TEXT("   - Activating H264 encoding on a live stream.\n")\
      MIL_TEXT("   - Displaying the Feature browser so that the user can control the digitizer\n")\
      MIL_TEXT("     and camera settings.\n")\
      MIL_TEXT("\n\n")\
      MIL_TEXT("Press <Enter> to start.\n\n")\
   );
   MosScreenRefresh();
   }

// Function to print the list of commands.
void printCommands()
   {
   MosPrintf(MIL_TEXT("Matrox MultiCameraDisplay\n")\
      MIL_TEXT("-------------------------\n\n")\
      MIL_TEXT("Cameras can be added or removed at any time.\n\n")\
      MIL_TEXT("Commands on a specific camera(s):\n")\
      MIL_TEXT("---------------------------------\n")\
      MIL_TEXT("  <a> to activate image processing.\n")\
      MIL_TEXT("  <e> to activate H264 encoding.\n")\
      MIL_TEXT("  <b> to open the feature browser.\n")\
      MIL_TEXT("  <d> to free a camera.\n")\
      MIL_TEXT("  <p> to change the pixel format of the grab buffers.\n")\
      MIL_TEXT("  <t> to toggle the display of information in the overlay.\n\n")\
      MIL_TEXT("Commands on window:\n")\
      MIL_TEXT("-------------------\n")\
      MIL_TEXT("  <f> to switch between full-screen and windowed mode.\n")\
      MIL_TEXT("  <g> to switch the display render source.\n")\
      MIL_TEXT("  <r> to rearrange the tiles on the display.\n")\
      MIL_TEXT("  <s> to toggle scaling between fit_to_screen or no-scaling.\n\n")\
      MIL_TEXT("Other commands:\n")\
      MIL_TEXT("---------------\n")\
      MIL_TEXT("  <n> to auto detect new cameras.\n")\
      MIL_TEXT("  <q> to quit.\n\n")\
      MIL_TEXT("Camera(s):\n")\
      MIL_TEXT("--------\n")
   );
   MosScreenRefresh();
   }

#include <set>
#include <algorithm>
using namespace std;
#include "../DisplayGL/C++/displayGLexport.h"
#include "MdigHandler.h"

#if M_MIL_USE_WINDOWS
// windows.h included only for moving the cursor.
#include <Windows.h>
#endif

// This structure contains all the information of the allocated systems, displays and cameras (digitizers).
typedef struct _SYSTEM_DATA
   {
   // This structure is used to allocate the camera detection thread.
   typedef struct
      {
      _SYSTEM_DATA *pSystem;
      MIL_ID systemID;
      MIL_ID threadcameradetectId;
      } CAMERA_DETECT_PARAM;

   _SYSTEM_DATA()
      {
      _mutex = M_NULL;
      }

   ~_SYSTEM_DATA()
      {
      Free();
      }

   void Free()
      {
      // Stop all the grabs.
      for (auto dig_iter = _digitizers.begin(); dig_iter != _digitizers.end(); ++dig_iter)
         (*dig_iter)->StopGrab();

      // Delete the digitizers.
      for (auto dig_iter = _digitizers.begin(); dig_iter != _digitizers.end(); ++dig_iter)
         delete *dig_iter;
      _digitizers.clear();

      // Free the display.
      if (_display)
         _display->Release();
      _display = NULL;

      // Free the mutex.
      if (_mutex)
         MthrFree(_mutex);
      _mutex = M_NULL;

      // Free the systems.
      for (auto iter_system = _systemIDs.begin(); iter_system != _systemIDs.end(); ++iter_system)
         MsysFree(*iter_system);
      _systemIDs.clear();
      }

   // container of allocated MIL systems.
   vector<MIL_ID> _systemIDs;

   // container of allocated cameras.
   list<CMILDigitizerHandler *> _digitizers;

   // display ptr. We have one display for all the cameras. Each camera is displayed in a tile (small window) on the display.
   IMilDisplayEx*  _display;

   // thread ID of the camera detect thread. 
   vector<CAMERA_DETECT_PARAM> _threadcameradetects;

   // serialization when modifying elements in the containers.
   MIL_ID _mutex;

   } SYSTEM_DATA;


void ProcessUserInput(MIL_INT keyPressed, SYSTEM_DATA &systemData, bool &sortAndRearrangeDisplay); // Process user keyboard input selection.
bool compare_digitizers_for_sorting(const CMILDigitizerHandler *first, const CMILDigitizerHandler *second); // for sorting cameras in alphabetic order.
MIL_INT MFTYPE CamPresentFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr); // callback function of MsysHookFunction(M_CAMERA_PRESENT).
void StartCameraDetectionThreads(SYSTEM_DATA &systemData, bool wait); // This function starts a camera detect thread that will try to allocate all the digitizers on the system. Then the thread will terminate.
void FreeCameraDetectionThreads(_SYSTEM_DATA &SystemData);  // This functions frees objects allocated by the StartCameraDetectionThread().

// helper class to lock a mutex on the stack.
class MilMutexLockGuard
   {
   public:
      MilMutexLockGuard(MIL_ID mutex)
         {
         _mutex = mutex;
         MthrControl(_mutex, M_LOCK, M_DEFAULT);
         }
      ~MilMutexLockGuard()
         {
         MthrControl(_mutex, M_UNLOCK, M_DEFAULT);
         }
      MIL_ID _mutex;
   };

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MosScreenResize(44, 80);

   MosScreenInit();

   PrintHeader();

   MIL_ID MilApplication;
   SYSTEM_DATA _systemData; // Contains all the information on the allocated systems, digitizers and display.

   MappAlloc(M_DEFAULT, &MilApplication);
   MappControl(M_ERROR, M_PRINT_DISABLE);

   MIL_INT NbAvailableSystems = 0;
   MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_COUNT, &NbAvailableSystems);

   // Loop for all installed MIL systems.
   MIL_STRING systemDescriptor;
   vector<MIL_STRING> excluded_systems;
   excluded_systems.push_back(MIL_TEXT("M_SYSTEM_HOST"));
   excluded_systems.push_back(MIL_TEXT("M_SYSTEM_GENTL"));
   for(MIL_INT i = 0; i < NbAvailableSystems; i++)
      {
      MappInquire(M_DEFAULT, M_INSTALLED_SYSTEM_DESCRIPTOR + i, systemDescriptor);
      // skip systems in excluded list.
      if (std::find(excluded_systems.begin(), excluded_systems.end(),  systemDescriptor) == excluded_systems.end())
         {
         MIL_ID milSystem = M_NULL;
         MIL_INT sysdevnum = 0;

         // Allocated all the systems device numbers we can.
         do
            {
            MsysAlloc(systemDescriptor.c_str(), M_DEV0 + sysdevnum, M_DEFAULT, &milSystem);
            if (milSystem == M_NULL)
               break;
            // This example is not supported on a DMIL system (Distributed MIL).
            if (MsysInquire(milSystem, M_LOCATION, M_NULL) == M_REMOTE)
               {
               MsysFree(milSystem);
               break;
               }

            _systemData._systemIDs.push_back(milSystem);
            sysdevnum++;
            } while (milSystem);
         }
      }

   MosGetch();

      // If we found no systems then use the host system.
   if (_systemData._systemIDs.size() == 0)
      {
      MIL_ID milSystem = M_NULL;
      MsysAlloc(M_SYSTEM_HOST, M_DEV0, M_DEFAULT, &milSystem);
      _systemData._systemIDs.push_back(milSystem);
      }

   // Allocate synchronization mutex.
   MthrAlloc(M_DEFAULT_HOST, M_MUTEX, M_DEFAULT, M_NULL, M_NULL, &_systemData._mutex);

   // Allocate a display.
   _systemData._display = GetMilDisplayEx("Matrox MultiCameraDisplay", 0, 0);
   _systemData._display->SetRenderSource(eRenderFromGrabCallBack);
   _systemData._display->OpenWindow();

   // Register a hook function on each system camera present event.
   // Used to:
   // 1- Allocate and start acquisition on a newly attached camera.
   // 2- Stop acquisition on a camera that has been removed.
   // 3- Resume acquisition on a camera that has been re-connected.
   // Some systems do not support the camera present hook, this will generate an error which we ignore.
   for (auto iter_system = _systemData._systemIDs.begin(); iter_system != _systemData._systemIDs.end(); ++iter_system)
      MsysHookFunction(*iter_system, M_CAMERA_PRESENT, CamPresentFunction, &_systemData);

   // Start the camera detect thread. While it is detecting cameras, we can start grabbing on the ones that are found.
   StartCameraDetectionThreads(_systemData, false);

   // Start the main loop.
   MIL_INT keyPressed = 0;
   bool sortCameraListInConsole = true;
   MIL_DOUBLE startTime, currentTime;
   MappTimer(M_TIMER_READ, &startTime);
   auto lastdigitizerCount = _systemData._digitizers.size();
   while (keyPressed != 'q')
      {
      // Slow down loop.
      MosSleep(50);

      // First lets check if the display had been closed by the user.
      if (_systemData._display->IsWindowClosing())
         break; // Exit.

      // Poll for events on the Window thread. Used for user inputs on the window (mouse, keyboard, etc).
      _systemData._display->PollEvents();

      // Print statistics in console.
      // Process input keys.
      MappTimer(M_TIMER_READ, &currentTime);
      if (keyPressed || ((currentTime - startTime) > 1.0))
         {
         startTime = currentTime;

         // Process user key inputs.
         if (keyPressed)
            {
            MosScreenScroll(TRUE);
            ProcessUserInput(keyPressed, _systemData, sortCameraListInConsole);
            keyPressed = 0;
            }

         // If a camera was added or removed, clean up the display.
         if (lastdigitizerCount != _systemData._digitizers.size())
            {
            sortCameraListInConsole = true;
            lastdigitizerCount = _systemData._digitizers.size();
            }

         // Reorder the list of cameras by name.
         if (sortCameraListInConsole)
            {
            MilMutexLockGuard Lock(_systemData._mutex);
            _systemData._digitizers.sort(compare_digitizers_for_sorting);
            MosScreenClear();
            printCommands();
            sortCameraListInConsole = false;
            }

         // Print camera descriptions starting at line 27 (after the list of commands).
         int i = 0;
         MosScreenSetPosition(0,27);
         {
         MilMutexLockGuard Lock(_systemData._mutex);

         for (auto dig_iter = _systemData._digitizers.begin(); dig_iter != _systemData._digitizers.end(); ++dig_iter)
            {
            auto pdig = *dig_iter;
            auto bufferFormat = pdig->GetPixelFormatString();

            MIL_STRING processing;
            if (pdig->IsProcessing())
               processing = MIL_TEXT("proc ");
            if (pdig->IsEncoding())
               processing += MIL_TEXT("encoding ");

            MIL_STRING description = pdig->GetInputDescription();
            description.resize(20, ' ');

            MIL_STRING_STREAM sstats;
            sstats << MIL_TEXT(" ") << i++ << MIL_TEXT(": ") << description << MIL_TEXT(" (") << bufferFormat << MIL_TEXT("): ") << processing << pdig->GetGrabStats();
            MIL_STRING stat = sstats.str();
            stat.resize(79, ' '); // limit MIL_STRING size to display properly in 80 col.
            MosPrintf(MIL_TEXT("%s\n"), stat.c_str());
            MosScreenRefresh();
            }
         }

         // Display render source.
         // When rendering from an independent thread, the rendering rate is controlled by display refresh rate. 
         // When rendering from the grab callback then the rendering follows the rate of the camera. The grab callback is from the camera selected
         // on the main window. This selection should be used when using AMD FreeSync technology.
         auto renderSource = _systemData._display->GetRenderSource();
         if (renderSource == eRenderFromThread)
            MosPrintf(MIL_TEXT("\nDisplay rendered from independent thread (rendering at display rate).     \n"));
         else
            MosPrintf(MIL_TEXT("\nDisplay rendered from grab callback (rendering at frame rate).            \n"));
         MosScreenRefresh();
         
         // Display latency statistics if enabled.
         // The calculation of the latency is only possible when a display output is connected on the input of a Matrox Clarity UHD
         // and the display is in full screen on that display.
         // When this condition is met, the latency calculation is automatically started and the statistics printed.
         {
         int latencyCount = 0;
         int latencyDropCount = 0;
         double curavgLatency_ms = 0;
         double avgLatency_ms = 0;
         const char *latencySrc = NULL;
         const char *latencyDest = NULL;
         if (_systemData._display->LatenciesGet(&latencySrc, &latencyDest, curavgLatency_ms, latencyCount, latencyDropCount, avgLatency_ms))
            {
            MosPrintf(MIL_TEXT("\nLatency in ms between input %s and display %s:\n"), latencySrc, latencyDest);
            MosPrintf(MIL_TEXT("  Avg latency of %.2f (cur: %.2f) on %d grabbed frames, %d frame(s) drop.      \n"), avgLatency_ms, curavgLatency_ms, latencyCount, latencyDropCount);
            }
         }        
         }
      MosScreenScroll(FALSE);
      
      // Check if a key is pressed.
      if (MosKbhit())
         keyPressed = tolower((int)MosGetch());
      }

   MosPrintf(MIL_TEXT("\nExiting.\n"));

   // Free thread objects allocated by the StartCameraDetectionThread().
   FreeCameraDetectionThreads(_systemData);

   // Unhook the camera present callback function.
   // Some systems do not support the camera present hook, this will generate an error which we ignore.
   for (auto iter_system = _systemData._systemIDs.begin(); iter_system != _systemData._systemIDs.end(); ++iter_system)
      MsysHookFunction(*iter_system, M_CAMERA_PRESENT + M_UNHOOK, CamPresentFunction, &_systemData);

   // Free everything in the system.
   _systemData.Free();
   
   MosScreenRelease();
   MappFree(MilApplication);
   return 0;
   }

// This function is used for sorting the digitizers in alphabetic order.
bool compare_digitizers_for_sorting(const CMILDigitizerHandler *first, const CMILDigitizerHandler *second)
   {
   if (*first < *second)
      return true;
   return false;
   }

// This thread is called to search for allocated cameras on a particuliar system.
// It is used at the start of the process to start grabbing immediately when 
// at least one camera is found. The grab starts before all cameras are found.
MIL_UINT32 MFTYPE CameraDetectThread(void *pCAMERA_DETECT_PARAM_VOID)
   {
   _SYSTEM_DATA::CAMERA_DETECT_PARAM *pcameraDetectParam = (_SYSTEM_DATA::CAMERA_DETECT_PARAM *)pCAMERA_DETECT_PARAM_VOID;

   if (pcameraDetectParam)
      {
      MIL_INT numberOfDigitizers = 0;
      CmilDigitizerFactory dig_factory;
      auto psystem = pcameraDetectParam->pSystem;

      // loop on the number of digitizers available on this system.
      MsysInquire(pcameraDetectParam->systemID, M_DIGITIZER_NUM, &numberOfDigitizers);
      for (int i = 0; i < numberOfDigitizers; i++)
         {
         auto pdig = dig_factory.AllocateMILDigHandler(pcameraDetectParam->systemID, i);
         if (pdig->DigAlloc())
            {
            // A camera is present on this digitizer... use it.
            MilMutexLockGuard Lock(psystem->_mutex);
            psystem->_digitizers.push_back(pdig);
            pdig->SetDisplay(psystem->_display);
            pdig->StartGrab();
            }
         else
            delete pdig;
         }
      // wait a little then rearrange the new grabs (tiles) on the screen so it will look nice.
      psystem->_display->RearrangeTiles(eSIDE_BY_SIDE_BOTTOM);
      }

   return 0;
   }

// This function starts a camera detect thread (one per system) that will try to allocate all the digitizers on the system. Then the threads will terminate.
void StartCameraDetectionThreads(_SYSTEM_DATA &SystemData, bool wait)
   {
   // wait for the camera detect thread to finish.
   for (auto itcamDetect = SystemData._threadcameradetects.begin(); itcamDetect != SystemData._threadcameradetects.end(); ++itcamDetect)
      {
      MthrWait(itcamDetect->threadcameradetectId, M_THREAD_END_WAIT, M_NULL);
      MthrFree(itcamDetect->threadcameradetectId);
      }
   SystemData._threadcameradetects.clear();

   // If the mutex is not yet allocated, allocate it.
   if (SystemData._mutex == M_NULL)
      MthrAlloc(M_DEFAULT_HOST, M_MUTEX, M_DEFAULT, M_NULL, M_NULL, &SystemData._mutex);

   // We allocate a camera detect thread per system.
   for (auto itsystem = SystemData._systemIDs.begin(); itsystem != SystemData._systemIDs.end(); ++itsystem)
      {
      _SYSTEM_DATA::CAMERA_DETECT_PARAM param = { &SystemData, *itsystem, 0 };
      SystemData._threadcameradetects.push_back(param);
      }

   // Start all the threads.
   for (auto itcamDetect = SystemData._threadcameradetects.begin(); itcamDetect != SystemData._threadcameradetects.end(); ++itcamDetect)
      {
      auto &cameraDetectParam = *itcamDetect;
      MthrAlloc(M_DEFAULT_HOST, M_THREAD, M_DEFAULT, &CameraDetectThread, &cameraDetectParam, &cameraDetectParam.threadcameradetectId);
      }

   if (wait)
      FreeCameraDetectionThreads(SystemData);
   }

// Free the objects allocated when starting the CameraDetectionThread().
void FreeCameraDetectionThreads(_SYSTEM_DATA &SystemData)
   {
   // wait for the camera detect thread to finish.
   for (auto itcamDetect = SystemData._threadcameradetects.begin(); itcamDetect != SystemData._threadcameradetects.end(); ++itcamDetect)
      {
      MthrWait(itcamDetect->threadcameradetectId, M_THREAD_END_WAIT, M_NULL);
      MthrFree(itcamDetect->threadcameradetectId);
      }
   SystemData._threadcameradetects.clear();
   }

// Callback function of MsysHookFunction(M_CAMERA_PRESENT) called when a camera is plugged or unplugged.
MIL_INT MFTYPE CamPresentFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   MIL_ID milsystem;
   MobjInquire(HookId, M_OWNER_SYSTEM, &milsystem);

   if (HookType == M_CAMERA_PRESENT)
      {
      auto &systemData = *(SYSTEM_DATA *)HookDataPtr;
      MilMutexLockGuard Lock(systemData._mutex);

      MIL_INT isCamPresent = 0;
      MIL_INT digitizerDeviceNbr = 0;
      auto &digitizers = systemData._digitizers;

      // Inquire the camera present state (present or not present).
      MsysGetHookInfo(milsystem, HookId, M_CAMERA_PRESENT, &isCamPresent);

      // Inquire the camera's digitizer device number.
      MsysGetHookInfo(milsystem, HookId, M_NUMBER, &digitizerDeviceNbr);

      // Find if the camera is already allocated.
      CMILDigitizerHandler *pcurrentDig = NULL;
      auto iter_dig = digitizers.begin();
      for (; iter_dig != digitizers.end(); ++iter_dig)
         {
         if (((*iter_dig)->GetSysId() == milsystem) && ((*iter_dig)->GetDevNum() == digitizerDeviceNbr))
            {
            // The camera already exists.
            pcurrentDig = *iter_dig;
            break;
            }
         }

      // Is this a hook of camera being detected?
      if (isCamPresent)
         {
         // The camera is already allocated and we receive a hook of camera present... reallocate it.
         if (pcurrentDig)
            {
            pcurrentDig->DigFree();
            if (pcurrentDig->DigAlloc())
               {
               pcurrentDig->StartGrab();
               }
            else
               {
               // cannot allocate digitizer (no camera) so remove it.
               delete pcurrentDig;
               digitizers.erase(iter_dig);
               }
            }
         else
            {
            // This is a new camera.
            CmilDigitizerFactory dig_factory;
            // This is a new camera, allocate it. The dig will be allocated in the main loop.
            auto dig = dig_factory.AllocateMILDigHandler(milsystem, M_DEV0 + digitizerDeviceNbr);
            if (dig && dig->DigAlloc())
               {
               // A camera is present on this digitizer... use it.
               digitizers.push_back(dig);
               dig->SetDisplay(systemData._display);
               dig->StartGrab();
               }
            }
         }
      // The camera is disconnected.
      else if (pcurrentDig)
         {
         delete pcurrentDig;
         digitizers.erase(iter_dig);
         }
      }
   return 0;
   }

// Process user keyboard input selection.
void ProcessUserInput(MIL_INT keyPressed, SYSTEM_DATA &systemData, bool &sortAndRearrangeDisplay)
   {
   // Process commands that are performed on the display.
   auto &digitizers = systemData._digitizers;
   decltype(systemData._digitizers) supportedDigitizers;

   PIXEL_FORMAT pixelFormat = eMono8; // Pixel to set (only used when key 'p' is pressed).

   sortAndRearrangeDisplay = true;
   switch (keyPressed)
      {
      // Process commands that are performed on a specific camera.
         case 'a': // toggle image processing.
         case 'd': // free the digitizer.
         case 'e': // toggle encoding.
         case 'b': // feature browser.
         case 't': // toggle text overlay.
         case 'p': // pixel format.
            {
            // pixel format
            if (keyPressed == 'p')
               {
               std::set<PIXEL_FORMAT> pixelFormats;

               // populate list of supported pixel formats.
               for (auto iter_dig = digitizers.begin(); iter_dig != digitizers.end(); ++iter_dig)
                  {
                  auto dig = *iter_dig;
                  auto digpixelFormats = dig->SupportedPixelFormats();
                  for (auto iter_pixelformats = digpixelFormats.begin(); iter_pixelformats != digpixelFormats.end(); ++iter_pixelformats)
                     pixelFormats.insert(*iter_pixelformats);
                  }

               int i = 0;
               MosPrintf(MIL_TEXT("\n\nSelect pixel format: \n"));
               for (auto iter_pf = pixelFormats.begin(); iter_pf != pixelFormats.end(); ++iter_pf)
                  {
                  MIL_STRING_STREAM pixelformat;
                  pixelformat << i++ << ": " << GetPixelFormatName(PfncFormat(*iter_pf)) << "\t\t" << GetPixelFormatDescription(PfncFormat(*iter_pf)) << std::endl;
                  MosPrintf(MIL_TEXT("%s"),pixelformat.str().c_str());
                  }
               MosScreenRefresh();
               auto secondKey = tolower((int)MosGetch());
               auto pixelFormatNumber = int(secondKey - '0');
               if (pixelFormatNumber >= 0 && pixelFormatNumber < int(pixelFormats.size()))
                  pixelFormat = PIXEL_FORMAT(*std::next(pixelFormats.begin(), pixelFormatNumber));
               }

            // Inquire which camera to perform the command.
            CMILDigitizerHandler *pselectedCamera = NULL;

            bool allCameras = false;
            if (digitizers.size() > 1)
               {
               MosPrintf(MIL_TEXT("\nSelect camera number (0 - %d) or 'a' for all cameras: "), (int)digitizers.size() - 1);
               MosScreenRefresh();
               MIL_INT Key = tolower((int)MosGetch());
               int cameraNumber = int(Key - '0');
               if (Key == 'a')
                  allCameras = true;
               else if (cameraNumber >= 0 && (cameraNumber < int(digitizers.size())))
                  {
                  MilMutexLockGuard Lock(systemData._mutex);

                  // Get the selected camera.
                  auto it = digitizers.begin();
                  std::advance(it, cameraNumber);
                  pselectedCamera = *it;
                  }
               }
            else if (digitizers.size() == 1)
               {
               // Only one camera, do not ask the user which camera when we only have one.
               allCameras = true;
               }

            // Is the selected camera valid?
            if (pselectedCamera || allCameras)
               {
               MilMutexLockGuard Lock(systemData._mutex);
               auto iter = digitizers.begin();
               while (iter != digitizers.end())
                  {
                  auto current = iter++;
                  auto dig = *current;
                  if (allCameras || pselectedCamera == dig)
                     {
                     if (keyPressed == 'd') // Free the digitizer.
                        {
                        delete dig;
                        digitizers.erase(current);
                        }
                     else if (keyPressed == 'b') // Open the feature browser.
                        MdigControl(dig->GetDigId(), M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
                     else if (keyPressed == 'p') // Change pixel format.
                        dig->SetPixelFormat(pixelFormat);
                     else if (keyPressed == 'a') // activate image processing.
                        dig->SetProcessing(!dig->IsProcessing());
                     else if (keyPressed == 'e') // activate image encoding.
                        {
                        // Do we have an encoding license?
                        if (!(MappInquire(M_DEFAULT, M_LICENSE_MODULES, M_NULL) & M_LICENSE_JPEGSTD))
                           {
                           MosPrintf(MIL_TEXT("Sorry, no encoding license present. Press <Enter> to continue.\n"));
                           MosScreenRefresh();
                           MosGetch();
                           }
                        else
                           {
                           dig->SetEncoding(!dig->IsEncoding());
                           }
                        }
                     else if (keyPressed == 't') // toggle display of text overlay.
                        {
                        if (dig->GetOverlayText().size())
                           dig->SetOverlayText(MIL_TEXT(""));
                        else
                           dig->SetOverlayText(dig->GetInputDescriptionBrief());
                        }
                     }
                  }
               }
            }
            break;

         case 'r':
            // Rearrange the tiles on the display.
            systemData._display->RearrangeTiles(eNEXTPATTERN);
            break;

         case 'g':
            // Toggle the display render thread (render from grab callback or from independent thread).
            {
            auto renderSource = systemData._display->GetRenderSource();
            if (renderSource == eRenderFromThread)
               systemData._display->SetRenderSource(eRenderFromGrabCallBack);
            else
               systemData._display->SetRenderSource(eRenderFromThread);
            }
            break;

         case 'f':
            // Switch the window between full-screen and windowed mode.
            {
            int monitorcount = systemData._display->GetMonitorCount();
            MosPrintf(MIL_TEXT("\nSelect monitor number to display window (0 - %d): \n"), monitorcount - 1);

            MosPrintf(MIL_TEXT("0: Windowed mode\n"));
            for (int i = 1; i < monitorcount; i++)
               {
               MIL_STRING_STREAM monitorstr;
               monitorstr << i << ": " << systemData._display->GetMonitorName(i) << std::endl;
               MosPrintf(MIL_TEXT("%s"),monitorstr.str().c_str());
               }
            MosScreenRefresh();
            auto secondKey = tolower((int)MosGetch());
            auto monitorNumber = int(secondKey - '0');
            if((monitorNumber >= 0) & (monitorNumber < monitorcount))
               systemData._display->SetWindowMonitor(monitorNumber);
            }
            break;

         case 's':
            // Switch scale to fit window.
            {
            auto FitToScreen = systemData._display->GetScalingFitToWindow();
            FitToScreen = !FitToScreen;
            systemData._display->SetScalingFitToWindow(FitToScreen);
            }
            break;

         case 'n':
            {
            // Restart the camera detect thread.
            StartCameraDetectionThreads(systemData, false);
            MosPrintf(MIL_TEXT("\nDetecting new cameras... please wait...\n"));
            }
            break;

         default:
            // invalid selection... do nothing.
            sortAndRearrangeDisplay = false;
      }
   }

