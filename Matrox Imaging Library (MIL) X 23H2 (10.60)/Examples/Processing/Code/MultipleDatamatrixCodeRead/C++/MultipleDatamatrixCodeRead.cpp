﻿//********************************************************************************************/
//
// File name:  MultipleDatamatrixCodeRead.cpp
//
// Synopsis:   This example shows three techniques to read several datamatrix codes in an image:
//
//             1- Performing a single reading in the whole image.
//
//             2- Performing multiple sequential readings in regions of interest (ROIs).
//
//             3- Performing multiple parallel readings in regions of interest (ROIs).
//
// In order to get meaningful timing benchmarks, this application must be compiled in 'Release' mode 
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

// Example image path.  
#define  EXAMPLE_IMAGE_PATH   M_IMAGE_PATH MIL_TEXT("MultipleDatamatrixCodeRead/")
// Source image file name. 
#define  IMAGE_FILE           EXAMPLE_IMAGE_PATH MIL_TEXT("MultipleDatamatrix.mim")
// Expected maximum size of any string.
#define  MAXIMUM_STRING_SIZE  100
// Offset of the text in the MIL display.
#define  TEXT_OFFSET_X        -150 
#define  TEXT_OFFSET_Y_1      -70 
#define  TEXT_OFFSET_Y_2      -40 
// Timing loop iterations.
#define NB_LOOP               4

// Example functions declarations.
void SingleReadingExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay);
void SequentialReadingsExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay);
void ParallelReadingsExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MultipleDatamatrixCodeRead\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows three techniques to locate and read several\n")
             MIL_TEXT("datamatrix codes in an image:\n")
             MIL_TEXT("1- Performing a single reading in the whole image.\n")
             MIL_TEXT("2- Performing multiple sequential readings in regions of interest (ROIs).\n")
             MIL_TEXT("3- Performing multiple parallel readings in regions of interest (ROIs).\n\n")
  
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, thread, display, buffer,\n")
             MIL_TEXT("image processing, blob, code, graphics.\n\n")
  
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication,     // Application identifier.
          MilSystem,          // System identifier.
          MilDisplay,         // Display identifier.
          MilCodeContext,     // Code reader context identifier.
          MilCodeModel,       // Code reader model identifier.
          MilDispGraList,     // Graphic list identifier.
          MilImage;           // Image buffer identifier.

   // Allocate MIL objects. 
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Allocate a code context.
   McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilCodeContext);

   // Add a datamatrix code model.
   McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, &MilCodeModel);

   // Deactivate the timeout
   McodeControl(MilCodeContext, M_TIMEOUT, M_DISABLE);

   // Restore source image into image buffer.
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);

   // Display the image buffer. Zoom it down first.
   MdispZoom(MilDisplay, 0.5, 0.5);
   MdispSelect(MilDisplay, MilImage);

   // Allocate a graphic list.
   MgraAllocList(MilSystem, M_DEFAULT, &MilDispGraList);
   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilDispGraList);

   // Print header. 
   PrintHeader();

   // Set the number of datamatrix to read to M_ALL.
   McodeControl(MilCodeModel, M_NUMBER, M_ALL);
   // Run the example that reads all the datamatrix at once.
   SingleReadingExample(MilSystem, MilCodeContext, MilImage, MilDisplay);

   // Set the number of datamatrix to read to 1.
   McodeControl(MilCodeModel, M_NUMBER, 1);
   // Run the example that reads all the datamatrix sequentially.
   SequentialReadingsExample(MilSystem, MilCodeContext, MilImage, MilDisplay);

   // Run the example that reads all the datamatrix in parallel.
   ParallelReadingsExample(MilSystem, MilCodeContext, MilImage, MilDisplay);

   // Free MIL objects.
   McodeFree(MilCodeContext);
   MgraFree(MilDispGraList);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

void SingleReadingExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay)
   {
   MIL_ID         MilDispGraList,   // Graphic list identifier. Will be used to display results.
                  MilCodeResult;    // Code reader result buffer identifier.
   MIL_INT        NumberOfCodes,    // Number of codes found and read.
                  CodeIndex,        // Code index used to loop through the results.
                  ReadIndex;        // Read loop counter index.
   MIL_DOUBLE     Time;             // Processing time.

   MosPrintf(MIL_TEXT("--------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("1- Performing a single reading in the whole image.\n\n"));

   MosPrintf(MIL_TEXT("A Code Reader context is set up to locate and read an unknown\n"));
   MosPrintf(MIL_TEXT("number of datamatrix codes in an image.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a code result buffer.
   McodeAllocResult(MilSystem, M_DEFAULT, &MilCodeResult);

   // Read the datamatrix.
   McodeRead(MilCodeContext, MilImage, MilCodeResult);

   // Get the number of datamatrix that were read.
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_NUMBER+M_TYPE_MIL_INT, &NumberOfCodes);

   // Disable the display update when the associated graphic list is modified.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Get the graphic list identifier and clear the graphic list.
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &MilDispGraList);
   MgraClear(M_DEFAULT, MilDispGraList);

   // Draw results in the graphic list.
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   McodeDraw(M_DEFAULT, MilCodeResult, MilDispGraList, M_DRAW_BOX, M_ALL, M_GENERAL, M_DEFAULT);

   // Print the result's header.
   MosPrintf(MIL_TEXT("\n\tString\t\t\tPosition\n\t------------------------------------------\n"));
   
   // Loop through the codes to get the results.
   for (CodeIndex=0; CodeIndex<NumberOfCodes; CodeIndex++)
      {
      MIL_DOUBLE PositionX,                                       // X coordinate of the code.
                 PositionY;                                       // Y coordinate of the code.
      MIL_STRING DecodedString;                                   // Decoded string.
      MIL_STRING CharBuffer(MAXIMUM_STRING_SIZE, MIL_TEXT('\0')); // String to display.

      // Get the decoded strings and their position.
      McodeGetResult(MilCodeResult, CodeIndex, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(MilCodeResult, CodeIndex, M_GENERAL, M_POSITION_Y, &PositionY);
      McodeGetResult(MilCodeResult, CodeIndex, M_GENERAL, M_STRING, DecodedString);

      // Display the string using the graphic list.
      MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("Code%d"), CodeIndex+1);
      MgraText(M_DEFAULT, MilDispGraList, PositionX+TEXT_OFFSET_X, PositionY+TEXT_OFFSET_Y_1, CharBuffer);
      MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("%s"), DecodedString.c_str());
      MgraText(M_DEFAULT, MilDispGraList, PositionX+TEXT_OFFSET_X, PositionY+TEXT_OFFSET_Y_2, CharBuffer);

      // Print the results.
      MosPrintf(MIL_TEXT("Code%d:\t%s\t(%.2f, %.2f)\n"), CodeIndex+1, DecodedString.c_str(), PositionX, PositionY);
      }

   // Enable the display updates.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   MosPrintf(MIL_TEXT("\n%d datamatrix codes were read.\n"), NumberOfCodes);

   //*******************************************************************************
   // Now, time the code reading. Do it in a loop to get the average processing time.
   //*******************************************************************************
   MosPrintf(MIL_TEXT("\nTiming benchmark in progress; please wait ...\n"));
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (ReadIndex=0; ReadIndex<NB_LOOP; ReadIndex++)
      McodeRead(MilCodeContext, MilImage, MilCodeResult);
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);

   // Print the average processing time.
   MosPrintf(MIL_TEXT("\nThe %d codes were read in %.2f msec.\n\n"), NumberOfCodes, Time*1000/NB_LOOP);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the code result buffer.
   McodeFree(MilCodeResult);
   }


