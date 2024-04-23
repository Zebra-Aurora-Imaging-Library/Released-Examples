﻿////////////////////////////////////////////////////////////////////////////
//
// File name: SegmentationAndAnalysisOfCells.cpp
//
// Synopsis:  This program shows how to use the blob reconstruction operation
//            to segment objects using an hysteresis thresholding technique.
//             - the image is filtered to remove the noise
//             - the reconstruction from seed operation is used to perform a robust 
//               segmentation of the objects
//             - blob features are calculated to select specific cells
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
////////////////////////////////////////////////////////////////////////////

#include <mil.h>  
 
// Target MIL image file specifications.  
#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("/BlobAnalysis/ElongatedCells.mim")

#define PIXEL_SIZE_X             0.15 // 1 pixel size x = 0.15 mm
#define PIXEL_SIZE_Y             0.17 // 1 pixel size y = 0.17 mm
#define LOW_THRESHOLD_VALUE      165L 
#define HIGH_THRESHOLD_VALUE     230L 
#define MINIMUM_AREA             4.59 // mm^2 
#define MAXIMUM_AREA             10.2 // mm^2 
#define MINIMUM_FERET_ELONGATION 2.00 

int MosMain(void)
   {
   MIL_ID     MilApplication,                // Application identifier.
              MilSystem,                     // System identifier.
              MilDisplay,                    // Display identifier.
              MilImage,                      // Image buffer identifier.
              MilImageToDisplay,             // Image to display identifier.
              MilGraphicList,                // Graphic list identifier.
              MilDestImage,                  // Image buffer identifier.
              MilBinLowImage,                // Binary image buffer identifier.
              MilBinHighImage,               // Binary image buffer identifier.
              MilBinImage,                   // Binary image buffer identifier.
              MilBlobResult,                 // Blob result buffer identifier.
              MilBlobContext;                // Context identifier.

   MIL_INT    SizeX,                         // Size X of the source buffer.
              SizeY,                         // Size Y of the source buffer.
              NumberOfBlobs;                 // Number of blobs.

   MIL_DOUBLE *AreaArray,                    // Blob areas.
              *CogXArray,                    // Blob centers of gravity X.
              *CogYArray,                    // Blob centers of gravity Y.
              *ElongationArray;              // Blob elongations.

   MIL_INT    i;

   // Allocations.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem); 
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Restore source image into image buffer.
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);

   // Allocate processing image buffers
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);

   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC, &MilDestImage);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinLowImage);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinHighImage);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

   // Allocate image to be displayed
   MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MilImageToDisplay);
   MbufCopy(MilImage, MilImageToDisplay);

   // Allocate a graphic list and associate it to the display.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Display the buffer.
   MdispSelect(MilDisplay, MilImageToDisplay);

   MosPrintf(MIL_TEXT("\nOBJECT ANALYSIS USING BLOB RECONSTRUCTION\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("This program identifies the isolated cells in an image.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Image noise reduction
   MIL_ID MilLinearFilterIIRContext = MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS, 50);
   MimConvolve(MilImage, MilDestImage, MilLinearFilterIIRContext);
   MimFree(MilLinearFilterIIRContext);

   // Display the result
   MbufCopy(MilDestImage, MilImageToDisplay);
   MosPrintf(MIL_TEXT("The image is smoothed to reduce noise.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
 
   // Binarize the smoothed image using a low threshold value./
   MimBinarize(MilDestImage, MilBinLowImage, M_FIXED+M_GREATER, LOW_THRESHOLD_VALUE, M_NULL);

   // Binarize the smoothed using a high threshold value.
   MimBinarize(MilDestImage, MilBinHighImage, M_FIXED+M_GREATER, HIGH_THRESHOLD_VALUE, M_NULL);

   // Display the binarize results
   MbufClear(MilImageToDisplay, 0L);
   MbufClearCond(MilImageToDisplay, 255, 0, 0, MilBinLowImage,  M_NOT_EQUAL, 0);
   MbufClearCond(MilImageToDisplay, 0, 255, 0, MilBinHighImage, M_NOT_EQUAL, 0);

   MosPrintf(MIL_TEXT("The blobs that are segmented using a low threshold value are displayed in red.\n"));
   MosPrintf(MIL_TEXT("The cells are well segmented, however there is also the presence noise.\n\n"));

   MosPrintf(MIL_TEXT("The blobs that are segmented using a high threshold value are displayed\nin green.\n\n"));
   MosPrintf(MIL_TEXT("The cells are well identified, and there is no noise. However the cells are\n"));
   MosPrintf(MIL_TEXT("not well segmented.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Reconstruct the blobs from seed
   MblobReconstruct(MilBinLowImage, MilBinHighImage, MilBinImage, M_RECONSTRUCT_FROM_SEED, M_BINARY);
   MbufClearCond(MilImageToDisplay, 0, 0, 255, MilBinImage, M_NOT_EQUAL, 0);
   MosPrintf(MIL_TEXT("The blobs segmented using a low threshold that are touching the blobs segmented\n"));
   MosPrintf(MIL_TEXT("using a high threshold and displayed in blue.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Calibrate the binary image to measure the blob features in world units.
   McalUniform(MilBinImage, 0, 0, PIXEL_SIZE_X, PIXEL_SIZE_Y, 0, M_DEFAULT);

   // Allocate a blob context. 
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
  
   // Enable blob features.
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobControl(MilBlobContext, M_FERETS, M_ENABLE);
 
   // Allocate a blob result buffer.
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);
 
   // Calculate selected features for each blob.
   MblobCalculate(MilBlobContext, MilBinImage, M_NULL, MilBlobResult);

   MblobControl(MilBlobResult, M_INPUT_SELECT_UNITS, M_WORLD);

   // Display the original image and the blob results
   MbufCopy(MilImage, MilImageToDisplay);

   // Display the segmented blobs.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_BLOBS, M_DEFAULT, M_DEFAULT);
 
   // Exclude the blobs whose the area is outside the expected range of values.
   MblobSelect(MilBlobResult, M_EXCLUDE, M_AREA, M_OUT_RANGE, MINIMUM_AREA, MAXIMUM_AREA); 

   // Exclude the blobs whose the elongation is less than the minimum expected value.
   MblobSelect(MilBlobResult, M_EXCLUDE, M_FERET_ELONGATION, M_LESS, MINIMUM_FERET_ELONGATION, M_NULL); 

   // Get the total number of selected blobs.
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   // Display the selected blobs.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);
   
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_CENTER_OF_GRAVITY, M_INCLUDED_BLOBS, M_DEFAULT);

   // Set the text's background fill mode and font properties
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraFont(M_DEFAULT, MIL_FONT_NAME(M_FONT_DEFAULT_TTF));
   MgraControl(M_DEFAULT, M_FONT_SIZE, 12);

   MosPrintf(MIL_TEXT("The reconstructed blobs are analyzed to detect only isolated\n"));
   MosPrintf(MIL_TEXT("cells (in blue) using their area and elongation measures.\n\n"));

   if (NumberOfBlobs>0)
      {
      MosPrintf(MIL_TEXT("Number of detected cells: %d\n\n"), NumberOfBlobs);

      AreaArray = new MIL_DOUBLE[NumberOfBlobs];
      CogXArray = new MIL_DOUBLE[NumberOfBlobs];
      CogYArray = new MIL_DOUBLE[NumberOfBlobs];
      ElongationArray = new MIL_DOUBLE[NumberOfBlobs];

      // Retrieve the results.
      MblobControl(MilBlobResult, M_RESULT_OUTPUT_UNITS, M_PIXEL);

      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_X + M_BINARY + M_TYPE_MIL_DOUBLE, CogXArray);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_Y + M_BINARY + M_TYPE_MIL_DOUBLE, CogYArray);

      MblobControl(MilBlobResult, M_RESULT_OUTPUT_UNITS, M_WORLD);

      MblobGetResult(MilBlobResult, M_DEFAULT, M_AREA + M_TYPE_MIL_DOUBLE, AreaArray);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_FERET_ELONGATION     + M_TYPE_MIL_DOUBLE, ElongationArray); 

      // Print the results of each blob.
      MIL_TEXT_CHAR TextIndex[16];
      for(i=0; i < NumberOfBlobs; i++)
         {
         MosSprintf(TextIndex, 3, MIL_TEXT("%d"), i);
         MgraText(M_DEFAULT, MilGraphicList, CogXArray[i]+2, CogYArray[i]-14, TextIndex);
         MosPrintf(MIL_TEXT("Blob #%ld:\t[area = %.2f mm^2] [elongation = %.2f]\n"), i, AreaArray[i], ElongationArray[i]);
         }

      // Release allocated arrays
      delete []AreaArray;
      delete []CogXArray;
      delete []CogYArray;
      delete []ElongationArray;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to terminate.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MgraFree(MilGraphicList);
   MblobFree(MilBlobResult); 
   MblobFree(MilBlobContext); 
   MbufFree(MilBinLowImage);
   MbufFree(MilBinHighImage);
   MbufFree(MilBinImage);
   MbufFree(MilDestImage);
   MbufFree(MilImage);
   MbufFree(MilImageToDisplay);
   MdispFree(MilDisplay);

   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }
