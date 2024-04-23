//***************************************************************************************/
// 
// File name: isodpmGrading.cpp  
//
// Synopsis:  This program contains an example of calibration and code grading  
//            for DataMatrix based on ISO/IEC 29158:2020 specifications.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("isodpmGrading\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program is an example of grading a 2D data code symbol. \n")
             MIL_TEXT("The example follows ISO/IEC 29158:2020 Quality Guidelines. The example \n")
             MIL_TEXT("also shows steps for reflectance calibration (required).\n\n")
  
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, system, display, buffer, \n")
             MIL_TEXT("graphic, code\n\n")
  
             MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

static const MIL_INT CodeType = M_DATAMATRIX;
static const MIL_INT CodeCalibrationType = M_UPC_A;

static const MIL_INT MinimumGrayScaleValue = 0;
static const MIL_INT MaximumGrayScaleValue = 255;

static const MIL_INT ApertureCalMode = M_RELATIVE;
static const double ApertureCalSize = M_AUTO;

static const MIL_INT ApertureMode = M_RELATIVE;
static const double ApertureSize = M_AUTO;

static const double MinimumMeanLight = 0.7;
static const double MaximumMeanLight = 0.86;

static const MIL_DOUBLE SRexposure = 60.0;
static const MIL_DOUBLE SRgain = 1.25;

static const MIL_INT CodeReflectanceCalibrationNumber = 7;

static const MIL_DOUBLE LightingConfiguration = M_UNSPECIFIED;

static const MIL_INT GradingStandard = M_ISO_DPM_GRADING;

// When M_GRADING_STANDARD is set to M_ISO_DPM_GRADING, the M_DEFAULT value of M_GRADING_STANDARD_EDITION is M_ISO_29158_2020.
static const MIL_INT GradingStandardEdition = M_DEFAULT;

static MIL_CONST_TEXT_PTR CodeReflectanceCalibrationFileName[CodeReflectanceCalibrationNumber] = 
   {
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration1.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration2.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration3.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration4.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration5.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration6.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/ReflectanceCalibration7.mim")
   };

const MIL_INT CodeReflectanceNumber = 3;

static MIL_CONST_TEXT_PTR CodeReflectanceFileName[CodeReflectanceNumber] =
   {
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Initial1.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Initial2.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Initial3.mim")
   };

const MIL_INT CodeSourceImageNumber = 5;

static MIL_CONST_TEXT_PTR CodeSourceImageFileName[CodeSourceImageNumber] = 
   {
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Image1.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Image2.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Image3.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Image4.mim"),
   M_IMAGE_PATH MIL_TEXT("ISODPMGrading/Image5.mim")
   };


//************************************
// functions declarations
//************************************
void ReflectanceCalibration(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename, MIL_ID MilSystem, MIL_ID MilDisplay, 
                            MIL_ID MilCodeContext, MIL_DOUBLE &SRcal);

void InitialReflectanceLevel(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename,
                             MIL_ID MilSystem, MIL_ID MilDisplay, MIL_DOUBLE &InitialSRTarget);

void ISODPMGrading(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename, MIL_ID MilSystem, MIL_ID MilDisplay, 
                   MIL_ID MilCodeContext, const MIL_DOUBLE SRcal, const MIL_DOUBLE InitialSRTarget);


//************************************
// Utility sub-functions declarations
//************************************

void AllocDisplayImage(MIL_ID MilSystem, 
                       MIL_ID MilSrcImage, 
                       MIL_ID MilDisplay, 
                       MIL_ID& MilDispProcImage, 
                       MIL_ID& MilOverlayImage);

MIL_CONST_TEXT_PTR GetGradingStandardEditionString(MIL_INT GradingStandardEdition);

