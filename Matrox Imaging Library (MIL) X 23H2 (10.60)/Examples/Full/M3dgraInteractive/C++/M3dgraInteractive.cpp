//----------------------------------------------------------------------------
//
// File name: M3dgraInteractive.cpp
//
// Synopsis: This program contains an example on how to interactively edit a
//           3D box geometry.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//----------------------------------------------------------------------------
#include <mil.h>

//-----------------------------------------------------------------------------
// Constants.
//-----------------------------------------------------------------------------
static const MIL_STRING PT_CLD_FILE =
   M_IMAGE_PATH MIL_TEXT("M3dgra/MaskOrganized.mbufc");

//----------------------------------------------------------------------------
struct SPickStruct
   {
   MIL_INT64 BoxLabel;
   MIL_ID    Box;
   MIL_ID    Gralist;
   MIL_ID    OriginalContainer;
   MIL_ID    CroppedContainer;
   };

//----------------------------------------------------------------------------
// Function Declaration.
//----------------------------------------------------------------------------
bool CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_ID Alloc3dDisplayId(MIL_ID MilApplication, MIL_ID MilSystem);
MIL_INT MFTYPE BoxModifiedHandler(MIL_INT HookType,
                                  MIL_ID EventId,
                                  void *UserDataPtr);
void RetrieveBoxAndCrop(SPickStruct* PickStruct);

//-----------------------------------------------------------------------------
// Main.
//-----------------------------------------------------------------------------
int MosMain()
   {
   MosPrintf(
      MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("M3dgraInteractive\n\n")
      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This example demonstrates how to interactively edit a 3D box geometry.\n\n")
      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, buffer, 3D display, 3D graphics.\n")
      MIL_TEXT("\n") );

   MIL_ID MilApplication,  // Application identifier.
          MilSystem;       // System identifier.

   // Allocate defaults.
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);

   // Check for required example files.
   if(!CheckForRequiredMILFile(PT_CLD_FILE))
      {
      return 0;
      }

   // Allocate the display.
   MIL_ID Mil3dDisplay = Alloc3dDisplayId(MilApplication, MilSystem);
   MIL_ID Mil3dGraList = (MIL_ID) M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Restore the point cloud from a file.
   MIL_ID OriginalContainer = MbufImport(PT_CLD_FILE, M_DEFAULT, M_RESTORE, MilSystem, M_NULL);

   // Create a cropped copy of the point cloud and add it to the graphic list.
   MIL_ID CroppedContainer = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_NULL);
   M3dgraAdd(Mil3dGraList, M_ROOT_NODE, CroppedContainer, M_DEFAULT);

   // Create an editable box in the graphic list.
   // Initialize the size of the box to a fraction of the original point cloud's size.
   MIL_ID BoundingBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, OriginalContainer, BoundingBox, M_DEFAULT);

   M3dgeoBox(BoundingBox, M_CENTER_AND_DIMENSION, M_UNCHANGED, M_UNCHANGED, M_UNCHANGED,
             M3dgeoInquire(BoundingBox, M_SIZE_X, M_NULL) * 0.5,
             M3dgeoInquire(BoundingBox, M_SIZE_Y, M_NULL) * 0.5,
             M_UNCHANGED, M_DEFAULT);

   MIL_INT64 BoxLabel =
      M3dgeoDraw3d(M_DEFAULT, BoundingBox, Mil3dGraList, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(Mil3dGraList, BoxLabel, M_EDITABLE, M_ENABLE);

   MIL_ID CroppingBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);

   // Create a hook to crop the container when the box is modified in the graphics list.
   SPickStruct PickStruct;
   PickStruct.Box = CroppingBox;
   PickStruct.BoxLabel = BoxLabel;
   PickStruct.Gralist = Mil3dGraList;
   PickStruct.OriginalContainer = OriginalContainer;
   PickStruct.CroppedContainer = CroppedContainer;

   M3dgraHookFunction(
      Mil3dGraList,
      M_EDITABLE_GRAPHIC_MODIFIED,
      BoxModifiedHandler,
      &PickStruct);

   // Crop a first time before starting the interactivity.
   RetrieveBoxAndCrop(&PickStruct);

   // Open the 3d display.
   M3ddispSelect(Mil3dDisplay, M_NULL, M_OPEN, M_DEFAULT);

   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a ply file and displayed.\n"));
   MosPrintf(MIL_TEXT("The box is editable.\n"));
   MosPrintf(MIL_TEXT("Only the points inside the interactive box are shown.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("- Use side box handles to resize.\n"));
   MosPrintf(MIL_TEXT("- Use axis arrow tips to translate.\n"));
   MosPrintf(MIL_TEXT("- Use axis arcs to rotate.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   M3dgeoFree(CroppingBox);
   M3dgeoFree(BoundingBox);
   MbufFree(CroppedContainer);
   MbufFree(OriginalContainer);
   M3ddispFree(Mil3dDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

//----------------------------------------------------------------------------
MIL_INT MFTYPE BoxModifiedHandler(MIL_INT /*HookType*/, MIL_ID /*EventId*/, void* UserDataPtr)
   {
   SPickStruct* PickStruct = static_cast<SPickStruct*>(UserDataPtr);
   RetrieveBoxAndCrop(PickStruct);

   return 0;
   }

//----------------------------------------------------------------------------
void RetrieveBoxAndCrop(SPickStruct* PickStruct)
   {
   // Retrieve the edited box from the graphics list.
   M3dgraCopy(PickStruct->Gralist,
              PickStruct->BoxLabel,
              PickStruct->Box,
              M_DEFAULT,
              M_GEOMETRY,
              M_DEFAULT);

   // Crop the point cloud using the retrieved box.
   M3dimCrop(PickStruct->OriginalContainer,
             PickStruct->CroppedContainer,
             PickStruct->Box,
             M_NULL,
             M_SAME,
             M_DEFAULT);
   }

//----------------------------------------------------------------------------
// Check for required files to run the example.
//----------------------------------------------------------------------------
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT,
                     FileName,
                     M_NULL,
                     M_NULL,
                     M_FILE_EXISTS,
                     M_DEFAULT,
                     &FilePresent);

   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//-----------------------------------------------------------------------------
// Allocates a 3D display and returns its MIL identifier.
//-----------------------------------------------------------------------------
MIL_ID Alloc3dDisplayId(MIL_ID MilApplication, MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_ID MilDisplay3D =
      M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      MsysFree(MilSystem);
      MappFree(MilApplication);
      exit(0);
      }

   return MilDisplay3D;
   }
