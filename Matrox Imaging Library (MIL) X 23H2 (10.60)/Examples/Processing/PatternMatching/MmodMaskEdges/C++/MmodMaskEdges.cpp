////////////////////////////////////////////////////////////////////////////////////////
//
// File name:    MmodMaskEdges.cpp
//
// Synopsis:    This program lets you interactively creates a "don't care" mask for
//              a Geometric Model Finder model.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//

#include <mil.h>

#define IMAGE_FILE M_IMAGE_PATH MIL_TEXT("SingleModel.mim")

// Model specifications.
#define MODEL_OFFSETX 176L
#define MODEL_OFFSETY 136L
#define MODEL_SIZEX   128L
#define MODEL_SIZEY   128L

// Data structure to be passed to the hook functions for example #1.
struct HookDataExample1
   {
   MIL_ID  MilDisplay;
   MIL_ID  MaskImage;
   MIL_ID  ResultBlobId;
   MIL_ID  ZoneOfInfluenceImage;
   MIL_ID  MilOverlayImage;
   MIL_INT MaskColor;
   MIL_INT TransparentColor;
   };

// Data structure to be passed to the hook functions for example #2.
struct HookDataExample2
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
   MIL_INT BrushSize;
   };

// Example #1 Prototypes.
MIL_INT MFTYPE SelectOrUnselectEdges(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
void MaskEdgesExample1(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID ModContext, MIL_ID MilGraphicList, MIL_ID MilOverlayImage);

// Example #2 Prototypes.
MIL_INT MFTYPE StopDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
MIL_INT MFTYPE ClearMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
MIL_INT MFTYPE StartDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
MIL_INT MFTYPE DrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr);
void MaskEdgesExample2(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID ModContext, MIL_ID MilGraphicList, MIL_ID MilOverlayImage);

// Example description.
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MmodMaskEdges\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example lets you interactively creates a \"don't care\" mask\n")
            MIL_TEXT("for a Geometric Model Finder model using two approaches:\n")
            MIL_TEXT("1 - Select edges to mask.\n")
            MIL_TEXT("2 - Draw regions over edges to mask.\n\n"));
   }

