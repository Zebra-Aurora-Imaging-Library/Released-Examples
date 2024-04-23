/***************************************************************************************/
/*
* File name: MultiComponentGrab.cpp
*
* Synopsis:  This program shows the use of the MbufAllocContainer() and MdigProcess()
*            functions to do real-time acquisition from devices with multiple components
*            such as 3D scanners.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include "MultiComponentGrab.h"
#define BUFFERING_SIZE  20

/******************************************************************************/
/* Example description.                                                       */
/******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("MultiComponentGrab\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This example demonstrates how to interface a multi-component\n")\
      MIL_TEXT("device using MIL multi-component buffer containers.\n")\
      MIL_TEXT("\nPress <Enter> to start.\n\n")\
   );
   }

/* Function prototypes */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
void PrintComponentInfo(MIL_INT ComponentCount, MIL_INT ComponentNb, const ComponentData& ComponentInfo);
void FreeDisplayData(ComponentDataList& Components);

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   ComponentDataList DisplayList;
   MIL_INT ProcessedCount;
   } HookDataStruct;

/* Main function. */
/* ---------------*/
int MosMain(void)
   {
   MIL_ID MilApplication,                          /* Application identifier.       */
          MilSystem,                               /* System identifier.            */
          MilDigitizer,                            /* Digitizer identifier.         */
          MilContainers[BUFFERING_SIZE] = { 0 };   /* Container buffer identifier.  */
   MIL_INT MilContainerCount = 0,
           ProcessFrameCount = 0,
           GenICamSupport = M_FALSE;
   MIL_DOUBLE ProcessFrameRate = 0;
   HookDataStruct UserHookData;

   PrintHeader();
   MosGetch();

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL,
      &MilDigitizer, M_NULL);

   MsysInquire(MilSystem, M_GENICAM_AVAILABLE, &GenICamSupport);
   if (GenICamSupport == M_TRUE)
      {
      /* Display the feature browser and ask the user to set-up his device */
      /* in multi-component mode. */
      MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
      MosPrintf(MIL_TEXT("Use the MIL feature browser to setup your camera as required\n"));
      MosPrintf(MIL_TEXT("and enable the desired components.\n"));
      MosPrintf(MIL_TEXT("See mainly ComponentSelector and ComponentEnable features.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to start.\n"));
      MosGetch();
      }

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nMULTI-COMPONENT ACQUISITION IN PROGRESS..\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------\n\n"));

   /* Allocate multiple containers. */
   /* The container will get filled with components once grab starts. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MIL_UINT i = 0; i < BUFFERING_SIZE; i++)
      {
      MbufAllocContainer(MilSystem, M_GRAB + M_PROC, M_DEFAULT, &MilContainers[i]);

      if (MilContainers[i] != M_NULL)
         MilContainerCount++;
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Initialize the user's processing function data structure. */
   UserHookData.MilDigitizer = MilDigitizer;
   UserHookData.ProcessedCount = 0;

   /* Increase default grab timeout. Multi component devices can have a low frame rates.*/
   MdigControl(MilDigitizer, M_GRAB_TIMEOUT, 10000);

   /* Start the processing. The processing function is called with every frame grabbed. */
   MdigProcess(MilDigitizer, MilContainers, MilContainerCount,
      M_START, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Here the main() is free to perform other tasks while the processing is executing. */
   /* --------------------------------------------------------------------------------- */
   /* Print a message and wait for a key press after a minimum number of frames.        */
   MosPrintf(MIL_TEXT("Press <Enter> to stop.                    \n\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilContainers, MilContainerCount,
      M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%d frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
      (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free per-component allocated MIL resources. */
   FreeDisplayData(UserHookData.DisplayList);

   /* Free the containers. */
   for (MIL_INT i = 0; i < MilContainerCount; i++)
      {
      MbufFree(MilContainers[i]);
      }

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);

   return 0;
   }

/* User's processing function called every time a grab buffer is ready. */
/* -------------------------------------------------------------------- */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   ComponentDataList& DisplayList = UserHookDataPtr->DisplayList;
   MIL_ID ModifiedContainerId = M_NULL;

   /* Retrieve the MIL_ID of the grabbed buffer a M_CONTAINER in this case. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedContainerId);

   /* Process each component contained in the container and display it. */
   std::vector<MIL_ID> Components;
   MbufInquireContainer(ModifiedContainerId, M_CONTAINER, M_COMPONENT_LIST, Components);

   for (size_t i = 0; i < Components.size(); i++)
      {
      MIL_INT64 ComponentType = 0, GroupId = 0, SourceId = 0, RegionId = 0;
      MIL_STRING ComponentName;
      /* Inquire the component's type. */
      MbufInquire(Components[i], M_COMPONENT_TYPE, &ComponentType);
      /* Inquire the component's name. */
      MbufInquire(Components[i], M_COMPONENT_TYPE_NAME, ComponentName);
      /* Inquire the component's group, source and region ids. */
      MbufInquire(Components[i], M_COMPONENT_GROUP_ID, &GroupId);
      MbufInquire(Components[i], M_COMPONENT_SOURCE_ID, &SourceId);
      MbufInquire(Components[i], M_COMPONENT_REGION_ID, &RegionId);

      /* Construct a unique name from the component's name, group, source and region Ids. */
      MIL_STRING_STREAM ConstructedName;
      ConstructedName << ComponentName << MIL_TEXT("[") << GroupId << MIL_TEXT(":") << SourceId << MIL_TEXT(":") << RegionId << MIL_TEXT("]");
      bool IsDisplayableComponent = ComponentType != M_COMPONENT_METADATA;

      /* Look-up the corresponding display buffer associated to this component and copy */
      /* the component to the display. */
      ComponentListIterator It = DisplayList.find(ConstructedName.str());
      if (It == DisplayList.end())
         {
         MIL_STRING PfncName;
         /* Inquire the component's pixel format. */
         MbufInquire(Components[i], M_PFNC_NAME, PfncName);

         /* Allocate per-component data as required by the component.. */
         ComponentData ComponentInfo(Components[i], IsDisplayableComponent, ConstructedName.str(), PfncName);

         if (IsDisplayableComponent)
            {
            /* Update the display buffer. */
            MbufCopy(Components[i], ComponentInfo.MilImageDisp);
            }

         /* Print per-component information. */
         PrintComponentInfo(Components.size(), i, ComponentInfo);

         /* Add display to list of displayable components. */
         DisplayList[ConstructedName.str()] = ComponentInfo;
         }
      else
         {
         ComponentData& ComponentInfo = It->second;
         if (IsDisplayableComponent)
            {
            MbufCopy(Components[i], ComponentInfo.MilImageDisp);
            }
         }
      }

   /* Print the number of containers processed. */
   MosPrintf(MIL_TEXT("Containers processed: %d\r"), ++UserHookDataPtr->ProcessedCount);

   return 0;
   }

/******************************************************************************/
/* Utility routine used to free the MIL display.                              */
/******************************************************************************/
void FreeDisplayData(ComponentDataList& Components)
   {
   /* Free per component data. */
   ComponentListIterator Iterator = Components.begin();
   for (; Iterator != Components.end(); ++Iterator)
      {
      ComponentData& ComponentData = Iterator->second;
      ComponentData.Free();
      }
   Components.clear();
   }

/******************************************************************************/
/* Utility routine used to print grabbed component information.               */
/******************************************************************************/
void PrintComponentInfo(MIL_INT ComponentCount, MIL_INT ComponentNb, const ComponentData& ComponentInfo)
   {
   if (ComponentNb == 0)
      {
      MosPrintf(MIL_TEXT("+------------------------------------------------------------------------------+\n"));
      MosPrintf(MIL_TEXT("|                         Container Component Count: %2d                        |\n"), (int)ComponentCount);
      MosPrintf(MIL_TEXT("|------------------------------+------------------------+----------------------|\n"));
      MosPrintf(MIL_TEXT("|        Component Name        |       Size & Type      |     PFNC Format      |\n"));
      MosPrintf(MIL_TEXT("|------------------------------|------------------------|----------------------|\n"));
      }

   MosPrintf(MIL_TEXT("|%29.29s | %-23.23s| %-21.21s|\n"),
             ComponentInfo.ComponentName.c_str(), ComponentInfo.ToString().c_str(),
             ComponentInfo.PixelFormatName.c_str());

   if (ComponentNb == ComponentCount - 1)
      {
      MosPrintf(MIL_TEXT("+------------------------------+------------------------+----------------------+\n"));
      MosPrintf(MIL_TEXT("Note: [x:x:x] component name suffix encoded as [GroupId:SourceId:RegionId]\n\n"));
      }
   }
