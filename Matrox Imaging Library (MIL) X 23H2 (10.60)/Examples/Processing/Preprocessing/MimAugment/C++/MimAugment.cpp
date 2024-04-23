/////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: MimAugment.cpp
//
// Synopsis:  This program shows different ways to use MimAugment 
//            to augment an image.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#include <map>
#include <iostream>

// Target MIL image specifications.
#define IMAGE_FILE M_IMAGE_PATH MIL_TEXT("BaboonRGB.mim")

// Utility constants.
#define NUMBER_OF_AUGMENTED_IMAGES 6
#define SEED_VALUE 0

//****************************************************************************
// MimAugment declarations.
//****************************************************************************
void Example1(MIL_ID MilSysId, MIL_ID MilDisplayId, MIL_ID MilSrcImage);
void Example2(MIL_ID MilSysId, MIL_ID MilDisplayId, MIL_ID MilSrcImage);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("MimAugment\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to augment an image.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, image processing.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_UNIQUE_APP_ID  MilApplication;  // Application identifier
   MIL_UNIQUE_SYS_ID  MilSystem;       // System identifier.    
   MIL_UNIQUE_DISP_ID MilDisplay;      // Display identifier.   

   // Allocate defaults.
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_UNIQUE_ID);

   PrintHeader();

   MosPrintf(MIL_TEXT("The displayed image will be used as the source for all examples.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   auto MilSrcImage = MbufRestore(IMAGE_FILE, MilSystem, M_UNIQUE_ID);
   MdispSelect(MilDisplay, MilSrcImage);
   MosGetch();

   // Run examples.
   Example1(MilSystem, MilDisplay, MilSrcImage);
   Example2(MilSystem, MilDisplay, MilSrcImage);

   return 0;
   }

void Example1(MIL_ID MilSysId,
              MIL_ID MilDisplayId,
              MIL_ID MilSrcImage)
   {
   MosPrintf(MIL_TEXT("\nEXAMPLE 1:\n"));
   MosPrintf(MIL_TEXT("----------\n"));
   MosPrintf(MIL_TEXT("This example shows how to create an augmented image with a few enabled\noperations.\n"));
   MosPrintf(MIL_TEXT("The enabled operations are: \n"));
   MosPrintf(MIL_TEXT("- Saturation\n"));
   MosPrintf(MIL_TEXT("- Gaussian Blur\n"));
   MosPrintf(MIL_TEXT("- Flip\n"));
   MosPrintf(MIL_TEXT("- Salt And Pepper Noise\n"));

   // Allocate an augmentation context.
   auto TheContext = MimAlloc(MilSysId, M_AUGMENTATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // ----------- Setup the context with the chosen operations: -----------------
   // For each operation:
   //    1) Enable the operation.
   //    2) Set a value to a setting related to that operation.

   // Intensity: Saturation.
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP, M_ENABLE); // Operation
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP_MIN, 0.5);  // Setting
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP_MAX, 0.7);  // Setting

   // LinearFilter: Gaussian Blur.
   MimControl(TheContext, M_AUG_SMOOTH_GAUSSIAN_OP, M_ENABLE); 
   MimControl(TheContext, M_AUG_SMOOTH_GAUSSIAN_OP_STDDEV_MIN, 2.0);
   MimControl(TheContext, M_AUG_SMOOTH_GAUSSIAN_OP_STDDEV_MAX, 10.0);

   // Geometric: Flip.
   MimControl(TheContext, M_AUG_FLIP_OP, M_ENABLE);
   MimControl(TheContext, M_AUG_FLIP_OP_DIRECTION, M_BOTH);

   // Noise: Salt And Pepper.
   MimControl(TheContext, M_AUG_NOISE_SALT_PEPPER_OP, M_ENABLE);
   MimControl(TheContext, M_AUG_NOISE_SALT_PEPPER_OP_DENSITY, 0.1);
   
   //------- Generate a batch of images with those augmentations -------
   MosPrintf(MIL_TEXT("\nPress <Enter> to create a new augmented image.\n"));
   MosGetch();

   MIL_INT SizeX = MbufInquire(MilSrcImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilSrcImage, M_SIZE_Y, M_NULL);
   auto Canvas       = MbufClone(MilSrcImage, MilSysId, 2*SizeX, SizeY, M_DEFAULT, M_IMAGE + M_PROC + M_DISP, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
   auto AugmentedDst = MbufChildColor2d(Canvas, M_ALL_BANDS, SizeX, 0, SizeX, SizeY, M_UNIQUE_ID);
   MgraText(M_DEFAULT, Canvas, 0, 0, MIL_TEXT("Original"));
   for(MIL_INT i = 0; i < NUMBER_OF_AUGMENTED_IMAGES; i++)
      {
      MbufClear(AugmentedDst, M_COLOR_BLACK);
      MimAugment(TheContext, MilSrcImage, AugmentedDst, M_DEFAULT, M_DEFAULT);
      MosPrintf(MIL_TEXT("Image %i/%i \r"), i+1, NUMBER_OF_AUGMENTED_IMAGES);
      MgraText(M_DEFAULT, Canvas, SizeX, 0, MIL_TEXT("Augmented"));
      MdispSelect(MilDisplayId, Canvas);
      MosGetch();
      }
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n"));
   MosGetch();

   //Disable some of the operations.
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP, M_DISABLE);
   MimControl(TheContext, M_AUG_FLIP_OP, M_DISABLE);
   MimControl(TheContext, M_AUG_NOISE_SALT_PEPPER_OP, M_DISABLE);
   MimAugment(TheContext, MilSrcImage, AugmentedDst, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nAll operations are disabled except Gaussian Blur.\n"));
   MgraText(M_DEFAULT, Canvas, SizeX, 0, MIL_TEXT("Augmented"));
   MdispSelect(MilDisplayId, Canvas);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n"));
   MosGetch();
   MdispSelect(MilDisplayId, M_NULL);
   }

