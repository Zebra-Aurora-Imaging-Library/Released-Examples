/*****************************************************************************/
/* 
 * File name: m3dmap.cpp
 *
 * Synopsis: This program inspects a wood surface using 
 *           sheet-of-light profiling (laser) to find any depth defects.
 *
 * Printable calibration grids in PDF format can be found in your
 * "Matrox Imaging/Images/" directory.
 *
 * When considering a laser-based 3D reconstruction system, the file "3D Setup Helper.xls"
 * can be used to accelerate prototyping by choosing an adequate hardware configuration
 * (angle, distance, lens, camera, ...). The file is located in your
 * "Matrox Imaging/Tools/" directory.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#include <math.h>

/* Example functions declarations. */  
void DepthCorrectionExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void CalibratedCameraExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* Utility functions declarations. */  
void PerformBlobAnalysis(MIL_ID MilSystem,
                         MIL_ID MilDisplay,
                         MIL_ID MilOverlayImage,
                         MIL_ID MilDepthMap);
void SetupColorDisplay(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_INT SizeBit);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);
/*****************************************************************************
 Main.
*****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,     /* Application identifier. */
          MilSystem,          /* System identifier.      */
          MilDisplay;         /* Display identifier.     */

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Run the depth correction example. */
   DepthCorrectionExample(MilSystem, MilDisplay);

   /* Run the calibrated camera example. */
   CalibratedCameraExample(MilSystem, MilDisplay);

   /* Free defaults. */    
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

/****************************************************************************
 Depth correction example.
****************************************************************************/

/* Input sequence specifications. */
#define REFERENCE_PLANES_SEQUENCE_FILE  M_IMAGE_PATH MIL_TEXT("ReferencePlanes.avi")
#define OBJECT_SEQUENCE_FILE            M_IMAGE_PATH MIL_TEXT("ScannedObject.avi")

/* Peak detection parameters. */
#define PEAK_WIDTH_NOMINAL         10
#define PEAK_WIDTH_DELTA            8
#define MIN_CONTRAST              140

/* Calibration heights in mm. */
static const double CORRECTED_DEPTHS[] = {1.25, 2.50, 3.75, 5.00};

#define SCALE_FACTOR   10000.0 /* (depth in world units) * SCALE_FACTOR gives gray levels */

/* Annotation position. */
#define CALIB_TEXT_POS_X         400   
#define CALIB_TEXT_POS_Y          15

void DepthCorrectionExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilOverlayImage,   /* Overlay image buffer identifier.              */
               MilImage,          /* Image buffer identifier (for processing).     */
               MilDepthMap,       /* Image buffer identifier (for results).        */
               MilLaser,          /* 3dmap laser profiling context identifier.     */
               MilCalibScan,      /* 3dmap result buffer identifier for laser      */
                                  /* line calibration.                             */
               MilScan;           /* 3dmap result buffer identifier.               */
   MIL_INT     SizeX,             /* Width of grabbed images.                      */
               SizeY,             /* Height of grabbed images.                     */
               NbReferencePlanes, /* Number of reference planes of known heights.  */
               NbObjectImages;    /* Number of frames for scanned objects.         */
   int         n;                 /* Counter.                                      */
   MIL_DOUBLE  FrameRate,         /* Number of grabbed frames per second (in AVI). */
               StartTime,         /* Time at the beginning of each iteration.      */
               EndTime,           /* Time after processing for each iteration.     */
               WaitTime;          /* Time to wait for next frame.                  */

   /* Inquire characteristics of the input sequences. */
   MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, M_SIZE_X,           &SizeX);
   MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, M_SIZE_Y,           &SizeY);
   MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, M_NUMBER_OF_IMAGES, &NbReferencePlanes);
   MbufDiskInquire(REFERENCE_PLANES_SEQUENCE_FILE, M_FRAME_RATE,       &FrameRate);
   MbufDiskInquire(OBJECT_SEQUENCE_FILE,           M_NUMBER_OF_IMAGES, &NbObjectImages);

   /* Allocate buffer to hold images. */
   MbufAlloc2d(MilSystem, SizeX, SizeY,  8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilImage);
   MbufClear(MilImage, 0.0);

   MosPrintf(MIL_TEXT("\nDEPTH ANALYSIS:\n"));
   MosPrintf(MIL_TEXT("---------------\n\n"));
   MosPrintf(MIL_TEXT("This program performs a surface inspection to detect "));
   MosPrintf(MIL_TEXT("depth defects \n"));
   MosPrintf(MIL_TEXT("on a wood surface using a laser (sheet-of-light) "));
   MosPrintf(MIL_TEXT("profiling system.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Select display. */
   MdispSelect(MilDisplay, MilImage);

   /* Prepare for overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);

   /* Allocate 3dmap objects. */
   M3dmapAlloc(MilSystem, M_LASER, M_DEPTH_CORRECTION, &MilLaser);
   M3dmapAllocResult(MilSystem, M_LASER_CALIBRATION_DATA, M_DEFAULT, &MilCalibScan);

   /* Set laser line extraction options. */
   MIL_ID MilPeakLocator;
   M3dmapInquire(MilLaser, M_DEFAULT, M_LOCATE_PEAK_1D_CONTEXT_ID+M_TYPE_MIL_ID, &MilPeakLocator);
   MimControl(MilPeakLocator, M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL);
   MimControl(MilPeakLocator, M_PEAK_WIDTH_DELTA  , PEAK_WIDTH_DELTA  );
   MimControl(MilPeakLocator, M_MINIMUM_CONTRAST  , MIN_CONTRAST      );

   /* Open the calibration sequence file for reading. */
   MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL,
                                                                   M_NULL, M_NULL, M_OPEN);

   /* Read and process all images in the input sequence. */
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);

   for (n = 0; n < NbReferencePlanes; n++)
      {
      MIL_TEXT_CHAR CalibString[32];

      /* Read image from sequence. */
      MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, M_DEFAULT, M_LOAD, M_NULL,
                                                          &MilImage, M_DEFAULT, 1, M_READ);

      /* Annotate the image with the calibration height. */
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
      MosSprintf(CalibString, 32, MIL_TEXT("Reference plane %d: %.2f mm"),
                     (int)(n+1), CORRECTED_DEPTHS[n]);
      MgraText(M_DEFAULT, MilOverlayImage, CALIB_TEXT_POS_X, CALIB_TEXT_POS_Y, CalibString);

      /* Set desired corrected depth of next reference plane. */
      M3dmapControl(MilLaser, M_DEFAULT, M_CORRECTED_DEPTH,
                                                         CORRECTED_DEPTHS[n]*SCALE_FACTOR);

      /* Analyze the image to extract laser line. */
      M3dmapAddScan(MilLaser, MilCalibScan, MilImage, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);

      /* Wait to have a proper frame rate, if necessary. */
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &EndTime);
      WaitTime = (1.0 / FrameRate) - (EndTime - StartTime);
      if (WaitTime > 0)
         { MappTimer(M_DEFAULT, M_TIMER_WAIT, &WaitTime); }
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);
      }

   /* Close the calibration sequence file. */
   MbufImportSequence(REFERENCE_PLANES_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL,
                                                                  M_NULL, M_NULL, M_CLOSE);

   /* Calibrate the laser profiling context using reference planes of known heights. */
   M3dmapCalibrate(MilLaser, MilCalibScan, M_NULL, M_DEFAULT);

   MosPrintf(MIL_TEXT("The laser profiling system has been calibrated using 4 reference\n"));
   MosPrintf(MIL_TEXT("planes of known heights.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("The wood surface is being scanned.\n\n"));

   /* Free the result buffer used for calibration because it will not be used anymore. */
   M3dmapFree(MilCalibScan);
   MilCalibScan = M_NULL;

   /* Allocate the result buffer for the scanned depth corrected data. */
   M3dmapAllocResult(MilSystem, M_DEPTH_CORRECTED_DATA, M_DEFAULT, &MilScan);

   /* Open the object sequence file for reading. */
   MbufDiskInquire(OBJECT_SEQUENCE_FILE, M_FRAME_RATE, &FrameRate);
   MbufImportSequence(OBJECT_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL,
                                                                           M_NULL, M_OPEN);

   /* Read and process all images in the input sequence. */
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   for (n = 0; n < NbObjectImages; n++)
      {
      /* Read image from sequence. */
      MbufImportSequence(OBJECT_SEQUENCE_FILE, M_DEFAULT, M_LOAD, M_NULL, &MilImage,
                                                                     M_DEFAULT, 1, M_READ);

      /* Analyze the image to extract laser line and correct its depth. */
      M3dmapAddScan(MilLaser, MilScan, MilImage, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);

      /* Wait to have a proper frame rate, if necessary. */
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &EndTime);
      WaitTime = (1.0/FrameRate) - (EndTime - StartTime);
      if (WaitTime > 0)
         { MappTimer(M_DEFAULT, M_TIMER_WAIT, &WaitTime); }
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);
      }

   /* Close the object sequence file. */
   MbufImportSequence(OBJECT_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL,
                                                                          M_NULL, M_CLOSE);

   /* Allocate the image for a partially corrected depth map. */
   MbufAlloc2d(MilSystem, SizeX, NbObjectImages, 16 + M_UNSIGNED,
                                                       M_IMAGE+M_PROC+M_DISP, &MilDepthMap);

   /* Get partially corrected depth map from accumulated information in the result buffer. */
   M3dmapCopyResult(MilScan, M_DEFAULT, MilDepthMap,M_PARTIALLY_CORRECTED_DEPTH_MAP, M_DEFAULT);

   /* Disable display updates. */
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

   /* Show partially corrected depth map and find defects. */
   SetupColorDisplay(MilSystem, MilDisplay, MbufInquire(MilDepthMap, M_SIZE_BIT, M_NULL));
   
   /* Display partially corrected depth map. */
   MdispSelect(MilDisplay, MilDepthMap);
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

   MosPrintf(MIL_TEXT("The pseudo-color depth map of the surface is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   PerformBlobAnalysis(MilSystem, MilDisplay, MilOverlayImage, MilDepthMap);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Disassociates display LUT and clear overlay. */
   MdispSelect(MilDisplay, M_NULL);
   MdispLut(MilDisplay, M_DEFAULT);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Free all allocations. */
   M3dmapFree(MilScan);
   M3dmapFree(MilLaser);
   MbufFree(MilDepthMap);
   MbufFree(MilImage);
   }

/* Values used for binarization. */
#define EXPECTED_HEIGHT     3.4   /* Inspected surface should be at this height (in mm)   */
#define DEFECT_THRESHOLD    0.2   /* Max acceptable deviation from expected height (mm)   */
#define SATURATED_DEFECT    1.0   /* Deviation at which defect will appear red (in mm)    */

/* Radius of the smallest particles to keep. */
#define MIN_BLOB_RADIUS              3L

/* Pixel offset for drawing text. */
#define TEXT_H_OFFSET_1            -50
#define TEXT_V_OFFSET_1             -6
#define TEXT_H_OFFSET_2            -30
#define TEXT_V_OFFSET_2              6

/* Find defects in corrected depth map, compute max deviation and draw contours.  */
void PerformBlobAnalysis(MIL_ID MilSystem,
                         MIL_ID MilDisplay,
                         MIL_ID MilOverlayImage,
                         MIL_ID MilDepthMap)
   {
   MIL_ID      MilBinImage,         /* Binary image buffer identifier.           */
               MilBlobContext,      /* Blob context identifier.                   */
               MilBlobResult;       /* Blob result buffer identifier.            */
   MIL_INT     SizeX,               /* Width of depth map.                       */
               SizeY,               /* Height of depth map.                      */
               TotalBlobs = 0,      /* Total number of blobs.                    */
               n,                   /* Counter.                                  */
              *MinPixels;           /* Maximum height of defects.                */
   MIL_DOUBLE  DefectThreshold,     /* A gray level below it is a defect.        */
              *CogX,                /* X coordinate of center of gravity.        */
              *CogY;                /* Y coordinate of center of gravity.        */

   /* Get size of depth map. */
   MbufInquire(MilDepthMap, M_SIZE_X, &SizeX);
   MbufInquire(MilDepthMap, M_SIZE_Y, &SizeY);

   /* Allocate a binary image buffer for fast processing. */
   MbufAlloc2d(MilSystem, SizeX, SizeY,  1 + M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

   /* Binarize image. */
   DefectThreshold = (EXPECTED_HEIGHT-DEFECT_THRESHOLD) * SCALE_FACTOR;
   MimBinarize(MilDepthMap, MilBinImage, M_FIXED+M_LESS_OR_EQUAL, DefectThreshold, M_NULL);

   /* Remove small particles. */
   MimOpen(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, M_BINARY);

   /* Allocate a blob context. */ 
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
  
   /* Enable the Center Of Gravity and Min Pixel features calculation. */ 
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY+M_GRAYSCALE, M_ENABLE);
   MblobControl(MilBlobContext, M_MIN_PIXEL, M_ENABLE);
 
   /* Allocate a blob result buffer. */
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);
 
   /* Calculate selected features for each blob. */ 
   MblobCalculate(MilBlobContext, MilBinImage, MilDepthMap, MilBlobResult);
 
   /* Get the total number of selected blobs. */ 
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &TotalBlobs);
   MosPrintf(MIL_TEXT("Number of defects: %lld\n"), (long long)TotalBlobs);

   /* Read and print the blob characteristics. */ 
   CogX = new MIL_DOUBLE[TotalBlobs];
   CogY = new MIL_DOUBLE[TotalBlobs];
   MinPixels = new MIL_INT[TotalBlobs];
   if(CogX && CogY && MinPixels)
      { 
      /* Get the results. */
      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_X + M_GRAYSCALE, CogX);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_Y + M_GRAYSCALE, CogY);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_MIN_PIXEL + M_TYPE_MIL_INT, MinPixels);

      /* Draw the defects. */
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MblobDraw(M_DEFAULT, MilBlobResult, MilOverlayImage,
                M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);
      MgraColor(M_DEFAULT, M_COLOR_WHITE);

      /* Print the depth of each blob. */
      for(n=0; n < TotalBlobs; n++)
         {
         MIL_DOUBLE    DepthOfDefect;
         MIL_TEXT_CHAR DepthString[16];

         /* Write the depth of the defect in the overlay. */
         DepthOfDefect = EXPECTED_HEIGHT - (MinPixels[n]/SCALE_FACTOR);
         MosSprintf(DepthString, 16, MIL_TEXT("%.2f mm"), DepthOfDefect);

         MosPrintf(MIL_TEXT("Defect #%lld: depth =%5.2f mm\n\n"),
                   (long long)n, DepthOfDefect);
         MgraText(M_DEFAULT, MilOverlayImage, CogX[n]+TEXT_H_OFFSET_1,
                                        CogY[n]+TEXT_V_OFFSET_1, MIL_TEXT("Defect depth"));
         MgraText(M_DEFAULT, MilOverlayImage, CogX[n]+TEXT_H_OFFSET_2,
                                                     CogY[n]+TEXT_V_OFFSET_2, DepthString);
         }

      delete[] CogX;       CogX = NULL;
      delete[] CogY;       CogY = NULL;
      delete[] MinPixels;  MinPixels = NULL;
      }
   else
      MosPrintf(MIL_TEXT("Error: Not enough memory.\n\n"));

   /* Free all allocations. */
   MblobFree(MilBlobResult);
   MblobFree(MilBlobContext);
   MbufFree(MilBinImage);
   }

