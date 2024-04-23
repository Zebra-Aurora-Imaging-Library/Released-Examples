﻿//*******************************************************************************
// 
// File name: BlisterPackInspection.cpp  
//
// Synopsis: Demonstrates the inspection of a blister pack using 3d
//           sheet-of-light profiling.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include <mil.h>
#include <math.h>
#include "BlisterPackInspection.h"

static const MIL_STRING FILENAMES[2] = {EX_PATH("Blister_pack_Cam0.ply"),
                                        EX_PATH("Blister_pack_Cam1.ply")};

static const MIL_STRING ILLUSTRATION_FILE = EX_PATH("BlisterPack.png");
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;

static const MIL_INT NBCLOUDS = 2;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("BlisterPackInspection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the inspection of a blister pack ")
             MIL_TEXT("using 3d\npoint clouds. ")
             MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, Display, Buffer, Graphics,\n"));
   MosPrintf(MIL_TEXT("Image Processing, Calibration, Geometric Model Finder,\n")
             MIL_TEXT("3D Metrology, 3D Image Processing and 3D Display. \n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   PrintHeader();

   // Check for the required example files.
   if(!CheckForRequiredMILFile(FILENAMES[0]))
      { return -1; }

   // Show illustration of the blister pack.
   MIL_ID IllustrationDispId  = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_FILE, MilSystem, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   // Visualization volume information.
   SMapGeneration MapData;
   MapData.BoxCornerX = -20.00;
   MapData.BoxCornerY = 0.00;
   MapData.BoxCornerZ = 0.50;
   MapData.BoxSizeX = 112.00;
   MapData.BoxSizeY = 112.00;
   MapData.BoxSizeZ = -8.00;
   MapData.MapSizeX = 400;
   MapData.MapSizeY = 400;
   MapData.PixelSizeX = 0.22;
   MapData.PixelSizeY = 0.22;
   MapData.GrayScaleZ = (MapData.BoxSizeZ / 65534.0);
   MapData.IntensityMapType = 8 + M_UNSIGNED;
   MapData.SetExtractOverlap = true;
   MapData.ExtractOverlap = M_MIN_Z;
   MapData.FillXThreshold = 1.0;
   MapData.FillYThreshold = 1.0;

   // Initialization.
   MIL_UNIQUE_BUF_ID ContainerIds[NBCLOUDS];

   //.....................................................................................
   // Import the acquired 3D point clouds.
   for(MIL_INT i = 0; i < NBCLOUDS; ++i)
      {
      ContainerIds[i] = MbufRestore(FILENAMES[i], MilSystem, M_UNIQUE_ID);
      }

   // Acquired clouds from two camera are merged into one container.
   MIL_UNIQUE_BUF_ID ContainerId = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);

   //.....................................................................................
   // Merge the acquired 3D point clouds.
   M3dimMerge(ContainerIds, ContainerId, NBCLOUDS, M_NULL, M_DEFAULT);

   MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);
   if(MilDisplay3D)
      {
      M3ddispControl(MilDisplay3D, M_TITLE, MIL_TEXT("Merged Cloud"));
      M3ddispSetView(MilDisplay3D, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      M3ddispSelect(MilDisplay3D, ContainerId, M_SELECT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_ZOOM, 1.5, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("Input files are imported and merged into a single cloud.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   MosPrintf(MIL_TEXT("The 3D cloud is projected into a depth map for 2D analysis.\n\n"));

   //.....................................................................................
   // Generate the depth map (orthogonal 2D-projection) of the acquired 3D point cloud.
   MIL_ID BlisterPackDepthmap = M_NULL;
   GenerateDepthMap(ContainerId, MilSystem, MapData, &BlisterPackDepthmap);

   //....................................
   // Analyze the generated depth map.
   CAnalyzeBlisterPack ProbObj;
   ProbObj.AllocProcessingObjects(MilSystem);
   ProbObj.Analyze(BlisterPackDepthmap);
   ProbObj.FreeProcessingObjects();

   if(BlisterPackDepthmap != M_NULL)
      MbufFree(BlisterPackDepthmap);

   // Free the 3D display.
   if(MilDisplay3D)
      M3ddispFree(MilDisplay3D);

   // Free illustration display.
   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   return 0;
   }

//*******************************************************************************
// Function that analyzes the scanned object.
//*******************************************************************************
void CAnalyzeBlisterPack::Analyze(MIL_ID MilDepthMap)
   {
   // Processing display zoom factor.
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_X = 1;
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_Y = 1;

   // Color specifications.
   const MIL_DOUBLE PROC_TEXT_PASS_COLOR     = M_COLOR_GREEN;
   const MIL_DOUBLE PROC_TEXT_FAIL_COLOR     = M_COLOR_RED;
   const MIL_DOUBLE MOD_BOX_COLOR            = M_COLOR_CYAN;
   const MIL_DOUBLE MOD_EDGE_COLOR           = M_COLOR_GREEN;

   const MIL_INT        ITEM_CHILD_SIZE_X   = 30;
   const MIL_INT        ITEM_CHILD_SIZE_Y   = 30;
   const MIL_INT        ITEM_DISTANCE_X     = 70;
   const MIL_INT        ITEM_DISTANCE_Y     = 72;
   const MIL_INT        NUM_ITEMS_X         = 3;
   const MIL_INT        NUM_ITEMS_Y         = 4;
   const MIL_DOUBLE     ITEM_HEIGHT         = -5.0;

   // Setup the display.
   MIL_UNIQUE_DISP_ID MilDisplay  = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID  MilGraphicList= MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilGraphics = M_DEFAULT;

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X , ILLUSTRATION_OFFSET_X);
  
   // Disable graphics list update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Setup the display.
   MgraClear(M_DEFAULT, MilGraphicList);
   MdispZoom(MilDisplay, PROC_DISPLAY_ZOOM_FACTOR_X, PROC_DISPLAY_ZOOM_FACTOR_Y);

   // Allocate the necessary buffers for processing.
   MIL_ID MilRemapped8BitImage;
   MbufAlloc2d(m_MilSystem,
               MbufInquire(MilDepthMap, M_SIZE_X, M_NULL),
               MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL),
               8, M_IMAGE + M_PROC + M_DISP, &MilRemapped8BitImage);

   // Remap 16-bit depth map to 8 bit.   
   M3dimRemapDepthMap(M_REMAP_CONTEXT_BUFFER_LIMITS, MilDepthMap, MilRemapped8BitImage, M_DEFAULT);

   // Remove the associated calibration coming from the 16-bit depth map.
   McalAssociate(M_NULL, MilRemapped8BitImage, M_DEFAULT);

   MdispSelect(MilDisplay, MilRemapped8BitImage);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Find the blister pack.
   MmodFind(m_MilModel, MilRemapped8BitImage, m_MilModelResult);

   MIL_INT NumOfOccurences = 0;
   MIL_INT PositionX = -1, PositionY = -1;
   MmodGetResult(m_MilModelResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOfOccurences);
   MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_X + M_TYPE_MIL_INT, &PositionX);
   MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_Y + M_TYPE_MIL_INT, &PositionY);

   if (NumOfOccurences >= 1)
      {
      // Draw the box if found.
      MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_OPAQUE);
      MgraControl(M_DEFAULT, M_FONT_SIZE, TEXT_FONT_SIZE_MEDIUM);

      MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);   
      MgraColor(M_DEFAULT, MOD_BOX_COLOR);
      MmodDraw(M_DEFAULT, m_MilModelResult, MilGraphicList, M_DRAW_BOX, M_ALL, M_DEFAULT);

      MIL_ID ItemChild;
      MIL_DOUBLE DeviationMean = 0.0;
      MIL_INT OffsetX, OffsetY;
      MIL_ID StatResultId = M3dmetAllocResult(m_MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
 
      for (MIL_INT y = 0; y < NUM_ITEMS_Y; y++)
         {
         for (MIL_INT x = 0; x < NUM_ITEMS_X; x++)
            {
            OffsetX = PositionX + x*ITEM_DISTANCE_X;
            OffsetY = PositionY + y*ITEM_DISTANCE_Y;

            MbufChild2d(MilDepthMap,
                        OffsetX, OffsetY,
                        ITEM_CHILD_SIZE_X, ITEM_CHILD_SIZE_Y,
                        &ItemChild);
          
            // Compute model's mean depth-map elevation.
            M3dmetStat(M_STAT_CONTEXT_MEAN, ItemChild, M_XY_PLANE, StatResultId, M_SIGNED_DISTANCE_Z_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
            M3dmetGetResult(StatResultId, M_STAT_MEAN, &DeviationMean);

            if (DeviationMean > ITEM_HEIGHT)
               {
               // Fail.
               MgraColor(MilGraphics, PROC_TEXT_FAIL_COLOR);
               MgraLine(MilGraphics, MilGraphicList, OffsetX, OffsetY, OffsetX+ITEM_CHILD_SIZE_X, OffsetY+ITEM_CHILD_SIZE_Y);
               MgraLine(MilGraphics, MilGraphicList, OffsetX+ITEM_CHILD_SIZE_X, OffsetY, OffsetX, OffsetY+ITEM_CHILD_SIZE_Y);
               MgraRect(MilGraphics, MilGraphicList, OffsetX, OffsetY, OffsetX+ITEM_CHILD_SIZE_X, OffsetY+ITEM_CHILD_SIZE_Y);
               }
            else
               {
               // Pass.
               MgraColor(MilGraphics, PROC_TEXT_PASS_COLOR);
               MgraRect(MilGraphics, MilGraphicList, OffsetX, OffsetY, OffsetX+ITEM_CHILD_SIZE_X, OffsetY+ITEM_CHILD_SIZE_Y);
               }

            MbufFree(ItemChild);
            }
         }
      M3dmetFree(StatResultId);

      MosPrintf(MIL_TEXT("The blister pack items were verified and the results"));
      MosPrintf(MIL_TEXT(" are shown.\n\tGreen box: Pass\n\tRed box: Missing or ")
                MIL_TEXT("broken\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Blister pack not found.\n"));
      }

   // Enable graphics list update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   // Show the result.
   MdispSelect(MilDisplay, MilRemapped8BitImage);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   MbufFree(MilRemapped8BitImage);
   }

//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CAnalyzeBlisterPack::AllocProcessingObjects(MIL_ID MilSystem)
   {
   m_MilSystem = MilSystem;

   MIL_CONST_TEXT_PTR PACK_MODEL = EX_PATH("PackModel.mmf"); 

   // Allocate objects to locate the blister pack.
   MmodAllocResult(MilSystem, M_DEFAULT, &m_MilModelResult);
   MmodRestore(PACK_MODEL, MilSystem, M_DEFAULT, &m_MilModel);

   // Preprocess the model.
   MmodPreprocess(m_MilModel, M_DEFAULT);
   }

//*******************************************************************************
// Function that frees processing objects.
//*******************************************************************************
void CAnalyzeBlisterPack::FreeProcessingObjects()
   {
   if(m_MilModelResult != M_NULL)
      { MmodFree(m_MilModelResult); m_MilModelResult = M_NULL; }

   if(m_MilModel != M_NULL)
      { MmodFree(m_MilModel); m_MilModel = M_NULL; }
   }