// Expected size of each datamatrix cell.
#define EXPECTED_CELL_SIZE             4.9
#define EXPECTED_CELL_NUMBER_X         16
#define EXPECTED_CELL_NUMBER_Y         16
// Maximum expected number of codes to read.
#define EXPECTED_MAX_NUMBER_OF_CODES   50

// Processing function parameters structure.
typedef struct 
   {
   MIL_ID      MilImage;               // Image buffer identifier.
   MIL_ID      MilResizedImage;        // Image buffer identifier.
   MIL_ID      MilResizedBinImage;     // Image buffer identifier.
   MIL_ID      MilStructElement;       // Structuring element buffer identifier.
   MIL_ID      MilBlobContext;         // Blob context identifier.
   MIL_ID      MilBlobResult;          // Blob result buffer identifier.
   MIL_ID      MilCodeContext;         // Code context identifier.
   MIL_ID      MilCodeResult;          // Code result identifier.
   MIL_ID      MilDispGraList;         // Graphic list identifier.
   MIL_ID      MilRoiGraList;          // Graphic list identifier.
   MIL_DOUBLE  ResizeFactor;           // Image resize factor.
   MIL_INT     NumberOfCodes;          // Number of codes found.
   bool        IsTimerActive;          // Indicates if a timing benchmark is in progress.
   } SEQUENTIAL_PROC_PARAM;

// Sequential processing functions declaration.
void SequentialInit(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilDisplay, MIL_ID MilImage, SEQUENTIAL_PROC_PARAM &ProcParamPtr);
void SequentialFree(SEQUENTIAL_PROC_PARAM &ProcParamPtr);
void SequentialProcessing(SEQUENTIAL_PROC_PARAM &ProcParamPtr);

void SequentialReadingsExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay)
   {
   MIL_INT     ReadIndex;                    // Read loop counter index.
   MIL_DOUBLE  Time;                         // Processing time.
   SEQUENTIAL_PROC_PARAM  ProcessingParam;   // Processing parameters.

   // Initialize the processing structure.
   SequentialInit(MilSystem, MilCodeContext, MilDisplay, MilImage, ProcessingParam);

   MosPrintf(MIL_TEXT("---------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("2- Performing multiple sequential readings in ROIs.\n\n"));

   MosPrintf(MIL_TEXT("A Code Reader context is set up to read a single datamatrix code.\n"));
   MosPrintf(MIL_TEXT("A custom preprocessing algorithm is used to locate potential\n"));
   MosPrintf(MIL_TEXT("datamatrix codes and to define an ROI around each one.\n"));
   MosPrintf(MIL_TEXT("The reading is performed for each ROI sequentially.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Print the result's header.
   MosPrintf(MIL_TEXT("\n\tString\t\t\tPosition\n\t------------------------------------------\n"));

   // Disable the display update when the associated graphic list is modified.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Perform the processing.
   SequentialProcessing(ProcessingParam);

   // Enable the display update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   MosPrintf(MIL_TEXT("\nA total of %d datamatrix codes were read.\n"), ProcessingParam.NumberOfCodes);

   //*******************************************************************************
   // Now, time the code reading. Do it in a loop to get the average processing time.
   //*******************************************************************************
   ProcessingParam.IsTimerActive = true;
   MosPrintf(MIL_TEXT("\nTiming benchmark in progress; please wait ...\n"));
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (ReadIndex=0; ReadIndex<NB_LOOP; ReadIndex++)
      {
      // Reset the number of codes found.
      ProcessingParam.NumberOfCodes = 0;
      // Perform the processing.
      SequentialProcessing(ProcessingParam);
      }
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);

   // Print the average processing time.
   MosPrintf(MIL_TEXT("\nThe %d codes were read with sequential readings in %.2f msec.\n\n"), ProcessingParam.NumberOfCodes, Time*1000/NB_LOOP);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the processing resources.
   SequentialFree(ProcessingParam);
   }

void SequentialInit(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilDisplay, MIL_ID MilImage, SEQUENTIAL_PROC_PARAM &ProcParamPtr)
   {
   MIL_INT  BufSizeX,         // Image buffer size X.
            BufSizeY,         // Image buffer size Y.
            BufType,          // Image buffer type.
            NewBufSizeX,      // Image buffer new size X.
            NewBufSizeY;      // Image buffer new size Y.

   // Allocate a code result buffer.
   McodeAllocResult(MilSystem, M_DEFAULT, &ProcParamPtr.MilCodeResult);

   // Allocate a context list.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &ProcParamPtr.MilBlobContext);

   // Add the bounding box to the context.
   MblobControl(ProcParamPtr.MilBlobContext, M_BOX, M_ENABLE);

   // Allocate a blob result buffer.
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &ProcParamPtr.MilBlobResult);

   MblobControl(ProcParamPtr.MilBlobContext, M_IDENTIFIER_TYPE, M_BINARY);

   // Get the size and type of the image buffer.
   MbufInquire(MilImage, M_SIZE_X, &BufSizeX);
   MbufInquire(MilImage, M_SIZE_Y, &BufSizeY);
   MbufInquire(MilImage, M_TYPE, &BufType);

   // The image will be resized so that a datamatrix cell is represented by a single pixel.
   ProcParamPtr.ResizeFactor = 1/EXPECTED_CELL_SIZE;
   NewBufSizeX = (MIL_INT) (BufSizeX * ProcParamPtr.ResizeFactor);
   NewBufSizeY = (MIL_INT) (BufSizeY * ProcParamPtr.ResizeFactor);

   // Allocate a smaller processing buffer.
   MbufAlloc2d(MilSystem, NewBufSizeX, NewBufSizeY, BufType, M_IMAGE+M_PROC, &ProcParamPtr.MilResizedImage);

   // Allocate a binary image that will be used for blob analysis.
   MbufAlloc2d(MilSystem, NewBufSizeX, NewBufSizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &ProcParamPtr.MilResizedBinImage);

   // Allocate a 3x3 structuring element and clear it with 1.
   MbufAlloc2d(MilSystem, 3, 3, 32+M_UNSIGNED, M_STRUCT_ELEMENT, &ProcParamPtr.MilStructElement);
   MbufClear(ProcParamPtr.MilStructElement, 1);

   // Get the graphic list identifier and clear the graphic list used to display the results.
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &ProcParamPtr.MilDispGraList);
   MgraClear(M_DEFAULT, ProcParamPtr.MilDispGraList);

   // Allocate a second graphic list and clear it. It will be used to define a ROI for the code reading.
   MgraAllocList(MilSystem, M_DEFAULT, &ProcParamPtr.MilRoiGraList);

   // Reset the number of codes read.
   ProcParamPtr.NumberOfCodes = 0;

   // Specify the timer is not active at first.
   ProcParamPtr.IsTimerActive = false;

   // Finish filling the processing parameter structure .
   ProcParamPtr.MilImage = MilImage;
   ProcParamPtr.MilCodeContext = MilCodeContext;
   }

void SequentialFree(SEQUENTIAL_PROC_PARAM &ProcParamPtr)
   {
   // Free MIL objects.
   McodeFree(ProcParamPtr.MilCodeResult);
   MgraFree(ProcParamPtr.MilRoiGraList);
   MbufFree(ProcParamPtr.MilResizedBinImage);
   MbufFree(ProcParamPtr.MilStructElement);
   MbufFree(ProcParamPtr.MilResizedImage);
   MblobFree(ProcParamPtr.MilBlobResult);
   MblobFree(ProcParamPtr.MilBlobContext);
   }


void SequentialProcessing(SEQUENTIAL_PROC_PARAM &ProcParamPtr)
   {
   MIL_INT     NumberOfBlobs;    // Number of blobs found.
   MIL_DOUBLE  ExpectedAreaMin,  // Minimum expected size of the area of a blob.
               ExpectedAreaMax;  // Maximum expected size of the area of a blob.

   // Resize the image in order to reduce the processing time. A datamatrix cell will be represented by a single pixel..
   MimResize(ProcParamPtr.MilImage, ProcParamPtr.MilResizedImage, ProcParamPtr.ResizeFactor, ProcParamPtr.ResizeFactor, M_NEAREST_NEIGHBOR+M_OVERSCAN_ENABLE);

   // Perform a bottom-hat filtering to make the background uniform.
   MimMorphic(ProcParamPtr.MilResizedImage, ProcParamPtr.MilResizedImage, ProcParamPtr.MilStructElement, M_BOTTOM_HAT, 5, M_GRAYSCALE);

   // Binarize the image.
   MimBinarize(ProcParamPtr.MilResizedImage, ProcParamPtr.MilResizedBinImage, M_BIMODAL+M_GREATER_OR_EQUAL, M_NULL, M_NULL);

   // Perform a dilate filtering to merge broken datamatrix.
   MimDilate(ProcParamPtr.MilResizedBinImage, ProcParamPtr.MilResizedBinImage, 1, M_BINARY);

   // Compute the blobs.
   MblobCalculate(ProcParamPtr.MilBlobContext, ProcParamPtr.MilResizedBinImage, M_NULL, ProcParamPtr.MilBlobResult);

   // Exclude the blobs that are smaller than a minimum expected area.
   ExpectedAreaMin = (EXPECTED_CELL_NUMBER_X * EXPECTED_CELL_NUMBER_Y * 0.8);
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_AREA, M_LESS, ExpectedAreaMin, M_NULL);
   // Exclude the blobs that are larger than a maximum expected area.
   ExpectedAreaMax = EXPECTED_CELL_NUMBER_X * EXPECTED_CELL_NUMBER_Y*1.3;
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_AREA, M_GREATER, ExpectedAreaMax, M_NULL);
   // Exclude the blobs that have a bounding box aspect ratio too far from 1.
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_BOX_ASPECT_RATIO, M_OUT_RANGE, 0.85, 1.15);

   // Get the number of included blobs.
   MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   if (NumberOfBlobs > 0)
      {
      MIL_INT              BlobIndex;  // Blob Index used to loop through blobs.
      std::vector<MIL_INT> BoxXMin,    // X coordinate of the blobs upper left corner.
                           BoxXMax,    // X coordinate of the blobs lower right corner.
                           BoxYMin,    // Y coordinate of the blobs upper left corner.
                           BoxYMax;    // Y coordinate of the blobs lower right corner.

      // Get the bounding box of each blob.
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_X_MIN, BoxXMin);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_X_MAX, BoxXMax);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_Y_MIN, BoxYMin);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_Y_MAX, BoxYMax);

      // Loop through the blobs and read the codes in ROI.
      for(BlobIndex=0; BlobIndex<NumberOfBlobs; BlobIndex++)
         {
         MIL_INT     CodeReadStatus;   // Read status.

         // Define a ROI around the blob. Leave a margin around the blob.
         MIL_INT  Margin = 3;
         MIL_INT  BoxStartX = (MIL_INT)((BoxXMin[BlobIndex] * (1/ProcParamPtr.ResizeFactor))-(Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxStartY = (MIL_INT)((BoxYMin[BlobIndex] * (1/ProcParamPtr.ResizeFactor))-(Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxSizeX = (MIL_INT)(((BoxXMax[BlobIndex] - BoxXMin[BlobIndex])*(1/ProcParamPtr.ResizeFactor))+(2*Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxSizeY = (MIL_INT)(((BoxYMax[BlobIndex] - BoxYMin[BlobIndex])*(1/ProcParamPtr.ResizeFactor))+(2*Margin*EXPECTED_CELL_SIZE));

         // Clear the graphic list.
         MgraClear(M_DEFAULT, ProcParamPtr.MilRoiGraList);
         // Draw the ROI in the graphic list.
         MgraRectAngle(M_DEFAULT, ProcParamPtr.MilRoiGraList, BoxStartX, BoxStartY, BoxSizeX, BoxSizeY, 0, M_CORNER_AND_DIMENSION+M_FILLED);

         // Associate the modified graphic list with the image to process.
         MbufSetRegion(ProcParamPtr.MilImage, ProcParamPtr.MilRoiGraList, M_DEFAULT, M_NO_RASTERIZE, M_DEFAULT);

         // Read the code.
         McodeRead(ProcParamPtr.MilCodeContext, ProcParamPtr.MilImage, ProcParamPtr.MilCodeResult);

         McodeGetResult(ProcParamPtr.MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &CodeReadStatus);
         if (CodeReadStatus == M_STATUS_READ_OK)
            {
            // If a timing benchmark is not in progress, get the results and display/print them.
            if (!ProcParamPtr.IsTimerActive)
               {
               MIL_DOUBLE  PositionX,                                       // X coordinate of the code.
                           PositionY;                                       // Y coordinate of the code.
               MIL_STRING  DecodedString;                                   // Decoded string.
               MIL_STRING  CharBuffer(MAXIMUM_STRING_SIZE, MIL_TEXT('\0')); // String to display.

               // Get the decoded string and position of the code.
               McodeGetResult(ProcParamPtr.MilCodeResult, 0, M_GENERAL, M_STRING, DecodedString);
               McodeGetResult(ProcParamPtr.MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionX);
               McodeGetResult(ProcParamPtr.MilCodeResult, 0, M_GENERAL, M_POSITION_Y, &PositionY);

               // Draw results in the graphic list.
               MgraColor(M_DEFAULT, M_COLOR_CYAN);
               McodeDraw(M_DEFAULT, ProcParamPtr.MilCodeResult, ProcParamPtr.MilDispGraList, M_DRAW_BOX, M_ALL, M_GENERAL, M_DEFAULT);
               // Display the string using the graphic list.
               MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("Code%d"), ProcParamPtr.NumberOfCodes+1);
               MgraText(M_DEFAULT, ProcParamPtr.MilDispGraList, PositionX+TEXT_OFFSET_X, PositionY+TEXT_OFFSET_Y_1, CharBuffer);
               MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("%s"), DecodedString.c_str());
               MgraText(M_DEFAULT, ProcParamPtr.MilDispGraList, PositionX+TEXT_OFFSET_X, PositionY+TEXT_OFFSET_Y_2, CharBuffer);

               // Print the results.
               MosPrintf(MIL_TEXT("Code%d:\t%s\t(%.2f, %.2f)\n"), ProcParamPtr.NumberOfCodes+1, DecodedString.c_str(), PositionX, PositionY);
               }

            // Increment the number of codes that were read.
            ProcParamPtr.NumberOfCodes++;
            }
         }

      // Delete the ROI information from the image buffer.
      MbufSetRegion(ProcParamPtr.MilImage, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);
      }
   }


#define MAXIMUM_NUMBER_OF_CODES 50

// ROI structure.
typedef struct SBoxParam
   {
   MIL_INT  MinX;    // Upper left x coordinate of the blob's bounding box.
   MIL_INT  MinY;    // Upper left y coordinate of the blob's bounding box.
   MIL_INT  MaxX;    // Lower right x coordinate of the blob's bounding box.
   MIL_INT  MaxY;    // Lower right y coordinate of the blob's bounding box.
   }BOX_PARAM;

// Thread parameters structure.
typedef struct SThreadParam
   {
   MIL_ID             MilThread;                            // Thread identifier.
   MIL_ID             MilImage;                             // Image buffer identifier.
   MIL_ID             MilCodeContext;                       // Code context identifier.
   MIL_ID             MilCodeResult;                        // Code result identifier.
   MIL_ID             MilRoiGraList;                        // Graphic list identifier.
   MIL_ID             ReadyEvent;                           // Event identifier used to indicate the thread is ready to process.
   MIL_ID             DoneEvent;                            // Event identifier used to indicate the thread has finished processing.
   MIL_INT            NumberOfCodes;                        // Number of codes found.
   MIL_INT            ReadStatus;                           // Status of the read operation.
   bool               DoExit;                               // Indicates to exit the processing thread.
   MIL_DOUBLE         ResizeFactor;                         // Image resize factor.
   BOX_PARAM          BlobBox;                              // Bounding box of the blob.
   MIL_STRING         CodesRead[MAXIMUM_NUMBER_OF_CODES];   // Decoded string.
   MIL_DOUBLE         PosX[MAXIMUM_NUMBER_OF_CODES];        // Position X of the code.
   MIL_DOUBLE         PosY[MAXIMUM_NUMBER_OF_CODES];        // Position Y of the code.
   } THREAD_PARAM;

// Processing function parameters structure.
typedef struct 
   {
   MIL_ID                    MilImage;                // Image buffer identifier.
   MIL_ID                    MilResizedImage;         // Image buffer identifier.
   MIL_ID                    MilResizedBinImage;      // Image buffer identifier.
   MIL_ID                    MilStructElement;        // Structuring element buffer identifier.
   MIL_ID                    MilBlobContext;          // Blob context identifier.
   MIL_ID                    MilBlobResult;           // Blob result buffer identifier.
   MIL_ID                    MilDispGraList;          // Graphic list identifier.
   std::vector<MIL_ID>       DoneEvents;              // Event identifiers.
   MIL_INT                   NumProcCores;            // Number of cores available.
   MIL_INT                   InitialMpUse;            // Initial state of the Use Mp functionality.
   MIL_DOUBLE                ResizeFactor;            // Image resize factor.
   bool                      IsTimerActive;           // Indicates if a timing benchmark is in progress.
   std::vector<THREAD_PARAM> ThreadParam;             // Parameter structure sent to each thread.
   } PARALLEL_PROC_PARAM;

// Parallel processing functions declaration.
void ParallelInit(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilDisplay, MIL_ID MilImage, PARALLEL_PROC_PARAM &ProcParamPtr);
void ParallelFree(PARALLEL_PROC_PARAM &ProcParamPtr);
void ParallelProcessing(PARALLEL_PROC_PARAM &ProcParamPtr);
void ParallelGetResults(THREAD_PARAM &ThreadParamPtr, MIL_ID MilDispGraList);
MIL_UINT32 MFTYPE ParallelProcessingThread(void *ThreadParam);

   
void ParallelReadingsExample(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilImage, MIL_ID MilDisplay)
   {
   MIL_INT              ReadIndex,           // Read loop counter index.
                        ThreadIndex,         // Thread loop counter index.
                        TotalNumberOfCodes;  // The total number of codes that were read.
   MIL_DOUBLE           Time;                // Processing time.
   PARALLEL_PROC_PARAM  ProcessingParam;     // Processing parameters.

   // Initialize the processing structure.
   ParallelInit(MilSystem, MilCodeContext, MilDisplay, MilImage, ProcessingParam);

   MosPrintf(MIL_TEXT("-------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("3- Performing multiple parallel readings in ROIs.\n\n"));

   MosPrintf(MIL_TEXT("A Code Reader context is set up to read a single datamatrix code.\n"));
   MosPrintf(MIL_TEXT("A custom preprocessing algorithm is used to locate potential\n"));
   MosPrintf(MIL_TEXT("datamatrix codes and to define an ROI around each one.\n"));
   MosPrintf(MIL_TEXT("Readings are then performed for several ROIs on parallel threads.\n"));
   MosPrintf(MIL_TEXT("The number of threads used is equal to the number of cores\n"));
   MosPrintf(MIL_TEXT("available on the system.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Print the result's header.
   MosPrintf(MIL_TEXT("\n\t\tString\t\t\tPosition\n\t\t------------------------------------------\n"));

   // Disable the display update when the associated graphic list is modified.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);

   // Perform the processing.
   ParallelProcessing(ProcessingParam);

   // Enable the display update.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);

   TotalNumberOfCodes = 0;
   MosPrintf(MIL_TEXT("\n"));
   for (ThreadIndex=0; ThreadIndex<ProcessingParam.NumProcCores; ThreadIndex++)
      {
      MosPrintf(MIL_TEXT("A total of %d datamatrix codes were read in thread %d.\n"), ProcessingParam.ThreadParam[ThreadIndex].NumberOfCodes, ThreadIndex+1);
      TotalNumberOfCodes+=ProcessingParam.ThreadParam[ThreadIndex].NumberOfCodes;
      }

   //*******************************************************************************
   // Now, time the code reading. Do it in a loop to get the average processing time.
   //*******************************************************************************
   ProcessingParam.IsTimerActive = true;
   MosPrintf(MIL_TEXT("\nTiming benchmark in progress; please wait ...\n"));
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
   for (ReadIndex=0; ReadIndex<NB_LOOP; ReadIndex++)
      {
      // Perform the processing.
      ParallelProcessing(ProcessingParam);
      }
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
 
   // Print the average processing time.
   MosPrintf(MIL_TEXT("\nThe %d codes were read with parallel readings in %.2f msec.\n\n"), TotalNumberOfCodes, Time*1000/NB_LOOP);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   // Free the processing resources.
   ParallelFree(ProcessingParam);
   }

void ParallelInit(MIL_ID MilSystem, MIL_ID MilCodeContext, MIL_ID MilDisplay, MIL_ID MilImage, PARALLEL_PROC_PARAM &ProcParamPtr)
   {
   MIL_INT                BufSizeX,            // Image buffer size X.
                          BufSizeY,            // Image buffer size Y.
                          BufType,             // Image buffer type.
                          NewBufSizeX,         // Image buffer new size X.
                          NewBufSizeY,         // Image buffer new size Y.
                          ContextByteSize,     // Memory size required to store the code context.
                          ThreadIndex;         // Thread loop counter index.
   std::vector<MIL_UINT8> MemVector;           // Vector of the memory used to store the code context.

   // Allocate a blob context.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &ProcParamPtr.MilBlobContext);

   // Add the bounding box to the context.
   MblobControl(ProcParamPtr.MilBlobContext, M_BOX, M_ENABLE);

   // Allocate a blob result buffer.
   MblobAllocResult(MilSystem, &ProcParamPtr.MilBlobResult);
   MblobControl(ProcParamPtr.MilBlobContext, M_IDENTIFIER_TYPE, M_BINARY);

   // Copy the image identifier in the processing structure.
   ProcParamPtr.MilImage = MilImage;

   // Get the size and type of the image buffer.
   MbufInquire(MilImage, M_SIZE_X, &BufSizeX);
   MbufInquire(MilImage, M_SIZE_Y, &BufSizeY);
   MbufInquire(MilImage, M_TYPE, &BufType);

   // The image will be resized so that a datamatrix cell is represented by a single pixel.
   // Compute this new size.
   ProcParamPtr.ResizeFactor = 1/EXPECTED_CELL_SIZE;
   NewBufSizeX = (MIL_INT) (BufSizeX * ProcParamPtr.ResizeFactor);
   NewBufSizeY = (MIL_INT) (BufSizeY * ProcParamPtr.ResizeFactor);

   // Allocate a smaller processing buffer.
   MbufAlloc2d(MilSystem, NewBufSizeX, NewBufSizeY, BufType, M_IMAGE+M_PROC, &ProcParamPtr.MilResizedImage);

   // Allocate a binary image that will be used for blob analysis.
   MbufAlloc2d(MilSystem, NewBufSizeX, NewBufSizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &ProcParamPtr.MilResizedBinImage);

   // Allocate a 3x3 structuring element and clear it with 1.
   MbufAlloc2d(MilSystem, 3, 3, 32+M_UNSIGNED, M_STRUCT_ELEMENT, &ProcParamPtr.MilStructElement);
   MbufClear(ProcParamPtr.MilStructElement, 1);

   // Get the graphic list identifier and clear the graphic list used to display the results.
   MdispInquire(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, &ProcParamPtr.MilDispGraList);
   MgraClear(M_DEFAULT, ProcParamPtr.MilDispGraList);

   // Specify the timer is not active at first.
   ProcParamPtr.IsTimerActive = false;

   // ************************************************
   // Initialization of the Thread parameter structure.
   // ************************************************

   // Inquire the initial state of the Use MP functionality.
   MappInquireMp(M_DEFAULT, M_MP_USE, M_DEFAULT, M_DEFAULT, &ProcParamPtr.InitialMpUse);

   // Inquire the number of cores available for processing.
   MappInquireMp(M_DEFAULT, M_CORE_MAX, M_DEFAULT, M_NULL, &ProcParamPtr.NumProcCores);

   if ((ProcParamPtr.NumProcCores > 1) && (ProcParamPtr.InitialMpUse == M_ENABLE))
      {
      // Disable MP processing to avoid contention between users threads and MP.
      MappControlMp(M_DEFAULT, M_MP_USE, M_DEFAULT, M_DISABLE, M_NULL);
      }

   // Resize the vector of thread parameter structures; one per core.
   ProcParamPtr.ThreadParam.resize(ProcParamPtr.NumProcCores);

    // Resize the vector of event identifiers; one per core.
   ProcParamPtr.DoneEvents.resize(ProcParamPtr.NumProcCores);

   // A copy of the code context must be made for each thread.
   // Stream the context to memory.
   McodeStream(MemVector, M_NULL, M_SAVE, M_MEMORY, M_DEFAULT, M_DEFAULT, &MilCodeContext, &ContextByteSize);

   // Initialize the thread parameter structures and start the threads.
   for (ThreadIndex=0; ThreadIndex<ProcParamPtr.NumProcCores; ThreadIndex++)
      {
      // Stream the code context from memory.
      McodeStream(MemVector, MilSystem, M_RESTORE, M_MEMORY, M_DEFAULT, M_DEFAULT, &ProcParamPtr.ThreadParam[ThreadIndex].MilCodeContext, &ContextByteSize);

      // Set the initial exit state to false.
      ProcParamPtr.ThreadParam[ThreadIndex].DoExit = false;

      // Set the resize factor in the thread structure.
      ProcParamPtr.ThreadParam[ThreadIndex].ResizeFactor = ProcParamPtr.ResizeFactor;

      // Allocate a code result buffer.
      McodeAllocResult(MilSystem, M_DEFAULT, &ProcParamPtr.ThreadParam[ThreadIndex].MilCodeResult);
      
      // Allocate one image per thread.
      MbufAlloc2d(MilSystem, BufSizeX, BufSizeY, BufType, M_IMAGE+M_PROC, &ProcParamPtr.ThreadParam[ThreadIndex].MilImage);

      // Copy the original image in each thread.
      MbufCopy(MilImage, ProcParamPtr.ThreadParam[ThreadIndex].MilImage);

      // Allocate a graphic list and clear it. It will be used to define a ROI for the code reading.
      MgraAllocList(MilSystem, M_DEFAULT, &ProcParamPtr.ThreadParam[ThreadIndex].MilRoiGraList);

      // Allocate the synchronization events.
      MthrAlloc(MilSystem, M_EVENT, M_NOT_SIGNALED+M_AUTO_RESET, M_NULL, M_NULL, &ProcParamPtr.ThreadParam[ThreadIndex].ReadyEvent);
      ProcParamPtr.DoneEvents[ThreadIndex] = MthrAlloc(MilSystem, M_EVENT, M_NOT_SIGNALED+M_AUTO_RESET, M_NULL, M_NULL, &ProcParamPtr.ThreadParam[ThreadIndex].DoneEvent);

      // Start the thread.
      MthrAlloc(MilSystem, M_THREAD, M_DEFAULT, &ParallelProcessingThread, &ProcParamPtr.ThreadParam[ThreadIndex], &ProcParamPtr.ThreadParam[ThreadIndex].MilThread);
      }
   }


void ParallelFree(PARALLEL_PROC_PARAM &ProcParamPtr)
   {
   MIL_ID   ThreadIndex;   // Thread loop counter index.

   if (ProcParamPtr.InitialMpUse == M_ENABLE)
      // Re-enable MP processing.
      MappControlMp(M_DEFAULT, M_MP_USE, M_DEFAULT, M_ENABLE, M_NULL);

   // Free MIL objects.
   MbufFree(ProcParamPtr.MilResizedBinImage);
   MbufFree(ProcParamPtr.MilStructElement);
   MbufFree(ProcParamPtr.MilResizedImage);
   MblobFree(ProcParamPtr.MilBlobResult);
   MblobFree(ProcParamPtr.MilBlobContext);

   // Free the MIL object allocated in each thread.
   for (ThreadIndex=0; ThreadIndex<ProcParamPtr.NumProcCores; ThreadIndex++)
      {
      // Stop the thread.
      ProcParamPtr.ThreadParam[ThreadIndex].DoExit = true;
      MthrControl(ProcParamPtr.ThreadParam[ThreadIndex].ReadyEvent, M_EVENT_SET, M_SIGNALED);
      MthrWait(ProcParamPtr.ThreadParam[ThreadIndex].DoneEvent, M_EVENT_WAIT, M_NULL);

      // Free the MIL resources that were allocated per thread.
      McodeFree(ProcParamPtr.ThreadParam[ThreadIndex].MilCodeContext);
      McodeFree(ProcParamPtr.ThreadParam[ThreadIndex].MilCodeResult);
      MgraFree(ProcParamPtr.ThreadParam[ThreadIndex].MilRoiGraList);
      MbufFree(ProcParamPtr.ThreadParam[ThreadIndex].MilImage);
      MthrFree(ProcParamPtr.ThreadParam[ThreadIndex].ReadyEvent);
      MthrFree(ProcParamPtr.ThreadParam[ThreadIndex].DoneEvent);

      // Free the thread.
      MthrFree(ProcParamPtr.ThreadParam[ThreadIndex].MilThread);
      }
   }


void ParallelProcessing(PARALLEL_PROC_PARAM &ProcParamPtr)
   {
   MIL_INT     NumberOfBlobs;    // Number of blobs found.
   MIL_ID      ThreadIndex;      // Thread loop counter index.
   MIL_DOUBLE  ExpectedAreaMin,  // Minimum expected size of the area of a blob.
               ExpectedAreaMax;  // Maximum expected size of the area of a blob.

   // Reset some processing parameters.
   for (ThreadIndex=0; ThreadIndex<ProcParamPtr.NumProcCores; ThreadIndex++)
      {
      // Set the number of codes found to 0.
      ProcParamPtr.ThreadParam[ThreadIndex].NumberOfCodes = 0;
      // Set the read status to 'not found'.
      ProcParamPtr.ThreadParam[ThreadIndex].ReadStatus = M_STATUS_NOT_FOUND;
      // Signal the 'done' event for each thread.
      MthrControl(ProcParamPtr.ThreadParam[ThreadIndex].DoneEvent, M_EVENT_SET, M_SIGNALED);
      }

   // Resize the image in order to reduce the processing time. A datamatrix cell will be represented by a single pixel.
   MimResize(ProcParamPtr.MilImage, ProcParamPtr.MilResizedImage, ProcParamPtr.ResizeFactor, ProcParamPtr.ResizeFactor, M_NEAREST_NEIGHBOR+M_OVERSCAN_ENABLE);

   // Perform a bottom-hat filtering to make the background uniform.
   MimMorphic(ProcParamPtr.MilResizedImage, ProcParamPtr.MilResizedImage, ProcParamPtr.MilStructElement, M_BOTTOM_HAT, 5, M_GRAYSCALE);

   // Binarize the image.
   MimBinarize(ProcParamPtr.MilResizedImage, ProcParamPtr.MilResizedBinImage, M_BIMODAL+M_GREATER_OR_EQUAL, M_NULL, M_NULL);

   // Perform a dilate filtering to merge broken datamatrix.
   MimDilate(ProcParamPtr.MilResizedBinImage, ProcParamPtr.MilResizedBinImage, 1, M_BINARY);

   // Compute the blobs.
   MblobCalculate(ProcParamPtr.MilBlobContext, ProcParamPtr.MilResizedBinImage, M_NULL, ProcParamPtr.MilBlobResult);

   // Exclude the blobs that are smaller than a minimum expected area.
   ExpectedAreaMin = (EXPECTED_CELL_NUMBER_X * EXPECTED_CELL_NUMBER_Y * 0.8);
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_AREA, M_LESS, ExpectedAreaMin, M_NULL);
   // Exclude the blobs that are larger than a maximum expected area.
   ExpectedAreaMax = EXPECTED_CELL_NUMBER_X * EXPECTED_CELL_NUMBER_Y*1.3;
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_AREA, M_GREATER, ExpectedAreaMax, M_NULL);
   // Exclude the blobs that have a bounding box aspect ratio too far from 1.
   MblobSelect(ProcParamPtr.MilBlobResult, M_EXCLUDE, M_BOX_ASPECT_RATIO, M_OUT_RANGE, 0.85, 1.15);

   // Get the number of included blobs.
   MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   if (NumberOfBlobs > 0)
      {
      MIL_INT              BlobIndex,     // Blob loop counter index.
                           ThreadIndex,   // Thread loop counter index.
                           EventIndex;    // Index of the signaled event.
      std::vector<MIL_INT> BoxXMin,       // X coordinate of the blobs upper left corner.
                           BoxXMax,       // X coordinate of the blobs lower right corner.
                           BoxYMin,       // Y coordinate of the blobs upper left corner.
                           BoxYMax;       // Y coordinate of the blobs lower right corner.

      // Get the bounding box of each blob.
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_X_MIN, BoxXMin);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_X_MAX, BoxXMax);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_Y_MIN, BoxYMin);
      MblobGetResult(ProcParamPtr.MilBlobResult, M_DEFAULT, M_BOX_Y_MAX, BoxYMax);

      // Read inside each blob region.
      for (BlobIndex=0; BlobIndex<NumberOfBlobs; BlobIndex++ )
         {
         // Wait for one of the thread to be ready to process.
         MthrWaitMultiple(&ProcParamPtr.DoneEvents[0], ProcParamPtr.NumProcCores, M_EVENT_WAIT, &EventIndex);

         // If a timing benchmark is not in progress, get the results.
         if (!ProcParamPtr.IsTimerActive)
            {
            if (ProcParamPtr.ThreadParam[EventIndex].ReadStatus == M_STATUS_READ_OK)
               {
               ParallelGetResults(ProcParamPtr.ThreadParam[EventIndex], ProcParamPtr.MilDispGraList);
               }
            }

         // Set the bounding box parameters in the thread parameter structure.
         ProcParamPtr.ThreadParam[EventIndex].BlobBox.MinX = BoxXMin[BlobIndex];
         ProcParamPtr.ThreadParam[EventIndex].BlobBox.MinY = BoxYMin[BlobIndex];
         ProcParamPtr.ThreadParam[EventIndex].BlobBox.MaxX = BoxXMax[BlobIndex];
         ProcParamPtr.ThreadParam[EventIndex].BlobBox.MaxY = BoxYMax[BlobIndex];

         // Set the 'ready' event to tell the thread it can process.
         MthrControl(ProcParamPtr.ThreadParam[EventIndex].ReadyEvent, M_EVENT_SET, M_SIGNALED);
         }

      // Wait for all of the thread to be finished.
      MthrWaitMultiple(&ProcParamPtr.DoneEvents[0], ProcParamPtr.NumProcCores, M_EVENT_WAIT+M_ALL_OBJECTS, M_NULL);

      // If a timing benchmark is not in progress, get the last results of each thread.
      if (!ProcParamPtr.IsTimerActive)
         {
         MIL_INT    CodeIndex;                                       // Code loop counter index.
         MIL_STRING CharBuffer(MAXIMUM_STRING_SIZE, MIL_TEXT('\0')); // String to display.

         for (ThreadIndex = 0; ThreadIndex < ProcParamPtr.NumProcCores; ThreadIndex++)
            {
            if (ProcParamPtr.ThreadParam[ThreadIndex].NumberOfCodes > 0)
               {
               // Get the results.
               ParallelGetResults(ProcParamPtr.ThreadParam[ThreadIndex], ProcParamPtr.MilDispGraList);

               // Display the decoded strings and print them in the console.
               for(CodeIndex = 0; CodeIndex < ProcParamPtr.ThreadParam[ThreadIndex].NumberOfCodes; CodeIndex++)
                  {
                  MIL_DOUBLE  PositionX = ProcParamPtr.ThreadParam[ThreadIndex].PosX[CodeIndex];
                  MIL_DOUBLE  PositionY = ProcParamPtr.ThreadParam[ThreadIndex].PosY[CodeIndex];

                  // Display the strings using the graphic list.
                  MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("Thread%d_Code%d"), ThreadIndex + 1, CodeIndex);
                  MgraText(M_DEFAULT, ProcParamPtr.MilDispGraList, PositionX + TEXT_OFFSET_X, PositionY + TEXT_OFFSET_Y_1, CharBuffer);
                  MosSprintf(&CharBuffer[0], MAXIMUM_STRING_SIZE, MIL_TEXT("%s"), ProcParamPtr.ThreadParam[ThreadIndex].CodesRead[CodeIndex].c_str());
                  MgraText(M_DEFAULT, ProcParamPtr.MilDispGraList, PositionX + TEXT_OFFSET_X, PositionY + TEXT_OFFSET_Y_2, CharBuffer);

                  // Print the results.
                  MosPrintf(MIL_TEXT("Thread%d Code%d\t%s\t(%.2f, %.2f)\n"), ThreadIndex + 1, CodeIndex + 1, ProcParamPtr.ThreadParam[ThreadIndex].CodesRead[CodeIndex].c_str(), PositionX, PositionY);
                  }
               }
            }
         }
      }
   }

