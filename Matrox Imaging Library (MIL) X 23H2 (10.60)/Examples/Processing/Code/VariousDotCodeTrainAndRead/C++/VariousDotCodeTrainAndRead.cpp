//***************************************************************************
//
// File name: VariousDotCodeTrainAndRead.cpp
// Location:  See Matrox Example Launcher in the MIL Control Center
//
// Synopsis:  This program trains and reads DotCodes under various conditions.
//            See the PrintHeader() function below for detailed description.
//
// Copyright (C) Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************

#include <mil.h>
#include <vector>
using std::vector;

// Example description.
void PrintHeader()
	{
	MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
		MIL_TEXT("VariousDotCodeTrainAndRead\n\n")

		MIL_TEXT("[SYNOPSIS]\n")
		MIL_TEXT("This program trains and reads DotCodes under various conditions.\n")
		
		MIL_TEXT("[MODULES USED]\n")
		MIL_TEXT("Modules used: application, system, display, buffer, graphic,\n")
		MIL_TEXT("code.\n\n"));

	MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
	MosGetch();
	}

const MIL_INT	 STRING_LENGTH_MAX = 50;
const MIL_DOUBLE MinimumAcceptance = 85.0;

const MIL_INT ImageNumber = 7;
static const MIL_STRING ImageFilename[ImageNumber] =
   {
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_1.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_2.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_3.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_4.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_5.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_6.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousDotCodeTrainandRead/DotCode_7.mim"),
   };

static  MIL_CONST_TEXT_PTR CodeDescription[7] =
   { 
   MIL_TEXT("Horizontal DotCode"),
   MIL_TEXT("Flipped DotCode"), 
   MIL_TEXT("DotCode with non-uniform lighting"),
   MIL_TEXT("Rotated DotCode"),
   MIL_TEXT("Blurred DotCode"),
   MIL_TEXT("DotCode with an aspect ratio other than 1"),
   MIL_TEXT("DotCode with Gaussian noise")
   };


void CodeTrain(const MIL_INT NumberOfImage,
   const MIL_STRING SrcFilename[],
   MIL_ID MilCodeContext,
   MIL_ID MilSystem, MIL_ID MilDisplay);

void CodeReadAndBench(const MIL_INT NumberOfImage,
   const MIL_STRING SrcFilename[], 
   MIL_ID MilCodeContext,
   MIL_ID MilCodeResult,
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   std::vector<MIL_DOUBLE> &BenchPerImage);

void AllocDisplayImage(MIL_ID MilSystem,
   MIL_ID MilSrcImage,
   MIL_ID MilDisplay,
   MIL_ID& MilDispProcImage,
   MIL_ID& MilOverlayImage);

int MosMain(void)
	{
	// Allocate the MIL objects.
	MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
	MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
	MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

	// Allocate a code context.
	MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_NULL);

	// Add a DotCode model.
	McodeModel(MilCodeContext, M_ADD, M_DOTCODE, M_NULL, M_DEFAULT, M_NULL);

	// Allocate a code result buffer.
	MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   std::vector<MIL_DOUBLE> ImprovedRecBench;
   std::vector<MIL_DOUBLE> TrainedCtxBench;
   ImprovedRecBench.resize(ImageNumber);
   TrainedCtxBench.resize(ImageNumber);

	// Print Header.
	PrintHeader();

	// Read the list of the code images before train.
	MosPrintf(MIL_TEXT("Decode using the Improved Recognition context before training.\n")
		MIL_TEXT("=============================================================\n\n"));
	CodeReadAndBench(ImageNumber,
					ImageFilename,
					MilCodeContext,
					MilCodeResult,
					MilSystem,
					MilDisplay,
					ImprovedRecBench);

	// Train the code reader context using the same list of images.
	CodeTrain(ImageNumber, ImageFilename, MilCodeContext, MilSystem, MilDisplay);

	// Read again using the trained context.
	MosPrintf(MIL_TEXT("Decode again using the trained context.\n")
		MIL_TEXT("=====================================\n\n"));
	CodeReadAndBench(ImageNumber,
					ImageFilename,
					MilCodeContext,
					MilCodeResult,
					MilSystem,
					MilDisplay,
					TrainedCtxBench);

   // Display bench comparison
   MosPrintf(MIL_TEXT("\n******************************\n"));
   MosPrintf(MIL_TEXT("Bench Comparison\n"));
   MosPrintf(MIL_TEXT("******************************\n"));
   MosPrintf(MIL_TEXT("ImageNum  ImprovedRec Bench(ms)  Trained Ctx Bench(ms)\tDiff(ms)\tDiff(%%)\n"));
   for (MIL_INT ii = 0; ii < ImageNumber; ii++)
      {
      MIL_DOUBLE DiffAbs = TrainedCtxBench[ii] - ImprovedRecBench[ii];
      MIL_DOUBLE DiffPercent = DiffAbs / ImprovedRecBench[ii];
      DiffPercent *= 100;
      MosPrintf(MIL_TEXT("%d\t\t%.2f\t\t\t%.2f\t\t%.2f\t\t%2.2f\n"), ii, ImprovedRecBench[ii], TrainedCtxBench[ii], DiffAbs, DiffPercent);
      }

	MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
	MosGetch();
	
	// Release the allocated objects.
	McodeFree(MilCodeResult);
	McodeFree(MilCodeContext);

	// Free other allocations.
	MdispFree(MilDisplay);
	MsysFree(MilSystem);
	MappFree(MilApplication);

	return 0;
}


