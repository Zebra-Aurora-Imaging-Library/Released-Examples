﻿//********************************************************************************************/
//
// File name:  ObjectSeparation.cpp
//
// Synopsis:   This example shows three techniques to separate touching objects in a binary image:
//
//             1- using binary morphological operations.
//
//             2- using the watershed operation.
//
//             3- using a zone of infulence detection.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

// Example image path.
#define  EXAMPLE_IMAGE_PATH   M_IMAGE_PATH MIL_TEXT("Preprocessing/")
// Source image file name.
#define  IMAGE_FILE           EXAMPLE_IMAGE_PATH MIL_TEXT("TouchingObjectsBin.mim")

// Example functions declarations.
void MorphoylogyExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2);
void WatershedExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2);
void ZoneOfInfluenceExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2);

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("ObjectSeparation\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows three techniques to separate touching objects\n")
             MIL_TEXT("in a binary image:\n")
             MIL_TEXT("1- using binary morphological operations.\n")
             MIL_TEXT("2- using the watershed operation.\n")
             MIL_TEXT("3- using a zone of influence detection.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate defaults.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate two displays.
   MIL_UNIQUE_DISP_ID MilDisplay1 = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID MilDisplay2= MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   // Restore source image into a image buffer.
   MIL_UNIQUE_BUF_ID MilImage = MbufRestore(IMAGE_FILE, MilSystem, M_UNIQUE_ID);

   // Set the title of the first display.
   MdispControl(MilDisplay1, M_TITLE, MIL_TEXT("Original image"));
   // Display the original image.
   MdispSelect(MilDisplay1, MilImage);

   // Print header.
   PrintHeader();

   // Run the example using Morphology.
   MorphoylogyExample(MilSystem, MilImage, MilDisplay2);

   // Run the example using watershed transformation.
   WatershedExample(MilSystem, MilImage, MilDisplay2);

   // Run the example using a zone of infulence detection.
   ZoneOfInfluenceExample(MilSystem, MilImage, MilDisplay2);

   return 0;
   }


// Eroding iteration number
#define ERODE_ITERATION_NB   7
// Thickening iteration number
#define THICK_ITERATION_NB   12

void MorphoylogyExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2)
   {
   // Inquire image size.
   MIL_INT SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Allocate processing image buffer.
   MIL_UNIQUE_BUF_ID MilBinImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, M_UNIQUE_ID);

   MIL_DOUBLE TotalTime = 0, Time = 0;

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Apply an erosion operation.
   MimErode(MilImage, MilBinImage, ERODE_ITERATION_NB, M_BINARY);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the erosion result.
   MdispControl(MilDisplay2, M_WINDOW_INITIAL_POSITION_X, SizeX+20);
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Erosion result"));
   MdispSelect(MilDisplay2, MilBinImage);

   MosPrintf(MIL_TEXT("----------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("1- Separation using binary morphological operations.\n\n"));
   MosPrintf(MIL_TEXT("First, a large erosion operation is applied to ensure breaking\n"));
   MosPrintf(MIL_TEXT("the links between touching objects.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Apply a large thickening operation.
   MimThick(MilBinImage, MilBinImage, THICK_ITERATION_NB, M_BINARY);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the thickening result.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Thickening result"));

   MosPrintf(MIL_TEXT("Next, a large thickening operation is applied.\n"));
   MosPrintf(MIL_TEXT("Note that it is important that the resulting objects get\n"));
   MosPrintf(MIL_TEXT("larger than the original objects.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   // Apply a logical AND operation.
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   MimArith(MilImage, MilBinImage, MilBinImage, M_AND);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the final separated objects.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Separated Objects"));
   MdispSelect(MilDisplay2, MilBinImage);

   MosPrintf(MIL_TEXT("Finally, the thickening result is combined with the original image\n"));
   MosPrintf(MIL_TEXT("using a logical AND operation to split the touching objects.\n"));

   MosPrintf(MIL_TEXT("\nThe total processing time is %.2f ms.\n"), TotalTime*1000.0);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   }


// The minimum variation of gray-level of a catchment basin. 
#define MIN_VARIATION   10

void WatershedExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2)
   {
    // Inquire image size.
   MIL_INT SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Allocate processing image buffer.
   MIL_UNIQUE_BUF_ID MilGrayImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   MIL_DOUBLE TotalTime = 0, Time = 0;

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Calculate the distance transformation.
   MimDistance(MilImage, MilGrayImage, M_CHAMFER_3_4);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the distance transformation result.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Distance transformation result"));
   MdispControl(MilDisplay2, M_VIEW_MODE, M_AUTO_SCALE);
   MdispSelect(MilDisplay2, MilGrayImage);

   MosPrintf(MIL_TEXT("--------------------------------------------\n"));
   MosPrintf(MIL_TEXT("2- Separation using the watershed operation.\n\n"));
   MosPrintf(MIL_TEXT("First, the distance transformation of the image is calculated.\n"));
   MosPrintf(MIL_TEXT("Note that the result is remapped for display purposes.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   //Apply a watershed transformation.
   MimWatershed(MilGrayImage, M_NULL, MilGrayImage, MIN_VARIATION, M_WATERSHED+M_MAXIMA_FILL);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the Watershed transformation result.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Lines of separation"));

   MosPrintf(MIL_TEXT("Next, a watershed transformation is applied to the distance\n"));
   MosPrintf(MIL_TEXT("transformation result to obtain lines of separation.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Apply a logical AND operation.
   MimArith(MilImage, MilGrayImage, MilGrayImage, M_AND);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the final separated objects.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Separated Objects"));

   MosPrintf(MIL_TEXT("Finally, the lines of separation are combined with the original\n"));
   MosPrintf(MIL_TEXT("image using a logical AND operation to split the touching objects.\n"));

   MosPrintf(MIL_TEXT("\nThe total processing time is %.2f ms.\n"), TotalTime*1000.0);

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   }


void ZoneOfInfluenceExample(MIL_ID MilSystem, MIL_ID MilImage, MIL_ID MilDisplay2)
{
    // Inquire image size.
    MIL_INT SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
    MIL_INT SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Allocate processing image buffers.
   MIL_UNIQUE_BUF_ID MimBinImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID MimZoneOfInfluenceImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID MimEdgeDetectImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);

   MIL_DOUBLE TotalTime = 0, Time = 0;

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Apply an erosion operation.
   MimErode(MilImage, MimBinImage, ERODE_ITERATION_NB, M_BINARY);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Erosion result"));
   MdispSelect(MilDisplay2, MimBinImage);

   MosPrintf(MIL_TEXT("--------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("3- Separation using a zone of influence detection.\n\n"));
   MosPrintf(MIL_TEXT("A large erosion operation is applied to ensure breaking\n"));
   MosPrintf(MIL_TEXT("the links between touching objects.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Perform a zone of influence detection.
   MimZoneOfInfluence(MimBinImage, MimZoneOfInfluenceImage, M_DEFAULT);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Zone of Influence result"));
   MdispSelect(MilDisplay2, MimZoneOfInfluenceImage);

   MosPrintf(MIL_TEXT("A zone of influence detection is performed to\n"));
   MosPrintf(MIL_TEXT("separate the image into zones.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Perform a convolution operation using a Sobel filter.
   MimConvolve(MimZoneOfInfluenceImage, MimEdgeDetectImage, M_EDGE_DETECT_SOBEL_FAST);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Edge Detect result"));
   MdispSelect(MilDisplay2, MimEdgeDetectImage);

   MosPrintf(MIL_TEXT("A convolution operation is performed using a Sobel filter\n"));
   MosPrintf(MIL_TEXT("to obtain the boundaries of zones.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Binarize the edge detect result.
   MimBinarize(MimEdgeDetectImage, MimBinImage, M_FIXED + M_LESS, 1.0, M_NULL);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Lines of separation"));
   MdispSelect(MilDisplay2, MimBinImage);

   MosPrintf(MIL_TEXT("Binarization is applied to obtain lines of separation.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
   // Apply a logical AND operation.
   MimArith(MilImage, MimBinImage, MimBinImage, M_AND);
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
   TotalTime += Time;

   // Display the final separated objects.
   MdispControl(MilDisplay2, M_TITLE, MIL_TEXT("Separated Objects"));

   MosPrintf(MIL_TEXT("Finally, the lines of separation are combined with the original\n"));
   MosPrintf(MIL_TEXT("image using a logical AND operation to split the touching objects.\n"));

   MosPrintf(MIL_TEXT("\nThe total processing time is %.2f ms.\n"), TotalTime*1000.0);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();
}