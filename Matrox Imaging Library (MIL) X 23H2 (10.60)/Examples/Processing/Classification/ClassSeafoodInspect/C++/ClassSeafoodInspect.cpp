﻿///*************************************************************************************
//
// File name: ClassSeaFoodInspect.cpp
//
// Synopsis:  This example identifies good/bad seafood products using
// a pre-trained classification module. 
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <algorithm>

// Path definitions.
#define EXAMPLE_IMAGE_DIR_PATH   M_IMAGE_PATH MIL_TEXT("/Classification/Seafood/")
#define EXAMPLE_CLASS_CTX_PATH   EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("MatroxNet_SeafoodInspect.mclass")
#define TARGET_IMAGE_DIR_PATH    EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("Products")

// Use the images from the example folder by default.
#define USE_EXAMPLE_IMAGE_FOLDER

// Util constant.
#define BUFFERING_SIZE_MAX 10
#define BINARIZATION_THRESHOLD    230

// Function declarations.
struct ClassStruct
   {
   MIL_INT NbOfFrames,
      SourceSizeX,
      SourceSizeY,
      SourceLayerSizeX,
      SourceLayerSizeY;

   MIL_ID MilBlobCtx,
      MilBlobRes,
      MilMimCtx,
      MilClassCtx,
      MilClassRes,
      MilDisplay,
      MilDispImage,
      MilOverlayImage;
   };

MIL_INT MFTYPE ClassificationFunc(MIL_INT HookType,
                                  MIL_ID  EventId,
                                  void*   pHookData);

void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_INT SourceSizeX,
                  MIL_INT SourceSizeY,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilOverlay);

void ProcessStatus(MIL_INT Status);

#ifdef USE_EXAMPLE_IMAGE_FOLDER
#define SYSTEM_TO_USE M_SYSTEM_HOST
#define DCF_TO_USE TARGET_IMAGE_DIR_PATH
#else
#define SYSTEM_TO_USE M_SYSTEM_DEFAULT
#define DCF_TO_USE MIL_TEXT("M_DEFAULT")
#endif 

