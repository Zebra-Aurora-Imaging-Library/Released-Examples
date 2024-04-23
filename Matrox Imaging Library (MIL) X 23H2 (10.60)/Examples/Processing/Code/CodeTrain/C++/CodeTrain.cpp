/***************************************************************************************/
/*
* File name: CodeTrain.cpp
*
* Synopsis:  This programs trains a code reader context from a set of sample images.
*            See the PrintHeader() function below for a detailed description.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <vector>
using std::vector;

///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("CodeTrain\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program trains a code reader context from set of sample images.\n")
             MIL_TEXT("Useful results of the training process are then retrieved and displayed.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

const MIL_INT  CONTROL_LENGTH_MAX = 30;
const MIL_INT  STRING_LENGTH_MAX  = 50;
const MIL_DOUBLE MinimumAcceptance = 85.0;

const MIL_INT TrainSetImageNumber = 5;
static MIL_CONST_TEXT_PTR TrainSetImageFilename[TrainSetImageNumber] =
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix1.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix2.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix3.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix4.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix5.mim")
   };

const MIL_INT ImageNumber = 4;
static MIL_CONST_TEXT_PTR ImageFilename[ImageNumber] =
   {
   M_IMAGE_PATH MIL_TEXT("CodeTrain/Image1.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeTrain/Image2.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeTrain/Image3.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeTrain/Image4.mim"),
   };

class CTrainControl
   {
   public:

      // Constructor
      CTrainControl(MIL_INT NbCodeModel)
         {
         m_NbCodeModel = NbCodeModel;

         m_CellNumberX.resize(NbCodeModel);
         m_CellNumberY.resize(NbCodeModel);
         m_CellNumberXMin.resize(NbCodeModel);
         m_CellNumberXMax.resize(NbCodeModel);
         m_CellNumberYMin.resize(NbCodeModel);
         m_CellNumberYMax.resize(NbCodeModel);
         m_DotSpacingMin.resize(NbCodeModel);
         m_DotSpacingMax.resize(NbCodeModel);
         m_ForegroundValue.resize(NbCodeModel);
         m_CodeFlip.resize(NbCodeModel);
         m_DatamatrixShape.resize(NbCodeModel);
         }

      std::vector<MIL_INT> m_CellNumberX;
      std::vector<MIL_INT> m_CellNumberY;
      std::vector<MIL_INT> m_CellNumberXMin;
      std::vector<MIL_INT> m_CellNumberXMax;
      std::vector<MIL_INT> m_CellNumberYMin;
      std::vector<MIL_INT> m_CellNumberYMax;
      std::vector<MIL_INT> m_DotSpacingMin;
      std::vector<MIL_INT> m_DotSpacingMax;
      std::vector<MIL_INT> m_ForegroundValue;
      std::vector<MIL_INT> m_CodeFlip;
      std::vector<MIL_INT> m_DatamatrixShape;

      MIL_INT m_NbCodeModel;
   };


void CodeTrain(const MIL_INT NumberOfImage,
               MIL_CONST_TEXT_PTR SrcFilename[],
               MIL_ID MilCodeContext,
               MIL_ID MilSystem,
               MIL_ID MilDisplay);

void CodeRead(const MIL_INT NumberOfImage,
              MIL_CONST_TEXT_PTR SrcFilename[],
              MIL_ID MilCodeContext,
              MIL_ID MilSystem,
              MIL_ID MilDisplay);

//************************************
// Utility sub-functions declarations
//************************************

void AllocDisplayImage(MIL_ID MilSystem,
                       MIL_ID MilSrcImage,
                       MIL_ID MilDisplay,
                       MIL_ID& MilDispProcImage,
                       MIL_ID& MilOverlayImage);

void GetControlTypesNames(MIL_INT64 ControlType,
                         MIL_TEXT_CHAR* ControlTypeName);

void GetControlValuesNames(MIL_INT64 ControlType,
                           MIL_DOUBLE ControlValue,
                           MIL_TEXT_CHAR* ControlValueName);

void GetCodeTypesNames(MIL_INT CodeType,
                       MIL_TEXT_CHAR* CodeTypeName);


//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_NULL);

   // Add a Data Matrix model.
   McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Print Header.
   PrintHeader();

   // Train the code reader context.
   CodeTrain(TrainSetImageNumber, TrainSetImageFilename, MilCodeContext, MilSystem, MilDisplay);

   // Decode images using the trained context.
   CodeRead(ImageNumber, ImageFilename, MilCodeContext, MilSystem, MilDisplay);

   // Release the allocated objects.
   McodeFree(MilCodeContext);

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }


/* This function trains a context from a set of sample images. */
void CodeTrain(const MIL_INT NumberOfImage,
               MIL_CONST_TEXT_PTR SrcFilename[],
               MIL_ID MilCodeContext,
               MIL_ID MilSystem,
               MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("1) Training a context from a list of sample images.\n")
             MIL_TEXT("   ================================================\n\n"));

   // Allocate a code result for training.
   MIL_ID MilCodeTrainResult = McodeAllocResult(MilSystem, M_CODE_TRAIN_RESULT, M_NULL);
   
   // Restore the image.
   std::vector<MIL_ID> MilSrcImage(NumberOfImage, M_NULL);
   for(MIL_INT ii = 0; ii < NumberOfImage; ii++)
      MilSrcImage[ii] = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   
   AllocDisplayImage(MilSystem, MilSrcImage[0], MilDisplay, MilDispProcImage, MilOverlayImage);

   // Display training status.
   MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
   MgraRectFill(M_DEFAULT, MilOverlayImage, 200, 200, 400, 300);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MosPrintf(MIL_TEXT("Training in progress using %d sample images... "), TrainSetImageNumber);
   MgraText(M_DEFAULT, MilOverlayImage, 210, 240, MIL_TEXT("Training in progress...")); 

   // Set the context to train all trainable controls.
   McodeControl(MilCodeContext, M_SET_TRAINING_STATE_ALL, M_ENABLE);

   // This application does not require reading codes with rotation (application specific prior knowledge).
   // This disables training the M_SEARCH_ANGLE parameter.
   McodeControl(MilCodeContext, M_SEARCH_ANGLE + M_TRAIN, M_DISABLE);

   // Train the context. 
   McodeTrain(MilCodeContext, NumberOfImage, &MilSrcImage.front(), M_DEFAULT, MilCodeTrainResult);
   
   MosPrintf(MIL_TEXT("completed.\n\n"));
   MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
   MgraRectFill(M_DEFAULT, MilOverlayImage, 200, 200, 400, 300);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraText(M_DEFAULT, MilOverlayImage, 210, 205, MIL_TEXT("Training completed."));

   // Retrieve training statistics.
   MIL_DOUBLE TrainingScore;
   MIL_INT NbFail, NbPass;
   MIL_INT NbTrainingImages;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_NUMBER_OF_TRAINING_IMAGES + M_TYPE_MIL_INT, &NbTrainingImages);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAINING_SCORE, &TrainingScore);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_PASSED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbPass);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_FAILED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbFail);

   MIL_TEXT_CHAR NumberString[STRING_LENGTH_MAX];
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("#PASS trained images: %d"), NbPass);
   MgraText(M_DEFAULT, MilOverlayImage, 210, 240, NumberString);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("#FAIL trained images: %d"), NbFail);
   MgraText(M_DEFAULT, MilOverlayImage, 210, 260, NumberString);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("Training score = %.2f"), TrainingScore);
   MgraText(M_DEFAULT, MilOverlayImage, 210, 280, NumberString);


   // Retrieving the global training results.
   std::vector<MIL_INT> ListIndexImagePass(NbPass,0);
   std::vector<MIL_INT> ListIndexImageFail(NbFail,0);
   std::vector<MIL_ID> ListIdImagePass(NbPass, M_NULL);
   std::vector<MIL_ID> ListIdImageFail(NbFail, M_NULL);

   if(NbPass > 0)
      {
      McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_PASSED_IMAGES_INDEX + M_TYPE_MIL_INT, &ListIndexImagePass.front());
      McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_PASSED_IMAGES_ID + M_TYPE_MIL_INT, &ListIdImagePass.front());
      }

   if(NbFail > 0)
      {
      McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_FAILED_IMAGES_INDEX + M_TYPE_MIL_INT, &ListIndexImageFail.front());
      McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_FAILED_IMAGES_ID + M_TYPE_MIL_INT, &ListIdImageFail.front());
      }

   // Retrieve the individual result ids for each trained image.
   std::vector<MIL_ID> MilCodeResult(NbTrainingImages, M_NULL);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_CODE_RESULT_ID + M_TYPE_MIL_ID, &MilCodeResult.front());

   // Retrieve the number of models that have been trained and their respective model ids.
   MIL_INT NbCodeModel = 0;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_NUMBER_OF_CODE_MODELS + M_TYPE_MIL_INT, &NbCodeModel);

   std::vector<MIL_ID> MilCodeModel(NbCodeModel, M_NULL);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_CODE_MODEL_ID + M_TYPE_MIL_ID, &MilCodeModel.front());

   std::vector<MIL_INT> NbOccurrenceByModel(NbCodeModel, 0);
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CODE_MODEL_NUMBER_OF_OCCURRENCES + M_TYPE_MIL_INT, &NbOccurrenceByModel.front());

   std::vector<MIL_INT> CodeType(NbCodeModel, 0);
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &CodeType.front());

   // Retrieve training results.
   CTrainControl ResTrain(NbCodeModel);

   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_X + M_TYPE_MIL_INT, &ResTrain.m_CellNumberX.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_Y + M_TYPE_MIL_INT, &ResTrain.m_CellNumberY.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_X_MIN + M_TYPE_MIL_INT, &ResTrain.m_CellNumberXMin.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_X_MAX + M_TYPE_MIL_INT, &ResTrain.m_CellNumberXMax.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_Y_MIN + M_TYPE_MIL_INT, &ResTrain.m_CellNumberYMin.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CELL_NUMBER_Y_MAX + M_TYPE_MIL_INT, &ResTrain.m_CellNumberYMax.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_DOT_SPACING_MIN + M_TYPE_MIL_INT, &ResTrain.m_DotSpacingMin.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_DOT_SPACING_MAX + M_TYPE_MIL_INT, &ResTrain.m_DotSpacingMax.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_FOREGROUND_VALUE + M_TYPE_MIL_INT, &ResTrain.m_ForegroundValue.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CODE_FLIP + M_TYPE_MIL_INT, &ResTrain.m_CodeFlip.front());
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_DATAMATRIX_SHAPE + M_TYPE_MIL_INT, &ResTrain.m_DatamatrixShape.front());

   MosPrintf(MIL_TEXT("Training statistics:\n\n"));
   MosPrintf(MIL_TEXT("  - Score   : %.2f\n"), TrainingScore);
   MosPrintf(MIL_TEXT("  - Nb Pass : %d\n"), NbPass);
   MosPrintf(MIL_TEXT("  - Nb Fail : %d\n\n"), NbFail);
   MosPrintf(MIL_TEXT("  - Successfully trained images: \n    "));
   MosPrintf(MIL_TEXT("\tIndex\tID\n"));
   MosPrintf(MIL_TEXT("\t=====\t==\n"));

   for(MIL_INT ii = 0; ii < NbPass; ii++)
      MosPrintf(MIL_TEXT("\t%d\t%d\n"), ListIndexImagePass[ii], ListIdImagePass[ii]);

   MosPrintf(MIL_TEXT("\n\nTraining informations:\n\n"));

   // Retrieve and print information for each enabled context control types to be trained 
   std::vector<MIL_INT64> EnabledContextControl;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES, EnabledContextControl);

   std::vector<MIL_INT> ContextControlState;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_STATE, ContextControlState);

   std::vector<MIL_DOUBLE> ContextControlValue;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_ORIGINAL_VALUE, ContextControlValue);

   std::vector<MIL_DOUBLE> ContextControlTrainedValue;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_TRAINED_VALUE, ContextControlTrainedValue);

   if(EnabledContextControl.size() > 0)
      {
      MosPrintf(MIL_TEXT("Context Control Type    \tState            \tOriginal Value       \tTrained Value   \n"));
      MosPrintf(MIL_TEXT("========================\t=================\t=====================\t================\n"));
      }

   MIL_TEXT_CHAR ControlTypeName[STRING_LENGTH_MAX];
   MIL_TEXT_CHAR ControlValueName[STRING_LENGTH_MAX];
   MIL_TEXT_CHAR CodeTypeName[STRING_LENGTH_MAX];

   for(MIL_INT ii = 0; ii < (MIL_INT)EnabledContextControl.size(); ii++)
      {
      GetControlTypesNames(EnabledContextControl[ii], ControlTypeName);
      MosPrintf(MIL_TEXT("%-25s\t"), ControlTypeName);

      if(ContextControlState[ii] == M_OPTIMIZABLE)
         MosPrintf(MIL_TEXT("%-17s\t"), MIL_TEXT("Optimizable"));
      else
         MosPrintf(MIL_TEXT("%-17s\t"), MIL_TEXT("Not Optimizable"));

      GetControlValuesNames(EnabledContextControl[ii], ContextControlValue[ii], ControlValueName);
      MosPrintf(MIL_TEXT("%-21s\t"), ControlValueName);

      GetControlValuesNames(EnabledContextControl[ii], ContextControlTrainedValue[ii], ControlValueName);
      MosPrintf(MIL_TEXT("%-21s\n"), ControlValueName);
      }


   // Retrieve and print information for each enabled model control types to be trained 
   for(MIL_INT ii = 0; ii < NbCodeModel; ii++)
      {
      std::vector<MIL_INT64> EnabledModelControl;
      McodeGetResult(MilCodeTrainResult, ii, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES, EnabledModelControl);

      std::vector<MIL_INT> ModelControlState;
      McodeGetResult(MilCodeTrainResult, ii, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_STATE, ModelControlState);

      std::vector<MIL_DOUBLE> ModelControlValue;
      McodeGetResult(MilCodeTrainResult, ii, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_ORIGINAL_VALUE, ModelControlValue);

      std::vector<MIL_DOUBLE> ModelControlTrainedValue;
      McodeGetResult(MilCodeTrainResult, ii, M_GENERAL, M_TRAIN_ENABLED_CONTROL_TYPES_TRAINED_VALUE, ModelControlTrainedValue);

      if(EnabledModelControl.size() > 0)
         {
         GetCodeTypesNames(CodeType[ii], CodeTypeName);

         MosPrintf(MIL_TEXT("\nModel #%d ( %s ) Number of occurrences used for training: %d \n"), ii, CodeTypeName, NbOccurrenceByModel[ii]);

         MosPrintf(MIL_TEXT("  Model Control Type      \tState            \tOriginal Value       \tTrained Value   \n"));
         MosPrintf(MIL_TEXT("  ========================\t=================\t=====================\t================\n"));
         }

      for(MIL_INT ii = 0; ii < (MIL_INT)EnabledModelControl.size(); ii++)
         {
         GetControlTypesNames(EnabledModelControl[ii], ControlTypeName);
         MosPrintf(MIL_TEXT("  %-25s\t"), ControlTypeName);

         if(ModelControlState[ii] == M_OPTIMIZABLE)
            MosPrintf(MIL_TEXT("%-17s\t"), MIL_TEXT("Optimizable"));
         else
            MosPrintf(MIL_TEXT("%-17s\t"), MIL_TEXT("Not Optimizable"));

         GetControlValuesNames(EnabledModelControl[ii], ModelControlValue[ii], ControlValueName);
         MosPrintf(MIL_TEXT("%-21s\t"), ControlValueName);

         GetControlValuesNames(EnabledModelControl[ii], ModelControlTrainedValue[ii], ControlValueName);
         MosPrintf(MIL_TEXT("%-21s\n"), ControlValueName);
         }
      }


   MosPrintf(MIL_TEXT("\n\nPress <Enter> to retrieve controls that could be modified by the training.\n\n"));
   MosGetch();

   // Retrieve and print context control types that could be modified by the training
   std::vector<MIL_INT64> ContextControlList;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAINED_CONTROL_TYPES, ContextControlList);

   MosPrintf(MIL_TEXT("\n\nThe following context controls could be modified by the training:\n"));

   for(MIL_INT ii = 0; ii < (MIL_INT)ContextControlList.size(); ii++)
      {
      GetControlTypesNames(ContextControlList[ii], ControlTypeName);
      MosPrintf(MIL_TEXT("%s\n"), ControlTypeName);
      }

   // Retrieve and print code model control types that could be modified by the training
   MosPrintf(MIL_TEXT("\n\nThe following code model controls could be modified by the training:\n"));
   for(MIL_INT ii = 0; ii < NbCodeModel; ii++)
      {
      std::vector<MIL_INT64> CodeModelControlList;
      McodeGetResult(MilCodeTrainResult, ii, M_GENERAL, M_TRAINED_CONTROL_TYPES, CodeModelControlList);

      if(CodeModelControlList.size() > 0)
         {
         GetCodeTypesNames(CodeType[ii], CodeTypeName);
         MosPrintf(MIL_TEXT("Model #%d ( %s ) \n"), ii, CodeTypeName);

         }

      for(MIL_INT ii = 0; ii < (MIL_INT)CodeModelControlList.size(); ii++)
         {
         GetControlTypesNames(CodeModelControlList[ii], ControlTypeName);
         MosPrintf(MIL_TEXT("  %s\n"), ControlTypeName);
         }

      }

   MosPrintf(MIL_TEXT("\n\nPress <Enter> to retrieve the individual training results.\n\n"));
   MosGetch();

   for(MIL_INT ii = 0; ii < NbTrainingImages; ii++)
      {
      MosPrintf(MIL_TEXT("Training results for sample image [%d]:\n"), ii);

      // Allocate a display image.
      MIL_ID MilDispProcCurImage,         // Display and destination buffer.
             MilOverlayCurImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage[ii], MilDisplay, MilDispProcCurImage, MilOverlayCurImage);

      // Retrieve the decoding status.
      MIL_INT ReadStatus;
      McodeGetResult(MilCodeResult[ii], M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);

      // Check if the decode operation was successful.
      if(ReadStatus == M_STATUS_READ_OK)
         {
         //Get number of codes read.
         MIL_INT NbOccFound = 0;
         McodeGetResult(MilCodeResult[ii], M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbOccFound);
         MosPrintf(MIL_TEXT("   - Successfully trained\n"));
         MosPrintf(MIL_TEXT("   - Number of occurrences found: %d\n"), NbOccFound);

         MgraColor(M_DEFAULT, M_COLOR_GREEN);

         for(MIL_INT jj = 0; jj < NbOccFound; jj++)
            {
            McodeDraw(M_DEFAULT, MilCodeResult[ii], MilOverlayCurImage, M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);
            MosPrintf(MIL_TEXT("        - Occurrence: %d\n"), jj);

            // Get decoded string.
            MIL_INT ResultStringSize;
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &ResultStringSize);
            std::vector<MIL_TEXT_CHAR> ResultString(ResultStringSize);
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_STRING, &ResultString.front());

            // Retrieve basic results.
            MIL_DOUBLE PositionX, PositionY, SizeX, SizeY;
            McodeControl(MilCodeResult[ii], M_RESULT_OUTPUT_UNITS, M_PIXEL);
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_POSITION_X, &PositionX);
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_POSITION_X, &PositionY);
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_SIZE_X, &SizeX);
            McodeGetResult(MilCodeResult[ii], jj, M_GENERAL, M_SIZE_Y, &SizeY);

            MosPrintf(MIL_TEXT("        - Code read: %s\n"), &ResultString.front());
            MosPrintf(MIL_TEXT("        - Position: (%.2f, %.2f)\n"), PositionX, PositionY);
            MosPrintf(MIL_TEXT("        - Dimensions: (%.2f x %.2f)\n\n"), SizeX, SizeY);
            
            MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();
            }
         }
      else
         {
         MosPrintf(MIL_TEXT("   - Unsuccessfully trained\n"));
         }

      MbufFree(MilDispProcCurImage);
      }

   // Saves a report containing most of the results from a train operation as a flat text file
   MIL_TEXT_PTR pOutFilename = const_cast<MIL_TEXT_PTR>(MIL_TEXT("TrainReportFile.txt"));
   McodeStream(pOutFilename, M_NULL, M_SAVE_REPORT, M_FILE, M_DEFAULT, M_DEFAULT, &MilCodeTrainResult, M_NULL);
   MosPrintf(MIL_TEXT("A train report was saved in TrainReportFile.txt\n\n"));

   // Validating the training score. 
   if (TrainingScore >= MinimumAcceptance)
      {
      MosPrintf(MIL_TEXT("Training has been successfully done and the context will be reset using\nthe result of the training.\n\n"));

      // The training has been successfully done.
      // The code reader context is reset using the result of the training.
      McodeControl(MilCodeContext, M_RESET_FROM_TRAINED_RESULTS, MilCodeTrainResult);
      }
   else
      {
      MosPrintf(MIL_TEXT("Training score too low.\n\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MbufFree(MilDispProcImage);
   McodeFree(MilCodeTrainResult);

   for(MIL_INT ii = 0; ii < NumberOfImage; ii++)
      {
      // Free Buffer allocation.
      MbufFree(MilSrcImage[ii]);
      }
   }

/* This function uses the trained context to decode images. */
void CodeRead(const MIL_INT NumberOfImage,
               MIL_CONST_TEXT_PTR SrcFilename[],
               MIL_ID MilCodeContext,
               MIL_ID MilSystem,
               MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("2) Reading codes in new images using the trained context.\n")
             MIL_TEXT("   ======================================================\n\n"));

   MIL_ID MilReadResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   for(MIL_INT ii = 0; ii < NumberOfImage; ii++)
      {
      MIL_ID MilSrcImage;
      MIL_INT LocalStatus = 0;
      MIL_INT NbDecoded = 0;

      // Restore the image.
      MilSrcImage = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);

      // Allocate a display image.
      MIL_ID MilDispProcImage,         // Display and destination buffer.
             MilOverlayImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

      // Display reading status.
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraText(M_DEFAULT, MilOverlayImage, 5, 5, MIL_TEXT("Reading after training..."));

      MosPrintf(MIL_TEXT("Image %i\n"), ii);

      // Read.
      McodeRead(MilCodeContext, MilSrcImage, MilReadResult);

      McodeGetResult(MilReadResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &LocalStatus);
      McodeGetResult(MilReadResult, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbDecoded);
      
      switch(LocalStatus)
         {
         case M_STATUS_READ_OK:
            MosPrintf(MIL_TEXT("STATUS: READ OK.\n"));
            MgraColor(M_DEFAULT, M_COLOR_GREEN);
            for(MIL_INT jj = 0; jj < NbDecoded; jj++)
               McodeDraw(M_DEFAULT, MilReadResult, MilOverlayImage, M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);
            break;

         case M_STATUS_NOT_FOUND:
            MosPrintf(MIL_TEXT("STATUS: NOT FOUND.\n"));
            break;

         case M_STATUS_TIMEOUT_END:
            MosPrintf(MIL_TEXT("STATUS: TIMEOUT END.\n"));
            break;

         default:
            MosPrintf(MIL_TEXT("Unrecognize status.\n"));
            break;
         }

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      MbufFree(MilDispProcImage);
      MbufFree(MilSrcImage);
      }

   McodeFree(MilReadResult);

   }

