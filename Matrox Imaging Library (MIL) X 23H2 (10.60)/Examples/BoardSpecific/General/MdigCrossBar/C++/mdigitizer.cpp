/********************************************************************************/
/* 
* File name: Mdigitizer.cpp
*
* Synopsis:  This class manages a digitizer.
*            
*            It manages:
*             - The allocations of the grab buffers.
*             - The drawing of the moving line.
*             - The starting and stopping of a grab stream.
*             - The statistics.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <windows.h> 
#include <mil.h>
#include <queue>
#include <list>
#include "mdigitizer.h"


// Private.
// Initialisation.
void CDigitizer::Init(bool iAll)
   {
   m_IsAllocated = false;
   m_IsDeviceUsed = false;
   m_CameraPresent = false;
   
   m_MilDigitizer = M_NULL;
   m_DetectDCFName.clear();
   memset(m_MilImageWork, 0, sizeof(m_MilImageWork));
   memset(m_MilBuffers, 0, sizeof(m_MilBuffers));
   m_GrabCount = 0;
   m_Index = 0;
   m_SizeX = 0;
   m_SizeY = 0;
   m_EnableMovingLine = true;
   m_GraLinePos = 0;
   m_GraLineDir = 0;
   m_LatencyFromDispIndex = -1;
   m_GrabHalfFrameRate = false;
   m_Decimation = 1;
   m_SourceId = 0;

   if(iAll)
      {
      m_MilSystem  = M_NULL;
      m_MilDitizerMinimal = M_NULL;
      m_DigDeviceNumber = M_INVALID;
      m_Channel = M_INVALID;
      m_DCFName.clear();
      m_ProcessingFunctionPtr = M_NULL;
      m_PrivateData = NULL;
      }

   }


// Private.
// Allocates the digitizer.
bool CDigitizer::AllocateDigitizer(MIL_ID iMilSystem, MIL_INT DevNum, const MIL_STRING& DCFName)
   {
   MIL_ID MilDigitizer;
   MdigAlloc(iMilSystem, DevNum, DCFName, M_DEFAULT, &MilDigitizer);
   if(MilDigitizer)
      {
      if(m_Channel != M_CH0)
         Sleep(250);

      MdigInquire(MilDigitizer, M_CAMERA_PRESENT, &m_CameraPresent);
      if(m_CameraPresent)
         {
         m_MilDigitizer = MilDigitizer;
         MdigInquire(m_MilDigitizer, M_SIZE_X, &m_SizeX);
         MdigInquire(m_MilDigitizer, M_SIZE_Y, &m_SizeY);
         }
      else
         MdigFree(MilDigitizer);
      }
   return m_MilDigitizer? true:false;
   }

// Private.
// Allocates the grab buffers.
bool CDigitizer::AllocateBuffers(MIL_INT iIndex)
   {
   MIL_INT ReturnValue = 0;
   m_Index = iIndex;
   m_IsAllocated = false;

   if(m_MilDigitizer)
      {
      MdigControl(m_MilDigitizer, M_GRAB_TIMEOUT, 5000);

      for(MIL_INT Buf = 0; Buf < MAX_BUFFERING; Buf++)
         {
         MbufAllocColor(m_MilSystem,
            3,
            m_SizeX,
            m_SizeY,
            8 + M_UNSIGNED,
            M_IMAGE + M_GRAB + M_BGR32 + M_PACKED,
            &m_MilBuffers[Buf]);

         MbufAllocColor(M_DEFAULT_HOST,
            3,
            m_SizeX,
            m_SizeY,
            8 + M_UNSIGNED,
            M_IMAGE + M_YUV16 + M_PACKED + M_HOST_MEMORY,
            &m_MilImageWork[Buf]);
         }

      m_IsAllocated = true;
      }

   return m_IsAllocated;
   }

// Private.
// Frees everything.
void CDigitizer::Free(bool iAll)
   {
   if(m_MilDigitizer)
      {
      MdigFree(m_MilDigitizer);
      for(MIL_INT Buf = 0; Buf < MAX_BUFFERING; Buf++)
         {
         if(m_MilBuffers[Buf])
            {
            MbufFree(m_MilBuffers[Buf]);
            m_MilBuffers[Buf] = M_NULL;
            }

         if(m_MilImageWork[Buf])
            {
            MbufFree(m_MilImageWork[Buf]);
            m_MilImageWork[Buf];
            }
         }
      }

   if(iAll)
      {
      if(m_MilDitizerMinimal)
         MdigFree(m_MilDitizerMinimal);
      }

   Init(iAll);
   }

// Public.
// Prints to the console the details of the detected input.
void CDigitizer::PrintDigitizerInfo()
   {
   MIL_STRING      DCFFormat;
   MIL_INT        InputMode;
   MIL_INT        ScanMode;
   MIL_INT        SizeX, SizeY, SizeBand;
   MIL_DOUBLE     FrameRate;
   MIL_INT        DigitizerNumber;

   MdigInquire(m_MilDigitizer, M_NUMBER, &DigitizerNumber);
   MdigInquire(m_MilDigitizer, M_SIZE_X, &SizeX);
   MdigInquire(m_MilDigitizer, M_SIZE_Y, &SizeY);
   MdigInquire(m_MilDigitizer, M_SIZE_BAND, &SizeBand);
   MdigInquire(m_MilDigitizer, M_INPUT_MODE, &InputMode);
   MdigInquire(m_MilDigitizer, M_SCAN_MODE, &ScanMode);
   MdigInquire(m_MilDigitizer, M_SELECTED_FRAME_RATE, &FrameRate);
   MdigInquire(m_MilDigitizer, M_FORMAT_DETECTED, DCFFormat);

   MosPrintf(MIL_TEXT("\t%lld x %lld, %d band(s). "), (long long)SizeX, (long long)SizeY, (int)SizeBand);

   switch(InputMode)
      {
      case M_ANALOG: MosPrintf(MIL_TEXT("Analog "));break;
      case M_DIGITAL: MosPrintf(MIL_TEXT("Digital "));break;
      default: MosPrintf(MIL_TEXT(" "));
      }
   switch(ScanMode)
      {
      case M_PROGRESSIVE: MosPrintf(MIL_TEXT("progressive "));break;
      case M_INTERLACE: MosPrintf(MIL_TEXT("interlaced "));break;
      default: MosPrintf(MIL_TEXT(" "));
      }

   m_DetectDCFName = DCFFormat;
   MosPrintf(MIL_TEXT("@ %0.2f fps.\n"), FrameRate);
   MosPrintf(MIL_TEXT("\tDCF: %s.\n\n"), DCFFormat.c_str());
   }

// Public.
// Start the grab stream.
void CDigitizer::Start(MIL_BUF_HOOK_FUNCTION_PTR UserProcessingFunctionPtr)
   {
   m_ProcessingFunctionPtr = UserProcessingFunctionPtr;

   if(m_MilDigitizer)
      if(!IsAllocated())
      AllocateBuffers(m_Index);

   if(IsAllocated())
      {
      MdigProcess(m_MilDigitizer, m_MilBuffers, MAX_BUFFERING,
         M_START, M_DEFAULT, DigProcessingFunction, this);

      m_IsDeviceUsed = true;
      }
   }

MIL_INT CDigitizer::PrepareForGrabbing()
   {
   MIL_INT lRetVal = 0;

   if(AllocateDigitizer(m_MilSystem, DeviceNumber(), GetDcfName()))
      MdigInquire(m_MilDigitizer, M_FORMAT_DETECTED, m_DetectDCFName);
   else
      lRetVal = M_INVALID;

   return lRetVal;
   }

// Public.
// Stops the grab stream.
void CDigitizer::Stop()
   {
   if(IsDeviceUsed())
      MdigProcess(m_MilDigitizer, m_MilBuffers, MAX_BUFFERING,
         M_STOP, M_DEFAULT, DigProcessingFunction, this);
   }

MIL_INT CDigitizer::StopGrabbing()
   {
   MIL_INT lRetVal = 0;

   Stop();
   Free(false);
   m_IsDeviceUsed = false;
   m_CameraPresent = false;
   m_DetectDCFName.clear();

   return lRetVal;
   }

// Public.
// Returns grab statistics.
void CDigitizer::GetStatistics(MIL_INT *FrameCount, MIL_DOUBLE *FrameRate, MIL_INT *FramesMissed)
   {
   if(FrameCount)
      MdigInquire(m_MilDigitizer, M_PROCESS_FRAME_COUNT,  FrameCount);
   if(FrameRate)
      MdigInquire(m_MilDigitizer, M_PROCESS_FRAME_RATE,   FrameRate);
   if(FramesMissed)
      MdigInquire(m_MilDigitizer, M_PROCESS_FRAME_MISSED,  FramesMissed);
   }

// Public.
void CDigitizer::FreeDigitizer()
   {
   if(m_MilDigitizer)
      {
      MdigFree(m_MilDigitizer);
      m_MilDigitizer = M_NULL;
      }
   }
   // Public.
void CDigitizer::UpdateDetectedDcfName()
{ 
   if(m_MilDitizerMinimal)
      {
      MdigInquire(m_MilDitizerMinimal, M_CAMERA_PRESENT, &m_CameraPresent);
      if(m_CameraPresent)
         MdigInquire(m_MilDitizerMinimal, M_FORMAT_DETECTED, m_DetectDCFName);
      }
}
bool CDigitizer::AllocateDigitizerMinimal(MIL_ID iMilSystem, MIL_INT DevNum, const MIL_STRING& DCFName)
   {
   MdigAlloc(iMilSystem, DevNum, DCFName, M_MINIMAL, &m_MilDitizerMinimal);

   if(m_MilDitizerMinimal)
      {
      MdigHookFunction(m_MilDitizerMinimal, M_CAMERA_PRESENT, CDigitizer::DigHookCameraPresent, this);
      UpdateDetectedDcfName();
      }

   return m_MilDitizerMinimal? true:false;
   }
// Private
// Hook on Camera present.
MIL_INT MFTYPE CDigitizer::DigHookCameraPresent(MIL_INT hookType, MIL_ID eventId, void *userData)
   {
   CDigitizer *pDig = (CDigitizer * )userData;
   MIL_INT CameraPresent;

   MdigInquire(pDig->MinimalDigitizer(), M_CAMERA_PRESENT, &CameraPresent);

   #ifdef SHOW_CAMERA_PRESENT_STATUS
   if(!CameraPresent)
      MosPrintf(MIL_TEXT("Camera disconnected from dev:%d \n"),(int)pDig->DeviceNumber());
   else
      MosPrintf(MIL_TEXT("Camera connected from dev:%d \n"),(int)pDig->DeviceNumber());
   #endif

   MthrControl(pDig->GetEvent(), M_EVENT_SET, M_SIGNALED);

   return 0;
   }
