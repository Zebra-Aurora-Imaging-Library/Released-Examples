#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: Magm.py 
#
# Synopsis:  This program consists of 2 examples that use the AGM module
#            to define a model and search for model occurrences in target images.
#            The first example extracts a single-definition model from a source image,
#            then quickly finds occurrences in a cluttered target image.
#            The second example constructs a composite-definition model through training,
#            then finds occurrences with slight variations in appearance in different target images.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved
 
import mil as MIL

# Target MIL image file specifications.
IMAGE_FILE = MIL.M_IMAGE_PATH + "Cell.mbufi"
IMAGE_CUP  = MIL.M_IMAGE_PATH + "PlasticCup.mim"
IMAGE_SMALL_PARTICLE_RADIUS = 1

# Path definitions.
EXAMPLE_IMAGE_DIR_PATH = MIL.M_IMAGE_PATH + "/Magm/"
MODEL_IMAGE_PATH       = EXAMPLE_IMAGE_DIR_PATH + "CircuitPinsModel.mim"
TARGET_IMAGE_PATH      = EXAMPLE_IMAGE_DIR_PATH + "CircuitBoardTarget.mim"
TRAIN_IMAGES_PATH      = EXAMPLE_IMAGE_DIR_PATH + "LabeledTrainImages.mbufc"
TEST_IMAGES_DIR_PATH   = EXAMPLE_IMAGE_DIR_PATH + "Testset/"

# Single-definition model example.
def SingleModelExample(MilApplication, MilSystem, MilDisplay):
   print("This example shows that AGM is able to quickly find occurrences")
   print("in a large cluttered target image.")
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Allocate a graphic list to hold the subpixel annotations to draw.
   MilGraphicList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)

   # Associate the graphic list to the display for annotations.
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList)

   # Restore the model image and display it.
   MilModelImage = MIL.MbufRestore(MODEL_IMAGE_PATH, MilSystem)
   
   # Make the display a little bigger since the image is small.
   WindowSizeX = MIL.MbufInquire(MilModelImage, MIL.M_SIZE_X, MIL.M_NULL) * 6;
   WindowSizeY = MIL.MbufInquire(MilModelImage, MIL.M_SIZE_Y, MIL.M_NULL) * 2;

   MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_X, WindowSizeX);
   MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_Y, WindowSizeY);

   # Display the model image.   
   MIL.MdispSelect(MilDisplay, MilModelImage)
   
   # Put the display back to its default state.
   MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_X, MIL.M_DEFAULT);
   MIL.MdispControl(MilDisplay, MIL.M_WINDOW_INITIAL_SIZE_Y, MIL.M_DEFAULT);

   # Allocate a find AGM context.
   MilFindContext = MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND, MIL.M_DEFAULT)

   # Allocate a find AGM result buffer.
   MilSearchResult = MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND_RESULT, MIL.M_DEFAULT)

   # Define the single-definition model.
   MIL.MagmDefine(MilFindContext, MIL.M_ADD, MIL.M_DEFAULT, MIL.M_SINGLE, MilModelImage, MIL.M_DEFAULT)

   # Pause to show the model.
   print("A single-definition model was defined from the displayed image.")
   print("Press <Enter> to continue.\n")

   MIL.MosGetch()

   # Set the minimum acceptable detection score.
   MIL.MagmControl(MilFindContext, MIL.M_AGM_MODEL_INDEX(0), MIL.M_ACCEPTANCE_DETECTION, 90)

   # Preprocess the find AGM context.
   MIL.MagmPreprocess(MilFindContext, MIL.M_DEFAULT)

   # Restore the target image.
   MilTargetImage = MIL.MbufRestore(TARGET_IMAGE_PATH, MilSystem)

   # Reset the time.
   MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_RESET + MIL.M_SYNCHRONOUS, MIL.M_NULL)

   # Find the model.
   MIL.MagmFind(MilFindContext, MilTargetImage, MilSearchResult, MIL.M_DEFAULT)

   # Read the find time.
   FindTime = MIL.MappTimer(MIL.M_DEFAULT, MIL.M_TIMER_READ + MIL.M_SYNCHRONOUS)

   # Get the number of occurrences found.
   NumOccurrences = MIL.MagmGetResult(MilSearchResult, MIL.M_DEFAULT, MIL.M_NUMBER)
   if NumOccurrences > 0:
      XPositions      = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_X)
      YPositions      = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_Y)
      DetectionScores = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_DETECTION)
      FitScores       = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_FIT)
      CoverageScores  = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_COVERAGE)

      # Print the results for each occurrence found.
      print("The model was found in the target image:\n")
      print("Result   X Position   Y Position   DetectionScore   FitScore   CoverageScore\n")
      for n in range(NumOccurrences):
         print("{:<9}{:<13.2f}{:<13.2f}{:<17.2f}{:<11.2f}{:<11.2f}"
               .format(int(n), XPositions[n], YPositions[n], DetectionScores[n], FitScores[n], CoverageScores[n]))
         
      print("\nNumber of occurrences found in target image: {}".format(int(NumOccurrences)))
      print("Search time: {:.1f} ms".format(FindTime * 1000.0))

      # Draw green edges and bounding boxes over the occurrences that were found.
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)
      MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_EDGES + MIL.M_DRAW_BOX, MIL.M_ALL, MIL.M_DEFAULT)

      # Draw red positions over the occurrences that were found.
      MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_RED)
      MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_POSITION, MIL.M_ALL, MIL.M_DEFAULT)

   else :
      print("The model was not found in target image")

   # Display the target image.
   MIL.MdispSelect(MilDisplay, MilTargetImage)
   print("Press <Enter> to continue.\n")
   MIL.MosGetch()

   # Remove the display.
   MIL.MdispSelect(MilDisplay, MIL.M_NULL);

   # Free MIL objects.
   MIL.MgraFree(MilGraphicList)
   MIL.MagmFree(MilFindContext)
   MIL.MagmFree(MilSearchResult)
   MIL.MbufFree(MilModelImage)
   MIL.MbufFree(MilTargetImage)