int MosMain(void)
   {
   PrintHeader();

   // Allocate defaults.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, MIL_TEXT("M_SYSTEM_HOST"), M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_GRA_ID MilGraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Associate the graphic list to the display. 
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Restore and display the model image.
   MIL_UNIQUE_BUF_ID SingleModel = MbufRestore(IMAGE_FILE, MilSystem, M_UNIQUE_ID);
   MdispSelect(MilDisplay, SingleModel);

   // Allocate a Geometric Model Finder context and a result buffer.
   MIL_UNIQUE_MOD_ID ModContext = MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_MOD_ID ModResult = MmodAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Define the model from the the model image.
   MmodDefine(ModContext, M_IMAGE, SingleModel, MODEL_OFFSETX, MODEL_OFFSETY, MODEL_SIZEX, MODEL_SIZEY);

   // Set the detail level to high to extract some small edges for masking purpose.
   MmodControl(ModContext, M_CONTEXT, M_DETAIL_LEVEL, M_HIGH);

   // Draw the model position and box.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmodDraw(M_DEFAULT, ModContext, MilGraphicList, M_DRAW_BOX + M_DRAW_POSITION, M_DEFAULT, M_ORIGINAL);

   MosPrintf(MIL_TEXT("A model context was defined with the model in the displayed image.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Deselect the model image from the display.
   MdispSelect(MilDisplay, M_NULL);

   // Clear the graphiclist.
   MgraClear(M_DEFAULT, MilGraphicList);

   // Extract the model.
   MIL_UNIQUE_BUF_ID Model = MbufAlloc2d(MilSystem, MODEL_SIZEX, MODEL_SIZEY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MmodDraw(M_DEFAULT, ModContext, Model, M_DRAW_IMAGE, M_DEFAULT, M_DEFAULT);

   // Zoom-in the display to facilitate selecting edges.
   MdispZoom(MilDisplay, 4, 4);

   // Display the model.
   MdispSelect(MilDisplay, Model);

   // Inquire the overlay.
   MIL_ID MilOverlayImage = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);

   // Run example #1 - Select edges to apply masking.
   MaskEdgesExample1(MilSystem, MilDisplay, ModContext, MilGraphicList, MilOverlayImage);

   // Delete the model that was masked by example #1.
   MmodDefine(ModContext, M_DELETE, 0, M_DEFAULT, M_DEFAULT, M_DEFAULT,  M_DEFAULT);

   // Redefine the model for example #2.
   MmodDefine(ModContext, M_IMAGE, SingleModel, MODEL_OFFSETX, MODEL_OFFSETY, MODEL_SIZEX, MODEL_SIZEY);

   // Run example #2 - Draw regions over edges to mask.
   MaskEdgesExample2(MilSystem, MilDisplay, ModContext, MilGraphicList, MilOverlayImage);

   return 0;
   }

// Example #1 - Select edges to apply masking.
void MaskEdgesExample1(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID ModContext, MIL_ID MilGraphicList, MIL_ID MilOverlayImage)
   {
   MosPrintf(MIL_TEXT("Example 1: Select edges to mask.\n"));
   MosPrintf(MIL_TEXT("--------------------------------\n\n"));

   // Draw the edges in the graphic list.
   MmodDraw(M_DEFAULT, ModContext, MilGraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);

   // Define a 16 bit image for the input edges of MimZoneOfInfluence().
   MIL_UNIQUE_BUF_ID ModelEdgeImage = MbufAlloc2d(MilSystem, MODEL_SIZEX, MODEL_SIZEY, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   // Clear the image.
   MbufClear(ModelEdgeImage, 0);

   // Set the color to 2^16.
   MgraColor(M_DEFAULT, 65535);

   // Draw the model edges into the image.
   MmodDraw(M_DEFAULT, ModContext, ModelEdgeImage, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);

   // Allocate the result buffer for MimZoneOfInfluence().
   MIL_UNIQUE_BUF_ID ZoneOfInfluenceImage = MbufAlloc2d(MilSystem, MODEL_SIZEX, MODEL_SIZEY, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   // Perform a zone of influence detection.
   MimZoneOfInfluence(ModelEdgeImage, ZoneOfInfluenceImage, M_DEFAULT);

   // Perform a logical AND between the edge image and the zone of influence image so each edge takes the color of its zone of influence.
   MimArith(ModelEdgeImage, ZoneOfInfluenceImage, ModelEdgeImage, M_AND);

   // Allocate a blob context and resut buffer.
   MIL_UNIQUE_BLOB_ID MilBlobContext = MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BLOB_ID MilBlobResult = MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   //Calculate the blobs using the labelled mode.
   MblobControl(MilBlobContext, M_BLOB_IDENTIFICATION_MODE, M_LABELED_TOUCHING);
   MblobCalculate(MilBlobContext, ModelEdgeImage, M_NULL, MilBlobResult);

   // Allocate an edge mask image.
   MIL_UNIQUE_BUF_ID MaskImage = MbufAlloc2d(MilSystem, MODEL_SIZEX, MODEL_SIZEY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   // Clear the mask image.
   MbufClear(MaskImage, 0X0);

   // Assign data to the hook data struct.
   HookDataExample1 HookDataStruct;
   HookDataStruct.MilDisplay = MilDisplay;
   HookDataStruct.ResultBlobId = MilBlobResult;
   HookDataStruct.ZoneOfInfluenceImage = ZoneOfInfluenceImage;
   HookDataStruct.MaskImage = MaskImage;
   HookDataStruct.MilOverlayImage = MilOverlayImage;
   HookDataStruct.MaskColor = M_COLOR_RED;
   HookDataStruct.TransparentColor = MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, M_NULL);

   MosPrintf(MIL_TEXT("The model and its edges are displayed.\n\n"));
   MosPrintf(MIL_TEXT("Use the mouse to select the edges to mask:\n"));
   MosPrintf(MIL_TEXT("- Left-click to mask the edge closest to the mouse cursor.\n"));
   MosPrintf(MIL_TEXT("- Right-click to unmask the edge closest to the mouse cursor.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to finish masking.\n\n"));

   // Hook a function when the left button is click or released to mask edges.
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP, SelectOrUnselectEdges, &HookDataStruct);

   // Hook a function when the right button is click or released to unslected the masked edges.
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP, SelectOrUnselectEdges, &HookDataStruct);

   // Finish masking if <Enter> is pressed.
   MIL_INT Ch = 0;
   while (Ch != '\r')
      {
      Ch = MosGetch();
      }

   // Unhook functions from display.
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, SelectOrUnselectEdges, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP + M_UNHOOK, SelectOrUnselectEdges, &HookDataStruct);

   // Clear the annotations.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MgraClear(M_DEFAULT, MilGraphicList);

   // Apply the mask to the model.
   MmodMask(ModContext, M_DEFAULT, MaskImage, M_DONT_CARE, M_DEFAULT);

   // Draw the final edges excluding masked edges.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmodDraw(M_DEFAULT, ModContext, MilGraphicList, M_DRAW_EDGE, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("A \"don't care\" mask image was created and associated to the model.\n")
             MIL_TEXT("Unmasked model edges are displayed.\n\n"));

   MosPrintf(MIL_TEXT("Press <ENTER> to continue.\n\n"));
   MosGetch();
   }

