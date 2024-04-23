//*************************************************************************************
//
// File name: Magm.cpp
//
// Synopsis: This program consists of 2 examples that use the AGM module
// to define a model and search for model occurrences in target images.
// The first example extracts a single-definition model from a source image,
// then quickly finds occurrences in a cluttered target image.
// The second example constructs a composite-definition model through training,
// then finds occurrences with slight variations in appearance in different target images.
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

// Path definitions.
#define EXAMPLE_IMAGE_DIR_PATH     M_IMAGE_PATH MIL_TEXT("/Magm/")
#define MODEL_IMAGE_PATH           EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("CircuitPinsModel.mim")
#define TARGET_IMAGE_PATH          EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("CircuitBoardTarget.mim")
#define TRAIN_IMAGES_PATH          EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("LabeledTrainImages.mbufc")
#define TEST_IMAGES_DIR_PATH       EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("Testset/")

// Function declarations.
void SingleModelExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void CompositeModelExample(MIL_ID MilSystem, MIL_ID MilDisplay);


//****************************************************************************
// Main.
//****************************************************************************
int MosMain(void)
   {
   MIL_ID MilApplication,    // Application identifier.
          MilSystem,         // System identifier.
          MilDisplay;        // Display identifier.

   // Allocate defaults.
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   // Print the example synopsis.
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Magm\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program shows the use of the AGM module.\n"));
   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Advanced Geometric Matcher, Buffer, Display, Graphics.\n\n"));

   // Run single-definition model example.
   SingleModelExample(MilSystem, MilDisplay);

   // Run composite-definition model example.
   CompositeModelExample(MilSystem, MilDisplay);

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();
   
   // Free defaults.
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

//****************************************************************************
// Single-definition model example.
//****************************************************************************
void SingleModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("This example shows that AGM is able "));
   MosPrintf(MIL_TEXT("to quickly find occurrences\n"));
   MosPrintf(MIL_TEXT("in a large cluttered target image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_ID MilGraphicList;    // Graphic list identifier.
   MIL_ID MilFindContext;    // Find AGM context identifier.
   MIL_ID MilSearchResult;   // Find AGM result buffer identifier.
   MIL_ID MilModelImage;     // Image buffer identifier.
   MIL_ID MilTargetImage;    // Image buffer identifier.

   // Allocate a graphic list to hold the subpixel annotations to draw.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Restore the model image.
   MbufRestore(MODEL_IMAGE_PATH, MilSystem, &MilModelImage);

   // Make the display a little bigger since the image is small.
   MIL_INT WindowSizeX = MbufInquire(MilModelImage, M_SIZE_X, M_NULL) * 6;
   MIL_INT WindowSizeY = MbufInquire(MilModelImage, M_SIZE_Y, M_NULL) * 2;

   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_X, WindowSizeX);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_Y, WindowSizeY);

   // Display the model image.
   MdispSelect(MilDisplay, MilModelImage);

   // Put the display back to its default state.
   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_X, M_DEFAULT);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_Y, M_DEFAULT);

   // Allocate a find AGM context.
   MagmAlloc(MilSystem, M_GLOBAL_EDGE_BASED_FIND, M_DEFAULT, &MilFindContext);

   // Allocate a find AGM result buffer.
   MagmAllocResult(MilSystem, M_GLOBAL_EDGE_BASED_FIND_RESULT, M_DEFAULT, &MilSearchResult);

   // Define the single-definition model.
   MagmDefine(MilFindContext, M_ADD, M_DEFAULT, M_SINGLE, MilModelImage, M_DEFAULT);

   // Pause to show the model.
   MosPrintf(MIL_TEXT("A single-definition model was defined "));
   MosPrintf(MIL_TEXT("from the displayed image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Set the minimum acceptable detection score.
   MagmControl(MilFindContext, M_AGM_MODEL_INDEX(0), M_ACCEPTANCE_DETECTION, 90);

   // Preprocess the find AGM context.
   MagmPreprocess(MilFindContext, M_DEFAULT);

   // Restore the target image.
   MbufRestore(TARGET_IMAGE_PATH, MilSystem, &MilTargetImage);

   // Reset the time.
   MIL_DOUBLE FindTime = 0.0;
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   // Find the model.
   MagmFind(MilFindContext, MilTargetImage, MilSearchResult, M_DEFAULT);

   // Read the find time.
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &FindTime);

   // Get the number of occurrences found.
   MIL_INT NumOccurrences = 0;
   MagmGetResult(MilSearchResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOccurrences);
   if(NumOccurrences > 0)
      {
      std::vector<MIL_DOUBLE> XPositions;
      std::vector<MIL_DOUBLE> YPositions;
      std::vector<MIL_DOUBLE> DetectionScores;
      std::vector<MIL_DOUBLE> FitScores;
      std::vector<MIL_DOUBLE> CoverageScores;
      MagmGetResult(MilSearchResult, M_ALL, M_POSITION_X, XPositions);
      MagmGetResult(MilSearchResult, M_ALL, M_POSITION_Y, YPositions);
      MagmGetResult(MilSearchResult, M_ALL, M_SCORE_DETECTION, DetectionScores);
      MagmGetResult(MilSearchResult, M_ALL, M_SCORE_FIT, FitScores);
      MagmGetResult(MilSearchResult, M_ALL, M_SCORE_COVERAGE, CoverageScores);

      // Print the results for each occurrence found.
      MosPrintf(MIL_TEXT("The model was found in the target image:\n\n"));
      MosPrintf(MIL_TEXT("Result   X Position   Y Position   ")
                MIL_TEXT("DetectionScore   FitScore   CoverageScore\n\n"));
      for(MIL_INT i = 0; i < NumOccurrences; ++i)
         {
         MosPrintf(MIL_TEXT("%-9i%-13.2f%-13.2f%-17.2f%-11.2f%-11.2f\n"),
                   i, XPositions[i], YPositions[i],
                   DetectionScores[i], FitScores[i], CoverageScores[i]);
         }
      MosPrintf(MIL_TEXT("\nNumber of occurrences found in target image: %i\n"), NumOccurrences);
      MosPrintf(MIL_TEXT("Search time: %.1f ms\n"), FindTime * 1000.0);

      // Draw green edges and bounding boxes over the occurrences that were found.
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MagmDraw(M_DEFAULT, MilSearchResult, MilGraphicList,
               M_DRAW_EDGES + M_DRAW_BOX, M_ALL, M_DEFAULT);

      // Draw red positions over the occurrences that were found.
      MgraColor(M_DEFAULT, M_COLOR_RED);
      MagmDraw(M_DEFAULT, MilSearchResult, MilGraphicList,
               M_DRAW_POSITION, M_ALL, M_DEFAULT);
      }
   else
      {
      MosPrintf(MIL_TEXT("The model was not found in target image.\n"));
      }

   // Display the target image.
   MdispSelect(MilDisplay, MilTargetImage);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Remove the display.
   MdispSelect(MilDisplay, M_NULL);

   // Free MIL objects.
   MgraFree(MilGraphicList);
   MagmFree(MilFindContext);
   MagmFree(MilSearchResult);
   MbufFree(MilModelImage);
   MbufFree(MilTargetImage);
   }

