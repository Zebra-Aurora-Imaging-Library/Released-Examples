///*************************************************************************************
//
// File name: Mclass.cpp
//
// Synopsis:  This example identifies the type of pastas using a 
// pre-trained classification module. 
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

// Path definitions.
#define EXAMPLE_IMAGE_DIR_PATH   M_IMAGE_PATH MIL_TEXT("/Classification/Pasta/")
#define EXAMPLE_CLASS_CTX_PATH   EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("MatroxNet_PastaEx.mclass")
#define TARGET_IMAGE_DIR_PATH    EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("Products")

#define DIG_IMAGE_FOLDER         TARGET_IMAGE_DIR_PATH
#define DIG_REMOTE_IMAGE_FOLDER  MIL_TEXT("remote:///") TARGET_IMAGE_DIR_PATH

// Util constant.
#define BUFFERING_SIZE_MAX 10

// Function declarations.
struct ClassStruct
   {
   MIL_INT NbCategories,
           NbOfFrames,
           SourceSizeX,
           SourceSizeY;

   MIL_ID ClassCtx,
          ClassRes,
          MilDisplay,
          MilDispChild,
          MilOverlayImage;
   };

MIL_INT MFTYPE ClassificationFunc(MIL_INT HookType,
                                  MIL_ID  EventId,
                                  void*   pHookData);

void ProcessStatus(MIL_INT Status);

void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_INT SourceSizeX,
                  MIL_INT SourceSizeY,
                  MIL_ID  ClassCtx,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilDispChild,
                  MIL_ID  &MilOverlay,
                  MIL_INT NbCategories);