// Hook function when the left or right button is clicked or released.
MIL_INT MFTYPE SelectOrUnselectEdges(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   MIL_DOUBLE PosX, PosY;

   HookDataExample1* HookDataPtr = (HookDataExample1*)UserDataPtr;

   // Inquire the mouse position in the displayed buffer.
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);

   if (PosX < MODEL_SIZEX && PosX > 0 && PosY < MODEL_SIZEY && PosY > 0)
      {
      // Extract the label using the mouse clicked position.
      MIL_INT SelectBlobLabel = 0, DisplayMaskColor = 0, BufferMaskColor = 0;

      // Select the blob that is the closest to the mouse clicked position.
      MbufGet2d(HookDataPtr->ZoneOfInfluenceImage, (MIL_INT)(PosX + 0.5), (MIL_INT)(PosY + 0.5), 1, 1, &SelectBlobLabel);
            
      if (SelectBlobLabel)
         {
         // Set proper colors to draw the masks if the left mouse button is released.
         if (HookType == M_MOUSE_LEFT_BUTTON_UP)
            {
            DisplayMaskColor = HookDataPtr->MaskColor;
            BufferMaskColor = 0x1;
            }
         // Set proper colors to erase the masks if the right mouse button is released.
         else if (HookType == M_MOUSE_RIGHT_BUTTON_UP)
            {
            DisplayMaskColor = HookDataPtr->TransparentColor;
            BufferMaskColor = 0x0;
            }

         // Draw or erase the selected edge in the display overlay.
         MgraColor(M_DEFAULT, (MIL_DOUBLE)DisplayMaskColor);
         MblobDraw(M_DEFAULT, HookDataPtr->ResultBlobId, HookDataPtr->MilOverlayImage, M_DRAW_BLOBS, M_BLOB_LABEL(SelectBlobLabel), M_DEFAULT);

         // Draw or erase the selected edge in the mask image.
         MgraColor(M_DEFAULT, (MIL_DOUBLE)BufferMaskColor);
         MblobDraw(M_DEFAULT, HookDataPtr->ResultBlobId, HookDataPtr->MaskImage, M_DRAW_BLOBS, M_BLOB_LABEL(SelectBlobLabel), M_DEFAULT);
         }
      }

   return 0;
   }