void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, double  Grade);
void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, MIL_INT Grade);
void PrintGrade(MIL_CONST_TEXT_PTR Text, double  Grade);
void PrintGrade(MIL_CONST_TEXT_PTR Text, MIL_INT Grade);


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay     = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Set lighting configuration
   McodeControl(MilCodeContext, M_LIGHTING_CONFIGURATION, LightingConfiguration);

   // Print Header.
   PrintHeader();
   
   //***************************
   // Reflectance Calibration    
   //***************************
   MIL_DOUBLE SRcal = 0.0;

   if((LightingConfiguration != M_90_DEGREE) || (GradingStandardEdition == M_ISO_29158_2011))
      {
      // This procedure is not used for lighting configuration 90.
      ReflectanceCalibration(CodeReflectanceCalibrationFileName, CodeReflectanceCalibrationNumber,
                             MilSystem, MilDisplay, MilCodeContext, SRcal);
      }

   //******************************************************
   // Initial reflectance level of the symbol under test   
   //******************************************************
   MIL_DOUBLE InitialSRTarget = SRcal;
   InitialReflectanceLevel(CodeReflectanceFileName, CodeReflectanceNumber,
                           MilSystem, MilDisplay, InitialSRTarget);

   //**************************************
   // Target calibration and grading    
   //**************************************
   ISODPMGrading(CodeSourceImageFileName, CodeSourceImageNumber,
                 MilSystem, MilDisplay, MilCodeContext, SRcal, InitialSRTarget);

   // Free MIL objects. 
   McodeFree(MilCodeContext);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//***********************************************************************************
// ReflectanceCalibration:
//   Uses a high-contrast images (such as traceable printed calibration cards)
//   with a known aperture size. The system response has been adjusted so that
//   the mean of the light elements is in the range of 70% to 86% of the maximum
//   grayscale(MLcal) and the black level (no light) is nominally equal to zero.
//   This function retreives from the code result (1) MLcal: mean of the light lobe, 
//   and (2) Rcal : reported reflectance value. These results are applied to the code 
//   context used to grade, based on ISO/IEC 29158:2020 specifications. This function 
//   also records the system response as the Reference System Response (SRcal).
//***********************************************************************************
void ReflectanceCalibration(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename,
                            MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilCodeTargetContext, MIL_DOUBLE &SRcal)
   {
   MIL_INT ii;
   MIL_DOUBLE MLcal = 0.0;
   MIL_DOUBLE Rcal = 255;

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, CodeCalibrationType, M_NULL, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Specifie to use the ISO standard to have M_MEAN_LIGHT_CALIBRATION and M_REFLECTANCE_CALIBRATION computed
   McodeControl(MilCodeContext, M_GRADING_STANDARD, M_ISO_GRADING);

   // Set the aperture mode and size (optional); by default it's M_RELATIVE
   McodeControl(MilCodeContext, M_APERTURE_MODE, ApertureCalMode);

   if(ApertureCalMode == M_RELATIVE || ApertureCalMode == M_DEFAULT)
      McodeControl(MilCodeContext, M_RELATIVE_APERTURE_FACTOR, ApertureCalSize);
   else if(ApertureCalMode == M_ABSOLUTE)
      McodeControl(MilCodeContext, M_ABSOLUTE_APERTURE_SIZE, ApertureCalSize);

   // Set the calibrated reflectance values (optional); the default values are [0, 255]
   McodeControl(MilCodeContext, M_MINIMUM_CALIBRATED_REFLECTANCE, MinimumGrayScaleValue);
   McodeControl(MilCodeContext, M_MAXIMUM_CALIBRATED_REFLECTANCE, MaximumGrayScaleValue);

   bool EndCalibration = false;

   for(ii = 0; ii < NbFilename && !EndCalibration; ii++)
      {
      // Restore source image into an automatically allocated image buffer.
      MIL_ID MilSrcImage;
      MbufRestore(SrcFilename[ii], MilSystem, &MilSrcImage);

      // Allocate a display image.
      MIL_ID MilDispProcImage,         // Display and destination buffer.
             MilOverlayImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

      // Read and grade the code and display the result.
      McodeGrade(MilCodeContext, MilDispProcImage, M_NULL, M_DEFAULT, MilCodeResult, M_DEFAULT);

      // Get decoding status.
      MIL_INT ReadStatus;
      McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);

      if(ReadStatus == M_STATUS_GRADE_OK)
         {
         // Get code decodability to ensure that M_MEAN_LIGHT_CALIBRATION and M_REFLECTANCE_CALIBRATION has been computed
         MIL_INT DecodeGrade;
         McodeGetResult(MilCodeResult, 0, M_GENERAL, M_DECODE_GRADE + M_TYPE_MIL_INT, &DecodeGrade);

         if(DecodeGrade == M_CODE_GRADE_A)
            {
            // Retrieve MLcal : mean of the light lobe from a histogram of the calibrated standard
            McodeGetResult(MilCodeResult, 0, M_GENERAL, M_MEAN_LIGHT_CALIBRATION + M_TYPE_DOUBLE, &MLcal);

            // Retrieve Rcal : reported reflectance value, Rmax, from a calibration standard  
            McodeGetResult(MilCodeResult, 0, M_GENERAL, M_REFLECTANCE_CALIBRATION + M_TYPE_DOUBLE, &Rcal);

            double RatioMLcal = MLcal / (double)MaximumGrayScaleValue;

            if((RatioMLcal >= MinimumMeanLight) && (RatioMLcal <= MaximumMeanLight))
               {
               // Record SRcal : value of system response parameters (such as exposure and/or gain) used to 
               //                create an image of the calibration standard 
               SRcal = SRexposure * SRgain;
               EndCalibration = true;
               MosPrintf(MIL_TEXT(" Calibration finished successfully:\n"));
               MosPrintf(MIL_TEXT(" - Mean light lobe:     %7.3f\n"), RatioMLcal);
               MosPrintf(MIL_TEXT(" - Highest reflectance: %7.3f\n"), Rcal);
               MosPrintf(MIL_TEXT(" - System Response:     %7.3f\n\n"), SRcal);

               // Retrieve calibration reflectance from the result and pass the information to target context
               McodeControl(MilCodeTargetContext, M_DPM_CALIBRATION_RESULTS, MilCodeResult);
               }
            else if(RatioMLcal < MinimumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too low. Increase \n"), RatioMLcal);
               MosPrintf(MIL_TEXT("exposure or gain and continue to calibrate.\n\n"));
               }
            else if(RatioMLcal > MaximumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too high. Decrease \n"), RatioMLcal);
               MosPrintf(MIL_TEXT("exposure or gain and continue to calibrate.\n\n"));
               }
            }
         else
            {
            MosPrintf(MIL_TEXT("Grading operation failed to read. Verify your setting and continue to calibrate.\n\n"));
            }
         }
      else
         {
         MosPrintf(MIL_TEXT("Grading operation failed. Verify your setting and continue to calibrate.\n\n"));
         }

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Free source image.
      MbufFree(MilSrcImage);

      // Free Display image
      MbufFree(MilDispProcImage);
      }

   // Free Context and result objects
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   }