//******************
// Utility functions
//******************

void AllocDisplayImage(MIL_ID MilSystem,
                       MIL_ID MilSrcImage,
                       MIL_ID MilDisplay,
                       MIL_ID& MilDispProcImage,
                       MIL_ID& MilOverlayImage)
   {
   // Retrieve the source image size.
   MIL_INT SrcSizeX, SrcSizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SrcSizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SrcSizeY);

   // Allocate the display image.
   MbufAlloc2d(MilSystem,
               SrcSizeX,
               SrcSizeY,
               8L + M_UNSIGNED,
               M_IMAGE + M_PROC + M_DISP,
               &MilDispProcImage);

   MbufCopy(MilSrcImage, MilDispProcImage);

   // Display the image buffer.
   MdispSelect(MilDisplay, MilDispProcImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   }


void GetControlValuesNames(MIL_INT64 ControlType,
                           MIL_DOUBLE ControlValue,
                           MIL_TEXT_CHAR* ControlValueName)
   {
   switch(ControlType)
      {
      case M_THRESHOLD_MODE:
         switch((MIL_INT)ControlValue)
            {
            case M_ADAPTIVE:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ADAPTIVE"));                          break;
            case M_GLOBAL_SEGMENTATION:               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_GLOBAL_SEGMENTATION"));               break;
            case M_GLOBAL_WITH_LOCAL_RESEGMENTATION:  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_GLOBAL_WITH_LOCAL_RESEGMENTATION"));  break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                           break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));                             break;
            }
         break;

      case M_THRESHOLD_VALUE:
         switch((MIL_INT)ControlValue)
            {
            case M_AUTO_COMPUTE:                      MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_AUTO_COMPUTE"));            break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                 break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%d"), (MIL_INT)ControlValue); break;
            }
         break;

      case M_SPEED:
         switch((MIL_INT)ControlValue)
            {
            case M_HIGH:                              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_HIGH"));                break;
            case M_LOW:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_LOW"));                 break;
            case M_MEDIUM:                            MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_MEDIUM"));              break;
            case M_VERY_HIGH:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_VERY_HIGH"));           break;
            case M_VERY_LOW:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_VERY_LOW"));            break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_SEARCH_ANGLE_MODE:
         switch((MIL_INT)ControlValue)
            {
            case M_ENABLE:                            MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENABLE"));              break;
            case M_DISABLE:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DISABLE"));             break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_CELL_NUMBER_X:
      case M_CELL_NUMBER_X_MIN:
      case M_CELL_NUMBER_X_MAX:
      case M_CELL_NUMBER_Y:
      case M_CELL_NUMBER_Y_MIN:
      case M_CELL_NUMBER_Y_MAX:
         switch((MIL_INT)ControlValue)
            {
            case M_ANY:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ANY"));                     break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                 break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%d"), (MIL_INT)ControlValue); break;
            }
         break;

      case M_CELL_SIZE_MIN:
      case M_CELL_SIZE_MAX:
         switch((MIL_INT)ControlValue)
            {
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));          break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%.3f"), ControlValue); break;
            }
         break;

      case M_DOT_SPACING_MIN:
      case M_DOT_SPACING_MAX:
         switch((MIL_INT)ControlValue)
            {
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                       break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%d"), (MIL_INT)ControlValue);       break;
            }
         break;

      case M_FOREGROUND_VALUE:
         switch((MIL_INT)ControlValue)
            {
            case M_FOREGROUND_ANY:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FOREGROUND_ANY"));      break;
            case M_FOREGROUND_BLACK:                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FOREGROUND_BLACK"));    break;
            case M_FOREGROUND_WHITE:                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FOREGROUND_WHITE"));    break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_SEARCH_ANGLE:
         switch((MIL_INT)ControlValue)
            {
            case M_ACCORDING_TO_REGION:               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ACCORDING_TO_REGION"));    break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%.3f"), ControlValue);       break;
            }
         break;

      case M_SEARCH_ANGLE_DELTA_POS:
      case M_SEARCH_ANGLE_DELTA_NEG:
         switch((MIL_INT)ControlValue)
            {
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%.3f"), ControlValue);       break;
            }
         break;

      case M_CODE_FLIP:
         switch((MIL_INT)ControlValue)
            {
            case M_ANY:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ANY"));                 break;
            case M_FLIP:                              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FLIP"));                break;
            case M_NO_FLIP:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_NO_FLIP"));             break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_DATAMATRIX_SHAPE:
         switch((MIL_INT)ControlValue)
            {
            case M_ANY:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ANY"));                 break;
            case M_RECTANGLE:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_RECTANGLE"));           break;
            case M_SQUARE:                            MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SQUARE"));              break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_ERROR_CORRECTION:
         switch((MIL_INT)ControlValue)
            {
            case M_ECC_NONE:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_NONE"));                     break;
            case M_ECC_200:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_200"));                      break;
            case M_ECC_CHECK_DIGIT:                   MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_CHECK_DIGIT"));              break;
            case M_ECC_REED_SOLOMON_0:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_0"));           break;
            case M_ECC_REED_SOLOMON_1:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_1"));           break;
            case M_ECC_REED_SOLOMON_2:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_2"));           break;
            case M_ECC_REED_SOLOMON_3:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_3"));           break;
            case M_ECC_REED_SOLOMON_4:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_4"));           break;
            case M_ECC_REED_SOLOMON_5:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_5"));           break;
            case M_ECC_REED_SOLOMON_6:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_6"));           break;
            case M_ECC_REED_SOLOMON_7:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_7"));           break;
            case M_ECC_REED_SOLOMON_8:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON_8"));           break;
            case M_ECC_REED_SOLOMON:                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_REED_SOLOMON"));             break;
            case M_ECC_COMPOSITE:                     MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_COMPOSITE"));                break;
            case M_ECC_L:                             MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_L"));                        break;
            case M_ECC_M:                             MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_M"));                        break;
            case M_ECC_H:                             MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_H"));                        break;
            case M_ECC_Q:                             MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_Q"));                        break;
            case M_ANY:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ANY"));                          break;
            case M_ECC_4STATE:                        MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ECC_4STATE"));                   break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                      break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("%d %%"), (MIL_INT)ControlValue);   break;
            }
         break;

      case M_ENCODING:
         switch((MIL_INT)ControlValue)
            {
            case M_ENC_NUM:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_NUM"));                            break;
            case M_ENC_ALPHA:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_ALPHA"));                          break;
            case M_ENC_ALPHANUM:                      MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_ALPHANUM"));                       break;
            case M_ENC_ALPHANUM_PUNC:                 MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_ALPHANUM_PUNC"));                  break;
            case M_ENC_ASCII:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_ASCII"));                          break;
            case M_ENC_ISO8:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_ISO8"));                           break;
            case M_ENC_STANDARD:                      MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_STANDARD"));                       break;
            case M_ENC_MODE2:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_MODE2"));                          break;
            case M_ENC_MODE3:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_MODE3"));                          break;
            case M_ENC_MODE4:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_MODE4"));                          break;
            case M_ENC_MODE5:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_MODE5"));                          break;
            case M_ENC_MODE6:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_MODE6"));                          break;
            case M_ENC_GS1_DATABAR_OMNI:              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_OMNI"));               break;
            case M_ENC_GS1_DATABAR_TRUNCATED:         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_TRUNCATED"));          break;
            case M_ENC_GS1_DATABAR_LIMITED:           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_LIMITED"));            break;
            case M_ENC_GS1_DATABAR_EXPANDED:          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_EXPANDED"));           break;
            case M_ENC_GS1_DATABAR_STACKED:           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_STACKED"));            break;
            case M_ENC_GS1_DATABAR_STACKED_OMNI:      MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_STACKED_OMNI"));       break;
            case M_ENC_GS1_DATABAR_EXPANDED_STACKED:  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_DATABAR_EXPANDED_STACKED"));   break;
            case M_ENC_EAN8:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_EAN8"));                           break;
            case M_ENC_EAN13:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_EAN13"));                          break;
            case M_ENC_UPCA:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_UPCA"));                           break;
            case M_ENC_UPCE:                          MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_UPCE"));                           break;
            case M_ENC_GS1_128_PDF417:                MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_128_PDF417"));                 break;
            case M_ENC_GS1_128_MICROPDF417:           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_GS1_128_MICROPDF417"));            break;
            case M_ENC_QRCODE_MODEL1:                 MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_QRCODE_MODEL1"));                  break;
            case M_ENC_QRCODE_MODEL2:                 MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_QRCODE_MODEL2"));                  break;
            case M_ENC_US_MAIL:                       MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_US_MAIL"));                        break;
            case M_ENC_UK_MAIL:                       MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_UK_MAIL"));                        break;
            case M_ENC_AUSTRALIA_MAIL_RAW:            MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AUSTRALIA_MAIL_RAW"));             break;
            case M_ENC_AUSTRALIA_MAIL_N:              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AUSTRALIA_MAIL_N"));               break;
            case M_ENC_AUSTRALIA_MAIL_C:              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AUSTRALIA_MAIL_C"));               break;
            case M_ENC_KOREA_MAIL:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_KOREA_MAIL"));                     break;
            case M_ENC_UPCA_ADDON:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_UPCA_ADDON"));                     break;
            case M_ENC_UPCE_ADDON:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_UPCE_ADDON"));                     break;
            case M_ENC_EAN13_ADDON:                   MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_EAN13_ADDON"));                    break;
            case M_ENC_EAN8_ADDON:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_EAN8_ADDON"));                     break;
            case M_ENC_AZTEC_COMPACT:                 MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AZTEC_COMPACT"));                  break;
            case M_ENC_AZTEC_FULL_RANGE:              MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AZTEC_FULL_RANGE"));               break;
            case M_ENC_AZTEC_RUNE:                    MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENC_AZTEC_RUNE"));                     break;
            case M_ANY:                               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ANY"));                                break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));                            break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));                              break;
            }
         break;

      case M_DECODE_ALGORITHM:
         switch((MIL_INT)ControlValue)
            {
            case M_CODE_DEFORMED:                     MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CODE_DEFORMED"));       break;
            case M_CODE_NOT_DEFORMED:                 MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CODE_NOT_DEFORMED"));   break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));             break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));               break;
            }
         break;

      case M_USE_PRESEARCH:
         switch((MIL_INT)ControlValue)
            {
            case M_DISABLE:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DISABLE"));               break;
            case M_FINDER_PATTERN_BASE:               MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FINDER_PATTERN_BASE"));   break;
            case M_STAT_BASE:                         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_STAT_BASE"));             break;
            case M_DEFAULT:                           MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DEFAULT"));               break;
            default:                                  MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));                 break;
            }
         break;

      default:
         MosSprintf(ControlValueName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown Control"));
         break;
      }

   }

