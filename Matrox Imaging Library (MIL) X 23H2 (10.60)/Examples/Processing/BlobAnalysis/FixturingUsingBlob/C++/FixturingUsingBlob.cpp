﻿/***************************************************************************************/
/* 
* File name: FixturingUsingBlob.cpp  
*
* Synopsis:  This program contains an example of reading a 1D Code 39 with fixturing  
*            using the Code Reader module. The fixturing is done with the Blob module.
*            See the PrintHeader() function below for a detailed description.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include "AutomaticBlobSelection.h"


///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("FixturingUsingBlob\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example reads linear codes using a predefined blob\n")
             MIL_TEXT("to fixture the operation.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to setup the read operation.\n\n"));
   MosGetch();
   }

///***************************************************************************
// Examples images.
///***************************************************************************

// Setup image.
static const MIL_CONST_TEXT_PTR SETUP_IMAGE = M_IMAGE_PATH MIL_TEXT("/FixturedCodeRead/BoardSetup.mim");

// Path of the target images to process.
static const MIL_CONST_TEXT_PTR PROCESS_PATH = M_IMAGE_PATH MIL_TEXT("/FixturedCodeRead@1fps");

///***************************************************************************
// Parameters.
///***************************************************************************

static const bool INTERACTIVE_EXAMPLE = false;

// Code parameters.
static const MIL_INT CODE_TYPE      = M_CODE39;
static const MIL_INT THRESHOLD_MODE = M_ADAPTIVE;

// Blob selection parameters.
static const MIL_DOUBLE THRESHOLD_VALUE = 128;
static const MIL_DOUBLE MIN_BREADTH = 6;
static const MIL_DOUBLE MIN_BLOB_AREA = 500;
static const MIL_DOUBLE MIN_DIM_FACTOR = 0.8;
static const MIL_DOUBLE MAX_DIM_FACTOR = 1.2;
static const MIL_DOUBLE MIN_FERET_RATIO = 1.5;

// Standalone parameters.
static const MIL_DOUBLE EXPECTED_BLOB_WIDTH = 415;
static const MIL_DOUBLE EXPECTED_BLOB_HEIGHT = 75;
static const MIL_DOUBLE CODE_REGION_START_X = 70;
static const MIL_DOUBLE CODE_REGION_START_Y = 330;
static const MIL_DOUBLE CODE_REGION_END_X = 500;
static const MIL_DOUBLE CODE_REGION_END_Y = 430;

// General parameters.
static const MIL_INT MAX_CODE_STRING_SIZE = 512;
static const MIL_INT GRA_TEXT_SIZE_Y = 16;

///***************************************************************************
// Useful structs.
///***************************************************************************
struct SProcessData
   {
   MIL_ID MilDisplay;
   MIL_ID MilGraList;
   MIL_ID MilRegionGraList;
   MIL_ID MilImage;
   MIL_ID MilSearchImage;
   MIL_ID MilSearchContext;
   MIL_ID MilSearchResult;
   MIL_ID MilFixturingOffset;
   MIL_ID MilCodeContext;
   MIL_ID MilCodeResult;

   MIL_DOUBLE BlobWidth;
   MIL_DOUBLE BlobHeight;
   };

///***************************************************************************
// Prototypes.
///***************************************************************************
bool CalculateBlobFixturePosition(MIL_ID MilImage,
                                  MIL_ID MilSearchImage,
                                  MIL_ID MilBlobContext,
                                  MIL_ID MilBlobResult,
                                  MIL_DOUBLE BlobWidth,
                                  MIL_DOUBLE BlobHeight,
                                  MIL_DOUBLE* pFixturePosX,
                                  MIL_DOUBLE* pFixturePosY,
                                  MIL_DOUBLE* pFixtureAngle);
void DeleteImpossibleBlobs(MIL_ID MilBlobResult);
void BinarizeImage(MIL_ID MilImage, MIL_ID MilSearchImage);
void DrawBlobFixtureAndRegion(MIL_ID MilDisplay, MIL_ID MilBlobResult, MIL_ID MilFixturingOffset, MIL_ID MilRegionGraList, MIL_ID MilGraList);
void ProcessImage(SProcessData* pProcessData);
MIL_INT MFTYPE DigProcess(MIL_INT HookType, MIL_ID MilEvent, void* pUserData);

///***************************************************************************
// Main.
///***************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem = M_DEFAULT_HOST;
   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Print header.
   PrintHeader();

   // Allocate a graphic list and associate it to the display. 
   MIL_ID MilGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   // Allocate a graphic list that will hold the region.
   MIL_ID MilRegionGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   // Restore the setup image.
   MIL_ID MilImage = MbufRestore(SETUP_IMAGE, MilSystem, M_NULL);
   MIL_ID MilGrabImage = MbufRestore(SETUP_IMAGE, MilSystem, M_NULL);
   MIL_INT ImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);
   MIL_ID MilSearchImage = MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   McalUniform(MilImage, 0, 0, 1, 1, 0, M_DEFAULT);
   MdispSelect(MilDisplay, MilImage);
            
   // Allocate blob. 
   MIL_ID MilBlobContext = MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilBlobResult = MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);
   MblobControl(MilBlobContext, M_BREADTH, M_ENABLE);
   MblobControl(MilBlobContext, M_BOX, M_ENABLE);
   MblobControl(MilBlobContext, M_MIN_AREA_BOX, M_ENABLE);
      
   // Choose a blob of a certain width and height.
   MIL_DOUBLE BlobWidth = EXPECTED_BLOB_WIDTH;
   MIL_DOUBLE BlobHeight = EXPECTED_BLOB_HEIGHT;

   if (INTERACTIVE_EXAMPLE)
      {
      bool BlobIsChosen = ChoosePossibleFixturingBlob(MilImage,
                                                   MilSearchImage,
                                                   MilDisplay,
                                                   MilBlobResult,
                                                   MilBlobContext,
                                                   MIN_DIM_FACTOR,
                                                   MAX_DIM_FACTOR,
                                                   MIN_FERET_RATIO,
                                                   BinarizeImage,
                                                   DeleteImpossibleBlobs,
                                                   &BlobWidth,
                                                   &BlobHeight);
   if (!BlobIsChosen)
      MosPrintf(MIL_TEXT("Default blob width and height will be used.\n\n"));
   }

   MIL_ID MilFixturingOffset = McalAlloc(MilSystem, M_FIXTURING_OFFSET, M_DEFAULT, M_NULL);
   MIL_DOUBLE FixturePosX;
   MIL_DOUBLE FixturePosY;
   MIL_DOUBLE FixtureAngle;
   CalculateBlobFixturePosition(MilImage, MilSearchImage, MilBlobContext, MilBlobResult, BlobWidth, BlobHeight, &FixturePosX, &FixturePosY, &FixtureAngle);
   McalFixture(M_NULL, MilFixturingOffset, M_LEARN_OFFSET, M_POINT_AND_ANGLE, M_DEFAULT, FixturePosX, FixturePosY, FixtureAngle, M_DEFAULT);
    
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);

   if (INTERACTIVE_EXAMPLE)
      {
      // Associate the region graphic list to the display.
      MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilRegionGraList);
   
      // Enable the interactivity of the graphic list.
      MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);

      // Define the Code Reader read region interactively.   
      MgraInteractive(M_DEFAULT, MilRegionGraList, M_GRAPHIC_TYPE_RECT, M_DEFAULT, M_DEFAULT);
      MIL_INT ListState;
      do
         {
         MosPrintf(MIL_TEXT("Interactively define a rectangular region around the code to be read.\n\n"));
         MosPrintf(MIL_TEXT("Press <Enter> when finished.\n\n"));
         MosGetch();
         MgraInquireList(MilRegionGraList, M_LIST, M_DEFAULT, M_INTERACTIVE_GRAPHIC_STATE, &ListState);
         }while(ListState == M_STATE_BEING_CREATED || ListState == M_STATE_WAITING_FOR_CREATION);

      // Make the rectangle not editable.
      MgraControlList(MilRegionGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_SELECTABLE, M_DISABLE);
      MgraControlList(MilRegionGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_EDITABLE, M_DISABLE);

      // Disable the interactivity of the graphic list.
      MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_DISABLE);

      // Print message.
      MosPrintf(MIL_TEXT("A relative region has been defined.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Reassociate the graphic list to the display.
      MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);   
      }
   else
      {
      MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
      MgraRect(M_DEFAULT, MilRegionGraList, CODE_REGION_START_X, CODE_REGION_START_Y, CODE_REGION_END_X, CODE_REGION_END_Y);
      }
      
   // Allocate a code reader context and result and add a code 39 code model to the context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_CODE39, M_NULL, M_DEFAULT, M_NULL);

   // Set the angle to according to the region and the threshold mode.
   McodeControl(MilCodeContext, M_THRESHOLD_MODE, THRESHOLD_MODE);
   McodeControl(MilCodeModel, M_SEARCH_ANGLE, M_ACCORDING_TO_REGION);

   // Allocate a virtual digitizer.
   MIL_ID MilDigitizer = MdigAlloc(MilSystem, M_DEFAULT, PROCESS_PATH, M_DEFAULT, M_NULL);

   // Pause to show the relative coordinate system, the fixturing offset and the region.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);   
   DrawBlobFixtureAndRegion(MilDisplay, MilBlobResult, MilFixturingOffset, MilRegionGraList, MilGraList);
   MosPrintf(MIL_TEXT("In the setup image, the user defined blob (in blue) is selected to fixture the\n")
             MIL_TEXT("read operation.\n")
             MIL_TEXT("The offset of the blob from the origin of the setup image (in green) is learnt.\n")
             MIL_TEXT("The relative region to perform the read operation (in red) is defined in\n")
             MIL_TEXT("world units.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("The code is now read continuously.\n")
      MIL_TEXT("For each image:\n")
      MIL_TEXT("-The blob is located in the image.\n")
      MIL_TEXT("-The relative coordinate system is moved accordingly.\n")
      MIL_TEXT("-The region is updated in the relative coordinate system.\n")
      MIL_TEXT("-The linear code is read in the region.\n\n")
      MIL_TEXT("Press <Enter> to end.\n\n"));

   // Process all the images.
   SProcessData ProcessData;
   ProcessData.MilDisplay = MilDisplay;
   ProcessData.MilGraList = MilGraList;
   ProcessData.MilRegionGraList = MilRegionGraList;
   ProcessData.MilImage = MilImage;
   ProcessData.MilSearchImage = MilSearchImage;
   ProcessData.MilSearchContext = MilBlobContext;
   ProcessData.MilSearchResult = MilBlobResult;
   ProcessData.MilFixturingOffset = MilFixturingOffset;
   ProcessData.MilCodeContext = MilCodeContext;
   ProcessData.MilCodeResult = MilCodeResult;
   ProcessData.BlobWidth = BlobWidth;
   ProcessData.BlobHeight = BlobHeight;
   MdigProcess(MilDigitizer, &MilGrabImage, 1, M_START, M_ASYNCHRONOUS, DigProcess, &ProcessData);
   MosGetch();

   // Stop the grab.
   MdigProcess(MilDigitizer, &MilGrabImage, 1, M_STOP, M_DEFAULT, M_NULL, M_NULL);

   // Free the digitizer.
   MdigFree(MilDigitizer);

   // Free Code.
   McodeFree(MilCodeResult);
   McodeFree(MilCodeContext);

   // Free the fixturing offset.
   McalFree(MilFixturingOffset);

   // Free Blob.
   MblobFree(MilBlobResult);
   MblobFree(MilBlobContext);

   // Free the image.
   if (MilSearchImage != MilImage)
      MbufFree(MilSearchImage);
   MbufFree(MilImage);   
   MbufFree(MilGrabImage);
   
   // Free the graphic list.
   MgraFree(MilRegionGraList);
   MgraFree(MilGraList);

   MdispFree(MilDisplay);
   if (MilSystem != M_DEFAULT_HOST)
      MsysFree(MilSystem);
   MappFree(MilApplication);
   return 0;
   }

///***************************************************************************
// BinarizeImage. Binarize the image for blob.
///***************************************************************************
void BinarizeImage(MIL_ID MilImage, MIL_ID MilSearchImage)
   {
   MimBinarize(MilImage, MilSearchImage, M_FIXED + M_GREATER, THRESHOLD_VALUE, M_NULL);
   }

///***************************************************************************
// DeleteImpossibleBlobs. Deletes blobs that definitely cannot be the 
//                        fixturing blob.
///***************************************************************************
void DeleteImpossibleBlobs(MIL_ID MilBlobResult)
   {
   MblobSelect(MilBlobResult, M_DELETE, M_AREA, M_LESS, MIN_BLOB_AREA, M_NULL);
   MblobSelect(MilBlobResult, M_DELETE, M_BLOB_TOUCHING_IMAGE_BORDERS, M_NULL, M_NULL, M_NULL);
   MblobSelect(MilBlobResult, M_DELETE, M_BREADTH, M_LESS, MIN_BREADTH, M_NULL);
   }

///***************************************************************************
// DrawBlobFixtureAndRegion. Draws the blob and the fixturing offset and copies
//                           the region in the displayed graphic list.
///***************************************************************************
void DrawBlobFixtureAndRegion(MIL_ID MilDisplay, MIL_ID MilBlobResult, MIL_ID MilFixturingOffset, MIL_ID MilRegionGraList, MIL_ID MilGraList)
   {
   // Disable the display updates.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
   
   // Draw the blob.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraList, M_DRAW_BLOBS, M_DEFAULT, M_DEFAULT);

   // Draw the coordinate system.
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_BLUE);
   McalDraw(M_DEFAULT, M_NULL, MilGraList, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   // Draw the fixturing offset.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   McalDraw(M_DEFAULT, MilFixturingOffset, MilGraList, M_DRAW_FIXTURING_OFFSET, M_DEFAULT, M_DEFAULT);
   
   // Copy the region in the display graphic list.
   MgraCopy(MilRegionGraList, MilGraList, M_COPY + M_INDEX_VALUE, M_DEFAULT, M_ALL, M_NULL, M_NULL, M_DEFAULT);
   
   // Enable the display updates.
   MdispControl(MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);
   }

///***************************************************************************
// CalculateBlobFixturePosition. Finds the desired blob that is our fixture
//                               source from the image.
///***************************************************************************
bool CalculateBlobFixturePosition(MIL_ID MilImage,
                                  MIL_ID MilSearchImage,
                                  MIL_ID MilBlobContext,
                                  MIL_ID MilBlobResult,
                                  MIL_DOUBLE BlobWidth,
                                  MIL_DOUBLE BlobHeight,
                                  MIL_DOUBLE* pFixturePosX,
                                  MIL_DOUBLE* pFixturePosY,
                                  MIL_DOUBLE* pFixtureAngle)
   {
   // Binarize the image.
   BinarizeImage(MilImage, MilSearchImage);
   
   // Calculate the blobs.
   MblobCalculate(MilBlobContext, MilSearchImage, M_NULL, MilBlobResult);
   
   // Keep only the blobs that correspond to our blob's characteristics.
   DeleteImpossibleBlobs(MilBlobResult);
   MblobSelect(MilBlobResult, M_DELETE, M_MIN_AREA_BOX_HEIGHT, M_OUT_RANGE, BlobHeight*MIN_DIM_FACTOR, BlobHeight*MAX_DIM_FACTOR);
   MblobSelect(MilBlobResult, M_DELETE, M_MIN_AREA_BOX_WIDTH, M_OUT_RANGE, BlobWidth*MIN_DIM_FACTOR, BlobWidth*MAX_DIM_FACTOR);
   
   // Get the position and angle of the blob.
   MIL_INT NbBlobs;
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NbBlobs);
   if (NbBlobs == 1)
      {
      MIL_INT* pBlobLabels = new MIL_INT[NbBlobs];
      MblobGetResult(MilBlobResult, M_DEFAULT, M_LABEL_VALUE + M_TYPE_MIL_INT, pBlobLabels);
      MblobGetResult(MilBlobResult, pBlobLabels[0], M_MIN_AREA_BOX_CENTER_X, pFixturePosX);
      MblobGetResult(MilBlobResult, pBlobLabels[0], M_MIN_AREA_BOX_CENTER_Y, pFixturePosY);
      MblobGetResult(MilBlobResult, pBlobLabels[0], M_MIN_AREA_BOX_ANGLE, pFixtureAngle);
      delete[] pBlobLabels;
      }
   else
      return false;

   return true;
   }

///***************************************************************************
// ProcessImage. Processing function of the grabbed image.
//               Calculate the fixture, set the region, read the code and 
//               display the results.
///***************************************************************************
void ProcessImage(SProcessData* pProcessData)
   {
   MIL_DOUBLE StartTime = MappTimer(M_TIMER_READ, M_NULL);

   // Clear the graphic list.
   MgraClear(M_DEFAULT, pProcessData->MilGraList);
   
   // Locate the blob in the image.
   MIL_DOUBLE FixturePosX;
   MIL_DOUBLE FixturePosY;
   MIL_DOUBLE FixtureAngle;
   CalculateBlobFixturePosition(pProcessData->MilImage,
                                pProcessData->MilSearchImage,
                                pProcessData->MilSearchContext,
                                pProcessData->MilSearchResult,
                                pProcessData->BlobWidth,
                                pProcessData->BlobHeight,
                                &FixturePosX, &FixturePosY, &FixtureAngle);

   // Fixture the image.
   McalFixture(pProcessData->MilImage, pProcessData->MilFixturingOffset, M_MOVE_RELATIVE, M_POINT_AND_ANGLE, M_DEFAULT_UNIFORM_CALIBRATION, FixturePosX, FixturePosY, FixtureAngle, M_DEFAULT);

   // Set the regions.
   MbufSetRegion(pProcessData->MilImage, pProcessData->MilRegionGraList, M_DEFAULT, M_NO_RASTERIZE + M_FILL_REGION, M_DEFAULT);

   // Read the code.
   McodeRead(pProcessData->MilCodeContext, pProcessData->MilImage, pProcessData->MilCodeResult);

   // Get the status of the read.
   MIL_INT Status;
   McodeGetResult(pProcessData->MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &Status);

   // If the code wasn't read, try to read at 180 degrees.
   if (Status != M_STATUS_READ_OK)
      {
      McalFixture(pProcessData->MilImage, pProcessData->MilFixturingOffset, M_MOVE_RELATIVE, M_POINT_AND_ANGLE, M_DEFAULT_UNIFORM_CALIBRATION, FixturePosX, FixturePosY, FixtureAngle+180, M_DEFAULT);
      McodeRead(pProcessData->MilCodeContext, pProcessData->MilImage, pProcessData->MilCodeResult);
      McodeGetResult(pProcessData->MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &Status);
      }

   MIL_DOUBLE EndTime = MappTimer(M_TIMER_READ, M_NULL);

   // Draw the blob result.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);
   DrawBlobFixtureAndRegion(pProcessData->MilDisplay, pProcessData->MilSearchResult, pProcessData->MilFixturingOffset, pProcessData->MilRegionGraList, pProcessData->MilGraList);

   // Draw the code read.
   MIL_TEXT_CHAR OutputString[MAX_CODE_STRING_SIZE] = MIL_TEXT("No Code Read");
   MIL_TEXT_CHAR CodeString[MAX_CODE_STRING_SIZE];
   MgraColor(M_DEFAULT, M_COLOR_RED);
   if (Status == M_STATUS_READ_OK)
      {
      McodeGetResult(pProcessData->MilCodeResult, 0, M_GENERAL, M_STRING, CodeString);
      MosSprintf(OutputString, MAX_CODE_STRING_SIZE, MIL_TEXT("Code Read: %s"), CodeString);
      McodeDraw(M_DEFAULT, pProcessData->MilCodeResult, pProcessData->MilGraList, M_DRAW_CODE, 0, M_GENERAL, M_DEFAULT);
      }
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_DISPLAY);
   MgraText(M_DEFAULT, pProcessData->MilGraList, 0, 0, OutputString);
   MosSprintf(OutputString, MAX_CODE_STRING_SIZE, MIL_TEXT("Processing Time: %.2f ms"), (EndTime - StartTime) * 1000);
   MgraText(M_DEFAULT, pProcessData->MilGraList, 0, GRA_TEXT_SIZE_Y, OutputString);
   }

///***************************************************************************
// DigProcess. Dig process callback.
///***************************************************************************
MIL_INT MFTYPE DigProcess(MIL_INT HookType, MIL_ID MilEvent, void* pUserData)
   {
   SProcessData* pProcessData = (SProcessData*)pUserData;

   // Disable display updates.
   MdispControl(pProcessData->MilDisplay, M_UPDATE, M_DISABLE);

   // Get the modified buffer, copy it and delete the region from the image.
   MIL_ID MilGrabBuffer;
   MdigGetHookInfo(MilEvent, M_MODIFIED_BUFFER + M_BUFFER_ID, &MilGrabBuffer);
   MbufSetRegion(pProcessData->MilImage, M_NULL, M_DEFAULT, M_DELETE, M_DEFAULT);
   MbufCopy(MilGrabBuffer, pProcessData->MilImage);

   // Process the image.
   ProcessImage(pProcessData);

   // Enable display updates.
   MdispControl(pProcessData->MilDisplay, M_UPDATE, M_ENABLE);
   return 0;
   }