// This function trains a context from a set of images.
void CodeTrain(const MIL_INT NumberOfImage,	const MIL_STRING SrcFilename[], MIL_ID MilCodeContext, MIL_ID MilSystem, MIL_ID MilDisplay)
	{
	MosPrintf(MIL_TEXT("Training a context from the same list of images.\n")
		MIL_TEXT("================================================\n\n"));

	// Allocate a code result for training.
	MIL_ID MilCodeTrainResult = McodeAllocResult(MilSystem, M_CODE_TRAIN_RESULT, M_NULL);

	// Restore the image.
	std::vector<MIL_ID> MilSrcImage(NumberOfImage, M_NULL);
	for (MIL_INT ii = 0; ii < NumberOfImage; ii++)
		MilSrcImage[ii] = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);

	// Allocate a display image and an overlay one.
	MIL_ID	MilDispProcImage, MilOverlayImage;  
	AllocDisplayImage(MilSystem, MilSrcImage[0], MilDisplay, MilDispProcImage, MilOverlayImage);

	// Display training status.
	MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
	MgraRectFill(M_DEFAULT, MilOverlayImage, 300, 200, 500, 300);
	MgraColor(M_DEFAULT, M_COLOR_CYAN);
	MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
	MosPrintf(MIL_TEXT("Training in progress using %d sample images... "), ImageNumber);
	MgraText(M_DEFAULT, MilOverlayImage, 310, 240, MIL_TEXT("Training in progress..."));

	// Set the context to train all trainable controls.
	McodeControl(MilCodeContext, M_SET_TRAINING_STATE_ALL, M_ENABLE);

	// Train the context. 
	McodeTrain(MilCodeContext, NumberOfImage, &MilSrcImage.front(), M_DEFAULT, MilCodeTrainResult);

	MosPrintf(MIL_TEXT("completed.\n\n"));
	MgraColor(M_DEFAULT, M_COLOR_DARK_BLUE);
	MgraRectFill(M_DEFAULT, MilOverlayImage, 300, 200, 500, 300);
	MgraColor(M_DEFAULT, M_COLOR_CYAN);
	MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
	MgraText(M_DEFAULT, MilOverlayImage, 310, 205, MIL_TEXT("Training completed."));

	// Retrieve training statistics.
	MIL_DOUBLE TrainingScore;
	MIL_INT NbFail, NbPass;
	McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_TRAINING_SCORE, &TrainingScore);
	McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_PASSED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbPass);
	McodeGetResult(MilCodeTrainResult, M_GENERAL, M_GENERAL, M_FAILED_NUMBER_OF_IMAGES + M_TYPE_MIL_INT, &NbFail);

	MIL_TEXT_CHAR NumberString[STRING_LENGTH_MAX];
	MgraColor(M_DEFAULT, M_COLOR_GREEN);
	MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("#PASS trained images: %d"), NbPass);
	MgraText(M_DEFAULT, MilOverlayImage, 310, 240, NumberString);
	MgraColor(M_DEFAULT, M_COLOR_RED);
	MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("#FAIL trained images: %d"), NbFail);
	MgraText(M_DEFAULT, MilOverlayImage, 310, 260, NumberString);
	MgraColor(M_DEFAULT, M_COLOR_CYAN);
	MosSprintf(NumberString, STRING_LENGTH_MAX, MIL_TEXT("Training score = %.2f"), TrainingScore);
	MgraText(M_DEFAULT, MilOverlayImage, 310, 280, NumberString);

	// Validating the training score. 
	if (TrainingScore >= MinimumAcceptance)
		{
		// Output that the training has been successfully done.
		MosPrintf(MIL_TEXT("Training has been successfully done and the context will be reset\n")
			MIL_TEXT("using the result of the training.\n\n"));

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

	for (MIL_INT ii = 0; ii < NumberOfImage; ii++)
		{
		// Free Buffer allocation.
		MbufFree(MilSrcImage[ii]);
		}
	}