MIL_UINT32 MFTYPE ParallelProcessingThread(void *ThreadParameters)
   {
   THREAD_PARAM   *ThreadParam = (THREAD_PARAM *)ThreadParameters;

   while(!ThreadParam->DoExit)
      {
      // Wait for the "ready" event to be signaled.
      MthrWait(ThreadParam->ReadyEvent, M_EVENT_WAIT, M_NULL);

      if (!ThreadParam->DoExit)
         {
         // Define a ROI around the blob. Leave a margin around the blob.
         MIL_INT  Margin = 3;
         MIL_INT  BoxStartX = (MIL_INT)((ThreadParam->BlobBox.MinX * (1/ThreadParam->ResizeFactor))-(Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxStartY = (MIL_INT)((ThreadParam->BlobBox.MinY * (1/ThreadParam->ResizeFactor))-(Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxSizeX = (MIL_INT)(((ThreadParam->BlobBox.MaxX - ThreadParam->BlobBox.MinX)*(1/ThreadParam->ResizeFactor))+(2*Margin*EXPECTED_CELL_SIZE));
         MIL_INT  BoxSizeY = (MIL_INT)(((ThreadParam->BlobBox.MaxY - ThreadParam->BlobBox.MinY)*(1/ThreadParam->ResizeFactor))+(2*Margin*EXPECTED_CELL_SIZE));

         // Clear the graphic list.
         MgraClear(M_DEFAULT, ThreadParam->MilRoiGraList);
         // Draw the ROI in the graphic list.
         MgraRectAngle(M_DEFAULT, ThreadParam->MilRoiGraList, BoxStartX, BoxStartY, BoxSizeX, BoxSizeY, 0, M_CORNER_AND_DIMENSION+M_FILLED);

         // Associate the modified graphic list with the image to process.
         MbufSetRegion(ThreadParam->MilImage, ThreadParam->MilRoiGraList, M_DEFAULT, M_NO_RASTERIZE, M_DEFAULT);

         // Read the code.
         McodeRead(ThreadParam->MilCodeContext, ThreadParam->MilImage, ThreadParam->MilCodeResult);

         // Get the read status.
         McodeGetResult(ThreadParam->MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ThreadParam->ReadStatus);
         
         // Increment the number of codes that were read on the current thread.
         if (ThreadParam->ReadStatus == M_STATUS_READ_OK)
            ThreadParam->NumberOfCodes++;
         }

      // Signal the "done" event.
      MthrControl(ThreadParam->DoneEvent, M_EVENT_SET, M_SIGNALED);
      }

   return(1);
   }

void ParallelGetResults(THREAD_PARAM &ThreadParamPtr, MIL_ID MilDispGraList)
   {
   MIL_INT CodeIndex = ThreadParamPtr.NumberOfCodes-1;

   if (CodeIndex >= 0)
      {
      // Get the decoded string and position of the code.
      McodeGetResult(ThreadParamPtr.MilCodeResult, 0, M_GENERAL, M_STRING, ThreadParamPtr.CodesRead[CodeIndex]);
      McodeGetResult(ThreadParamPtr.MilCodeResult, 0, M_GENERAL, M_POSITION_X, &ThreadParamPtr.PosX[CodeIndex]);
      McodeGetResult(ThreadParamPtr.MilCodeResult, 0, M_GENERAL, M_POSITION_Y, &ThreadParamPtr.PosY[CodeIndex]);

      // Draw results in the graphic list.
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      McodeDraw(M_DEFAULT, ThreadParamPtr.MilCodeResult, MilDispGraList, M_DRAW_BOX, M_ALL, M_GENERAL, M_DEFAULT);
      }
   }
