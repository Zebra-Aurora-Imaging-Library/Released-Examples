/////////////////////////////////////////////////////////////////////////////////////////////
//
// File name:  ColorToGrayscalePCA.cpp
//
// Synopsis:   This example shows how to convert a color image to grayscale by projecting 
//             the colors on the axis resulting from a principal component analysis (PCA):
//             
//             1- Using all the color image's pixels to compute the PCA.
//
//             2- Using a subset of the color image's pixels to compute the PCA.
//
//             3- Using labels to invert the polarity of the resulting grayscale image.
//
//             4- Using the result of the PCA computed on an image to convert the colors of
//                another image.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/////////////////////////////////////////////////////////////////////////////////////////////
#include <mil.h>
 
// Images file path.
#define  IMAGE1_PATH   M_IMAGE_PATH MIL_TEXT("ColorToGrayscalePCA\\GreenBlueRedProcessed.jpg")
#define  IMAGE2_PATH   M_IMAGE_PATH MIL_TEXT("ColorToGrayscalePCA\\OrangeBluePinkProcessed.jpg")

// Display image margin.
#define DISPLAY_MARGIN_X  3

// Structure to hold rectangle data.
typedef struct
{
   MIL_DOUBLE StartX;
   MIL_DOUBLE StartY;
   MIL_DOUBLE EndX;
   MIL_DOUBLE EndY;
}STRUCT_RECTANGLE;

// Function declarations.
void PrintFailStatus(MIL_INT Status);

