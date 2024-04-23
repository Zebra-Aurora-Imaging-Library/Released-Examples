//******************************************************************************
// 
// File name: Utilities.h
//
// Synopsis:  This file holds the utility functions used by the program.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************

#ifndef __UTILITIES_H
#define __UTILITIES_H

#include <mil.h>

//******************************************************************************
// Utility structures.
//******************************************************************************
/**
 * Display information.
 */
struct DisplayInfo
   {
   MIL_UINT   PositionX; // X position of the display.
   MIL_UINT   PositionY; // Y position of the display.
   MIL_UINT   Size;      // Size of the display (in both directions).
   MIL_STRING Title;     // Title of the display.
   };

/**
 * Result structure.
 */
struct DstResult
   {
   MIL_UNIQUE_BUF_ID PC;             // Destination point cloud container.
   MIL_UNIQUE_BUF_ID OutlierPoints;  // Outlier points' container.
   MIL_UNIQUE_3DIM_ID Context;       // Outlier removal context.
   MIL_UNIQUE_3DDISP_ID Display;     // Display.
   MIL_INT64 GraList;                // Graphic list of the display.

   /**
    * Default constructor.
    */
   DstResult() {}

   /**
    * Constructor.
    */
   DstResult(MIL_ID MilPC, MIL_ID MilOutlierPoints,
             MIL_ID MilDisplay, MIL_INT64 GraList)
      : PC(MilPC),
        OutlierPoints(MilOutlierPoints),
        Display(MilDisplay),
        GraList(GraList)
   {}

   /**
    * Sets the title of the display.
    */
   void SetDisplayTitle(MIL_STRING Title)
      {
      M3ddispControl(Display, M_TITLE, Title);
      }
   };

//******************************************************************************
// Constants.
//******************************************************************************
static const MIL_UINT DISP3D_SIZE = 500;

static DisplayInfo SRC_DISPLAY_INFO =
   { 0, 0, DISP3D_SIZE, MIL_TEXT("Scanned point cloud") };

static DisplayInfo DST_DISPLAY_INFO[] =
   {
   {DISP3D_SIZE, 0          , DISP3D_SIZE, MIL_TEXT("M_NUMBER_WITHIN_DISTANCE outlier mode")},
   {0          , DISP3D_SIZE, DISP3D_SIZE, MIL_TEXT("M_LOCAL_DISTANCE outlier mode + ")
                                           MIL_TEXT("M_ROBUST_STD_DEVIATION threshold mode")},
   {DISP3D_SIZE, DISP3D_SIZE, DISP3D_SIZE, MIL_TEXT("M_LOCAL_DENSITY_PROBABILITY outlier mode")},
   };

//*******************************************************************************
// Pauses the execution until a key is pressed.
//*******************************************************************************
void WaitForKey()
   {
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*******************************************************************************
// Checks if the buffer file exists and restore it.
//*******************************************************************************
MIL_UNIQUE_BUF_ID RestoreFile(MIL_ID MilSystem, MIL_STRING FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT,
                     &FilePresent);

   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The file needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      WaitForKey();
      exit(EXIT_FAILURE);
      }

   auto Milbuf = MbufRestore(FileName, MilSystem, M_UNIQUE_ID);

   return Milbuf;
   }


//*******************************************************************************
// Allocates a 3D display if possible.  
//*******************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Mil3dDisp = M3ddispAlloc(MilSystem, M_DEFAULT,
                                 MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!Mil3dDisp)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support 3D display.\n"));
      WaitForKey();
      exit(EXIT_FAILURE);
      }

   return Mil3dDisp;
   }

//*******************************************************************************
// Allocates a 3D display if possible and displays a container or geometry. 
//*******************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem,
                                     MIL_INT PositionX, MIL_INT PositionY,
                                     MIL_INT SizeX, MIL_INT SizeY,
                                     const MIL_STRING& Title)
   {
   auto Mil3dDisp = Alloc3dDisplayId(MilSystem);

   M3ddispControl(Mil3dDisp, M_TITLE, Title);
   M3ddispControl(Mil3dDisp, M_WINDOW_INITIAL_POSITION_X, PositionX);
   M3ddispControl(Mil3dDisp, M_WINDOW_INITIAL_POSITION_Y, PositionY);
   M3ddispControl(Mil3dDisp, M_SIZE_X, SizeX);
   M3ddispControl(Mil3dDisp, M_SIZE_Y, SizeY);
   M3ddispSelect(Mil3dDisp, M_NULL, M_OPEN, M_DEFAULT);

   return Mil3dDisp;
   }

//*******************************************************************************
// Sets the color, thickness, and opacity of a graphic label.
//*******************************************************************************
void SetGraphicFormat(MIL_ID GraphicsList, MIL_INT64 ModelLabel, MIL_INT Color,
                      MIL_INT Thickness, MIL_INT Opacity)
   {
   M3dgraControl(GraphicsList, ModelLabel, M_COLOR, Color);
   M3dgraControl(GraphicsList, ModelLabel, M_THICKNESS, Thickness);
   M3dgraControl(GraphicsList, ModelLabel, M_OPACITY, Opacity);
   }

//*******************************************************************************
// Draws a 3D bounding box. 
//*******************************************************************************
void DrawBoundingBox(MIL_ID MilSystem, MIL_ID MilStatResult, MIL_ID GraList)
   {
   auto MilBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimCopyResult(MilStatResult, MilBox, M_BOUNDING_BOX, M_DEFAULT);
   auto MilBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBox, GraList, M_ROOT_NODE, M_DEFAULT);
   SetGraphicFormat(GraList, MilBoxLabel, M_COLOR_GREEN, 3, 20);
   }

//*******************************************************************************
// Draws the outlier points. 
//*******************************************************************************
void DrawOutlierPoints(const DstResult& Result)
   {
   auto Label = M3ddispSelect(Result.Display, Result.OutlierPoints, M_ADD, M_DEFAULT);
   M3dgraControl(Result.GraList, Label, M_COLOR_COMPONENT, M_NULL);
   M3dgraControl(Result.GraList, Label, M_COLOR, M_COLOR_RED);
   }

//*******************************************************************************
// Executes and computes the processing time, in ms. 
//*******************************************************************************
MIL_INT TimeComputation(MIL_ID SrcPC, DstResult& Result)
   {
   MIL_DOUBLE MinTime = MIL_DOUBLE_MAX; // In s.

   // Run computation many times for consistency.
   const auto NbRuns = 10;
   for (int i = 0; i < NbRuns; ++i)
      {
      auto StartTime = MappTimer(M_TIMER_READ, M_NULL); // In s.
      M3dimOutliers(Result.Context, SrcPC, Result.PC, M_NULL, M_DEFAULT);
      auto EndTime = MappTimer(M_TIMER_READ, M_NULL);   // In s.
      MinTime = std::min({ MinTime, EndTime - StartTime });
      }
   return static_cast<MIL_INT>(MinTime * 1000); // In ms.
   }
#endif