// Read using the default Improved Recognition Context then read using the trained one.
void CodeReadAndBench(const MIL_INT NumberOfImage,
                      const MIL_STRING SrcFilename[],
                      MIL_ID MilCodeContext,
                      MIL_ID MilCodeResult,
                      MIL_ID MilSystem,
                      MIL_ID MilDisplay,
                      std::vector<MIL_DOUBLE> &BenchPerImage)
	{
	MIL_DOUBLE Time = 0;

	// Array of characters read.
	MIL_TEXT_CHAR DecodedString[STRING_LENGTH_MAX]; 

	for (MIL_INT ii = 0; ii < NumberOfImage; ii++)
		{
		MIL_ID	MilSrcImage;
		MIL_INT DecodeStatus = 0;

		// Restore the image.
		MilSrcImage = MbufRestore(SrcFilename[ii], MilSystem, M_NULL);

		// Allocate a display image and an overlay image.
		MIL_ID	MilDispProcImage, MilOverlayImage;
		AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

		// Reset time for benchmark.
		MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);
		
		// Read the code image one at a time.
		McodeRead(MilCodeContext, MilSrcImage, MilCodeResult);

		// Retrieve the decoding time and accumulate the total decoding time.
		MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);
	    BenchPerImage[ii] = Time * 1000;

		McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS + M_TYPE_MIL_INT, &DecodeStatus);

		// Output the image index and the type of the image.
		MosPrintf(MIL_TEXT("Image %d: %s\n\n"), ii, CodeDescription[ii]);
	
		if (DecodeStatus == M_STATUS_READ_OK)
			{
			// Check if the string must be formatted as GS1 human readable.
			MIL_DOUBLE IsGs1;
			McodeGetResult(MilCodeResult, 0, M_GENERAL, M_IS_GS1, &IsGs1);
			if (IsGs1)
				McodeControl(MilCodeResult, M_STRING_FORMAT, M_GS1_HUMAN_READABLE);
			else
				McodeControl(MilCodeResult, M_STRING_FORMAT, M_DEFAULT);

			// Get decoded string.
			McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, DecodedString);

			// Output the string in the console.
			MosPrintf(MIL_TEXT("The DotCode was decoded: %s\n"), DecodedString);

			// Draw the box of the code on the overlay.
			MgraColor(M_DEFAULT, M_COLOR_GREEN);
			McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, 0, M_DEFAULT);
			}
		else
			{
			MosPrintf(MIL_TEXT("Decoding failed!\n"));
			}

		MosPrintf(MIL_TEXT("The decoding time is %.2f msec.\n"), BenchPerImage[ii]);

		MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
		MosGetch();

		MbufFree(MilDispProcImage);
		MbufFree(MilSrcImage);
		}
	}

// Function to allocate a display image and an overlay one.
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
	MbufAlloc2d(MilSystem, SrcSizeX, SrcSizeY, 8L + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDispProcImage);

	// Display the image.
	MbufCopy(MilSrcImage, MilDispProcImage);
	MdispSelect(MilDisplay, MilDispProcImage);

	// Prepare for overlay annotations.
	MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
	MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
	MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
	}