void GetControlTypesNames(MIL_INT64 ControlType,
                           MIL_TEXT_CHAR* ControlTypeName)
   {
   switch(ControlType)
      {
      case M_SPEED:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SPEED"));
         break;
      case M_THRESHOLD_MODE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_THRESHOLD_MODE"));
         break;
      case M_THRESHOLD_VALUE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_THRESHOLD_VALUE"));
         break;
      case M_SEARCH_ANGLE_MODE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SEARCH_ANGLE_MODE"));
         break;
      case M_CELL_NUMBER_X:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_X"));
         break;
      case M_CELL_NUMBER_Y:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_Y"));
         break;
      case M_CELL_NUMBER_X_MIN:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_X_MIN"));
         break;
      case M_CELL_NUMBER_Y_MIN:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_Y_MIN"));
         break;
      case M_CELL_NUMBER_X_MAX:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_X_MAX"));
         break;
      case M_CELL_NUMBER_Y_MAX:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_NUMBER_Y_MAX"));
         break;
      case M_CELL_SIZE_MIN:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_SIZE_MIN"));
         break;
      case M_CELL_SIZE_MAX:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CELL_SIZE_MAX"));
         break;
      case M_CODE_FLIP:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_CODE_FLIP"));
         break;
      case M_DATAMATRIX_SHAPE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DATAMATRIX_SHAPE"));
         break;
      case M_DECODE_ALGORITHM:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DECODE_ALGORITHM"));
         break;
      case M_DOT_SPACING_MIN:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DOT_SPACING_MIN"));
         break;
      case M_DOT_SPACING_MAX:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_DOT_SPACING_MAX"));
         break;
      case M_ENCODING:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ENCODING"));
         break;
      case M_ERROR_CORRECTION:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_ERROR_CORRECTION"));
         break;
      case M_FOREGROUND_VALUE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_FOREGROUND_VALUE"));
         break;
      case M_SEARCH_ANGLE:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SEARCH_ANGLE"));
         break;
      case M_SEARCH_ANGLE_DELTA_POS:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SEARCH_ANGLE_DELTA_POS"));
         break;
      case M_SEARCH_ANGLE_DELTA_NEG:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_SEARCH_ANGLE_DELTA_NEG"));
         break;
      case M_USE_PRESEARCH:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("M_USE_PRESEARCH"));
         break;
      default:
         MosSprintf(ControlTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));
         break;
      }
   }

