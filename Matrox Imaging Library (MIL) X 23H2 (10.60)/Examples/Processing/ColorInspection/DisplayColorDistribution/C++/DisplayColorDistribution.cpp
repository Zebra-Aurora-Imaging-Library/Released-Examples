//************************************************************************************/
// File name: DisplayColorDistribution.cpp
//
// Synopsis:  This program contains an example to display the color distribution
//            of an image using 3D display module.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//************************************************************************************/

#include <mil.h>
#include <vector>

using namespace std;
//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("DisplayColorDistribution\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to display the color distribution of an image using the MIL 3D display module.\n")
             MIL_TEXT("The example then performs a principal component analysis (PCA) and displays some components of the color distribution.\n\n"));


   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Buffer, Display, Graphic, 3D Display,\n")
             MIL_TEXT("3D Graphics, 3D Geometry, Image Processing, Color Analysis.\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING CANDY_FILE = M_IMAGE_PATH MIL_TEXT("Candy.mim");
static const MIL_INT    BRUSH_SIZE = 10;

//****************************************************************************
//Data structure to be passed to the hook functions.
//****************************************************************************
struct HookData
   {
   MIL_ID  MilSystem;
   MIL_ID  MilDisplay;
   MIL_ID  MilGraphicList;
   MIL_ID  MaskImage;
   MIL_ID  MilOverlayImage;
   bool    DoDraw;
   bool    DoErase;
   MIL_INT MaskColor;
   MIL_INT TransparentColor;
   };

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_INT MFTYPE StopDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
MIL_INT MFTYPE StartDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
MIL_INT MFTYPE DrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
void SelectROI(MIL_ID MilDisplay, MIL_ID MilGraphicList, MIL_ID MilOverlayImage, MIL_ID MaskImage);
void Set3dDisplay(MIL_ID MilSystem, MIL_ID MilDisplay3d, MIL_ID MilGraphicList3d, MIL_INT64& AxisLabel, MIL_INT64& NodeLabel);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   MIL_UNIQUE_SYS_ID MilSystem;           // System identifier.
   MIL_UNIQUE_BUF_ID MilImageId;          // Original Image
   MIL_UNIQUE_DISP_ID MilDisplay;         // 2D MIL Display
   MIL_UNIQUE_3DDISP_ID MilDisplay3d;     // 3D Mil Display.

   // Allocate a MIL system. 
   MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   //Configure 3D display.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MilDisplay3d = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
   // Make sure we meet the minimum requirements for the 3d display.
   if(!MilDisplay3d)
      {
      MosPrintf(MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to end.\n"));
      MosGetch();
      return 0;
      }
   M3ddispControl(MilDisplay3d, M_TITLE, MIL_TEXT("Color distribution in the RGB space"));
   M3ddispControl(MilDisplay3d, M_WINDOW_INITIAL_POSITION_X, 600);

   // Configure 2D display.
   MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Color image"));
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispControl(MilDisplay, M_SCALE_DISPLAY, M_ENABLE);
   // Associate the 2D graphic list to the 2D display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Restore and display the model image.
   MilImageId = MbufRestore(CANDY_FILE, MilSystem, M_UNIQUE_ID);
   MdispSelect(MilDisplay, MilImageId);
   MosPrintf(MIL_TEXT("The color distribution of this image will be displayed.\n"));

   //Ask if the user wants to use his own image.
   MosPrintf(MIL_TEXT("Press <c> to choose another RGB image or press any other key to continue.\n\n"));
   MIL_INT key = MosGetch();

   if((key == 'c') || (key == 'C'))
      {
      MIL_INT Bands = 0;
      while(Bands != 3)
         {
         MappControl(M_ERROR, M_PRINT_DISABLE);
         MilImageId = MbufRestore(M_INTERACTIVE, MilSystem, M_UNIQUE_ID); //  To choose your own image.
         MappControl(M_ERROR, M_PRINT_ENABLE);

         /* Restore a default context if needed. */
         if(MilImageId == M_NULL)
            {
            MilImageId = MbufRestore(CANDY_FILE, MilSystem, M_UNIQUE_ID);
            }
         
         Bands = MbufInquire(MilImageId, M_SIZE_BAND, M_NULL);
         if(Bands != 3)
            {
            MosPrintf(MIL_TEXT("You have chosen a single band image. Please choose a 3 bands image.\n\n"));
            }
         }
      MdispSelect(MilDisplay, MilImageId);
      }

   // Set the 3D display.
   MIL_ID MilGraphicList3d = (MIL_ID)M3ddispInquire(MilDisplay3d, M_3D_GRAPHIC_LIST_ID, M_NULL);
   MIL_INT64 AxisLabel, NodeLabel;
   Set3dDisplay(MilSystem, MilDisplay3d, MilGraphicList3d, AxisLabel, NodeLabel);

   // Get data from all bands of the image and place it in a vector.
   MIL_INT SizeX = MbufInquire(MilImageId, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilImageId, M_SIZE_Y, M_NULL);
   vector<MIL_UINT8> pixels;
   MbufGetColor(MilImageId, M_PLANAR, M_ALL_BANDS, pixels);

   // Allocate a container and set its range component.
   MIL_UNIQUE_BUF_ID imageContainerId = MbufAllocContainer(MilSystem, M_DISP + M_PROC, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID RangeID = MbufAllocComponent(imageContainerId, 3, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_COMPONENT_RANGE, M_NULL);
   MbufPutColor(RangeID, M_PLANAR, M_ALL_BANDS, pixels);

   // Display the point cloud.
   MIL_INT64 ContainerLabel = M3dgraAdd(MilGraphicList3d, AxisLabel, imageContainerId, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_THICKNESS, 3);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_OPACITY + M_RECURSIVE, 100.0);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   MosPrintf(MIL_TEXT("The color distribution of the image is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Use the mouse to set the 3D view in the display.\n")
      MIL_TEXT("\t- Left click and drag   : Orbits around the interest point.\n")
      MIL_TEXT("\t- Right click and drag  : Translates in the screen's plane.\n")
      MIL_TEXT("\t- Middle click and drag : Rolls.\n")
      MIL_TEXT("\t- Mouse wheel           : Zooms in, Zooms out.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to continue.\n\n"));
   MosGetch();


   // Ask the user to select pixels.
   MosPrintf(MIL_TEXT("If you wish, define a selection mask over the color image to display\n")
             MIL_TEXT("only the color distribution of the selected pixels.\n"));

   // Allocate the mask.
   MIL_UNIQUE_BUF_ID MaskImageId = MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(MaskImageId, M_SOURCE_LABEL);
   
   MbufClear(MaskImageId, 0X0);
   MIL_ID MilOverlayImage = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   SelectROI(MilDisplay, MilGraphicList, MilOverlayImage, MaskImageId);

   // Find pixels with  M_SOURCE_LABEL value in the MaskImage.
   MIL_INT SrcLabelFound   = MimDetectEvent(MaskImageId, M_EQUAL, M_SOURCE_LABEL, M_NULL);
   if (SrcLabelFound == M_FALSE)
      {
      MbufClear(MaskImageId, M_SOURCE_LABEL);
      MosPrintf(MIL_TEXT("A selection mask was not defined. The color distribution of the original image is displayed.\nPress <ENTER> to continue.\n\n"));
      }
   else
      {
      // Update the color image to show only the selected pixels.
      MbufClearCond(MilImageId, 0, 0, 0, MaskImageId, M_EQUAL, 0);
      MosPrintf(MIL_TEXT("The color distribution of the selected pixels is displayed.\nPress <ENTER> to continue.\n\n"));
      }

   // Set confidence component.
   MbufControl(MaskImageId, M_COMPONENT_TYPE, M_COMPONENT_CONFIDENCE);
   MbufCopyComponent(MaskImageId, imageContainerId, M_DEFAULT, M_APPEND, M_IDENTICAL);
   MosGetch();

   // Allocate buffer to store PCA.
   MIL_UNIQUE_BUF_ID PrincipalCompResult;
   PrincipalCompResult = MbufAlloc2d(MilSystem, 5, 3, 32 + M_FLOAT, M_ARRAY, M_UNIQUE_ID);

   // Compute the PCA.
   McolProject(
      MilImageId,
      MaskImageId,         // This mask determines which areas of the source image (MilimageId) to use to calculate the principal components.
      PrincipalCompResult,
      M_NULL,
      M_PRINCIPAL_COMPONENTS,
      M_DEFAULT,
      M_NULL);

   MIL_FLOAT princCompArray[3][5];
   MbufGet(PrincipalCompResult, princCompArray);

   // Draw the principal, second and third components in the 3D display.
   vector<MIL_DOUBLE> Colors = {M_COLOR_MAGENTA, M_COLOR_YELLOW, M_COLOR_CYAN};
   for(MIL_INT c = 0; c < 3; c++)
      {
      MIL_DOUBLE Length = 200.0 / ((c * 3) + 1);
      MIL_DOUBLE X0 = princCompArray[0][4] - princCompArray[0][c] * Length;
      MIL_DOUBLE Y0 = princCompArray[1][4] - princCompArray[1][c] * Length;
      MIL_DOUBLE Z0 = princCompArray[2][4] - princCompArray[2][c] * Length;
      MIL_DOUBLE X1 = princCompArray[0][4] + princCompArray[0][c] * Length;
      MIL_DOUBLE Y1 = princCompArray[1][4] + princCompArray[1][c] * Length;
      MIL_DOUBLE Z1 = princCompArray[2][4] + princCompArray[2][c] * Length;
      MIL_INT64 LineLabel = M3dgraLine(MilGraphicList3d, NodeLabel, M_TWO_POINTS, M_DEFAULT, X0, Y0, Z0, X1, Y1, Z1, M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicList3d, LineLabel, M_THICKNESS, 4);
      M3dgraControl(MilGraphicList3d, LineLabel, M_COLOR, Colors[c]);
      M3dgraControl(MilGraphicList3d, LineLabel, M_OPACITY + M_RECURSIVE, 100.0);
      }

   MosPrintf(MIL_TEXT("The orientation of the principal (magenta), second (yellow) and third (cyan)\n")
             MIL_TEXT("components of the PCA are displayed.\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to end.\n"));
   MosGetch();
   
   return 0;
   }

MIL_INT MFTYPE StartDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   MIL_DOUBLE PosX, PosY;

   // Inquire the mouse position in the displayed buffer.
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);

   HookData* HookDataPtr = (HookData*)UserDataPtr;

   MIL_INT DisplayMaskColor = 0, BufferMaskColor = 0;

   if(HookType == M_MOUSE_LEFT_BUTTON_DOWN)
      {
      // Set the draw flag and mask color.
      HookDataPtr->DoDraw = true;
      DisplayMaskColor = HookDataPtr->MaskColor;
      BufferMaskColor = M_SOURCE_LABEL;
      }
   else if(HookType == M_MOUSE_RIGHT_BUTTON_DOWN)
      {
      // Set the erase flag and mask color.
      HookDataPtr->DoErase = true;
      DisplayMaskColor = HookDataPtr->TransparentColor;
      BufferMaskColor = 0;
      }

   // Draw or erase the rectangle mask in the overlay.
   MgraColor(M_DEFAULT, (MIL_DOUBLE)DisplayMaskColor);
   MgraRectFill(M_DEFAULT, HookDataPtr->MilOverlayImage,
      (MIL_INT)PosX - BRUSH_SIZE / 2, (MIL_INT)PosY - BRUSH_SIZE / 2,
      (MIL_INT)PosX + BRUSH_SIZE, (MIL_INT)PosY + BRUSH_SIZE);

   // Draw or erase the rectangle region in the mask image.
   MgraColor(M_DEFAULT, (MIL_DOUBLE)BufferMaskColor);
   MgraRectFill(M_DEFAULT, HookDataPtr->MaskImage,
      (MIL_INT)PosX - BRUSH_SIZE / 2, (MIL_INT)PosY - BRUSH_SIZE / 2,
      (MIL_INT)PosX + BRUSH_SIZE, (MIL_INT)PosY + BRUSH_SIZE);

   return 0;
   }


