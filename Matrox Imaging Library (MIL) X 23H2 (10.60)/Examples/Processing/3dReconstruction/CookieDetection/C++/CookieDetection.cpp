﻿//*******************************************************************************
// 
// File name: CookieDetection.cpp  
//
// Synopsis: Demonstrations inspection of cookies using 3D data.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include "CookieDetection.h"

static const MIL_STRING FILENAMES[2] = {EX_PATH("Cookies_Cam0.ply"),
                                        EX_PATH("Cookies_Cam1.ply")};
static const MIL_STRING ILLUSTRATION_FILE = EX_PATH("CookiesObject.tif");
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;
static const MIL_INT NBCLOUDS = 2;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("CookieDetection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to count cookies using ")
             MIL_TEXT("3D point clouds.\n\n "));


   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Display, Buffer, Graphics,\n")
             MIL_TEXT("Calibration, Image processing, Model Finder,\n"));
   MosPrintf(MIL_TEXT("3D Image Processing, 3D Metrology and 3D Display.\n\n"));
   }


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   PrintHeader();

   // Check for the required example files.
   if(!CheckForRequiredMILFile(FILENAMES[0]))
      { return -1; }

   // Show illustration of the cookies.
   MIL_ID IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_FILE, MilSystem, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   // Visualization volume information.
   SMapGeneration MapData;
   MapData.BoxCornerX = -10.00;
   MapData.BoxCornerY = 0.00;
   MapData.BoxCornerZ = 1.00;
   MapData.BoxSizeX = 220.00;
   MapData.BoxSizeY = 220.00;
   MapData.BoxSizeZ = -39.00;
   MapData.MapSizeX = 695;
   MapData.MapSizeY = 695;
   MapData.PixelSizeX = 0.317;
   MapData.PixelSizeY = 0.317;
   MapData.GrayScaleZ = (MapData.BoxSizeZ / 65534.0);
   MapData.IntensityMapType = 8 + M_UNSIGNED;
   MapData.SetExtractOverlap = true;
   MapData.ExtractOverlap = M_MIN_Z;
   MapData.FillXThreshold = 1.0;
   MapData.FillYThreshold = 1.0;

   //.....................................................................................
   // Initialization.
   MIL_UNIQUE_BUF_ID ContainerIds[NBCLOUDS];
   // Import the acquired 3D point clouds.
   for(MIL_INT i = 0; i < NBCLOUDS; ++i)
      {
      MbufImport(FILENAMES[i], M_DEFAULT, M_RESTORE, MilSystem, &ContainerIds[i]);
      }

   //.....................................................................................
   // Acquired clouds from the two cameras are merged into one container.
   MIL_UNIQUE_BUF_ID ContainerId = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
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
   MIL_ID CookieBoxDepthmap = M_NULL;
   GenerateDepthMap(ContainerId, MilSystem, MapData, &CookieBoxDepthmap);

   //....................................
   // Analyze the generated depth map.
   CCookieCounting ProbObj;
   ProbObj.AllocProcessingObjects(MilSystem);
   ProbObj.Analyze(CookieBoxDepthmap);
   ProbObj.FreeProcessingObjects();

   if(CookieBoxDepthmap != M_NULL)
      { MbufFree(CookieBoxDepthmap); }
   if(MilDisplay3D)
      { M3ddispFree(MilDisplay3D); }

   // Free illustration display.
   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   return 0;
   }

