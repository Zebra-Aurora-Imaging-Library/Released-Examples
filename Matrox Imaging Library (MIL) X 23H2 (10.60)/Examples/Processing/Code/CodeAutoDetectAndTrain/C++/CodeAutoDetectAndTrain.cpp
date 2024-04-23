/***************************************************************************************/
/*
* File name: CodeAutoDetectAndTrain.cpp
*
* Synopsis:  This program automatically detects code types and trains a code reader context.
*            It also compares the speed performance of a context using the default settings
*            of an Improved Recognition context versus a trained context.
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
             MIL_TEXT("CodeAutoDetectAndTrain\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program automatically detects code types and trains a code reader context.\n")
             MIL_TEXT("It also compares the speed performance of a context using the default settings\n")
             MIL_TEXT("of an Improved Recognition context versus a trained context.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));
}

// Util constants.
const MIL_INT ImageNumber = 4;

static const MIL_STRING ImageFilename[ImageNumber] =
   {
   M_IMAGE_PATH MIL_TEXT("CodeAutoDetectAndTrain/4codes_DiffOrient_90.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeAutoDetectAndTrain/4codes_DiffOrient_135.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeAutoDetectAndTrain/4codes_DiffOrient_180.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeAutoDetectAndTrain/4codes_DiffOrient_225.mim")
};

// The number of expected barcodes in each image.
static const MIL_INT ImageNbOfCodes[ImageNumber] =
   {
   4,
   4,
   4,
   4
   };

// Function declarations.
void CodeAutoDetect(const MIL_STRING SrcFilename,
    const MIL_INT NbCodePerFile[],
    MIL_ID MilCodeContext,
    MIL_ID MilSystem,
    MIL_ID MilDisplay);

void CodeTrain(const MIL_INT NumberOfImage,
    const MIL_STRING SrcFilename[],
    MIL_ID MilCodeContext,
    MIL_ID MilSystem,
    MIL_ID MilDisplay);

// Utility sub-functions declarations.
void AllocDisplayImage(MIL_ID MilSystem,
                       MIL_ID MilSrcImage,
                       MIL_ID MilDisplay,
                       MIL_ID& MilDispProcImage,
                       MIL_ID& MilOverlayImage);

void ReadAndBenchImages(const MIL_INT NumberOfImage,
                        const MIL_STRING SrcFilename[],
                        const MIL_INT NbCodePerFile[],
                        MIL_ID MilCodeContext,
                        MIL_ID MilSystem,
                        MIL_ID MilDisplay,
                        const MIL_STRING Title,
                        bool DisplayBench,
                        std::vector<MIL_DOUBLE> &BenchPerImage);


//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Allocate a code context
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   std::vector<MIL_DOUBLE> ImprovedRecBench;
   std::vector<MIL_DOUBLE> TrainedCtxBench;
   ImprovedRecBench.resize(ImageNumber);
   TrainedCtxBench.resize(ImageNumber);

   // Print Header.
   PrintHeader();

   // Auto detect code type using the first image and allocate code context.
   CodeAutoDetect(ImageFilename[0], ImageNbOfCodes, MilCodeContext, MilSystem, MilDisplay);

   //Read with improved recognition context.
   MosPrintf(MIL_TEXT("Read using the default settings of an Improved Recognition context:\n")
             MIL_TEXT("================================================================\n\n"));

   const MIL_STRING TitleImprovedRec= MIL_TEXT("IMPROVED RECOGNITION CONTEXT");
   McodeControl(MilCodeContext, M_NUMBER, M_ALL);
   McodeControl(MilCodeContext, M_INITIALIZATION_MODE, M_IMPROVED_RECOGNITION);
   ReadAndBenchImages(ImageNumber,
                      ImageFilename,
                      ImageNbOfCodes,
                      MilCodeContext,
                      MilSystem,
                      MilDisplay,
                      TitleImprovedRec,
                      true,
                      ImprovedRecBench);

   // Train after the auto Detect.
   CodeTrain(ImageNumber, ImageFilename, MilCodeContext, MilSystem, MilDisplay);

   // Read with trained context.
   MosPrintf(MIL_TEXT("Reading codes using the newly trained context:\n")
            MIL_TEXT("==============================================\n\n"));
   const MIL_STRING TitleTrained = MIL_TEXT("TRAINED CONTEXT");
   ReadAndBenchImages(ImageNumber,
                      ImageFilename,
                      ImageNbOfCodes,
                      MilCodeContext,
                      MilSystem,
                      MilDisplay,
                      TitleTrained,
                      true,
                      TrainedCtxBench);

   // Display bench comparison
   MosPrintf(MIL_TEXT("\n******************************\n"));
   MosPrintf(MIL_TEXT("Bench Comparison\n"));
   MosPrintf(MIL_TEXT("******************************\n"));
   MosPrintf(MIL_TEXT("ImageNum  ImprovedRec Bench(s)  Trained Ctx Bench(s)\t  Diff(s)\tDiff(%%)\n"));
   for(MIL_INT ii = 0; ii < ImageNumber; ii++)
      {
      MIL_DOUBLE DiffAbs = TrainedCtxBench[ii] - ImprovedRecBench[ii];
      MIL_DOUBLE DiffPercent = DiffAbs / ImprovedRecBench[ii];
      DiffPercent *= 100;
      MosPrintf(MIL_TEXT("%d\t\t%f\t\t%f\t%f\t%2.2f\n"),ii, ImprovedRecBench[ii], TrainedCtxBench[ii], DiffAbs, DiffPercent);
      }
   MosPrintf(MIL_TEXT("Press <Enter> to finish.\n\n"));
   MosGetch();

   // Release the allocated objects.
   McodeFree(MilCodeContext);

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

// Automatically detect the code types using the first image.
void CodeAutoDetect(const MIL_STRING SrcFilename,
                    const MIL_INT NbCodePerFile[],
                    MIL_ID MilCodeContext,
                    MIL_ID MilSystem,
                    MIL_ID MilDisplay)
   {
   // Allocate a code result.
   MIL_ID DetectResult = McodeAllocResult(MilSystem, M_CODE_DETECT_RESULT, M_NULL);

   MosPrintf(MIL_TEXT("Detecting code type automatically:\n")
             MIL_TEXT("===================================\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);
   MosPrintf(MIL_TEXT("Image 0\n\n"));

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
   MgraText(M_DEFAULT, MilOverlayImage, 5, 5, MIL_TEXT("CODE TYPE DETECTION"));

   MosPrintf(MIL_TEXT("Detecting code type...Done!\n\n"));
   // Detect the code type present in the image
   McodeDetect(MilSrcImage, 0, M_NULL, NbCodePerFile[0], M_DEFAULT, M_DEFAULT, DetectResult);

   // Retrieve number of detected code type
   MIL_INT NbCodeType = 0;
   McodeGetResult(DetectResult, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbCodeType);
   MosPrintf(MIL_TEXT("%i barcodes detected on %i expected barcodes\n\n"), NbCodeType, NbCodePerFile[0]);

   if(NbCodeType > 0)
      {
      MIL_TEXT_CHAR *CodeTypeString;
      for(MIL_INT jj = 0; jj < NbCodeType; jj++)
         {
         // Draw the detect code boxe.
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         McodeDraw(M_DEFAULT, DetectResult, MilOverlayImage, M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);

         // Fetch the detected code type string to output it.
         MIL_INT CodeTypeStringSize;
         McodeGetResult(DetectResult, jj, M_GENERAL, M_CODE_TYPE_NAME + M_STRING_SIZE + M_TYPE_MIL_INT, &CodeTypeStringSize);
         CodeTypeString = new MIL_TEXT_CHAR[CodeTypeStringSize];
         McodeGetResult(DetectResult, jj, M_GENERAL, M_CODE_TYPE_NAME + M_TYPE_STRING, CodeTypeString);
         MosPrintf(MIL_TEXT("Type detected :  %s\n"), CodeTypeString);

         // Fetect the detected code position.
         double DrawPosX, DrawPosY;
         McodeGetResult(DetectResult, jj, M_GENERAL, M_BOTTOM_LEFT_X, &DrawPosX);
         McodeGetResult(DetectResult, jj, M_GENERAL, M_BOTTOM_LEFT_Y, &DrawPosY);
         DrawPosY += 10;

         // Annotate the code type under the code detected.
         MgraColor(M_DEFAULT, M_COLOR_CYAN);
         MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
         MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
         MgraText(M_DEFAULT, MilOverlayImage, DrawPosX, DrawPosY, CodeTypeString);

         delete[]CodeTypeString;
         }

         // Reset the context from the detected results.
         McodeModel(MilCodeContext, M_RESET_FROM_DETECTED_RESULTS, M_NULL, M_ALL, DetectResult, M_NULL);
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(DetectResult);
   }

// Train the context using all 4 images.
void CodeTrain(const MIL_INT NumberOfImage,
               const MIL_STRING SrcFilename[],
               MIL_ID MilCodeContext,
               MIL_ID MilSystem,
               MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("Training section:\n")
            MIL_TEXT("=================\n\n"));

   MosPrintf(MIL_TEXT("Training in progress using %d sample images..."), NumberOfImage);
   MIL_ID MilCodeTrainResult = McodeAllocResult(MilSystem, M_CODE_TRAIN_RESULT, M_NULL);
   std::vector<MIL_ID> MilSrcImage(NumberOfImage, M_NULL);

   for(MIL_INT ii = 0; ii < NumberOfImage; ii++)
      {
      // Restore the image.
      MilSrcImage[ii] = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);
      }

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage[0], MilDisplay, MilDispProcImage, MilOverlayImage);
   //Display training status
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
   MgraText(M_DEFAULT, MilOverlayImage, 5, 5, MIL_TEXT("Training Context..."));

   // Set proper controls for train.
   McodeControl(MilCodeContext, M_SET_TRAINING_STATE_ALL, M_ENABLE);
   McodeControl(MilCodeContext, M_NUMBER, M_ALL);
   McodeControl(MilCodeContext, M_POSITION_ACCURACY, M_HIGH);
   McodeControl(MilCodeContext, M_TIMEOUT, M_DISABLE);

   // Train the context.
   McodeTrain(MilCodeContext, NumberOfImage, &MilSrcImage.front(), M_DEFAULT, MilCodeTrainResult);

   MosPrintf(MIL_TEXT("completed.\n\n"));
   MgraText(M_DEFAULT, MilOverlayImage, 5, 5, MIL_TEXT("Training completed!"));

   // Get statics results from train then output them.
   MIL_DOUBLE TrainingScore;
   MIL_INT NbFail, NbPass, NbTrainingImages;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_NUMBER_OF_TRAINING_IMAGES + M_TYPE_MIL_INT, &NbTrainingImages);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAINING_SCORE, &TrainingScore);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_PASSED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbPass);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_FAILED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbFail);

   MIL_TEXT_CHAR NumberString[50];
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MosSprintf(NumberString, 50, MIL_TEXT("#PASS trained image: %d"), NbPass);
   MgraText(M_DEFAULT, MilOverlayImage, 5, 25, NumberString);

   MosSprintf(NumberString, 50, MIL_TEXT("#FAIL trained image:  "));
   MgraText(M_DEFAULT, MilOverlayImage, 5, 45, NumberString);
   if (NbFail)
       MgraColor(M_DEFAULT, M_COLOR_RED);
   MosSprintf(NumberString, 50, MIL_TEXT("%d"), NbFail);
   MgraText(M_DEFAULT, MilOverlayImage, 175, 45, NumberString);

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

   std::vector<MIL_ID> MilCodeResult(NbTrainingImages, M_NULL);

   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_CODE_RESULT_ID + M_TYPE_MIL_ID, &MilCodeResult.front());

   MIL_INT NbCodeModel = 0;
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_NUMBER_OF_CODE_MODELS + M_TYPE_MIL_INT, &NbCodeModel);

   std::vector<MIL_ID> MilCodeModel(NbCodeModel, M_NULL);
   McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_CODE_MODEL_ID + M_TYPE_MIL_ID, &MilCodeModel.front());

   std::vector<MIL_INT> NboccurrenceByModel(NbCodeModel, 0);
   McodeGetResult(MilCodeTrainResult, M_ALL, M_GENERAL, M_CODE_MODEL_NUMBER_OF_OCCURRENCES + M_TYPE_MIL_INT, &NboccurrenceByModel.front());

   MosPrintf(MIL_TEXT("Training statistics:\n\n"));
   MosPrintf(MIL_TEXT("  - Score   : %.2f\n"), TrainingScore);
   MosPrintf(MIL_TEXT("  - Nb Pass : %d\n"), NbPass);
   MosPrintf(MIL_TEXT("  - Nb Fail : %d\n\n"), NbFail);
   MosPrintf(MIL_TEXT("  - Successfully trained images: \n    "));
   MosPrintf(MIL_TEXT("\tIndex    ID \n"));
   MosPrintf(MIL_TEXT("\t=====    == \n"));

   for(MIL_INT ii = 0; ii < NbPass; ii++)
      {
      MosPrintf(MIL_TEXT("\t  %d\t %d \n"), ListIndexImagePass[ii], ListIdImagePass[ii]);
      }

   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MosSprintf(NumberString, 50, MIL_TEXT("Training Score = %.2f"), TrainingScore);
   MgraText(M_DEFAULT, MilOverlayImage, 5, 65, NumberString);
   if(TrainingScore >= 85.0)
      {
      MgraText(M_DEFAULT, MilOverlayImage, 5, 85, MIL_TEXT("New trained context set!"));
      // Reset context used for train.
      McodeControl(MilCodeContext, M_RESET_FROM_TRAINED_RESULTS, MilCodeTrainResult);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeTrainResult);

   for(MIL_INT ii = 0; ii < NumberOfImage; ii++)
      {
      // Free Buffer allocation
      MbufFree(MilSrcImage[ii]);
      }
   }


//************************************
// Utility sub-functions definitions
//************************************

// Display the image.
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

// Read and bench each image.
void ReadAndBenchImages(const MIL_INT NumberOfImage,
                        const MIL_STRING SrcFilename[],
                        const MIL_INT NbCodePerFile[],
                        MIL_ID MilCodeContext,
                        MIL_ID MilSystem,
                        MIL_ID MilDisplay,
                        const MIL_STRING Title,
                        bool DisplayBench,
                        std::vector<MIL_DOUBLE> &BenchPerImage)
   {
   MIL_ID ReadResultId = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);
   MIL_DOUBLE Time = 0;
   for (MIL_INT ii = 0; ii < NumberOfImage; ii++)
      {
      MIL_INT LocalStatus = 0;
      MIL_INT NbDecoded = 0;
      MIL_ID MilSrcImage = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);
      // Allocate a display image.
      MIL_ID MilDispProcImage,         // Display and destination buffer.
             MilOverlayImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);
      //Display title.
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
      MgraColor(M_DEFAULT, M_COLOR_CYAN);
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraText(M_DEFAULT, MilOverlayImage, 5, 5, Title);

      MosPrintf(MIL_TEXT("Image %i\n"), ii);

      // Read and bench the image.
      MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
      McodeRead(MilCodeContext, MilSrcImage, ReadResultId);
      MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

      BenchPerImage[ii] = Time;

      // Fetch the read results and output them.
      McodeGetResult(ReadResultId, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &LocalStatus);
      McodeGetResult(ReadResultId, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbDecoded);
      MIL_TEXT_CHAR StatusString[50];
      switch (LocalStatus)
         {
         case M_STATUS_READ_OK:
              MosPrintf(MIL_TEXT("STATUS_READ_OK\n"));
              MosStrcpy(StatusString, 16, MIL_TEXT("STATUS: READ OK"));
              MgraColor(M_DEFAULT, M_COLOR_GREEN);
              for (MIL_INT jj = 0; jj < NbDecoded; jj++)
                 {
                 McodeDraw(M_DEFAULT, ReadResultId, MilOverlayImage, M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);
                 }
              break;
         case M_STATUS_NOT_FOUND:
              MosPrintf(MIL_TEXT("STATUS_NOT_FOUND\n"));
              MosStrcpy(StatusString, 22, MIL_TEXT("STATUS: NO CODE FOUND"));
              MgraColor(M_DEFAULT, M_COLOR_RED);
              break;
         case M_STATUS_TIMEOUT_END:
              MosPrintf(MIL_TEXT("STATUS_TIMEOUT_END\n"));
              MosStrcpy(StatusString, 20, MIL_TEXT("STATUS: TIMEOUT END"));
              MgraColor(M_DEFAULT, M_COLOR_RED);
              break;
         default:
              MgraColor(M_DEFAULT, M_COLOR_RED);
              MosPrintf(MIL_TEXT("Unrecognize status received\n"));
              MosStrcpy(StatusString, 21, MIL_TEXT("STATUS: UNRECOGNIZED"));
              break;
         } 
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraText(M_DEFAULT, MilOverlayImage, 5, 25, StatusString);
      if (NbDecoded > 0)
         {
         MgraColor(M_DEFAULT, M_COLOR_CYAN);
         MosSprintf(StatusString, 50, MIL_TEXT("%d/%d CODE(S) FOUND"), NbDecoded, NbCodePerFile[ii]);
         MgraText(M_DEFAULT, MilOverlayImage, 5, 45, StatusString);
          }

      MosPrintf(MIL_TEXT("%d occurrences found on %d present codes\n\n"), NbDecoded, NbCodePerFile[ii]);
      if (DisplayBench)
         {
         MosSprintf(StatusString, 50, MIL_TEXT("READ PROCESSING TIME : %f SEC"), BenchPerImage[ii]);
         MgraText(M_DEFAULT, MilOverlayImage, 5, 65, StatusString);
         MosPrintf(MIL_TEXT("Read Processing Time : %f sec\n"), BenchPerImage[ii]);
         }

      if (NbDecoded > 0 || ii == ImageNumber - 1)
         {
         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();
         }

      MbufFree(MilDispProcImage);
      MbufFree(MilSrcImage);
      }

   McodeFree(ReadResultId);
   }
