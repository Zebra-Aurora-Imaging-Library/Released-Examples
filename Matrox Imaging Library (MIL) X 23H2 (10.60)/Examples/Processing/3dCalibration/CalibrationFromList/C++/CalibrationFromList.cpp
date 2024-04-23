﻿//***************************************************************************************
// 
// File name: CalibrationFromList.cpp  
//
// Synopsis:  This program contains an example of 3d calibration using 2d calibration
//            data.  See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("CalibrationFromList\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program demonstrates how to setup a 3d calibration using\n"));
   MosPrintf(MIL_TEXT("a list of points generated by multiple 2d calibrations.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer, graphic,\n")
             MIL_TEXT("              image processing, calibration.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

///*****************************************************************************
// Constants 
//******************************************************************************
// Source image files specification. 
#define GRIDS_IMAGE_FILE M_IMAGE_PATH MIL_TEXT("CalibrationFromList/Grids.mim")

//Grid location enum
enum GridLocation { eTop, eLeft, eRight, eNumGrids };

//General constants
static const MIL_INT NUM_POLYGON_POINTS = 4;
static const MIL_INT NUM_GRIDS = eNumGrids;
static const MIL_DOUBLE ANNOTATIONS_COLOR = M_COLOR_GREEN;
static const MIL_DOUBLE OUTSIDE_ROI_COLOR = M_RGB888(170, 175, 200);

struct PolygonPointsStruct
   {
   MIL_DOUBLE x[NUM_POLYGON_POINTS];
   MIL_DOUBLE y[NUM_POLYGON_POINTS];
   MIL_DOUBLE z[NUM_POLYGON_POINTS];
   };

struct GridCalInfoStruct
   {
   MIL_DOUBLE GridOffsetX;
   MIL_DOUBLE GridOffsetY;
   MIL_DOUBLE GridOffsetZ;
   MIL_INT RowNumber;
   MIL_INT ColumnNumber;
   MIL_DOUBLE RowSpacing;
   MIL_DOUBLE ColumnSpacing;

   MIL_DOUBLE CornerHintX;
   MIL_DOUBLE CornerHintY;
   };

struct CalibrationPointsStruct
   {
   MIL_DOUBLE* PixelPointsX;
   MIL_DOUBLE* PixelPointsY;
   MIL_DOUBLE* WorldPointsX;
   MIL_DOUBLE* WorldPointsY;
   MIL_DOUBLE* WorldPointsZ;
   MIL_INT NumPoints;
   };

//Calibration constants
static const MIL_DOUBLE GRIDS_Z_OFFSET = 20;

//Top
static const PolygonPointsStruct TOP_GRID_POLYGON =
   {
      { 29, 321, 591, 301},  //x-coordinates for polygon
      { 112, 10, 184, 324}   //y-coordinates for polygon
   };
static const GridCalInfoStruct TOP_GRID_CAL_INFO =
   {
   8,    //GridOffsetX
   12,   //GridOffsetY
   0,    //GridOffsetZ
   8,    //RowNumber
   9,    //ColumnNumber
   8,    //RowSpacing
   8,    //ColumnSpacing
   68,   //CornerHintX 
   122   //CornerHintY
   };

static const PolygonPointsStruct TOP_POLYGON_WORLD_COORDINATES = 
   {
      { 0, 0, 80, 80},   //x-coordinates for polygon
      { 0, 80, 80, 0},   //y-coordinates for polygon
      { 40+GRIDS_Z_OFFSET, 40+GRIDS_Z_OFFSET, 
        40+GRIDS_Z_OFFSET, 40+GRIDS_Z_OFFSET}  //z-coordinates for polygon
   };

//Left
static const PolygonPointsStruct LEFT_GRID_POLYGON =
   {
      { 29, 297, 267, 20},   //x-coordinates for polygon
      { 121, 323, 468, 275}  //y-coordinates for polygon
   };
static const GridCalInfoStruct LEFT_GRID_CAL_INFO =
   {
   8,    //GridOffsetX
   8,    //GridOffsetY
   0,    //GridOffsetZ
   4,    //RowNumber
   9,    //ColumnNumber
   8,    //RowSpacing
   8,    //ColumnSpacing
   36,   //CornerHintX 
   143   //CornerHintY
   };

static const PolygonPointsStruct LEFT_POLYGON_WORLD_COORDINATES = 
   {
      { 0, 0, 80, 80},   //x-coordinates for polygon
      { 0, 0, 0, 0},     //y-coordinates for polygon
      { 40+GRIDS_Z_OFFSET, 0, 0, 40+GRIDS_Z_OFFSET} //z-coordinates for polygon
   };

//Right
static const PolygonPointsStruct RIGHT_GRID_POLYGON =
   {
      { 304, 589, 558, 282},  //x-coordinates for polygon
      { 326, 188, 336, 471}   //y-coordinates for polygon
   };
static const GridCalInfoStruct RIGHT_GRID_CAL_INFO =
   {
   8,    //GridOffsetX
   8,    //GridOffsetY
   0,    //GridOffsetZ
   4,    //RowNumber
   9,    //ColumnNumber
   8,    //RowSpacing
   8,    //ColumnSpacing
   311,  //CornerHintX 
   328   //CornerHintY
   };

static const PolygonPointsStruct RIGHT_POLYGON_WORLD_COORDINATES = 
   {
      { 80, 80, 80, 80},   //x-coordinates for polygon
      { 0, 0, 80, 80},     //y-coordinates for polygon
      { 40+GRIDS_Z_OFFSET, 0, 0, 40+GRIDS_Z_OFFSET} //z-coordinates for polygon
   };

//Functions
void CreateCalibrationPoints(MIL_ID MilDisplay, MIL_ID MilDisplayImage, 
   MIL_ID MilSystem, MIL_ID MilGridsImage, const PolygonPointsStruct& GridPolygon, 
   const GridCalInfoStruct& GridCalInfo, CalibrationPointsStruct& CalibrationPoints);

void Create3dCalibrationPoints(const CalibrationPointsStruct* CalibrationPoints2d,
   CalibrationPointsStruct* CalibrationPoints3d);

void DestroyCalibrationPoints(CalibrationPointsStruct& CalibrationPoints);

void CalibrateWithMask(MIL_ID MilDisplay, MIL_ID MilDisplayImage, MIL_ID MilCalibration,
   MIL_ID MilGridsImage, const PolygonPointsStruct& GridPolygon, 
   const GridCalInfoStruct& GridCalInfo);

void AllocAndGetCalibrationPoints(MIL_ID MilCalibration, 
   CalibrationPointsStruct& CalibrationPoints);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID            MilApplication,        // Application identifier.
                     MilDisplay,            // Display identifier.
                     MilSystem,             // System identifier.
                     MilDisplayImage,       // Display image identifier.
                     MilGridsImage,         // Image identifier for the grids
                     MilCalibration;        // Calibration identifier

   CalibrationPointsStruct CalibrationPoints[NUM_GRIDS];
   CalibrationPointsStruct CalibrationPoints3d;

   // Allocate MIL objects. 
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, 
              &MilDisplay);

   // Print Header. 
   PrintHeader();

   //Restore the image with grids
   MbufRestore(GRIDS_IMAGE_FILE, MilSystem, &MilGridsImage);

   //Allocate the display image
   MbufAlloc2d(MilSystem,
               MbufInquire(MilGridsImage, M_SIZE_X, M_NULL),
               MbufInquire(MilGridsImage, M_SIZE_Y, M_NULL),
               8,
               M_IMAGE+M_PROC+M_DISP,
               &MilDisplayImage);

   //Copy the grids image to the display image
   MbufCopy(MilGridsImage, MilDisplayImage);

   //Select the image to the display
   MdispSelect(MilDisplay, MilDisplayImage);

   MosPrintf(MIL_TEXT("A 3d calibration will be set up using the 2d calibration data \n")
             MIL_TEXT("generated with the grids on the displayed object.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Show region in display
   MdispControl(MilDisplay, M_REGION_OUTSIDE_COLOR, OUTSIDE_ROI_COLOR);
   MdispControl(MilDisplay, M_REGION_OUTSIDE_SHOW, M_OPAQUE);

   //Calibrate each grid separately by masking the other grids during calibration
   //1. Top grid
   MosPrintf(MIL_TEXT("Calibrating with the top grid...\n"));
   CreateCalibrationPoints(MilDisplay, MilDisplayImage, MilSystem, MilGridsImage, 
      TOP_GRID_POLYGON, TOP_GRID_CAL_INFO, CalibrationPoints[eTop]);
   //2. Left grid
   MosPrintf(MIL_TEXT("Calibrating with the left grid...\n"));
   CreateCalibrationPoints(MilDisplay, MilDisplayImage, MilSystem, MilGridsImage, 
      LEFT_GRID_POLYGON, LEFT_GRID_CAL_INFO, CalibrationPoints[eLeft]);
   //3. Right grid
   MosPrintf(MIL_TEXT("Calibrating with the right grid...\n"));
   CreateCalibrationPoints(MilDisplay, MilDisplayImage, MilSystem, MilGridsImage, 
      RIGHT_GRID_POLYGON, RIGHT_GRID_CAL_INFO, CalibrationPoints[eRight]);

   //Stop showing the region
   MdispControl(MilDisplay, M_REGION_OUTSIDE_SHOW, M_TRANSPARENT);
   MbufSetRegion(MilDisplayImage, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);

   //Allocate a 3d calibration context
   McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, &MilCalibration);

   //Create 3d calibration points using the calibration points generated above
   Create3dCalibrationPoints(CalibrationPoints, &CalibrationPoints3d);

   //Set the principal point
   McalControl(MilCalibration, M_PRINCIPAL_POINT_X, 
      MbufInquire(MilGridsImage, M_SIZE_X, M_NULL)/2);

   McalControl(MilCalibration, M_PRINCIPAL_POINT_Y, 
      MbufInquire(MilGridsImage, M_SIZE_Y, M_NULL)/2);

   //Calibrate using McalList
   McalList(MilCalibration, CalibrationPoints3d.PixelPointsX, 
      CalibrationPoints3d.PixelPointsY, CalibrationPoints3d.WorldPointsX, 
      CalibrationPoints3d.WorldPointsY, CalibrationPoints3d.WorldPointsZ,
      CalibrationPoints3d.NumPoints, M_FULL_CALIBRATION, M_DEFAULT);

   MIL_ID MilGraphicsContext, MilGraphicList;
   MgraAlloc(MilSystem, &MilGraphicsContext);
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   //Set the annotations color
   MgraColor(MilGraphicsContext, ANNOTATIONS_COLOR);

   //Associate the graphic list to the display
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   MbufCopy(MilGridsImage, MilDisplayImage);

   //Draw the calibration data
   McalDraw(MilGraphicsContext, MilCalibration, MilGraphicList, 
      M_DRAW_ABSOLUTE_COORDINATE_SYSTEM+M_DRAW_FRAME, M_DEFAULT, M_DEFAULT);

   //Transform the wire frame world coordinates to pixel coordinates and 
   // draw the wire frame
   PolygonPointsStruct TopPolygonPixelCoordinates;
   PolygonPointsStruct LeftPolygonPixelCoordinates;
   PolygonPointsStruct RightPolygonPixelCoordinates;
  
   //Convert the top polygon from world to pixel
   McalTransformCoordinate3dList(MilCalibration, M_ABSOLUTE_COORDINATE_SYSTEM, 
      M_PIXEL_COORDINATE_SYSTEM, NUM_POLYGON_POINTS, TOP_POLYGON_WORLD_COORDINATES.x, 
      TOP_POLYGON_WORLD_COORDINATES.y, TOP_POLYGON_WORLD_COORDINATES.z, 
      TopPolygonPixelCoordinates.x, TopPolygonPixelCoordinates.y, M_NULL, M_DEFAULT);

   MgraLines(MilGraphicsContext, MilGraphicList, NUM_POLYGON_POINTS, 
      TopPolygonPixelCoordinates.x, TopPolygonPixelCoordinates.y, 
      M_NULL, M_NULL, M_POLYGON);

   //Convert the left polygon from world to pixel
   McalTransformCoordinate3dList(MilCalibration, M_ABSOLUTE_COORDINATE_SYSTEM, 
      M_PIXEL_COORDINATE_SYSTEM, NUM_POLYGON_POINTS, LEFT_POLYGON_WORLD_COORDINATES.x, 
      LEFT_POLYGON_WORLD_COORDINATES.y, LEFT_POLYGON_WORLD_COORDINATES.z, 
      LeftPolygonPixelCoordinates.x, LeftPolygonPixelCoordinates.y, M_NULL, M_DEFAULT);

   MgraLines(MilGraphicsContext, MilGraphicList, NUM_POLYGON_POINTS, 
      LeftPolygonPixelCoordinates.x, LeftPolygonPixelCoordinates.y, 
      M_NULL, M_NULL, M_POLYLINE);

   //Convert the right polygon from world to pixel
   McalTransformCoordinate3dList(MilCalibration, M_ABSOLUTE_COORDINATE_SYSTEM, 
      M_PIXEL_COORDINATE_SYSTEM, NUM_POLYGON_POINTS, RIGHT_POLYGON_WORLD_COORDINATES.x, 
      RIGHT_POLYGON_WORLD_COORDINATES.y, RIGHT_POLYGON_WORLD_COORDINATES.z, 
      RightPolygonPixelCoordinates.x, RightPolygonPixelCoordinates.y, M_NULL, M_DEFAULT);

   MgraLines(MilGraphicsContext, MilGraphicList, NUM_POLYGON_POINTS, 
      RightPolygonPixelCoordinates.x, RightPolygonPixelCoordinates.y, 
      M_NULL, M_NULL, M_POLYLINE);

   //Draw the world coordinates and the units
   const MIL_INT TEXT_SIZE = 256;
   MIL_TEXT_CHAR OutputText[TEXT_SIZE];

   MgraControl(MilGraphicsContext, M_BACKGROUND_MODE, M_OPAQUE);

   for (MIL_INT i=0; i<NUM_POLYGON_POINTS; i++)
      {
      MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("(%.0f, %.0f, %.0f)"), 
         TOP_POLYGON_WORLD_COORDINATES.x[i], TOP_POLYGON_WORLD_COORDINATES.y[i], 
         TOP_POLYGON_WORLD_COORDINATES.z[i]);

      if (i == NUM_POLYGON_POINTS-2)
         {
         MgraControl(MilGraphicsContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
         MgraControl(MilGraphicsContext, M_TEXT_ALIGN_VERTICAL, M_BOTTOM);
         }

      MgraText(MilGraphicsContext, MilGraphicList, TopPolygonPixelCoordinates.x[i], 
         TopPolygonPixelCoordinates.y[i], OutputText);
      }

   //Prepare alignment of bottom text
   MgraControl(MilGraphicsContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
   MgraControl(MilGraphicsContext, M_TEXT_ALIGN_VERTICAL, M_BOTTOM);
   MgraControl(MilGraphicsContext, M_INPUT_UNITS, M_DISPLAY);

   MgraText(MilGraphicsContext, MilGraphicList, MbufInquire(MilDisplayImage,
      M_SIZE_X, M_NULL)-1, MbufInquire(MilDisplayImage, M_SIZE_Y, M_NULL)-1, 
      MIL_TEXT("Coordinates shown in mm"));

   MosPrintf(MIL_TEXT("The 3d calibration has been defined and the object's wire frame \n")
             MIL_TEXT("along with its world coordinates (in mm) are shown.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   DestroyCalibrationPoints(CalibrationPoints[eTop]);
   DestroyCalibrationPoints(CalibrationPoints[eLeft]);
   DestroyCalibrationPoints(CalibrationPoints[eRight]);
   DestroyCalibrationPoints(CalibrationPoints3d); 
   MgraFree(MilGraphicsContext);
   MgraFree(MilGraphicList);
   MbufFree(MilGridsImage);
   MbufFree(MilDisplayImage);
   McalFree(MilCalibration);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   return 0;
   }

//*******************************************************************************
// CreateCalibrationPoints.  Create calibration points with given information.
//*******************************************************************************
void CreateCalibrationPoints(MIL_ID MilDisplay, MIL_ID MilDisplayImage, 
   MIL_ID MilSystem, MIL_ID MilGridsImage, const PolygonPointsStruct& GridPolygon, 
   const GridCalInfoStruct& GridCalInfo, CalibrationPointsStruct& CalibrationPoints)
   {
   MIL_ID MilCalibration;

   //Allocate calibration context
   McalAlloc(MilSystem, M_LINEAR_INTERPOLATION , M_DEFAULT, &MilCalibration);

   //Apply the provided mask then calibrate remaining grid
   CalibrateWithMask(MilDisplay, MilDisplayImage, MilCalibration, MilGridsImage, 
      GridPolygon, GridCalInfo);

   //Allocate memory for calibration points and fill the data
   AllocAndGetCalibrationPoints(MilCalibration, CalibrationPoints);

   //Free the calibration points
   McalFree(MilCalibration);
   }

//*******************************************************************************
// CalibrateWithMask.  Apply masks to the image then calibrate.
//*******************************************************************************
void CalibrateWithMask(MIL_ID MilDisplay, MIL_ID MilDisplayImage, MIL_ID MilCalibration, 
   MIL_ID MilGridsImage, const PolygonPointsStruct& GridPolygon, 
   const GridCalInfoStruct& GridCalInfo)
   {
   MIL_ID MilSystem;
   MbufInquire(MilGridsImage, M_OWNER_SYSTEM, &MilSystem);

   //Setup graphic list to define mask polygon
   MIL_ID MilGraphicsContext = MgraAlloc(MilSystem, M_NULL);
   MIL_ID MilGraListRoi = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   //Define the polygon in a graphic list and set the region to the image
   MgraLines(MilGraphicsContext, MilGraListRoi, NUM_POLYGON_POINTS,
             GridPolygon.x, GridPolygon.y, M_NULL, M_NULL, M_POLYGON + M_FILLED);
   MbufSetRegion(MilGridsImage, MilGraListRoi, M_DEFAULT, M_RASTERIZE, M_DEFAULT);

   //Provide the corner hint
   McalControl(MilCalibration, M_GRID_HINT_PIXEL_X, GridCalInfo.CornerHintX);
   McalControl(MilCalibration, M_GRID_HINT_PIXEL_Y, GridCalInfo.CornerHintY);

   //Calibrate with the masked grid
   McalGrid(MilCalibration, MilGridsImage,
            GridCalInfo.GridOffsetX,
            GridCalInfo.GridOffsetY,
            GridCalInfo.GridOffsetZ,
            GridCalInfo.RowNumber,
            GridCalInfo.ColumnNumber,
            GridCalInfo.RowSpacing,
            GridCalInfo.ColumnSpacing,
            M_DEFAULT, M_DEFAULT);

   //Remove the region
   MbufSetRegion(MilGridsImage, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);

   //Set the annotations color
   MgraColor(MilGraphicsContext, ANNOTATIONS_COLOR);

   //Draw the calibration points
   MIL_ID MilGraListAnnotations = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   McalDraw(MilGraphicsContext, MilCalibration, MilGraListAnnotations, M_DRAW_IMAGE_POINTS,
            M_DEFAULT, M_DEFAULT);

   //Associate the graphic list to the display
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraListAnnotations);

   //Show the region in the display
   MbufSetRegion(MilDisplayImage, MilGraListRoi, M_DEFAULT, M_RASTERIZE, M_DEFAULT);

   //Show the calibration
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Clean up
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   MgraFree(MilGraListAnnotations);
   MgraFree(MilGraListRoi);
   MgraFree(MilGraphicsContext);
   }

//*******************************************************************************
// Create3dCalibrationPoints.  Create the 3d calibration points.
//*******************************************************************************
void Create3dCalibrationPoints(const CalibrationPointsStruct* CalibrationPoints2d,
   CalibrationPointsStruct* CalibrationPoints3d)
   {
   CalibrationPoints3d->NumPoints = 0;

   //Get the total number of points
   for (MIL_INT i=0; i<NUM_GRIDS; i++)
      CalibrationPoints3d->NumPoints += CalibrationPoints2d[i].NumPoints;

   //Allocate memory for all points
   CalibrationPoints3d->PixelPointsX = new MIL_DOUBLE[CalibrationPoints3d->NumPoints];
   CalibrationPoints3d->PixelPointsY = new MIL_DOUBLE[CalibrationPoints3d->NumPoints];
   CalibrationPoints3d->WorldPointsX = new MIL_DOUBLE[CalibrationPoints3d->NumPoints];
   CalibrationPoints3d->WorldPointsY = new MIL_DOUBLE[CalibrationPoints3d->NumPoints];
   CalibrationPoints3d->WorldPointsZ = new MIL_DOUBLE[CalibrationPoints3d->NumPoints];

   //Copy the list of points generated by the 2d calibration and generate the 
   // z world positions
   MIL_INT Point3d=0;
   MIL_INT MaxPoint = 0;
   for (MIL_INT i=0; i<NUM_GRIDS; i++) //Top, left, right order
      {
      MaxPoint += CalibrationPoints2d[i].NumPoints;

      for (MIL_INT j=0;Point3d<MaxPoint; j++,Point3d++)
         {
         CalibrationPoints3d->PixelPointsX[Point3d] = 
            CalibrationPoints2d[i].PixelPointsX[j];

         CalibrationPoints3d->PixelPointsY[Point3d] = 
            CalibrationPoints2d[i].PixelPointsY[j];

         if (i==eLeft)  
            {
            CalibrationPoints3d->WorldPointsX[Point3d] = 
               CalibrationPoints2d[i].WorldPointsX[j];

            CalibrationPoints3d->WorldPointsY[Point3d] = 0;  
            }
         else if (i==eRight)
            {
            CalibrationPoints3d->WorldPointsX[Point3d] = 
               (LEFT_GRID_CAL_INFO.ColumnNumber+1)*
               LEFT_GRID_CAL_INFO.ColumnSpacing;

            CalibrationPoints3d->WorldPointsY[Point3d] = 
               CalibrationPoints2d[i].WorldPointsX[j];
            }
         else
            {
            CalibrationPoints3d->WorldPointsX[Point3d] = 
               CalibrationPoints2d[i].WorldPointsY[j];

            CalibrationPoints3d->WorldPointsY[Point3d] = 
               CalibrationPoints2d[i].WorldPointsX[j];
            }
         }
      }

   //Initialize the z coordinates
   for (MIL_INT i=0; i<CalibrationPoints3d->NumPoints; i++)
      CalibrationPoints3d->WorldPointsZ[i] = 0;

   //Set the z world positions
   MIL_INT CurrPoint=0;
   MIL_INT TotalPoints=CalibrationPoints2d[eTop].NumPoints;
   //1. Top
   for (; CurrPoint<TotalPoints; CurrPoint++)
      {
      CalibrationPoints3d->WorldPointsZ[CurrPoint] = 
         (LEFT_GRID_CAL_INFO.RowNumber+1)*LEFT_GRID_CAL_INFO.RowSpacing+GRIDS_Z_OFFSET;
      }

   //2. Left
   MIL_DOUBLE CurrentHeight=0;
   TotalPoints+= CalibrationPoints2d[eLeft].NumPoints;

   for (MIL_INT i=CalibrationPoints2d[eLeft].NumPoints; CurrPoint<TotalPoints; 
        i--,CurrPoint++)
      {
      CurrentHeight = ( ceil((MIL_DOUBLE)i/LEFT_GRID_CAL_INFO.ColumnNumber) ) *
         LEFT_GRID_CAL_INFO.RowSpacing+GRIDS_Z_OFFSET;

      CalibrationPoints3d->WorldPointsZ[CurrPoint] = CurrentHeight;   
      }

   //3. Right
   TotalPoints+= CalibrationPoints2d[eRight].NumPoints;
   for (MIL_INT i=CalibrationPoints2d[eRight].NumPoints; CurrPoint<TotalPoints; 
        i--,CurrPoint++)
      {
      CurrentHeight = ( ceil((MIL_DOUBLE)i/RIGHT_GRID_CAL_INFO.ColumnNumber) ) *
         RIGHT_GRID_CAL_INFO.RowSpacing+GRIDS_Z_OFFSET;

      CalibrationPoints3d->WorldPointsZ[CurrPoint] = CurrentHeight;   
      }
   }

//*******************************************************************************
// AllocAndGetCalibrationPoints.  Allocate memory and get calibration points.
//*******************************************************************************
void AllocAndGetCalibrationPoints(MIL_ID MilCalibration, 
   CalibrationPointsStruct& CalibrationPoints)
   {
   //Get the number of calibration points
   McalInquire(MilCalibration, M_NUMBER_OF_CALIBRATION_POINTS+M_TYPE_MIL_INT, 
      &CalibrationPoints.NumPoints);

   //Allocate memory for calibration points
   CalibrationPoints.PixelPointsX = new MIL_DOUBLE[CalibrationPoints.NumPoints];
   CalibrationPoints.PixelPointsY = new MIL_DOUBLE[CalibrationPoints.NumPoints];
   CalibrationPoints.WorldPointsX = new MIL_DOUBLE[CalibrationPoints.NumPoints];
   CalibrationPoints.WorldPointsY = new MIL_DOUBLE[CalibrationPoints.NumPoints];
   CalibrationPoints.WorldPointsZ = M_NULL;

   //Get the points in pixel units
   McalInquire(MilCalibration, M_CALIBRATION_IMAGE_POINTS_X, 
      CalibrationPoints.PixelPointsX);

   McalInquire(MilCalibration, M_CALIBRATION_IMAGE_POINTS_Y, 
      CalibrationPoints.PixelPointsY);

   //Get the points in world units
   McalInquire(MilCalibration, M_CALIBRATION_WORLD_POINTS_X, 
      CalibrationPoints.WorldPointsX);

   McalInquire(MilCalibration, M_CALIBRATION_WORLD_POINTS_Y, 
      CalibrationPoints.WorldPointsY);
   }

//*******************************************************************************
// DestroyCalibrationPoints.  Detroys calibration points objects.
//*******************************************************************************
void DestroyCalibrationPoints(CalibrationPointsStruct& CalibrationPoints)
   {
   delete [] CalibrationPoints.PixelPointsX;
   delete [] CalibrationPoints.PixelPointsY;
   delete [] CalibrationPoints.WorldPointsX;
   delete [] CalibrationPoints.WorldPointsY;
   delete [] CalibrationPoints.WorldPointsZ;
   }