/*******************************************************************************/
/*
* File name: MCal.cpp
*
* Synopsis:  This program uses the Calibration module to:
*              - Remove distortion and then take measurements in world units using a 2D 
*                calibration.
*              - Perform a 3D calibration to take measurements at several known elevations.
*              - Calibrate a scene using a partial calibration grid that has a 2D code 
*                fiducial.
*
* Printable calibration grids in PDF format can be found in your
* "Matrox Imaging/Images/" directory.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>

/* Example selection. */
#define RUN_LINEAR_CALIBRATION_EXAMPLE       M_YES
#define RUN_TSAI_CALIBRATION_EXAMPLE         M_YES
#define RUN_PARTIAL_GRID_CALIBRATION_EXAMPLE M_YES

/* Grid offset specifications. */
#define GRID_OFFSET_X          0
#define GRID_OFFSET_Y          0
#define GRID_OFFSET_Z          0

/* Example functions declarations. */  
void LinearInterpolationCalibration(MIL_ID MilSystem, MIL_ID MilDisplay);
void TsaiCalibration(MIL_ID MilSystem, MIL_ID MilDisplay);
void PartialGridCalibration(MIL_ID MilSystem, MIL_ID MilDisplay);

/*  Main function. */
int MosMain(void)
   {
   MIL_ID MilApplication, /* Application identifier. */
          MilSystem,      /* System Identifier.      */
          MilDisplay;     /* Display identifier.     */

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Print module name. */
   MosPrintf(MIL_TEXT("CALIBRATION MODULE:\n"));
   MosPrintf(MIL_TEXT("-------------------\n\n"));

#if (RUN_LINEAR_CALIBRATION_EXAMPLE)
   LinearInterpolationCalibration(MilSystem, MilDisplay);
#endif

#if (RUN_TSAI_CALIBRATION_EXAMPLE)
   TsaiCalibration(MilSystem, MilDisplay);
#endif

#if (RUN_PARTIAL_GRID_CALIBRATION_EXAMPLE)
   PartialGridCalibration(MilSystem, MilDisplay);
#endif

   /* Free defaults. */    
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

/*****************************************************************************
Linear interpolation example. 
*****************************************************************************/
/* Source image files specification. */
#define GRID_IMAGE_FILE        M_IMAGE_PATH MIL_TEXT("CalGrid.mim")
#define BOARD_IMAGE_FILE       M_IMAGE_PATH MIL_TEXT("CalBoard.mim")

/* World description of the calibration grid. */
#define GRID_ROW_SPACING       1
#define GRID_COLUMN_SPACING    1
#define GRID_ROW_NUMBER       18
#define GRID_COLUMN_NUMBER    25

/* Measurement boxes specification. */
#define MEAS_BOX_POS_X1       55
#define MEAS_BOX_POS_Y1       24
#define MEAS_BOX_WIDTH1        7
#define MEAS_BOX_HEIGHT1     425

#define MEAS_BOX_POS_X2      225
#define MEAS_BOX_POS_Y2       11
#define MEAS_BOX_WIDTH2        7
#define MEAS_BOX_HEIGHT2     450

/* Specification of the stripes' constraints. */
#define WIDTH_APPROXIMATION  410
#define WIDTH_VARIATION       25
#define MIN_EDGE_VALUE     5

void LinearInterpolationCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID  MilImage,          /* Image buffer identifier.       */
           MilOverlayImage,   /* Overlay image.                 */
           MilCalibration,    /* Calibration identifier.        */
           MeasMarker1,       /* Measurement marker identifier. */
           MeasMarker2;       /* Measurement marker identifier. */
   MIL_DOUBLE  WorldDistance1,  WorldDistance2;
   MIL_DOUBLE  PixelDistance1,  PixelDistance2;
   MIL_DOUBLE  PosX1, PosY1, PosX2, PosY2, PosX3, PosY3, PosX4, PosY4;
   MIL_INT     CalibrationStatus;

   /* Clear the display */
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Restore source image into an automatically allocated image buffer. */
   MbufRestore(GRID_IMAGE_FILE, MilSystem, &MilImage);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Prepare for overlay annotation. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("\nLINEAR INTERPOLATION CALIBRATION:\n"));
   MosPrintf(MIL_TEXT("---------------------------------\n\n"));
   MosPrintf(MIL_TEXT("The displayed grid has been grabbed with a high distortion\n"));
   MosPrintf(MIL_TEXT("camera and will be used to calibrate the camera.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate a camera calibration context. */
   McalAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilCalibration);

   /* Calibrate the camera with the image of the grid and its world description. */
   McalGrid(MilCalibration, MilImage,
            GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
            GRID_ROW_NUMBER, GRID_COLUMN_NUMBER,
            GRID_ROW_SPACING, GRID_COLUMN_SPACING,
            M_DEFAULT, M_DEFAULT);

   McalInquire(MilCalibration, M_CALIBRATION_STATUS + M_TYPE_MIL_INT, &CalibrationStatus);
   if( CalibrationStatus == M_CALIBRATED )
      {
      /* Perform a first image transformation with the calibration grid. */
      McalTransformImage(MilImage, MilImage, MilCalibration,
                         M_BILINEAR + M_OVERSCAN_CLEAR, M_DEFAULT, M_DEFAULT);

      /* Pause to show the corrected image of the grid. */
      MosPrintf(MIL_TEXT("The camera has been calibrated and the image of the grid\n"));
      MosPrintf(MIL_TEXT("has been transformed to remove its distortions.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Read the image of the board and associate the calibration to the image. */
      MbufLoad(BOARD_IMAGE_FILE, MilImage);
      McalAssociate(MilCalibration, MilImage, M_DEFAULT);

      /* Allocate the measurement markers. */
      MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, &MeasMarker1);
      MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, &MeasMarker2);

      /* Set the markers' measurement search region. */
      MmeasSetMarker(MeasMarker1, M_BOX_ORIGIN, MEAS_BOX_POS_X1, MEAS_BOX_POS_Y1);
      MmeasSetMarker(MeasMarker1, M_BOX_SIZE, MEAS_BOX_WIDTH1, MEAS_BOX_HEIGHT1);
      MmeasSetMarker(MeasMarker2, M_BOX_ORIGIN, MEAS_BOX_POS_X2, MEAS_BOX_POS_Y2);
      MmeasSetMarker(MeasMarker2, M_BOX_SIZE, MEAS_BOX_WIDTH2, MEAS_BOX_HEIGHT2);

      /* Set markers' orientation. */
      MmeasSetMarker(MeasMarker1, M_ORIENTATION, M_HORIZONTAL, M_NULL);
      MmeasSetMarker(MeasMarker2, M_ORIENTATION, M_HORIZONTAL, M_NULL);

      /* Set markers' settings to locate the largest stripe within the range
         [WIDTH_APPROXIMATION - WIDTH_VARIATION,
          WIDTH_APPROXIMATION + WIDTH_VARIATION],
         and with an edge strength over MIN_EDGE_VALUE. */
      MmeasSetMarker(MeasMarker1, M_EDGEVALUE_MIN, MIN_EDGE_VALUE, M_NULL);

      /* Remove the default strength characteristic score mapping. */
      MmeasSetScore(MeasMarker1, M_STRENGTH_SCORE,
                                 0.0,
                                 0.0,
                                 M_MAX_POSSIBLE_VALUE,
                                 M_MAX_POSSIBLE_VALUE,
                                 M_DEFAULT,
                                 M_DEFAULT,
                                 M_DEFAULT);

      /* Add a width characteristic score mapping (increasing ramp)
         to find the largest stripe within a given range.

         Width score mapping to find the largest stripe within a given
         width range ]Wmin, Wmax]:

            Score
               ^
               |         /|
               |       /  |
               |     /    |
               +---------------> Width
                   Wmin  Wmax
      */
      MmeasSetScore(MeasMarker1, M_STRIPE_WIDTH_SCORE,
                                 WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 M_DEFAULT,
                                 M_PIXEL,
                                 M_DEFAULT);

      /* Set the same settings for the second marker. */
      MmeasSetMarker(MeasMarker2, M_EDGEVALUE_MIN, MIN_EDGE_VALUE, M_NULL);

      MmeasSetScore(MeasMarker2, M_STRENGTH_SCORE,
                                 0.0,
                                 0.0,
                                 M_MAX_POSSIBLE_VALUE,
                                 M_MAX_POSSIBLE_VALUE,
                                 M_DEFAULT,
                                 M_DEFAULT,
                                 M_DEFAULT);

      MmeasSetScore(MeasMarker2, M_STRIPE_WIDTH_SCORE,
                                 WIDTH_APPROXIMATION - WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 WIDTH_APPROXIMATION + WIDTH_VARIATION,
                                 M_DEFAULT,
                                 M_PIXEL,
                                 M_DEFAULT);

      /* Find and measure the position and width of the board. */
      MmeasFindMarker(M_DEFAULT, MilImage, MeasMarker1, M_STRIPE_WIDTH+M_POSITION);
      MmeasFindMarker(M_DEFAULT, MilImage, MeasMarker2, M_STRIPE_WIDTH+M_POSITION);

      /* Get the world width of the two markers. */
      MmeasGetResult(MeasMarker1, M_STRIPE_WIDTH, &WorldDistance1, M_NULL);
      MmeasGetResult(MeasMarker2, M_STRIPE_WIDTH, &WorldDistance2, M_NULL);

      /* Get the pixel width of the two markers. */
      MmeasSetMarker(MeasMarker1, M_RESULT_OUTPUT_UNITS, M_PIXEL, M_NULL);
      MmeasSetMarker(MeasMarker2, M_RESULT_OUTPUT_UNITS, M_PIXEL, M_NULL);
      MmeasGetResult(MeasMarker1, M_STRIPE_WIDTH, &PixelDistance1, M_NULL);
      MmeasGetResult(MeasMarker2, M_STRIPE_WIDTH, &PixelDistance2, M_NULL);

      /* Get the edges position in pixel to draw the annotations. */
      MmeasGetResult(MeasMarker1, M_POSITION+M_EDGE_FIRST,  &PosX1, &PosY1);
      MmeasGetResult(MeasMarker1, M_POSITION+M_EDGE_SECOND, &PosX2, &PosY2);
      MmeasGetResult(MeasMarker2, M_POSITION+M_EDGE_FIRST,  &PosX3, &PosY3);
      MmeasGetResult(MeasMarker2, M_POSITION+M_EDGE_SECOND, &PosX4, &PosY4);

      /* Draw the measurement indicators on the image.  */
      MgraColor(M_DEFAULT, M_COLOR_YELLOW);
      MmeasDraw(M_DEFAULT, MeasMarker1, MilOverlayImage, M_DRAW_WIDTH, M_DEFAULT, M_RESULT);
      MmeasDraw(M_DEFAULT, MeasMarker2, MilOverlayImage, M_DRAW_WIDTH, M_DEFAULT, M_RESULT);

      MgraBackColor(M_DEFAULT, M_COLOR_BLACK);
      MgraText(M_DEFAULT, MilOverlayImage, (MIL_INT)(PosX1+0.5-40),
         (MIL_INT)((PosY1+0.5)+((PosY2 - PosY1)/2.0)), MIL_TEXT(" Distance 1 "));
      MgraText(M_DEFAULT, MilOverlayImage, (MIL_INT)(PosX3+0.5-40),
         (MIL_INT)((PosY3+0.5)+((PosY4 - PosY3)/2.0)), MIL_TEXT(" Distance 2 "));

      /* Pause to show the original image and the measurement results. */
      MosPrintf(MIL_TEXT("A distorted image grabbed with the same camera was loaded and\n"));
      MosPrintf(MIL_TEXT("calibrated measurements were done to evaluate ")
                                       MIL_TEXT("the board dimensions.\n"));
      MosPrintf(MIL_TEXT("\n========================================================\n"));
      MosPrintf(MIL_TEXT("                      Distance 1          Distance 2 \n"));
      MosPrintf(MIL_TEXT("--------------------------------------------------------\n"));
      MosPrintf(MIL_TEXT(" Calibrated unit:   %8.2lf cm           %6.2lf cm    \n"),
                                                         WorldDistance1, WorldDistance2);
      MosPrintf(MIL_TEXT(" Uncalibrated unit: %8.2lf pixels       %6.2lf pixels\n"),
                                                         PixelDistance1, PixelDistance2);
      MosPrintf(MIL_TEXT("========================================================\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Clear the display overlay. */
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      /* Read the image of the PCB. */
      MbufLoad(BOARD_IMAGE_FILE, MilImage);

      /* Transform the image of the board. */
      McalTransformImage(MilImage, MilImage, MilCalibration,
         M_BILINEAR+M_OVERSCAN_CLEAR, M_DEFAULT, M_DEFAULT);

      /* show the transformed image of the board. */
      MosPrintf(MIL_TEXT("The image was corrected to remove its distortions.\n"));

      /* Free measurement markers. */
      MmeasFree(MeasMarker1);
      MmeasFree(MeasMarker2);
      }
   else
      {
      MosPrintf(MIL_TEXT("Calibration generated an exception.\n"));
      MosPrintf(MIL_TEXT("See User Guide to resolve the situation.\n\n"));
      }

   /* Wait for a key to be pressed. */ 
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free all allocations. */
   McalFree(MilCalibration);
   MbufFree(MilImage);
   }


/*****************************************************************************
Tsai example. 
*****************************************************************************/
/* Source image files specification. */
#define GRID_ORIGINAL_IMAGE_FILE     M_IMAGE_PATH MIL_TEXT("CalGridOriginal.mim")
#define OBJECT_ORIGINAL_IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("CalObjOriginal.mim")
#define OBJECT_MOVED_IMAGE_FILE      M_IMAGE_PATH MIL_TEXT("CalObjMoved.mim")

/* World description of the calibration grid. */
#define GRID_ORG_ROW_SPACING       1.5
#define GRID_ORG_COLUMN_SPACING    1.5
#define GRID_ORG_ROW_NUMBER        12
#define GRID_ORG_COLUMN_NUMBER     13
#define GRID_ORG_OFFSET_X          0
#define GRID_ORG_OFFSET_Y          0 
#define GRID_ORG_OFFSET_Z          0

/* Region parameters for metrology */
#define MEASURED_CIRCLE_LABEL       1
#define RING1_POS_X                2.3
#define RING1_POS_Y                3.9
#define RING2_POS_X                10.7
#define RING2_POS_Y                11.1

#define RING_START_RADIUS          1.25
#define RING_END_RADIUS            2.3

/* measured plane position */
#define RING_THICKNESS             0.175
#define STEP_THICKNESS             4.0

/* Color definitions */
#define ABSOLUTE_COLOR   M_RGB888(255,0,0)
#define RELATIVE_COLOR   M_RGB888(0,255,0)
#define REGION_COLOR     M_RGB888(0,100,255)
#define FEATURE_COLOR    M_RGB888(255,0,255)

/* Functions declarations for this example. */
void MeasureRing(MIL_ID MilSystem, MIL_ID MilOverlayImage, MIL_ID MilImage, 
                 MIL_DOUBLE MeasureRing1X, MIL_DOUBLE MeasureRing1Y );
void ShowCameraInformation(MIL_ID MilCalibration);

void TsaiCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID  MilImage,          /* Image buffer identifier.         */
           MilOverlayImage,   /* Overlay image buffer identifier. */
           MilCalibration;    /* Calibration identifier.          */

   MIL_INT CalibrationStatus;          

   /* Restore source image into an automatically allocated image buffer. */
   MbufRestore(GRID_ORIGINAL_IMAGE_FILE, MilSystem, &MilImage);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);
   /* Prepare for overlay annotation. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("\nTSAI BASED CALIBRATION:\n"));
   MosPrintf(MIL_TEXT("-----------------------\n\n"));
   MosPrintf(MIL_TEXT("The displayed grid has been grabbed with a high perspective\n"));
   MosPrintf(MIL_TEXT("camera and will be used to calibrate the camera.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate a camera calibration context. */
   McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, &MilCalibration);

   /* Calibrate the camera with the image of the grid and its world description. */
   McalGrid(MilCalibration, MilImage,
            GRID_ORG_OFFSET_X, GRID_ORG_OFFSET_Y, GRID_ORG_OFFSET_Z,
            GRID_ORG_ROW_NUMBER, GRID_ORG_COLUMN_NUMBER,
            GRID_ORG_ROW_SPACING, GRID_ORG_COLUMN_SPACING,
            M_DEFAULT, M_DEFAULT);

   /* Verify if the camera calibration is successful. */
   McalInquire(MilCalibration, M_CALIBRATION_STATUS + M_TYPE_MIL_INT, &CalibrationStatus);
   if( CalibrationStatus == M_CALIBRATED )
      {
      /* Display the world absolute coordinate system */
      MgraColor(M_DEFAULT, ABSOLUTE_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilOverlayImage, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM+M_DRAW_AXES, M_DEFAULT, M_DEFAULT);

      /* Print camera information */
      MosPrintf(MIL_TEXT("The camera has been calibrated.\n"));
      MosPrintf(MIL_TEXT("The world absolute coordinate system is shown in red.\n\n"));
      ShowCameraInformation(MilCalibration);

      /* Load source image into an image buffer. */
      MbufLoad(OBJECT_ORIGINAL_IMAGE_FILE, MilImage);

      /* Associate the calibration to the image */
      McalAssociate(MilCalibration, MilImage, M_DEFAULT);
      
      /* Set the offset of the camera calibration plane. */
      /* This moves the relative origin at the top of the first metallic part */
      McalSetCoordinateSystem(MilImage,
         M_RELATIVE_COORDINATE_SYSTEM,
         M_ABSOLUTE_COORDINATE_SYSTEM,
         M_TRANSLATION + M_ASSIGN,
         M_NULL,
         0, 0, -RING_THICKNESS,
         M_DEFAULT);

      /* Display the world relative coordinate system */
      MgraColor(M_DEFAULT, RELATIVE_COLOR);
      McalDraw(M_DEFAULT, MilImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM + M_DRAW_FRAME, M_DEFAULT, M_DEFAULT);

      /* Measure the first circle. */
      MosPrintf(MIL_TEXT("The relative coordinate system (shown in green) was translated by %.3f cm\n"), -RING_THICKNESS);
      MosPrintf(MIL_TEXT("in z to align it with the top of the first metallic part.\n"));
      MeasureRing(MilSystem, MilOverlayImage, MilImage, RING1_POS_X, RING1_POS_Y );
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Modify the offset of the camera calibration plane */
      /* This moves the relative origin at the top of the second metallic part  */
      McalSetCoordinateSystem(MilImage,
         M_RELATIVE_COORDINATE_SYSTEM,
         M_ABSOLUTE_COORDINATE_SYSTEM,
         M_TRANSLATION + M_COMPOSE_WITH_CURRENT,
         M_NULL,
         0, 0, -STEP_THICKNESS,
         M_DEFAULT);

      /* Clear the overlay */
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
      /* Display the world absolute coordinate system */
      MgraColor(M_DEFAULT, ABSOLUTE_COLOR);
      McalDraw(M_DEFAULT, MilCalibration, MilOverlayImage, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM + M_DRAW_AXES, M_DEFAULT, M_DEFAULT);
      /* Display the world relative coordinate system */
      MgraColor(M_DEFAULT, RELATIVE_COLOR);
      McalDraw(M_DEFAULT, MilImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM + M_DRAW_FRAME, M_DEFAULT, M_DEFAULT);

      /* Measure the second circle. */
      MosPrintf(MIL_TEXT("The relative coordinate system was translated by another %.3f cm\n"), -STEP_THICKNESS);
      MosPrintf(MIL_TEXT("in z to align it with the top of the second metallic part.\n"));
      MeasureRing(MilSystem, MilOverlayImage, MilImage, RING2_POS_X, RING2_POS_Y );
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }
   else
      {
      MosPrintf(MIL_TEXT("Calibration generated an exception.\n"));
      MosPrintf(MIL_TEXT("See User Guide to resolve the situation.\n\n"));
      }

   /* Free all allocations. */
   McalFree(MilCalibration);
   MbufFree(MilImage);
   }