// This function is called when a mouse move event is trapped to continue masking or continue erasing mask.
MIL_INT MFTYPE DrawOrEraseMask(MIL_INT , MIL_ID EventId, void *UserDataPtr)
   {
   MIL_DOUBLE PosX, PosY;

   // Inquire the mouse position in the displayed buffer.
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);

   HookData* HookDataPtr = (HookData*)UserDataPtr;

   if((HookDataPtr->DoDraw || HookDataPtr->DoErase))
      {
      MIL_INT DisplayMaskColor = 0, BufferMaskColor = 0;

      if(HookDataPtr->DoDraw)
         {
         // Set proper color to continue masking.
         DisplayMaskColor = HookDataPtr->MaskColor;
         BufferMaskColor = M_SOURCE_LABEL;
         }
      else if(HookDataPtr->DoErase)
         {
         // Set proper color to continue erasing mask.
         DisplayMaskColor = HookDataPtr->TransparentColor;
         BufferMaskColor = 0;
         }

      // Draw or erase a rectangle region in the overlay.
      MgraColor(M_DEFAULT, (MIL_DOUBLE)DisplayMaskColor);
      MgraRectFill(M_DEFAULT, HookDataPtr->MilOverlayImage, (MIL_INT)PosX - BRUSH_SIZE / 2, (MIL_INT)PosY - BRUSH_SIZE / 2,
         (MIL_INT)PosX + BRUSH_SIZE, (MIL_INT)PosY + BRUSH_SIZE);

      // Draw or erase a rectangle region in the mask image.
      MgraColor(M_DEFAULT, (MIL_DOUBLE)BufferMaskColor);
      MgraRectFill(M_DEFAULT, HookDataPtr->MaskImage, (MIL_INT)PosX - BRUSH_SIZE / 2, (MIL_INT)PosY - BRUSH_SIZE / 2,
         (MIL_INT)PosX + BRUSH_SIZE, (MIL_INT)PosY + BRUSH_SIZE);
      }

   return 0;
   }

