//***************************************************************************************
// 
// File name: M3ddisp.cpp  
//
// Synopsis: This program contains an example of how to use 3D displays in mil.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include <mil.h>
#include <math.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("M3ddisp\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use MIL 3D displays.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, buffer, 3D display, 3D graphics.\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING BACKGROUND_IMAGE_FILE = M_IMAGE_PATH MIL_TEXT("imaginglogo.mim");
static const MIL_DOUBLE PI = 3.14159265358979323846;

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_UNIQUE_BUF_ID Generate3DContainer(MIL_ID MilSystem);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the MIL system.
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the MIL 3D Display.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3d = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // Make sure we meet the minimum requirements for the 3d display.
   if(!MilDisplay3d)
      {
      MosPrintf(MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press <ENTER> to end.\n"));
      MosGetch();
      return 0;
      }

   // Show the display.
   MIL_ID MilGraphicList3d;
   M3ddispInquire(MilDisplay3d, M_3D_GRAPHIC_LIST_ID, &MilGraphicList3d);

   //                                            X,     Y,     Z
   M3ddispSetView(MilDisplay3d, M_VIEWPOINT,  100.0,  75.0,  75.0, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_UP_VECTOR,   0.0,   0.0,   1.0, M_DEFAULT);   
   M3ddispSelect(MilDisplay3d, M_NULL, M_OPEN, M_DEFAULT);

   // Draw an axis and a grid.
   MosPrintf(MIL_TEXT("MIL 3D displays can be used with 0, 1 or many point clouds.\n")
             MIL_TEXT("This allows you to show only the content of the display's graphics list.\n")
             MIL_TEXT("In this case, an axis and a grid are shown.\n\n"));

   MIL_DOUBLE AxisLength = 15.0;
   MIL_INT64 AxisLabel = M3dgraAxis(MilGraphicList3d, M_ROOT_NODE, M_DEFAULT, AxisLength, M_NULL, M_DEFAULT, M_DEFAULT);

   // Draw a grid to be displayed.
   MIL_UNIQUE_3DGEO_ID MatrixGrid = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MatrixGrid, M_TRANSLATION, AxisLength, AxisLength * 1.5, 0.0, M_DEFAULT, M_DEFAULT);
   MIL_INT64 GridLabel = M3dgraGrid(MilGraphicList3d, AxisLabel, M_SIZE_AND_SPACING, MatrixGrid, AxisLength * 2, AxisLength * 3, 5.0, 5.0, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, GridLabel, M_FILL_COLOR, M_COLOR_WHITE);
   M3dgraControl(MilGraphicList3d, GridLabel, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(MilGraphicList3d, GridLabel, M_OPACITY, 20);

   // Translate both point clouds.
   MIL_UNIQUE_3DGEO_ID MatrixTranslate1 = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MatrixTranslate1, M_TRANSLATION, AxisLength, AxisLength * 2.2,  0.0, M_DEFAULT, M_DEFAULT);
   MIL_UNIQUE_3DGEO_ID MatrixTranslate2 = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MatrixTranslate2, M_TRANSLATION, AxisLength, AxisLength * 0.75, 0.0, M_DEFAULT, M_DEFAULT);

   // Save viewpoint.
   MIL_UNIQUE_3DGEO_ID InitialViewpointMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("Use the mouse to set the 3D view in the display.\n"));
   MosPrintf(MIL_TEXT("   - Left click and drag   : Orbits around the interest point.\n"));
   MosPrintf(MIL_TEXT("   - Right click and drag  : Translates in the screen's plane.\n"));
   MosPrintf(MIL_TEXT("   - Middle click and drag : Rolls.\n"));
   MosPrintf(MIL_TEXT("   - Mouse wheel           : Zooms in, Zooms out.\n"));
   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("The resulting 3D view will be stored in a matrix using M3ddispCopy\n"));
   MosPrintf(MIL_TEXT("and will be reused later.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to copy the current 3D view and continue.\n"));
   MosGetch();
   M3ddispCopy(MilDisplay3d, InitialViewpointMatrix, M_VIEW_MATRIX, M_DEFAULT);

   MosPrintf(MIL_TEXT("Two point clouds have been added using M3ddispSelect.\n\n"));

   // Generate a first meshed point cloud container.
   MIL_UNIQUE_BUF_ID MilContainerId1 = Generate3DContainer(MilSystem);

   // Clone the first to a second container.
   MIL_UNIQUE_BUF_ID MilContainerId2 = MbufClone(MilContainerId1, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);

   M3ddispControl(MilDisplay3d, M_UPDATE, M_DISABLE);

   // Select the containers to the 3D display and keep their corresponding labels in the graphics list of the 3D display.
   MIL_INT64 ContainerLabel1 = M3ddispSelect(MilDisplay3d, MilContainerId1, M_ADD, M_DEFAULT);
   MIL_INT64 ContainerLabel2 = M3ddispSelect(MilDisplay3d, MilContainerId2, M_ADD, M_DEFAULT);

   // Move the second container.
   M3dgraCopy(MatrixTranslate1, M_DEFAULT, MilGraphicList3d, ContainerLabel1, M_TRANSFORMATION_MATRIX, M_DEFAULT);
   M3dgraCopy(MatrixTranslate2, M_DEFAULT, MilGraphicList3d, ContainerLabel2, M_TRANSFORMATION_MATRIX, M_DEFAULT);

   M3ddispControl(MilDisplay3d, M_UPDATE, M_ENABLE);

   // Setting the viewpoint.
   MosPrintf(MIL_TEXT("Many options exist to change the display's viewpoint.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to set the viewpoint, interest point and up vector.\n\n"));
   MosGetch();
   M3ddispControl(MilDisplay3d, M_UPDATE, M_DISABLE);
   //                                                            X,                Y,                 Z
   M3ddispSetView(MilDisplay3d, M_VIEWPOINT     , AxisLength * 3.0,       AxisLength, AxisLength * 10.0, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_INTEREST_POINT,       AxisLength,       AxisLength,               0.0, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_UP_VECTOR     ,             -1.0,              0.0,               0.0, M_DEFAULT);

   M3ddispControl(MilDisplay3d, M_UPDATE, M_ENABLE);

   // Show the options to move the view.
   MosPrintf(MIL_TEXT("The view parameters can be either specific values or values composed\n"));
   MosPrintf(MIL_TEXT("with the current 3D view.\n"));
   MosPrintf(MIL_TEXT("Different options will be shown:\n"));
   MosPrintf(MIL_TEXT(" -Move the viewpoint, relative to its current position, while keeping\n"));
   MosPrintf(MIL_TEXT("  the interest point constant.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i < 100) && !MosKbhit(); i++)
      {
      MosSleep(15);
      MIL_DOUBLE Sign = (i < 50) ? 1.0 : -1.0;
      M3ddispSetView(MilDisplay3d, M_VIEWPOINT + M_COMPOSE_WITH_CURRENT, 0.0, Sign * 3.0, 0.0, M_DEFAULT);
      }

   // Show the effect of moving only interest point.
   MosPrintf(MIL_TEXT(" -Move the interest point while keeping the viewpoint constant.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i < 100) && !MosKbhit(); i++)
      {
      MosSleep(15);
      MIL_DOUBLE Sign = (i < 50) ? 1.0 : -1.0;
      M3ddispSetView(MilDisplay3d, M_INTEREST_POINT + M_COMPOSE_WITH_CURRENT, 0.0, Sign * 0.5, 0.0, M_DEFAULT);
      }

   // Reset the point of view.
   M3ddispControl(MilDisplay3d, M_UPDATE, M_DISABLE);
   //                                                            X,                Y,                 Z
   M3ddispSetView(MilDisplay3d, M_VIEWPOINT     , AxisLength * 3.0,       AxisLength, AxisLength * 10.0, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_INTEREST_POINT,       AxisLength,       AxisLength,               0.0, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_UP_VECTOR     ,             -1.0,              0.0,               0.0, M_DEFAULT);
   M3ddispControl(MilDisplay3d, M_UPDATE, M_ENABLE);

   // Rotate up vector.
   MosPrintf(MIL_TEXT(" -Modify the up vector (the same can be done by modifying the roll value).\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i <= 100) && !MosKbhit(); i++)
      {
      MosSleep(15);
      M3ddispSetView(MilDisplay3d, M_UP_VECTOR, cos((2.0 * PI*i / 100.0) + PI), sin((2.0 * PI*i / 100.0)), 0.0, M_DEFAULT);
      }

   // Translate the viewpoint and interestpoint.
   MosPrintf(MIL_TEXT(" -Translate both the view and interest point.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i < 50) && !MosKbhit(); i++)
      {
      MosSleep(15);
      M3ddispSetView(MilDisplay3d, M_TRANSLATE, 0.0, 1.0, 0.0, M_DEFAULT);
      }
   for(int i = 0; (i < 50) && !MosKbhit(); i++)
      {
      MosSleep(15);
      M3ddispSetView(MilDisplay3d, M_TRANSLATE, 0.0, -0.95, 0.05, M_DEFAULT);
      }

   // Zoom.
   MosPrintf(MIL_TEXT(" -Zoom in and out.        \n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i < 100) && !MosKbhit(); i++)
      {
      MosSleep(15);
      MIL_DOUBLE Zoom = (i < 50) ? 1.01 : 0.99;
      M3ddispSetView(MilDisplay3d, M_ZOOM, Zoom, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }

   // Azimuth and Elevation.
   MosPrintf(MIL_TEXT(" -Modify the azimuth and the elevation.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   for(int i = 0; (i < 50) && !MosKbhit(); i++)
      {
      MosSleep(15);
      M3ddispSetView(MilDisplay3d, M_ELEVATION + M_COMPOSE_WITH_CURRENT, cos((PI*i / 50.0) + PI), M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }
   for(int i = 0; (i < 50) && !MosKbhit(); i++)
      {
      MosSleep(15);
      M3ddispSetView(MilDisplay3d, M_AZIMUTH + M_COMPOSE_WITH_CURRENT, cos((PI*i / 50.0) + PI), M_DEFAULT, M_DEFAULT, M_DEFAULT);
      }

   // Specific node.
   MosPrintf(MIL_TEXT(" -Set the view to something that includes graphics of a specific node.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   M3ddispSetView(MilDisplay3d, M_AUTO, M_DEFAULT, ContainerLabel1, M_DEFAULT, M_DEFAULT);

   // Viewbox.
   MosPrintf(MIL_TEXT(" -Set the view to something that includes everything in the scene.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   
   M3ddispSetView(MilDisplay3d, M_VIEW_BOX, M_WHOLE_SCENE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   
   // Predefined orientations.
   MosPrintf(MIL_TEXT(" -Set the view to a view from the top.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, M_TOP_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT(" -Set the view to a view from below.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT(" -Set the view to a view from the side.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, M_LEFT_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT(" -Set the view to an angled view from the top.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT(" -Set the view to an angled view from below.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, M_BOTTOM_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Restore copied view.
   MosPrintf(MIL_TEXT("Press <Enter> to restore the previously copied view.\n\n"));
   MosGetch();
   M3ddispCopy(InitialViewpointMatrix, MilDisplay3d, M_VIEW_MATRIX, M_DEFAULT);

   MosPrintf(MIL_TEXT("The display's background color can be set to a solid color,\n"));
   MosPrintf(MIL_TEXT("a gradient, or an image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to change the background color.\n\n"));
   MosGetch();

   M3ddispControl(MilDisplay3d, M_BACKGROUND_MODE, M_SINGLE_COLOR);
   M3ddispControl(MilDisplay3d, M_BACKGROUND_COLOR, M_RGB888(50, 150, 125));
   
   MosPrintf(MIL_TEXT("Press <Enter> to apply a gradient to the background.\n\n"));
   MosGetch();

   M3ddispControl(MilDisplay3d, M_BACKGROUND_MODE, M_GRADIENT_VERTICAL);

   M3ddispControl(MilDisplay3d, M_BACKGROUND_COLOR, M_COLOR_DARK_BLUE);
   M3ddispControl(MilDisplay3d, M_BACKGROUND_COLOR_GRADIENT, M_COLOR_DARK_YELLOW);

   MosPrintf(MIL_TEXT("Press <Enter> to use an image for the display's background.\n\n"));
   MosGetch();

   MIL_UNIQUE_BUF_ID Image = MbufRestore(BACKGROUND_IMAGE_FILE, MilSystem, M_UNIQUE_ID);
   // Make the image darker.
   MimShift(Image, Image, -1);
   M3ddispCopy(Image, MilDisplay3d, M_BACKGROUND_IMAGE, M_DEFAULT);
   M3ddispControl(MilDisplay3d, M_BACKGROUND_MODE, M_BACKGROUND_IMAGE);

   MosPrintf(MIL_TEXT("A gyroscope indicating interaction with the 3d display can be permanently\n"));
   MosPrintf(MIL_TEXT("visible. Its appearance can also be modified.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to make the gyroscope permanently visible.\n\n"));
   MosGetch();
   M3ddispControl(MilDisplay3d, M_ROTATION_INDICATOR, M_ENABLE);
   M3ddispControl(MilDisplay3d, M_BACKGROUND_MODE, M_SINGLE_COLOR);
   M3ddispControl(MilDisplay3d, M_BACKGROUND_COLOR, M_RGB888(50, 150, 125));

   MosPrintf(MIL_TEXT("Many keys are assigned to interactive actions.\n"));
   MosPrintf(MIL_TEXT("   Arrows : Orbit around the interest point.\n"));
   MosPrintf(MIL_TEXT("   Ctrl   : Speed modifier for the arrow keys.\n"));
   MosPrintf(MIL_TEXT("   Alt    : Action modifier for the arrow keys. Press Alt and Up/Down arrow for\n"));
   MosPrintf(MIL_TEXT("            zooming; press Alt and Left/Right arrow for rolling.\n"));
   MosPrintf(MIL_TEXT("   Shift  : Modifies the arrows' function, moving the screen's plane instead.\n"));
   MosPrintf(MIL_TEXT("   1 - 8  : Specify the predefined viewpoint. Press a number key.\n"));

   MosPrintf(MIL_TEXT("Set focus to the 3D display window to use the keyboard.\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Generate a MIL 3D Container to display.
//****************************************************************************
MIL_UNIQUE_BUF_ID Generate3DContainer(MIL_ID MilSystem)
   {
   // Use a SDCF to acquire a MIL container with 3D data.
   MIL_UNIQUE_DIG_ID MilDigitizer = MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_3D_SIMULATOR"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID MilContainer3D = MbufAllocDefault(MilSystem, MilDigitizer, M_GRAB + M_DISP, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MdigGrab(MilDigitizer, MilContainer3D);

   return MilContainer3D;
   }
