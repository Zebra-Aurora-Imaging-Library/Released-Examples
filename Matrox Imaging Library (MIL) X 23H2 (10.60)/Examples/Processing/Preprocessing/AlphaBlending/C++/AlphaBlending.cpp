﻿//*****************************************************************************/
// 
// File name: AlphaBlending.cpp
// 
// Synopsis:  This program demonstrates how to combine MimArithMultiple and
//            MbufClearCond to create a constant alpha-blended overlay.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("AlphaBlending\n\n")
             
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to combine MimArithMultiple and \n")
             MIL_TEXT("MbufClearCond to create a constant alpha-blended overlay.\n\n")
             
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
// Target MIL image file specifications.
static MIL_CONST_TEXT_PTR COLOR_IMAGE_FILE = M_IMAGE_PATH MIL_TEXT("BaboonRGB.mim");
static MIL_CONST_TEXT_PTR ALPHA_OVERLAY_IMAGE = M_IMAGE_PATH MIL_TEXT("imaginglogo.mim");

// Transparent color of the overlay image.
static const MIL_INT USER_TRANSPARENT_COLOR = M_COLOR_WHITE;

// Alpha blending value.
static const MIL_INT INITIAL_ALPHA_VALUE = 160;
static const MIL_INT ALPHA_INCREMENT = 5;

// Graphic list color palette definitions.
static const MIL_INT COLOR_SPACING = 5;
static const MIL_INT SQUARE_SIZE = 50;
static const MIL_INT NB_COLORS = 5;
static const MIL_INT PALETTE_COLORS[NB_COLORS] = 
   {
   M_RGB888(192, 0, 0),
   M_RGB888(0, 192, 0),
   M_RGB888(0, 0, 192),
   M_RGB888(192, 0, 192),
   M_RGB888(192, 192, 0)
   };

// Size of the brush. 
static const MIL_INT BRUSH_RADIUS = 5;

//*****************************************************************************
// Callback structure.
//*****************************************************************************
struct SHookData
   {
   MIL_ID MilDisplay;
   MIL_ID MilGraContext;
   MIL_ID MilGraList;
   MIL_ID MilDisplayedImage;
   MIL_ID MilOverlay;
   MIL_ID MilUserOverlay;
   MIL_INT DrawingColor;
   MIL_INT OverlayTransparentColor;
   MIL_INT Alpha;
   MIL_INT MinusAlphaLabel;
   MIL_INT PlusAlphaLabel;
   };

//*****************************************************************************
// Function prototypes.
//*****************************************************************************
void AlphaBlend(MIL_ID MilDisplayedImage, MIL_ID MilOverlay, MIL_ID MilUserOverlay,
                MIL_INT UserTransparentColor, MIL_INT OverlayTransparentColor,  MIL_INT Alpha);