//******************************************************************************************
// Example description.
//******************************************************************************************
void PrintHeader()
   {
   MosPrintf(  MIL_TEXT("[EXAMPLE NAME]\n")
               MIL_TEXT("ColorToGrayscalePCA\n\n")
               MIL_TEXT("[SYNOPSIS]\n")
               MIL_TEXT("This example shows how to convert a color image to grayscale by projecting\n")
               MIL_TEXT("the colors on the principal axis resulting from a principal component analysis (PCA).\n")
               MIL_TEXT("Different results are shown, depending whether the example is using:\n\n")
               MIL_TEXT("1- All the color image's pixels to compute the PCA.\n")
               MIL_TEXT("2- A subset of the color image's pixels to compute the PCA.\n")
               MIL_TEXT("3- Labels to invert the polarity of the resulting grayscale image.\n")
               MIL_TEXT("4- The result of the PCA computed on an image to convert the colors of another image.\n\n")
               MIL_TEXT("[MODULES USED]\n")
               MIL_TEXT("Modules used: application, system, display, buffer, color,\n")
               MIL_TEXT("image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
   }

//******************************************************************************************
// Main.
//******************************************************************************************
int MosMain(void)
   { 
   MIL_ID   MilApplication = M_NULL,   // Application identifier.
            MilSystem      = M_NULL,   // System identifier.
            MilDisplay1    = M_NULL,   // Display 1 identifier.
            MilDisplay2    = M_NULL,   // Display 2 identifier.
            MilDisplay3    = M_NULL,   // Display 3 identifier.
            MilDisplay4    = M_NULL;   // Display 4 identifier.

   // Allocate MIL objects. 
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay1);

   // Allocate more displays and set them up.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay2);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay3);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay4);
   MdispZoom(MilDisplay1, 0.65, 0.65);
   MdispZoom(MilDisplay2, 0.65, 0.65);
   MdispZoom(MilDisplay3, 0.65, 0.65);
   MdispZoom(MilDisplay4, 0.65, 0.65);
   MdispControl(MilDisplay2, M_WINDOW_INITIAL_POSITION_X, 417);
   MdispControl(MilDisplay3, M_WINDOW_INITIAL_POSITION_Y, 345);
   MdispControl(MilDisplay4, M_WINDOW_INITIAL_POSITION_Y, 690);

   // Print the example header.
   PrintHeader();

   // Read image information.
   MIL_INT ImageSizeX = MbufDiskInquire(IMAGE1_PATH, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufDiskInquire(IMAGE1_PATH, M_SIZE_Y, M_NULL);
   MIL_INT ImageBands = MbufDiskInquire(IMAGE1_PATH, M_SIZE_BAND, M_NULL);
   MIL_INT Status = 0;

   // Allocate all buffers.
   MIL_ID MilImage         = MbufAllocColor(MilSystem, 3, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilMask          = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilProjectResult = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);

   MIL_ID MilDispImage1          = MbufAlloc2d(MilSystem, 2 * ImageSizeX + DISPLAY_MARGIN_X, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilMaskedProjectResult = MbufChild2d(MilDispImage1, ImageSizeX + DISPLAY_MARGIN_X, 0, ImageSizeX, ImageSizeY, M_NULL);

   MIL_ID MilDispImage2             = MbufAlloc2d(MilSystem, 2 * ImageSizeX + DISPLAY_MARGIN_X, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilInvMaskedProjectResult = MbufChild2d(MilDispImage2, ImageSizeX + DISPLAY_MARGIN_X, 0, ImageSizeX, ImageSizeY, M_NULL);

   // Clear the display images.
   MbufClear(MilDispImage1, 0);
   MbufClear(MilDispImage2, 0);

   // Load the image.
   MbufLoad(IMAGE1_PATH, MilImage);

   // 1- Perform the principal component projection.
   McolProject(MilImage, M_NULL, MilProjectResult, M_NULL, M_PRINCIPAL_COMPONENT_PROJECTION, M_DEFAULT, &Status);

   // Check success status.
   if (Status == M_SUCCESS)
   {
      // Display result.
      MdispControl(MilDisplay1, M_TITLE, MIL_TEXT("Color image A"));
      MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Result of the projection"));
      MdispSelect(MilDisplay1, MilImage);
      MdispSelect(MilDisplay2, MilProjectResult);

      MosPrintf(MIL_TEXT("1- The colors of image A were projected on the principal axis resulting from a PCA\n"));
      MosPrintf(MIL_TEXT("   performed using all the color image's pixels.\n"));
      MosPrintf(MIL_TEXT("   Notice the colored pen caps end up with somewhat similar grayscale values.\n\n"));
      MosPrintf(MIL_TEXT("You may run the example called \"DisplayColorDistribution\" if you want to see what\n"));
      MosPrintf(MIL_TEXT("the color distribution and PCA result of this image looks like.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
   }
   else
      PrintFailStatus(Status);

   // 2- Perform principal component projection using a subset of pixels.
   // Clear the mask image and draw 3 rectangular areas over the pen caps labelled M_SOURCE_LABEL.
   MbufClear(MilMask, 0);
   STRUCT_RECTANGLE  Pen1 = { 50.0, 115.0, 75.0, 210.0 },
                     Pen2 = { 305.0, 115.0, 330.0, 210.0 },
                     Pen3 = { 555.0, 115.0, 580.0, 210.0 };
   MgraColor(M_DEFAULT, M_SOURCE_LABEL);
   MgraRectFill(M_DEFAULT, MilMask, Pen1.StartX, Pen1.StartY, Pen1.EndX, Pen1.EndY);
   MgraRectFill(M_DEFAULT, MilMask, Pen2.StartX, Pen2.StartY, Pen2.EndX, Pen2.EndY);
   MgraRectFill(M_DEFAULT, MilMask, Pen3.StartX, Pen3.StartY, Pen3.EndX, Pen3.EndY);

   // Perform the principal component projection.
   McolProject(MilImage, MilMask, MilMaskedProjectResult, M_NULL, M_PRINCIPAL_COMPONENT_PROJECTION, M_DEFAULT, &Status);

   // Check success status.
   if (Status == M_SUCCESS)
   {
      // Display result.
      MdispControl(MilDisplay3, M_TITLE, MIL_TEXT("On the left, subset of pixels of image A used to compute the PCA. On the right, result of the projection of image A"));
      MdispSelect(MilDisplay3, MilDispImage1);
      MIL_ID OverlayId1 = MdispInquire(MilDisplay3, M_OVERLAY_ID, M_NULL);
      MbufCopyCond(MilImage, OverlayId1, MilMask, M_NOT_EQUAL, 0);

      MosPrintf(MIL_TEXT("2- The colors of image A were projected on the principal axis resulting from a PCA\n"));
      MosPrintf(MIL_TEXT("   performed using a subset of the color image's pixels.\n"));
      MosPrintf(MIL_TEXT("   Notice the colored pen caps end up with more distinct grayscale values.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
   }
   else
      PrintFailStatus(Status);

   // 3- Use M_BRIGHT_LABEL and M_DARK_LABEL labels to invert the polarity of the resulting grayscale image.
   // Draw rectangular areas labelled M_DARK_LABEL and M_BRIGHT_LABEL in the mask
   STRUCT_RECTANGLE  Dark     = { 55.0, 160.0, 70.0, 175.0 },
                     Bright   = { 560.0, 160.0, 575.0, 175.0 };
   MgraColor(M_DEFAULT, M_DARK_LABEL);
   MgraRectFill(M_DEFAULT, MilMask, Dark.StartX, Dark.StartY, Dark.EndX, Dark.EndY);
   MgraColor(M_DEFAULT, M_BRIGHT_LABEL);
   MgraRectFill(M_DEFAULT, MilMask, Bright.StartX, Bright.StartY, Bright.EndX, Bright.EndY);

   // Perform the principal component projection.
   McolProject(MilImage, MilMask, MilInvMaskedProjectResult, M_NULL, M_PRINCIPAL_COMPONENT_PROJECTION, M_DEFAULT, &Status);

   // Check success status.
   if (Status == M_SUCCESS)
   {
      // Display result.
      MdispControl(MilDisplay4, M_TITLE, MIL_TEXT("On the left, polarity labels were added to the mask. On the right, result of the projection of image A"));
      MdispSelect(MilDisplay4, MilDispImage2);
      MIL_ID OverlayId2 = MdispInquire(MilDisplay4, M_OVERLAY_ID, M_NULL);
      MbufCopyCond(MilImage, OverlayId2, MilMask, M_NOT_EQUAL, 0);
      
      // Draw the labelled regions in the overlay.
      MgraColor(M_DEFAULT, 64);
      MgraRectFill(M_DEFAULT, OverlayId2, Dark.StartX, Dark.StartY, Dark.EndX, Dark.EndY);
      MgraColor(M_DEFAULT, 192);
      MgraRectFill(M_DEFAULT, OverlayId2, Bright.StartX, Bright.StartY, Bright.EndX, Bright.EndY);

      // Draw text in the overlay.
      MgraControl(M_DEFAULT, M_FONT_X_SCALE, 1.5);
      MgraControl(M_DEFAULT, M_FONT_Y_SCALE, 1.5);
      MgraText(M_DEFAULT, OverlayId2, 10, 250, MIL_TEXT("Dark label"));
      MgraText(M_DEFAULT, OverlayId2, 495, 250, MIL_TEXT("Bright label"));

      MosPrintf(MIL_TEXT("3- Polarity labels can be used to invert the polarity of the resulting grayscale image.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
   }
   else
      PrintFailStatus(Status);

   // Clear Overlays.
   MdispControl(MilDisplay4, M_OVERLAY_CLEAR, M_DEFAULT);

   // Stop displaying on Display4.
   MdispSelect(MilDisplay4, M_NULL);

   // Allocate a second image.
   MIL_ID MilImage2 = MbufAllocColor(MilSystem, ImageBands, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MbufLoad(IMAGE2_PATH, MilImage2);

   // Allocate a new larger display image.
   MIL_ID MilDispImage3 = MbufAlloc2d(MilSystem, 3 * ImageSizeX + 2 * DISPLAY_MARGIN_X, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilProjectResult2 = MbufChild2d(MilDispImage3, ImageSizeX + DISPLAY_MARGIN_X, 0, ImageSizeX, ImageSizeY, M_NULL);
   MIL_ID MilProjectResult3 = MbufChild2d(MilDispImage3, 2 * ImageSizeX + 2 * DISPLAY_MARGIN_X, 0, ImageSizeX, ImageSizeY, M_NULL);
   MIL_ID ProjectionMatrix = MbufAlloc1d(MilSystem, 4, 32 + M_FLOAT, M_ARRAY, M_NULL);
   MbufClear(MilDispImage3, 0);

   // Draw the regions in the mask to get rid of the polarity labels.
   MgraColor(M_DEFAULT, M_SOURCE_LABEL);
   MgraRectFill(M_DEFAULT, MilMask, Pen1.StartX, Pen1.StartY, Pen1.EndX, Pen1.EndY);
   MgraRectFill(M_DEFAULT, MilMask, Pen2.StartX, Pen2.StartY, Pen2.EndX, Pen2.EndY);
   MgraRectFill(M_DEFAULT, MilMask, Pen3.StartX, Pen3.StartY, Pen3.EndX, Pen3.EndY);

   // First method to perform principal component projection (direct method).
   McolProject(MilImage2, MilMask, MilProjectResult2, M_NULL, M_PRINCIPAL_COMPONENT_PROJECTION, M_DEFAULT, &Status);
   // Check success status.
   if (Status == M_SUCCESS)
   {
      // Prepare displays.
      MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Color image B"));
      MdispControl(MilDisplay4, M_TITLE, MIL_TEXT("On the left, subset of pixels of image B used to compute the PCA. In the middle, result of the projection of image B"));

      // Display result image B.
      MdispSelect(MilDisplay2, MilImage2);
      MdispSelect(MilDisplay4, MilDispImage3);
      MIL_ID OverlayId2 = MdispInquire(MilDisplay4, M_OVERLAY_ID, M_NULL);
      MbufCopyCond(MilImage2, OverlayId2, MilMask, M_EQUAL, M_SOURCE_LABEL);

      MosPrintf(MIL_TEXT("4- The colors of images A and B were projected on the principal axis resulting from their respective PCA.\n"));
      MosPrintf(MIL_TEXT("Notice how the color blue gets projected on different grayscale values.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Compute the PCA of image A and get the resulting color projection matrix.
      McolProject(MilImage, MilMask, ProjectionMatrix, M_NULL, M_PRINCIPAL_COMPONENT_PROJECTION, M_DEFAULT, &Status);

      // Apply the color projection matrix to image B
      MimConvert(MilImage2, MilProjectResult3, ProjectionMatrix);

      MdispControl(MilDisplay4, M_TITLE, MIL_TEXT("On the left, subset of pixels of image B used to compute the PCA.")
                                         MIL_TEXT("In the middle, result of the projection of image B. On the right,")
                                         MIL_TEXT("result of the projection of image B using the PCA result of image A"));

      MosPrintf(MIL_TEXT("The colors of image B were projected on the principal axis resulting from the PCA performed on image A.\n"));
      MosPrintf(MIL_TEXT("Notice the color blue now gets projected on the same grayscale value.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
   }
   else
      PrintFailStatus(Status);

   // Free resources.
   MbufFree(MilProjectResult);
   MbufFree(MilProjectResult2);
   MbufFree(MilProjectResult3);
   MbufFree(MilInvMaskedProjectResult);
   MbufFree(MilMaskedProjectResult);
   MbufFree(MilDispImage1);
   MbufFree(MilDispImage2);
   MbufFree(MilDispImage3);
   MbufFree(ProjectionMatrix);
   MbufFree(MilMask);
   MbufFree(MilImage);
   MbufFree(MilImage2);
   MdispFree(MilDisplay1);
   MdispFree(MilDisplay2);
   MdispFree(MilDisplay3);
   MdispFree(MilDisplay4);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   }

//******************************************************************************************
// Print Success status
//******************************************************************************************
void PrintFailStatus(MIL_INT Status)
   {
   MosPrintf(MIL_TEXT("The color projection failed!\n"));
   if(Status == M_NO_SOURCE_DEFINED)
      MosPrintf(MIL_TEXT("No pixels were set to M_SOURCE_LABEL in the data identification image.\n"));
   else if (Status == M_UNSTABLE_POLARITY)
      MosPrintf(MIL_TEXT("The polarity of the projection is unstable.\n"));
   MosPrintf(MIL_TEXT("Press ENTER to continue"));
   MosGetch();
   }