/* Measuring function with MilMetrology module */
void MeasureRing(MIL_ID MilSystem, MIL_ID MilOverlayImage, MIL_ID MilImage, 
                    MIL_DOUBLE MeasureRingX, MIL_DOUBLE MeasureRingY )
   {
   MIL_ID  MilMetrolContext,  /* Metrology Context              */
           MilMetrolResult;   /* Metrology Result               */
           
   MIL_DOUBLE Value;

   /* Allocate metrology context and result. */
   MmetAlloc(MilSystem, M_DEFAULT, &MilMetrolContext);
   MmetAllocResult(MilSystem, M_DEFAULT, &MilMetrolResult);

   /* Add a first measured segment feature to context and set its search region */
   MmetAddFeature(MilMetrolContext, M_MEASURED, M_CIRCLE, MEASURED_CIRCLE_LABEL,
                  M_DEFAULT, M_NULL, M_NULL, 0, M_DEFAULT);

   MmetSetRegion(MilMetrolContext, M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), 
                 M_DEFAULT, M_RING,
                 MeasureRingX, MeasureRingY,
                 RING_START_RADIUS, RING_END_RADIUS,
                 M_NULL, M_NULL );

   /* Calculate */
   MmetCalculate(MilMetrolContext, MilImage, MilMetrolResult, M_DEFAULT);

   /* Draw region */
   MgraColor(M_DEFAULT, REGION_COLOR);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilOverlayImage, M_DRAW_REGION, M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), M_DEFAULT);

   /* Draw the circle */
   MgraColor(M_DEFAULT, FEATURE_COLOR);
   MmetDraw(M_DEFAULT, MilMetrolResult, MilOverlayImage, M_DRAW_FEATURE, M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), M_DEFAULT);

   MmetGetResult(MilMetrolResult, M_FEATURE_LABEL(MEASURED_CIRCLE_LABEL), M_RADIUS, &Value);
   MosPrintf(MIL_TEXT("The large circle's radius was measured: %.3f cm.\n"), Value);

   /* Free all allocations. */
   MmetFree(MilMetrolResult);
   MmetFree(MilMetrolContext);
   }