void GetCodeTypesNames(MIL_INT CodeType,
                       MIL_TEXT_CHAR* CodeTypeName)
   {
   switch(CodeType)
      {
      case M_CODE39:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Code39"));
         break;
      case M_DATAMATRIX:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Datamatrix"));
         break;
      case M_EAN13:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("EAN13"));
         break;
      case M_MAXICODE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Maxicode"));
         break;
      case M_INTERLEAVED25:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Interleaved25"));
         break;
      case M_CODE128:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Code128"));
         break;
      case M_BC412:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("BC412"));
         break;
      case M_CODABAR:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Codabar"));
         break;
      case M_PDF417:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("PDF417"));
         break;
      case M_POSTNET:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Postnet"));
         break;
      case M_PLANET:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Planet"));
         break;
      case M_UPC_A:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("UPC_A"));
         break;
      case M_UPC_E:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("UPC_E"));
         break;
      case M_PHARMACODE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Pharmacode"));
         break;
      case M_GS1_DATABAR:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("GS1_DataBar"));
         break;
      case M_EAN8:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("EAN8"));
         break;
      case M_MICROPDF417:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("MicroPDF417"));
         break;
      case M_COMPOSITECODE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("CompositeCode"));
         break;
      case M_GS1_128:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("GS1_128"));
         break;
      case M_QRCODE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("QRCode"));
         break;
      case M_MICROQRCODE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("MicroQRCode"));
         break;
      case M_CODE93:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Code93"));
         break;
      case M_TRUNCATED_PDF417:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Truncated_PDF417"));
         break;
      case M_4_STATE:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("4-State"));
         break;
      case M_EAN14:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("EAN14"));
         break;
      case M_INDUSTRIAL25:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Industrial25"));
         break;
      case M_AZTEC:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Aztec"));
         break;

      default:
         MosSprintf(CodeTypeName, CONTROL_LENGTH_MAX, MIL_TEXT("Unknown"));
         break;
      }
   }
