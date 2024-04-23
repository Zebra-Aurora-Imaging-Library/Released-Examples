//***************************************************************************************/
//
// File name: StudInspection.cpp
//
// Synopsis:  This program contains an example where studs are inspected using the
//            mil3dmet module.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>

#define RAD_TO_DEG (180.0 / 3.14159265358979)

// Source file specification.
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("StudInspection/StudConnection.mbufc");
static const MIL_STRING ILLUSTRATION_FILE = M_IMAGE_PATH MIL_TEXT("StudInspection/StudInspectionIllustration.png");
static const MIL_INT ILLUSTRATION_OFFSET_X = 800;

// Number of studs.
static const MIL_INT NUMBER_OF_STUDS = 4;

// Tolerance for the plane fits.
static const MIL_DOUBLE PLANE_TOLERANCE = 1.0;

// Tolerance for the cylinder fits.
static const MIL_DOUBLE CYLINDER_TOLERANCE = 0.5;

// Values used for validation.
static const MIL_DOUBLE EXPECTED_RADIUS  =  4.5; // in mm
static const MIL_DOUBLE EXPECTED_HEIGHT  = 22.0;
static const MIL_DOUBLE HEIGHT_TOLERANCE =  1.0;
static const MIL_DOUBLE RADIUS_TOLERANCE =  1.0;

static const MIL_DOUBLE EXPECTED_ANGLE   = 90.0; // in degrees
static const MIL_DOUBLE ANGLE_TOLERANCE  =  5.0;

// Function declarations.
bool   CheckForRequiredMILFile(const MIL_STRING& FileName);
void   InspectStuds(MIL_ID MilSystem);
MIL_ID Alloc3dDisplayId(MIL_ID MilSystem);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("StudInspection\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to inspect cylindrical studs.\n")
             MIL_TEXT("A cylinder is fit on each stud. Its height, radius and \n")
             MIL_TEXT("angle, along with the planar surface, are used to detect defects.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Geometry, 3D Metrology, 3D Image Processing,\n")
             MIL_TEXT("Blob, 3D Display, Display, Buffer, Graphics, and 3D Graphics.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   // Check for required example files.
   if(!CheckForRequiredMILFile(PT_CLD_FILE))
      {
      MappFree(MilApplication);
      return 0;
      }
     
   MIL_ID MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);

   // Show illustration of light orientations.
   MIL_ID IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_FILE, MilSystem, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Object to inspect."));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   InspectStuds(MilSystem);

   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   MsysFree(MilSystem);
   MappFree(MilApplication);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }

//*****************************************************************************
// Main processing function.
//*****************************************************************************
void InspectStuds(MIL_ID MilSystem)
   {
   // Restore the point cloud.
   MIL_UNIQUE_BUF_ID MilPointCloud    = MbufImport(PT_CLD_FILE, M_MIL_NATIVE, M_RESTORE, MilSystem, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID MilCroppedCloud  = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID MilCylinderCloud = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
  
   // Allocate the display.
   MIL_ID Mil3dDisplay = Alloc3dDisplayId(MilSystem);
   MosPrintf(MIL_TEXT("A 3D point cloud is restored from an MBUFC file and displayed.\n\n"));

   MIL_ID MilGraphicList = M_NULL;
   M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, &MilGraphicList);
   MIL_ID ReflectanceBuffer = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);

   M3ddispSetView(Mil3dDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Display the point cloud.
   MIL_INT64 PointCloudGraphics = M3ddispSelect(Mil3dDisplay, MilPointCloud, M_SELECT, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fit and display a plane on the background.
   MIL_UNIQUE_3DMET_ID FitContext = M3dmetAlloc(MilSystem, M_FIT_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_3DMET_ID MilFitResult = M3dmetAllocResult(MilSystem, M_FIT_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dmetControl(FitContext, M_EXPECTED_OUTLIER_PERCENTAGE, 50);
   M3dmetFit   (FitContext, MilPointCloud, M_PLANE, MilFitResult, PLANE_TOLERANCE, M_DEFAULT);
   M3dmetDraw3d(M_DEFAULT, MilFitResult, MilGraphicList, M_ROOT_NODE, M_DEFAULT);

   // Get the plane normal vector.
   MIL_DOUBLE PlaneNx, PlaneNy, PlaneNz;
   M3dmetGetResult(MilFitResult, M_NORMAL_X, &PlaneNx);
   M3dmetGetResult(MilFitResult, M_NORMAL_Y, &PlaneNy);
   M3dmetGetResult(MilFitResult, M_NORMAL_Z, &PlaneNz);

   MosPrintf(MIL_TEXT("A plane is fit on the background.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Create a copy of the point cloud so we can crop without affecting the display.
   MbufCopyComponent(MilPointCloud, MilCroppedCloud, M_COMPONENT_ALL, M_REPLACE, M_DEFAULT);
   MIL_ID RangeBuffer = MbufInquireContainer(MilCroppedCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   MIL_ID ConfidenceBuffer = MbufInquireContainer(MilCroppedCloud, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);

   // Remove the points on the plane.
   M3dmetCopyResult(MilFitResult, ConfidenceBuffer, M_OUTLIER_MASK, M_DEFAULT);

   // Find the studs by doing a blob analysis on the confidence.
   MIL_UNIQUE_BLOB_ID MilBlobContext = MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BLOB_ID MilBlobResult = MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MblobControl(MilBlobContext, M_SORT1, M_AREA);
   MblobControl(MilBlobContext, M_SORT1_DIRECTION, M_SORT_DOWN);
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobCalculate(MilBlobContext, ConfidenceBuffer, M_NULL, MilBlobResult);

   MosPrintf(MIL_TEXT("Blob analysis is performed on the points above the plane. \n"));

   // Find the number of blobs.
   MIL_INT NbBlobs;
   MblobGetResult(MilBlobResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbBlobs);
   if(NbBlobs == 0)
      {
      MosPrintf(MIL_TEXT("No blobs were found. \n"));
      }
   else
      {
      // Select up to 4 studs.
      if(NbBlobs > NUMBER_OF_STUDS)
         { NbBlobs = NUMBER_OF_STUDS; }
      MblobSelect(MilBlobResult, M_DELETE, M_INDEX_VALUE, M_GREATER_OR_EQUAL, (MIL_DOUBLE)NbBlobs, M_NULL);

      // Color the points on the studs.
      MIL_INT SizeX = MbufInquire(ReflectanceBuffer, M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(ReflectanceBuffer, M_SIZE_Y, M_NULL);
      MIL_UNIQUE_BUF_ID BlobLabels = MbufAlloc2d(MilSystem, SizeX, SizeY, M_UNSIGNED + 8, M_IMAGE + M_PROC, M_UNIQUE_ID);
      MIL_UNIQUE_BUF_ID CylinderColors = MbufAllocColor(MilSystem, 3, 256, 1, M_UNSIGNED + 8, M_LUT, M_UNIQUE_ID);
      MgenLutFunction(CylinderColors, M_COLORMAP_DISTINCT_256, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MblobLabel(MilBlobResult, BlobLabels, M_CLEAR);
      MbufSetRegion(BlobLabels, BlobLabels, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MimLutMap(BlobLabels, ReflectanceBuffer, CylinderColors);

      MosPrintf(MIL_TEXT("The largest blobs are used to locate up to %i studs. \n"), NUMBER_OF_STUDS);
      MosPrintf(MIL_TEXT("For each blob, the nearby points are cropped and a cylinder \n"));
      MosPrintf(MIL_TEXT("is fit on them. The cylinder's radius, height, and angle \n"));
      MosPrintf(MIL_TEXT("with respect to the plane are used to verify the stud.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to go from one stud to the next.\n\n"));
      MosGetch();

      MosPrintf(MIL_TEXT("Expected radius: %4.1f +/-%4.1f mm\n"), EXPECTED_RADIUS, RADIUS_TOLERANCE);
      MosPrintf(MIL_TEXT("Expected height: %4.1f +/-%4.1f mm\n"), EXPECTED_HEIGHT, HEIGHT_TOLERANCE);
      MosPrintf(MIL_TEXT("Expected angle:  %4.1f +/-%4.1f deg\n\n"), EXPECTED_ANGLE, ANGLE_TOLERANCE);
      MosPrintf(MIL_TEXT("Index   Center (X, Y, Z)     Radius  Height  Angle    Status\n"));
      MosPrintf(MIL_TEXT("-----------------------------------------------------------------\n"));

      // Analyze each stud separately.
      for(MIL_INT i = 0; i < NbBlobs; i++)
         {
         // Find the center of the blob.
         MIL_INT BlobCenterX, BlobCenterY;
         MblobGetResult(MilBlobResult, M_BLOB_INDEX(i), M_CENTER_OF_GRAVITY_X + M_TYPE_MIL_INT, &BlobCenterX);
         MblobGetResult(MilBlobResult, M_BLOB_INDEX(i), M_CENTER_OF_GRAVITY_Y + M_TYPE_MIL_INT, &BlobCenterY);

         // Convert the center from pixel coordinates to world coordinates by looking at the range.
         MIL_FLOAT CenterX, CenterY, CenterZ;
         MbufGetColor2d(RangeBuffer, M_SINGLE_BAND, 0, BlobCenterX, BlobCenterY, 1, 1, &CenterX);
         MbufGetColor2d(RangeBuffer, M_SINGLE_BAND, 1, BlobCenterX, BlobCenterY, 1, 1, &CenterY);
         MbufGetColor2d(RangeBuffer, M_SINGLE_BAND, 2, BlobCenterX, BlobCenterY, 1, 1, &CenterZ);

         // Creater a bounding cylinder centered around the blob and perpedicular to the plane.
         MIL_UNIQUE_3DGEO_ID MilBoundingCylinder = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
         M3dgeoCylinder(MilBoundingCylinder, M_POINT_AND_VECTOR, CenterX, CenterY, CenterZ, PlaneNx, PlaneNy, PlaneNz, EXPECTED_RADIUS * 2, M_INFINITE,M_DEFAULT);

         // Crop the points inside.
         M3dimCrop(MilCroppedCloud, MilCylinderCloud, MilBoundingCylinder, M_NULL, M_UNORGANIZED, M_DEFAULT);

         // Fit a cylinder on the stud.
         M3dmetFit(FitContext, MilCylinderCloud, M_CYLINDER, MilFitResult, CYLINDER_TOLERANCE, M_DEFAULT);

         // Get the cylinder's parameters and print it.
         MIL_DOUBLE Radius, Height, StartX, StartY, StartZ, AxisX, AxisY, AxisZ;
         M3dmetGetResult(MilFitResult, M_RADIUS, &Radius);
         M3dmetGetResult(MilFitResult, M_LENGTH, &Height);
         M3dmetGetResult(MilFitResult, M_START_POINT_X, &StartX);
         M3dmetGetResult(MilFitResult, M_START_POINT_Y, &StartY);
         M3dmetGetResult(MilFitResult, M_START_POINT_Z, &StartZ);
         M3dmetGetResult(MilFitResult, M_AXIS_X, &AxisX);
         M3dmetGetResult(MilFitResult, M_AXIS_Y, &AxisY);
         M3dmetGetResult(MilFitResult, M_AXIS_Z, &AxisZ);

         MIL_DOUBLE Angle = RAD_TO_DEG * acos(AxisX * PlaneNx + AxisY * PlaneNy + AxisZ * PlaneNz);
         if(Angle > 90.0)
            { Angle = 180.0 - Angle; }
         Angle = 90.0 - Angle;

         MosPrintf(MIL_TEXT("  %i   (%5.1f, %5.1f, %4.1f)   %4.1f    %4.1f    %4.1f    "),
                   i, StartX, StartY, StartZ, Radius, Height, Angle);

         // Verify the stud.
         bool HasFailed = true;
         if(abs(Angle - EXPECTED_ANGLE) > ANGLE_TOLERANCE)
            {
            MosPrintf(MIL_TEXT("FAIL: incorrect angle\n"));
            }
         else if(abs(Height - EXPECTED_HEIGHT) > HEIGHT_TOLERANCE)
            {
            MosPrintf(MIL_TEXT("FAIL: incorrect height\n"));
            }
         else if(abs(Radius - EXPECTED_RADIUS) > RADIUS_TOLERANCE)
            {
            MosPrintf(MIL_TEXT("FAIL: incorrect radius\n"));
            }
         else
            {
            MosPrintf(MIL_TEXT(" OK \n"));
            HasFailed = false;
            }

         // Display the fitted cylinder.
         MIL_INT64 CylinderLabel = M3dmetDraw3d(M_DEFAULT, MilFitResult, MilGraphicList, M_ROOT_NODE, M_DEFAULT);
         M3dgraControl(MilGraphicList, CylinderLabel, M_OPACITY+M_RECURSIVE, 75);
         M3dgraControl(MilGraphicList, CylinderLabel, M_COLOR + M_RECURSIVE, HasFailed ? M_COLOR_RED : M_COLOR_GREEN);

         MosGetch();
         }
      }

      M3dgraControl(MilGraphicList, PointCloudGraphics, M_OPACITY, 0);
      MosGetch();

      if(Mil3dDisplay)
         { M3ddispFree(Mil3dDisplay); }
   }

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
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
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return MilDisplay3D;
   }
