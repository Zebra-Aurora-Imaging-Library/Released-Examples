//***************************************************************************************
// 
// File name: AlignLaserScans.cpp  
//
// Synopsis: This program contains an example showing how to align and fixture
//           laser scans represented as 3D point clouds.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************
#include <mil.h>

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING FILENAME = M_IMAGE_PATH MIL_TEXT("AlignLaserScans/ScannedDisk.mbufc");

static const MIL_STRING SCAN_ILLUSTRATION_FILENAME =
                        M_IMAGE_PATH MIL_TEXT("AlignLaserScans/ScanDisk.png");

static const MIL_STRING CORR_ILLUSTRATION_FILENAME =
                        M_IMAGE_PATH MIL_TEXT("AlignLaserScans/ScanCorrections.png");

static const MIL_INT NUM_DISPLAY = 2;

static const MIL_INT DISPLAY_SIZE_X = 320;
static const MIL_INT DISPLAY_SIZE_Y = 320;

// Enumerators definitions.
enum { eSide = 0, eBottom = 1 };

//****************************************************************************
// Function Declaration.
//****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName);
void Alloc3dDisplayId(MIL_ID MilSystem, MIL_UNIQUE_3DDISP_ID* MilDisplay3D);
void DisplayContainer(MIL_ID MilContainer, MIL_ID MilDisplay3D, MIL_ID MilGraphicList,
                      const MIL_UNIQUE_DISP_ID MilDisplayArray[2],
                      const MIL_UNIQUE_BUF_ID MilDepthMapArray[2]);
void ShowDifferentViews(MIL_ID MilSystem,
                        MIL_ID MilContainer,
                        const MIL_UNIQUE_DISP_ID MilDisplayArray[2],
                        const MIL_UNIQUE_BUF_ID MilDepthMapArray[2]);
void ShearScaleCorrection(MIL_ID MilSystem, MIL_ID MilSrcContainer, MIL_ID MilDstContainer,
                          MIL_ID MilResult, bool UseBufConvert);
void FullCorrection(MIL_ID FullMatrix, MIL_ID MilSrcContainer,
                    MIL_ID MilDstContainer, MIL_ID MilResult);
void DrawAxis(MIL_ID MilSystem, MIL_ID MilGraList, MIL_INT AxisType);


//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("AlignLaserScans\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to correct scans with a misaligned 3D profile sensor.\n"));
   MosPrintf(MIL_TEXT("It also shows how to perform fixturing. \n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Map, Display, \n")
             MIL_TEXT("3D Geometry, 3D Image Processing, 3D Display, and 3D Graphics. \n\n"));
   }