void MaskEdgesExample2(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID ModContext, MIL_ID MilGraphicList, MIL_ID MilOverlayImage)
   {
   MosPrintf(MIL_TEXT("Example 2: Draw regions over edges to mask.\n"));
   MosPrintf(MIL_TEXT("-------------------------------------------\n\n"));

   // Draw the edges in green in the graphic list.
   MmodDraw(M_DEFAULT, ModContext, MilGraphicList, M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);

   // Allocate a mask image.
   MIL_UNIQUE_BUF_ID MaskImage = MbufAlloc2d(MilSystem, MODEL_SIZEX, MODEL_SIZEY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(MaskImage, 0X0);

   // Assign data to the hood data struct.
   HookDataExample2 HookDataStruct;
   HookDataStruct.MilDisplay = MilDisplay;
   HookDataStruct.MilGraphicList = MilGraphicList;
   HookDataStruct.MaskImage = MaskImage;
   HookDataStruct.MilOverlayImage = MilOverlayImage;
   HookDataStruct.DoDraw = false;
   HookDataStruct.DoErase = false;
   HookDataStruct.MaskColor = M_COLOR_RED;
   HookDataStruct.TransparentColor = MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, M_NULL);
   HookDataStruct.BrushSize = 2;

   MosPrintf(MIL_TEXT("The model and its edges are displayed.\n\n"));
   MosPrintf(MIL_TEXT("Use the mouse to draw over the edges to mask:\n"));
   MosPrintf(MIL_TEXT("- Click and hold the left mouse button and drag the mouse to apply the mask.\n"));
   MosPrintf(MIL_TEXT("- Click and hold the right mouse button and drag the mouse to erase the mask.\n"));
   MosPrintf(MIL_TEXT("- Press 'C' or 'c' to clear the entire mask.\n\n"));
   MosPrintf(MIL_TEXT("Press <ENTER> to finish masking.\n\n"));

   // Hook functions from display.
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_DOWN, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE, DrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_KEY_CHAR, ClearMask, &HookDataStruct);

   // Finish masking if <Enter> is pressed.
   MIL_INT Ch = 0;
   while (Ch != '\r')
      {
      Ch = MosGetch();
      }

   // Unhook functions from display.
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_DOWN + M_UNHOOK, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN + M_UNHOOK, StartDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_MOVE + M_UNHOOK, DrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_MOUSE_RIGHT_BUTTON_UP + M_UNHOOK, StopDrawOrEraseMask, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_KEY_CHAR + M_UNHOOK, ClearMask, &HookDataStruct);

   // Clear the annotations.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   MgraClear(M_DEFAULT, MilGraphicList);

   // Apply the mask to the model.
   MmodMask(ModContext, M_DEFAULT, MaskImage, M_DONT_CARE, M_DEFAULT);

   // Draw the final edges excluding masked edges.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmodDraw(M_DEFAULT, ModContext, MilGraphicList, M_DRAW_EDGE, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("A \"don't care\" mask image was created and associated to the model.\n")
      MIL_TEXT("Unmasked model edges are displayed.\n\n"));

   MosPrintf(MIL_TEXT("Press <ENTER> to end.\n\n"));
   MosGetch();

   return;
   }

