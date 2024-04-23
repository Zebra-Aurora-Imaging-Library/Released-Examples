//***************************************************************************************/
// 
// File name: FlatFieldCorrection.cpp  
//
// Synopsis:  This program illustrates how to setup and apply flat field correction.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>
#include "HistogramDisplay.h"

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("FlatFieldCorrection\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates how to set up a flat field correction\n")
             MIL_TEXT("context. The flat field correction is then applied to a sequence\n")
             MIL_TEXT("or live images. Two typical flat field correction scenarios will\n")
             MIL_TEXT("be presented:\n\n")
             MIL_TEXT(" - Case 1: Flat field correction for sensor anomalies and non-uniform lighting.\n")
             MIL_TEXT(" - Case 2: Flat field correction for sensor anomalies only.\n\n")
             

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************

// The path of the avi.
#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("FlatFieldCorrection/")

// The path of the intro images.
static MIL_CONST_TEXT_PTR INTRO_NON_UNIFORM_IMAGE      = EXAMPLE_IMAGE_PATH MIL_TEXT("NonUniformIntro.mim");
static MIL_CONST_TEXT_PTR INTRO_VIGNETTING_IMAGE       = EXAMPLE_IMAGE_PATH MIL_TEXT("VignettingIntro.mim");
static MIL_CONST_TEXT_PTR INTRO_OFFSET_IMAGE           = EXAMPLE_IMAGE_PATH MIL_TEXT("OffsetIntro.mim");

// The messages for grabbing each image of the flat field correction.
#define NB_LIGHT_STEP 3
static MIL_CONST_TEXT_PTR LIGHT_MESSAGE[NB_LIGHT_STEP] = 
   {
             // FLAT_MESSAGE
             MIL_TEXT("The camera is grabbing continuously with normal exposure time.\n")
             MIL_TEXT("To set up a Flat image, grab a uniform light gray area (such as grabbing an\n")
             MIL_TEXT("image of a blank piece of paper).\n"),

             // DARK_MESSAGE
             MIL_TEXT("The camera is grabbing continuously with normal exposure time.\n")
             MIL_TEXT("To set up a Dark image, grab a uniform dark area (such as grabbing with the \n")
             MIL_TEXT("camera's lens cap firmly in place).\n"),
   };

static MIL_CONST_TEXT_PTR LIGHT_CORRECTION_MESSAGE = 
             MIL_TEXT("The offset correction is visible in the histogram. The darkest pixel is now 0.\n")
             MIL_TEXT("The non-uniform lighting correction is visible in the image and histogram. \n")
             MIL_TEXT("The top-right and bottom-left white square are now even. The histogram\n")
             MIL_TEXT("distribution of the white areas is much less dispersed.\n");