/* Color constants for display LUT. */
#define BLUE_HUE  171.0          /* Expected depths will be blue.   */
#define RED_HUE   0.0            /* Worst defects will be red.      */
#define FULL_SATURATION 255      /* All colors are fully saturated. */
#define HALF_LUMINANCE  128      /* All colors have half luminance. */

/* Creates a color display LUT to show defects in red. */
void SetupColorDisplay(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_INT SizeBit)
   {
   MIL_ID  MilRampLut1Band,      /* LUT containing hue values.             */
           MilRampLut3Band,      /* RGB LUT used by display.               */
           MilColorImage;        /* Image used for HSL to RGB conversion.  */
   MIL_INT DefectGrayLevel,      /* Gray level under which all is red.     */
           ExpectedGrayLevel,    /* Gray level over which all is blue.     */
           NbGrayLevels;

   /* Number of possible gray levels in corrected depth map. */
   NbGrayLevels = (MIL_INT)((MIL_INT)1 << SizeBit);

   /* Allocate 1-band LUT that will contain hue values. */
   MbufAlloc1d(MilSystem, NbGrayLevels, 8 + M_UNSIGNED, M_LUT, &MilRampLut1Band);

   /* Compute limit gray values. */
   DefectGrayLevel   = (MIL_INT)((EXPECTED_HEIGHT-SATURATED_DEFECT)*SCALE_FACTOR);
   ExpectedGrayLevel = (MIL_INT)(EXPECTED_HEIGHT*SCALE_FACTOR);

   /* Create hue values for each possible gray level. */
   MgenLutRamp(MilRampLut1Band, 0, RED_HUE, DefectGrayLevel, RED_HUE);
   MgenLutRamp(MilRampLut1Band, DefectGrayLevel, RED_HUE, ExpectedGrayLevel, BLUE_HUE);
   MgenLutRamp(MilRampLut1Band, ExpectedGrayLevel, BLUE_HUE, NbGrayLevels-1, BLUE_HUE);

   /* Create a HSL image buffer. */
   MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + M_UNSIGNED, M_IMAGE, &MilColorImage);
   MbufClear(MilColorImage, M_RGB888(0, FULL_SATURATION, HALF_LUMINANCE));

   /* Set its H band (hue) to the LUT contents and convert the image to RGB. */
   MbufCopyColor2d(MilRampLut1Band, MilColorImage, 0, 0, 0, 0, 0, 0, NbGrayLevels, 1);
   MimConvert(MilColorImage, MilColorImage, M_HSL_TO_RGB);

   /* Create RGB LUT to give to display and copy image contents. */
   MbufAllocColor(MilSystem, 3, NbGrayLevels, 1, 8 + M_UNSIGNED, M_LUT, &MilRampLut3Band);
   MbufCopy(MilColorImage, MilRampLut3Band);

   /* Associates LUT to display. */
   MdispLut(MilDisplay, MilRampLut3Band);

   /* Free all allocations. */
   MbufFree(MilRampLut1Band);
   MbufFree(MilRampLut3Band);
   MbufFree(MilColorImage);
   }