// This function is called when :
//    a mouse left button up event is trapped to finish masking.
//    a mouse right button up event is trapped to finish erasing mask.
MIL_INT MFTYPE StopDrawOrEraseMask(MIL_INT , MIL_ID , void *UserDataPtr)
   {
   HookData* HookDataPtr = (HookData*)UserDataPtr;

   // Reset the Draw flag when a mouse left button is released.
   if(HookDataPtr->DoDraw)
      { HookDataPtr->DoDraw = false; }
   // Reset the Erase flag when a mouse right button is released.
   else if(HookDataPtr->DoErase)
      { HookDataPtr->DoErase = false; }

   return 0;
   }

// This function is called to select a region of interest in an image using a brush.
void SelectROI(MIL_ID MilDisplay, MIL_ID MilGraphicList, MIL_ID MilOverlayImage, MIL_ID MaskImage)
   {
   // Assign data to the hood data struct.
   HookData HookDataStruct;
   HookDataStruct.MilDisplay = MilDisplay;
   HookDataStruct.MilGraphicList = MilGraphicList;
   HookDataStruct.MaskImage = MaskImage;
   HookDataStruct.MilOverlayImage = MilOverlayImage;
   HookDataStruct.DoDraw = false;
   HookDataStruct.DoErase = false;
   HookDataStruct.MaskColor = M_COLOR_BLUE;
   HookDataStruct.TransparentColor = MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, M_NULL);
   
   MosPrintf(MIL_TEXT("Draw over the color image using the mouse to select pixels.\n"));
   MosPrintf(MIL_TEXT("\t- Click and hold the left button and drag the mouse to draw.\n"));
   MosPrintf(MIL_TEXT("\t- Click and hold the right button and drag the mouse to erase.\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to finish the pixel selection.\n\n"));

   // Hook functions from display.
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_DOWN, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE, DrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP, StopDrawOrEraseMask, &HookDataStruct);

   // Finish masking if <Enter> is pressed.
   MIL_INT Ch = 0;
   while((Ch != '\r'))
      { Ch = MosGetch();  }

   // Unhook functions from display.
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_DOWN + M_UNHOOK, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN + M_UNHOOK, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE + M_UNHOOK, DrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP + M_UNHOOK, StopDrawOrEraseMask, &HookDataStruct);

   // Clear the annoations.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MgraClear(M_DEFAULT, MilGraphicList);
   }

