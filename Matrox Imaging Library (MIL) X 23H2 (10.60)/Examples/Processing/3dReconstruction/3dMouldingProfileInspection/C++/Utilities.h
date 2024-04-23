//******************************************************************************
// 
// File name: Utilities.h
//
// Synopsis:  This file holds the utility functions used by the 3D Moulding
//            Profile Inspection program.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//******************************************************************************

#ifndef __UTILITIES_H
#define __UTILITIES_H

#include <mil.h>

//******************************************************************************
// Constants.
//******************************************************************************
static const MIL_UINT DISP3D_SIZE_X = 500;
static const MIL_UINT DISP3D_SIZE_Y = 500;
static const MIL_UINT DISP2D_SIZE_X = 600;
static const MIL_UINT DISP2D_SIZE_Y = 600;


//******************************************************************************
// Utility structures.
//******************************************************************************
/**
* 3D point structure.
*/
template <typename NumType>
struct Point3D
   {
   NumType x;
   NumType y;
   NumType z;
   };

/**
* 3D vector structure.
*/
template <typename NumType>
using Vector3D = Point3D<NumType>;

/**
* 2 vectors of x- and y-coordinates.
*/
template <typename NumType>
struct ProfileXY
   {
   std::vector<NumType> x;
   std::vector<NumType> y;
   };

/**
* Structure that holds results of a profile inspection,
* including whether the profile was successfully computed,
* whether the inspection passed, and the area between the curves.
*/
struct InspectionResult
   {
   MIL_INT Status  = M_NULL;
   bool Passed     = false;
   MIL_DOUBLE Area = MIL_DOUBLE_MAX;
   };

/**
* For a profile inspection failure, the structure
* holds the profile plane's location and the area between the curves.
*/
struct FailedResult
   {
   MIL_DOUBLE Position;
   MIL_DOUBLE Area;
   };

/**
 * Structure of a scanned object for which a profile is obtained.
 */
struct ProfileObject
{
   MIL_ID     Id;                               // MIL ID of the object.
   MIL_DOUBLE Length;                           // Length of the object.
   MIL_ID     SlicingPlaneTransformationMatrix; // Transformation matrix defining the profile plane.
   MIL_DOUBLE SamplingDistance;                 // Distance at which to sample the profile.
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
// Draws the slicing plane.
//*******************************************************************************
MIL_INT64 DrawSlicingPlane(MIL_ID MilSystem, MIL_ID MilGraphicList, MIL_ID MilProfileResult,
                           const MIL_DOUBLE PlaneSize)
   {
   // Retrieve the profile plane to world transformation matrix.
   auto PlaneToWorldTransMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX,
                                              M_DEFAULT, M_UNIQUE_ID);
   M3dimCopyResult(MilProfileResult, PlaneToWorldTransMatrix,
                   M_MATRIX_PROFILE_PLANE_TO_WORLD, M_DEFAULT);

   // Construct the profile plane and display it.
   Point3D<MIL_DOUBLE> PlanePoint;
   M3dgeoMatrixGetTransform(PlaneToWorldTransMatrix, M_TRANSLATION,
                            &PlanePoint.x, &PlanePoint.y, &PlanePoint.z,
                            M_NULL, M_DEFAULT);
   Vector3D<MIL_DOUBLE> PlaneNormal;
   M3dgeoMatrixGetTransform(PlaneToWorldTransMatrix, M_ROTATION_AXIS_Z,
                            &PlaneNormal.x, &PlaneNormal.y, &PlaneNormal.z,
                            M_NULL, M_DEFAULT);

   auto PlaneLabel = M3dgraPlane(MilGraphicList, M_DEFAULT, M_POINT_AND_NORMAL,
                                 PlanePoint.x, PlanePoint.y, PlanePoint.z,
                                 PlaneNormal.x, PlaneNormal.y, PlaneNormal.z,
                                 M_DEFAULT, M_DEFAULT, M_DEFAULT, PlaneSize, M_DEFAULT);

   M3dgraControl(MilGraphicList, PlaneLabel, M_OPACITY, 80);
   M3dgraControl(MilGraphicList, PlaneLabel, M_COLOR, M_COLOR_YELLOW);

   return PlaneLabel;
   }

