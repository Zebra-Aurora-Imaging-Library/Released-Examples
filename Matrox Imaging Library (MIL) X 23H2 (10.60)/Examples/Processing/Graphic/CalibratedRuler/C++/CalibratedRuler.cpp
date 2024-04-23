//***************************************************************************************
// 
// File name: CalibratedRuler.cpp  
//
// Synopsis: Generates a ruler calibrated to world units and allows to measure the world
//           distance between two points selected by the user.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>

///*****************************************************************************
// Constants.
//*****************************************************************************
// Source image files specification. 
#define GRID_IMAGE_FILE        M_IMAGE_PATH MIL_TEXT("CalGrid.mim")
#define DISTORTED_IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("CalibratedRuler/Distorted.mim")

// World description of the calibration grid.
#define GRID_OFFSET_X            0
#define GRID_OFFSET_Y            0
#define GRID_OFFSET_Z            0
#define GRID_ROW_SPACING         1
#define GRID_COLUMN_SPACING      1
#define GRID_ROW_NUMBER          18
#define GRID_COLUMN_NUMBER       25

// Ruler Color.
#define RULER_COLOR              M_COLOR_RED

// Measurement defines.
#define MEAS_BOX_WIDTH           15 // In pixel units.
#define MEAS_BOX_HEIGHT          15 // In pixel units.
#define MEAS_COLOR               M_COLOR_GREEN

#define STRING_SIZE              128
#define M_Round(x) ((MIL_INT)((x) + ((x) >= 0 ? 0.5 : -0.5)))

//****************************************************************************
// Data structures to handle parameters for the hook functions.
//****************************************************************************

// Ruler data structures.
typedef struct
   {
   MIL_ID MilDisplay;                     // Display identifier.    
   MIL_ID MilImage;                       // Image buffer identifier
   MIL_ID MilCalibration;                 // Calibration identifier.
   MIL_ID MilGraphics;                    // Graphics identifier.
   MIL_ID MilRulerGraphicList;            // Ruler graphic list identifier.
   MIL_ID MilMeasGraphicList;             // Measurement graphic list identifier.
   MIL_ID MilMeasBoxGraphicList;          // Measurement box graphic list identifier.
   MIL_ID MilDisplayGraphicList;          // Display graphic list identifier.

   MIL_INT ImageSizeX;                    // Size x of MilImage.
   MIL_INT ImageSizeY;                    // Size y of MilImage.

   MIL_DOUBLE PrevDispZoomX;              // Last display x zoom factor.
   MIL_DOUBLE PrevDispZoomY;              // Last display y zoom factor.
   MIL_DOUBLE PrevDispPanOffsetX;         // Last display x pan offset.
   MIL_DOUBLE PrevDispPanOffsetY;         // Last display y pan offset.

   MIL_INT    NumCalibrationPoints;       // Number of calibration points.
   MIL_INT    RowNumber;                  // Number rows in cal grid.
   MIL_INT    ColumnNumber;               // Number of columns in cal grid.
   MIL_DOUBLE ColumnWorldSpacing;         // Spacing between columns in world units.
   MIL_DOUBLE RowWorldSpacing;            // Spacing between rows in world units.

   MIL_DOUBLE* WorldCalibrationPointsX;   // X calibration points in world units.
   MIL_DOUBLE* WorldCalibrationPointsY;   // Y calibration points in world units.

   MIL_DOUBLE* PixelCalibrationPointsX;   // X pixel points in world units.
   MIL_DOUBLE* PixelCalibrationPointsY;   // Y pixel points in world units.

   } DispHookRulerDataStruct;

typedef struct
   {
   MIL_DOUBLE  Measure;
   bool        DrawMajorMeasure;
   bool        DrawMediumMeasure;
   bool        DrawMinorMeasure;
   } RulerDataStruct;

enum RulerType
   {
   eXAxis,
   eYAxis
   };

// Measurement data structures.
typedef struct
   {
   MIL_ID  MilDisplay;              // Display identifier.    
   MIL_ID  MilImage;                // Image buffer identifier
   MIL_ID  MilCalibration;          // Calibration identifier.
   MIL_ID  MilGraphics;             // Graphics identifier.
   MIL_ID  MilRulerGraphicList;     // Ruler graphic list identifier.
   MIL_ID  MilMeasGraphicList;      // Measurement graphic list identifier.
   MIL_ID  MilMeasBoxGraphicList;   // Measurement box graphic list identifier.
   MIL_ID  MilDisplayGraphicList;   // Display graphic list identifier.

   MIL_ID  MilMeasMarker1;          // Measurement marker identifier.
   MIL_ID  MilMeasMarker2;          // Measurement marker identifier.
   MIL_ID  MilMeasCalculateRes;     // Measurement calculate result identifier.
   MIL_INT NumDefinedMarkers;       // Number of markers the user has defined.
   } DispHookMeasureDataStruct;

//****************************************************************************
// Function declarations.
//****************************************************************************

// Ruler functions.
MIL_INT MFTYPE DrawRuler(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr); 

void FillRulerMeasures(const DispHookRulerDataStruct& HookData, MIL_INT RulerArraySize, 
                       RulerType Type, RulerDataStruct* RulerArray);

void FillSubRulerMeasures(MIL_ID MilCalibratedImage, MIL_DOUBLE WorldStartCoord, 
                          MIL_DOUBLE WorldEndCoord, MIL_DOUBLE ZoomX, MIL_DOUBLE ZoomY, 
                          MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY, RulerType Type, 
                          MIL_INT RulerArraySize, RulerDataStruct* RulerArray, 
                          MIL_INT SubRulerSize);

void DisplayToPixel(MIL_DOUBLE* DisplayX, MIL_DOUBLE* DisplayY, MIL_DOUBLE* PixelX, 
                    MIL_DOUBLE* PixelY, MIL_INT NumCoordinates, MIL_DOUBLE ZoomX, 
                    MIL_DOUBLE ZoomY, MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY);

void PixelToDisplay(MIL_DOUBLE* PixelX, MIL_DOUBLE* PixelY, MIL_DOUBLE* DisplayX, 
                    MIL_DOUBLE* DisplayY, MIL_INT NumCoordinates, MIL_DOUBLE ZoomX, 
                    MIL_DOUBLE ZoomY, MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY);

