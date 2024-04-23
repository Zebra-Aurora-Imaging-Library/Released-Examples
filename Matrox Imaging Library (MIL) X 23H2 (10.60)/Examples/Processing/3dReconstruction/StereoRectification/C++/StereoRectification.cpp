/*******************************************************************************/
/*
* File name: StereoRectification.cpp
*
* Synopsis:  This example shows how to perform stereo rectification and 3D
*            reconstruction using MIL.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include <algorithm> // std::min, std::max, std::minmax_element
#include <cassert>   // assert
#include <cmath>     // std::ceil
#include <iterator>  // std::begin, std::end
#include "vec3.h"    // Useful 3D vector operations

//---------------------------------------------------------------------------
// Example description.
//---------------------------------------------------------------------------
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("StereoRectification\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to perform stereo rectification and 3D\n"));
   MosPrintf(MIL_TEXT("reconstruction using MIL.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer, graphic,\n")
             MIL_TEXT("              image processing, calibration.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

// ---------------------------------------------------------------------------
// Constants and Data Structures. 
// ---------------------------------------------------------------------------

// Used to index an array.
enum ECamIdx
   {
   LeftCam = 0,
   RightCam,
   NUM_CAMS
   };

// Used to convert disparity D in Z distance:
// Z = Num / (Denom + D).
struct StereoParams
   {
   MIL_DOUBLE Num;
   MIL_DOUBLE Denom;
   };

// Used to define a principal axis.
struct PrincipalAxis
   {
   Vec3 Origin, Direction;
   };

// Used to define a bounding box by its top-left and bottom-right corner.
struct Box
   {
   MIL_DOUBLE x1, y1, x2, y2;
   };

// Source image files specification.
const MIL_STRING GridFiles[NUM_CAMS] =
   {
   M_IMAGE_PATH MIL_TEXT("StereoRectification/left_grid.png"),
   M_IMAGE_PATH MIL_TEXT("StereoRectification/right_grid.png"),
   };
const MIL_STRING ObjectFiles[NUM_CAMS] =
   {
   M_IMAGE_PATH MIL_TEXT("StereoRectification/left_object.png"),
   M_IMAGE_PATH MIL_TEXT("StereoRectification/right_object.png"),
   };

// Hard coded pixel coordinates in ObjectFile before rectification.
static const MIL_INT NUM_POINTS = 4;
const MIL_DOUBLE NoRectPixelsX[NUM_CAMS][NUM_POINTS] =
   {
   // Left camera.
   {
      268.0, 665.0,  // Measure 0 Start/End X coords.
      884.0, 1024.0  // Measure 1 Start/End X coords.
   },

   // Right camera.
   {
      202.0, 526.0, // Measure 0 Start/End X coords.
      690.0, 819.0  // Measure 1 Start/End X coords.
   },
   };
const MIL_DOUBLE NoRectPixelsY[NUM_CAMS][NUM_POINTS] =
   {
   // Left camera.
   {
      480.0, 284.0, // Measure 0 Start/End Y coords.
      400.0, 596.0  // Measure 1 Start/End Y coords.
   },

   // Right camera.
   {
      570.0, 362.0, // Measure 0 Start/End Y coords.
      447.0, 612.0  // Measure 1 Start/End Y coords.
   },
   };

// Hard coded pixel coordinates in ObjectFiles to measure two lengths.
const MIL_DOUBLE PixelsX[NUM_CAMS][NUM_POINTS] =
   {
   // Left camera.
   {
      275.0, 658.0, // Measure 0 Start/End X coords.
      824.0, 959.0  // Measure 1 Start/End X coords.
   },

   // Right camera.
   {
      338.0, 741.0, // Measure 0 Start/End X coords.
      865.0, 980.0  // Measure 1 Start/End X coords.
   },
   };
const MIL_DOUBLE PixelsY[NUM_CAMS][NUM_POINTS] =
   {
   // Left camera.
   {
      481.0, 293.0, // Measure 0 Start/End Y coords.
      435.0, 626.0  // Measure 1 Start/End Y coords.
   },

   // Right camera.
   {
      481.0, 293.0, // Measure 0 Start/End Y coords.
      435.0, 626.0  // Measure 1 Start/End Y coords.
   },
   };

//---------------------------------------------------------------------------
// Functions 
//---------------------------------------------------------------------------
StereoParams StereoRectifyPreprocess(MIL_ID LeftCalId, MIL_ID RightCalId, MIL_INT ImgSizeX,
   MIL_INT ImgSizeY, MIL_ID* LeftRectifiedImgIdPtr, MIL_ID* RightRectifiedImgIdPtr);

MIL_DOUBLE SetRectifiedPlane(MIL_ID LeftCalId, MIL_ID RightCalId);

PrincipalAxis GetPrincipalAxis(MIL_ID CalId);

Vec3 ComputeRelativeZAxis(const Vec3& RelXAxis, const Vec3& LeftZAxis, const Vec3& RightZAxis);

MIL_ID ConstructHMatrix(const Vec3& RelXAxis, const Vec3& RelYAxis, const Vec3& RelZAxis,
   const Vec3& RelOrig);

Box ComputeBBox(MIL_ID  CalId, MIL_INT ImgSizeX, MIL_INT ImgSizeY);

void AdjustBoxes(Box* pLeftBBox, Box* pRightBBox, MIL_DOUBLE /*NormBaseline*/);