//*******************************************************************************
// Function that analyzes the scanned object.
//*******************************************************************************
void CCookieCounting::Analyze(MIL_ID MilDepthMap)
   {
   // Processing display zoom factor.
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_X = 1.0;
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_Y = 1.0;

   // Color specifications.
   const MIL_DOUBLE PROC_TEXT_PASS_COLOR = M_COLOR_DARK_GREEN;
   const MIL_DOUBLE PROC_TEXT_FAIL_COLOR = M_COLOR_RED;
   const MIL_DOUBLE MOD_BOX_COLOR        = M_COLOR_CYAN;
   const MIL_DOUBLE MOD_EDGE_COLOR       = M_COLOR_GREEN;

   const MIL_INT NUM_COOKIE_POSITIONS = 6;
   const MIL_INT COOKIE_RELATIVE_OFFSETS[NUM_COOKIE_POSITIONS][2] =
      {
         {  40,  42 },
         { 267,  49 },
         {  24, 230 },
         { 258, 224 },
         {  36, 424 },
         { 269, 419 }
      };

   const MIL_INT COOKIE_CHILD_SIZE[2] = { 75, 67 };
   const MIL_DOUBLE COOKIE_HEIGHT     = 10.0;
   const MIL_INT EXPECTED_NUM_COOKIES = 3;

   // Setup the display.
   MIL_UNIQUE_DISP_ID MilDisplay     = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID  MilGraphicList = MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilGraphics = M_DEFAULT;

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Specify the window position.
   MdispControl(MilDisplay,M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);

   // Disable graphics list update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Set the invalid data to 0.
   MbufClearCond(MilDepthMap, 0, 0, 0, MilDepthMap, M_EQUAL, 65535);

   // Setup the display.
   MgraClear(M_DEFAULT, MilGraphicList);
   MdispZoom(MilDisplay, PROC_DISPLAY_ZOOM_FACTOR_X, PROC_DISPLAY_ZOOM_FACTOR_Y);

   // Allocate the necessary buffers for processing.
   MIL_ID MilRemapped8BitImage;
   MbufAlloc2d(m_MilSystem,
               MbufInquire(MilDepthMap, M_SIZE_X, M_NULL),
               MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL),
               8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilRemapped8BitImage);

   // Remap to 8 bit.
   M3dimRemapDepthMap(M_REMAP_CONTEXT_BUFFER_LIMITS, MilDepthMap, MilRemapped8BitImage, M_DEFAULT);

   MgraClear(M_DEFAULT, MilGraphicList);

   MdispSelect(MilDisplay, MilRemapped8BitImage);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Find the cookie box.
   MmodFind(m_MilModel, MilRemapped8BitImage, m_MilModelResult);

   MIL_INT NumOfOccurences = 0;
   MIL_INT PositionX, PositionY;

   // Get information on the find.
   MmodGetResult(m_MilModelResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOfOccurences);
   MmodControl(m_MilModelResult, M_DEFAULT, M_RESULT_OUTPUT_UNITS, M_PIXEL);

   MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_X + M_TYPE_MIL_INT, &PositionX);
   MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_Y + M_TYPE_MIL_INT, &PositionY);

   if (NumOfOccurences >= 1)
      {
      // Draw the box if found.
      MgraControl(MilGraphics, M_BACKGROUND_MODE, M_OPAQUE);
      MgraControl(MilGraphics, M_FONT_SIZE, TEXT_FONT_SIZE_MEDIUM);

      MgraControl(MilGraphics, M_BACKGROUND_MODE, M_TRANSPARENT);         
      MgraColor(MilGraphics, MOD_BOX_COLOR);
      MmodDraw(MilGraphics, m_MilModelResult, MilGraphicList, M_DRAW_BOX, M_ALL, M_DEFAULT);
      MgraColor(MilGraphics, MOD_EDGE_COLOR);
      MmodDraw(MilGraphics, m_MilModelResult, MilGraphicList, M_DRAW_EDGES, M_ALL, M_DEFAULT);

      MIL_DOUBLE DeviationMean = 0.0;
      MIL_INT NumCookies = 0;
      MIL_TEXT_CHAR CookieString[MAX_STRING_LEN];
      MIL_TEXT_CHAR TempString[MAX_STRING_LEN];

      for (MIL_INT i = 0; i < NUM_COOKIE_POSITIONS; i++) 
         {
         // Create a child for each location of cookies in the box.
         MIL_ID CookieChild = 
         MbufChild2d(MilDepthMap, 
                     PositionX+COOKIE_RELATIVE_OFFSETS[i][0], 
                     PositionY+COOKIE_RELATIVE_OFFSETS[i][1],
                     COOKIE_CHILD_SIZE[0], COOKIE_CHILD_SIZE[1], 
                     M_NULL);

         // Calculate 3D statistics for each cookie location.
         MIL_ID StatResult = M3dmetAllocResult(m_MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
         M3dmetStat(M_STAT_CONTEXT_MEAN, CookieChild, M_XY_PLANE, StatResult, M_ABSOLUTE_DISTANCE_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
         M3dmetGetResult(StatResult, M_STAT_MEAN, &DeviationMean);
         M3dmetFree(StatResult);

         // Determine the number of cookies according to the height.
         NumCookies = (MIL_INT)(DeviationMean/COOKIE_HEIGHT);

         if (NumCookies == EXPECTED_NUM_COOKIES)
            { MgraColor(MilGraphics, PROC_TEXT_PASS_COLOR); }
         else
            { MgraColor(MilGraphics, PROC_TEXT_FAIL_COLOR); }

         if (NumCookies == 1)
            { MosSprintf(TempString, MAX_STRING_LEN, MIL_TEXT("%s"), MIL_TEXT("")); }
         else
            { MosSprintf(TempString, MAX_STRING_LEN, MIL_TEXT("%s"), MIL_TEXT("s")); }

         MosSprintf(CookieString, MAX_STRING_LEN, MIL_TEXT("%d cookie%s"), NumCookies, TempString);

         MgraText(MilGraphics, MilGraphicList, 
                  PositionX+COOKIE_RELATIVE_OFFSETS[i][0]-10,
                  TEXT_OFFSET_Y+PositionY+COOKIE_RELATIVE_OFFSETS[i][1], 
                  CookieString);

         MbufFree(CookieChild);
         }

      // Enable graphics list update.
      MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

      // Show the result.
      MdispSelect(MilDisplay, MilRemapped8BitImage);

      MosPrintf(MIL_TEXT("The number of cookies in each location has been ")
                MIL_TEXT("calculated using the height.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }
   else
      {
      // Enable graphics list update.
      MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

      MosPrintf(MIL_TEXT("Could not find the cookie box.\n"));  
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   MbufFree(MilRemapped8BitImage);
   MilRemapped8BitImage = M_NULL;
   }

//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CCookieCounting::AllocProcessingObjects(MIL_ID MilSystem)
   {
   m_MilSystem = MilSystem;

   MmodAllocResult(MilSystem, M_DEFAULT, &m_MilModelResult);
   MmodRestore(COOKIE_BOX_MODEL, MilSystem, M_DEFAULT, &m_MilModel);

   // Associate the depth map calibration.
   McalRestore(DEPTH_MAP_CALIBRATION, MilSystem, M_DEFAULT, &m_MilDepthMapCalibration);
   MmodControl(m_MilModel, M_ALL, M_ASSOCIATED_CALIBRATION, m_MilDepthMapCalibration);

   // Preprocess the model.
   MmodPreprocess(m_MilModel, M_DEFAULT);         
   }

//*******************************************************************************
// Frees processing objects.
//*******************************************************************************
void CCookieCounting::FreeProcessingObjects()
   {
   MmodFree(m_MilModel);               m_MilModel               = M_NULL;
   MmodFree(m_MilModelResult);         m_MilModelResult         = M_NULL;

   McalFree(m_MilDepthMapCalibration); m_MilDepthMapCalibration = M_NULL;
   }