// ---------------------
struct AugmentationInfo
   {
   MIL_STRING Name;
   MIL_DOUBLE Value;
   std::vector<AugmentationInfo> ResultTypes;

   void Print()
      {
      if(Value == 0)
         {
         MosPrintf(MIL_TEXT("%s : no \n"), Name.c_str());
         }
      else
         {
         MosPrintf(MIL_TEXT("%s : yes \n"), Name.c_str());
         }
      for(const auto& Result : ResultTypes)
         {
         MosPrintf(MIL_TEXT("   %s : %f \n"), Result.Name.c_str(), Result.Value);
         }
      }
   };

void Example2(MIL_ID MilSysId,
              MIL_ID MilDisplayId,
              MIL_ID MilSrcImage)
   {
   MosPrintf(MIL_TEXT("\nEXAMPLE 2:\n"));
   MosPrintf(MIL_TEXT("----------\n"));
   MosPrintf(MIL_TEXT("This example shows how to use an augmentation result instead of a buffer\n"));
   MosPrintf(MIL_TEXT("image as the destination. It is possible to retrieve information about the\n"));
   MosPrintf(MIL_TEXT("augmented image, such as the applied operations and the random values chosen.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate an augmentation context.
   auto TheContext = MimAlloc(MilSysId, M_AUGMENTATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate an augmentation result.
   auto AugmentationResult = MimAllocResult(MilSysId, M_DEFAULT, M_AUGMENTATION_RESULT, M_UNIQUE_ID);

   // Allocate a canvas to display a comparison between the original image vs the augmented image.
   MIL_INT SrcSizeX = MbufInquire(MilSrcImage, M_SIZE_X, M_NULL);
   MIL_INT SrcSizeY = MbufInquire(MilSrcImage, M_SIZE_Y, M_NULL);
   auto Canvas         = MbufClone(MilSrcImage, MilSysId, 2 * SrcSizeX, SrcSizeY, M_DEFAULT, M_IMAGE + M_PROC + M_DISP, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
   auto AugmentedImage = MbufChild2d(Canvas, SrcSizeX, 0, SrcSizeX, SrcSizeY, M_UNIQUE_ID);

   // Enable ControlType (operation) to add a motion blur effect.
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP, M_ENABLE);

   // Set probability to 100.0 for the motion blur augmentation to ensure it is always applied.
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP + M_PROBABILITY, 100.0);

   // Set Range Values for the Parameters of the motion blur operation.
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP_SIZE_MIN, 5);
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP_SIZE_MAX, 15);
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP_ANGLE_MIN, 0.0);
   MimControl(TheContext, M_AUG_BLUR_MOTION_OP_ANGLE_MAX, 120.0);

   // Enable ControlType (operation) to change the saturation.
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP, M_ENABLE);
   // Adjust the probability so the saturation gain operation does not happen all the time.
   MimControl(TheContext, M_AUG_SATURATION_GAIN_OP + M_PROBABILITY, 50.0);

   // Generate multiple results. To make them repeatable, the randomness can be controlled with the seed.
   MimControl(TheContext, M_AUG_SEED_MODE, M_RNG_INIT_VALUE);
   MimControl(TheContext, M_AUG_RNG_INIT_VALUE, SEED_VALUE);

   for(MIL_INT AugmentationIdx = 0; AugmentationIdx < NUMBER_OF_AUGMENTED_IMAGES; AugmentationIdx++)
      {
      // Use a map to represent flag as string.
      std::map<MIL_INT, AugmentationInfo> AugmentMap;
      AugmentMap[M_AUG_BLUR_MOTION_OP    ].Name = MIL_TEXT("M_AUG_BLUR_MOTION_OP")    ;
      AugmentMap[M_AUG_SATURATION_GAIN_OP].Name = MIL_TEXT("M_AUG_SATURATION_GAIN_OP");
      AugmentMap[M_AUG_BLUR_MOTION_ANGLE ].Name = MIL_TEXT("BlurMotionAngle")         ;
      AugmentMap[M_AUG_BLUR_MOTION_SIZE  ].Name = MIL_TEXT("BlurMotionSize")          ;
      AugmentMap[M_AUG_SATURATION_GAIN   ].Name = MIL_TEXT("SaturationGain")          ;

      // Apply augmentations.
      MimAugment(TheContext, MilSrcImage, AugmentationResult, M_DEFAULT, M_DEFAULT);

      // Get all operations that were enabled in the context.
      std::vector<MIL_INT> OperationsEnabled;
      MimGetResult(AugmentationResult, M_AUG_OPERATIONS_ENABLED, OperationsEnabled);
      // Ex. output: OperationsEnabled = [M_AUG_BLUR_MOTION_OP, M_AUG_SATURATION_GAIN_OP].

      // Get vector of M_TRUE/M_FALSE indicating if the operations found in OperationsEnabled have been applied.
      std::vector<MIL_INT> OperationsApplied;
      MimGetResult(AugmentationResult, M_AUG_OPERATIONS_APPLIED, OperationsApplied);
      // Ex. output: OperationsApplied = [M_TRUE, M_FALSE].

      for(size_t i = 0; i < OperationsEnabled.size(); i++)
         { AugmentMap[OperationsEnabled[i]].Value = (MIL_DOUBLE) OperationsApplied[i]; }

      // Get all result types corresponding to the operations that have been applied.
      std::vector<MIL_INT> OperationResultTypes;
      MimGetResult(AugmentationResult, M_AUG_OPERATION_RESULT_TYPES, OperationResultTypes);
      // Ex. output: OperationResultTypes = [M_AUG_BLUR_MOTION_SIZE, M_AUG_BLUR_MOTION_ANGLE].

      // Get result values.
      std::vector<MIL_DOUBLE> OperationResultValues;
      MimGetResult(AugmentationResult, M_AUG_OPERATION_RESULT_VALUES, OperationResultValues);
      // Ex. output: OperationResultValues = [5, 21.55].

      // Those two vectors let you conclude that M_AUG_BLUR_MOTION_SIZE = 5 and M_AUG_BLUR_MOTION_ANGLE = 21.55.
      // However, it is also possible to make an individual call for each result type.
      MIL_DOUBLE Value;
      MimGetResult(AugmentationResult, M_AUG_BLUR_MOTION_SIZE, &Value);  // Ex. output: Value = 5.
      MimGetResult(AugmentationResult, M_AUG_BLUR_MOTION_ANGLE, &Value); // Ex. output Value = 21.55.

      for(size_t i = 0; i<OperationResultValues.size(); i++)
         { AugmentMap[OperationResultTypes[i]].Value = OperationResultValues[i]; }

      // Get the correspondence between operation and result types.
      std::vector<MIL_INT> OperationAssociatedWithResultTypes;
      MimGetResult(AugmentationResult, M_AUG_OPERATION_ASSOCIATED_WITH_RESULT_TYPES, OperationAssociatedWithResultTypes);

      // Iterate through the indices Vector to know the augmentation to which the parameter corresponds.
      for(size_t i = 0; i<OperationAssociatedWithResultTypes.size(); i++)
         {
         MIL_INT Operation  = OperationAssociatedWithResultTypes[i];
         MIL_INT ResultType = OperationResultTypes[i];
         AugmentMap[Operation].ResultTypes.push_back(AugmentMap[ResultType]);
         }

      // Print the enabled operations with their results.
      MosPrintf(MIL_TEXT("========= Augmentation Result %i/%i =========\n"), AugmentationIdx + 1, NUMBER_OF_AUGMENTED_IMAGES);
      for(const auto& Operation : OperationsEnabled)
         {
         AugmentMap[Operation].Print();
         }

      // The printed result is also available in a report.
      MIL_STRING ReportName = MIL_TEXT("AugmentationReport_") + M_TO_STRING(AugmentationIdx + 1) + MIL_TEXT(".txt");
      MimStream(ReportName, M_NULL, M_SAVE_REPORT, M_FILE, M_DEFAULT, M_DEFAULT, &AugmentationResult, M_NULL);
      MosPrintf(MIL_TEXT("-> %s\n\n"), ReportName.c_str());

      // Get the result image directly from AugmentationResult.
      MimDraw(M_DEFAULT, AugmentationResult, M_NULL, AugmentedImage, M_DRAW_AUG_IMAGE, M_NULL, M_NULL, M_DEFAULT);
      MgraText(M_DEFAULT, Canvas, 0, 0, MIL_TEXT("Original"));
      MgraText(M_DEFAULT, Canvas, SrcSizeX, 0, MIL_TEXT("Augmented"));
      MdispSelect(MilDisplayId, Canvas);
      MosGetch();
      MdispSelect(MilDisplayId, M_NULL);
      }
   }