///****************************************************************************
//    Main.
///****************************************************************************
int main(void)
   {
   MIL_ID MilApplication,    // MIL application identifier.
      MilSystem,             // MIL system identifier.
      MilDisplay,            // MIL display identifier.
      MilOverlay,            // MIL overlay identifier.
      MilDigitizer,          // MIL digitizer identifier.
      MilDispImage,          // MIL image identifier.
      MilMimCtx,             // MIL Image Processing context. 
      MilBlobCtx,            // MIl Blob Context.
      MilBlobRes,            // MIl Blob Result.
      MilClassCtx,           // MIL classification Context.
      MilClassRes;           // MIL classification Result.

   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX];   // MIL image identifier.

   MIL_INT NumberOfCategories,      // Number of categories.
      BufIndex,                     // Digitizer Buffer index.
      SourceSizeX,                  // Digitizer Input image Size X.
      SourceSizeY,                  // Digitizer Input image Size Y.
      SourceLayerSizeX,             // Classification target image size X.
      SourceLayerSizeY;             // Classification target image size Y.

   // Allocate MIL objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, SYSTEM_TO_USE, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);
   MdigAlloc(MilSystem, M_DEFAULT, DCF_TO_USE, M_DEFAULT, &MilDigitizer);

   // Print the example synopsis.
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("ClassSeafoodInspect\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows the usage of a pre-trained classification\n"));
   MosPrintf(MIL_TEXT("tool to inspect seafood (mussels) captured by an X-ray camera.\n"));
   MosPrintf(MIL_TEXT("Mussels with remaining pieces of shell must be rejected.\n\n"));
   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Classification, Blob, Buffer, Display, Graphics, Image Processing.\n\n"));

   // Wait for user.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetchar();

   MosPrintf(MIL_TEXT("Restoring the classification context from file.."));
   MclassRestore(EXAMPLE_CLASS_CTX_PATH, MilSystem, M_DEFAULT, &MilClassCtx);
   MosPrintf(MIL_TEXT("."));

   // Preprocess the context.
   MclassPreprocess(MilClassCtx, M_DEFAULT);
   MosPrintf(MIL_TEXT(".ready.\n\n"));

   MclassInquire(MilClassCtx, M_CONTEXT, M_NUMBER_OF_CLASSES + M_TYPE_MIL_INT, &NumberOfCategories);
   MclassInquire(MilClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_X + M_TYPE_MIL_INT, &SourceLayerSizeX);
   MclassInquire(MilClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_Y + M_TYPE_MIL_INT, &SourceLayerSizeY);

   // Inquire and print source layer information.
   MosPrintf(MIL_TEXT("The classifier was trained to recognize %d classes.\n"), NumberOfCategories);
   MosPrintf(MIL_TEXT("- Good mussel (keep).\n"));
   MosPrintf(MIL_TEXT("- Defective mussel (reject).\n\n"));
   MosPrintf(MIL_TEXT("The classifier was trained using %dx%d source images.\n\n"), SourceLayerSizeX, SourceLayerSizeY);
   MosPrintf(MIL_TEXT("Mussels classified as Good, and the score of that classification, are shown\nin green.\n"));
   MosPrintf(MIL_TEXT("Mussels classified as Defective, and the score of that classification, are\nshown in red.\n\n"));

   // Allocate a classification result buffer.
   MclassAllocResult(MilSystem, M_PREDICT_CNN_RESULT, M_DEFAULT, &MilClassRes);

   // Allocate a Blob Context and Result. 
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobCtx);
   MblobAllocResult(MilSystem, &MilBlobRes);

   // Control Block for Blob Context.
   MblobControl(MilBlobCtx, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobControl(MilBlobCtx, M_FOREGROUND_VALUE, M_ZERO);
   MblobControl(MilBlobCtx, M_BOX, M_ENABLE);

   // Allocate an Image Processing context. 
   MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, &MilMimCtx);
   MimControl(MilMimCtx, M_FILTER_OPERATION, M_SMOOTH);
   MimControl(MilMimCtx, M_FILTER_TYPE, M_SHEN);
   MimControl(MilMimCtx, M_FILTER_SMOOTHNESS, 80);

   // Inquire the size of the source image.
   MdigInquire(MilDigitizer, M_SIZE_X, &SourceSizeX);
   MdigInquire(MilDigitizer, M_SIZE_Y, &SourceSizeY);

   // Setup the example display.
   SetupDisplay(MilSystem,
                MilDisplay,
                SourceSizeX,
                SourceSizeY,
                MilDispImage,
                MilOverlay);

   // Retrieve the number of frames in the source directory.
   MIL_INT NumberOfFrames;
   MdigInquire(MilDigitizer, M_SOURCE_NUMBER_OF_FRAMES, &NumberOfFrames);

   // Prepare data for Hook Function.
   ClassStruct ClassificationData;
   ClassificationData.MilBlobCtx = MilBlobCtx;
   ClassificationData.MilBlobRes = MilBlobRes;
   ClassificationData.MilClassCtx = MilClassCtx;
   ClassificationData.MilClassRes = MilClassRes;
   ClassificationData.MilMimCtx = MilMimCtx;
   ClassificationData.MilDisplay = MilDisplay;
   ClassificationData.MilDispImage = MilDispImage;
   ClassificationData.MilOverlayImage = MilOverlay;

   ClassificationData.SourceSizeX = SourceSizeX;
   ClassificationData.SourceSizeY = SourceSizeY;
   ClassificationData.SourceLayerSizeX = SourceLayerSizeX;
   ClassificationData.SourceLayerSizeY = SourceLayerSizeY;
   ClassificationData.NbOfFrames = NumberOfFrames;

   // Allocate the grab buffers.
   for(BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
      {
      MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, 8 + M_UNSIGNED, M_IMAGE + M_GRAB + M_PROC + M_DISP, &MilGrabBufferList[BufIndex]);
      }

   // Start the grab.
   if(NumberOfFrames != M_INFINITE)
      MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, M_SEQUENCE + M_COUNT(NumberOfFrames), M_SYNCHRONOUS, &ClassificationFunc, &ClassificationData);
   else
      MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, M_START, M_DEFAULT, &ClassificationFunc, &ClassificationData);

   // Ready to exit.
   MosPrintf(MIL_TEXT("\nPress <Enter> to exit.\n"));
   MosGetch();

   // Stop the digitizer.
   MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE_MAX, M_STOP, M_DEFAULT, M_NULL, M_NULL);

   // Free the allocated resources.
   MdigFree(MilDigitizer);
   MbufFree(MilDispImage);

   for(BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
      {
      MbufFree(MilGrabBufferList[BufIndex]);
      }

   // Free objects
   MimFree(MilMimCtx);

   MblobFree(MilBlobCtx);
   MblobFree(MilBlobRes);

   MclassFree(MilClassRes);
   MclassFree(MilClassCtx);

   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_INT SourceSizeX,
                  MIL_INT SourceSizeY,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilOverlay)
   {
   // Allocate the display image. 
   MilDispImage = MbufAllocColor(MilSystem, 1, SourceSizeX, SourceSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);

   MbufClear(MilDispImage, M_COLOR_BLACK);

   // Display the window with black color.
   MdispSelect(MilDisplay, MilDispImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);

   return;
   }

