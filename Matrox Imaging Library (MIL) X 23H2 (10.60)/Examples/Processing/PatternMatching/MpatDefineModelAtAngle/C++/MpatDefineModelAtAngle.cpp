//**********************************************************************************/
//
// File name: MpatDefineModelAtAngle.cpp 
//
// Synopsis:  This example demonstrates how to define a model at an angle using the
//            possibility to add models with an associated region to a context.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**********************************************************************************/

#include <mil.h>
#include <vector>

// Model image and target image definitions.
#define MODEL_IMAGE_SIZE_X          239
#define MODEL_IMAGE_SIZE_Y          241
#define MODEL_POSITION_X            22
#define MODEL_POSITION_Y            22
#define MODEL_SIZE_X                199
#define MODEL_SIZE_Y                152
#define MODEL_RECT_POSITION_X       7
#define MODEL_RECT_POSITION_Y       91
#define MODEL_RECT_SIZE_X           98
#define MODEL_RECT_SIZE_Y           209
#define MODEL_RECT_ANGLE            62.5

#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH MIL_TEXT("MpatDefineModelAtAngle/")

#define MODEL_IMAGE_NAME      MIL_TEXT("Model.mim")
#define MODEL_IMAGE_FILE      EXAMPLE_IMAGE_PATH MODEL_IMAGE_NAME
#define TARGET_IMAGE_FILE     EXAMPLE_IMAGE_PATH MIL_TEXT("Hook.mim")

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
   MIL_TEXT("MpatDefineModelAtAngle\n\n")

   MIL_TEXT("[SYNOPSIS]\n")
   MIL_TEXT("This example demonstrates how to define a model at angle using the\n")
   MIL_TEXT("possibility to add model with an associated region to a context.\n\n")

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
                  MilPatContext,      // Pattern matching context identifier.
                  MilPatResult;       // Pattern matching result identifier.

   MIL_TEXT_CHAR  ModelImageSource[MODEL_NAME_LENGTH]; // Model name array.
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
   MbufAllocColor(MilSystem, 3, TargetImageSizeX + MODEL_IMAGE_SIZE_X, TargetImageSizeY,
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
   MbufChild2d(MilDispImage, TargetImageSizeX, 0L, MODEL_IMAGE_SIZE_X, TargetImageSizeY, &MilRightSubImage);

   // Allocate the pattern matching context and result buffers.
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &MilPatContext);
   MpatAllocResult(MilSystem, M_DEFAULT, &MilPatResult);
      
   // Get the model image name.
   MosSprintf(ModelImageSource, MODEL_NAME_LENGTH, MODEL_IMAGE_FILE);

   // Restore and draw the model image.
   MbufRestore(ModelImageSource, MilSystem, &MilModelImage);
   MbufCopy(MilModelImage, MilRightSubImage);

   // Add the model to the context.
   MpatDefine(MilPatContext, M_REGULAR_MODEL, MilModelImage,
              MODEL_POSITION_X,
              MODEL_POSITION_Y,
              MODEL_SIZE_X,
              MODEL_SIZE_Y,
              M_DEFAULT);

   // Set the acceptance and the separation distance.
   MpatControl(MilPatContext, 0, M_ACCEPTANCE, ACCCEPTANCE);

   // Move the child image to draw the model.
   MbufChildMove(MilRightSubImage, TargetImageSizeX, 0, MODEL_IMAGE_SIZE_X, MODEL_IMAGE_SIZE_Y, M_DEFAULT);

   // Draw the model image.
   MgraColor(M_DEFAULT,M_COLOR_RED);
   MpatDraw(M_DEFAULT, MilPatContext, MilRightSubImage, M_DRAW_BOX, 0, M_ORIGINAL);

   // Draw the model name without the path ".mim" extension.
   MgraColor(M_DEFAULT,M_COLOR_GREEN);
   MosSprintf(ModelImageSource, MODEL_NAME_LENGTH, MODEL_IMAGE_NAME);
   ModelImageSource[5] = 0;
   MgraText(M_DEFAULT, MilRightSubImage, 0, MODEL_IMAGE_SIZE_Y- MgraInquire(M_DEFAULT, M_FONT_SIZE, M_NULL), MIL_CONST_TEXT_PTR(ModelImageSource));
     
   MosPrintf(MIL_TEXT("A target image containing multiple occurrences of the object is displayed.\n"));
   MosPrintf(MIL_TEXT("A Model is also defined in the source image and displayed on the right.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Pre-process the context then find all the occurrences for all models.
   MpatControl(MilPatContext, M_ALL, M_NUMBER, M_ALL);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_MODE, M_ENABLE);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_DELTA_POS, 90);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_DELTA_NEG, 90);
   MpatPreprocess(MilPatContext, M_DEFAULT, MilTargetImage);
   MpatFind(MilPatContext, MilTargetImage, MilPatResult);

   // Get and print the number of occurrences found (TotalNumFound).
   MpatGetResult(MilPatResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &TotalNumFound);
   MosPrintf(MIL_TEXT("A total of %i occurrences are found.\n\n"), TotalNumFound);

   if(TotalNumFound)
      {
      // Draw the box and the score for each occurrence.
      MIL_TEXT_CHAR ModelScoreChar[5] = {0};
      MIL_DOUBLE posX, posY, Score;
      for(i = 0; i < TotalNumFound; i++)
         {
         MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX+M_DRAW_POSITION, i, M_DEFAULT);
         MpatGetResult(MilPatResult, i, M_POSITION_X, &posX);
         MpatGetResult(MilPatResult, i, M_POSITION_Y, &posY);
         MpatGetResult(MilPatResult, i, M_SCORE, &Score);
         MosSprintf(ModelScoreChar, 5, MIL_TEXT("%.1f"), Score);
         MgraText(M_DEFAULT, MilGraphicList, posX, posY, MIL_CONST_TEXT_PTR(ModelScoreChar));
         }
      }

   MosPrintf(MIL_TEXT("The %i found occurrences are displayed and the score for each\n"),TotalNumFound);
   MosPrintf(MIL_TEXT("occurrence, is drawn. Scores are low with the current defined\n"));
   MosPrintf(MIL_TEXT("model, which includes an unwanted piece from another object.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("\nMpatDefine (Model with an associated region)\n")
             MIL_TEXT("---------------------------------------------\n\n"));

   // Move the child image to draw the model.
   MbufCopy(MilModelImage, MilRightSubImage);

   // Remove graphics from a 2D graphics list.
   MgraClear(M_DEFAULT, MilGraphicList);

   // Remove the model from the context.
   MpatDefine(MilPatContext, M_DELETE, M_NULL, M_ALL, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Create a rotated bounding box in a graphic list.
   MIL_ID GraphCtx = MgraAlloc(MilSystem, M_NULL);
   MIL_ID RotatedRectGraphList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MgraRectAngle(GraphCtx, RotatedRectGraphList,
                 MODEL_RECT_POSITION_X,
                 MODEL_RECT_POSITION_Y,
                 MODEL_RECT_SIZE_X,
                 MODEL_RECT_SIZE_Y,
                 MODEL_RECT_ANGLE,
                 M_CORNER_AND_DIMENSION);

   // Associate the graphic list to the Model.
   MbufSetRegion(MilModelImage, RotatedRectGraphList, M_DEFAULT, M_NO_RASTERIZE + M_FILL_REGION, M_DEFAULT);

   // Call MpatDefine with all parameters set to M_DEFAULT
   MpatDefine(MilPatContext, M_REGULAR_MODEL, MilModelImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   
   // Draw the model image.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MpatDraw(M_DEFAULT, MilPatContext, MilRightSubImage, M_DRAW_BOX, 0, M_ORIGINAL);

   // Draw the model name without the path ".mim" extension.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MosSprintf(ModelImageSource, MODEL_NAME_LENGTH, MODEL_IMAGE_NAME);
   ModelImageSource[5] = 0;
   MgraText(M_DEFAULT, MilRightSubImage, 0, MODEL_IMAGE_SIZE_Y - 3 * MgraInquire(M_DEFAULT, M_FONT_SIZE, M_NULL), MIL_CONST_TEXT_PTR(ModelImageSource));
   MgraText(M_DEFAULT, MilRightSubImage, 0, MODEL_IMAGE_SIZE_Y - 2 * MgraInquire(M_DEFAULT, M_FONT_SIZE, M_NULL), MIL_TEXT("With an associated region."));

   MosPrintf(MIL_TEXT("A target image containing multiple occurrences of object is displayed.\n"));
   MosPrintf(MIL_TEXT("A Model with an associate region is also defined in the source image\n"));
   MosPrintf(MIL_TEXT("and displayed on the right.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Pre-process the context then find all the occurrences for all models.
   MpatControl(MilPatContext, M_ALL, M_NUMBER, M_ALL);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_MODE, M_ENABLE);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_DELTA_POS, 90);
   MpatControl(MilPatContext, M_ALL, M_SEARCH_ANGLE_DELTA_NEG, 90);
   MpatPreprocess(MilPatContext, M_DEFAULT, MilTargetImage);
   MpatFind(MilPatContext, MilTargetImage, MilPatResult);

   // Get and print the number of occurrences found (TotalNumFound).
   MpatGetResult(MilPatResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &TotalNumFound);
   MosPrintf(MIL_TEXT("A total of %i occurrences are found.\n\n"), TotalNumFound);

   if(TotalNumFound)
      {
      // Draw the box and the score for each occurrence.
      MIL_TEXT_CHAR ModelScoreChar[5] = {0};
      MIL_DOUBLE posX, posY, Score;
      for(i = 0; i < TotalNumFound; i++)
         {
         MpatDraw(M_DEFAULT, MilPatResult, MilGraphicList, M_DRAW_BOX + M_DRAW_POSITION, i, M_DEFAULT);
         MpatGetResult(MilPatResult, i, M_POSITION_X, &posX);
         MpatGetResult(MilPatResult, i, M_POSITION_Y, &posY);
         MpatGetResult(MilPatResult, i, M_SCORE, &Score);
         MosSprintf(ModelScoreChar, 5, MIL_TEXT("%.1f"), Score);
         MgraText(M_DEFAULT, MilGraphicList, posX, posY, MIL_CONST_TEXT_PTR(ModelScoreChar));
         }
      }

   MosPrintf(MIL_TEXT("The %i found occurrences are displayed and the score for each\n"), TotalNumFound);
   MosPrintf(MIL_TEXT("occurrence, is drawn. Scores are higher with the model defined\n"));
   MosPrintf(MIL_TEXT("at an angle that excludes an unwanted piece from another object.\n\n"));

   MosPrintf(MIL_TEXT("\nPress <Enter> to finish.\n"));
   MosGetch();

   // Free allocations.
   MgraFree(RotatedRectGraphList);
   MgraFree(GraphCtx);
   MbufFree(MilModelImage);
   MpatFree(MilPatResult);
   MpatFree(MilPatContext);
   MbufFree(MilRightSubImage);
   MbufFree(MilLeftSubImage);
   MgraFree(MilGraphicList);
   MbufFree(MilDispImage);
   MbufFree(MilTargetImage);
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