void DrawAlphaString(MIL_ID MilGraContext, MIL_ID MilGraList, MIL_INT Alpha);
MIL_INT MFTYPE MouseMove(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
MIL_INT MFTYPE GraphicSelected(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Print Header.
   PrintHeader();

   // Allocate defaults.
   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay;
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Allocate a graphic context.
   MIL_ID MilGraContext = MgraAlloc(MilSystem, M_NULL);

   // Restore the color image and get its size.
   MIL_ID MilImage = MbufRestore(COLOR_IMAGE_FILE, MilSystem, M_NULL);
   MIL_INT ColorImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT ColorImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Select to display.
   MdispSelect(MilDisplay, MilImage);

   // Allocate an image that has the same size as the source image and
   // clear it with the user's transparent color.
   MIL_ID MilUserOverlay = MbufAllocColor(MilSystem, 3, ColorImageSizeX, ColorImageSizeY,
      8 + M_UNSIGNED, M_IMAGE + M_PROC + M_BGR32 + M_PACKED, M_NULL);
   MbufClear(MilUserOverlay, (MIL_DOUBLE)USER_TRANSPARENT_COLOR);

   // Load the overlay image into the cleared buffer.
   MIL_INT AlphaImageSizeX = MbufDiskInquire(ALPHA_OVERLAY_IMAGE, M_SIZE_X, M_NULL);
   MIL_INT AlphaImageSizeY = MbufDiskInquire(ALPHA_OVERLAY_IMAGE, M_SIZE_Y, M_NULL);
   MIL_ID MilTransparentImageChild = MbufChild2d(MilUserOverlay,
                                                 ColorImageSizeX - AlphaImageSizeX,
                                                 0,
                                                 AlphaImageSizeX,
                                                 AlphaImageSizeY,
                                                 M_NULL);
   MbufLoad(ALPHA_OVERLAY_IMAGE, MilTransparentImageChild);
   MbufFree(MilTransparentImageChild);

   // Get the true overlay image.
   MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   MIL_INT OverlayTransparentColor = MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, M_NULL);
   
   // Perform the alpha-blending between the overlay image and the image.
   AlphaBlend(MilImage, MilOverlay, MilUserOverlay,
              USER_TRANSPARENT_COLOR, OverlayTransparentColor, INITIAL_ALPHA_VALUE);

   // Print message.
   MosPrintf(MIL_TEXT("The user overlay image has been alpha blended with the displayed\n")
             MIL_TEXT("image. Only the non-transparent pixels have been copied on the\n")
             MIL_TEXT("display's overlay.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a graphic list.
   MIL_ID MilGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   // Create the brush and color palette of the graphic list
   MgraColor(MilGraContext, (MIL_DOUBLE)PALETTE_COLORS[0]);
   MgraArcFill(MilGraContext, MilGraList, -BRUSH_RADIUS, -BRUSH_RADIUS,
      BRUSH_RADIUS, BRUSH_RADIUS, 0, 360);
   MgraControl(MilGraContext, M_INPUT_UNITS, M_DISPLAY);
   MgraControlList(MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_SELECTABLE, M_DISABLE);
   MgraColor(MilGraContext, M_COLOR_BLACK);
   MgraRectAngle(MilGraContext, MilGraList,
      0,
      0,
      SQUARE_SIZE + 2 * COLOR_SPACING,
      (NB_COLORS+1) * (SQUARE_SIZE + COLOR_SPACING) + COLOR_SPACING,
      0, M_FILLED);
   MgraControlList(MilGraList, M_GRAPHIC_INDEX(1), M_DEFAULT, M_SELECTABLE, M_DISABLE);
   for(MIL_INT c = 0; c < NB_COLORS; c++)
      {
      MgraColor(MilGraContext, (MIL_DOUBLE)PALETTE_COLORS[c]);
      MgraRectAngle(MilGraContext, MilGraList,
         COLOR_SPACING,
         COLOR_SPACING + c * (SQUARE_SIZE + COLOR_SPACING),
         SQUARE_SIZE,
         SQUARE_SIZE,
         0, M_FILLED);
      }

   // Draw the alpha control.
   MgraColor(MilGraContext, M_COLOR_WHITE);
   MgraRectAngle(MilGraContext, MilGraList,
                COLOR_SPACING,
                COLOR_SPACING + NB_COLORS * (SQUARE_SIZE + COLOR_SPACING),
                SQUARE_SIZE/2,
                SQUARE_SIZE,
                0, M_FILLED);
   MIL_INT MinusLabel = MgraInquireList(MilGraList, M_LIST, M_DEFAULT, M_LAST_LABEL, M_NULL);
   MgraRectAngle(MilGraContext, MilGraList,
                COLOR_SPACING + SQUARE_SIZE/2,
                COLOR_SPACING + NB_COLORS * (SQUARE_SIZE + COLOR_SPACING),
                SQUARE_SIZE/2,
                SQUARE_SIZE,
                0, M_FILLED);
   MIL_INT PlusLabel = MgraInquireList(MilGraList, M_LIST, M_DEFAULT, M_LAST_LABEL, M_NULL);
   MgraControl(MilGraContext, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   MgraControl(MilGraContext, M_TEXT_ALIGN_VERTICAL, M_CENTER);
   MgraControl(MilGraContext, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraControl(MilGraContext, M_SELECTABLE, M_DISABLE);
   MgraColor(MilGraContext, M_COLOR_BLACK);
   MgraText(MilGraContext, MilGraList,
      COLOR_SPACING + SQUARE_SIZE/2,
      COLOR_SPACING + SQUARE_SIZE/3 + NB_COLORS * (SQUARE_SIZE + COLOR_SPACING),
      MIL_TEXT("- a +"));
   DrawAlphaString(MilGraContext, MilGraList, INITIAL_ALPHA_VALUE);

   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   // Fill the hook structure.
   SHookData HookData;
   HookData.MilDisplay = MilDisplay;
   HookData.MilGraContext = MilGraContext;
   HookData.MilGraList = MilGraList;
   HookData.DrawingColor = PALETTE_COLORS[0];
   HookData.MilDisplayedImage = MilImage;
   HookData.MilOverlay = MilOverlay;
   HookData.MilUserOverlay = MilUserOverlay;
   HookData.OverlayTransparentColor = OverlayTransparentColor;
   HookData.Alpha = INITIAL_ALPHA_VALUE;
   HookData.MinusAlphaLabel = MinusLabel;
   HookData.PlusAlphaLabel = PlusLabel;

   // Hook a function to the display mouse.
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE, MouseMove, &HookData);

   // Hook a function to the click of a graphic object.
   MgraControlList(MilGraList, M_LIST, M_DEFAULT, M_MULTIPLE_SELECTION, M_DISABLE);
   MgraHookFunction(MilGraList, M_GRAPHIC_SELECTION_MODIFIED,
      GraphicSelected, &HookData);

   // Make the graphic list interactive.
   MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);

   // Print message.
   MosPrintf(MIL_TEXT("You can now interact with the display to modify the alpha-blended\n")
             MIL_TEXT("overlay with your mouse:\n")
             MIL_TEXT("   - Left click to select a brush color from the palette.\n")
             MIL_TEXT("   - Left click on the plus or minus to change the alpha value.\n")
             MIL_TEXT("   - Left click and move the mouse to draw some lines.\n")
             MIL_TEXT("   - Right click and move the mouse to erase areas.\n\n")
             
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Unhook a function to the click of a graphic object.
   MgraHookFunction(MilGraList, M_GRAPHIC_SELECTION_MODIFIED + M_UNHOOK,
      GraphicSelected, &HookData);

   // Unhook a function to the display mouse.
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE + M_UNHOOK,
      MouseMove, &HookData);

   // Free allocations.
   MbufFree(MilUserOverlay);
   MbufFree(MilImage);
   MgraFree(MilGraList);
   MgraFree(MilGraContext);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*****************************************************************************
// AlphaBlend. Blends the displayed image with the user overlay, coying only
//             non transparent pixels.
//*****************************************************************************
void AlphaBlend(MIL_ID MilDisplayedImage, MIL_ID MilOverlay, MIL_ID MilUserOverlay,
                MIL_INT UserTransparentColor, MIL_INT OverlayTransparentColor, MIL_INT Alpha)
   {
   if(Alpha > 0)
      {
      // Perform the alpha-blending between the overlay image and the image.
      MimArithMultiple(MilDisplayedImage, 256-Alpha, MilUserOverlay, (MIL_DOUBLE)Alpha, 256,
         MilOverlay, M_MULTIPLY_ACCUMULATE_2, M_DEFAULT);

      MbufClearCond(MilOverlay,
         M_RGB888_R(OverlayTransparentColor), 
         M_RGB888_G(OverlayTransparentColor),
         M_RGB888_B(OverlayTransparentColor),
         MilUserOverlay, M_EQUAL, (MIL_DOUBLE)UserTransparentColor);
      }
   else
      MbufClear(MilOverlay, (MIL_DOUBLE)OverlayTransparentColor);
   }

//*****************************************************************************
// MouseMove. Display callback that draws an arc each time the mouse is moved 
//            and a button is clicked.
//*****************************************************************************
MIL_INT MFTYPE MouseMove(MIL_INT HookType, MIL_ID MilEvent, void *pUserData)
   {
   SHookData* pHookData = (SHookData*) pUserData;

   // Get the color to draw the arc.
   MIL_INT ArcToDrawColor = -1;
   MIL_INT CombinationKeys;
   MdispGetHookInfo(MilEvent, M_COMBINATION_KEYS, &CombinationKeys);
   if((CombinationKeys & M_MOUSE_LEFT_BUTTON) == M_MOUSE_LEFT_BUTTON)
      ArcToDrawColor = pHookData->DrawingColor;
   else if((CombinationKeys & M_MOUSE_RIGHT_BUTTON) == M_MOUSE_RIGHT_BUTTON)
      ArcToDrawColor = USER_TRANSPARENT_COLOR;

   // Get the position of the mouse in the displayed image.
   MIL_DOUBLE MousePosBufX;
   MIL_DOUBLE MousePosBufY;
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_X, &MousePosBufX);
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_Y, &MousePosBufY);
   if(ArcToDrawColor != -1)
      {
      // Draw the arc in the image.
      MgraColor(pHookData->MilGraContext, (MIL_DOUBLE)ArcToDrawColor);
      MgraArcFill(pHookData->MilGraContext, pHookData->MilUserOverlay, MousePosBufX,
         MousePosBufY, BRUSH_RADIUS, BRUSH_RADIUS, 0, 360);

      // Refresh the overlay.
      MdispControl(pHookData->MilDisplay, M_UPDATE, M_DISABLE);
      AlphaBlend(pHookData->MilDisplayedImage, pHookData->MilOverlay,
         pHookData->MilUserOverlay, USER_TRANSPARENT_COLOR, 
         pHookData->OverlayTransparentColor, pHookData->Alpha);
      MdispControl(pHookData->MilDisplay, M_UPDATE, M_ENABLE);
      }

   // Move the brush.
   MgraControlList(pHookData->MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT,
      M_POSITION_X, MousePosBufX);
   MgraControlList(pHookData->MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT,
      M_POSITION_Y, MousePosBufY);

   return 0;
   }

//*****************************************************************************
// MouseMove. Graphic list callback get the color of the palette of the
//            selected rectangle.
//*****************************************************************************
MIL_INT MFTYPE GraphicSelected(MIL_INT HookType, MIL_ID MilEvent, void *pUserData)
   {
   SHookData* pHookData = (SHookData*) pUserData;

   // Get the label of the selected graphic.
   MIL_INT GraphicSelected;
   MgraGetHookInfo(MilEvent, M_GRAPHIC_LABEL_VALUE, &GraphicSelected);
   MIL_ID MilGraList;
   MgraGetHookInfo(MilEvent, M_GRAPHIC_LIST_ID, &MilGraList);

   // If a graphic was selected.
   if(GraphicSelected != M_NO_LABEL)
      {
      bool UpdateAlpha = true;
      if(GraphicSelected == pHookData->MinusAlphaLabel)
         pHookData->Alpha -= pHookData->Alpha > 4 ? ALPHA_INCREMENT : 0;
      else if(GraphicSelected == pHookData->PlusAlphaLabel)
         pHookData->Alpha += pHookData->Alpha < 251 ? ALPHA_INCREMENT : 0;
      else
         {
         // Set the color of the brush.
         MgraInquireList(MilGraList, M_GRAPHIC_LABEL(GraphicSelected), M_DEFAULT,
            M_COLOR + M_TYPE_MIL_INT, &pHookData->DrawingColor);
         MgraControlList(MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT,
            M_COLOR, pHookData->DrawingColor);
         UpdateAlpha = false;
         }
      MgraControlList(MilGraList, M_GRAPHIC_LABEL(GraphicSelected), M_DEFAULT,
         M_GRAPHIC_SELECTED, M_FALSE);

      // Update the alpha blending.
      if(UpdateAlpha)
         {
         MdispControl(pHookData->MilDisplay, M_UPDATE, M_DISABLE);
         MIL_INT NbGraphics = MgraInquireList(MilGraList, M_LIST, M_DEFAULT,
            M_NUMBER_OF_GRAPHICS, M_NULL);
         MgraControlList(MilGraList, M_GRAPHIC_INDEX(NbGraphics-1), M_DEFAULT,
            M_DELETE, M_DEFAULT);         
         AlphaBlend(pHookData->MilDisplayedImage, pHookData->MilOverlay,
            pHookData->MilUserOverlay, USER_TRANSPARENT_COLOR, 
            pHookData->OverlayTransparentColor, pHookData->Alpha);
         DrawAlphaString(pHookData->MilGraContext, MilGraList, pHookData->Alpha);
         MdispControl(pHookData->MilDisplay, M_UPDATE, M_ENABLE);
         }
      }
   return 0;
   }

//*****************************************************************************
// DrawAlphaString. Draw the string of the alpha value.
//*****************************************************************************
void DrawAlphaString(MIL_ID MilGraContext, MIL_ID MilGraList, MIL_INT Alpha)
   {
   MIL_TEXT_CHAR AlphaString[6];
   MosSprintf(AlphaString, 6, MIL_TEXT(" %i "), Alpha);
   MgraColor(MilGraContext, M_COLOR_BLACK);
   MgraText(MilGraContext, MilGraList,
      COLOR_SPACING + SQUARE_SIZE/2,
      COLOR_SPACING + 2*SQUARE_SIZE/3 + NB_COLORS * (SQUARE_SIZE + COLOR_SPACING),
      AlphaString);
   }