void ImgPreprocess(MIL_ID MimCtx, MIL_ID MilImage, MIL_ID &MilPrepImage)
   {
   // Remove white noises on the object.
   MimOpen(MilImage, MilPrepImage, 2, M_GRAYSCALE);

   // Remove noise.
   MimConvolve(MilPrepImage, MilPrepImage, MimCtx);

   // Binarize the image.
   MimBinarize(MilPrepImage, MilPrepImage, M_FIXED + M_GREATER, BINARIZATION_THRESHOLD, M_NULL);

   // Fill the holes.
   MimDilate(MilPrepImage, MilPrepImage, 3, M_BINARY);

   // Separate adjacent objects.
   MimClose(MilPrepImage, MilPrepImage, 10, M_BINARY);

   return;
   }

void ProcessStatus(MIL_INT Status)
   {
   if(Status != M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("The prediction failed to complete.\n"));
      MosPrintf(MIL_TEXT("The status returned was: "));
      switch(Status)
         {
         default:
         case M_INTERNAL_ERROR:
            MosPrintf(MIL_TEXT("M_INTERNAL_ERROR\n"));
            break;
         case M_PREDICT_NOT_PERFORMED:
            MosPrintf(MIL_TEXT("M_PREDICT_NOT_PERFORMED\n"));
            break;
         case M_CURRENTLY_PREDICTING:
            MosPrintf(MIL_TEXT("M_CURRENTLY_PREDICTING\n"));
            break;
         case M_STOPPED_BY_REQUEST:
            MosPrintf(MIL_TEXT("M_STOPPED_BY_REQUEST\n"));
            break;
         case M_TIMEOUT_REACHED:
            MosPrintf(MIL_TEXT("M_TIMEOUT_REACHED\n"));
            break;
         case M_NOT_ENOUGH_MEMORY:
            MosPrintf(MIL_TEXT("M_NOT_ENOUGH_MEMORY\n"));
            break;
         }
      }
   }