//***********************************************************************************
// InitialReflectanceLevel:
//   Uses image with the symbol under test and an aperture factor of 0.5. This 
//   function records the system response that gave the mean of light elements in the 
//   range of 70% to 86% of the maximum grayscale. This function must be called after 
//   the reflectance calibration step (ReflectanceCalibration).
//***********************************************************************************
void InitialReflectanceLevel(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename,
                            MIL_ID MilSystem, MIL_ID MilDisplay, MIL_DOUBLE &InitialSRTarget)
   {
   MIL_INT ii;
   MIL_DOUBLE MeanLight = 0.0;

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, CodeType, M_NULL, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Specifie to use the ISO standard to have M_MEAN_LIGHT_CALIBRATION
   McodeControl(MilCodeContext, M_GRADING_STANDARD, M_ISO_GRADING);

   // Set the aperture mode and size (optional); by default it's M_RELATIVE
   McodeControl(MilCodeContext, M_APERTURE_MODE, M_RELATIVE);

   if(GradingStandardEdition == M_ISO_29158_2011)
      {
      // Set the aperture factor to 0.8
      McodeControl(MilCodeContext, M_RELATIVE_APERTURE_FACTOR, 0.8);
      }
   else
      {
      // Set the aperture factor to 0.5
      McodeControl(MilCodeContext, M_RELATIVE_APERTURE_FACTOR, 0.5);
      }

   // Set the calibrated reflectance values (optional); the default values are [0, 255]
   McodeControl(MilCodeContext, M_MINIMUM_CALIBRATED_REFLECTANCE, MinimumGrayScaleValue);
   McodeControl(MilCodeContext, M_MAXIMUM_CALIBRATED_REFLECTANCE, MaximumGrayScaleValue);

   // Set foreground color
   McodeControl(MilCodeContext, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);

   // Set decode algorithm
   McodeControl(MilCodeContext, M_DECODE_ALGORITHM, M_CODE_DEFORMED);

   // Enable Presearch
   McodeControl(MilCodeContext, M_USE_PRESEARCH, M_STAT_BASE);

   bool EndInitialization = false;

   for(ii = 0; ii < NbFilename && !EndInitialization; ii++)
      {
      // Restore source image into an automatically allocated image buffer.
      MIL_ID MilSrcImage;
      MbufRestore(SrcFilename[ii], MilSystem, &MilSrcImage);

      // Allocate a display image.
      MIL_ID MilDispProcImage,         // Display and destination buffer.
             MilOverlayImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

      // Read and grade the code and display the result.
      McodeGrade(MilCodeContext, MilDispProcImage, M_NULL, M_DEFAULT, MilCodeResult, M_DEFAULT);

      // Get decoding status.
      MIL_INT ReadStatus;
      McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);

      if(ReadStatus == M_STATUS_GRADE_OK)
         {
         // Get code decodability to ensure that M_MEAN_LIGHT_CALIBRATION had been computed
         MIL_INT DecodeGrade;
         McodeGetResult(MilCodeResult, 0, M_GENERAL, M_DECODE_GRADE + M_TYPE_MIL_INT, &DecodeGrade);

         if(DecodeGrade == M_CODE_GRADE_A)
            {
            // Retrieve mean of the light lobe from a histogram of the symbol under test
            McodeGetResult(MilCodeResult, 0, M_GENERAL, M_MEAN_LIGHT_CALIBRATION + M_TYPE_DOUBLE, &MeanLight);

            double RatioMeanLight = MeanLight / (double)MaximumGrayScaleValue;

            if((RatioMeanLight >= MinimumMeanLight) && (RatioMeanLight <= MaximumMeanLight))
               {
               // Record the system response (such as exposure or gain)
               InitialSRTarget = SRexposure * SRgain;
               EndInitialization = true;
               MosPrintf(MIL_TEXT(" Initial image reflectance level of the symbol under test finished\n successfully:\n"));
               MosPrintf(MIL_TEXT(" - Mean light lobe:     %7.3f\n"), RatioMeanLight);
               MosPrintf(MIL_TEXT(" - System Response:     %7.3f\n\n"), InitialSRTarget);
               }
            else if(RatioMeanLight < MinimumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too low. Increase \n"), RatioMeanLight);
               MosPrintf(MIL_TEXT("exposure or gain and continue to adjust the system response.\n\n"));
               }
            else if(RatioMeanLight > MaximumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too high. Decrease \n"), RatioMeanLight);
               MosPrintf(MIL_TEXT("exposure or gain and continue to adjust the system response.\n\n"));
               }
            }
         else
            {
            MosPrintf(MIL_TEXT("Grading operation failed to read. Verify your setting and continue to get\n"));
            MosPrintf(MIL_TEXT("initial reflectance level of the symbol under test.\n\n"));
            }
         }
      else
         {
         MosPrintf(MIL_TEXT("Grading operation failed. Verify your setting and continue to get\n"));
         MosPrintf(MIL_TEXT("initial reflectance level of the symbol under test.\n\n"));
         }

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Free source image.
      MbufFree(MilSrcImage);

      // Free Display image
      MbufFree(MilDispProcImage);
      }

   // Free Context and result objects
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   }