///****************************************************************************
//    Main.
///****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication,    // MIL application identifier
          MilSystem,         // MIL system identifier
          MilDisplay,        // MIL display identifier
          MilOverlay,        // MIL overlay identifier
          MilDigitizer,      // MIL digitizer identifier
          MilDispImage,      // MIL image identifier
          MilDispChild,      // MIL image identifier
          ClassCtx,          // MIL classification Context
          ClassRes;          // MIL classification Result

   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX],   // MIL image identifier
          MilChildBufferList[BUFFERING_SIZE_MAX];  // MIL child identifier

   MIL_INT NumberOfCategories,
           BufIndex,
           SourceSizeX,
           SourceSizeY,
           InputSizeX,
           InputSizeY;

   NumberOfCategories = 0;

   // Allocate MIL objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_DEFAULT, M_DEFAULT, M_DEFAULT, &MilSystem);
   MIL_INT SystemType {};
   MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
   if(SystemType != M_SYSTEM_HOST_TYPE)
      {
      MsysFree(MilSystem);
      MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
      }

   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);
    
   MIL_INT MilSystemLocation = MsysInquire(MilSystem, M_LOCATION, M_NULL);
   MIL_CONST_TEXT_PTR DigImageFolder = (MilSystemLocation == M_REMOTE) ? DIG_REMOTE_IMAGE_FOLDER : DIG_IMAGE_FOLDER;
   MdigAlloc(MilSystem, M_DEFAULT, DigImageFolder, M_DEFAULT, &MilDigitizer);
      
   // Print the example synopsis.
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Mclass\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This programs shows the use of a pre-trained classification\n"));
   MosPrintf(MIL_TEXT("tool to recognize product categories.\n\n"));
   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Classification, Buffer, Display, Graphics, Image Processing.\n\n"));

   // Wait for user.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   
   MosPrintf(MIL_TEXT("Restoring the classification context from file.."));
   MclassRestore(EXAMPLE_CLASS_CTX_PATH, MilSystem, M_DEFAULT, &ClassCtx);
   MosPrintf(MIL_TEXT("."));

   // Preprocess the context.
   MclassPreprocess(ClassCtx, M_DEFAULT);
   MosPrintf(MIL_TEXT(".ready.\n"));

   MclassInquire(ClassCtx, M_CONTEXT, M_NUMBER_OF_CLASSES   + M_TYPE_MIL_INT, &NumberOfCategories);
   MclassInquire(ClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_X + M_TYPE_MIL_INT, &InputSizeX);
   MclassInquire(ClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_Y + M_TYPE_MIL_INT, &InputSizeY);

   if(NumberOfCategories > 0)
      {
      // Inquire and print source layer information.
      MosPrintf(MIL_TEXT(" - The classifier was trained to recognize %d categories\n"), NumberOfCategories);
      MosPrintf(MIL_TEXT(" - The classifier was trained for %dx%d source images\n\n"), InputSizeX, InputSizeY);

      // Allocate a classification result buffer.
      MclassAllocResult(MilSystem, M_PREDICT_CNN_RESULT, M_DEFAULT, &ClassRes);

      // Inquire the size of the source image.
      MdigInquire(MilDigitizer, M_SIZE_X, &SourceSizeX);
      MdigInquire(MilDigitizer, M_SIZE_Y, &SourceSizeY);

      // Setup the example display.
      SetupDisplay(MilSystem,
                   MilDisplay,
                   SourceSizeX,
                   SourceSizeY,
                   ClassCtx,
                   MilDispImage,
                   MilDispChild,
                   MilOverlay,
                   NumberOfCategories);

      // Retrieve the number of frame in the source directory.
      MIL_INT NumberOfFrames;
      MdigInquire(MilDigitizer, M_SOURCE_NUMBER_OF_FRAMES, &NumberOfFrames);

      // Prepare data for Hook Function.
      ClassStruct ClassificationData;
      ClassificationData.ClassCtx = ClassCtx;
      ClassificationData.ClassRes = ClassRes;
      ClassificationData.MilDisplay = MilDisplay;
      ClassificationData.MilDispChild = MilDispChild;
      ClassificationData.NbCategories = NumberOfCategories;
      ClassificationData.MilOverlayImage = MilOverlay;
      ClassificationData.SourceSizeX = SourceSizeX;
      ClassificationData.SourceSizeY = SourceSizeY;
      ClassificationData.NbOfFrames = NumberOfFrames;

      // Allocate the grab buffers.
      for(BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
         {
         MbufAlloc2d(MilSystem, SourceSizeX, SourceSizeY, 8 + M_UNSIGNED, M_IMAGE + M_GRAB + M_PROC, &MilGrabBufferList[BufIndex]);
         MbufChild2d(MilGrabBufferList[BufIndex], (SourceSizeX - InputSizeX) / 2, (SourceSizeY - InputSizeY) / 2, InputSizeX, InputSizeY, &MilChildBufferList[BufIndex]);
         MobjControl(MilGrabBufferList[BufIndex], M_OBJECT_USER_DATA_PTR, &MilChildBufferList[BufIndex]);
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

      MbufFree(MilDispChild);
      MbufFree(MilDispImage);

      for(BufIndex = 0; BufIndex < BUFFERING_SIZE_MAX; BufIndex++)
         {
         MbufFree(MilChildBufferList[BufIndex]);
         MbufFree(MilGrabBufferList[BufIndex]);
         }

      MclassFree(ClassRes);
      MclassFree(ClassCtx);
      }

   // Free the allocated resources.
   MdigFree(MilDigitizer);

   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_INT SourceSizeX,
                  MIL_INT SourceSizeY,
                  MIL_ID  ClassCtx,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilDispChild,
                  MIL_ID  &MilOverlay,
                  MIL_INT NbCategories
                  )
   {
   MIL_ID MilImageLoader,  // MIL image identifier       
          MilChildSample;  // MIL child image identifier
          
   // Allocate a color buffer.
   MIL_INT IconSize = SourceSizeY / NbCategories;
   MilDispImage = MbufAllocColor(MilSystem, 3, SourceSizeX + IconSize, SourceSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MbufClear(MilDispImage, M_COLOR_BLACK);
   MilDispChild = MbufChild2d(MilDispImage, 0, 0, SourceSizeX, SourceSizeY, M_NULL);

   // Set annotation color.
   MgraColor(M_DEFAULT, M_COLOR_RED);

   // Setup the display.
   for (int iter = 0; iter < NbCategories; iter++)
      {
      // Allocate a child buffer per product categorie.   
      MbufChild2d(MilDispImage, SourceSizeX, iter * IconSize, IconSize, IconSize, &MilChildSample);

      // Load the sample image.
      MclassInquire(ClassCtx, M_CLASS_INDEX(iter), M_CLASS_ICON_ID + M_TYPE_MIL_ID, &MilImageLoader);
      
      if (MilImageLoader != M_NULL)
         { MimResize(MilImageLoader, MilChildSample, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BICUBIC + M_OVERSCAN_FAST); }

      // Draw an initial red rectangle around the buffer.
      MgraRect(M_DEFAULT, MilChildSample, 0, 1, IconSize - 1, IconSize - 2);

      // Free the allocated buffers.
      MbufFree(MilChildSample);
      }

   // Display the window with black color.
   MdispSelect(MilDisplay, MilDispImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
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
   MIL_ID MilImage, *pMilInputImage;
   MdigGetHookInfo(EventId, M_MODIFIED_BUFFER + M_BUFFER_ID, &MilImage);

   ClassStruct* data = static_cast<ClassStruct*>(DataPtr);
   MdispControl(data->MilDisplay, M_UPDATE, M_DISABLE);
   MobjInquire(MilImage, M_OBJECT_USER_DATA_PTR, (void**)&pMilInputImage);
   
   // Display the new target image.
   MbufCopy(MilImage, data->MilDispChild);

   // Perform product recognition using the classification module.
   MclassPredict(data->ClassCtx, *pMilInputImage, data->ClassRes, M_DEFAULT);

   MIL_INT Status {};
   MclassGetResult(data->ClassRes, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);
   ProcessStatus(Status);

   // Retrieve best classification score and class index.
   MIL_DOUBLE BestScore;
   MclassGetResult(data->ClassRes, M_GENERAL, M_BEST_CLASS_SCORE + M_TYPE_MIL_DOUBLE, &BestScore);

   MIL_INT BestIndex;
   MclassGetResult(data->ClassRes, M_GENERAL, M_BEST_CLASS_INDEX + M_TYPE_MIL_INT, &BestIndex);

   // Clear the overlay buffer.
   MdispControl(data->MilDisplay, M_OVERLAY_CLEAR, M_TRANSPARENT_COLOR);
   
   // Draw a green rectangle around the winning sample.
   MIL_INT IconSize = data->SourceSizeY / data->NbCategories;
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraRect(M_DEFAULT, data->MilOverlayImage, data->SourceSizeX, (BestIndex*IconSize)+1, data->SourceSizeX + IconSize - 1, (BestIndex + 1)*IconSize - 2);

   // Print the classification accuracy in the sample buffer.
   MIL_TEXT_CHAR Accuracy_text[256];
   MosSprintf(Accuracy_text, 256, MIL_TEXT("%.1lf%% score"), BestScore);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);
   MgraText(M_DEFAULT, data->MilOverlayImage, data->SourceSizeX+ 2, BestIndex*IconSize + 4, Accuracy_text);

   // Update the display.
   MdispControl(data->MilDisplay, M_UPDATE, M_ENABLE);

   // Wait for the user.
   if (data->NbOfFrames != M_INFINITE)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\r"));
      MosGetch();
      }

   return 0;
   }