MIL_DOUBLE AllocateAndCalibrateRectifiedImage(MIL_ID SysId, const Box& BBox, MIL_DOUBLE PixelSize,
   MIL_ID* RectifiedImageIdPtr);

MIL_DOUBLE ComputePixelSize(MIL_ID LeftCalId, MIL_ID RightCalId);

void DrawPoints(MIL_ID MilGraphics, MIL_ID MilGraList, MIL_INT XOffset,
                const MIL_DOUBLE PixelsX[NUM_CAMS][NUM_POINTS],
                const MIL_DOUBLE PixelsY[NUM_CAMS][NUM_POINTS]);

void DrawLines(MIL_ID MilGraphics, MIL_ID MilGraList, MIL_INT XOffset, 
               const MIL_DOUBLE PixelsX[NUM_CAMS][NUM_POINTS],
               const MIL_DOUBLE PixelsY[NUM_CAMS][NUM_POINTS]);


//---------------------------------------------------------------------------
// Main.
//---------------------------------------------------------------------------
int MosMain(void)
   {
   MIL_ID  MilApplication,             // Application identifier.
           MilSystem,                  // System identifier.
           MilDisplay,                 // Display identifier.
           MilOverlay,                 // Overlay identifier.
           MilGraphics,                // Graphics identfier.
           MilGraList,                 // Graphics list identifier.
           MilCalibration[NUM_CAMS],   // Calibration identifiers.
           GridImgs[NUM_CAMS],         // Grid image identifiers.
           ObjectImgs[NUM_CAMS],       // Object image identifiers.
           RectifiedImgs[NUM_CAMS],    // Rectified image identifiers.
           DisplayParentImg,           // Parent image display identifier.
           DisplayChildImgs[NUM_CAMS]; // Child image display identifiers.
   MIL_INT CamImgSizeX,                // Image size.
           CamImgSizeY;

   // Print header.
   PrintHeader();

   // Allocate MIL objects. 
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
   
   MgraAlloc(MilSystem, &MilGraphics);
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList);

   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);
   MdispControl(MilDisplay, M_SCALE_DISPLAY, M_ENABLE);
   MdispZoom(MilDisplay, 0.5, 0.5);

   // Normal camera calibration.
   MosPrintf(MIL_TEXT("Calibrating cameras...\n"));
   for(MIL_INT CamIdx = 0; CamIdx < NUM_CAMS; ++CamIdx)
      {
      MilCalibration[CamIdx] = McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, M_NULL);

      // Calibrate using partial chessboard grid with fiducial.
      McalControl(MilCalibration[CamIdx], M_GRID_PARTIAL, M_ENABLE);
      McalControl(MilCalibration[CamIdx], M_GRID_FIDUCIAL, M_DATAMATRIX);

      MbufRestore(GridFiles[CamIdx], MilSystem, &GridImgs[CamIdx]);

      McalGrid(MilCalibration[CamIdx], GridImgs[CamIdx], 0.0, 0.0, 0.0,
               M_UNKNOWN, M_UNKNOWN, M_FROM_FIDUCIAL, M_FROM_FIDUCIAL,
               M_DEFAULT, M_CHESSBOARD_GRID);

      if(McalInquire(MilCalibration[CamIdx], M_CALIBRATION_STATUS, M_NULL) != M_CALIBRATED)
         {
         MosPrintf(MIL_TEXT("Calibration failed. Stop execution.\n\n"));
         MosGetch();
         exit(0);
         }
      }

   // We assume same size for both images.
   CamImgSizeX = MbufInquire(GridImgs[0], M_SIZE_X, M_NULL);
   CamImgSizeY = MbufInquire(GridImgs[0], M_SIZE_Y, M_NULL);

   // Allocating buffers for display.
   MbufAlloc2d(MilSystem, 2 * CamImgSizeX, CamImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC,
               &DisplayParentImg);
   MbufChild2d(DisplayParentImg, 0, 0, CamImgSizeX, CamImgSizeY, &DisplayChildImgs[0]);
   MbufChild2d(DisplayParentImg, CamImgSizeX, 0, CamImgSizeX, CamImgSizeY, &DisplayChildImgs[1]);

   // Show grid images.
   MbufCopy(GridImgs[0], DisplayChildImgs[0]);
   MbufCopy(GridImgs[1], DisplayChildImgs[1]);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MdispSelect(MilDisplay, DisplayParentImg);
   
   MgraFont(MilGraphics, M_FONT_DEFAULT_LARGE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlay);
   MgraText(MilGraphics, MilOverlay, 15, 15, MIL_TEXT("Left Camera"));
   MgraText(MilGraphics, MilOverlay, 1295, 15, MIL_TEXT("Right Camera"));

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   MosPrintf(MIL_TEXT("Calibration done: the images acquired by the two cameras are displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Prepare for stereo rectification.
   const StereoParams Params = StereoRectifyPreprocess(MilCalibration[0], MilCalibration[1],
                                                       CamImgSizeX, CamImgSizeY,
                                                       &RectifiedImgs[0], &RectifiedImgs[1]);

   // Show feature points before rectification.
   MgraColor(MilGraphics, M_COLOR_YELLOW);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   for(MIL_INT CamIdx = 0; CamIdx < NUM_CAMS; ++CamIdx)
      {
      ObjectImgs[CamIdx] = MbufRestore(ObjectFiles[CamIdx], MilSystem, M_NULL);
      MbufCopy(ObjectImgs[CamIdx], DisplayChildImgs[CamIdx]);
      }
   DrawPoints(MilGraphics, MilGraList, CamImgSizeX, NoRectPixelsX, NoRectPixelsY);
   DrawLines(MilGraphics, MilGraList, CamImgSizeX, NoRectPixelsX, NoRectPixelsY);

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   MosPrintf(MIL_TEXT("Images before rectification: the epipolar lines from same features are\n"));
   MosPrintf(MIL_TEXT("not horizontal.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Rectify runtime images.
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   for(MIL_INT CamIdx = 0; CamIdx < NUM_CAMS; ++CamIdx)
      {
      McalTransformImage(ObjectImgs[CamIdx], RectifiedImgs[CamIdx], MilCalibration[CamIdx],
                         M_BILINEAR + M_OVERSCAN_CLEAR, M_FULL_CORRECTION,
                         M_WARP_IMAGE + M_USE_DESTINATION_CALIBRATION);
      MbufCopy(RectifiedImgs[CamIdx], DisplayChildImgs[CamIdx]);
      }

   // Show feature points on epipolar lines.
   MgraClear(MilGraphics, MilGraList);
   DrawPoints(MilGraphics, MilGraList, CamImgSizeX, PixelsX, PixelsY);
   DrawLines(MilGraphics, MilGraList, CamImgSizeX, PixelsX, PixelsY);

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   MosPrintf(MIL_TEXT("Images after rectification: the epipolar lines from same features are\n"));
   MosPrintf(MIL_TEXT("now horizontal.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Convert pixels and disparities to 3d points.
   MIL_DOUBLE WldPtX[NUM_POINTS];
   MIL_DOUBLE WldPtY[NUM_POINTS];
   MIL_DOUBLE WldPtZ[NUM_POINTS];

   // This is a simple scale*pixel + offset equation, and could be done manually.
   McalTransformCoordinateList(RectifiedImgs[0], M_PIXEL_TO_WORLD, NUM_POINTS, PixelsX[LeftCam],
                               PixelsY[LeftCam], WldPtX, WldPtY);

   // Scale vectors using disparity.
   for(MIL_INT PtIdx = 0; PtIdx < NUM_POINTS; ++PtIdx)
      {
      const MIL_DOUBLE Disparity = PixelsX[LeftCam][PtIdx] - PixelsX[RightCam][PtIdx];
      const MIL_DOUBLE Divisor = Params.Denom + Disparity;
      if(Divisor != 0.0)
         {
         const MIL_DOUBLE ScaleFactor = Params.Num / Divisor;
         WldPtX[PtIdx] *= ScaleFactor;
         WldPtY[PtIdx] *= ScaleFactor;
         WldPtZ[PtIdx] = ScaleFactor;
         }
      else
         {
         // Point at infinity. Print an error message.
         MosPrintf(MIL_TEXT("Point at infinity. Stop execution.\n"));
         MosGetch();
         exit(0);
         }
      }

   // Measuring the two lengths using the left image.
   const MIL_DOUBLE Length1 = Distance(Vec3 {WldPtX[0], WldPtY[0], WldPtZ[0]},
                                       Vec3 {WldPtX[1], WldPtY[1], WldPtZ[1]});
   const MIL_DOUBLE Length2 = Distance(Vec3 {WldPtX[2], WldPtY[2], WldPtZ[2]},
                                       Vec3 {WldPtX[3], WldPtY[3], WldPtZ[3]});

   // Draw measured features.
   MgraClear(MilGraphics, MilGraList);
   DrawPoints(MilGraphics, MilGraList, CamImgSizeX, PixelsX, PixelsY);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   MgraColor(MilGraphics, M_COLOR_RED);
   MgraLine(MilGraphics, MilGraList, PixelsX[0][0], PixelsY[0][0],
            PixelsX[0][1], PixelsY[0][1]);
   MgraLine(MilGraphics, MilGraList, PixelsX[1][0] + CamImgSizeX, PixelsY[1][0],
            PixelsX[1][1] + CamImgSizeX, PixelsY[1][1]);
   MgraColor(MilGraphics, M_COLOR_BLUE);
   MgraLine(MilGraphics, MilGraList, PixelsX[0][2], PixelsY[0][2],
            PixelsX[0][3], PixelsY[0][3]);
   MgraLine(MilGraphics, MilGraList, PixelsX[1][2] + CamImgSizeX, PixelsY[1][2],
            PixelsX[1][3] + CamImgSizeX, PixelsY[1][3]);
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

   MosPrintf(MIL_TEXT("The points displayed in yellow are converted to 3D world units.\n"));
   MosPrintf(MIL_TEXT("Length of first feature (in red) is %.1f mm.\n"), Length1);
   MosPrintf(MIL_TEXT("Length of second feature (in blue) is %.1f mm.\n\n"), Length2);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();
 
   // MIL tear-down.
   for(MIL_INT CamIdx = 0; CamIdx < NUM_CAMS; ++CamIdx)
      {
      MbufFree(GridImgs[CamIdx]);
      MbufFree(ObjectImgs[CamIdx]);
      MbufFree(RectifiedImgs[CamIdx]);
      McalFree(MilCalibration[CamIdx]);
      MbufFree(DisplayChildImgs[CamIdx]);
      }
   MbufFree(DisplayParentImg);
   MgraFree(MilGraList);
   MdispFree(MilDisplay);
   MgraFree(MilGraphics);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   }


//---------------------------------------------------------------------------
// StereoRectifyPreprocess:
// Prepare for stereo rectification:
// - Set relative coordinate system to rectification plane.
// - Allocate destination images.
// - Calibrate destination to set the relative coordinate system and world 
//   extents correctly.
//---------------------------------------------------------------------------
StereoParams StereoRectifyPreprocess(MIL_ID  LeftCalId, MIL_ID  RightCalId, MIL_INT ImgSizeX,
                                     MIL_INT ImgSizeY, MIL_ID* LeftRectifiedImgIdPtr,
                                     MIL_ID* RightRectifiedImgIdPtr)
   {
   // Use calibration information to set the relative coordinate system of both camera
   // calibrations where we want to rectify the images.
   const MIL_DOUBLE NormBaseline = SetRectifiedPlane(LeftCalId, RightCalId);

   // Compute the pixel size, in the new relative Z=0 plane, for the rectified images.
   const MIL_DOUBLE PixelSize = ComputePixelSize(LeftCalId, RightCalId);

   // Compute the world bounding box, on the rectified plane, for the rectified images.
   Box LeftBBox = ComputeBBox(LeftCalId, ImgSizeX, ImgSizeY);
   Box RightBBox = ComputeBBox(RightCalId, ImgSizeX, ImgSizeY);
   AdjustBoxes(&LeftBBox, &RightBBox, NormBaseline);

   MIL_ID SysId;
   MobjInquire(LeftCalId, M_OWNER_SYSTEM, &SysId);

   // Allocate and calibrate destination images for the rectification.
   MIL_DOUBLE LeftWorldOffsetX = AllocateAndCalibrateRectifiedImage(SysId, LeftBBox, PixelSize, 
                                                                    LeftRectifiedImgIdPtr);
   MIL_DOUBLE RightWorldOffsetX = AllocateAndCalibrateRectifiedImage(SysId, RightBBox, PixelSize, 
                                                                     RightRectifiedImgIdPtr);

   // Construct the stereo parameters (StereoParams) to convert disparity D in Z distance:
   // Z = Num / (Denom + D).
   return {NormBaseline / PixelSize, 
           (LeftWorldOffsetX - RightWorldOffsetX + NormBaseline) / PixelSize};
   }

//---------------------------------------------------------------------------
// SetRectifiedPlane:  
// Compute the baseline between the two calibrations and the rectification plane 
// to use. Changes the relative coordinate system of both calibrations.
//---------------------------------------------------------------------------
MIL_DOUBLE SetRectifiedPlane(MIL_ID LeftCalId, MIL_ID RightCalId)
   {
   // Get camera principal axes.
   const PrincipalAxis LeftPrincipalAxis = GetPrincipalAxis(LeftCalId);
   const PrincipalAxis RightPrincipalAxis = GetPrincipalAxis(RightCalId);

   // Calculate baseline. Baseline goes from left camera origin to right camera origin.
   const Vec3 Baseline = RightPrincipalAxis.Origin - LeftPrincipalAxis.Origin;
   const MIL_DOUBLE NormBaseline = Norm(Baseline);

   if(NormBaseline == 0.0)
      {
      MosPrintf(MIL_TEXT("Computation of rectification plane failed. Stop execution.\n"));
      MosGetch();
      exit(0);
      }

   // Construct relative coordinate system manually.

   // X axis is along the baseline (don't forget to normalize!).
   const Vec3 RelXAxis = Baseline / NormBaseline;

   // Z axis is the closest vector to both principal axes that is perpendicular to the X axis.
   const Vec3 RelZAxis = ComputeRelativeZAxis(RelXAxis, LeftPrincipalAxis.Direction, 
                                              RightPrincipalAxis.Direction);

   // Y axis is computed using the cross product of Z and X.
   const Vec3 RelYAxis = Cross(RelZAxis, RelXAxis);

   // Set relative Z=0 plane (where images will be corrected) in front of the cameras.
   // The origin is centered on the left camera, then moved 1 unit along the relative Z axis.
   const Vec3 RelOrig = LeftPrincipalAxis.Origin + RelZAxis;

   // Construct the relative-to-absolute matrix with the previous information, then set
   // it to both camera calibration.
   const MIL_ID Rel2AbsMatId = ConstructHMatrix(RelXAxis, RelYAxis, RelZAxis, RelOrig);

   // Set relative coordinate system.
   McalSetCoordinateSystem(LeftCalId, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                           M_HOMOGENEOUS_MATRIX, Rel2AbsMatId, M_DEFAULT, M_DEFAULT, M_DEFAULT, 
                           M_DEFAULT);
   McalSetCoordinateSystem(RightCalId, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                           M_HOMOGENEOUS_MATRIX, Rel2AbsMatId, M_DEFAULT, M_DEFAULT, M_DEFAULT, 
                           M_DEFAULT);

   // Free buffer.
   MbufFree(Rel2AbsMatId);

   return NormBaseline;
   }

//---------------------------------------------------------------------------
// GetPrincipalAxis:
// Transform points (0, 0, 0) and (0, 0, 1) in the camera coordinate system (the 
// principal axis) to the absolute coordinate system.
//---------------------------------------------------------------------------
PrincipalAxis GetPrincipalAxis(MIL_ID CalId)
   {
   MIL_DOUBLE ZAxisExtremesX[2] = {0.0, 0.0};
   MIL_DOUBLE ZAxisExtremesY[2] = {0.0, 0.0};
   MIL_DOUBLE ZAxisExtremesZ[2] = {0.0, 1.0};

   McalTransformCoordinate3dList(CalId, M_CAMERA_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM, 2,
                                 ZAxisExtremesX, ZAxisExtremesY, ZAxisExtremesZ,
                                 ZAxisExtremesX, ZAxisExtremesY, ZAxisExtremesZ,
                                 M_DEFAULT);

   PrincipalAxis Axis;
   Axis.Origin = {ZAxisExtremesX[0], ZAxisExtremesY[0], ZAxisExtremesZ[0]};
   Axis.Direction = Vec3 {ZAxisExtremesX[1], ZAxisExtremesY[1], ZAxisExtremesZ[1]} - Axis.Origin;
   return Axis;
   }

//---------------------------------------------------------------------------
// ComputeRelativeZAxis:
// Given a vector along the baseline (RelXAxis) and the two camera's principal 
// axes, compute a virtual principal axis that is close to the real ones, 
// but is perpendicular to the baseline.
//---------------------------------------------------------------------------
 Vec3 ComputeRelativeZAxis(const Vec3& RelXAxis, const Vec3& LeftZAxis, const Vec3& RightZAxis)

   {
   // Make Z axes orthogonal to baseline (RelXAxis) by removing their projection onto it.
   const Vec3 OrthoLeftZAxis = LeftZAxis - ProjectUnit(LeftZAxis, RelXAxis);
   const Vec3 OrthoRightZAxis = RightZAxis - ProjectUnit(RightZAxis, RelXAxis);

   // Average the two orthogonalized axes.
   const Vec3 AvgZDir = 0.5 * (OrthoLeftZAxis + OrthoRightZAxis);
   const MIL_DOUBLE NormAvgZDir = Norm(AvgZDir);
   if(NormAvgZDir == 0.0) 
      {
      MosPrintf(MIL_TEXT("Computation of relative z axis failed. Stop execution.\n"));
      MosGetch();
      exit(0);
      }

   // Normalize before return.
   return AvgZDir / NormAvgZDir;
   }

///---------------------------------------------------------------------------**
// ConstructHMatrix:
// Construct a 4x4 homogeneous transformation matrix from the XYZ axes and the 
// origin.
//---------------------------------------------------------------------------***
MIL_ID ConstructHMatrix(const Vec3& RelXAxis, const Vec3& RelYAxis, const Vec3& RelZAxis,
                           const Vec3& RelOrig)
   {
   // Construct homogeneous transformation matrix for relative coordinate system.
   const MIL_FLOAT HMatData[] =
      {
      static_cast<MIL_FLOAT>(RelXAxis.x), static_cast<MIL_FLOAT>(RelYAxis.x), 
      static_cast<MIL_FLOAT>(RelZAxis.x), static_cast<MIL_FLOAT>(RelOrig.x),
      static_cast<MIL_FLOAT>(RelXAxis.y), static_cast<MIL_FLOAT>(RelYAxis.y), 
      static_cast<MIL_FLOAT>(RelZAxis.y), static_cast<MIL_FLOAT>(RelOrig.y),
      static_cast<MIL_FLOAT>(RelXAxis.z), static_cast<MIL_FLOAT>(RelYAxis.z), 
      static_cast<MIL_FLOAT>(RelZAxis.z), static_cast<MIL_FLOAT>(RelOrig.z),
      0.0f, 0.0f, 0.0f, 1.0f
      };
   MIL_ID HMatId = MbufAlloc2d(M_DEFAULT_HOST, 4, 4, 32 + M_FLOAT, M_ARRAY, M_NULL);
   MbufPut(HMatId, HMatData);
   return HMatId;
   }

//---------------------------------------------------------------------------
// ComputePixelSize:
// In the images, the image plane is at a distance of "focal length", in pixels, 
// and the size of a pixel is 1. In the world relative coordinate system, we 
// placed the Z=0 plane at a distance of 1 world unit, and we want to know the
// size of one pixel, in world units, at that distance (use similar triangles).
//---------------------------------------------------------------------------
MIL_DOUBLE ComputePixelSize(MIL_ID LeftCalId, MIL_ID RightCalId)
   {
   // Inquire focal lenght.
   MIL_DOUBLE LeftFocal, RightFocal;
   McalInquire(LeftCalId, M_FOCAL_LENGTH, &LeftFocal);
   McalInquire(RightCalId, M_FOCAL_LENGTH, &RightFocal);
   MIL_DOUBLE AvgFocal = 0.5 * (LeftFocal + RightFocal);
   if(AvgFocal == 0.0)
      {
      MosPrintf(MIL_TEXT("Computation of pixel size failed. Stop execution.\n"));
      MosGetch();
      exit(0);
      }
   return 1.0 / AvgFocal;
   }

//---------------------------------------------------------------------------
// ComputeBBox:
// Compute images bounding box.
// Note: This is an approximate bounding box. If lens distortion is important, 
// a better algorithm may be needed here.
//---------------------------------------------------------------------------
Box ComputeBBox(MIL_ID  CalId, MIL_INT ImgSizeX, MIL_INT ImgSizeY)
   {
   // Transform the image four corners into world points on the relative Z=0 plane, and
   // compute the 2D axis-aligned bounding box of the points.
   const MIL_INT NUM_POINTS = 4;
   MIL_DOUBLE X[NUM_POINTS] = {-0.5, ImgSizeX - 0.5, ImgSizeX - 0.5,          -0.5};
   MIL_DOUBLE Y[NUM_POINTS] = {-0.5,          -0.5, ImgSizeY - 0.5, ImgSizeY - 0.5};
   McalTransformCoordinateList(CalId, M_PIXEL_TO_WORLD + M_NO_POINTS_BEHIND_CAMERA,
                               NUM_POINTS, X, Y, X, Y);

   // Check that all points were converted correctly. If camera angle is too large, some
   // corners may never intersect the relative Z=0 plane. In a stereo application, it
   // should not happen since the cameras should be roughly perpendicular to the
   // relative Z=0 plane.
   for(MIL_INT PtIdx = 0; PtIdx < NUM_POINTS; ++PtIdx)
      {
      if(X[PtIdx] == M_INVALID_POINT || Y[PtIdx] == M_INVALID_POINT) 
         {
         MosPrintf(MIL_TEXT("Conversion of the four corners to world units failed:\n"));
         MosPrintf(MIL_TEXT("No intersection found.\n"));
         MosPrintf(MIL_TEXT("Stop execution.\n"));
         MosGetch();
         exit(0);
         }
      }

   const auto ExtremeX = std::minmax_element(std::begin(X), std::end(X));
   const auto ExtremeY = std::minmax_element(std::begin(Y), std::end(Y));

   return {*ExtremeX.first, *ExtremeY.first, *ExtremeX.second, *ExtremeY.second};
   }

//---------------------------------------------------------------------------
// AdjustBoxes:
// Note: This is an approximate bounding box. If lens distortion is important, 
// a better algorithm may be needed here.
//---------------------------------------------------------------------------
void AdjustBoxes(Box* pLeftBBox, Box* pRightBBox, MIL_DOUBLE /*NormBaseline*/)
   {
   // Change the bounding boxes to keep only the common rows, and to align the rows.
   const MIL_DOUBLE MinY = std::max(pLeftBBox->y1, pRightBBox->y1);
   const MIL_DOUBLE MaxY = std::min(pLeftBBox->y2, pRightBBox->y2);
   if(MinY > MaxY) 
      {
      MosPrintf(MIL_TEXT("Bounding box approximation failed. Stop execution.\n"));
      MosGetch();
      exit(0);
      } 
   pLeftBBox->y1 = pRightBBox->y1 = MinY;
   pLeftBBox->y2 = pRightBBox->y2 = MaxY;
   }

//---------------------------------------------------------------------------
// AllocateAndCalibrateRectifiedImage:
// Allocate and calibrate an image so that its corners map to the given world 
// box, given a pixel size. Returns WorldOffsetX.
//---------------------------------------------------------------------------
MIL_DOUBLE AllocateAndCalibrateRectifiedImage(MIL_ID SysId, const Box& BBox, MIL_DOUBLE PixelSize,
                                              MIL_ID* RectifiedImageIdPtr)
   {
   // Determine image size.
   const MIL_DOUBLE BoxSizeX = BBox.x2 - BBox.x1;
   const MIL_DOUBLE BoxSizeY = BBox.y2 - BBox.y1;
   const MIL_INT RectifiedSizeX = static_cast<MIL_INT>(std::ceil(BoxSizeX / PixelSize));
   const MIL_INT RectifiedSizeY = static_cast<MIL_INT>(std::ceil(BoxSizeY / PixelSize));

   // Determine world offset, which is pixel (0, 0). Add 0.5 because 0 is the pixel center.
   const MIL_DOUBLE WorldOffsetX = BBox.x1 + 0.5*PixelSize;
   const MIL_DOUBLE WorldOffsetY = BBox.y1 + 0.5*PixelSize;

   MbufAlloc2d(SysId, RectifiedSizeX, RectifiedSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, 
               RectifiedImageIdPtr);
   McalUniform(*RectifiedImageIdPtr, WorldOffsetX, WorldOffsetY, PixelSize, PixelSize, 0.0, 
               M_DEFAULT);

   return WorldOffsetX;
   }


//---------------------------------------------------------------------------
// DrawPoints:
// Draw feature points on displayed images.
//---------------------------------------------------------------------------
void DrawPoints(MIL_ID MilGraphics, MIL_ID MilGraList, MIL_INT XOffset, 
                const MIL_DOUBLE PixelsX[NUM_CAMS][NUM_POINTS],
                 const MIL_DOUBLE PixelsY[NUM_CAMS][NUM_POINTS])
   {
   for(MIL_INT CamIdx = 0; CamIdx < NUM_CAMS; ++CamIdx)
      {
      for(MIL_INT PtIdx = 0; PtIdx < NUM_POINTS; ++PtIdx)
         {
         const MIL_DOUBLE Dim = 6.0;
         const MIL_DOUBLE Px = PixelsX[CamIdx][PtIdx];
         const MIL_DOUBLE Py = PixelsY[CamIdx][PtIdx];
         MgraRect(MilGraphics, MilGraList, Px - Dim + CamIdx * XOffset, Py - Dim,
                  Px + Dim + CamIdx * XOffset, Py + Dim);
         }
      }
   }

//---------------------------------------------------------------------------
// DrawLines:
// Draw epipolar lines on displayed images.
//---------------------------------------------------------------------------
void DrawLines(MIL_ID MilGraphics, MIL_ID MilGraList, MIL_INT XOffset, 
               const MIL_DOUBLE PixelsX[NUM_CAMS][NUM_POINTS],
               const MIL_DOUBLE PixelsY[NUM_CAMS][NUM_POINTS])
   {
   for(MIL_INT PtIdx = 0; PtIdx < NUM_POINTS; ++PtIdx)
      {
      MgraLine(MilGraphics, MilGraList, PixelsX[0][PtIdx], PixelsY[0][PtIdx],
               PixelsX[1][PtIdx] + XOffset, PixelsY[1][PtIdx]);
      }
   }
