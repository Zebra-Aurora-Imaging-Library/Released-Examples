﻿//***************************************************************************************
// 
// File name: TireStringRead.cpp  
//
// Synopsis: Example that reads characters on a tire using 3d data.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************
#include "TireStringRead.h"


//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("TireStringRead\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the reading of strings on a tire using\n")
             MIL_TEXT("3D point clouds.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Display, Buffer, Graphics,\n")
             MIL_TEXT("Image Processing, 3D Image Processing, 3D Display, 3D Geometry\n")
             MIL_TEXT("3D Graphics, Model Finder, Measurement, String Reader.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Check for the required example files.
   if(!CheckForRequiredMILFile(FILENAME))
      { return -1; }

   // Visualization volume information.
   SMapGeneration MapData;
   MapData.BoxCornerX = -29.80;
   MapData.BoxCornerY = -0.21;
   MapData.BoxCornerZ = 1.86;
   MapData.BoxSizeX = 229.00;
   MapData.BoxSizeY = 247.00;
   MapData.BoxSizeZ = -19.00;
   MapData.MapSizeX = 842;
   MapData.MapSizeY = 906;
   MapData.PixelSizeX = 0.273;
   MapData.PixelSizeY = 0.273;
   MapData.GrayScaleZ = (MapData.BoxSizeZ / 65534.0);
   MapData.IntensityMapType = 8 + M_UNSIGNED;
   MapData.SetExtractOverlap = false;
   MapData.ExtractOverlap = M_MAX_Z;
   MapData.FillXThreshold = 1.0;
   MapData.FillYThreshold = 1.0;

   MosPrintf(MIL_TEXT("Reading the input PLY file.\n\n"));

   // Import the acquired 3D point clouds.
   MIL_UNIQUE_BUF_ID ContainerId = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MbufImport(FILENAME, M_DEFAULT, M_LOAD, MilSystem, &ContainerId);

   MIL_ID MilDisplay3D = Alloc3dDisplayId(MilSystem);

   // Display point cloud.
   if(MilDisplay3D)
      {
      M3ddispControl(MilDisplay3D, M_TITLE, MIL_TEXT("3D Cloud"));
      M3ddispSetView(MilDisplay3D, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("The point cloud is shown in a 3D display.\n\n"));
      M3ddispSelect(MilDisplay3D, ContainerId, M_SELECT, M_DEFAULT);

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   MosPrintf(MIL_TEXT("The 3D point cloud is projected into a depth map for 2D analysis.\n\n"));

   // Generate the depth map (orthogonal 2D-projection) of the acquired 3D point cloud.
   MIL_ID TireDepthmap = M_NULL;
   GenerateDepthMap(ContainerId, MilSystem, MapData, &TireDepthmap);

   // Analyze the generated depth map.
   CTireStringRead ProbObj;
   ProbObj.AllocProcessingObjects(MilSystem);
   ProbObj.Analyze(MilDisplay3D, TireDepthmap);
   ProbObj.FreeProcessingObjects();

   // Free the allocated objects.
   if(TireDepthmap != M_NULL)
      { MbufFree(TireDepthmap); }
   if(MilDisplay3D)
      { M3ddispFree(MilDisplay3D); }

   return 0;
   }

//*******************************************************************************
// Function that analyzes the scanned object.
//*******************************************************************************
void CTireStringRead::Analyze(MIL_ID Mil3dDisplay, MIL_ID MilDepthMap)
   {
   //Display position
   const MIL_INT DISPLAY_POSITION_X = 800;

   // Processing display zoom factor.
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_X = 1;
   const MIL_DOUBLE PROC_DISPLAY_ZOOM_FACTOR_Y = 1;

   // Color specifications.
   const MIL_DOUBLE MEAS_COLOR        = M_COLOR_GREEN;
   const MIL_DOUBLE PROC_TEXT_COLOR   = M_COLOR_BLUE;

   const MIL_DOUBLE MEAS_RING_CENTER_X   = -220;
   const MIL_DOUBLE MEAS_RING_CENTER_Y   = 465;
   const MIL_DOUBLE MEAS_INNER_RADIUS    = 800;
   const MIL_DOUBLE MEAS_OUTER_RADIUS    = 870;
   const MIL_DOUBLE MEAS_NUM_SUB_REGIONS = 20;

   const MIL_DOUBLE POLAR_DELTA_RADIUS = 250;
   const MIL_DOUBLE POLAR_START_ANGLE  = 25;
   const MIL_DOUBLE POLAR_END_ANGLE    =-15;

   const MIL_INT FIRST_CHILD_OFFSET_X = -375;
   const MIL_INT FIRST_CHILD_OFFSET_Y = -20;
   const MIL_INT FIRST_CHILD_SIZE_X   = 340;
   const MIL_INT FIRST_CHILD_SIZE_Y   = 40;

   const MIL_INT SECOND_CHILD_OFFSET_X = 15;
   const MIL_INT SECOND_CHILD_OFFSET_Y = -14;
   const MIL_INT SECOND_CHILD_SIZE_X   = 295;
   const MIL_INT SECOND_CHILD_SIZE_Y   = 33;

   // Allocate the 2D display.
   MIL_UNIQUE_DISP_ID MilDisplay     = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID  MilGraphicList = MgraAllocList(m_MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID MilGraphics = M_DEFAULT;

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Adjust the display size.
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, DISPLAY_POSITION_X);

   // Disable graphics list update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Setup the display.
   MgraClear(M_DEFAULT, MilGraphicList);   
   MdispZoom(MilDisplay, PROC_DISPLAY_ZOOM_FACTOR_X, PROC_DISPLAY_ZOOM_FACTOR_Y);

   // Allocate a string point cloud to display the read strings.
   MIL_ID MilStringPointCloud = MbufAllocContainer(m_MilSystem, M_DISP + M_PROC, M_DEFAULT, M_NULL);

   // Allocate the necessary buffers for processing.
   MIL_ID MilEqualizedImage, MilRemapped8BitImage, MilDepthResultMask;
   MbufAlloc2d(m_MilSystem,
               MbufInquire(MilDepthMap, M_SIZE_X, M_NULL),
               MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL),
               16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilEqualizedImage);

   MbufAlloc2d(m_MilSystem,
               MbufInquire(MilDepthMap, M_SIZE_X, M_NULL),
               MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL),
               8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilRemapped8BitImage);

   MbufAlloc2d(m_MilSystem,
               MbufInquire(MilDepthMap, M_SIZE_X, M_NULL),
               MbufInquire(MilDepthMap, M_SIZE_Y, M_NULL),
               8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDepthResultMask);

   MbufClear(MilEqualizedImage, 0);
   MbufClear(MilRemapped8BitImage, 0);
   MbufClear(MilDepthResultMask, 0);

   // Do an adaptive equalize of the depth map image.
   MimHistogramEqualizeAdaptive(m_MilAdaptiveEqualizeContext, 
                                MilDepthMap, 
                                MilEqualizedImage, M_DEFAULT);

   // Remap to 8 bit.
   MimShift(MilEqualizedImage, MilRemapped8BitImage, -8);

   MdispSelect(MilDisplay, MilRemapped8BitImage);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Find the arc using measurement.
   MmeasSetMarker(m_MilCircleMarker, M_POLARITY, M_NEGATIVE, M_NEGATIVE);

   MmeasSetMarker(m_MilCircleMarker, M_RING_CENTER, MEAS_RING_CENTER_X, MEAS_RING_CENTER_Y);
   MmeasSetMarker(m_MilCircleMarker, M_RING_RADII , MEAS_INNER_RADIUS, MEAS_OUTER_RADIUS);
   MmeasSetMarker(m_MilCircleMarker, M_SUB_REGIONS_NUMBER, MEAS_NUM_SUB_REGIONS, M_NULL);  

   // Find the circle and measure its position and radius.
   MmeasFindMarker(M_DEFAULT, MilRemapped8BitImage, m_MilCircleMarker, M_DEFAULT);

   // If occurrence is found, show the results.
   MIL_INT NumberOccurrencesFound = 0;
   MmeasGetResult(m_MilCircleMarker, M_NUMBER + M_TYPE_MIL_INT, &NumberOccurrencesFound, M_NULL);

   if (NumberOccurrencesFound >= 1)
      {
      MIL_DOUBLE CircleCenterX, CircleCenterY, CircleRadius;
      MmeasSetMarker(m_MilCircleMarker, M_RESULT_OUTPUT_UNITS, M_PIXEL, M_NULL);
      MmeasGetResult(m_MilCircleMarker, M_POSITION, &CircleCenterX, &CircleCenterY);
      MmeasGetResult(m_MilCircleMarker, M_RADIUS  , &CircleRadius, M_NULL);

      // Using the circle, unwrap with a polar transform.
      MIL_DOUBLE  SizeRadius,SizeAngle;
      MimPolarTransform(MilRemapped8BitImage, M_NULL, 
                        CircleCenterX, CircleCenterY, 
                        CircleRadius-POLAR_DELTA_RADIUS, 
                        CircleRadius+POLAR_DELTA_RADIUS, 
                        POLAR_START_ANGLE, POLAR_END_ANGLE, 
                        M_RECTANGULAR_TO_POLAR, 
                        M_NEAREST_NEIGHBOR + M_OVERSCAN_ENABLE, 
                        &SizeAngle, &SizeRadius);
   
      MIL_INT SizeX = (MIL_INT) ceil(SizeAngle);
      MIL_INT SizeY = (MIL_INT) ceil(SizeRadius);

      MIL_ID MilUnWrappedImage =
      MbufAlloc2d(m_MilSystem, SizeX, SizeY, 8, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MbufClear(MilUnWrappedImage, 0);

      MIL_ID MilUnWrappedMask = MbufAlloc2d(m_MilSystem, SizeX, SizeY, 8, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MbufClear(MilUnWrappedMask, 0);

      MimPolarTransform(MilRemapped8BitImage, MilUnWrappedImage, 
                        CircleCenterX, CircleCenterY, 
                        CircleRadius-POLAR_DELTA_RADIUS, 
                        CircleRadius+POLAR_DELTA_RADIUS, 
                        POLAR_START_ANGLE, POLAR_END_ANGLE, 
                        M_RECTANGULAR_TO_POLAR, 
                        M_NEAREST_NEIGHBOR + M_OVERSCAN_ENABLE, 
                        &SizeAngle, &SizeRadius);

      // Clear the graphics list.
      MgraClear(M_DEFAULT, MilGraphicList);

      MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

      MdispSelect(MilDisplay, MilUnWrappedImage);

      // Find the model shape around the second string.
      MmodFind(m_MilModel, MilUnWrappedImage, m_MilModelResult);

      MIL_INT NumOfOccurences = 0;
      MmodGetResult(m_MilModelResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOfOccurences);

      if (NumOfOccurences >= 1)
         {
         // Get the reference point position.
         MIL_DOUBLE RefPointX, RefPointY;

         MmodControl(m_MilModelResult, M_DEFAULT, M_RESULT_OUTPUT_UNITS, M_PIXEL);

         MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_X + M_TYPE_MIL_DOUBLE, &RefPointX);
         MmodGetResult(m_MilModelResult, M_DEFAULT, M_POSITION_Y + M_TYPE_MIL_DOUBLE, &RefPointY);

         // Create a child around the two strings relative to the reference point.
         MIL_INT FirstOffsetX = (MIL_INT)(RefPointX + FIRST_CHILD_OFFSET_X);
         MIL_INT FirstOffsetY = (MIL_INT)(RefPointY + FIRST_CHILD_OFFSET_Y);

         MIL_ID MilFirstStringChildImage = 
         MbufChild2d(MilUnWrappedImage, FirstOffsetX, FirstOffsetY, FIRST_CHILD_SIZE_X, FIRST_CHILD_SIZE_Y, M_NULL);

         MIL_INT SecondOffsetX = (MIL_INT)RefPointX + SECOND_CHILD_OFFSET_X;
         MIL_INT SecondOffsetY = (MIL_INT)(RefPointY + SECOND_CHILD_OFFSET_Y);

         MIL_ID MilSecondStringChildImage =
         MbufChild2d(MilUnWrappedImage, SecondOffsetX, SecondOffsetY, SECOND_CHILD_SIZE_X, SECOND_CHILD_SIZE_Y, M_NULL);

         // Read the first string.
         MstrRead(m_MilFirstStringReader, MilFirstStringChildImage, m_MilFirstStringReaderResult);
         // Read the second string.
         MstrRead(m_MilSecondStringReader, MilSecondStringChildImage, m_MilSecondStringReaderResult);

         MIL_INT NumberOfStringRead = 0;
         MstrGetResult(m_MilFirstStringReaderResult, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

         MgraControl(MilGraphics, M_BACKGROUND_MODE, M_OPAQUE);
         MgraColor(MilGraphics, M_COLOR_GREEN);
         MgraControl(MilGraphics, M_FONT_SIZE, TEXT_FONT_SIZE_SMALL);
         MgraText(MilGraphics, MilGraphicList, TEXT_OFFSET_X, TEXT_OFFSET_Y, MIL_TEXT("Read strings in unwrapped depth map"));

         // Show the wrapped tire zoomed out in the top right corner.
         const MIL_DOUBLE ZoomFactor = 0.3;

         MIL_INT ZoomSizeX = (MIL_INT)(MbufInquire(MilRemapped8BitImage, M_SIZE_X, M_NULL) * ZoomFactor);
         MIL_INT ZoomSizeY = (MIL_INT)(MbufInquire(MilRemapped8BitImage, M_SIZE_Y, M_NULL) * ZoomFactor);

         MIL_ID MilResizedImage = 
         MbufAlloc2d(m_MilSystem, ZoomSizeX, ZoomSizeY, 8, M_IMAGE + M_PROC, M_NULL);
         MimResize(MilRemapped8BitImage, MilResizedImage, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BICUBIC);

         MIL_ID MilRotatedImage = MbufAlloc2d(m_MilSystem, ZoomSizeY, ZoomSizeX, 8, M_IMAGE + M_PROC, M_NULL);
         MimRotate(MilResizedImage, MilRotatedImage, 90, (MIL_DOUBLE)(ZoomSizeX/2), 
                  (MIL_DOUBLE)(ZoomSizeY/2), (MIL_DOUBLE)(ZoomSizeY/2), 
                  (MIL_DOUBLE)(ZoomSizeX/2), M_BICUBIC);

         const MIL_INT TireOffsetY = 50;
         const MIL_INT TireSizeY = 550;

         MIL_ID MilUnWrappedImageChild =
         MbufChild2d(MilUnWrappedImage,
                     MbufInquire(MilUnWrappedImage, M_SIZE_X, M_NULL)-ZoomSizeY,         
                     0, ZoomSizeY, (MIL_INT)(TireSizeY*ZoomFactor), M_NULL);

         MbufCopyColor2d(MilRotatedImage, MilUnWrappedImageChild, M_ALL_BANDS, 0, 
                         (MIL_INT)(TireOffsetY*ZoomFactor), M_ALL_BANDS, 0, 0, ZoomSizeY, 
                         (MIL_INT)(TireSizeY*ZoomFactor));

         MbufFree(MilResizedImage);
         MbufFree(MilRotatedImage);

         // Annotate the acquired depth map.
         MgraControl(MilGraphics, M_BACKGROUND_MODE, M_OPAQUE);
         MgraColor(MilGraphics, M_COLOR_GREEN);
         MgraControl(MilGraphics, M_FONT_SIZE, TEXT_FONT_SIZE_SMALL);
         MgraText(MilGraphics, MilGraphicList, 
                  MbufInquire(MilUnWrappedImage, M_SIZE_X, M_NULL)-ZoomSizeY+TEXT_OFFSET_X, 
                  TEXT_OFFSET_Y, MIL_TEXT("Acquired depth map"));

         MgraControl(MilGraphics, M_BACKGROUND_MODE, M_TRANSPARENT);

         if(NumberOfStringRead >= 1)
            {
            // Draw the first string mask.
            MIL_ID MilFirstStringMask =
            MbufChild2d(MilUnWrappedMask, FirstOffsetX, FirstOffsetY, FIRST_CHILD_SIZE_X, FIRST_CHILD_SIZE_Y, M_NULL);
            MgraColor(MilGraphics, 255);
            MstrDraw(MilGraphics, m_MilFirstStringReaderResult, MilFirstStringMask, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT);
            MbufFree(MilFirstStringMask);

            // Draw the first string.
            MgraColor(MilGraphics, PROC_TEXT_COLOR);
            MgraText(MilGraphics, MilGraphicList, FirstOffsetX, FirstOffsetY-30, MIL_TEXT("Embossed"));

            MgraControl(MilGraphics, M_DRAW_OFFSET_X, -FirstOffsetX);
            MgraControl(MilGraphics, M_DRAW_OFFSET_Y, -FirstOffsetY);
            MstrDraw(MilGraphics, m_MilFirstStringReaderResult, MilGraphicList, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT);
            }
         else
            {
            MosPrintf(MIL_TEXT("Required string was not found.\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();
            }

         NumberOfStringRead = 0;
         MstrGetResult(m_MilSecondStringReaderResult, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &NumberOfStringRead);

         if( NumberOfStringRead >= 1)
            {
            // Draw the second string mask.
            MIL_ID MilSecondStringMask =
            MbufChild2d(MilUnWrappedMask, SecondOffsetX, SecondOffsetY, SECOND_CHILD_SIZE_X, SECOND_CHILD_SIZE_Y, M_NULL);
            MgraColor(MilGraphics, 255);
            MstrDraw(MilGraphics, m_MilSecondStringReaderResult, MilSecondStringMask, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT);
            MbufFree(MilSecondStringMask);
            
            // Draw the second string.
            MgraControl(MilGraphics, M_DRAW_OFFSET_X, M_DEFAULT);
            MgraControl(MilGraphics, M_DRAW_OFFSET_Y, M_DEFAULT);

            MgraColor(MilGraphics, PROC_TEXT_COLOR);
            MgraText(MilGraphics, MilGraphicList, SecondOffsetX, SecondOffsetY-30, MIL_TEXT("Imprinted"));

            MgraControl(MilGraphics, M_DRAW_OFFSET_X, -SecondOffsetX);
            MgraControl(MilGraphics, M_DRAW_OFFSET_Y, -SecondOffsetY);

            MstrDraw(MilGraphics, m_MilSecondStringReaderResult, MilGraphicList, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT);

            // Wrap back the mask.
            MimPolarTransform(MilUnWrappedMask, MilDepthResultMask,
                              CircleCenterX, CircleCenterY,
                              CircleRadius - POLAR_DELTA_RADIUS,
                              CircleRadius + POLAR_DELTA_RADIUS,
                              POLAR_START_ANGLE, POLAR_END_ANGLE,
                              M_POLAR_TO_RECTANGULAR,
                              M_NEAREST_NEIGHBOR + M_OVERSCAN_CLEAR,
                              &SizeAngle, &SizeRadius);

            if(Mil3dDisplay)
               {
               MIL_ID Mil3dGraList;
               M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, &Mil3dGraList);

               // Get a crop point cloud from the depth map and display as a solid color.
               MbufConvert3d(MilDepthMap, MilStringPointCloud, M_NULL, M_DEFAULT, M_DEFAULT);
               M3dimCrop(MilStringPointCloud, MilStringPointCloud, MilDepthResultMask, M_NULL, M_UNORGANIZED, M_DEFAULT);
               MIL_INT64 StringPointCloudLabel = M3dgraAdd(Mil3dGraList, M_DEFAULT, MilStringPointCloud, M_DEFAULT);
               M3dgraControl(Mil3dGraList, StringPointCloudLabel, M_THICKNESS, 5);
               M3dgraControl(Mil3dGraList, StringPointCloudLabel, M_COLOR, PROC_TEXT_COLOR);
               M3dgraControl(Mil3dGraList, StringPointCloudLabel, M_COLOR_COMPONENT, M_NULL);
               }

            MosPrintf(MIL_TEXT("A polar transform was done to unroll the tire's ")
                      MIL_TEXT("sidewall and\nthe two strings have been read.\n\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
            MosGetch();
            }
         else
            {
            MosPrintf(MIL_TEXT("Required string was not found.\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
            MosGetch();
            }

         MbufFree(MilUnWrappedImageChild);
         MbufFree(MilFirstStringChildImage);
         MbufFree(MilSecondStringChildImage);
         }
      else
         {
         MosPrintf(MIL_TEXT("Required model was not found.\n"));
         MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
         MosGetch();
         }

      MbufFree(MilUnWrappedMask);
      MbufFree(MilUnWrappedImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("Required circle was not found.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   MbufFree(MilStringPointCloud);
   MbufFree(MilDepthResultMask);
   MbufFree(MilEqualizedImage);
   MbufFree(MilRemapped8BitImage);
   }

//*******************************************************************************
// Function that allocates processing objects.
//*******************************************************************************
void CTireStringRead::AllocProcessingObjects(MIL_ID MilSystem)
   {
   m_MilSystem = MilSystem;

   MIL_CONST_TEXT_PTR FIRST_STRING_FONT   = EX_PATH("FirstStringFont.msr");
   MIL_CONST_TEXT_PTR SECOND_STRING_FONT  = EX_PATH("SecondStringFont.msr");
   MIL_CONST_TEXT_PTR SECOND_STRING_MODEL = EX_PATH("SecondStringModel.mmf");

   MimAlloc(MilSystem, M_HISTOGRAM_EQUALIZE_ADAPTIVE_CONTEXT, M_DEFAULT, &m_MilAdaptiveEqualizeContext);
   MmeasAllocMarker(MilSystem, M_CIRCLE, M_DEFAULT, &m_MilCircleMarker);      
   MmodAllocResult(MilSystem, M_DEFAULT, &m_MilModelResult);

   MmodRestore(SECOND_STRING_MODEL, MilSystem, M_DEFAULT, &m_MilModel);
   MmodPreprocess(m_MilModel, M_DEFAULT);

   // Restore the first string reader context.
   MstrRestore(FIRST_STRING_FONT, MilSystem, M_DEFAULT, &m_MilFirstStringReader);
   MstrAllocResult(MilSystem, M_DEFAULT, &m_MilFirstStringReaderResult);
   MstrPreprocess(m_MilFirstStringReader, M_DEFAULT);

   // Restore the second string reader context.
   MstrRestore(SECOND_STRING_FONT, MilSystem, M_DEFAULT, &m_MilSecondStringReader);
   MstrAllocResult(MilSystem, M_DEFAULT, &m_MilSecondStringReaderResult);
   MstrPreprocess(m_MilSecondStringReader, M_DEFAULT);
   }

//*******************************************************************************
// Function that frees processing objects.
//*******************************************************************************
void CTireStringRead::FreeProcessingObjects()
   {
   MimFree(m_MilAdaptiveEqualizeContext);    m_MilAdaptiveEqualizeContext  = M_NULL;
   MmeasFree(m_MilCircleMarker);             m_MilCircleMarker             = M_NULL;
   MmodFree(m_MilModel);                     m_MilModel                    = M_NULL;
   MmodFree(m_MilModelResult);               m_MilModelResult              = M_NULL;

   MstrFree(m_MilFirstStringReader);         m_MilFirstStringReader        = M_NULL;
   MstrFree(m_MilFirstStringReaderResult);   m_MilFirstStringReaderResult  = M_NULL;
   MstrFree(m_MilSecondStringReader);        m_MilSecondStringReader       = M_NULL;
   MstrFree(m_MilSecondStringReaderResult);  m_MilSecondStringReaderResult = M_NULL;
   }