// This function is called when a mouse click down event is trapped to start masking or start erasing the mask.
MIL_INT MFTYPE StartDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   MIL_DOUBLE PosX, PosY;

   // Inquire the mouse position in the displayed buffer. 
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);

   HookDataExample2* HookDataPtr = (HookDataExample2*)UserDataPtr;

   MIL_INT DisplayMaskColor = 0, BufferMaskColor = 0;

   if (HookType == M_MOUSE_LEFT_BUTTON_DOWN)
      {
      // Set the draw flag and mask color.
      HookDataPtr->DoDraw = true;
      DisplayMaskColor = HookDataPtr->MaskColor;
      BufferMaskColor = 0x1;
      }
   else if (HookType == M_MOUSE_RIGHT_BUTTON_DOWN)
      {
      // Set the erase flag and mask color.
      HookDataPtr->DoErase = true;
      DisplayMaskColor = HookDataPtr->TransparentColor;
      BufferMaskColor = 0x0;
      }

   // Draw or erase a filled circle in the display overlay.
   MgraColor(M_DEFAULT, (MIL_DOUBLE)DisplayMaskColor);
   MgraArcFill(M_DEFAULT, HookDataPtr->MilOverlayImage, PosX, PosY, HookDataPtr->BrushSize, HookDataPtr->BrushSize, 0, 360);

   // Draw or erase a filled circle in the mask image.
   MgraColor(M_DEFAULT, (MIL_DOUBLE)BufferMaskColor);
   MgraArcFill(M_DEFAULT, HookDataPtr->MaskImage, PosX, PosY, HookDataPtr->BrushSize, HookDataPtr->BrushSize, 0, 360);

   return 0;
   }


// This function is called when a mouse move event is trapped to continue masking or continue erasing the mask.
MIL_INT MFTYPE DrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   MIL_DOUBLE PosX, PosY;

   // Inquire the mouse position in the displayed buffer.
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
   MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);

   HookDataExample2* HookDataPtr = (HookDataExample2*)UserDataPtr;

   if ((HookDataPtr->DoDraw || HookDataPtr->DoErase))
      {
      MIL_INT DisplayMaskColor = 0, BufferMaskColor = 0;

      if (HookDataPtr->DoDraw)
         {
         // Set proper color to continue masking.
         DisplayMaskColor = HookDataPtr->MaskColor;
         BufferMaskColor = 0x1;
         }
      else if (HookDataPtr->DoErase)
         {
         // Set proper color to continue erasing mask.
         DisplayMaskColor = HookDataPtr->TransparentColor;
         BufferMaskColor = 0x0;
         }
 
      // Draw or erase a filled circle in the overlay.
      MgraColor(M_DEFAULT, (MIL_DOUBLE)DisplayMaskColor);
      MgraArcFill(M_DEFAULT, HookDataPtr->MilOverlayImage, PosX, PosY, HookDataPtr->BrushSize, HookDataPtr->BrushSize, 0, 360);

      // Draw or erase a filled circle in the mask image.
      MgraColor(M_DEFAULT, (MIL_DOUBLE)BufferMaskColor);
      MgraArcFill(M_DEFAULT, HookDataPtr->MaskImage, PosX, PosY, HookDataPtr->BrushSize, HookDataPtr->BrushSize, 0, 360);
      }

   return 0;
   }

// This function is called when a mouse click up event is trapped to stop masking or stop erasing the mask.
MIL_INT MFTYPE StopDrawOrEraseMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   HookDataExample2* HookDataPtr = (HookDataExample2*)UserDataPtr;

   // Reset the Draw flag when a mouse left button is released.
   if (HookDataPtr->DoDraw)
      {
      HookDataPtr->DoDraw = false;
      }
   // Reset the Erase flag when a mouse right button is released.
   else if (HookDataPtr->DoErase)
      {
      HookDataPtr->DoErase = false;
      }
    
   return 0;
   }

// This function is called when 'C' or 'c' is pressed to clear the mask.
MIL_INT MFTYPE ClearMask(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr)
   {
   HookDataExample2* HookDataPtr = (HookDataExample2*)UserDataPtr;

   MIL_INT KeyVal;
   MdispGetHookInfo(EventId, M_KEY_VALUE, &KeyVal);

   if (KeyVal == 'c' || KeyVal == 'C')
      {
      // Clear the mask image and annotations in the overlay.
      MdispControl(HookDataPtr->MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
      MbufClear(HookDataPtr->MaskImage, 0x0);
      }

   return 0;
   }