MIL_INT MFTYPE ClassificationFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* DataPtr)
   {
   MIL_ID MilImage,        // Digitizer input image.
      MilPrepImage,        // Preprocessing image.
      PredictionChild;     // Classification child.

   MIL_INT Counter,        // Number of objects available in the image.
      ChildPosX,           // Classification child pose X.
      ChildPosY;           // Classification child pose Y.

   MIL_INT WinMargin = 2;

   std::vector<MIL_INT> CoG_X, CoG_Y;     // Center of Gravity of the blob.

   // Get image from digitizer.
   MdigGetHookInfo(EventId, M_MODIFIED_BUFFER + M_BUFFER_ID, &MilImage);

   ClassStruct* data = static_cast<ClassStruct*>(DataPtr);
   MdispControl(data->MilDisplay, M_UPDATE, M_DISABLE);
   MbufCopy(MilImage, data->MilDispImage);

   // Clear the overlay buffer.
   MdispControl(data->MilDisplay, M_OVERLAY_CLEAR, M_TRANSPARENT_COLOR);

   // Preprocess the image.
   MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &MilPrepImage);
   ImgPreprocess(data->MilMimCtx, MilImage, MilPrepImage);

   // Blob Calculation.
   MblobCalculate(data->MilBlobCtx, MilPrepImage, M_NULL, data->MilBlobRes);
   MblobSelect(data->MilBlobRes, M_EXCLUDE, M_AREA, M_LESS, 5, M_NULL);
   MblobSelect(data->MilBlobRes, M_EXCLUDE, M_BLOB_TOUCHING_IMAGE_BORDERS, M_NULL, M_NULL, M_NULL);

   // Get number of blobs.
   MblobGetResult(data->MilBlobRes, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &Counter);
   if(Counter > 0)
      {
      CoG_X.resize(Counter);
      CoG_Y.resize(Counter);

      for(MIL_INT i = 0; i < Counter; i++)
         {
         MblobGetResult(data->MilBlobRes, M_INCLUDED_BLOBS, M_CENTER_OF_GRAVITY_X, CoG_X);
         MblobGetResult(data->MilBlobRes, M_INCLUDED_BLOBS, M_CENTER_OF_GRAVITY_Y, CoG_Y);

         // Check if the child is in the boundary.
         ChildPosX = std::max(int(CoG_X[i] - data->SourceLayerSizeX / 2), (int)WinMargin);
         ChildPosY = std::max(int(CoG_Y[i] - data->SourceLayerSizeY / 2), (int)WinMargin);

         ChildPosX = std::min(data->SourceSizeX - data->SourceLayerSizeX - (int)WinMargin, ChildPosX);
         ChildPosY = std::min(data->SourceSizeY - data->SourceLayerSizeY - (int)WinMargin, ChildPosY);

         // Allocate the child for classification.
         MbufChild2d(MilImage, ChildPosX, ChildPosY, data->SourceLayerSizeX, data->SourceLayerSizeY, &PredictionChild);

         // Perform product recognition using the classification module.
         MclassPredict(data->MilClassCtx, PredictionChild, data->MilClassRes, M_DEFAULT);

         MIL_INT Status {};
         MclassGetResult(data->MilClassRes, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);
         ProcessStatus(Status);

         // Retrieve best classification score and class index.
         MIL_DOUBLE BestScore;
         MclassGetResult(data->MilClassRes, M_GENERAL, M_BEST_CLASS_SCORE + M_TYPE_MIL_DOUBLE, &BestScore);

         MIL_INT BestIndex;
         MclassGetResult(data->MilClassRes, M_GENERAL, M_BEST_CLASS_INDEX + M_TYPE_MIL_INT, &BestIndex);

         // Draw a green/red rectangle around the object if it is clear/not clear, receptively.
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         if(BestIndex == 1)
            MgraColor(M_DEFAULT, M_COLOR_RED);

         MgraRect(M_DEFAULT,
                  data->MilOverlayImage,
                  ChildPosX,
                  ChildPosY,
                  ChildPosX + data->SourceLayerSizeX,
                  ChildPosY + data->SourceLayerSizeY);

         // Print the classification accuracy in the sample buffer.
         MIL_TEXT_CHAR Accuracy_text[256];
         MosSprintf(Accuracy_text, 256, MIL_TEXT("  %.2lf%% score"), BestScore);
         MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
         MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);
         MgraText(M_DEFAULT, data->MilOverlayImage, ChildPosX, ChildPosY, Accuracy_text);

         MbufFree(PredictionChild);
         }
      }
   MbufFree(MilPrepImage);

   // Update the display.
   MdispControl(data->MilDisplay, M_UPDATE, M_ENABLE);

   // Wait for the user.
   if(data->NbOfFrames != M_INFINITE)
      {
      MosPrintf(MIL_TEXT("A prediction was performed on a target image.\nPress <Enter> to continue.\r"));
      MosGetch();
      }

   return 0;
   }