//***********************************************************************************
// ISODPMGrading :
//   This function performs code grading for DataMatrix based on ISO/IEC 29158:2020 
//   specifications. This function must be called after the reflectance calibration
//   step (ReflectanceCalibration) and after the initial image reflectance level of 
//   the symbol under test step (InitialReflectanceLevel).
//***********************************************************************************
void ISODPMGrading(MIL_CONST_TEXT_PTR SrcFilename[], MIL_INT NbFilename,
                   MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilCodeContext, const MIL_DOUBLE SRcal, const MIL_DOUBLE InitialSRTarget)
   {
   MIL_INT ii;

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, CodeType, M_NULL, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Set grading standard 
   McodeControl(MilCodeContext, M_GRADING_STANDARD, GradingStandard);

   // Set grading standard edition
   McodeControl(MilCodeModel, M_GRADING_STANDARD_EDITION, GradingStandardEdition);

   // Set the aperture mode and size (optional); by default it's M_RELATIVE 
   McodeControl(MilCodeContext, M_APERTURE_MODE, ApertureMode);

   if(ApertureMode == M_RELATIVE || ApertureMode == M_DEFAULT)
      McodeControl(MilCodeContext, M_RELATIVE_APERTURE_FACTOR, ApertureSize);
   else if(ApertureMode == M_ABSOLUTE)
      McodeControl(MilCodeContext, M_ABSOLUTE_APERTURE_SIZE, ApertureSize);

   // Set the calibrated reflectance values (optional); the default values are [0, 255]
   McodeControl(MilCodeContext, M_MINIMUM_CALIBRATED_REFLECTANCE, MinimumGrayScaleValue);
   McodeControl(MilCodeContext, M_MAXIMUM_CALIBRATED_REFLECTANCE, MaximumGrayScaleValue);

   // Set SRcal : value of system response parameters (such as exposure and/or gain) used to 
   //             create an image of the calibration standard
   McodeControl(MilCodeContext, M_SYSTEM_RESPONSE_CALIBRATION, SRcal);

   // Set foreground color
   McodeControl(MilCodeContext, M_FOREGROUND_VALUE, M_FOREGROUND_WHITE);

   // Set decode algorithm
   McodeControl(MilCodeContext, M_DECODE_ALGORITHM, M_CODE_DEFORMED);

   // Enable Presearch
   McodeControl(MilCodeContext, M_USE_PRESEARCH, M_STAT_BASE);


   bool EndAdjust = false;
   MIL_DOUBLE SRtarget = InitialSRTarget;
   MIL_DOUBLE MLtarget;

   for(ii = 0; ii < NbFilename && !EndAdjust; ii++)
      {
      // Restore source image into an automatically allocated image buffer.
      MIL_ID MilSrcImage;
      MbufRestore(SrcFilename[ii], MilSystem, &MilSrcImage);

      // Allocate a display image.
      MIL_ID MilDispProcImage,         // Display and destination buffer.
             MilOverlayImage;          // Overlay buffer.
      AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

      // Set SRtarget : value of system response parameters (such as exposure and/or gain) used to 
      //                create an image to grade
      McodeControl(MilCodeContext, M_SYSTEM_RESPONSE_TARGET, SRtarget);

      // Read the code and display the result.
      McodeGrade(MilCodeContext, MilDispProcImage, M_NULL, M_DEFAULT, MilCodeResult, M_DEFAULT);

      // Get decoding status.
      MIL_INT ReadStatus;
      McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);

      if(ReadStatus == M_STATUS_GRADE_OK)
         {
         // Get code decodability to ensure that M_MEAN_LIGHT_TARGET had been computed
         MIL_INT DecodeGrade;
         McodeGetResult(MilCodeResult, 0, M_GENERAL, M_DECODE_GRADE + M_TYPE_MIL_INT, &DecodeGrade);

         if(DecodeGrade == M_CODE_GRADE_A)
            {
            // Retrieve Mean of light elements
            McodeGetResult(MilCodeResult, 0, M_GENERAL, M_MEAN_LIGHT_TARGET, &MLtarget);

            double RatioMLtarget = MLtarget / (double)MaximumGrayScaleValue;

            if((RatioMLtarget >= MinimumMeanLight) && (RatioMLtarget <= MaximumMeanLight))
               {
               // Record the system response (such as exposure or gain)
               SRtarget = SRexposure * SRgain;
               EndAdjust = true;
               MosPrintf(MIL_TEXT(" Grading finished successfully:\n"));
               MosPrintf(MIL_TEXT(" - Mean light lobe: %7.3f\n"), RatioMLtarget);
               MosPrintf(MIL_TEXT(" - System Response: %7.3f\n\n"), SRtarget);
               }
            else if(RatioMLtarget < MinimumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too low. Increase \n"), RatioMLtarget);
               MosPrintf(MIL_TEXT("exposure or gain and continue to adjust the system response.\n\n"));
               }
            else if(RatioMLtarget > MaximumMeanLight)
               {
               MosPrintf(MIL_TEXT("Mean light lobe is %7.3f. This is too high. Decrease \n"), RatioMLtarget);
               MosPrintf(MIL_TEXT("exposure or gain and continue to adjust the system response.\n\n"));
               }
            }
         else
            {
            MosPrintf(MIL_TEXT("Grading operation failed to read. Verify your setting and continue to adjust the system response.\n\n"));
            }
         }
      else
         {
         MosPrintf(MIL_TEXT("Grading operation failed. Verify your setting and continue to adjust the system response.\n\n"));
         }

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Free source image.
      MbufFree(MilSrcImage);

      // Free Display image
      MbufFree(MilDispProcImage);
      }

   // Retrieve and print grading result
   MIL_INT    GradingEditionStandardUsed  = M_DEFAULT;
   MIL_DOUBLE OverallGrade                = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE DecodeGrade                 = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE CellContrast                = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE CellContrastGrade           = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE CellModulationGrade         = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE MinimumReflectance          = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE MinimumReflectanceGrade     = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE UnusedErrorCorrection       = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE UnusedErrorCorrectionGrade  = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE AxialNonUniformity          = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE AxialNonUniformityGrade     = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE GridNonUniformity           = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE GridNonUniformityGrade      = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE FixedPatternDamageGrade     = M_CODE_GRADE_NOT_AVAILABLE;
   MIL_DOUBLE PrintGrowth                 = M_CODE_GRADE_NOT_AVAILABLE;

   // Retrieve Results
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_GRADING_STANDARD_EDITION_USED + M_TYPE_MIL_INT,    &GradingEditionStandardUsed);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_OVERALL_SYMBOL_GRADE,             &OverallGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_DECODE_GRADE,                     &DecodeGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_CELL_CONTRAST,                    &CellContrast);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_CELL_CONTRAST_GRADE,              &CellContrastGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_CELL_MODULATION_GRADE,            &CellModulationGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_MINIMUM_REFLECTANCE,              &MinimumReflectance);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_MINIMUM_REFLECTANCE_GRADE,        &MinimumReflectanceGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_UNUSED_ERROR_CORRECTION,          &UnusedErrorCorrection);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_UNUSED_ERROR_CORRECTION_GRADE,    &UnusedErrorCorrectionGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_AXIAL_NONUNIFORMITY,              &AxialNonUniformity);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_AXIAL_NONUNIFORMITY_GRADE,        &AxialNonUniformityGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_GRID_NONUNIFORMITY,               &GridNonUniformity);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_GRID_NONUNIFORMITY_GRADE,         &GridNonUniformityGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_FIXED_PATTERN_DAMAGE_GRADE,       &FixedPatternDamageGrade);
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_PRINT_GROWTH,                     &PrintGrowth);

   // Print Results
   MosPrintf(MIL_TEXT("Grading Standard Used:   %s\n"), GetGradingStandardEditionString(GradingEditionStandardUsed));

   PrintGrade(MIL_TEXT(" - Overall Grade:                "), OverallGrade);
   PrintGrade(MIL_TEXT(" - Decode Grade:                 "), DecodeGrade);
   PrintValueAndGrade(MIL_TEXT(" - Cell Contrast:              "), CellContrast, CellContrastGrade);
   PrintGrade(MIL_TEXT(" - Cell Modulation Grade:        "), CellModulationGrade);
   PrintValueAndGrade(MIL_TEXT(" - Minimum Reflectance:        "), MinimumReflectance, MinimumReflectanceGrade);
   PrintValueAndGrade(MIL_TEXT(" - Unused Error Correction:    "), UnusedErrorCorrection, UnusedErrorCorrectionGrade);
   PrintValueAndGrade(MIL_TEXT(" - Axial Non-Uniformity:       "), AxialNonUniformity, AxialNonUniformityGrade);
   PrintValueAndGrade(MIL_TEXT(" - Grid Non-Uniformity:        "), GridNonUniformity, GridNonUniformityGrade);
   PrintGrade(MIL_TEXT(" - Fixed Pattern Damage Grade:   "), FixedPatternDamageGrade);
   MosPrintf(MIL_TEXT(" - Print Growth:               %7.3f\n\n"), PrintGrowth);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free Context and result objects
   McodeFree(MilCodeResult);
   }