//****************************************************************************
// Composite-definition model example.
//****************************************************************************
void CompositeModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("This example shows that AGM is able "));
   MosPrintf(MIL_TEXT("to confidently find occurrences with appearance\n"));
   MosPrintf(MIL_TEXT("variation in a complex background "));
   MosPrintf(MIL_TEXT("after training a composite-definition model.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   MIL_ID MilGraphicList;       // Graphic list identifier.
   MIL_ID MilTrainContext;      // Train AGM context identifier.
   MIL_ID MilTrainResult;       // Train AGM result buffer identifier.
   MIL_ID MilFindContext;       // Find context identifier.
   MIL_ID MilSearchResult;      // Find AGM result buffer identifier.
   MIL_ID Regions;              // Graphic list identifier.
   MIL_ID TrainImagesContainer; // Container buffer identifier.

   // Restore the training images.
   MbufRestore(TRAIN_IMAGES_PATH, MilSystem, &TrainImagesContainer);

   // Print message about training image labels.
   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("LOADING LABELED TRAINING IMAGES...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   MosPrintf(MIL_TEXT("Training requires labeled images with positive and negative samples.\n"));
   MosPrintf(MIL_TEXT("Positive samples are occurrences delimited by blue boxes and\n"));
   MosPrintf(MIL_TEXT("negative samples are background parts delimited by red boxes.\n"));
   MosPrintf(MIL_TEXT("Typically, when false positives are detected in training images,\n"));
   MosPrintf(MIL_TEXT("they should be used as negative samples to improve the training.\n"));
   MosPrintf(MIL_TEXT("To ease the labeling of images, use the example AgmLabelingTool.\n"));

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("\nPress <Enter> to show the labeled images used in this training.\n"));
   MosGetch();

   // Get the components from the container.
   std::vector<MIL_ID> TrainImages;
   MbufInquireContainer(TrainImagesContainer, M_CONTAINER, M_COMPONENT_LIST, TrainImages);

   // Allocate a graphic list to hold the subpixel annotations to draw.
   MgraAllocList(MilSystem, M_DEFAULT, &Regions);

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, Regions);

   // Display each labeled training image.
   MIL_INT NumTrainImage = (MIL_INT)TrainImages.size();
   for(MIL_INT i = 0; i < NumTrainImage; ++i)
      {
      MgraClear(M_DEFAULT, Regions);
      MbufSetRegion(TrainImages[i], Regions, M_DEFAULT, M_EXTRACT, M_DEFAULT);
      MdispSelect(MilDisplay, TrainImages[i]);
      MosPrintf(MIL_TEXT("Training image %i/%i\n"), i+1, NumTrainImage);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
      MosGetch();
      }

   // Disassociate the graphic list from the display and stop displaying the training images.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   MdispSelect(MilDisplay, M_NULL);

   // Allocate a find AGM context.
   MagmAlloc(MilSystem, M_GLOBAL_EDGE_BASED_FIND, M_DEFAULT, &MilFindContext);

   // Allocate a find AGM result buffer.
   MagmAllocResult(MilSystem, M_GLOBAL_EDGE_BASED_FIND_RESULT, M_DEFAULT, &MilSearchResult);

   // Allocate a train AGM context.
   MagmAlloc(MilSystem, M_GLOBAL_EDGE_BASED_TRAIN, M_DEFAULT, &MilTrainContext);

   // Allocate a train AGM result buffer.
   MagmAllocResult(MilSystem, M_GLOBAL_EDGE_BASED_TRAIN_RESULT, M_DEFAULT, &MilTrainResult);

   // Define the composite-definition model.
   MagmDefine(MilTrainContext, M_ADD, M_DEFAULT, M_COMPOSITE, M_NULL, M_DEFAULT);

   // Preprocess the train AGM context.
   MagmPreprocess(MilTrainContext, M_DEFAULT);

   // Train the composite-definition model.
   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("TRAINING... THIS WILL TAKE SOME TIME...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));
   MagmTrain(MilTrainContext, &TrainImagesContainer, 1, MilTrainResult, M_DEFAULT);

   // Check that the training process completed successfully.
   MIL_INT TrainStatus = -1;
   MagmGetResult(MilTrainResult, M_DEFAULT, M_STATUS, &TrainStatus);
   if(TrainStatus == M_COMPLETE)
   {
   MosPrintf(MIL_TEXT("Training complete!\n"));

   // Ensure that the trained model is valid before copying to the find AGM context.
   MIL_INT TrainedModelStatus = -1;
   MagmGetResult(MilTrainResult, M_AGM_MODEL_INDEX(0), M_STATUS, &TrainedModelStatus);
   if(TrainedModelStatus == M_STATUS_TRAIN_OK)
      {
      MagmCopyResult(MilTrainResult, M_DEFAULT, MilFindContext,
                     M_DEFAULT, M_TRAINED_MODEL, M_DEFAULT);
      }
   }

   // Preprocess find AGM context.
   MagmPreprocess(MilFindContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("FINDING WITH THE TRAINED MODEL...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   // Restore test images.
   MIL_INT NumberOfImages = 0;
   MIL_STRING FilesToSearch = TEST_IMAGES_DIR_PATH;
   FilesToSearch += MIL_TEXT("*.mim");
   MappFileOperation(M_DEFAULT, FilesToSearch, M_NULL, M_NULL,
                     M_FILE_NAME_FIND_COUNT, M_DEFAULT, &NumberOfImages);
   std::vector<MIL_ID> TestImages(NumberOfImages);
   for(MIL_INT i = 0; i < NumberOfImages; i++)
      {
      MIL_STRING Filename;
      MappFileOperation(M_DEFAULT, FilesToSearch, M_NULL, M_NULL,
                        M_FILE_NAME_FIND, i, Filename);
      MIL_STRING FilePath = TEST_IMAGES_DIR_PATH + Filename;
      MbufRestore(FilePath, MilSystem, &TestImages[i]);
      }

   // Wait for a key to be pressed.
   MosPrintf(MIL_TEXT("\nPress <Enter> to search for the trained model in different test images.\n\n"));
   MosGetch();

   // Allocate a graphic list to hold the subpixel annotations to draw.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Assign the color to draw.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   for(MIL_INT i = 0; i < NumberOfImages; ++i)
      {
      // Find the model in the test image.
      MagmFind(MilFindContext, TestImages[i], MilSearchResult, M_DEFAULT);

      // Get the number of occurrences found.
      MIL_INT NumOccurrences = 0;
      MagmGetResult(MilSearchResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumOccurrences);

      if(NumOccurrences > 0)
         {
         // Get the results of the search.
         std::vector<MIL_DOUBLE> XPositions;
         std::vector<MIL_DOUBLE> YPositions;
         std::vector<MIL_DOUBLE> DetectionScores;
         std::vector<MIL_DOUBLE> FitScores;
         std::vector<MIL_DOUBLE> CoverageScores;
         MagmGetResult(MilSearchResult, M_ALL, M_POSITION_X, XPositions);
         MagmGetResult(MilSearchResult, M_ALL, M_POSITION_Y, YPositions);
         MagmGetResult(MilSearchResult, M_ALL, M_SCORE_DETECTION, DetectionScores);
         MagmGetResult(MilSearchResult, M_ALL, M_SCORE_FIT, FitScores);
         MagmGetResult(MilSearchResult, M_ALL, M_SCORE_COVERAGE, CoverageScores);

         // Print the results for each occurrence foud.
         MosPrintf(MIL_TEXT("The model was found in the target image:\n\n"));
         MosPrintf(MIL_TEXT("Result   X Position   Y Position   ")
                   MIL_TEXT("DetectionScore   FitScore   CoverageScore\n\n"));
         for(MIL_INT j = 0; j < NumOccurrences; ++j)
            {
            MosPrintf(MIL_TEXT("%-9i%-13.2f%-13.2f%-17.2f%-11.2f%-11.2f\n"),
                      j, XPositions[j], YPositions[j],
                      DetectionScores[j], FitScores[j], CoverageScores[j]);
            }

         // Empty the graphic list.
         MgraClear(M_DEFAULT, MilGraphicList);

         // Draw bounding box
         MagmDraw(M_DEFAULT, MilSearchResult, MilGraphicList, M_DRAW_BOX, M_ALL, M_DEFAULT);
         }
      else
         {
         MosPrintf(MIL_TEXT("The model was not found in target image.\n"));
         }

      // Display the test image.
      MdispSelect(MilDisplay, TestImages[i]);

      // Wait for a key to be pressed.
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   // Remove the display.
   MdispSelect(MilDisplay, M_NULL);

   // Free MIL objects.
   for(MIL_INT i = 0; i < NumberOfImages; ++i)
      {
      MbufFree(TestImages[i]);
      }
   MgraFree(MilGraphicList);
   MgraFree(Regions);
   MagmFree(MilTrainContext);
   MagmFree(MilTrainResult);
   MagmFree(MilFindContext);
   MagmFree(MilSearchResult);
   MbufFree(TrainImagesContainer);
   }