void Set3dDisplay(MIL_ID MilSystem, MIL_ID MilDisplay3d, MIL_ID MilGraphicList3d, MIL_INT64& AxisLabel, MIL_INT64& NodeLabel)
   {
   // Show the 3D display.
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, -2, -1.1, -1, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_UP_VECTOR, 0, 0, 1, M_DEFAULT);
   M3ddispSelect(MilDisplay3d, M_NULL, M_OPEN, M_DEFAULT);

   // Draw an axis and a grid.
   MIL_DOUBLE AxisLength = 320;
   AxisLabel = M3dgraAxis(MilGraphicList3d, M_ROOT_NODE, M_DEFAULT, AxisLength, M_NULL, M_DEFAULT, M_DEFAULT);

   MIL_DOUBLE RGBValues = 256;
   MIL_UNIQUE_3DGEO_ID Matrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, RGBValues * 0.5, RGBValues * 0.5, 0, M_DEFAULT, M_DEFAULT);
   MIL_INT64 GridLabel = M3dgraGrid(MilGraphicList3d, AxisLabel, M_SIZE_AND_SPACING, Matrix, RGBValues, RGBValues, 16, 16, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, GridLabel, M_FILL_COLOR, M_COLOR_WHITE);
   M3dgraControl(MilGraphicList3d, GridLabel, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(MilGraphicList3d, GridLabel, M_OPACITY, 30);

   //  Draw the box.
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, 0, 0, 0, M_DEFAULT, M_DEFAULT);
   NodeLabel = M3dgraNode(MilGraphicList3d, AxisLabel, Matrix, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_OPACITY, 0);
   MIL_INT64 BoxLabel = M3dgraBox(MilGraphicList3d, NodeLabel, M_CENTER_AND_DIMENSION, RGBValues / 2, RGBValues / 2, RGBValues / 2,
                                  RGBValues, RGBValues, RGBValues, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, BoxLabel, M_THICKNESS, 3);
   M3dgraControl(MilGraphicList3d, BoxLabel, M_OPACITY + M_RECURSIVE, 20.0);

   // Text.
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_FONT_SIZE, 18);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_VERTICAL, M_BOTTOM);

   M3dgeoMatrixSetWithAxes(Matrix, M_XY_AXES, 0, 0, 330, 0, 1, 0, 0, 0, 1, M_DEFAULT);
   MIL_INT64 BLabel = M3dgraText(MilGraphicList3d, NodeLabel, MIL_TEXT("B"), Matrix, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, BLabel, M_COLOR, M_COLOR_BLUE);
   M3dgraControl(MilGraphicList3d, BLabel, M_OPACITY + M_RECURSIVE, 100.0);

   M3dgeoMatrixSetWithAxes(Matrix, M_XY_AXES, 0, 330, 0, 0, 1, 0, 0, 0, 1, M_DEFAULT);
   MIL_INT64 GLabel = M3dgraText(MilGraphicList3d, NodeLabel, MIL_TEXT("G"), Matrix, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, GLabel, M_COLOR, M_COLOR_GREEN);
   M3dgraControl(MilGraphicList3d, GLabel, M_OPACITY + M_RECURSIVE, 100.0);

   M3dgeoMatrixSetWithAxes(Matrix, M_ZY_AXES, 330, 0, 0, 0, 1, 0, 0, 0, 1, M_DEFAULT);
   MIL_INT64 RLabel = M3dgraText(MilGraphicList3d, NodeLabel, MIL_TEXT("R"), Matrix, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, RLabel, M_COLOR, M_COLOR_RED);
   M3dgraControl(MilGraphicList3d, RLabel, M_OPACITY + M_RECURSIVE, 100.0);
   }