/************************************************************************
GetGradeString
************************************************************************/
MIL_CONST_TEXT_PTR GetGradeString(double Grade)
   {
   switch(static_cast<MIL_INT>(Grade + 0.5))
      {
      case M_CODE_GRADE_A:             return MIL_TEXT("A");  break;
      case M_CODE_GRADE_B:             return MIL_TEXT("B");   break;
      case M_CODE_GRADE_C:             return MIL_TEXT("C");   break;
      case M_CODE_GRADE_D:             return MIL_TEXT("D");   break;
      case M_CODE_GRADE_F:             return MIL_TEXT("F");   break;
      case M_CODE_GRADE_NOT_AVAILABLE: return MIL_TEXT("N/A"); break;
      }
   return MIL_TEXT("N/A");
   }

/************************************************************************
GetGradingStandardEditionString
************************************************************************/
MIL_CONST_TEXT_PTR GetGradingStandardEditionString(MIL_INT GradingStandardEdition)
   {
   switch(GradingStandardEdition)
      {
      case M_ISO_15416_2000:                return MIL_TEXT("ISO/IEC 15416:2000");                              break;
      case M_ISO_15416_2016:                return MIL_TEXT("ISO/IEC 15416:2016");                              break;
      case M_ISO_15415_2011_15416_2000:     return MIL_TEXT("ISO/IEC 15415:2011 & ISO/IEC 15416:2000");         break;
      case M_ISO_15415_2011_15416_2016:     return MIL_TEXT("ISO/IEC 15415:2011 & ISO/IEC 15416:2016");         break;
      case M_ISO_29158_2011:                return MIL_TEXT("ISO/IEC TR 29158:2011");                           break;
      case M_ISO_29158_2020:                return MIL_TEXT("ISO/IEC 29158:2020");                              break;
      case M_SEMI_T10_0701:                 return MIL_TEXT("SEMI T10-0701");                                   break;
      default:                              return MIL_TEXT("Unavailable");                                     break;
      }

   }

