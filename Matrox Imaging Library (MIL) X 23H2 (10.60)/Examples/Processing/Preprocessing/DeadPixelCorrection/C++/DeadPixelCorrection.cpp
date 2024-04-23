//***************************************************************************************
// 
// File name: DeadPixelCorrection.cpp  
//
// Synopsis:  This program performs a dead pixel correction operation.
//            See the PrintHeader() function below for detailed description.
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("DeadPixelCorrection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to correct dead pixels in an image.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, graphic, image processing.\n"));
   }

///****************************************************************************
// Constants.
//*****************************************************************************

// Source image files specification. 
#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("CircuitsBoard.mim")  // Image to use.
#define BOX_SIZE     12.0 // Used to draw a box around the dead pixels.

//****************************************************************************
// DrawBoxAroundDeadPixels declarations.
//****************************************************************************
void DrawBoxAroundDeadPixels(MIL_ID     DisplayOverlay, 
                             MIL_DOUBLE BoxColor, 
                             MIL_INT    NumberOfDeadPixel, 
                             MIL_INT    PixelPosX[], 
                             MIL_INT    PixelPosY[]);


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID   MilApplication,         // Application identifier.
            MilSystem,              // System identifier.     
            MilDisplay,             // Display identifier.
            MilDeadPixelContext,    // DeadPixelContext identifier.
            MilOverlayImage,        // Image buffer overlay.
            MilSourceImage;         // Image buffer identifier.

   // Coordinates of the dead pixels.
   MIL_INT DeadPixelArrayX[] = {100,  65, 245, 404, 404, 404, 404, 404, 404, 405, 403, 403, 404, 145, 123, 123, 481, 476};
   MIL_INT DeadPixelArrayY[] = {150, 156, 168, 113, 114, 115, 116, 117, 118, 118, 118, 119, 119,  32, 124, 123, 442, 476};
   MIL_INT NumberOfDeadPixel = sizeof(DeadPixelArrayX)/sizeof(MIL_INT);

   // Allocate MIL objects.
   MappAlloc(M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Print Header. 
   PrintHeader();

   // Restore the source image and simulate dead pixels.
   MbufRestore(IMAGE_FILE, MilSystem, &MilSourceImage);

   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraDots(M_DEFAULT, MilSourceImage, NumberOfDeadPixel, DeadPixelArrayX, DeadPixelArrayY, M_DEFAULT);

   // Display the source image and annotations.
   MdispSelect(MilDisplay, MilSourceImage);
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MilOverlayImage = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   DrawBoxAroundDeadPixels(MilOverlayImage, M_COLOR_RED, NumberOfDeadPixel, DeadPixelArrayX, DeadPixelArrayY);
   
   // Print a message.
   MosPrintf(MIL_TEXT("A source image with dead pixels is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a dead pixel correction context.
   MimAlloc(MilSystem, M_DEAD_PIXEL_CONTEXT, M_DEFAULT, &MilDeadPixelContext);

   // Set the list of the dead pixels coordinates to the context.
   MimPut(MilDeadPixelContext, M_XY_DEAD_PIXELS+M_TYPE_MIL_INT , NumberOfDeadPixel, DeadPixelArrayX, DeadPixelArrayY, M_DEFAULT);

   // Correct the dead pixels
   MimDeadPixelCorrection(MilDeadPixelContext, MilSourceImage, MilSourceImage, M_DEFAULT);
   
   // Display the corrected image.
   DrawBoxAroundDeadPixels(MilOverlayImage, M_COLOR_GREEN, NumberOfDeadPixel, DeadPixelArrayX, DeadPixelArrayY);

   //Display message.
   MosPrintf(MIL_TEXT("The dead pixels have been corrected.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to terminate.\n\n"));
   MosGetch();

   // Release the allocated MIL objects.
   MimFree(MilDeadPixelContext);
   MbufFree(MilSourceImage);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//****************************************
// Draw box annotation around dead pixels.
//****************************************
void DrawBoxAroundDeadPixels(MIL_ID     DisplayOverlay, 
                             MIL_DOUBLE BoxColor, 
                             MIL_INT    NumberOfDeadPixel, 
                             MIL_INT    PixelPosX[], 
                             MIL_INT    PixelPosY[])
   {
   bool* ActivePixel = new bool[NumberOfDeadPixel];
   for (MIL_INT Pixel = 0; Pixel < NumberOfDeadPixel; Pixel++)
      ActivePixel[Pixel]=true;

   MgraColor(M_DEFAULT, BoxColor);
   for (MIL_INT Pixel = 0; Pixel < NumberOfDeadPixel; Pixel++)
      {
      if(ActivePixel[Pixel])
         {
         MIL_DOUBLE BoxStartX = PixelPosX[Pixel]-BOX_SIZE;
         MIL_DOUBLE BoxStartY = PixelPosY[Pixel]-BOX_SIZE;
         MIL_DOUBLE BoxEndX   = PixelPosX[Pixel]+BOX_SIZE;
         MIL_DOUBLE BoxEndY   = PixelPosY[Pixel]+BOX_SIZE;

         MgraRect(M_DEFAULT, DisplayOverlay, BoxStartX, BoxStartY, BoxEndX, BoxEndY);

         for (MIL_INT NextPixel = Pixel+1; NextPixel < NumberOfDeadPixel; NextPixel++)
            {
            bool InBoxRangeX = (PixelPosX[NextPixel]>BoxStartX)&&(PixelPosX[NextPixel]<BoxEndX);
            bool InBoxRangeY = (PixelPosY[NextPixel]>BoxStartY)&&(PixelPosY[NextPixel]<BoxEndY);
            ActivePixel[NextPixel]=!(InBoxRangeX&&InBoxRangeY);
            }
         }
      }
   delete[] ActivePixel;
   }