# Composite-definition model example.
def CompositeModelExample(MilApplication, MilSystem, MilDisplay):
   print("This example shows that AGM is able to confidently find occurrences with appearance")
   print("variation in a complex background after training a composite-definition model.")
   print("Press <Enter> to continue.")
   MIL.MosGetch()

   # Restore the training images.
   TrainImagesContainer = MIL.MbufRestore(TRAIN_IMAGES_PATH, MilSystem)

   # Print message about training image labels.
   print("\n*******************************************************")
   print("LOADING LABELED TRAINING IMAGES...")
   print("*******************************************************")

   print("Training requires labeled images with positive and negative samples.")
   print("Positive samples are occurrences delimited by blue boxes and")
   print("negative samples are background parts delimited by red boxes.")
   print("Typically, when false positives are detected in training images,")
   print("they should be used as negative samples to improve the training.")
   print("To ease the labeling of images, use the example AgmLabelingTool.")

   # Wait for a key to be pressed.
   print("\nPress <Enter> to show the labeled images used in this training.")
   MIL.MosGetch()

   # Get the components from the container.
   NumTrainImage = MIL.MbufInquireContainer(TrainImagesContainer, MIL.M_CONTAINER, MIL.M_COMPONENT_LIST + MIL.M_NB_ELEMENTS)
   TrainImages = MIL.MbufInquireContainer(TrainImagesContainer, MIL.M_CONTAINER, MIL.M_COMPONENT_LIST)

   # Allocate a graphic list to hold the subpixel annotations to draw.
   Regions = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)

   # Associate the graphic list to the display for annotations.
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, Regions)

   # Display each labeled training image.
   for n in range(NumTrainImage):
      MIL.MgraClear(MIL.M_DEFAULT, Regions)
      MIL.MbufSetRegion(TrainImages[n], Regions, MIL.M_DEFAULT, MIL.M_EXTRACT, MIL.M_DEFAULT)
      MIL.MdispSelect(MilDisplay, TrainImages[n])
      print("Training image {}/{}".format(int(n+1), int(NumTrainImage)))
      print("Press <Enter> to continue.")
      MIL.MosGetch()

   # Disassociate the graphic list from the display and stop displaying the training images.
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MIL.M_NULL)
   MIL.MdispSelect(MilDisplay, MIL.M_NULL)

   # Allocate a find AGM context.
   MilFindContext = MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND, MIL.M_DEFAULT)

   # Allocate a find AGM result buffer.
   MilSearchResult = MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_FIND_RESULT, MIL.M_DEFAULT)

   # Allocate a train AGM context.
   MilTrainContext = MIL.MagmAlloc(MilSystem, MIL.M_GLOBAL_EDGE_BASED_TRAIN, MIL.M_DEFAULT)

   # Allocate a train AGM result buffer.
   MilTrainResult = MIL.MagmAllocResult(MilSystem, MIL.M_GLOBAL_EDGE_BASED_TRAIN_RESULT, MIL.M_DEFAULT)

   # Define the composite-definition model.
   MIL.MagmDefine(MilTrainContext, MIL.M_ADD, MIL.M_DEFAULT, MIL.M_COMPOSITE, MIL.M_NULL, MIL.M_DEFAULT)

   # Preprocess the train AGM context.
   MIL.MagmPreprocess(MilTrainContext, MIL.M_DEFAULT)

   # Train the composite-definition model.
   print("\n*******************************************************")
   print("TRAINING... THIS WILL TAKE SOME TIME...")
   print("*******************************************************")

   TrainImagesContainerPtr = [TrainImagesContainer]

   MIL.MagmTrain(MilTrainContext, TrainImagesContainerPtr, 1, MilTrainResult, MIL.M_DEFAULT)

   # Check that the training process completed successfully.
   TrainStatus = MIL.MagmGetResult(MilTrainResult, MIL.M_DEFAULT, MIL.M_STATUS)
   if TrainStatus == MIL.M_COMPLETE:
      print("Training complete!")
      # Ensure that the trained model is valid before copying to the find AGM context.
      TrainedModelStatus = MIL.MagmGetResult(MilTrainResult, MIL.M_AGM_MODEL_INDEX(0), MIL.M_STATUS)
      if TrainedModelStatus == MIL.M_STATUS_TRAIN_OK : 
         MIL.MagmCopyResult(MilTrainResult, MIL.M_DEFAULT, MilFindContext, MIL.M_DEFAULT, MIL.M_TRAINED_MODEL, MIL.M_DEFAULT)

   # Preprocess find AGM context.
   MIL.MagmPreprocess(MilFindContext, MIL.M_DEFAULT)

   print("\n*******************************************************")
   print("FINDING WITH THE TRAINED MODEL...")
   print("*******************************************************")

   # Restore test images.
   FilesToSearch = TEST_IMAGES_DIR_PATH + "*.mim"
   NumberOfImages = MIL.MappFileOperation(MIL.M_DEFAULT, FilesToSearch, MIL.M_NULL, MIL.M_NULL, MIL.M_FILE_NAME_FIND_COUNT, MIL.M_DEFAULT)
   TestImages = []
   for n in range(NumberOfImages):
      Filename = MIL.MappFileOperation(MIL.M_DEFAULT, FilesToSearch, MIL.M_NULL, MIL.M_NULL, MIL.M_FILE_NAME_FIND, n)
      FilePath = TEST_IMAGES_DIR_PATH + Filename
      TestImages.append(MIL.MbufRestore(FilePath, MilSystem))
      
   # Wait for a key to be pressed.
   print("\nPress <Enter> to search for the trained model in different test images.\n")
   MIL.MosGetch()

   # Allocate a graphic list to hold the subpixel annotations to draw.
   MilGraphicList = MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT)

   # Associate the graphic list to the display for annotations.
   MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList)

   # Assign the color to draw.
   MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN)

   for n in range(NumberOfImages):
      # Find the model in the test image.
      MIL.MagmFind(MilFindContext, TestImages[n], MilSearchResult, MIL.M_DEFAULT)

      # Get the number of occurrences found.
      NumOccurrences = MIL.MagmGetResult(MilSearchResult, MIL.M_DEFAULT, MIL.M_NUMBER + MIL.M_TYPE_MIL_INT)

      if NumOccurrences > 0:
         # Get the results of the search.
         XPositions = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_X)
         YPositions = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_POSITION_Y)
         DetectionScores = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_DETECTION)
         FitScores = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_FIT)
         CoverageScores = MIL.MagmGetResult(MilSearchResult, MIL.M_ALL, MIL.M_SCORE_COVERAGE)

         # Print the results for each occurrence foud.
         print("The model was found in the target image:\n")
         print("Result   X Position   Y Position   DetectionScore   FitScore   CoverageScore\n")
         for k in range(NumOccurrences):
            print("{:<9}{:<13.2f}{:<13.2f}{:<17.2f}{:<11.2f}{:<11.2f}"
               .format(int(k), XPositions[k], YPositions[k], DetectionScores[k], FitScores[k], CoverageScores[k]))

         # Empty the graphic list.
         MIL.MgraClear(MIL.M_DEFAULT, MilGraphicList)

         # Draw bounding box
         MIL.MagmDraw(MIL.M_DEFAULT, MilSearchResult, MilGraphicList, MIL.M_DRAW_BOX, MIL.M_ALL, MIL.M_DEFAULT)
         
      else:
         print("The model was not found in target image.\n")
         
      # Display the test image.
      MIL.MdispSelect(MilDisplay, TestImages[n])

      # Wait for a key to be pressed.
      print("Press <Enter> to continue.\n")
      MIL.MosGetch()

   # Remove the display.
   MIL.MdispSelect(MilDisplay, MIL.M_NULL);

   # Free MIL objects.
   for k in range(NumberOfImages):
      MIL.MbufFree(TestImages[k])
      
   MIL.MgraFree(MilGraphicList)
   MIL.MgraFree(Regions)
   MIL.MagmFree(MilTrainContext)
   MIL.MagmFree(MilTrainResult)
   MIL.MagmFree(MilFindContext)
   MIL.MagmFree(MilSearchResult)
   MIL.MbufFree(TrainImagesContainer)

def MagmExample():
   # Allocate a default MIL application, system, display and image.
   MilApplication, MilSystem, MilDisplay = MIL.MappAllocDefault(MIL.M_DEFAULT, DigIdPtr=MIL.M_NULL, ImageBufIdPtr=MIL.M_NULL)

   # Print the example synopsis.
   print("[EXAMPLE NAME]")
   print("Magm\n")
   print("[SYNOPSIS]")
   print("This program shows the use of the AGM module.")
   print("[MODULES USED]")
   print("Advanced Geometric Matcher, Buffer, Display, Graphics.\n")

   # Run single-definition model example.
   SingleModelExample(MilApplication, MilSystem, MilDisplay)

   # Run composite-definition model example.
   CompositeModelExample(MilApplication, MilSystem, MilDisplay)

   # Wait for a key to be pressed.
   print("Press <Enter> to end.")
   MIL.MosGetch()
   
   # Free defaults.   
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL)
   
   return 0


if __name__ == "__main__":
   MagmExample()