/************************************************************************
PrintGrade
************************************************************************/
void PrintGrade(MIL_CONST_TEXT_PTR Text, double Grade)
   {
   if (Grade != M_CODE_GRADE_NOT_AVAILABLE)
      MosPrintf(MIL_TEXT("%sN/A    [ Grade: %.1f (%s)]\n"), Text, Grade, GetGradeString(Grade));
   }

void PrintGrade(MIL_CONST_TEXT_PTR Text, MIL_INT Grade)
   {
   PrintGrade(Text, static_cast<double>(Grade));
   }

/************************************************************************
PrintValueAndGrade
************************************************************************/
void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, double Grade)
   {
   if (Grade != M_CODE_GRADE_NOT_AVAILABLE)
      MosPrintf(MIL_TEXT("%s%7.3f  [ Grade: %.1f (%s)]\n"), Text, Value, Grade, GetGradeString(Grade));
   }

void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, MIL_INT Grade)
   {
   PrintValueAndGrade(Text, Value, static_cast<double>(Grade));
   }


/************************************************************************
AllocDisplayImage
************************************************************************/
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
      8L+M_UNSIGNED,
      M_IMAGE+M_PROC+M_DISP, 
      &MilDispProcImage);

   MbufCopy(MilSrcImage, MilDispProcImage);

   // Display the image buffer.
   MdispSelect(MilDisplay, MilDispProcImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   }