/* Print the current camera position and orientation  */
void ShowCameraInformation(MIL_ID MilCalibration)
   {
   MIL_DOUBLE CameraPosX,
              CameraPosY,
              CameraPosZ,
              CameraYaw,
              CameraPitch,
              CameraRoll;

   McalGetCoordinateSystem(MilCalibration,
                           M_CAMERA_COORDINATE_SYSTEM,
                           M_ABSOLUTE_COORDINATE_SYSTEM,
                           M_TRANSLATION,
                           M_NULL,
                           &CameraPosX, &CameraPosY, &CameraPosZ, 
                           M_NULL);

   McalGetCoordinateSystem(MilCalibration,
                           M_CAMERA_COORDINATE_SYSTEM,
                           M_ABSOLUTE_COORDINATE_SYSTEM,
                           M_ROTATION_YXZ,
                           M_NULL,
                           &CameraYaw, &CameraPitch, &CameraRoll, 
                           M_NULL);

   /* Pause to show the camera position and orientation. */
   MosPrintf(MIL_TEXT("Camera position in cm:          (x, y, z)           ")
             MIL_TEXT("(%.2f, %.2f, %.2f)\n"), CameraPosX, CameraPosY, CameraPosZ);
   MosPrintf(MIL_TEXT("Camera orientation in degrees:  (yaw, pitch, roll)  ")
             MIL_TEXT("(%.2f, %.2f, %.2f)\n"), CameraYaw, CameraPitch, CameraRoll);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/*****************************************************************************
Partial grid example. 
*****************************************************************************/
/* Source image files specification. */
#define PARTIAL_GRID_IMAGE_FILE        M_IMAGE_PATH MIL_TEXT("PartialGrid.mim")

/* Definition of the region to correct. */
#define CORRECTED_SIZE_X         60.0
#define CORRECTED_SIZE_Y         50.0
#define CORRECTED_OFFSET_X       -35.0
#define CORRECTED_OFFSET_Y       -5.0
#define CORRECTED_IMAGE_SIZE_X   512

/* Functions declarations for this example. */
void DrawGridInfo(MIL_ID MilGraList, MIL_CONST_TEXT_PTR InfoTag, MIL_DOUBLE Value,
                  MIL_INT LineOffsetY, MIL_CONST_TEXT_PTR Units);

void PartialGridCalibration(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID  MilImage,                /* Image buffer identifier.       */
           MilCorrectedImage,       /* Corrected image identifier.    */
           MilGraList,              /* Graphic list identifier.       */
           MilCalibration;          /* Calibration identifier.        */

   MIL_INT       CalibrationStatus, ImageType,CorrectedImageSizeY;
   MIL_DOUBLE    RowSpacing, ColumnSpacing, CorrectedPixelSize;
   MIL_TEXT_CHAR UnitName[M_GRID_UNIT_SHORT_NAME_MAX_SIZE];

   /* Clear the display */
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Allocate a graphic list and associate it to the display. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   /* Restore source image into an automatically allocated image buffer. */
   MbufRestore(PARTIAL_GRID_IMAGE_FILE, MilSystem, &MilImage);
   MbufInquire(MilImage, M_TYPE, &ImageType);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Pause to show the partial grid image. */
   MosPrintf(MIL_TEXT("\nPARTIAL GRID CALIBRATION:\n"));
   MosPrintf(MIL_TEXT("-------------------------\n\n"));
   MosPrintf(MIL_TEXT("A camera will be calibrated using a rectangular grid that\n"));
   MosPrintf(MIL_TEXT("is only partially visible in the camera's field of view.\n"));
   MosPrintf(MIL_TEXT("The 2D code in the center is used as a fiducial to retrieve\n"));
   MosPrintf(MIL_TEXT("the characteristics of the calibration grid.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Allocate the calibration object. */
   McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, &MilCalibration);

   /* Set the calibration to calibrate a partial grid with fiducial. */
   McalControl(MilCalibration, M_GRID_PARTIAL, M_ENABLE);
   McalControl(MilCalibration, M_GRID_FIDUCIAL, M_DATAMATRIX);

   /* Calibrate the camera with the partial grid with fiducial. */
   McalGrid(MilCalibration, MilImage,
            GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
            M_UNKNOWN, M_UNKNOWN, M_FROM_FIDUCIAL, M_FROM_FIDUCIAL,
            M_DEFAULT, M_CHESSBOARD_GRID);

   McalInquire(MilCalibration, M_CALIBRATION_STATUS + M_TYPE_MIL_INT, &CalibrationStatus);
   if(CalibrationStatus == M_CALIBRATED)
      {
      /* Draw the absolute coordinate system. */
      MgraColor(M_DEFAULT, M_COLOR_RED);
      McalDraw(M_DEFAULT, MilCalibration, MilGraList, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM,
         M_DEFAULT, M_DEFAULT);

      /* Draw a box around the fiducial. */
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      McalDraw(M_DEFAULT, MilCalibration, MilGraList, M_DRAW_FIDUCIAL_BOX,
         M_DEFAULT, M_DEFAULT);

      /* Get the information of the grid read from the fiducial. */
      McalInquire(MilCalibration, M_ROW_SPACING, &RowSpacing);
      McalInquire(MilCalibration, M_COLUMN_SPACING, &ColumnSpacing);
      McalInquire(MilCalibration, M_GRID_UNIT_SHORT_NAME, UnitName);

      /* Draw the information of the grid read from the fiducial. */
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MgraControl(M_DEFAULT, M_INPUT_UNITS, M_DISPLAY);
      DrawGridInfo(MilGraList, MIL_TEXT("Row spacing"), RowSpacing, 0, UnitName);
      DrawGridInfo(MilGraList, MIL_TEXT("Col spacing"), ColumnSpacing, 1, UnitName);

      /* Pause to show the calibration result. */
      MosPrintf(MIL_TEXT("The camera has been calibrated.\n\n"));
      MosPrintf(MIL_TEXT("The grid information read is displayed.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Calculate the pixel size and size Y of the corrected image. */
      CorrectedPixelSize = CORRECTED_SIZE_X / CORRECTED_IMAGE_SIZE_X;
      CorrectedImageSizeY = (MIL_INT)(CORRECTED_SIZE_Y / CorrectedPixelSize);

      /* Allocate the corrected image. */
      MbufAlloc2d(MilSystem, CORRECTED_IMAGE_SIZE_X, CorrectedImageSizeY, ImageType,
         M_IMAGE + M_PROC + M_DISP, &MilCorrectedImage);

      /* Calibrate the corrected image. */
      McalUniform(MilCorrectedImage, CORRECTED_OFFSET_X, CORRECTED_OFFSET_Y,
         CorrectedPixelSize, CorrectedPixelSize, 0.0, M_DEFAULT);

      /* Correct the calibrated image. */
      McalTransformImage(MilImage, MilCorrectedImage, MilCalibration,
         M_BILINEAR + M_OVERSCAN_CLEAR, M_DEFAULT,
         M_WARP_IMAGE + M_USE_DESTINATION_CALIBRATION);

      /* Select the corrected image on the display. */
      MgraClear(M_DEFAULT, MilGraList);
      MdispSelect(MilDisplay, MilCorrectedImage);

      /* Pause to show the corrected image. */
      MosPrintf(MIL_TEXT("A sub-region of the grid was selected and transformed\n"));
      MosPrintf(MIL_TEXT("to remove the distortions.\n"));
      MosPrintf(MIL_TEXT("The sub-region dimensions and position are:\n"));
      MosPrintf(MIL_TEXT("   Size X  : % 3.3g %s\n"), CORRECTED_SIZE_X, UnitName);
      MosPrintf(MIL_TEXT("   Size Y  : % 3.3g %s\n"), CORRECTED_SIZE_Y, UnitName);
      MosPrintf(MIL_TEXT("   Offset X: % 3.3g %s\n"), CORRECTED_OFFSET_X, UnitName);
      MosPrintf(MIL_TEXT("   Offset Y: % 3.3g %s\n"), CORRECTED_OFFSET_Y, UnitName);

      /* Wait for a key to be pressed. */
      MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
      MosGetch();

      MbufFree(MilCorrectedImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("Calibration generated an exception.\n"));
      MosPrintf(MIL_TEXT("See User Guide to resolve the situation.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
      MosGetch();
      }

   /* Free all allocations. */
   McalFree(MilCalibration);
   MbufFree(MilImage);
   MgraFree(MilGraList);
   }

/* Definition of the parameters for the drawing of the grid info */
#define LINE_HEIGHT      16
#define MAX_INFO_SIZE    64

/* Draw an information of the grid. */
void DrawGridInfo(MIL_ID MilGraList, MIL_CONST_TEXT_PTR InfoTag,  MIL_DOUBLE Value,
                  MIL_INT LineOffsetY, MIL_CONST_TEXT_PTR Units)
   {
   MIL_TEXT_CHAR Info[MAX_INFO_SIZE];
   MosSprintf(Info, MAX_INFO_SIZE, MIL_TEXT("%s: %.3g %s"), InfoTag, Value, Units);
   MgraText(M_DEFAULT, MilGraList, 0, LineOffsetY * LINE_HEIGHT, Info);
   }