//*******************************************************************************
// Calibrates the profile image.
//*******************************************************************************
void CalibrateProfileImage(MIL_ID MilProfileImage, const ProfileXY<MIL_DOUBLE>& ProfilePoints)
   {
   // Bounding box of profile points
   const auto BoundsX = std::minmax_element(std::begin(ProfilePoints.x),
                                            std::end(ProfilePoints.x));
   const auto BoundsY = std::minmax_element(std::begin(ProfilePoints.y),
                                            std::end(ProfilePoints.y));
   const MIL_DOUBLE Dx = BoundsX.second[0] - BoundsX.first[0]; 
   const MIL_DOUBLE Dy = BoundsY.second[0] - BoundsY.first[0]; 
   const MIL_DOUBLE BoxSize = 1.3*sqrt((Dx*Dx + Dy*Dy));

   // Calibrate the profile image
   const MIL_DOUBLE X0 = -0.5*(BoxSize) + BoundsX.first[0] + 0.5*Dx;
   const MIL_DOUBLE Y0 = -0.5*(BoxSize) + BoundsY.first[0] + 0.5*Dy;
   const MIL_DOUBLE PixelSizeX = BoxSize/DISP2D_SIZE_X;
   const MIL_DOUBLE PixelSizeY = BoxSize/DISP2D_SIZE_Y;
   const MIL_DOUBLE Rotation = 0.0;
   McalUniform(MilProfileImage, X0, Y0,  PixelSizeX, PixelSizeY, Rotation, M_DEFAULT);
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
   }

//*******************************************************************************
// Allocates a color image and calibrates it.
//*******************************************************************************
MIL_UNIQUE_BUF_ID GetProfileImage(MIL_ID MilSystem, const ProfileXY<MIL_DOUBLE>& ProfilePoints)
   {
   const MIL_INT NbBands = 3;
   auto MilProfileImage = MbufAllocColor(MilSystem, NbBands, DISP2D_SIZE_X, DISP2D_SIZE_Y,
                                         8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC,
                                         M_UNIQUE_ID);
   MbufClear(MilProfileImage, M_COLOR_BLACK);

   // Calibrate the profile image.
   CalibrateProfileImage(MilProfileImage, ProfilePoints);

   return MilProfileImage;
   }

//*******************************************************************************
// Checks if the file exists.
//*******************************************************************************
void CheckIfFileIsPresent(MIL_STRING FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT,
      &FilePresent);

   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The file needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      WaitForKey();
      
      exit(EXIT_FAILURE);
      }
   }

//*******************************************************************************
// Restores buffer file.
//*******************************************************************************
MIL_UNIQUE_BUF_ID RestoreFile(MIL_ID MilSystem, MIL_STRING FileName)
   {
   CheckIfFileIsPresent(FileName);

   auto Milbuf = MbufRestore(FileName, MilSystem, M_UNIQUE_ID);
   return Milbuf;
   }

//*******************************************************************************
// Restores a 3D geometry file.
//*******************************************************************************
MIL_UNIQUE_3DGEO_ID RestoreGeometry(MIL_ID MilSystem, MIL_STRING FileName)
   {
   CheckIfFileIsPresent(FileName);

   MIL_UNIQUE_3DGEO_ID MilGeo = M3dgeoRestore(FileName, MilSystem, M_DEFAULT, M_UNIQUE_ID);
   return MilGeo;
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
// Allocates a 3D display, if possible, and displays a point cloud or
// 3D geometry object. 
//*******************************************************************************
MIL_UNIQUE_3DDISP_ID Display3dObject(MIL_ID MilSystem, MIL_ID MilObject,
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

   const auto ObjectType = MobjInquire(MilObject, M_OBJECT_TYPE, M_NULL);

   switch (ObjectType)
      {
      case M_3DGEO_GEOMETRY:
         {
         auto MilGraList = M3ddispInquire(Mil3dDisp, M_3D_GRAPHIC_LIST_ID, M_NULL);
         M3dgeoDraw3d(M_DEFAULT, MilObject, MilGraList, M_DEFAULT, M_DEFAULT);
         }
         break;
      case M_CONTAINER:
         {
         M3ddispSelect(Mil3dDisp, MilObject, M_SELECT, M_DEFAULT);
         }
         break;
      default:
         {
         MosPrintf(MIL_TEXT("Only 3D geometry and container object types")
            MIL_TEXT(" are supported.\n"));
         exit(EXIT_FAILURE);
         }
         break;
      }

   return Mil3dDisp;
   }

//*******************************************************************************
// Allocates a 2D display and displays an image. 
//*******************************************************************************
MIL_UNIQUE_DISP_ID Display2dImage(MIL_ID MilSystem, MIL_ID MilBuf,
                                  MIL_INT PositionX, MIL_INT PositionY,
                                  const MIL_STRING& Title)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Mil2dDisp = MdispAlloc(MilSystem, M_DEFAULT,
                               MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   MdispControl(Mil2dDisp, M_TITLE, Title);
   MdispControl(Mil2dDisp, M_WINDOW_INITIAL_POSITION_X, PositionX);
   MdispControl(Mil2dDisp, M_WINDOW_INITIAL_POSITION_Y, PositionY);

   MdispSelect(Mil2dDisp, MilBuf);

   return Mil2dDisp;
   }

#endif
