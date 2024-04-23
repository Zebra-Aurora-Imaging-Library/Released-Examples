﻿//**********************************************************************************/
//
// File name: MpatFindMultipleModels.cpp 
//
// Synopsis:  This example demonstrates how to locate multiple models with the 
//            Pattern Matching module using two different search mode controls.
//
//            1 M_FIND_ALL_MODELS. This finds all occurrences for each model.
//
//            2 M_FIND_BEST_MODELS. This finds the best model for each occurrence.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**********************************************************************************/

#include <mil.h>
#include <vector>

// Model images and target image definitions.
#define NUM_OF_MODELS         4
#define MODEL_SIZE_X          126
#define MODEL_SIZE_Y          126

#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH MIL_TEXT("MpatFindMultipleModels/")

#define MODEL_IMAGE_NAME      MIL_TEXT("Model_%d.mim")
#define MODEL_IMAGE_FILE      EXAMPLE_IMAGE_PATH MODEL_IMAGE_NAME
#define TARGET_IMAGE_FILE     EXAMPLE_IMAGE_PATH MIL_TEXT("Buttons.mim")

// Define the acceptance for pattern matching.
#define ACCCEPTANCE           60

// Define the length of the model names.
#define MODEL_NAME_LENGTH     200

//**********************************************************************************/
// Example description.
//**********************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
   MIL_TEXT("MpatFindMultipleModels\n\n")

   MIL_TEXT("[SYNOPSIS]\n")
   MIL_TEXT("This example demonstrates how to locate multiple models with the\n")
   MIL_TEXT("Pattern Matching module using two different search mode controls:\n\n")
   MIL_TEXT("\t1 M_FIND_ALL_MODELS. This finds all occurrences for each\n")
   MIL_TEXT("\t  model.\n\n")
   MIL_TEXT("\t2 M_FIND_BEST_MODELS. This finds the best model for each\n")
   MIL_TEXT("\t  occurrence.\n\n")

   MIL_TEXT("[MODULES USED]\n")
   MIL_TEXT("Modules used: application, buffer, display, graphics,\n")
   MIL_TEXT("image processing, pattern matching, system.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


//**********************************************************************************/
// Main.
//**********************************************************************************/
int MosMain(void)
   {
   MIL_ID         MilApplication,     // Application identifier.
                  MilSystem,          // System identifier.
                  MilDisplay,         // Display identifier.
                  MilGraphicList,     // Graphic list identifier.
                  MilDispImage,       // Display image buffer identifier.
                  MilRightSubImage,   // Sub-display image buffer identifier for model image.
                  MilLeftSubImage,    // Sub-display image buffer identifier for target image.
                  MilTargetImage,     // Source image buffer identifier.
                  MilModelImage,      // Model Image buffer identifier array.
                  MilModelMaskImage,  // Image buffer identifier for processing.
                  MilPatContext,      // Pattern matching context identifier.
                  MilPatResult;       // Pattern matching result identifier.

   MIL_TEXT_CHAR  ModelImagesSource[NUM_OF_MODELS][MODEL_NAME_LENGTH]; // Model name array.
   MIL_INT        TotalNumFound, i;

   // Print header.
   PrintHeader();

   // Allocate objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   MbufRestore(TARGET_IMAGE_FILE, MilSystem, &MilTargetImage);

   // Retrieving target image size.
   MIL_ID TargetImageSizeX = MbufInquire(MilTargetImage, M_SIZE_X, M_NULL);
   MIL_ID TargetImageSizeY = MbufInquire(MilTargetImage, M_SIZE_Y, M_NULL);

   // Allocate a display buffer to display the target image and models.
   MbufAllocColor(MilSystem, 3, TargetImageSizeX + MODEL_SIZE_X, TargetImageSizeY, 
      8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilDispImage);
   MbufClear(MilDispImage, 0L);
   MdispSelect(MilDisplay, MilDispImage);

   // Allocate a graphic list to draw the sub-pixel annotations.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   // Associate the graphic list to the display for annotations.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Define the left and right part of the display buffer as two child buffers,
   // to display the target image and models side by side.
   MbufChild2d(MilDispImage, 0L, 0L, TargetImageSizeX, TargetImageSizeY, &MilLeftSubImage);
   MbufCopy(MilTargetImage, MilLeftSubImage);
   MbufChild2d(MilDispImage, TargetImageSizeX, 0L, MODEL_SIZE_X, 
      (TargetImageSizeY/NUM_OF_MODELS), &MilRightSubImage);

   // Allocate a buffer for the mask image.
   MbufAlloc2d(MilSystem, MODEL_SIZE_X, MODEL_SIZE_Y, 8+M_UNSIGNED, M_IMAGE+M_PROC, 
      &MilModelMaskImage);

   // Allocate pattern matching context and result buffers.
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &MilPatContext);
   MpatAllocResult(MilSystem, M_DEFAULT, &MilPatResult);

   // Add all four models in the context.
   for (i = 0; i < NUM_OF_MODELS; i++)
      {
      // Get the model image name.
      MosSprintf(ModelImagesSource[i], MODEL_NAME_LENGTH, MODEL_IMAGE_FILE, i);

      if (i==0)
         {
         // Restore the first model image.
         MbufRestore(ModelImagesSource[i], MilSystem, &MilModelImage);
         // Binarize the model image.
         MimBinarize(MilModelImage, MilModelMaskImage, M_BIMODAL+M_GREATER, M_NULL, M_NULL);
         // Fill up the holes in the model.
         MblobReconstruct(MilModelMaskImage, M_NULL, MilModelMaskImage, M_FILL_HOLES,
            M_FOREGROUND_ZERO);
         //Erode the binarized image for masking.
         MimErode(MilModelMaskImage, MilModelMaskImage, 4, M_BINARY);
         }
      else
         {
         // Load the second to the last model image.
         MbufLoad(ModelImagesSource[i], MilModelImage);
         }

      MpatDefine(MilPatContext, M_REGULAR_MODEL, MilModelImage, 0, 0, MODEL_SIZE_X, 
         MODEL_SIZE_Y, M_DEFAULT);

      // Set the acceptance and the separation distance.
      MpatControl(MilPatContext, i, M_ACCEPTANCE, ACCCEPTANCE);
      
      // Apply the don't care mask.
      MpatMask(MilPatContext, i, MilModelMaskImage, M_DONT_CARE, M_DEFAULT);

      // Move down the child image to draw the next model.
      MbufChildMove(MilRightSubImage, TargetImageSizeX, i*(TargetImageSizeY/NUM_OF_MODELS),
         MODEL_SIZE_X, MODEL_SIZE_Y, M_DEFAULT);

      // Draw the model image with the mask.
      MgraColor(M_DEFAULT,M_COLOR_RED);
      MpatDraw(M_DEFAULT, MilPatContext, MilRightSubImage, M_DRAW_IMAGE+M_DRAW_DONT_CARE, i, 
         M_DEFAULT);

      // Draw the model name without the path ".mim" extension.
      MgraColor(M_DEFAULT,M_COLOR_GREEN);
      MosSprintf(ModelImagesSource[i], MODEL_NAME_LENGTH, MODEL_IMAGE_NAME, i);
      ModelImagesSource[i][7]=0;
      MgraText(M_DEFAULT, MilRightSubImage, 0,
         MODEL_SIZE_Y- MgraInquire(M_DEFAULT, M_FONT_SIZE, M_NULL),
         MIL_CONST_TEXT_PTR(ModelImagesSource[i]));
      }

   MosPrintf(MIL_TEXT("A target image containing multiple occurrences of %i objects is\n"),
      NUM_OF_MODELS);
   MosPrintf(MIL_TEXT("displayed. %i models (one for each object) are also defined and\n"),
      NUM_OF_MODELS);
   MosPrintf(MIL_TEXT("displayed on the right. A don't care mask, drawn in red, is\napplied ")
      MIL_TEXT("to each model to limit the matching process to the\n")
      MIL_TEXT("object pixels only.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("\n1. M_FIND_ALL_MODELS\n")
      MIL_TEXT("--------------------\n\n"));

   // Pre-process the context then find all the occurrences for all models.
   MpatControl(MilPatContext, M_ALL, M_NUMBER, M_ALL);
   MpatPreprocess(MilPatContext, M_DEFAULT, MilTargetImage);
   MpatFind(MilPatContext, MilTargetImage, MilPatResult);

   // Get and print out the TotalNumFound of the occurrences found.
   MpatGetResult(MilPatResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &TotalNumFound);
   MosPrintf(MIL_TEXT("M_FIND_ALL_MODELS is used to find all occurrences for each\nmodel"));
   MosPrintf(MIL_TEXT(" in the target image. A total of %i occurrences are found.\n\n"), 
      TotalNumFound);

   if(TotalNumFound)
      {
      // Get the model index array for all occurrences.
      std::vector<MIL_INT> Indexes(TotalNumFound);
      MpatGetResult(MilPatResult, M_ALL, M_INDEX + M_TYPE_MIL_INT, &Indexes.front());

      // Draw the box and the position of the occurrences for each model.
      MIL_INT ModelIndex, OccurrenceIndex=0, NumFoundPerModel;
      for(ModelIndex = 0; ModelIndex< NUM_OF_MODELS && OccurrenceIndex < TotalNumFound;
         ModelIndex++)
         {
         NumFoundPerModel=0;
         for(; OccurrenceIndex< TotalNumFound; OccurrenceIndex++)
            {
            if(Indexes[OccurrenceIndex] == ModelIndex)
               {
               MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX+M_DRAW_POSITION,
                  OccurrenceIndex, M_DEFAULT);
               NumFoundPerModel++;
               }
            else
               {
               break;
               }
            }
         MosPrintf(MIL_TEXT("For Model_%i, %i occurrences are found and displayed.\n\n"), 
            ModelIndex, NumFoundPerModel);
         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();
         MgraClear(M_DEFAULT,MilGraphicList);
         }
      }

   MosPrintf(MIL_TEXT("\n2. M_FIND_BEST_MODELS\n")
      MIL_TEXT("---------------------\n\n"));

   // Find the best model for each occurrence in the target image.
   MpatControl(MilPatContext, M_CONTEXT, M_SEARCH_MODE, M_FIND_BEST_MODELS);
   MpatPreprocess(MilPatContext, M_DEFAULT, MilTargetImage);
   MpatFind(MilPatContext, MilTargetImage, MilPatResult);

   // Get and print out the TotalNumFound of the occurrences found.
   MpatGetResult(MilPatResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &TotalNumFound);
   MosPrintf(MIL_TEXT("M_FIND_BEST_MODELS is used to find the best model for each\n"));
   MosPrintf(MIL_TEXT("occurrence in the target image. A total of %i occurrences\n"),
      TotalNumFound);
   MosPrintf(MIL_TEXT("are found.\n\n"));

   if(TotalNumFound)
      {
      // Get the model index array for all occurrences.
      std::vector<MIL_INT> Indexes(TotalNumFound);
      MpatGetResult(MilPatResult, M_ALL, M_INDEX + M_TYPE_MIL_INT, &Indexes.front());

      // Draw the box and the model index for each occurrence.
      MIL_TEXT_CHAR ModelIndexChar[2] = {0};
      MIL_DOUBLE posX, posY;
      for(i = 0; i < TotalNumFound; i++)
         {
         MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX+M_DRAW_POSITION, i,
            M_DEFAULT);
         MpatGetResult(MilPatResult, i, M_POSITION_X, &posX);
         MpatGetResult(MilPatResult, i, M_POSITION_Y, &posY);
         MosSprintf(ModelIndexChar, 2, MIL_TEXT("%d"), Indexes[i]);
         MgraText(M_DEFAULT, MilLeftSubImage, posX, posY, MIL_CONST_TEXT_PTR(ModelIndexChar));
         }
      }

   MosPrintf(MIL_TEXT("The %i found occurrences are displayed and the model index\n"),
      TotalNumFound);
   MosPrintf(MIL_TEXT("for each occurrence, is drawn.\n"));

   MosPrintf(MIL_TEXT("\nPress <Enter> to finish.\n"));
   MosGetch();

   // Free allocations.
   MbufFree(MilModelImage);
   MpatFree(MilPatResult);
   MpatFree(MilPatContext);
   MbufFree(MilModelMaskImage);
   MbufFree(MilRightSubImage);
   MbufFree(MilLeftSubImage);
   MgraFree(MilGraphicList);
   MbufFree(MilDispImage);
   MbufFree(MilTargetImage);
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