//****************************************************************************
// Scanning guidelines.
//****************************************************************************
void PrintScanningGuidelines(MIL_ID MilSystem)
   {
   MIL_UNIQUE_DISP_ID IllustrationDispId;
   MIL_UNIQUE_BUF_ID  IllustrationImageId;

   IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Scanning a calibration disk."));
   IllustrationImageId = MbufRestore(SCAN_ILLUSTRATION_FILENAME, MilSystem, M_UNIQUE_ID);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   MosPrintf(MIL_TEXT("Scanning guidelines.\n\n"));
   MosPrintf(MIL_TEXT("1 - The alignment disk must cover at least 50%% of the\n"));
   MosPrintf(MIL_TEXT("    scanned width (X direction).\n"));
   MosPrintf(MIL_TEXT("2 - The alignment disk edge must be fully visible in the scan.\n"));
   MosPrintf(MIL_TEXT("3 - The alignment disk must cover at least 30%% of the\n"));
   MosPrintf(MIL_TEXT("    scanned length (Y direction).\n"));
   MosPrintf(MIL_TEXT("4 - The alignment disk's holes must be at least \n"));
   MosPrintf(MIL_TEXT("    30 scan lines (Y-direction) and 30 points (X-direction).\n"));
   MosPrintf(MIL_TEXT("    The radii of the holes must be within 5 to 10%% of the disk's radius.\n"));
   MosPrintf(MIL_TEXT("    The depth of the holes must be at least 20%% of the total disk's height.\n"));
   MosPrintf(MIL_TEXT("5 - A floor (background plane) must be present in the scan.\n"));
   MosPrintf(MIL_TEXT("6 - Ensure the alignment disk surface is parallel to the motion plane.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   if(!CheckForRequiredMILFile(FILENAME))
      {
      return -1;
      }

   PrintScanningGuidelines(MilSystem);

   MIL_ID MilGraphicList3D = M_NULL;

   MIL_UNIQUE_BUF_ID    MilDisplayImageArray[2]; // Image buffers used for 2D display.
   MIL_UNIQUE_DISP_ID   MilDisplayArray[2];      // 2D Mil Display of the scanned object.
   MIL_UNIQUE_GRA_ID    MilGraListArray[2];      // 2D Graphic lists.
   MIL_UNIQUE_3DDISP_ID MilDisplay3D;            // 3D Mil Display.

   // Allocate MIL objects.
   auto MilContext    = M3dmapAlloc(MilSystem, M_ALIGN_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilResult     = M3dmapAllocResult(MilSystem, M_ALIGN_RESULT, M_DEFAULT, M_UNIQUE_ID);
  
   auto MilContainer  = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   auto MilFullMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   auto MilLut        = MbufAllocColor(MilSystem, 3, 255 + 1, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);

   Alloc3dDisplayId(MilSystem, &MilDisplay3D);
   if(MilDisplay3D)
      {
      MilGraphicList3D = (MIL_ID) M3ddispInquire(MilDisplay3D, M_3D_GRAPHIC_LIST_ID, M_NULL);
      M3dgraControl(MilGraphicList3D, M_DEFAULT_SETTINGS, M_FONT_SIZE, 15);

      // Draw the axis at the origin.
      MIL_INT64 MilAxis = M3dgraAxis(MilGraphicList3D, M_DEFAULT, M_IDENTITY_MATRIX, 120, MIL_TEXT("Origin"), M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicList3D, MilAxis, M_THICKNESS, 3);
      M3ddispControl(MilDisplay3D, M_SIZE_X, DISPLAY_SIZE_X);
      M3ddispControl(MilDisplay3D, M_SIZE_Y, DISPLAY_SIZE_Y);
      M3ddispControl(MilDisplay3D, M_TITLE, MIL_TEXT("Tilted 3D View"));
      M3ddispControl(MilDisplay3D, M_BACKGROUND_COLOR, M_COLOR_BLACK);
      M3ddispControl(MilDisplay3D, M_BACKGROUND_COLOR_GRADIENT, M_COLOR_BLACK);

      MIL_INT64 MilGrid = M3dgraGrid(MilGraphicList3D, M_ROOT_NODE, M_SIZE_AND_SPACING, M_DEFAULT, 250, 500, 25, 25, M_DEFAULT);
      M3dgraControl(MilGraphicList3D, MilGrid, M_APPEARANCE, M_WIREFRAME);
      M3dgraControl(MilGraphicList3D, MilGrid, M_OPACITY, 30);
      }

   MgenLutFunction(MilLut, M_COLORMAP_TURBO + M_LAST_GRAY, M_DEFAULT,
                   M_RGB888(250, 250, 250), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   for(MIL_INT i = 0; i < NUM_DISPLAY; ++i)
      {
      MilDisplayImageArray[i] = MbufAlloc2d(MilSystem, DISPLAY_SIZE_X, DISPLAY_SIZE_Y, M_UNSIGNED + 8, M_IMAGE | M_PROC | M_DISP, M_UNIQUE_ID);
      MilDisplayArray[i]      = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
      MilGraListArray[i]      = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);

      MdispLut(MilDisplayArray[i], MilLut); 
      MdispControl(MilDisplayArray[i], M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraListArray[i]);
      }

   MdispControl(MilDisplayArray[eBottom], M_WINDOW_INITIAL_POSITION_X, DISPLAY_SIZE_X);
   MdispControl(MilDisplayArray[eBottom], M_TITLE, MIL_TEXT("Bottom 2D View (XY)"));
   DrawAxis(MilSystem, MilGraListArray[eBottom], M_XY_AXES);

   MdispControl(MilDisplayArray[eSide], M_WINDOW_INITIAL_POSITION_X, DISPLAY_SIZE_X * 2);
   MdispControl(MilDisplayArray[eSide], M_TITLE, MIL_TEXT("Side 2D View (XZ)"));
   DrawAxis(MilSystem, MilGraListArray[eSide], M_XZ_AXES);

   // Restore the 3D data.
   MosPrintf(MIL_TEXT("A scan of the calibration disk is restored from file (.mbufc) and displayed.\n"));
   auto MilCalibrationContainer = MbufRestore(FILENAME, MilSystem, M_UNIQUE_ID);
 
   // Convert to 3d processable container.
   MbufConvert3d(MilCalibrationContainer, MilCalibrationContainer, M_NULL, M_DEFAULT,M_DEFAULT);

   // Display the point clouds.
   DisplayContainer(MilCalibrationContainer, MilDisplay3D, MilGraphicList3D, MilDisplayArray,
                    MilDisplayImageArray);

   MosPrintf(MIL_TEXT("The scan of the disk is deformed and misaligned.\n\n"));
   
   ShowDifferentViews(MilSystem, MilCalibrationContainer, MilDisplayArray, MilDisplayImageArray); 

   MosPrintf(MIL_TEXT("Running M3dmapAlignScan.."));
   
   // Set the controls based on the specifications of the calibration disk.
   M3dmapControl(MilContext, M_DEFAULT, M_OBJECT_SHAPE, M_DISK);
   M3dmapControl(MilContext, M_DEFAULT, M_DIAMETER, 70.0); // Ideal disk diameter.
   M3dmapControl(MilContext, M_DEFAULT, M_HEIGHT, 50.0);

   M3dmapAlignScan(MilContext, MilCalibrationContainer, MilResult, M_DEFAULT);

   MosPrintf(MIL_TEXT(".\n\n"));

   ShearScaleCorrection(MilSystem, MilCalibrationContainer, MilContainer,
                        MilResult, true);

   DisplayContainer(MilContainer, MilDisplay3D, MilGraphicList3D, MilDisplayArray,
                    MilDisplayImageArray);

   ShowDifferentViews(MilSystem, MilContainer, MilDisplayArray, MilDisplayImageArray);

   MosPrintf(MIL_TEXT("M3dmapAlignScan will fixture the scan.\n"));
   MosPrintf(MIL_TEXT("The origin of the Z-axis (Z = 0) will be set at the floor.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   FullCorrection(MilFullMatrix, MilCalibrationContainer, MilContainer, MilResult);

   ShowDifferentViews(MilSystem,MilContainer, MilDisplayArray, MilDisplayImageArray); 

   MosPrintf(MIL_TEXT("The origin of the Z-axis (Z = 0) will be set at the disk's top.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Running M3dmapAlignScan..\n\n"));
   M3dmapControl(MilContext, M_CONTEXT, M_ALIGN_Z_POSITION, M_OBJECT_TOP);
   M3dmapAlignScan(MilContext, MilCalibrationContainer, MilResult, M_DEFAULT);

   FullCorrection(MilFullMatrix, MilCalibrationContainer, MilContainer, MilResult);

   ShowDifferentViews(MilSystem,MilContainer, MilDisplayArray, MilDisplayImageArray);

   MosPrintf(MIL_TEXT("The origin of the X-axis (X = 0) will be set at the disk's center.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Running M3dmapAlignScan..\n\n"));
   M3dmapControl(MilContext, M_CONTEXT, M_ALIGN_X_POSITION, M_OBJECT_CENTER);
   M3dmapAlignScan(MilContext, MilCalibrationContainer, MilResult, M_DEFAULT);

   FullCorrection(MilFullMatrix, MilCalibrationContainer, MilContainer, MilResult);

   ShowDifferentViews(MilSystem,MilContainer, MilDisplayArray, MilDisplayImageArray );

   MosPrintf(MIL_TEXT("The Z-axis direction will be set to point downwards.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Running M3dmapAlignScan..\n\n"));
   M3dmapControl(MilContext, M_CONTEXT, M_ALIGN_Z_DIRECTION, M_Z_DOWN);
   M3dmapAlignScan(MilContext, MilCalibrationContainer, MilResult, M_DEFAULT);

   FullCorrection(MilFullMatrix, MilCalibrationContainer, MilContainer, MilResult);
   if(MilDisplay3D)
      {
      M3ddispSetView(MilDisplay3D, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_ZOOM, 2, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
   ShowDifferentViews(MilSystem, MilContainer, MilDisplayArray, MilDisplayImageArray);

   MosPrintf(MIL_TEXT("The Z-axis direction of the scan is now pointing downwards.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Display a 3D container.
//*****************************************************************************
void DisplayContainer(MIL_ID MilContainer, MIL_ID MilDisplay3D, MIL_ID MilGraphicList,
                      const MIL_UNIQUE_DISP_ID MilDisplayArray[2],
                      const MIL_UNIQUE_BUF_ID MilDepthMapArray[2])
   {
   if(MilDisplay3D)
      {
      MIL_INT64 MilContainerGraphics = M3ddispSelect(MilDisplay3D, MilContainer, M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_USE_LUT, M_TRUE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3dgraControl(MilGraphicList, MilContainerGraphics, M_COLOR_COMPONENT_BAND, 2);

      // Adjust the view of the 3D displays.
      M3ddispSetView(MilDisplay3D, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      M3ddispSetView(MilDisplay3D, M_ZOOM, 1.8, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
  
   M3dimCalibrateDepthMap(MilContainer, MilDepthMapArray[eBottom], M_NULL, M_NULL, M_DEFAULT, M_DEFAULT, M_CENTER);
   M3dimProject(MilContainer, MilDepthMapArray[eBottom], M_NULL, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_BLUE);

   // Display the projected point cloud container.
   MdispSelect(MilDisplayArray[eBottom], MilDepthMapArray[eBottom]);
   }

//*****************************************************************************
// Show different views of a container in 2D display.
//*****************************************************************************
void ShowDifferentViews(MIL_ID MilSystem, MIL_ID MilContainer,
                        const MIL_UNIQUE_DISP_ID MilDisplayArray[2],
                        const MIL_UNIQUE_BUF_ID MilDepthMapArray[2])
   {
   MIL_UNIQUE_BUF_ID RotatedContainer = MbufAllocContainer(MilSystem, M_PROC, M_DEFAULT, M_UNIQUE_ID);

   M3dimCalibrateDepthMap(MilContainer, MilDepthMapArray[eBottom], M_NULL, M_NULL, M_DEFAULT, M_DEFAULT, M_CENTER);
   M3dimProject(MilContainer, MilDepthMapArray[eBottom], M_NULL, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   McalDraw(M_DEFAULT, MilDepthMapArray[eBottom], MilDepthMapArray[eBottom], M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT); 
   MdispSelect(MilDisplayArray[eBottom], MilDepthMapArray[eBottom]);

   // Rotate the point cloud container to be in the xy plane before projecting.
   M3dimRotate(MilContainer, RotatedContainer, M_ROTATION_XYZ, 90, 0, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   M3dimCalibrateDepthMap(RotatedContainer, MilDepthMapArray[eSide], M_NULL, M_NULL, M_DEFAULT, M_DEFAULT, M_CENTER);
   M3dimProject(RotatedContainer, MilDepthMapArray[eSide], M_NULL, M_DEFAULT, M_MIN_Z, M_DEFAULT, M_DEFAULT);
   McalDraw(M_DEFAULT, MilDepthMapArray[eSide], MilDepthMapArray[eSide], M_DRAW_RELATIVE_COORDINATE_SYSTEM+ M_DRAW_MAJOR_MARKS, M_DEFAULT, M_DEFAULT);
   MdispSelect(MilDisplayArray[eSide], MilDepthMapArray[eSide]);
   }

//*****************************************************************************
// Correct for the shape deformations due to shear and scales.
//*****************************************************************************
void ShearScaleCorrection(MIL_ID MilSystem,MIL_ID MilSrcContainer, MIL_ID MilDstContainer,
                          MIL_ID MilResult, bool UseBufConvert)
   {
   // When UseBufConvert = true, MbufConvert3d is used to apply the corrections.
   // When UseBufConvert = false, M3dimMatrixTransform is used to apply the corrections.
   if(UseBufConvert)
      {
      MbufCopyComponent(MilSrcContainer, MilDstContainer, M_COMPONENT_ALL, M_REPLACE, M_DEFAULT);
      MIL_DOUBLE ShearZ, ShearX, ScaleY;
      M3dmapGetResult(MilResult, M_DEFAULT, M_3D_SHEAR_Z, &ShearZ);
      M3dmapGetResult(MilResult, M_DEFAULT, M_3D_SHEAR_X, &ShearX);
      M3dmapGetResult(MilResult, M_DEFAULT, M_3D_SCALE_Y, &ScaleY);

      MosPrintf(MIL_TEXT("M3dmapAlignScan found the following corrections : \n"), ShearZ);
      MosPrintf(MIL_TEXT("The correction SHEAR_Z : %f \n"), ShearZ);
      MosPrintf(MIL_TEXT("The correction SHEAR_X : %f \n"), ShearX);
      MosPrintf(MIL_TEXT("The correction SCALE_Y : %f \n\n"), ScaleY);

      MosPrintf(MIL_TEXT("There are two ways to correct the scan. The first is with M3dimMatrixTransform\n"));     
      MosPrintf(MIL_TEXT("and the second, which this example uses, is with MbufConvert3d.\n\n"));

      // Show corrections illustration.
         {
         auto IllustrationDispId = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
         MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Scan corrections."));
         auto IllustrationImageId = MbufRestore(CORR_ILLUSTRATION_FILENAME, MilSystem, M_UNIQUE_ID);
         MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_Y, DISPLAY_SIZE_Y + 40);
         MdispSelect(IllustrationDispId, IllustrationImageId);

         MosPrintf(MIL_TEXT("Press <Enter> to correct the scan.\n\n"));
         MosGetch();
         }

      MIL_ID Range = MbufInquireContainer(MilDstContainer, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
      MbufControl(Range, M_3D_SHEAR_Z, ShearZ);
      MbufControl(Range, M_3D_SCALE_Y, ScaleY);
      MbufControl(Range, M_3D_SHEAR_X, ShearX);
      MbufConvert3d(MilDstContainer, MilDstContainer, M_NULL, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      // Method using M3dimMatrixTransform.
      auto ShearMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dmapCopyResult(MilResult, M_DEFAULT, ShearMatrix, M_SHEAR_MATRIX, M_DEFAULT);
      M3dimMatrixTransform(MilSrcContainer, MilDstContainer, ShearMatrix, M_DEFAULT);
      }
   }

//*****************************************************************************
// Correct for the shape deformations due to shear and scales and
// fixture the scan.
//*****************************************************************************
void FullCorrection(MIL_ID FullMatrix, MIL_ID MilSrcContainer,
                    MIL_ID MilDstContainer, MIL_ID MilResult)
   {
   M3dmapCopyResult(MilResult, M_DEFAULT, FullMatrix, M_TRANSFORMATION_MATRIX, M_DEFAULT);
   M3dimMatrixTransform(MilSrcContainer, MilDstContainer, FullMatrix, M_DEFAULT);
   }

//*****************************************************************************
// Draws the 2D axis in the 2D display.
//*****************************************************************************
void DrawAxis(MIL_ID MilSystem, MIL_ID MilGraphicList, MIL_INT AxisType)
   {
   auto MilGraContext  = MgraAlloc(MilSystem, M_UNIQUE_ID);
 
   MgraControl(MilGraContext, M_DRAW_DIRECTION, M_PRIMARY_DIRECTION);
   MgraControl(MilGraContext, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraColor  (MilGraContext, M_COLOR_BLACK);
   MgraControl(MilGraContext, M_INPUT_UNITS, M_DISPLAY);

   if(AxisType == M_XY_AXES)
      {
      MgraLine(MilGraContext, MilGraphicList, 5, 5, 40, 5); // Horizontal line.
      MgraLine(MilGraContext, MilGraphicList, 5, 5, 5, 48); // Vertical line.
      MgraText(MilGraContext, MilGraphicList, 20, 8, MIL_TEXT("X"));
      MgraText(MilGraContext, MilGraphicList, 8, 25, MIL_TEXT("Y"));
      }
   else //M_XZ_AXES
      {
      MgraLine(MilGraContext, MilGraphicList, 5, 40, 40, 40); // Horizontal line.
      MgraLine(MilGraContext, MilGraphicList, 5, 40, 5, 5);   // Vertical line.
      MgraText(MilGraContext, MilGraphicList, 20, 42, MIL_TEXT("X"));
      MgraText(MilGraContext, MilGraphicList, 8, 15,  MIL_TEXT("Z"));
      }
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
void Alloc3dDisplayId(MIL_ID MilSystem, MIL_UNIQUE_3DDISP_ID* MilDisplay)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   *MilDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay->get())
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();
      }
   }

//*******************************************************************************
// Checks the required file exist.
//*****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
