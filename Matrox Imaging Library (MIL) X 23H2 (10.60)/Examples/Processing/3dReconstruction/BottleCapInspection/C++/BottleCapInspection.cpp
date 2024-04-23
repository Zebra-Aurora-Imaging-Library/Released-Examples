//*******************************************************************************
// 
// File name: BottleCapInspection.cpp  
//
// Synopsis: Demonstrates the inspection of bottle caps using 3D data.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include "BottleCapInspection.h"

static const MIL_STRING FILENAMES[2] = {EX_PATH("Bottles_Cam0.ply"),
                                        EX_PATH("Bottles_Cam1.ply")};

static const MIL_STRING ILLUSTRATION_FILE[2] = {EX_PATH("BottleBox.png"),
                                                EX_PATH("BottleCapAnalysis.png")};
static const MIL_INT ILLUSTRATION_OFFSET_Y = 700;
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;
static const MIL_INT NBCLOUDS = 2;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("BottleCapInspection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the inspection of bottle caps using ")
             MIL_TEXT("3d\npoint clouds. "));
   MosPrintf(MIL_TEXT("\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Display, Buffer, Graphics, Image Processing,\n")
             MIL_TEXT("Calibration, "));
   MosPrintf(MIL_TEXT("3D Image Processing, Model finder, 3D Metrology and 3D Display. \n\n"));
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

   // Show illustration of the bottles.
   MIL_ID IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_FILE[0], MilSystem, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_Y, ILLUSTRATION_OFFSET_Y);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Visualization volume information.
   SMapGeneration MapData;
   MapData.BoxCornerX =  -25.00;
   MapData.BoxCornerY =    8.00;
   MapData.BoxCornerZ =   24.00;
   MapData.BoxSizeX   =  220.00;
   MapData.BoxSizeY   =  266.00;
   MapData.BoxSizeZ   =  -30.00;
   MapData.MapSizeX   =  830;
   MapData.MapSizeY   = 1020;
   MapData.PixelSizeX = MapData.BoxSizeX / (MapData.MapSizeX - 1.0);
   MapData.PixelSizeY = MapData.BoxSizeY / (MapData.MapSizeY - 1.0);
   MapData.GrayScaleZ = MapData.BoxSizeZ / 65534.0;
   MapData.IntensityMapType = 8 + M_UNSIGNED;
   MapData.SetExtractOverlap = true;
   MapData.ExtractOverlap = M_MAX_Z;
   MapData.FillXThreshold = 1.0;
   MapData.FillYThreshold = 1.0;

   MosPrintf(MIL_TEXT("Input files are imported and merged into a single cloud.\n\n"));

  // Import the acquired 3d point clouds.
   MIL_UNIQUE_BUF_ID ContainerIds[NBCLOUDS];
   for(MIL_INT i = 0; i < NBCLOUDS; ++i)
      {
      ContainerIds[i] = MbufRestore(FILENAMES[i], MilSystem, M_UNIQUE_ID);
      }

   // Acquired clouds from two camera are merged into one container.
   MIL_UNIQUE_BUF_ID ContainerId = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   M3dimMerge(ContainerIds, ContainerId, NBCLOUDS, M_NULL, M_DEFAULT);

   MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);
   if(MilDisplay3D)
      {
      M3ddispControl(MilDisplay3D, M_TITLE, MIL_TEXT("Merged Cloud"));
      M3ddispSetView(MilDisplay3D, M_AZIM_ELEV_ROLL, 310.0, -70.0, 145.0, M_DEFAULT);

      M3ddispSelect(MilDisplay3D, ContainerId, M_SELECT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_ZOOM, 1.8, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   MosPrintf(MIL_TEXT("The 3D cloud is projected into a depth map for 2D analysis.\n\n"));

   // Generate the depth map (orthogonal 2D-projection) of the acquired 3D point cloud.
   MIL_ID BottleCapsDepthmap = M_NULL;
   GenerateDepthMap(ContainerId, MilSystem, MapData, &BottleCapsDepthmap);

   // Show an illustration of the analyzed image.
   MbufFree(IllustrationImageId);
   IllustrationImageId = MbufRestore(ILLUSTRATION_FILE[1], MilSystem, M_NULL);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   // Analyze the generated depth map.
   CAnalyzeBottleCap ProbObj;
   ProbObj.AllocProcessingObjects(MilSystem);
   ProbObj.Analyze(BottleCapsDepthmap);
   ProbObj.FreeProcessingObjects();

   if(BottleCapsDepthmap != M_NULL)
      { MbufFree(BottleCapsDepthmap); }
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
void CAnalyzeBottleCap::Analyze(MIL_ID MilDepthMap)
   {
   // Processing display zoom factor.
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_X = 0.8;
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_Y = 0.8;

   // Color specifications.
   const MIL_DOUBLE PROC_PASS_COLOR = M_COLOR_GREEN;
   const MIL_DOUBLE PROC_FAIL_COLOR = M_COLOR_RED;

   const MIL_INT    CAP_DELTA_X          = 40;
   const MIL_INT    CAP_DELTA_Y          = 40;
   const MIL_INT    MAX_CAP_MISSING_DATA = 1000;

   const MIL_INT    PLANE_DELTA_X        = 40;
   const MIL_INT    PLANE_DELTA_Y        = 40;
   const MIL_INT    PLANE_SIZE_X         = PLANE_DELTA_X*2;
   const MIL_INT    PLANE_SIZE_Y         = PLANE_DELTA_Y*2;
   const MIL_DOUBLE ANGLE_TOLERANCE_DEG  = 4.0;
   const MIL_DOUBLE HEIGHT_TOLERANCE     = 2.0;   

   // Setup the display.
   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID  MilGraphicList = MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID , MilGraphicList);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X  , ILLUSTRATION_OFFSET_X);

   MIL_ID MilGraphics = M_DEFAULT;
   MgraControl(MilGraphics, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraControl(MilGraphics, M_FONT_X_SCALE, 2);
   MgraControl(MilGraphics, M_FONT_Y_SCALE, 2);

   MIL_ID MilGeometry          = m_Geometry;
   MIL_ID MilReferenceGeometry = m_ReferenceGeometry;
   MIL_DOUBLE AverageHeight = 0.0;

   // Disable graphics list update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Set 0's to invalid data.
   MbufClearCond(MilDepthMap, 65535, 65535, 65535, MilDepthMap, M_EQUAL, 0);

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

   MgraClear(M_DEFAULT, MilGraphicList);

   // Disassociate the calibration from the binarized image because we will not use it.
   McalAssociate(M_NULL, MilRemapped8BitImage, M_DEFAULT);

   MdispSelect(MilDisplay, MilRemapped8BitImage);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Find the bottle caps.
   MmodFind(m_CapModel, MilRemapped8BitImage, m_CapModelResult);

   // Get information on the find.   
   std::vector<MIL_INT> PositionX, PositionY;
   MmodGetResult(m_CapModelResult, M_DEFAULT, M_POSITION_X, PositionX);
   MmodGetResult(m_CapModelResult, M_DEFAULT, M_POSITION_Y, PositionY);

   MIL_INT NumOfOccurrences = static_cast<MIL_INT>(PositionX.size());

   SortCapPositions(&PositionX[0], &PositionY[0], NumOfOccurrences);

   MIL_UNIQUE_3DIM_ID MilStatResultId = M3dimAllocResult(m_MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   if (NumOfOccurrences > 0)
      {
      CAnalyzeBottleCap::SResults* BottleResults = new CAnalyzeBottleCap::SResults[NumOfOccurrences];

      // Check measurements on each bottle cap location.
      for (MIL_INT i = 0; i < NumOfOccurrences; i++)
         {
         MIL_INT PosX, PosY;
         MIL_TEXT_CHAR OccIdxStr[MAX_STRING_LEN];
         MosSprintf(OccIdxStr, MAX_STRING_LEN, MIL_TEXT("%2d"), (int) i);

         PosX = PositionX[i] - CAP_DELTA_X;
         PosY = PositionY[i] - CAP_DELTA_Y;

         MIL_ID CapChild;
         MIL_INT MissingData = 0;
         MbufChild2d(MilDepthMap, 
                     PosX, 
                     PosY,
                     CAP_DELTA_X*2, CAP_DELTA_Y*2, 
                     &CapChild);

         // Check if the bottle is open by looking for missing data.
         M3dimStat(M_STAT_CONTEXT_NUMBER_OF_POINTS, CapChild, MilStatResultId, M_DEFAULT );
         M3dimGetResult(MilStatResultId, M_NUMBER_OF_POINTS_MISSING_DATA, &MissingData);

         MosSprintf(BottleResults[i].MissingData, MAX_STRING_LEN, MIL_TEXT("%d"), MissingData);

         if (MissingData > MAX_CAP_MISSING_DATA)
            {
            MosSprintf(BottleResults[i].Status, MAX_STRING_LEN, MIL_TEXT("open"));
            MosSprintf(BottleResults[i].Angle, MAX_STRING_LEN, MIL_TEXT("n/a"));
            MosSprintf(BottleResults[i].MeanDeviation, MAX_STRING_LEN, MIL_TEXT("n/a"));

            MgraColor(MilGraphics, PROC_FAIL_COLOR);

            MgraText(MilGraphics, MilGraphicList, PosX+10, PosY+20, MIL_TEXT("open"));
            }
         else
            {
            // Create a child for location of plane fit.
            MIL_ID MilDepthMapChild =
            MbufChild2d(MilDepthMap,
                        PositionX[i] - PLANE_DELTA_X,
                        PositionY[i] - PLANE_DELTA_Y, 
                        PLANE_SIZE_X,
                        PLANE_SIZE_Y,
                        M_NULL);
          

            MIL_UNIQUE_3DMET_ID FitResultId = M3dmetAllocResult(m_MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
            const MIL_DOUBLE FIT_OUTLIER_DISTANCE = 2.0;
            // Define the plane Ax + By + Z0 = D using the mask.
            M3dmetFit(M_DEFAULT, MilDepthMapChild, M_PLANE, FitResultId,
                               FIT_OUTLIER_DISTANCE, M_DEFAULT);

            MbufFree(MilDepthMapChild);

            MIL_INT Status;
            M3dmetGetResult(FitResultId, M_STATUS, &Status);
            if(Status == M_SUCCESS)
               {
               MIL_DOUBLE A, B, C;
               M3dmetCopyResult(FitResultId, MilGeometry, M_FITTED_GEOMETRY, M_DEFAULT);
               // Get the plane coefficients.
               M3dgeoInquire(MilGeometry, M_COEFFICIENT_A, &A);
               M3dgeoInquire(MilGeometry, M_COEFFICIENT_B, &B);
               C = -1.0; // by definition of z(x,y) = Z0 + AX*x + Ay*y

               // Calculate the dot product between ref plane and cap plane 
               // assuming the plane is horizontal with normal (0, 0, -1).
               MIL_DOUBLE PlaneDotProduct = -C;

               // Get the length of the vectors.
               MIL_DOUBLE RefVectorLength = 1.0;  // Length of (0, 0, -1)
               MIL_DOUBLE CapVectorLength = sqrt(A*A + B*B + C*C);

               // Calculate the angle between the reference plane and the cap plane.
               MIL_DOUBLE AngleRad = acos(PlaneDotProduct / (RefVectorLength * CapVectorLength));
               MIL_DOUBLE AngleDeg = (AngleRad * 180.0) / 3.14159;
               MosSprintf(BottleResults[i].Angle, MAX_STRING_LEN, MIL_TEXT("%.2f"), AngleDeg);

               if (AngleDeg < ANGLE_TOLERANCE_DEG)
                  {
                  // Check the elevation relative to the reference plane.
                  MIL_ID StatResult = M3dmetAllocResult(m_MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_NULL);
                  M3dmetStat(M_STAT_CONTEXT_MEAN, CapChild, MilReferenceGeometry, StatResult, M_ABSOLUTE_DISTANCE_TO_SURFACE, M_ALL, M_NULL, M_NULL, M_DEFAULT);
                  M3dmetGetResult(StatResult, M_STAT_MEAN, &AverageHeight);
                  M3dmetFree(StatResult);
                  MosSprintf(BottleResults[i].MeanDeviation, MAX_STRING_LEN, MIL_TEXT("%.2f"), AverageHeight);

                  if (AverageHeight > HEIGHT_TOLERANCE)
                     {
                     MosSprintf(BottleResults[i].Status, MAX_STRING_LEN, MIL_TEXT("elevated"));
                     MgraColor(MilGraphics, PROC_FAIL_COLOR);
                     MgraText(MilGraphics, MilGraphicList, PosX-5, PosY+20, MIL_TEXT("elevated"));
                     }
                  else
                     {
                     MosSprintf(BottleResults[i].Status, MAX_STRING_LEN, MIL_TEXT("pass"));
                     MgraColor(MilGraphics, PROC_PASS_COLOR);
                     MgraRect(MilGraphics, MilGraphicList,PosX, PosY, PosX+CAP_DELTA_X*2, PosY+CAP_DELTA_Y*2);
                     }
                  }
               else
                  {
                  MosSprintf(BottleResults[i].Status, MAX_STRING_LEN, MIL_TEXT("tilted"));
                  MosSprintf(BottleResults[i].MeanDeviation, MAX_STRING_LEN, MIL_TEXT("n/a"));

                  MgraColor(MilGraphics, PROC_FAIL_COLOR);
                  MgraText(MilGraphics, MilGraphicList, PosX+10, PosY+20, MIL_TEXT("tilted"));
                  }
               }
            }

         // Draw the occurrence number in the current color.
         MgraText(MilGraphics, MilGraphicList, PosX-50, PosY-50, OccIdxStr);
         MbufFree(CapChild);
         }

      // Enable graphics list update.
      MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

      // Show the result.
      MdispSelect(MilDisplay, MilRemapped8BitImage);

      MosPrintf(MIL_TEXT("The bottle caps have been extracted and the inspection ")
                MIL_TEXT("results are displayed.\nFor each cap that was found, its ")
                MIL_TEXT("inclination was verified relative to a \nknown reference ")
                MIL_TEXT("bottle cap to determine whether it was tilted.\n\n"));

      MosPrintf(MIL_TEXT("---------------------------------------------------------------\n"));
      MosPrintf(MIL_TEXT("Index   Missing Data   Angle Deg.  Mean Deviation      Status  \n"));
      MosPrintf(MIL_TEXT("---------------------------------------------------------------\n"));

      for (MIL_INT i = 0; i < NumOfOccurrences; i++)
         {
         MosPrintf(MIL_TEXT("  %-2d     %-12s    %-5s     %10s           %-1s\n"), i, 
            BottleResults[i].MissingData, BottleResults[i].Angle,
            BottleResults[i].MeanDeviation, BottleResults[i].Status);
         }

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();

      delete [] BottleResults;
      }
   else
      {
      MosPrintf(MIL_TEXT("Error: No bottle caps were found.\n\n"));
      }

   MbufFree(MilRemapped8BitImage);
   }

//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CAnalyzeBottleCap::AllocProcessingObjects(MIL_ID MilSystem)
   {
   m_MilSystem = MilSystem;

   MIL_CONST_TEXT_PTR CAP_MODEL = EX_PATH("CapModel.mmf");
   const MIL_DOUBLE CAP_REF_PLANE_HEIGHT = -4.0;

   // Restore and setup the cap model.
   MmodAllocResult(MilSystem, M_DEFAULT, &m_CapModelResult);
   MmodRestore(CAP_MODEL, MilSystem, M_DEFAULT, &m_CapModel);

   // Preprocess the model.
   MmodPreprocess(m_CapModel, M_DEFAULT);

   // Allocate a geometry object.
   M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, &m_Geometry);

   // Allocate a geometry object to use as the caps reference plane.
   M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, &m_ReferenceGeometry);
   M3dgeoPlane(m_ReferenceGeometry, M_COEFFICIENTS,0.0,0.0,1, CAP_REF_PLANE_HEIGHT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

//*******************************************************************************
// Function that frees processing objects.
//*******************************************************************************
void CAnalyzeBottleCap::FreeProcessingObjects()
   {
   MmodFree(m_CapModel); m_CapModel = M_NULL;
   MmodFree(m_CapModelResult); m_CapModelResult = M_NULL;

   M3dgeoFree(m_Geometry); m_Geometry = M_NULL;
   M3dgeoFree(m_ReferenceGeometry); m_ReferenceGeometry = M_NULL;
   }

//*******************************************************************************
// Function to sort the found cap positions.
//*******************************************************************************
void CAnalyzeBottleCap::SortCapPositions(MIL_INT* pX, MIL_INT* pY, MIL_INT Nb)
   {
   const MIL_INT RowMaxYDeviation = 80;

   // Use a simple non-optimal sort for the sake of simplicity.
   // Sort in Y then X.
   for(MIL_INT i = 0; i < Nb; i++)
      {
      for(MIL_INT j = 0; j < Nb; j++)
         {
         bool SwapPos = false;

         MIL_INT DeltaY = (pY[i] - pY[j]);
         if(abs((MIL_INT32)DeltaY) <= RowMaxYDeviation) // Same row
            {
            MIL_INT DeltaX = pX[i] - pX[j]; // column discriminate
            SwapPos = (DeltaX < 0);
            }
         else
            {
            SwapPos = (DeltaY < 0); // row discriminate
            }

         if(SwapPos)
            {
            std::swap(pX[i], pX[j]);
            std::swap(pY[i], pY[j]);
            }
         }
      }
   }