// Measurement functions.
MIL_INT MFTYPE MeasMouseRightClick(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr);
MIL_INT MFTYPE MeasMouseMove(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr);
MIL_INT MFTYPE MeasureDistance(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr, 
   bool RightClick);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("CalibratedRuler\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example allows to interactively measure the distance, in\n")
             MIL_TEXT("world units, between two points in a corrected image.  It also\n")
             MIL_TEXT("displays rulers with world unit measures.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, graphic,\n")
             MIL_TEXT("calibration, measurement.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication,                             // Application identifier.
          MilSystem;                                  // System identifier.

   // Structure to hold data for the ruler 
   // display hook.
   DispHookRulerDataStruct DisplayHookRulerData; 

   // Structure to hold data for the measure 
   // display hook.
   DispHookMeasureDataStruct DisplayHookMeasureData;

   MIL_INT CalibrationStatus;

   // Initialize the pointers.
   DisplayHookRulerData.WorldCalibrationPointsX = M_NULL;
   DisplayHookRulerData.WorldCalibrationPointsY = M_NULL;

   DisplayHookRulerData.PixelCalibrationPointsX = M_NULL; 
   DisplayHookRulerData.PixelCalibrationPointsY = M_NULL;

   // Allocate MIL objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, 
              &DisplayHookRulerData.MilDisplay);
   MdispControl(DisplayHookRulerData.MilDisplay, M_CENTER_DISPLAY, M_DISABLE);

   PrintHeader();

   // Restore source image into an automatically allocated image buffer.
   MbufRestore(GRID_IMAGE_FILE, MilSystem, &DisplayHookRulerData.MilImage);

   // Display the image buffer. 
   MdispSelect(DisplayHookRulerData.MilDisplay, DisplayHookRulerData.MilImage);

   // Allocate a graphic context.
   MgraAlloc(MilSystem, &DisplayHookRulerData.MilGraphics);

   // Allocate a graphic list to associate to the display.
   MgraAllocList(MilSystem, M_DEFAULT, &DisplayHookRulerData.MilDisplayGraphicList);

   // Allocate a graphic list used for the ruler.
   MgraAllocList(MilSystem, M_DEFAULT, &DisplayHookRulerData.MilRulerGraphicList);

   // Allocate a graphic list used for measurement.
   MgraAllocList(MilSystem, M_DEFAULT, &DisplayHookMeasureData.MilMeasGraphicList);

   // Allocate a graphic list used for measurement box.
   MgraAllocList(MilSystem, M_DEFAULT, &DisplayHookMeasureData.MilMeasBoxGraphicList);

   // Set the graphics mode to transparent.
   MgraControl(DisplayHookRulerData.MilGraphics, M_BACKGROUND_MODE, M_TRANSPARENT);

   // Associate the graphic list to the display for annotations.
   MdispControl(DisplayHookRulerData.MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, 
                DisplayHookRulerData.MilDisplayGraphicList);

   // Pause to show the original image. 
   MosPrintf(MIL_TEXT("The displayed grid has been grabbed with high lens distortion\n"));
   MosPrintf(MIL_TEXT("and will be used to calibrate the camera.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a calibration context.
   McalAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &DisplayHookRulerData.MilCalibration);

   // Calibrate the context with the image of the grid and its world description.
   McalGrid(DisplayHookRulerData.MilCalibration, DisplayHookRulerData.MilImage,
      GRID_OFFSET_X, GRID_OFFSET_Y, GRID_OFFSET_Z,
      GRID_ROW_NUMBER, GRID_COLUMN_NUMBER,
      GRID_ROW_SPACING, GRID_COLUMN_SPACING,
      M_DEFAULT, M_DEFAULT);

   // Allocate the measurement markers.
   MmeasAllocMarker(MilSystem, M_EDGE, M_DEFAULT, &DisplayHookMeasureData.MilMeasMarker1);
   MmeasAllocMarker(MilSystem, M_EDGE, M_DEFAULT, &DisplayHookMeasureData.MilMeasMarker2);

   // Allocate the measurement result.
   MmeasAllocResult(MilSystem, M_DEFAULT, &DisplayHookMeasureData.MilMeasCalculateRes);

   // Set the box search angle mode of each marker to check all angles for edges.
   MmeasSetMarker(DisplayHookMeasureData.MilMeasMarker1, M_BOX_ANGLE_MODE, M_ENABLE, 
      M_NULL);

   MmeasSetMarker(DisplayHookMeasureData.MilMeasMarker1, M_BOX_ANGLE, M_ANY, M_NULL);

   MmeasSetMarker(DisplayHookMeasureData.MilMeasMarker2, M_BOX_ANGLE_MODE, M_ENABLE, 
      M_NULL);

   MmeasSetMarker(DisplayHookMeasureData.MilMeasMarker2, M_BOX_ANGLE, M_ANY, M_NULL);

   // Set the required data for the measure function.
   DisplayHookMeasureData.MilDisplay            = 
      DisplayHookRulerData.MilDisplay;

   DisplayHookMeasureData.MilImage              = 
      DisplayHookRulerData.MilImage;

   DisplayHookMeasureData.MilCalibration        = 
      DisplayHookRulerData.MilCalibration;

   DisplayHookMeasureData.MilDisplayGraphicList = 
      DisplayHookRulerData.MilDisplayGraphicList;

   DisplayHookMeasureData.MilRulerGraphicList   = 
      DisplayHookRulerData.MilRulerGraphicList;

   DisplayHookMeasureData.MilGraphics           = 
      DisplayHookRulerData.MilGraphics;

   // Set the required data for the ruler function.
   DisplayHookRulerData.MilMeasGraphicList      = 
      DisplayHookMeasureData.MilMeasGraphicList;

   DisplayHookRulerData.MilMeasBoxGraphicList   = 
      DisplayHookMeasureData.MilMeasBoxGraphicList;

   McalInquire(DisplayHookRulerData.MilCalibration, M_CALIBRATION_STATUS + M_TYPE_MIL_INT, 
      &CalibrationStatus);

   if( CalibrationStatus == M_CALIBRATED )
      {
      // Read the file of the distorted image and associate the calibration to the image. 
      MbufLoad(DISTORTED_IMAGE_FILE, DisplayHookRulerData.MilImage);
      McalAssociate(DisplayHookRulerData.MilCalibration, DisplayHookRulerData.MilImage, 
         M_DEFAULT);

      // Set the buffer sizes.
      DisplayHookRulerData.ImageSizeX = MbufInquire(DisplayHookRulerData.MilImage, 
         M_SIZE_X, M_NULL);

      DisplayHookRulerData.ImageSizeY = MbufInquire(DisplayHookRulerData.MilImage, 
         M_SIZE_Y, M_NULL);
      
      // Pause to show the original image and the measurement results. 
      MosPrintf(MIL_TEXT("A distorted image grabbed with the same camera was loaded.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Transform the new image. 
      McalTransformImage(DisplayHookRulerData.MilImage, DisplayHookRulerData.MilImage, 
         DisplayHookRulerData.MilCalibration, M_BILINEAR+M_OVERSCAN_CLEAR, 
         M_DEFAULT, M_DEFAULT);

      // Initialize the display and zoom information.
      DisplayHookRulerData.PrevDispPanOffsetX = -1;
      DisplayHookRulerData.PrevDispPanOffsetY = -1;
      DisplayHookRulerData.PrevDispZoomX      = -1;
      DisplayHookRulerData.PrevDispZoomY      = -1;

      // Initialize the calibration data.
      McalInquire(DisplayHookRulerData.MilCalibration, 
         M_NUMBER_OF_CALIBRATION_POINTS+M_TYPE_MIL_INT, 
         &DisplayHookRulerData.NumCalibrationPoints);

      DisplayHookRulerData.WorldCalibrationPointsX = 
         new MIL_DOUBLE [DisplayHookRulerData.NumCalibrationPoints];

      DisplayHookRulerData.WorldCalibrationPointsY = 
         new MIL_DOUBLE [DisplayHookRulerData.NumCalibrationPoints];

      DisplayHookRulerData.PixelCalibrationPointsX = 
         new MIL_DOUBLE [DisplayHookRulerData.NumCalibrationPoints]; 

      DisplayHookRulerData.PixelCalibrationPointsY = 
         new MIL_DOUBLE [DisplayHookRulerData.NumCalibrationPoints];

      // Get the world coordinates of the calibration points.
      McalInquire(DisplayHookRulerData.MilCalibration, M_CALIBRATION_WORLD_POINTS_X, 
         DisplayHookRulerData.WorldCalibrationPointsX);
      McalInquire(DisplayHookRulerData.MilCalibration, M_CALIBRATION_WORLD_POINTS_Y, 
         DisplayHookRulerData.WorldCalibrationPointsY);

      // Transform the points from world to pixel.
      McalTransformCoordinateList(DisplayHookRulerData.MilImage, M_WORLD_TO_PIXEL, 
         DisplayHookRulerData.NumCalibrationPoints, 
         DisplayHookRulerData.WorldCalibrationPointsX, 
         DisplayHookRulerData.WorldCalibrationPointsY, 
         DisplayHookRulerData.PixelCalibrationPointsX, 
         DisplayHookRulerData.PixelCalibrationPointsY);

      // Get the number of rows and columns in the calibration grid.
      McalInquire(DisplayHookRulerData.MilCalibration, M_ROW_NUMBER+M_TYPE_MIL_INT, 
         &DisplayHookRulerData.RowNumber);

      McalInquire(DisplayHookRulerData.MilCalibration, M_COLUMN_NUMBER+M_TYPE_MIL_INT, 
         &DisplayHookRulerData.ColumnNumber);

      // Get the row and column spacings in world units.
      McalInquire(DisplayHookRulerData.MilCalibration, M_ROW_SPACING, 
         &DisplayHookRulerData.RowWorldSpacing);

      McalInquire(DisplayHookRulerData.MilCalibration, M_COLUMN_SPACING, 
         &DisplayHookRulerData.ColumnWorldSpacing);

      // Draw the ruler in the transformed image.
      DrawRuler(0, 0, (void*)&DisplayHookRulerData);

      // Initialize the number of defined measurement markers.
      DisplayHookMeasureData.NumDefinedMarkers = 0;

      // Hook the DrawRuler function to the display.  Calls DrawRuler each time 
      // the user presses a key, clicks the mouse or scrolls the mouse wheel.
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_KEY_CHAR, DrawRuler, 
         (void*)&DisplayHookRulerData);

      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_KEY_UP, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_LEFT_BUTTON_UP, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_MIDDLE_BUTTON_UP, 
         DrawRuler, (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_LEFT_DOUBLE_CLICK, 
         DrawRuler, (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_MOVE, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_WHEEL, DrawRuler, 
         (void*)&DisplayHookRulerData);

      // Hook to drawing measurement box when the mouse moves.
      MdispHookFunction(DisplayHookMeasureData.MilDisplay, M_MOUSE_MOVE, MeasMouseMove,
         (void*)&DisplayHookMeasureData);

      // Hook the MeasureDistance function to the display.
      // Calls MeasureDistance each time the user does a right-click.
      MdispHookFunction(DisplayHookMeasureData.MilDisplay, M_MOUSE_RIGHT_BUTTON_UP, 
         MeasMouseRightClick, (void*)&DisplayHookMeasureData);

      // Show the transformed image. 
      MosPrintf(MIL_TEXT("The image was corrected to remove its distortions and a "));
      MosPrintf(MIL_TEXT("ruler was added.\nThe ruler is marked with measurements in "));   
      MosPrintf(MIL_TEXT("world coordinates.\nYou can:\n"));
      MosPrintf(MIL_TEXT("- Zoom and pan the image to view measurements at various "));   
      MosPrintf(MIL_TEXT("locations.\n- Right-click on areas with edges to add ")
                MIL_TEXT("two points, measure and view\n  the distance between them.\n"));

      // Wait for a key to be pressed.
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Unhook ruler functions from the display.
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_KEY_CHAR+M_UNHOOK, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_KEY_UP+M_UNHOOK, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, 
         M_MOUSE_LEFT_BUTTON_UP+M_UNHOOK, DrawRuler, (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, 
         M_MOUSE_MIDDLE_BUTTON_UP+M_UNHOOK, DrawRuler, (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, 
         M_MOUSE_LEFT_DOUBLE_CLICK+M_UNHOOK, DrawRuler, (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_MOVE+M_UNHOOK, DrawRuler, 
         (void*)&DisplayHookRulerData);
      MdispHookFunction(DisplayHookRulerData.MilDisplay, M_MOUSE_WHEEL+M_UNHOOK, DrawRuler, 
         (void*)&DisplayHookRulerData);

      // Unhook measurement functions from the display.
      MdispHookFunction(DisplayHookMeasureData.MilDisplay, M_MOUSE_MOVE +M_UNHOOK, 
         MeasMouseMove, (void*)&DisplayHookMeasureData);
      MdispHookFunction(DisplayHookMeasureData.MilDisplay, 
         M_MOUSE_RIGHT_BUTTON_UP +M_UNHOOK, MeasMouseRightClick, 
         (void*)&DisplayHookMeasureData);

      // Free all allocations. 
      delete [] DisplayHookRulerData.WorldCalibrationPointsX;
      delete [] DisplayHookRulerData.WorldCalibrationPointsY;
      delete [] DisplayHookRulerData.PixelCalibrationPointsX; 
      delete [] DisplayHookRulerData.PixelCalibrationPointsY;
      }
   else
      {
      MosPrintf(MIL_TEXT("Calibration did not succeed with this grid image.\n"));
      MosPrintf(MIL_TEXT("See User Guide to resolve the situation.\n\n"));

      // Wait for a key to be pressed.  
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   MgraFree(DisplayHookRulerData.MilGraphics);
   MgraFree(DisplayHookRulerData.MilDisplayGraphicList);
   MgraFree(DisplayHookRulerData.MilRulerGraphicList);
   MgraFree(DisplayHookMeasureData.MilMeasGraphicList);
   MgraFree(DisplayHookMeasureData.MilMeasBoxGraphicList);
   McalFree(DisplayHookRulerData.MilCalibration);
   MmeasFree(DisplayHookMeasureData.MilMeasMarker1);
   MmeasFree(DisplayHookMeasureData.MilMeasMarker2);
   MmeasFree(DisplayHookMeasureData.MilMeasCalculateRes);
   MbufFree(DisplayHookRulerData.MilImage);
   MdispFree(DisplayHookRulerData.MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

#define MAJOR_MEASURE_DISTANCE      40
#define MEDIUM_MEASURE_DISTANCE     20
#define MINOR_MEASURE_DISTANCE      10
//*****************************************************************************
// Function to draw the calibrated ruler.
//*****************************************************************************
MIL_INT MFTYPE DrawRuler(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr)
   {
   const MIL_INT RulerWidth = 1;
   const MIL_INT MeasureLineLength = 2;  
   const MIL_INT MajorMeasureLineHeight = 8;
   const MIL_INT MediumMeasureLineHeight = 4;
   const MIL_INT MinorMeasureLineHeight = 2;

   DispHookRulerDataStruct* pRulerData = (DispHookRulerDataStruct*)UserDataPtr;

   MIL_ID MilDisplay            = pRulerData->MilDisplay;
   MIL_ID MilCalibration        = pRulerData->MilCalibration;
   MIL_ID MilCalibratedImage    = pRulerData->MilImage;
   MIL_ID MilDisplayGraphicList = pRulerData->MilDisplayGraphicList;
   MIL_ID MilRulerGraphicList   = pRulerData->MilRulerGraphicList;
   MIL_ID MilMeasGraphicList    = pRulerData->MilMeasGraphicList;
   MIL_ID MilMeasBoxGraphicList = pRulerData->MilMeasBoxGraphicList;
   MIL_ID MilGraphics           = pRulerData->MilGraphics;
   MIL_INT SizeX                = pRulerData->ImageSizeX;
   MIL_INT SizeY                = pRulerData->ImageSizeY;

   MIL_DOUBLE PanOffsetX, ZoomFactorX;
   MIL_DOUBLE PanOffsetY, ZoomFactorY;

   // Set the color of the ruler.
   MgraColor(MilGraphics, RULER_COLOR);

   // Set the input units to display so the ruler size is not altered when the display.
   // is panned or zoomed.
   MgraControl(MilGraphics, M_INPUT_UNITS, M_DISPLAY);

   // Get the zoom factors and offsets of the displayed image.
   MdispInquire(MilDisplay, M_PAN_OFFSET_X, &PanOffsetX);
   MdispInquire(MilDisplay, M_PAN_OFFSET_Y, &PanOffsetY);

   MdispInquire(MilDisplay, M_ZOOM_FACTOR_X, &ZoomFactorX);
   MdispInquire(MilDisplay, M_ZOOM_FACTOR_Y, &ZoomFactorY);

   // Do not redraw the ruler if zoom and pan have not changed.
   if ( PanOffsetX  == pRulerData->PrevDispPanOffsetX && 
        PanOffsetY  == pRulerData->PrevDispPanOffsetY &&
        ZoomFactorX == pRulerData->PrevDispZoomX      &&
        ZoomFactorY == pRulerData->PrevDispZoomY )
      { return 0; }

   // Keep the new zoom factors and pan offsets.
   pRulerData->PrevDispPanOffsetX   = PanOffsetX;
   pRulerData->PrevDispPanOffsetY   = PanOffsetY;
   pRulerData->PrevDispZoomX        = ZoomFactorX;
   pRulerData->PrevDispZoomY        = ZoomFactorY;

   // Disable update of the graphics list to the display while updating the list.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Clear the graphics lists.
   MgraClear(M_DEFAULT, MilDisplayGraphicList);
   MgraClear(M_DEFAULT, MilRulerGraphicList);

   // Determine the ruler size on the x-axis.
   MIL_INT RulerSize = 
      (MIL_INT)(ceil( (SizeX*ZoomFactorX-1) - PanOffsetX*ZoomFactorX) + 1);

   // Draw the x-axis.
   MgraRectFill(MilGraphics, MilRulerGraphicList, 0, 0, RulerSize-1, RulerWidth);  

   // Allocate the ruler array.
   RulerDataStruct* RulerArray = new RulerDataStruct[RulerSize];

   // Fill the ruler values on the x-axis.
   FillRulerMeasures(*pRulerData, RulerSize, eXAxis, RulerArray);

   // Draw the ruler lines and measurements on the x-axis.
   MIL_TEXT_CHAR MeasureString[STRING_SIZE];

   for (MIL_INT i=0; i < RulerSize; i++)
      {
      if (i > MINOR_MEASURE_DISTANCE)
         {
         if (RulerArray[i].DrawMajorMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, i, 0, i+MeasureLineLength, 
               MajorMeasureLineHeight);

            if (i > MEDIUM_MEASURE_DISTANCE)
               {
               MosSprintf(MeasureString, STRING_SIZE, MIL_TEXT("%.2f"), 
                  RulerArray[i].Measure);  

               MgraText(MilGraphics, MilRulerGraphicList, i-6, MajorMeasureLineHeight+1, 
                  MeasureString); 
               }
            }
         else if (RulerArray[i].DrawMediumMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, i, 0, i+MeasureLineLength, 
               MediumMeasureLineHeight);
            }
         else if (RulerArray[i].DrawMinorMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, i, 0, i+MeasureLineLength, 
               MinorMeasureLineHeight);
            }
         }
      }

   // Delete the array.
   delete [] RulerArray;

   // Determine the ruler size on the y-axis.
   RulerSize = (MIL_INT)(ceil((SizeY*ZoomFactorY-1)-PanOffsetY*ZoomFactorY) + 1);

   // Draw the y-axis.
   MgraRectFill(MilGraphics, MilRulerGraphicList, 0, 0, RulerWidth, RulerSize-1);  

   // Allocate the ruler array.
   RulerArray = new RulerDataStruct[RulerSize];

   // Fill the ruler values on the y-axis.
   FillRulerMeasures(*pRulerData, RulerSize, eYAxis, RulerArray);

   // Draw the ruler lines and measurements on the y-axis.
   for (MIL_INT i=0; i < RulerSize; i++)
      {
      if (i > MINOR_MEASURE_DISTANCE)
         {
         if (RulerArray[i].DrawMajorMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, 0, i, MajorMeasureLineHeight, 
               i+MeasureLineLength);

            if (i > MEDIUM_MEASURE_DISTANCE)
               {
               MosSprintf(MeasureString, STRING_SIZE, MIL_TEXT("%.2f"), 
                  RulerArray[i].Measure);  

               MgraText(MilGraphics, MilRulerGraphicList, MajorMeasureLineHeight+3, i-4, 
                  MeasureString); 
               }
            }
         else if (RulerArray[i].DrawMediumMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, 0, i, MediumMeasureLineHeight, 
               i+MeasureLineLength);
            }
         else if (RulerArray[i].DrawMinorMeasure)
            {
            MgraRectFill(MilGraphics, MilRulerGraphicList, 0, i, MinorMeasureLineHeight, 
               i+MeasureLineLength);
            }
         }
      }

   // Delete the array.
   delete [] RulerArray;

   // Copy the ruler graphics list to the display graphics list.
   MgraCopy(MilRulerGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   // Copy the measurement graphics list to the display graphics list.
   MgraCopy(MilMeasGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   MgraCopy(MilMeasBoxGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   // Enable update of the graphics list to the display.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   return 0;
   }

//*****************************************************************************
// Find all the measures we would like to draw in the ruler and fill the 
// necessary data in the array to indicate where to drawn them.
//*****************************************************************************
void FillRulerMeasures(const DispHookRulerDataStruct& HookData, MIL_INT RulerArraySize, 
   RulerType Type, RulerDataStruct* RulerArray)
   {
   MIL_INT NumCalibrationPoints = HookData.NumCalibrationPoints;
   MIL_INT RowNumber = HookData.RowNumber;
   MIL_INT ColumnNumber = HookData.ColumnNumber;

   MIL_DOUBLE ColumnWorldSpacing = HookData.ColumnWorldSpacing;
   MIL_DOUBLE RowWorldSpacing = HookData.RowWorldSpacing;

   MIL_DOUBLE* WorldCalibrationPointsX = HookData.WorldCalibrationPointsX;
   MIL_DOUBLE* WorldCalibrationPointsY = HookData.WorldCalibrationPointsY;

   MIL_DOUBLE* PixelCalibrationPointsX = HookData.PixelCalibrationPointsX;
   MIL_DOUBLE* PixelCalibrationPointsY = HookData.PixelCalibrationPointsY;

   MIL_DOUBLE* DisplayCalibrationPointsX = M_NULL;
   MIL_DOUBLE* DisplayCalibrationPointsY = M_NULL;

   MIL_ID MilCalibration = HookData.MilCalibration;
   MIL_ID MilCalibratedImage = HookData.MilImage;
   MIL_DOUBLE ZoomFactorX = HookData.PrevDispZoomX;
   MIL_DOUBLE ZoomFactorY = HookData.PrevDispZoomY;
   MIL_DOUBLE PanOffsetX = HookData.PrevDispPanOffsetX;
   MIL_DOUBLE PanOffsetY = HookData.PrevDispPanOffsetY;

   MIL_INT ImageSizeX = HookData.ImageSizeX;
   MIL_INT ImageSizeY = HookData.ImageSizeY;

   // Initialize the ruler.
   for (MIL_INT i=0; i < RulerArraySize; i++)
      {
      RulerArray[i].DrawMajorMeasure = false;
      RulerArray[i].DrawMediumMeasure = false;
      RulerArray[i].DrawMinorMeasure = false;
      }

   DisplayCalibrationPointsX = new MIL_DOUBLE [NumCalibrationPoints];
   DisplayCalibrationPointsY = new MIL_DOUBLE [NumCalibrationPoints];

   // Transform the calibration points from pixel to display units.
   PixelToDisplay(PixelCalibrationPointsX, PixelCalibrationPointsY, 
                  DisplayCalibrationPointsX, DisplayCalibrationPointsY,
                  NumCalibrationPoints, ZoomFactorX, ZoomFactorY, PanOffsetX, 
                  PanOffsetY);

   // Find the places in the ruler where the measures will be.
   MIL_INT PreviousRulerCoord = 0;
   MIL_INT CurrentRulerCoord = 0;

   MIL_DOUBLE PreviousPixelCoordX = (MIL_DOUBLE)PreviousRulerCoord;
   MIL_DOUBLE PreviousPixelCoordY = (MIL_DOUBLE)PreviousRulerCoord;

   MIL_DOUBLE PreviousWorldCoordX = 0;
   MIL_DOUBLE PreviousWorldCoordY = 0;
   MIL_DOUBLE CurrentWorldCoord = 0;

   // Initialize the previous world coordinates according to the current zoom 
   // factors and pan offsets.
   DisplayToPixel(&PreviousPixelCoordX, &PreviousPixelCoordY, &PreviousPixelCoordX, 
      &PreviousPixelCoordY, 1, ZoomFactorX, ZoomFactorY, PanOffsetX, PanOffsetY);
   McalTransformCoordinate(MilCalibratedImage, M_PIXEL_TO_WORLD, PreviousPixelCoordX, 
      PreviousPixelCoordY, &PreviousWorldCoordX, &PreviousWorldCoordY);

   if (Type == eXAxis)
      {
      MIL_INT SpacingFactor = (MIL_INT)
         ceil(MAJOR_MEASURE_DISTANCE/(DisplayCalibrationPointsX[1]-
                                      DisplayCalibrationPointsX[0]));

      // Position the previous world coordinate so that the distance between it and the
      // first point to draw from is a multiple of the world spacing.  This ensures
      // an even spacing between ruler measures.
      for (MIL_INT i=0; i < ColumnNumber; i++)
         {
         if ( DisplayCalibrationPointsX[i] > MAJOR_MEASURE_DISTANCE)
            {
            CurrentWorldCoord = WorldCalibrationPointsX[i];
            while (CurrentWorldCoord - PreviousWorldCoordX > 0)   
               {
               CurrentWorldCoord = CurrentWorldCoord - SpacingFactor*ColumnWorldSpacing;
               }
            PreviousWorldCoordX = CurrentWorldCoord;
            break;
            }
         }

      //Assign the calibration point measures then fill the gaps with more measures.
      for (MIL_INT i=0; i < ColumnNumber; i++)
         {
         CurrentRulerCoord = M_Round(DisplayCalibrationPointsX[i]);
         CurrentWorldCoord = WorldCalibrationPointsX[i];

         if ( (CurrentRulerCoord - PreviousRulerCoord) > MAJOR_MEASURE_DISTANCE)
            {
            RulerArray[CurrentRulerCoord].DrawMajorMeasure = true;
            RulerArray[CurrentRulerCoord].Measure = WorldCalibrationPointsX[i];

            FillSubRulerMeasures(MilCalibratedImage, PreviousWorldCoordX, 
               CurrentWorldCoord, ZoomFactorX, ZoomFactorY, PanOffsetX, PanOffsetY, Type, 
               RulerArraySize, RulerArray, (CurrentRulerCoord - PreviousRulerCoord)-1);

            PreviousRulerCoord = CurrentRulerCoord;            
            PreviousWorldCoordX = CurrentWorldCoord;
            }
         }

      MIL_DOUBLE EndWorldCoordX = 0;
      MIL_DOUBLE EndWorldCoordY = 0;

      // Initialize the last coordinates of ruler and fill the measures until 
      // the end of the image.
      McalTransformCoordinate(MilCalibratedImage, M_PIXEL_TO_WORLD, 
         (MIL_DOUBLE)ImageSizeX-1, 0, &EndWorldCoordX, &EndWorldCoordY);

      MIL_DOUBLE PreviousWorldCoordX2 = PreviousWorldCoordX;

      while (EndWorldCoordX - PreviousWorldCoordX2 > 0)  
         {
         PreviousWorldCoordX2 = PreviousWorldCoordX2 + SpacingFactor*ColumnWorldSpacing;
         }

      EndWorldCoordX = PreviousWorldCoordX2;

      FillSubRulerMeasures(MilCalibratedImage, PreviousWorldCoordX, EndWorldCoordX, 
         ZoomFactorX, ZoomFactorY, PanOffsetX, PanOffsetY, Type, RulerArraySize, 
         RulerArray, (RulerArraySize-PreviousRulerCoord)-1);
      }
   else if (Type == eYAxis)
      {
      MIL_INT SpacingFactor = (MIL_INT)
         ceil(MAJOR_MEASURE_DISTANCE/
         (DisplayCalibrationPointsY[ColumnNumber]-DisplayCalibrationPointsY[0]));

      // Position the previous world coordinate so that the distance between it and the
      // first point to draw from is a multiple of the world spacing.  This ensures
      // an even spacing between ruler measures.
      for (MIL_INT i=0; i < NumCalibrationPoints; i+=ColumnNumber)
         {
         if ( DisplayCalibrationPointsY[i] > MAJOR_MEASURE_DISTANCE)
            {            
            CurrentWorldCoord = WorldCalibrationPointsY[i];
            while (CurrentWorldCoord - PreviousWorldCoordY > 0)   
               {
               CurrentWorldCoord = CurrentWorldCoord - SpacingFactor*RowWorldSpacing;
               }
            PreviousWorldCoordY = CurrentWorldCoord;
            break;
            }
         }

      // Assign the calibration point measures then fill the gaps with more measures.
      for (MIL_INT i=0; i < NumCalibrationPoints; i+=ColumnNumber)
         {
         CurrentRulerCoord = M_Round(DisplayCalibrationPointsY[i]);
         CurrentWorldCoord = WorldCalibrationPointsY[i];

         if ( (CurrentRulerCoord - PreviousRulerCoord) > MAJOR_MEASURE_DISTANCE)
            {
            RulerArray[CurrentRulerCoord].DrawMajorMeasure = true;
            RulerArray[CurrentRulerCoord].Measure = WorldCalibrationPointsY[i];

            FillSubRulerMeasures(MilCalibratedImage, PreviousWorldCoordY, 
               CurrentWorldCoord, ZoomFactorX, ZoomFactorY, PanOffsetX, PanOffsetY, 
               Type, RulerArraySize, RulerArray, 
               (CurrentRulerCoord - PreviousRulerCoord)-1);

            PreviousRulerCoord = CurrentRulerCoord;
            PreviousWorldCoordY = CurrentWorldCoord;
            }
         }

      MIL_DOUBLE EndWorldCoordX = 0;
      MIL_DOUBLE EndWorldCoordY = 0;

      // Initialize the last coordinates of ruler and fill the measures until the 
      // end of the image.
      McalTransformCoordinate(MilCalibratedImage, M_PIXEL_TO_WORLD, 0, 
         (MIL_DOUBLE)ImageSizeY-1, &EndWorldCoordX, &EndWorldCoordY);

      MIL_DOUBLE PreviousWorldCoordY2 = PreviousWorldCoordY;

      while (EndWorldCoordY - PreviousWorldCoordY2 > 0)
         {
         PreviousWorldCoordY2 = PreviousWorldCoordY2 + SpacingFactor*RowWorldSpacing;
         }

      EndWorldCoordY = PreviousWorldCoordY2;

      FillSubRulerMeasures(MilCalibratedImage, PreviousWorldCoordY, EndWorldCoordY, 
         ZoomFactorX, ZoomFactorY, PanOffsetX, PanOffsetY, Type, RulerArraySize, 
         RulerArray,  (RulerArraySize-PreviousRulerCoord)-1);
      }

   // Free the display calibration point arrays.
   delete[] DisplayCalibrationPointsX;
   delete[] DisplayCalibrationPointsY;
   }

//*****************************************************************************
// Fill the ruler with measurements between the two given points on the ruler
// using a stack.
//*****************************************************************************
void FillSubRulerMeasures(MIL_ID MilCalibratedImage, MIL_DOUBLE WorldStartCoord, 
   MIL_DOUBLE WorldEndCoord, MIL_DOUBLE ZoomX, MIL_DOUBLE ZoomY, MIL_DOUBLE OffsetX,  
   MIL_DOUBLE OffsetY, RulerType Type, MIL_INT RulerArraySize, RulerDataStruct* RulerArray,
   MIL_INT SubRulerSize)
   {
   typedef struct
      {
      MIL_DOUBLE StartCoord;
      MIL_DOUBLE EndCoord;
      } CoordStruct;

   enum CoordType
      {
      eStart,
      eEnd
      };

   const MIL_INT NumCoordType = 2;

   // The size of the sub-ruler should be at least 1.
   if (SubRulerSize < 1)
      return;

   MIL_INT NumStackElements = 0;

   // Allocate the stack.
   CoordStruct* MidPointStack = new CoordStruct[SubRulerSize];
   
   // Initialize the stack with the start and end world coordinates.
   MidPointStack[0].StartCoord = WorldStartCoord;
   MidPointStack[0].EndCoord   = WorldEndCoord;
   NumStackElements = 1;

   MIL_DOUBLE WorldCoordX  [NumCoordType];
   MIL_DOUBLE WorldCoordY  [NumCoordType];
   MIL_DOUBLE PixelCoordX  [NumCoordType];
   MIL_DOUBLE PixelCoordY  [NumCoordType];
   MIL_DOUBLE DisplayCoordX[NumCoordType];
   MIL_DOUBLE DisplayCoordY[NumCoordType];

   MIL_DOUBLE WorldMidPointCoord;
   MIL_DOUBLE PrevWorldStartCoord, PrevWorldEndCoord;

   // Process the coordinates in the stack.
   while (NumStackElements > 0 && NumStackElements < SubRulerSize)
      {
      // Get required coordinates from the top element of the stack.
      PrevWorldStartCoord = MidPointStack[NumStackElements-1].StartCoord;
      PrevWorldEndCoord   = MidPointStack[NumStackElements-1].EndCoord;

      // Pop the stack.
      NumStackElements--; 

      // Find the midpoint between the two given world coordinates on top of the stack.
      WorldMidPointCoord = PrevWorldStartCoord + 
         (PrevWorldEndCoord - PrevWorldStartCoord)/2;

      if (Type == eXAxis)
         {
         // Get the start and end of the first half of the sub-ruler.
         WorldCoordX[eStart] = PrevWorldStartCoord;
         WorldCoordY[eStart] = 0;

         WorldCoordX[eEnd] = WorldMidPointCoord;
         WorldCoordY[eEnd] = 0;         

         // Transform the coordinates from world to pixel.
         McalTransformCoordinateList(MilCalibratedImage, M_WORLD_TO_PIXEL, 
            NumCoordType, WorldCoordX, WorldCoordY, PixelCoordX, PixelCoordY);

         // Transform the coordinates from pixel to display units.
         PixelToDisplay(PixelCoordX, PixelCoordY, DisplayCoordX, DisplayCoordY, 
            NumCoordType, ZoomX, ZoomY, OffsetX, OffsetY);

         // Get the distance on the ruler.
         MIL_DOUBLE SubRulerDistance = DisplayCoordX[eEnd]-DisplayCoordX[eStart];

         if (SubRulerDistance > MINOR_MEASURE_DISTANCE)
            {
            MIL_INT RoundedDisplayEndCoordX = M_Round(DisplayCoordX[eEnd]);

            if ( DisplayCoordX[eEnd]>=0 &&  RoundedDisplayEndCoordX < RulerArraySize)
               {
               // Set the measurement information.
               if (SubRulerDistance > MAJOR_MEASURE_DISTANCE)
                  {
                  RulerArray[RoundedDisplayEndCoordX].DrawMajorMeasure = true;
                  RulerArray[RoundedDisplayEndCoordX].Measure = WorldMidPointCoord;
                  }
               else if (SubRulerDistance > MEDIUM_MEASURE_DISTANCE)
                  {
                  RulerArray[RoundedDisplayEndCoordX].DrawMediumMeasure = true;
                  RulerArray[RoundedDisplayEndCoordX].Measure = WorldMidPointCoord;
                  }
               else 
                  {
                  RulerArray[RoundedDisplayEndCoordX].DrawMinorMeasure = true;
                  RulerArray[RoundedDisplayEndCoordX].Measure = WorldMidPointCoord;
                  }
               }

            // Add the first subdivision to the stack.
            MidPointStack[NumStackElements].StartCoord = WorldCoordX[eStart];
            MidPointStack[NumStackElements].EndCoord   = WorldCoordX[eEnd];
            NumStackElements++;

            // Add the second subdivision to the stack.
            MidPointStack[NumStackElements].StartCoord = WorldMidPointCoord;  
            MidPointStack[NumStackElements].EndCoord   = PrevWorldEndCoord;
            NumStackElements++;
            }
         }
      else // eYAxis
         {
         // Get the start and end of the first half of the sub-ruler.
         WorldCoordX[eStart] = 0;
         WorldCoordY[eStart] = PrevWorldStartCoord;

         WorldCoordX[eEnd] = 0;
         WorldCoordY[eEnd] = WorldMidPointCoord;         

         // Transform the start coordinates from world to pixel.
         McalTransformCoordinateList(MilCalibratedImage, M_WORLD_TO_PIXEL, NumCoordType, 
            WorldCoordX, WorldCoordY, PixelCoordX, PixelCoordY);

         // Transform the start coordinates from pixel to display units.
         PixelToDisplay(PixelCoordX, PixelCoordY, DisplayCoordX, DisplayCoordY, 
            NumCoordType, ZoomX, ZoomY, OffsetX, OffsetY);

         // Get the distance on the ruler.
         MIL_DOUBLE SubRulerDistance = DisplayCoordY[eEnd] - DisplayCoordY[eStart];

         if (SubRulerDistance > MINOR_MEASURE_DISTANCE)
            {
            MIL_INT RoundedDisplayEndCoordY = M_Round(DisplayCoordY[eEnd]);

            if (DisplayCoordY[eEnd]>=0 && RoundedDisplayEndCoordY < RulerArraySize)
               {
               // Set the measurement information.
               if (SubRulerDistance > MAJOR_MEASURE_DISTANCE)
                  {
                  RulerArray[RoundedDisplayEndCoordY].DrawMajorMeasure = true;
                  RulerArray[RoundedDisplayEndCoordY].Measure = WorldMidPointCoord;
                  }
               else if (SubRulerDistance > MEDIUM_MEASURE_DISTANCE)
                  {
                  RulerArray[RoundedDisplayEndCoordY].DrawMediumMeasure = true;
                  RulerArray[RoundedDisplayEndCoordY].Measure = WorldMidPointCoord;
                  }
               else 
                  {
                  RulerArray[RoundedDisplayEndCoordY].DrawMinorMeasure = true;
                  RulerArray[RoundedDisplayEndCoordY].Measure = WorldMidPointCoord;
                  }
               }

            // Add the first subdivision to the stack.
            MidPointStack[NumStackElements].StartCoord = WorldCoordY[eStart];
            MidPointStack[NumStackElements].EndCoord   = WorldCoordY[eEnd];
            NumStackElements++;

            // Add the second subdivision to the stack.
            MidPointStack[NumStackElements].StartCoord = WorldMidPointCoord;
            MidPointStack[NumStackElements].EndCoord   = PrevWorldEndCoord;
            NumStackElements++;
            }
         }
      }

   delete [] MidPointStack;
   }

//*****************************************************************************
// Function that manages measurement positions and calculations.
//*****************************************************************************
MIL_INT MFTYPE MeasureDistance(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr, 
   bool RightClick)
   {
   DispHookMeasureDataStruct* pMeasData = (DispHookMeasureDataStruct*)UserDataPtr;

   MIL_ID MilDisplay            = pMeasData->MilDisplay;
   MIL_ID MilCalibration        = pMeasData->MilCalibration;
   MIL_ID MilCalibratedImage    = pMeasData->MilImage;
   MIL_ID MilDisplayGraphicList = pMeasData->MilDisplayGraphicList;
   MIL_ID MilRulerGraphicList   = pMeasData->MilRulerGraphicList;
   MIL_ID MilMeasGraphicList    = pMeasData->MilMeasGraphicList;
   MIL_ID MilMeasBoxGraphicList = pMeasData->MilMeasBoxGraphicList;
   MIL_ID MilGraphics           = pMeasData->MilGraphics;

   MIL_ID MilMeasMarker1        = pMeasData->MilMeasMarker1;
   MIL_ID MilMeasMarker2        = pMeasData->MilMeasMarker2;
   MIL_ID MilMeasCalculateRes   = pMeasData->MilMeasCalculateRes;

   MIL_INT* NumDefinedMarkers = &(pMeasData->NumDefinedMarkers);

   MIL_DOUBLE MousePositionX, MousePositionY;

   // Get the current position of the mouse in buffer coordinates.
   MdispGetHookInfo(EventID, M_MOUSE_POSITION_BUFFER_X, &MousePositionX);
   MdispGetHookInfo(EventID, M_MOUSE_POSITION_BUFFER_Y, &MousePositionY);

   // Set the color of the measurement annotations.
   MgraColor(MilGraphics, MEAS_COLOR);

   // Set the input units to pixel.
   MgraControl(MilGraphics, M_INPUT_UNITS, M_PIXEL); 

   // Disable update of the graphics list to the display while updating the list.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Clear the graphics lists.
   MgraClear(M_DEFAULT, MilDisplayGraphicList);
   MgraClear(M_DEFAULT, MilMeasBoxGraphicList);
   if ( RightClick || ((*NumDefinedMarkers) == 1) )
      { MgraClear(M_DEFAULT, MilMeasGraphicList); }

   // If there are already two defined markers, clear them and start over.
   if ( ((*NumDefinedMarkers) == 2) && RightClick)
      { (*NumDefinedMarkers) = 0; }

   // If it's just a mouse move and we don't have exactly one marker defined then 
   // just draw a box.
   if (!RightClick && (*NumDefinedMarkers) != 1)
      {
      // Set the measurement search box information according to the current position
      // of the mouse.
      MIL_DOUBLE BoxOriginX, BoxOriginY;
      MIL_DOUBLE BoxEndX, BoxEndY;

      // Set measurement box information in pixel units.
      BoxOriginX = MousePositionX - MEAS_BOX_WIDTH/2;
      BoxOriginY = MousePositionY - MEAS_BOX_HEIGHT/2;

      BoxEndX = BoxOriginX + MEAS_BOX_WIDTH - 1;
      BoxEndY = BoxOriginY + MEAS_BOX_HEIGHT - 1;

      // Define the search box in pixel units in measurement.
      MmeasSetMarker(MilMeasMarker2, M_BOX_ORIGIN, BoxOriginX, BoxOriginY);
      MmeasSetMarker(MilMeasMarker2, M_BOX_SIZE, BoxEndX-BoxOriginX+1, 
         BoxEndY-BoxOriginY+1);

      // Find the edge marker.
      MmeasFindMarker(M_DEFAULT, MilCalibratedImage, MilMeasMarker2, M_POSITION);

      // Inquire the number of markers found.
      MIL_DOUBLE NumMarkers = 0;
      MmeasGetResult(MilMeasMarker2, M_NUMBER, &NumMarkers, M_NULL);

      if (NumMarkers > 0)
         {
         // Draw the position of the second edge marker.
         MmeasDraw(MilGraphics, MilMeasMarker2, MilMeasBoxGraphicList, 
            M_DRAW_SEARCH_REGION, M_DEFAULT, M_DEFAULT);
         }
      else
         {
         // Draw a red box if no edges were found.
         MgraColor(MilGraphics, M_COLOR_RED);
         MgraRect(MilGraphics, MilMeasBoxGraphicList, BoxOriginX-0.5, 
            BoxOriginY-0.5, BoxEndX+0.5, BoxEndY+0.5);
         MgraColor(MilGraphics, MEAS_COLOR);
         }
      }
   // If there are no defined markers, add current position as the first marker.
    else if ((*NumDefinedMarkers) == 0)
      {
      // Set the measurement search box information according to the current 
      // position of the mouse.
      MIL_DOUBLE BoxOriginX, BoxOriginY;
      MIL_DOUBLE BoxEndX, BoxEndY;

      // Set measurement box information in pixel units.
      BoxOriginX = MousePositionX - MEAS_BOX_WIDTH/2;
      BoxOriginY = MousePositionY - MEAS_BOX_HEIGHT/2;

      BoxEndX = BoxOriginX + MEAS_BOX_WIDTH - 1;
      BoxEndY = BoxOriginY + MEAS_BOX_HEIGHT - 1;

      // Define the search box in pixel units in measurement.
      MmeasSetMarker(MilMeasMarker1, M_BOX_ORIGIN, BoxOriginX, BoxOriginY);
      MmeasSetMarker(MilMeasMarker1, M_BOX_SIZE, BoxEndX-BoxOriginX+1, 
         BoxEndY-BoxOriginY+1);

      // Find the edge marker.
      MmeasFindMarker(M_DEFAULT, MilCalibratedImage, MilMeasMarker1, M_POSITION);

      // Inquire the number of markers found.
      MIL_DOUBLE NumMarkers = 0;
      MmeasGetResult(MilMeasMarker1, M_NUMBER, &NumMarkers, M_NULL);

      if (NumMarkers > 0)
         {
         // Draw the marker search region.
         MmeasDraw(MilGraphics, MilMeasMarker1, MilMeasGraphicList, M_DRAW_SEARCH_REGION, 
            M_DEFAULT, M_DEFAULT);         
         (*NumDefinedMarkers)++;
         }
      else
         {
         // Draw a red box if no edges were found.
         MgraColor(MilGraphics, M_COLOR_RED);
         MgraRect(MilGraphics, MilMeasBoxGraphicList, BoxOriginX-0.5, BoxOriginY-0.5, 
            BoxEndX+0.5, BoxEndY+0.5);        
         MgraColor(MilGraphics, MEAS_COLOR);
         }
      }
   // Otherwise, add the second marker, draw the line between the two and 
   // measure the distance.
   else if ((*NumDefinedMarkers) == 1)
      {
      // Set the measurement search box information according to the current 
      // position of the mouse.
      MIL_DOUBLE BoxOriginX, BoxOriginY;
      MIL_DOUBLE BoxEndX, BoxEndY;

      //Set measurement box information in pixel units
      BoxOriginX = MousePositionX - MEAS_BOX_WIDTH/2;
      BoxOriginY = MousePositionY - MEAS_BOX_HEIGHT/2;

      BoxEndX = BoxOriginX + MEAS_BOX_WIDTH - 1;
      BoxEndY = BoxOriginY + MEAS_BOX_HEIGHT - 1;

      MmeasDraw(MilGraphics, MilMeasMarker1, MilMeasGraphicList, M_DRAW_SEARCH_REGION, 
         M_DEFAULT, M_DEFAULT);

      // Define the search box in pixel units in measurement.
      MmeasSetMarker(MilMeasMarker2, M_BOX_ORIGIN, BoxOriginX, BoxOriginY);
      MmeasSetMarker(MilMeasMarker2, M_BOX_SIZE, BoxEndX-BoxOriginX+1, 
         BoxEndY-BoxOriginY+1);

      // Find the edge marker.
      MmeasFindMarker(M_DEFAULT, MilCalibratedImage, MilMeasMarker2, M_POSITION);

      // Inquire the number of markers found.
      MIL_DOUBLE NumMarkers=0;
      MmeasGetResult(MilMeasMarker2, M_NUMBER, &NumMarkers, M_NULL);

      if (NumMarkers > 0)
         {
         // Draw the position of the second edge marker.
         MmeasDraw(MilGraphics, MilMeasMarker2, MilMeasGraphicList, M_DRAW_SEARCH_REGION, 
            M_DEFAULT, M_DEFAULT);

         MmeasCalculate(M_DEFAULT, MilMeasMarker1, MilMeasMarker2, MilMeasCalculateRes, 
            M_DISTANCE);

         MmeasDraw(MilGraphics, MilMeasCalculateRes, MilMeasGraphicList, M_DRAW_LINE, 
            M_DEFAULT, M_DEFAULT);

         // Get the pixel position of the two markers.
         MIL_DOUBLE PixelPositionX1, PixelPositionY1;
         MIL_DOUBLE PixelPositionX2, PixelPositionY2;

         MmeasSetMarker(MilMeasMarker1, M_RESULT_OUTPUT_UNITS, M_PIXEL, M_NULL);
         MmeasSetMarker(MilMeasMarker2, M_RESULT_OUTPUT_UNITS, M_PIXEL, M_NULL);
         MmeasGetResult(MilMeasMarker1, M_POSITION, &PixelPositionX1, &PixelPositionY1); 
         MmeasGetResult(MilMeasMarker2, M_POSITION, &PixelPositionX2, &PixelPositionY2);

         // Calculate and write the distance value between the two measurement positions.
         MIL_DOUBLE WorldDistance;
         MmeasControl(MilMeasCalculateRes, M_RESULT_OUTPUT_UNITS, M_WORLD);

         MmeasGetResult(MilMeasCalculateRes, M_DISTANCE+M_TYPE_DOUBLE, &WorldDistance, 
            M_NULL);

         MIL_TEXT_CHAR WorldDistanceString[STRING_SIZE];

         MosSprintf(WorldDistanceString, STRING_SIZE, MIL_TEXT("%.2f"), WorldDistance);  

         MgraFont(MilGraphics, M_FONT_DEFAULT_MEDIUM);

         MgraText(MilGraphics, MilMeasGraphicList, 
            PixelPositionX1+(PixelPositionX2-PixelPositionX1)/2+4, 
            PixelPositionY1+(PixelPositionY2-PixelPositionY1)/2+4, WorldDistanceString);

         MgraFont(MilGraphics, M_FONT_DEFAULT);

         if (RightClick)
            (*NumDefinedMarkers)++;
         }
      else
         {
         // Draw a red box if no edges were found.
         MgraColor(MilGraphics, M_COLOR_RED);
         MgraRect(MilGraphics, MilMeasBoxGraphicList, BoxOriginX-0.5, BoxOriginY-0.5, 
            BoxEndX+0.5, BoxEndY+0.5);        
         MgraColor(MilGraphics, MEAS_COLOR);
         }
      }

   // Copy the ruler graphics list to the display graphics list.
   MgraCopy(MilRulerGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   // Copy the measurement graphics list to the display graphics list.
   MgraCopy(MilMeasGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   // Copy the measurement box graphics list to the display graphics list.
   MgraCopy(MilMeasBoxGraphicList, MilDisplayGraphicList, M_DEFAULT, M_DEFAULT, M_ALL, 
      M_NULL, M_NULL, M_DEFAULT);

   // Enable update of the graphics list to the display .
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   return 0;
   }

//*****************************************************************************
// Handle measurement for mouse right-click event.
//*****************************************************************************
MIL_INT MFTYPE MeasMouseRightClick(MIL_INT HookType, MIL_ID EventID, 
   void* UserDataPtr)
   {
   MeasureDistance(HookType, EventID, UserDataPtr, true);
   return 0;
   }

//*****************************************************************************
// Handle measurement for mouse move event after the first marker has been 
// defined.
//*****************************************************************************
MIL_INT MFTYPE MeasMouseMove(MIL_INT HookType, MIL_ID EventID, void* UserDataPtr)
   {
   MeasureDistance(HookType, EventID, UserDataPtr, false);
   return 0;
   }

//*****************************************************************************
// Transform the list of coordinates from display to pixel units.
//*****************************************************************************
void DisplayToPixel(MIL_DOUBLE* DisplayX, MIL_DOUBLE* DisplayY, MIL_DOUBLE* PixelX, 
   MIL_DOUBLE* PixelY, MIL_INT NumCoordinates, MIL_DOUBLE ZoomX, MIL_DOUBLE ZoomY, 
   MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY)
   {
   for (MIL_INT i=0; i < NumCoordinates; i++)
      {
      PixelX[i] = (DisplayX[i] + 0.5)/ZoomX + OffsetX - 0.5; 
      PixelY[i] = (DisplayY[i] + 0.5)/ZoomY + OffsetY - 0.5; 
      }
   }

//*****************************************************************************
// Transform the list of coordinates from pixel to display units.
//*****************************************************************************
void PixelToDisplay(MIL_DOUBLE* PixelX, MIL_DOUBLE* PixelY, MIL_DOUBLE* DisplayX, 
   MIL_DOUBLE* DisplayY, MIL_INT NumCoordinates, MIL_DOUBLE ZoomX, MIL_DOUBLE ZoomY, 
   MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY)
   {
   for (MIL_INT i=0; i < NumCoordinates; i++)
      {
      DisplayX[i] = (PixelX[i] + 0.5 - OffsetX) * ZoomX - 0.5; 
      DisplayY[i] = (PixelY[i] + 0.5 - OffsetY) * ZoomY - 0.5; 
      }
   }