static MIL_CONST_TEXT_PTR LIGHT_AVI[NB_LIGHT_STEP] = 
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("FlatLightImage.avi"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("DarkImage.avi"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("TargetLightImage.avi")
   };

#define NB_SENSOR_STEP 4
static MIL_CONST_TEXT_PTR SENSOR_MESSAGE[NB_SENSOR_STEP] = 
   {
             // FLAT_MESSAGE
             MIL_TEXT("The camera is grabbing continuously with short exposure time.\n")
             MIL_TEXT("To set up a Flat image, grab a uniform light gray area (such as grabbing an\n")
             MIL_TEXT("image of a blank piece of paper). Adjust your lighting intensity to\n")
             MIL_TEXT("maximize the dynamic range of the image (avoid white saturation).\n"),

             // DARK_MESSAGE
             MIL_TEXT("The camera is grabbing continuously with normal exposure time.\n")
             MIL_TEXT("To set up a Dark image, grab a uniform dark area (such as grabbing with the \n")
             MIL_TEXT("camera's lens cap firmly in place).\n"),

   M_NULL,
   
             // OFFSET_MESSAGE
             MIL_TEXT("The camera is grabbing continuously with short exposure time.\n")
             MIL_TEXT("To set up an Offset image, grab a uniform dark area (such as grabbing with the \n")
             MIL_TEXT("camera's lens cap firmly in place).\n")
   };

static MIL_CONST_TEXT_PTR SENSOR_CORRECTION_MESSAGE = 
             MIL_TEXT("The lens vignetting correction is visible in the image and histogram.\n")
             MIL_TEXT("The corners of the image are now as bright as the center. The histogram\n")
             MIL_TEXT("distribution of the white areas is much less dispersed.\n");

static MIL_CONST_TEXT_PTR SENSOR_AVI[NB_SENSOR_STEP] = 
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("FlatSensorImage.avi"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("DarkSensorImage.avi"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("TargetSensorImage.avi"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("OffsetSensorImage.avi")
   };

// The enum that describe the step.
enum FlatFieldStepEnum
   {
   enFlat = 0,
   enDark,
   enTarget,
   enOffset,
   enTotalStep,
   };

// The number of images to average the frames.
static const MIL_INT NB_ACCUMULATE_FRAMES = 16;

// The short exposure time ratio.
static const MIL_DOUBLE SHORT_EXPOSURE_TIME_MULTIPLIER = 8;

// Offsets of the windows.
static const MIL_INT WINDOWS_OFFSET_X = 15;
static const MIL_INT WINDOWS_OFFSET_Y = 38;

// Useful defines.
static const MIL_DOUBLE PI = 3.14159265358979323846;

//*****************************************************************************
// Callback structures.
//*****************************************************************************
struct SStatCumulativeData
   {
   MIL_ID MilStatResult[3];
   MIL_ID MilStatContext;
   MIL_ID MilSrcImage;
   MIL_ID MilDispImage;
   };

struct SUserData
   {
   MIL_ID MilDispImage;
   MIL_ID MilSrcImage;
   MIL_ID MilCorrectedImage;
   
   MIL_ID MilFlatFieldContext;

   MIL_ID MilDisplay;
   MIL_ID MilCorrectedDisplay;

   CHistogramDisplay* pSrcHistDisplay;
   CHistogramDisplay* pDstHistDisplay;
   };

//*****************************************************************************
// Function prototypes.
//*****************************************************************************
MIL_INT MFTYPE CalImageAccumulationFunc(MIL_INT HookType, MIL_ID EventId, void* pHookData);
MIL_INT MFTYPE FlatFieldCorrectionFunc(MIL_INT HookType, MIL_ID EventId, void* pHookData);

bool AskForInteractive();
void DrawArrow(MIL_ID MilGraContext,
               MIL_ID MilDest,
               MIL_DOUBLE ArrowCenterX,
               MIL_DOUBLE ArrowCenterY,
               MIL_DOUBLE ArrowLength,
               MIL_DOUBLE ArrowTickness,
               MIL_DOUBLE ArrowAngle);

void GrabAndSetImageInContext(MIL_ID MilDigitizer,
                              MIL_ID MilDisplay,
                              MIL_ID MilDispImage,
                              MIL_ID MilSrcImage,
                              MIL_ID* pMilGrabImages,
                              MIL_INT NbGrabImages,
                              MIL_ID MilDestImage,
                              MIL_ID MilFlatFieldContext,
                              MIL_ID MilStatCumulativeContext,
                              MIL_INT ControlFlag,
                              MIL_CONST_TEXT_PTR ImageTypeTag,
                              MIL_CONST_TEXT_PTR Message);

void SensorOnlyFlatFieldExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilGralist,
                                MIL_ID MilCorrectedDisplay, MIL_ID MilCorrectedGraList,
                                bool IsInteractive);
void LightingAndSensorFlatFieldExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilGralist,
                                       MIL_ID MilCorrectedDisplay, MIL_ID MilCorrectedGraList,
                                       bool IsInteractive);
void FlatFieldExample(MIL_ID MilSystem,
                      MIL_ID MilDisplay,
                      MIL_ID MilGraList,
                      MIL_ID MilCorrectedDisplay,
                      MIL_ID MilCorrectedGraList,
                      MIL_INT NbSteps,
                      MIL_CONST_TEXT_PTR* StepMessages,
                      MIL_CONST_TEXT_PTR* StepAvi,
                      MIL_CONST_TEXT_PTR  CorrectionMessage,
                      bool IsInteractive);

void IntroNonUniform(MIL_ID MilSystem,
                     MIL_ID MilDisplay,
                     MIL_ID MilGraList,
                     MIL_ID MilIntroImage);
void IntroVignetting(MIL_ID MilSystem,
                     MIL_ID MilDisplay,
                     MIL_ID MilGraList,
                     MIL_ID MilIntroImage);
void IntroDust(MIL_ID MilSystem,
               MIL_ID MilDisplay,
               MIL_ID MilGraList,
               MIL_ID MilIntroImage,
               MIL_ID MilEqualizeDisplay,
               MIL_ID MilEqualizeGraList,
               MIL_INT NbDust,
               const MIL_DOUBLE* pDustX,
               const MIL_DOUBLE* pDustY,
               const MIL_DOUBLE* pDustRadius);
void IntroOffset(MIL_ID MilSystem,
                 MIL_ID MilDisplay,
                 MIL_ID MilGraList,
                 MIL_ID MilIntroImage);
void GetScreenSize(MIL_INT* pMaxSizeX, MIL_INT* pMaxSizeY);
//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Print Header.
   PrintHeader();

   // Ask whether to run the example interactively.
   bool IsInteractive = AskForInteractive();
   
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_CONST_TEXT_PTR SystemDescriptor = IsInteractive ? M_SYSTEM_DEFAULT : M_SYSTEM_HOST;
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, SystemDescriptor, M_DEFAULT, M_DEFAULT, M_NULL);

   // Check if the example can run in interactive mode.
   MIL_INT SystemType = MsysInquire(MilSystem, M_SYSTEM_TYPE, M_NULL);
   if (IsInteractive && SystemType == M_SYSTEM_HOST_TYPE)
      {
      IsInteractive = false;
      MosPrintf(MIL_TEXT("This example requires a real digitizer to run properly in interactive mode.\n")
                MIL_TEXT("The current default configuration (system, digitizer, ...) needs to be changed.\n\n")
                MIL_TEXT("Press <Enter> to continue in standalone mode.\n\n"));
      MosGetch();
      }   
   
   // Allocate the displays.
   MIL_ID MilDisplay          = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, 0);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Source image"));
   MIL_ID MilCorrectedDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Allocate a graphic lists for the displays.
   MIL_ID MilGralist = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGralist);
   MIL_ID MilCorrectedGralist = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilCorrectedDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilCorrectedGralist);

   if(IsInteractive)
      {
      // Ask the user to choose the example.
      bool ValidChoice = true;
      do 
         {
         MosPrintf(MIL_TEXT("Please choose the type of flat field correction to perform.\n")
                   MIL_TEXT("   a. Lighting and Sensor correction.\n")
                   MIL_TEXT("   b. Sensor only correction.\n\n")
                   MIL_TEXT("Your choice: "));
         
         switch(MosGetch())
            {
            case MIL_TEXT('a'):
            case MIL_TEXT('A'):
               MosPrintf(MIL_TEXT("   a. Lighting and Sensor correction.\n\n"));
               LightingAndSensorFlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGralist, true);
               ValidChoice = true;
               break;

            case MIL_TEXT('b'):
            case MIL_TEXT('B'):
               MosPrintf(MIL_TEXT("   b. Sensor only correction.\n\n"));
               SensorOnlyFlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGralist, true);
               ValidChoice = true;
               break;

            default:
               ValidChoice = false;
            }
         } while (!ValidChoice);

      }
   else
      {
      // Show the lighting correction example.
      LightingAndSensorFlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGralist, false);

      // Show the Sensor correction example.
      SensorOnlyFlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGralist, false);
      }

   // Free the graphic lists.
   MgraFree(MilCorrectedGralist);
   MgraFree(MilGralist);

   // Free the displays.
   MdispFree(MilCorrectedDisplay);
   MdispFree(MilDisplay);

   // Free allocated objects.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*****************************************************************************