/****************************************************************************
 Calibrated camera example.
****************************************************************************/

/* Input sequence specifications. */
#define GRID_FILENAME                M_IMAGE_PATH MIL_TEXT("GridForLaser.mim")
#define LASERLINE_FILENAME           M_IMAGE_PATH MIL_TEXT("LaserLine.mim")
#define OBJECT2_SEQUENCE_FILE        M_IMAGE_PATH MIL_TEXT("Cookie.avi")

/* Camera calibration grid parameters. */
#define GRID_NB_ROWS             13
#define GRID_NB_COLS             12
#define GRID_ROW_SPACING         5.0     /* in mm                */
#define GRID_COL_SPACING         5.0     /* in mm                */

/* Laser device setup parameters. */
#define CONVEYOR_SPEED           -0.2     /* in mm/frame          */

/* Fully corrected depth map generation parameters. */
#define DEPTH_MAP_SIZE_X         480      /* in pixels            */
#define DEPTH_MAP_SIZE_Y         480      /* in pixels            */
#define GAP_DEPTH                1.5      /* in mm                */

/* Peak detection parameters. */
#define PEAK_WIDTH_NOMINAL_2        9
#define PEAK_WIDTH_DELTA_2          7
#define MIN_CONTRAST_2             75

/* Everything below this is considered as noise. */
#define MIN_HEIGHT_THRESHOLD 1.0 /* in mm */

void CalibratedCameraExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilOverlayImage, /* Overlay image buffer identifier.          */
               MilImage,        /* Image buffer identifier (for processing). */
               MilCalibration,  /* Calibration context.                      */
               MilDepthMap,     /* Image buffer identifier (for results).    */
               MilLaser,        /* 3dmap laser profiling context identifier. */
               MilCalibScan,    /* 3dmap result buffer identifier for laser  */
                                /* line calibration.                         */
               MilScan,         /* 3map result buffer identifier.            */
               MilContainerId,  /* Point cloud container identifier.         */
               FillGapsContext; /* Fill gaps context identifier.             */
   MIL_INT     CalibrationStatus, /* Used to ensure if McalGrid() worked.    */
               SizeX,           /* Width of grabbed images.                  */
               SizeY,           /* Height of grabbed images.                 */
               NumberOfImages,  /* Number of frames for scanned objects.     */
               n;               /* Counter.                                  */
   MIL_DOUBLE  FrameRate,       /* Number of grabbed frames per second (in AVI). */
               StartTime,       /* Time at the beginning of each iteration.  */
               EndTime,         /* Time after processing for each iteration. */
               WaitTime,        /* Time to wait for next frame.              */
               Volume;          /* Volume of scanned object.                 */
              

   MosPrintf(MIL_TEXT("\n3D PROFILING AND VOLUME ANALYSIS:\n"));
   MosPrintf(MIL_TEXT("---------------------------------\n\n"));
   MosPrintf(MIL_TEXT("This program generates fully corrected 3D data of a\n"));
   MosPrintf(MIL_TEXT("scanned cookie and computes its volume.\n"));
   MosPrintf(MIL_TEXT("The laser (sheet-of-light) profiling system uses a\n"));
   MosPrintf(MIL_TEXT("3d-calibrated camera.\n\n"));

   /* Load grid image for camera calibration. */
   MbufRestore(GRID_FILENAME, MilSystem, &MilImage);

   /* Select display. */
   MdispSelect(MilDisplay, MilImage);

   MosPrintf(MIL_TEXT("Calibrating the camera...\n\n"));

   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   /* Allocate calibration context in 3D mode. */
   McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, &MilCalibration);

   /* Calibrate the camera. */
   McalGrid(MilCalibration, MilImage, 0.0, 0.0, 0.0, GRID_NB_ROWS, GRID_NB_COLS,
            GRID_ROW_SPACING, GRID_COL_SPACING, M_DEFAULT, M_CHESSBOARD_GRID);

   McalInquire(MilCalibration, M_CALIBRATION_STATUS+M_TYPE_MIL_INT, &CalibrationStatus);
   if (CalibrationStatus != M_CALIBRATED)
      {
      McalFree(MilCalibration);
      MbufFree(MilImage);
      MosPrintf(MIL_TEXT("Camera calibration failed.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      return;
      }

   /* Prepare for overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MgraColor(M_DEFAULT, M_COLOR_GREEN);

   /* Draw camera calibration points. */
   McalDraw(M_DEFAULT, MilCalibration, MilOverlayImage, M_DRAW_IMAGE_POINTS,
                                                                     M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The camera was calibrated using a chessboard grid.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Disable overlay. */
   MdispControl(MilDisplay, M_OVERLAY, M_DISABLE);

   /* Load laser line image. */
   MbufLoad(LASERLINE_FILENAME, MilImage);

   /* Allocate 3dmap objects. */
   M3dmapAlloc(MilSystem, M_LASER, M_CALIBRATED_CAMERA_LINEAR_MOTION, &MilLaser);
   M3dmapAllocResult(MilSystem, M_LASER_CALIBRATION_DATA, M_DEFAULT, &MilCalibScan);

   /* Set laser line extraction options. */
   MIL_ID MilPeakLocator;
   M3dmapInquire(MilLaser, M_DEFAULT, M_LOCATE_PEAK_1D_CONTEXT_ID+M_TYPE_MIL_ID, &MilPeakLocator);
   MimControl(MilPeakLocator, M_PEAK_WIDTH_NOMINAL, PEAK_WIDTH_NOMINAL_2);
   MimControl(MilPeakLocator, M_PEAK_WIDTH_DELTA  , PEAK_WIDTH_DELTA_2  );
   MimControl(MilPeakLocator, M_MINIMUM_CONTRAST  , MIN_CONTRAST_2      );

   /* Calibrate laser profiling context. */
   M3dmapAddScan(MilLaser, MilCalibScan, MilImage, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);
   M3dmapCalibrate(MilLaser, MilCalibScan, MilCalibration, M_DEFAULT);

   MosPrintf(MIL_TEXT("The laser profiling system has been calibrated using the image\n"));
   MosPrintf(MIL_TEXT("of one laser line.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free the result buffer use for calibration as it will not be used anymore. */
   M3dmapFree(MilCalibScan);
   MilCalibScan = M_NULL;

   /* Allocate the result buffer to hold the scanned 3D points. */
   M3dmapAllocResult(MilSystem, M_POINT_CLOUD_RESULT, M_DEFAULT, &MilScan);

   /* Set speed of scanned object (speed in mm/frame is constant). */
   M3dmapControl(MilLaser, M_DEFAULT, M_SCAN_SPEED, CONVEYOR_SPEED);

   /* Inquire characteristics of the input sequence. */
   MbufDiskInquire(OBJECT2_SEQUENCE_FILE, M_NUMBER_OF_IMAGES, &NumberOfImages);
   MbufDiskInquire(OBJECT2_SEQUENCE_FILE, M_FRAME_RATE, &FrameRate);

   /* Open the object sequence file for reading. */
   MbufImportSequence(OBJECT2_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL,
                                                                           M_NULL, M_OPEN);

   MosPrintf(MIL_TEXT("The cookie is being scanned to generate 3D data.\n\n"));

   /* Read and process all images in the input sequence. */
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);

   for (n = 0; n < NumberOfImages; n++)
      {
      /* Read image from sequence. */
      MbufImportSequence(OBJECT2_SEQUENCE_FILE, M_DEFAULT, M_LOAD, M_NULL, &MilImage,
                                                                     M_DEFAULT, 1, M_READ);

      /* Analyze the image to extract laser line and correct its depth. */
      M3dmapAddScan(MilLaser, MilScan, MilImage, M_NULL, M_NULL, M_POINT_CLOUD_LABEL(1), M_DEFAULT);

      /* Wait to have a proper frame rate, if necessary. */
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &EndTime);
      WaitTime = (1.0/FrameRate) - (EndTime - StartTime);
      if (WaitTime > 0)
         MappTimer(M_DEFAULT, M_TIMER_WAIT, &WaitTime);
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &StartTime);
      }

   /* Close the object sequence file. */
   MbufImportSequence(OBJECT2_SEQUENCE_FILE, M_DEFAULT, M_NULL, M_NULL, NULL, M_NULL,
                                                                          M_NULL, M_CLOSE);

   /* Convert to M_CONTAINER for 3D processing. */
   MbufAllocContainer(MilSystem, M_PROC | M_DISP, M_DEFAULT, &MilContainerId);
   M3dmapCopyResult(MilScan, M_ALL, MilContainerId, M_POINT_CLOUD_UNORGANIZED, M_DEFAULT);

   /* The container's reflectance is 16bits, but only uses the bottom 8. Set the maximum value to display it properly. */
   MbufControlContainer(MilContainerId, M_COMPONENT_REFLECTANCE, M_MAX, 255);

   /* Allocate image for the fully corrected depth map. */
   MbufAlloc2d(MilSystem, DEPTH_MAP_SIZE_X, DEPTH_MAP_SIZE_Y, 16 + M_UNSIGNED,
               M_IMAGE + M_PROC + M_DISP, &MilDepthMap);

   /* Include all points during depth map generation. */
   M3dimCalibrateDepthMap(MilContainerId, MilDepthMap, M_NULL, M_NULL, M_DEFAULT, M_NEGATIVE, M_DEFAULT);

   /* Remove noise in the container close to the Z = 0. */
   MIL_ID MilPlane = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_NULL);
   M3dgeoPlane(MilPlane, M_COEFFICIENTS, 0.0, 0.0, 1.0,MIN_HEIGHT_THRESHOLD, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* M_INVERSE remove what is above the plane. */
   M3dimCrop(MilContainerId, MilContainerId, MilPlane, M_NULL, M_SAME, M_INVERSE);
   M3dgeoFree(MilPlane);

   MosPrintf(MIL_TEXT("Fully corrected 3D data of the cookie is displayed.\n\n"));

   MIL_ID M3dDisplay = Alloc3dDisplayId(MilSystem);
   if(M3dDisplay)
      {
      MosPrintf(MIL_TEXT("Press <R> on the display window to stop/start the rotation.\n\n"));
      M3ddispSelect(M3dDisplay, MilContainerId, M_SELECT, M_DEFAULT);
      M3ddispSetView(M3dDisplay, M_AUTO, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispControl(M3dDisplay, M_AUTO_ROTATE, M_ENABLE);
      }

   /* Get fully corrected depth map from accumulated information in the result buffer. */
   M3dimProject(MilContainerId, MilDepthMap, M_NULL, M_DEFAULT, M_MIN_Z, M_DEFAULT, M_DEFAULT);

   /* Set fill gaps parameters. */
   M3dimAlloc(MilSystem, M_FILL_GAPS_CONTEXT, M_DEFAULT, &FillGapsContext);
   M3dimControl(FillGapsContext, M_FILL_MODE,                  M_X_THEN_Y);
   M3dimControl(FillGapsContext, M_FILL_SHARP_ELEVATION,       M_MIN);
   M3dimControl(FillGapsContext, M_FILL_SHARP_ELEVATION_DEPTH, GAP_DEPTH);
   M3dimControl(FillGapsContext, M_FILL_BORDER,                M_DISABLE);

   M3dimFillGaps(FillGapsContext, MilDepthMap, M_NULL, M_DEFAULT);

   /* Compute the volume of the depth map. */
   M3dmetVolume(MilDepthMap, M_XY_PLANE, M_TOTAL,  M_DEFAULT, &Volume, M_NULL);

   MosPrintf(MIL_TEXT("Volume of the cookie is %4.1f cm^3.\n\n"), Volume / 1000.0);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free all allocations. */
   if (M3dDisplay)
      { M3ddispFree(M3dDisplay); }
   M3dimFree(FillGapsContext);
   MbufFree(MilContainerId);
   M3dmapFree(MilScan);
   M3dmapFree(MilLaser);
   McalFree(MilCalibration);
   MbufFree(MilDepthMap);
   MbufFree(MilImage);
   }
//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
 MIL_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
    MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
    MIL_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
    MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

     if(!MilDisplay3D)
         {
         MosPrintf(MIL_TEXT("\n")
                   MIL_TEXT("The current system does not support the 3D display.\n\n"));
         }
      return MilDisplay3D;
     }