// Non-Uniform lighting introduction functions.
//*****************************************************************************
static const MIL_DOUBLE PROFILE_DISPLAY_SIZE_Y = 75;
static const MIL_DOUBLE PROFILE_CENTER_X = 320;
static const MIL_DOUBLE PROFILE_CENTER_Y = 240;
static const MIL_INT PROFILE_HEIGHT = 25;
static const MIL_INT PROFILE_LENGTH = 720;
static const MIL_DOUBLE PROFILE_ANGLE = 36.87;
void IntroNonUniform(MIL_ID MilSystem,
                     MIL_ID MilDisplay,
                     MIL_ID MilGraList,
                     MIL_ID MilIntroImage)
   {
   // Clear the graphic list.
   MgraClear(M_DEFAULT, MilGraList);

   // Get the size Y of the image.
   MIL_INT ImageSizeY = MbufInquire(MilIntroImage, M_SIZE_Y, M_NULL);

   // Allocate a profile image.
   MIL_ID MilProfileImage = MbufAlloc2d(MilSystem, PROFILE_LENGTH, PROFILE_HEIGHT, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, M_NULL);

   // Allocate the profile data.
   MIL_ID MilProfileResult = MimAllocResult(MilSystem, PROFILE_LENGTH, M_PROJ_LIST, M_NULL);
   MIL_DOUBLE ProfileData[PROFILE_LENGTH];
   MIL_DOUBLE ProfileIndex[PROFILE_LENGTH];

   // Allocate a profile display.
   MIL_ID MilProfileDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID MilProfileGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilProfileDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilProfileGraList);
   MdispControl(MilProfileDisplay, M_WINDOW_INITIAL_POSITION_Y, ImageSizeY + WINDOWS_OFFSET_Y);
   MdispControl(MilProfileDisplay, M_TITLE, MIL_TEXT("Profile display"));

   // Draw the profile region in the graphic list.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraRectAngle(M_DEFAULT, MilGraList, PROFILE_CENTER_X, PROFILE_CENTER_Y, PROFILE_LENGTH, PROFILE_HEIGHT, PROFILE_ANGLE, M_CENTER_AND_DIMENSION + M_FILLED);
   MgraRectAngle(M_DEFAULT, MilProfileGraList, 0, 0, PROFILE_LENGTH-1, PROFILE_HEIGHT-1, 0, M_DEFAULT);
   DrawArrow(M_DEFAULT, MilGraList, PROFILE_CENTER_X, PROFILE_CENTER_Y, (MIL_DOUBLE)PROFILE_LENGTH, (MIL_DOUBLE)PROFILE_HEIGHT, PROFILE_ANGLE);

   // Get the profile image.
   MimRotate(MilIntroImage, MilProfileImage, -PROFILE_ANGLE, PROFILE_CENTER_X, PROFILE_CENTER_Y, M_DEFAULT, M_DEFAULT, M_BILINEAR + M_OVERSCAN_CLEAR);

   // Get the profile data.
   MimProjection(MilProfileImage, MilProfileResult, M_0_DEGREE, M_DEFAULT, M_NULL);
   MimGetResult(MilProfileResult, M_TYPE_MIL_DOUBLE, ProfileData);

   // Rescale the data.
   for(MIL_INT ValIdx = 0; ValIdx < PROFILE_LENGTH; ValIdx++)
      {
      ProfileData[ValIdx] = (PROFILE_HEIGHT - ProfileData[ValIdx] / 255);
      ProfileIndex[ValIdx] = (MIL_DOUBLE)ValIdx;
      }

   // Draw the profile.
   MgraLines(M_DEFAULT, MilProfileGraList, PROFILE_LENGTH, ProfileIndex, ProfileData, M_NULL, M_NULL, M_POLYLINE);

   // Display the profile. Rescale the display to fit the desired size.
   MdispZoom(MilProfileDisplay, 1, (MIL_DOUBLE)PROFILE_DISPLAY_SIZE_Y / PROFILE_HEIGHT);
   MdispSelect(MilProfileDisplay, MilProfileImage);

   // Print message about non-uniform lighting.
   MosPrintf(MIL_TEXT("(a) Non-uniform lighting.\n")
             MIL_TEXT("The displayed flat image exhibits non-uniform lighting, which is\n")
             MIL_TEXT("illustrated by the intensity profile along the diagonal.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));

   MosGetch();

   // Free the profile display.
   MgraFree(MilProfileGraList);
   MdispFree(MilProfileDisplay);

   // Free the profile result.
   MimFree(MilProfileResult);

   // Free the profile image.
   MbufFree(MilProfileImage);
   }


//*****************************************************************************
// Vignetting introduction functions.
//*****************************************************************************
static const MIL_DOUBLE ARROW_RADIUS = 270;
static const MIL_DOUBLE ARROW_LENGTH  = 40;
static const MIL_DOUBLE ARROW_THICKNESS = 10;
static const MIL_DOUBLE ARROW_ANGLE = 36.87;
static const MIL_DOUBLE ARROW_CENTER_DIST_X = (ARROW_RADIUS) * cos(ARROW_ANGLE * PI / 180);
static const MIL_DOUBLE ARROW_CENTER_DIST_Y = (ARROW_RADIUS) * sin(ARROW_ANGLE * PI / 180);
void IntroVignetting(MIL_ID MilSystem,
                     MIL_ID MilDisplay,
                     MIL_ID MilGraList,
                     MIL_ID MilIntroImage)
   {
   // Clear the graphic list.
   MgraClear(M_DEFAULT, MilGraList);

   // Get the size of the image;
   MIL_INT ImageSizeX = MbufInquire(MilIntroImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilIntroImage, M_SIZE_Y, M_NULL);
   MIL_DOUBLE ImageCenterX = (MIL_DOUBLE)ImageSizeX / 2;
   MIL_DOUBLE ImageCenterY = (MIL_DOUBLE)ImageSizeY / 2;

   // Draw the 4 arrows.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   DrawArrow(M_DEFAULT, MilGraList, ImageCenterX - ARROW_CENTER_DIST_X, ImageCenterY - ARROW_CENTER_DIST_Y, ARROW_LENGTH, ARROW_THICKNESS, 180 - ARROW_ANGLE);
   DrawArrow(M_DEFAULT, MilGraList, ImageCenterX + ARROW_CENTER_DIST_X, ImageCenterY - ARROW_CENTER_DIST_Y, ARROW_LENGTH, ARROW_THICKNESS, ARROW_ANGLE);
   DrawArrow(M_DEFAULT, MilGraList, ImageCenterX + ARROW_CENTER_DIST_X, ImageCenterY + ARROW_CENTER_DIST_Y, ARROW_LENGTH, ARROW_THICKNESS, - ARROW_ANGLE);
   DrawArrow(M_DEFAULT, MilGraList, ImageCenterX - ARROW_CENTER_DIST_X, ImageCenterY + ARROW_CENTER_DIST_Y, ARROW_LENGTH, ARROW_THICKNESS, 180 + ARROW_ANGLE);

   // Print message about vignetting.
   MosPrintf(MIL_TEXT("(a) Lens vignetting\n")
             MIL_TEXT("The displayed flat image exhibits lens vignetting which is visible in\n")
             MIL_TEXT("its corners.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }



//*****************************************************************************
// Dust introduction functions.
//*****************************************************************************
void IntroDust(MIL_ID MilSystem,
               MIL_ID MilDisplay,
               MIL_ID MilGraList,
               MIL_ID MilIntroImage,
               MIL_ID MilEqualizeDisplay,
               MIL_ID MilEqualizeGraList,
               MIL_INT NbDust,
               const MIL_DOUBLE* pDustX,
               const MIL_DOUBLE* pDustY,
               const MIL_DOUBLE* pDustRadius)
   {
   MIL_INT ImageSizeX = MbufInquire(MilIntroImage, M_SIZE_X, M_NULL);

   // Setup the displays.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraText(M_DEFAULT, MilGraList, 0, 0, MIL_TEXT("Source image"));
   MdispControl(MilEqualizeDisplay, M_WINDOW_INITIAL_POSITION_X, ImageSizeX + WINDOWS_OFFSET_X);
   MdispControl(MilEqualizeDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);
   MdispControl(MilEqualizeDisplay, M_TITLE, MIL_TEXT("Equalized image"));
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraText(M_DEFAULT, MilEqualizeGraList, 0, 0, MIL_TEXT("Equalized image"));

   // Clear the graphic lists.
   MgraClear(M_DEFAULT, MilGraList);
   MgraClear(M_DEFAULT, MilEqualizeGraList);

   // Create the equalized image.
   MIL_ID MilEqualizeIntroImage = MbufClone(MilIntroImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilHistogramEqualizeAdaptiveContext   = MimAlloc(MilSystem, M_HISTOGRAM_EQUALIZE_ADAPTIVE_CONTEXT, M_DEFAULT, M_NULL);
   MimHistogramEqualizeAdaptive(MilHistogramEqualizeAdaptiveContext, MilIntroImage, MilEqualizeIntroImage, M_DEFAULT);
   MimFree(MilHistogramEqualizeAdaptiveContext);

   // Select it on the equalized display.
   MdispSelect(MilEqualizeDisplay, MilEqualizeIntroImage);

   // Draw circles around the dust.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   for(MIL_INT DustIdx = 0; DustIdx < NbDust; DustIdx++)
      {
      MgraArc(M_DEFAULT, MilGraList, pDustX[DustIdx], pDustY[DustIdx], pDustRadius[DustIdx], pDustRadius[DustIdx], 0, 360);
      MgraArc(M_DEFAULT, MilEqualizeGraList, pDustX[DustIdx], pDustY[DustIdx], pDustRadius[DustIdx], pDustRadius[DustIdx], 0, 360);
      }

   // Print message about dust.
   MosPrintf(MIL_TEXT("(b) Sensor and lens anomalies.\n")
             MIL_TEXT("Dust, scratches and defects are examples of sensor and lens anomalies that can\n")
             MIL_TEXT("affect an image's quality. Sensor sensitivity variations can also introduce\n")
             MIL_TEXT("unwanted artifacts. The displayed flat image exhibits several dust problems\n")
             MIL_TEXT("which are circled in blue. The image is equalized to enhance the visualization\n")
             MIL_TEXT("of the anomalies.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Deselect the image.
   MdispSelect(MilEqualizeDisplay, M_NULL);

   // Free the equalize image.
   MbufFree(MilEqualizeIntroImage);
   }

//*****************************************************************************
// Offset introduction functions.
//*****************************************************************************
void IntroOffset(MIL_ID MilSystem,
                 MIL_ID MilDisplay,
                 MIL_ID MilGraList,
                 MIL_ID MilIntroImage)
   {
   // Clear the graphic list.
   MgraClear(M_DEFAULT, MilGraList);

   // Get the size Y.
   MIL_INT ImageSizeY = MbufInquire(MilIntroImage, M_SIZE_Y, M_NULL);

   // Allocate a histogram display.
   CHistogramDisplay* pIntroHistogram = new CHistogramDisplay(MilSystem);
   pIntroHistogram->SetWindowInitialPosition(0, ImageSizeY + WINDOWS_OFFSET_Y);
   pIntroHistogram->Update(MilIntroImage, M_NULL);
   pIntroHistogram->Show();

   // Print message about offset.
   MosPrintf(MIL_TEXT("(c) Black offset.\n")
             MIL_TEXT("The displayed image exhibits a black offset that can be observed\n")
             MIL_TEXT("in its histogram.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Delete the histogram display.
   delete pIntroHistogram;
   }

//*****************************************************************************
// Flat field example function
//*****************************************************************************
static const MIL_INT LIGHTING_NB_DUST = 3;
static const MIL_DOUBLE LIGHTING_DUST_POS_X[LIGHTING_NB_DUST]  = {325, 132, 402};
static const MIL_DOUBLE LIGHTING_DUST_POS_Y[LIGHTING_NB_DUST]  = {378, 338, 373};
static const MIL_DOUBLE LIGHTING_DUST_RADIUS[LIGHTING_NB_DUST] = {  9,  85,  38};
void LightingAndSensorFlatFieldExample(MIL_ID MilSystem,
                                    MIL_ID MilDisplay,
                                    MIL_ID MilGralist,
                                    MIL_ID MilCorrectedDisplay,
                                    MIL_ID MilCorrectedGraList,
                                    bool IsInteractive)
   {
   // Print the example title.
   MosPrintf(MIL_TEXT("CASE 1: LIGHTING AND SENSOR CORRECTION\n")
             MIL_TEXT("--------------------------------------\n\n")
             MIL_TEXT("This flat-field case shows you how to correct:\n")
             MIL_TEXT("  (a) Non-uniform lighting.\n")
             MIL_TEXT("  (b) Sensor and lens anomalies.\n")
             MIL_TEXT("  (c) Black offset.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   
   // Restore the non-uniform lighting introduction image.
   MIL_ID MilIntroImage = MbufRestore(INTRO_NON_UNIFORM_IMAGE, MilSystem, M_NULL);
   MdispSelect(MilDisplay, MilIntroImage);

   // Non-uniform lighting introduction.
   IntroNonUniform(MilSystem, MilDisplay, MilGralist, MilIntroImage);

   // Dust introduction.
   IntroDust(MilSystem, MilDisplay, MilGralist, MilIntroImage, MilCorrectedDisplay, MilCorrectedGraList, LIGHTING_NB_DUST, LIGHTING_DUST_POS_X, LIGHTING_DUST_POS_Y, LIGHTING_DUST_RADIUS);

   // Free the intro image.
   MbufFree(MilIntroImage);

   // Restore the offset introduction image.
   MbufRestore(INTRO_OFFSET_IMAGE, MilSystem, &MilIntroImage);
   MdispSelect(MilDisplay, MilIntroImage);

   // Offset introduction.
   IntroOffset(MilSystem, MilDisplay, MilGralist, MilIntroImage);

   // Free the intro image.
   MbufFree(MilIntroImage);

   // Print a summary of the flat field setup.
   MosPrintf(MIL_TEXT("SETUP PROCEDURE FOR LIGHTING AND SENSOR CORRECTION:\n")
             MIL_TEXT("For this type of correction, you need to set up:\n")
             MIL_TEXT(" - a Flat image: gray image under the application's lighting (NORMAL exposure).\n")
             MIL_TEXT(" - a Dark image: black image with the lens cap in place (NORMAL exposure).\n\n")
             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   // Run the example.
   FlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGraList, 
                    NB_LIGHT_STEP, LIGHT_MESSAGE, LIGHT_AVI, LIGHT_CORRECTION_MESSAGE, IsInteractive);
   }

static const MIL_INT VIGNETTING_NB_DUST = 1;
static const MIL_DOUBLE VIGNETTING_DUST_POS_X[VIGNETTING_NB_DUST]    = {315};
static const MIL_DOUBLE VIGNETTING_DUST_POS_Y[VIGNETTING_NB_DUST]    = {196};
static const MIL_DOUBLE VIGNETTING_DUST_RADIUS[VIGNETTING_NB_DUST]   = { 48};
void SensorOnlyFlatFieldExample(MIL_ID MilSystem,
                                MIL_ID MilDisplay,
                                MIL_ID MilGralist,
                                MIL_ID MilCorrectedDisplay,
                                MIL_ID MilCorrectedGraList,
                                bool IsInteractive)
   {
   // Print the example title.
   MosPrintf(MIL_TEXT("CASE 2: SENSOR ONLY CORRECTION\n")
             MIL_TEXT("------------------------------\n\n")
             MIL_TEXT("This flat-field case shows you how to correct:\n")
             MIL_TEXT("  (a) Lens vignetting.\n")
             MIL_TEXT("  (b) Sensor and lens anomalies.\n")
             MIL_TEXT("  (c) Black offset.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Restore the vignetting introduction image.
   MIL_ID MilIntroImage = MbufRestore(INTRO_VIGNETTING_IMAGE, MilSystem, M_NULL);
   MdispSelect(MilDisplay, MilIntroImage);

   // Non-uniform lighting introduction.
   IntroVignetting(MilSystem, MilDisplay, MilGralist, MilIntroImage);

   // Dust introduction.
   IntroDust(MilSystem, MilDisplay, MilGralist, MilIntroImage, MilCorrectedDisplay, MilCorrectedGraList, VIGNETTING_NB_DUST, VIGNETTING_DUST_POS_X, VIGNETTING_DUST_POS_Y, VIGNETTING_DUST_RADIUS);

   // Free the intro image.
   MbufFree(MilIntroImage);

   // Restore the offset introduction image.
   MbufRestore(INTRO_OFFSET_IMAGE, MilSystem, &MilIntroImage);
   MdispSelect(MilDisplay, MilIntroImage);

   // Offset introduction.
   IntroOffset(MilSystem, MilDisplay, MilGralist, MilIntroImage);

   // Free the intro image.
   MbufFree(MilIntroImage);

   // Print a summary of the flat field setup.
   MosPrintf(MIL_TEXT("SETUP PROCEDURE FOR SENSOR ONLY CORRECTION:\n")
             MIL_TEXT("For this type of correction, you need to set up:\n")
             MIL_TEXT(" - an Offset image: black image with the lens cap in place (SHORT exposure).\n")
             MIL_TEXT(" - a Flat image: gray image with uniform lighting (SHORT exposure).\n")
             MIL_TEXT(" - a Dark image: black image with the lens cap in place (NORMAL exposure).\n\n")
             MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();

   FlatFieldExample(MilSystem, MilDisplay, MilGralist, MilCorrectedDisplay, MilCorrectedGraList,
                    NB_SENSOR_STEP, SENSOR_MESSAGE, SENSOR_AVI, SENSOR_CORRECTION_MESSAGE, IsInteractive);
   }

void FlatFieldExample(MIL_ID MilSystem,
                      MIL_ID MilDisplay,
                      MIL_ID MilGraList,
                      MIL_ID MilCorrectedDisplay,
                      MIL_ID MilCorrectedGraList,
                      MIL_INT NbSteps,
                      MIL_CONST_TEXT_PTR* StepMessages,
                      MIL_CONST_TEXT_PTR* StepAvi,
                      MIL_CONST_TEXT_PTR  CorrectionMessage,
                      bool IsInteractive)
   {
   // Allocate the digitizer.
   MIL_ID MilDigitizers[enTotalStep];
   if(!IsInteractive)
      {
      for(MIL_INT StepIdx = 0; StepIdx < NbSteps; StepIdx++)
         MdigAlloc(MilSystem, M_DEFAULT, StepAvi[StepIdx], M_DEFAULT, &MilDigitizers[StepIdx]);
      }
   else
      {
      MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizers[0]);
      for(MIL_INT StepIdx = 1; StepIdx < NbSteps; StepIdx++)
         MilDigitizers[StepIdx] = MilDigitizers[0];
      }

   // Get the parameters of the digitizer.
   MIL_INT Type     = MdigInquire(MilDigitizers[0], M_TYPE, M_NULL);
   MIL_INT SizeX    = MdigInquire(MilDigitizers[0], M_SIZE_X, M_NULL);
   MIL_INT SizeY    = MdigInquire(MilDigitizers[0], M_SIZE_Y, M_NULL);
   MIL_INT SizeBand = MdigInquire(MilDigitizers[0], M_SIZE_BAND, M_NULL);
   MIL_INT SizeBit  = MdigInquire(MilDigitizers[0], M_SIZE_BIT, M_NULL);

   if (SizeBit != 8)
      {
      MdispControl(MilDisplay, M_VIEW_MODE, M_AUTO_SCALE);
      }

   // Allocate the images.
   const MIL_INT NB_GRABIMAGES = 3;
   MIL_ID MilGrabImages[NB_GRABIMAGES];
   for (MIL_INT i = 0; i < NB_GRABIMAGES; i++)
      {
      MilGrabImages[i] = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_GRAB, M_NULL);
      }
   MIL_ID MilOffsetImage    = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilFlatImage      = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilDarkImage      = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilSrcImage       = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilCorrectedImage = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP, M_NULL);
   MIL_ID MilDispImage      = MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, M_IMAGE + M_PROC + M_DISP + M_GRAB, M_NULL);

   // Allocate and setup the histogram displays.
   CHistogramDisplay* pSrcHistDisplay = new CHistogramDisplay(MilSystem, MIL_TEXT("Source histogram"), M_COLOR_RED);
   CHistogramDisplay* pDstHistDisplay = new CHistogramDisplay(MilSystem, MIL_TEXT("Corrected histogram"), M_COLOR_CYAN);
   pSrcHistDisplay->Preprocess(MilGrabImages[0]);
   pDstHistDisplay->Preprocess(MilGrabImages[0]);

   // Get the maximum size of the display.
   MIL_INT MaxSizeX;
   MIL_INT MaxSizeY;
   GetScreenSize(&MaxSizeX, &MaxSizeY);

   // Get the zoom factor of the image.
   MIL_DOUBLE ZoomX = (MIL_DOUBLE)(MaxSizeX - 2 * WINDOWS_OFFSET_X) / (SizeX * 2);
   MIL_DOUBLE ZoomY = (MIL_DOUBLE)(MaxSizeY - pSrcHistDisplay->GetHistImageSizeY() - 2 * WINDOWS_OFFSET_Y) / SizeY;
   MIL_DOUBLE Zoom = (ZoomX < ZoomY) ? ZoomX : ZoomY;
   if (Zoom < 1.0)
      {
      MdispZoom(MilDisplay, Zoom, Zoom);
      MdispZoom(MilCorrectedDisplay, Zoom, Zoom);
      }
   else
      Zoom = 1.0;

   // Set the position of the histograms.
   pSrcHistDisplay->SetWindowInitialPosition(0, (MIL_INT)((MIL_DOUBLE)SizeY * Zoom + WINDOWS_OFFSET_Y));
   pDstHistDisplay->SetWindowInitialPosition((MIL_INT)((MIL_DOUBLE)SizeX * Zoom + WINDOWS_OFFSET_X), (MIL_INT)((MIL_DOUBLE)SizeY * Zoom + WINDOWS_OFFSET_Y));

   // Allocate the flat field context.
   MIL_ID MilFlatFieldContext = MimAlloc(MilSystem, M_FLAT_FIELD_CONTEXT, M_DEFAULT, M_NULL);

   // Allocate the stat Cumulative context and preprocess it.
   MIL_ID MilStatCumulativeContext = MimAlloc(MilSystem, M_STATISTICS_CUMULATIVE_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilStatCumulativeContext, M_STAT_MEAN, M_ENABLE);

   // Clear the graphic lists.
   MgraClear(M_DEFAULT, MilGraList);
   MgraClear(M_DEFAULT, MilCorrectedGraList);

   // If all the steps are performed, grab the offset image first and reduce the exposure time.
   MIL_DOUBLE CurExposureTime;
   MIL_INT ExposureTimeControlError = M_NULL_ERROR;
   if(NbSteps == enTotalStep)
      {
      if(IsInteractive)
         {
         // Reduce the exposure time if supported.
         MIL_DOUBLE MinExposureTime;
         
         // Disable the error printing.
         MappControl(M_ERROR, M_PRINT_DISABLE);
         
         // Get the current exposure time
         MdigInquire(MilDigitizers[0], M_EXPOSURE_TIME , &CurExposureTime);
         MappGetError(M_CURRENT, &ExposureTimeControlError);

         if (ExposureTimeControlError == M_NULL_ERROR)
            {         
            MdigInquire(MilDigitizers[0], M_EXPOSURE_TIME + M_MIN_VALUE, &MinExposureTime);
            MappGetError(M_CURRENT, &ExposureTimeControlError);
            if (ExposureTimeControlError == M_NULL_ERROR)
               {
               // Set the short exposure time.
               MIL_DOUBLE ShortExposureTime = MinExposureTime * SHORT_EXPOSURE_TIME_MULTIPLIER;
               ShortExposureTime = (ShortExposureTime > CurExposureTime) ? CurExposureTime : ShortExposureTime;
               MdigControl(MilDigitizers[0], M_EXPOSURE_TIME, ShortExposureTime);
               MappGetError(M_CURRENT, &ExposureTimeControlError);
               }
            }

         // If an error occured while setting the exposure time
         if (ExposureTimeControlError != M_NULL_ERROR)
            {
            MosPrintf(MIL_TEXT("LIMITATION DETECTED:\n")
                      MIL_TEXT("This digitizer does not allow dynamic control of the exposure time.\n")
                      MIL_TEXT("The example will continue using the normal exposure time instead of\n")
                      MIL_TEXT("a short exposure time. Sensor only correction might not work as expected.\n\n")
                      MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();
            }

         // Enable the error printing.
         MappControl(M_ERROR, M_PRINT_ENABLE);
         }

      // Grab and set the offset image.
      GrabAndSetImageInContext(MilDigitizers[enOffset], MilDisplay, MilDispImage, MilSrcImage, MilGrabImages, NB_GRABIMAGES, MilOffsetImage, MilFlatFieldContext,
                               MilStatCumulativeContext, M_OFFSET_IMAGE, MIL_TEXT("Offset"), StepMessages[enOffset]);
      }

   // Grab and set the flat image.
   GrabAndSetImageInContext(MilDigitizers[enFlat], MilDisplay, MilDispImage, MilSrcImage, MilGrabImages, NB_GRABIMAGES, MilFlatImage, MilFlatFieldContext,
                            MilStatCumulativeContext, M_FLAT_IMAGE, MIL_TEXT("Flat"), StepMessages[enFlat]);

   // Put the exposure time back to the dcf value.
   if(IsInteractive && (NbSteps == enTotalStep) && (ExposureTimeControlError == M_NULL_ERROR))
      MdigControl(MilDigitizers[0], M_EXPOSURE_TIME, CurExposureTime);
      
   // Grab and set the dark image.
   GrabAndSetImageInContext(MilDigitizers[enDark], MilDisplay, MilDispImage, MilSrcImage, MilGrabImages, NB_GRABIMAGES, MilDarkImage, MilFlatFieldContext,
                            MilStatCumulativeContext, M_DARK_IMAGE, MIL_TEXT("Dark"), StepMessages[enDark]);

   // If the offset image wasn't grabbed set the dark image as the offset image. The flat image
   // needed to be grabbed with normal exposure time.
   if(NbSteps != enTotalStep)
      MimControl(MilFlatFieldContext, M_OFFSET_IMAGE, MilDarkImage);

   // Use the automatic gain.
   MimControl(MilFlatFieldContext, M_GAIN_CONST, M_AUTOMATIC);

   // Preprocess the flat field context.
   MimFlatField(MilFlatFieldContext, MilDispImage, MilCorrectedImage, M_PREPROCESS);

   // Deselect the image on the display.
   MdispSelect(MilDisplay, M_NULL);

   // Print message.
   MosPrintf(MIL_TEXT("The flat field context is now preprocessed.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Setup the displays.
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, 0);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Source image"));
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraText(M_DEFAULT, MilGraList, 0, 0, MIL_TEXT("Source image"));
   MdispControl(MilCorrectedDisplay, M_WINDOW_INITIAL_POSITION_X, SizeX * Zoom + WINDOWS_OFFSET_X);
   MdispControl(MilCorrectedDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);
   MdispControl(MilCorrectedDisplay, M_TITLE, MIL_TEXT("Corrected image"));
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraText(M_DEFAULT, MilCorrectedGraList, 0, 0, MIL_TEXT("Corrected image"));

   MdispSelect(MilDisplay, M_NULL);
   MdispSelect(MilDisplay, MilDispImage);
   MdispSelect(MilCorrectedDisplay, MilCorrectedImage);
      
   // Fill the user data structure.
   SUserData UserData;
   UserData.MilDispImage         = MilDispImage;
   UserData.MilSrcImage          = MilSrcImage;
   UserData.MilCorrectedImage    = MilCorrectedImage;

   UserData.MilFlatFieldContext  = MilFlatFieldContext;

   UserData.MilDisplay           = MilDisplay;
   UserData.MilCorrectedDisplay  = MilCorrectedDisplay;
   UserData.pSrcHistDisplay      = pSrcHistDisplay;
   UserData.pDstHistDisplay      = pDstHistDisplay;

   // Show the histogram display.
   pSrcHistDisplay->Show();
   pDstHistDisplay->Show();

   // Start grabbing and wait for the user input to stop.
   MdigProcess(MilDigitizers[enTarget], MilGrabImages, NB_GRABIMAGES, M_START, M_DEFAULT, FlatFieldCorrectionFunc, &UserData);
   MosPrintf(MIL_TEXT("The images of a continuous grab are now flat field corrected.\n")
             MIL_TEXT("%s\n")
             MIL_TEXT("Press <Enter> to end.\n\n"),
             IsInteractive ? MIL_TEXT("") : CorrectionMessage);
   MosGetch();

   // Stop grabbing.
   MdigProcess(MilDigitizers[enTarget], MilGrabImages, NB_GRABIMAGES, M_STOP, M_DEFAULT, FlatFieldCorrectionFunc, &UserData);

   // Free the histogram displays.
   delete pDstHistDisplay;
   delete pSrcHistDisplay;

   // Free the stat cumulative context.
   MimFree(MilStatCumulativeContext);

   // Free the flat field context.
   MimFree(MilFlatFieldContext);

   // Free the images.
   MbufFree(MilCorrectedImage);
   MbufFree(MilSrcImage);
   MbufFree(MilDarkImage);
   MbufFree(MilFlatImage);
   MbufFree(MilOffsetImage);
   MbufFree(MilDispImage);
   for (MIL_INT i = 0; i < NB_GRABIMAGES; i++)
      {
      MbufFree(MilGrabImages[i]);
      }

   if (Zoom < 1.0)
      {
      MdispZoom(MilDisplay, 1.0, 1.0);
      MdispZoom(MilCorrectedDisplay, 1.0, 1.0);
      }

   // Free the digitizer.
   if(!IsInteractive)
      {
      for(MIL_INT StepIdx = 1; StepIdx < NbSteps; StepIdx++)
         MdigFree(MilDigitizers[StepIdx]);
      }
   MdigFree(MilDigitizers[0]);
   }


//*****************************************************************************
// Mean image processing function callback.
//*****************************************************************************
MIL_INT MFTYPE CalImageAccumulationFunc(MIL_INT HookType, MIL_ID EventId, void* pHookData)
   {
   SStatCumulativeData* pStatCumulativeData = (SStatCumulativeData*)pHookData;

   // Get the modified buffer id.
   MIL_ID MilModifiedBuffer;
   MdigGetHookInfo(EventId, M_MODIFIED_BUFFER + M_BUFFER_ID, &MilModifiedBuffer);
   MIL_INT SizeBand = MbufInquire(MilModifiedBuffer, M_SIZE_BAND, M_NULL);

   // Copy the image in the src image, in case the format of the image is not planar.
   MbufCopy(MilModifiedBuffer, pStatCumulativeData->MilSrcImage);

   // Add the grab buffer to the stat cumulative result.
   for (MIL_INT ResultIdx = 0; ResultIdx < SizeBand; ResultIdx++)
      {
      MIL_ID MilBand = MbufChildColor(pStatCumulativeData->MilSrcImage, ResultIdx, M_NULL);
      MimStatCalculate(pStatCumulativeData->MilStatContext, MilBand, pStatCumulativeData->MilStatResult[ResultIdx], M_DEFAULT);
      MbufFree(MilBand);
      }

   // Copy the modified buffer in the displayed image.
   MbufCopy(MilModifiedBuffer, pStatCumulativeData->MilDispImage);

   return 0;
   }
//*****************************************************************************
// Flat field processing function callback.
//*****************************************************************************
MIL_INT MFTYPE FlatFieldCorrectionFunc(MIL_INT HookType, MIL_ID EventId, void* pHookData)
   {
   SUserData* pUserData = (SUserData*)pHookData;
   
   // Get the modified buffer id.
   MIL_ID MilModifiedBuffer;
   MdigGetHookInfo(EventId, M_MODIFIED_BUFFER + M_BUFFER_ID, &MilModifiedBuffer);

   // Disable display updates.
   MdispControl(pUserData->MilDisplay, M_UPDATE, M_DISABLE);
   MdispControl(pUserData->MilCorrectedDisplay, M_UPDATE, M_DISABLE);
      
   // Copy the image in the src image, in case the format of the image is not planar.
   MbufCopy(MilModifiedBuffer, pUserData->MilSrcImage);

   // Perform the flat field correction.
   MimFlatField(pUserData->MilFlatFieldContext, pUserData->MilSrcImage, pUserData->MilCorrectedImage, M_DEFAULT);

   // Update the histograms.
   MIL_DOUBLE MaxVal = pUserData->pSrcHistDisplay->Update(pUserData->MilDispImage, M_NULL);
   pUserData->pDstHistDisplay->Update(pUserData->MilCorrectedImage, MaxVal);
   
   // Copy the modified buffer in the displayed image.
   MbufCopy(MilModifiedBuffer, pUserData->MilDispImage);

   // Enable display updates.
   MdispControl(pUserData->MilCorrectedDisplay, M_UPDATE, M_ENABLE);
   MdispControl(pUserData->MilDisplay, M_UPDATE, M_ENABLE);

   return 0;
   }

//*****************************************************************************
// Function that grabs and sets the grabbed image in the flat field context.
//*****************************************************************************
void GrabAndSetImageInContext(MIL_ID MilDigitizer,
                              MIL_ID MilDisplay,
                              MIL_ID MilDispImage,
                              MIL_ID MilSrcImage,
                              MIL_ID* pMilGrabImages,
                              MIL_INT NbGrabImages,
                              MIL_ID MilDestImage,
                              MIL_ID MilFlatFieldContext,
                              MIL_ID MilStatCumulativeContext,
                              MIL_INT ControlFlag,
                              MIL_CONST_TEXT_PTR ImageTypeTag,
                              MIL_CONST_TEXT_PTR Message)
   {
   // Allocate a stat cumulative result and preprocess it.
   MIL_ID MilSystem = MbufInquire(pMilGrabImages[0], M_OWNER_SYSTEM, M_NULL);
   MIL_INT SizeBand = MbufInquire(pMilGrabImages[0], M_SIZE_BAND, M_NULL);
   SStatCumulativeData StatCumulativeData;
   StatCumulativeData.MilStatContext = MilStatCumulativeContext;
   for (MIL_INT ResultIdx = 0; ResultIdx < SizeBand; ResultIdx++)
      {
      MIL_ID MilBand = MbufChildColor(MilSrcImage, ResultIdx, M_NULL);
      MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, &StatCumulativeData.MilStatResult[ResultIdx]);
      MimStatCalculate(MilStatCumulativeContext, MilBand, StatCumulativeData.MilStatResult[ResultIdx], M_PREPROCESS);
      MbufFree(MilBand);
      }
   StatCumulativeData.MilDispImage = MilDispImage;
   StatCumulativeData.MilSrcImage = MilSrcImage;

   // Select the image on the display.
   MbufClear(MilDispImage, 0);
   MdispSelect(MilDisplay, MilDispImage);

   // Start grabbing the flat field image.
   MdigGrabContinuous(MilDigitizer, MilDispImage);

   MosPrintf(MIL_TEXT("%s\nPress <Enter> to grab and set the %s image.\n\n"), Message, ImageTypeTag);
   MosGetch();
   
   // Stop the grab.
   MdigHalt(MilDigitizer);

   // Grab a sequence of images.
   MdigProcess(MilDigitizer, pMilGrabImages, NbGrabImages, M_SEQUENCE + M_COUNT(NB_ACCUMULATE_FRAMES),
               M_SYNCHRONOUS, CalImageAccumulationFunc, &StatCumulativeData);

   for (MIL_INT ResultIdx = 0; ResultIdx < SizeBand; ResultIdx++)
      {
      MIL_ID MilBand = MbufChildColor(MilDestImage, ResultIdx, M_NULL);

      // Get the mean grabbed image band.
      MimDraw(M_DEFAULT, StatCumulativeData.MilStatResult[ResultIdx], M_NULL, MilBand, M_DRAW_STAT_RESULT, M_STAT_MEAN, M_NULL, M_DEFAULT);
 
      // Free the stat Cumulative result.
      MimFree(StatCumulativeData.MilStatResult[ResultIdx]);

      MbufFree(MilBand);
      }

   // Set the mean grabbed image in the flat field context.
   MimControl(MilFlatFieldContext, ControlFlag, MilDestImage);
   }

//*****************************************************************************
// Draws a thick arrow.
//*****************************************************************************
void DrawArrow(MIL_ID MilGraContext,
               MIL_ID MilDest,
               MIL_DOUBLE ArrowCenterX,
               MIL_DOUBLE ArrowCenterY,
               MIL_DOUBLE ArrowLength,
               MIL_DOUBLE ArrowTickness,
               MIL_DOUBLE ArrowAngle)
   {
   MIL_DOUBLE ArrowEndX[3] = {ArrowLength/2 + ArrowTickness, ArrowLength/2, ArrowLength/2};
   MIL_DOUBLE ArrowEndY[3] = {0, -ArrowTickness, ArrowTickness}; 

   MIL_DOUBLE RotatedArrowEndX[3];
   MIL_DOUBLE RotatedArrowEndY[3];
   MIL_DOUBLE Vx = cos(-ArrowAngle * PI / 180);
   MIL_DOUBLE Vy = sin(-ArrowAngle * PI / 180);
   for(MIL_INT PointIdx = 0; PointIdx < 3; PointIdx++)
      {
      RotatedArrowEndX[PointIdx] = ArrowCenterX + Vx * ArrowEndX[PointIdx] - Vy * ArrowEndY[PointIdx];
      RotatedArrowEndY[PointIdx] = ArrowCenterY + Vy * ArrowEndX[PointIdx] + Vx * ArrowEndY[PointIdx];
      }
   MgraRectAngle(MilGraContext, MilDest, ArrowCenterX, ArrowCenterY, ArrowLength, ArrowTickness, ArrowAngle, M_CENTER_AND_DIMENSION + M_FILLED);
   MgraLines(MilGraContext, MilDest, 3, RotatedArrowEndX, RotatedArrowEndY, M_NULL, M_NULL, M_POLYGON + M_FILLED);
   }

//*****************************************************************************
// Ask the user for interactive method choice.
//*****************************************************************************
bool AskForInteractive()
   {
   // Ask the user if he wants to run the example in interactive mode.
   MosPrintf(MIL_TEXT("Do you want to run the example in interactive mode ? (Y or N)\n\n"));
   do 
      {
      switch(MosGetch())
         {
         case 'y':
         case 'Y':
            return true;
            
         case 'n':
         case 'N':
            return false;
         }
      } while(1);
   
   return true;
   }

//*****************************************************************************
//Get the size of the screen.
//*****************************************************************************
void GetScreenSize(MIL_INT* pMaxSizeX, MIL_INT* pMaxSizeY)
   {
   MIL_ID MilExclusiveDisp = MdispAlloc(M_DEFAULT_HOST, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_EXCLUSIVE, M_NULL);
   MdispInquire(MilExclusiveDisp, M_SIZE_X, pMaxSizeX);
   MdispInquire(MilExclusiveDisp, M_SIZE_Y, pMaxSizeY);
   MdispFree(MilExclusiveDisp);
   }
