/////////////////////////////////////////////////////////////////////////////////////////////////
//
// File name: ColorRelativeCalibration.cpp
//
// Synopsis:  This program shows the functionality of color calibration in color machine vision
//            applications. Color appearance distortions introduced by different camera
//            settings or lighting conditions will be corrected after color calibration to
//            enable better precision of color-based analysis in future processing.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("ColorRelativeCalibration/")
#define TEXT_POSITION    2
#define NUM_COLOR_PATCH  140

enum ApplicationCaseEnum
   {
   enFoodInspection,
   enPrintInspection,
   enBoardInspection,
   NumOfApplicationCases
   };

enum DispalyOptionEnum
   {
   enShowPreprocessingInfo,
   enShowCalibrationResults,
   enResetDisplay
   };

enum ColorRenderingCaseEnum
   {
   enFirstShowCase,
   enSecondShowCase,
   enThirdShowCase,
   enNonColorRenderingCase
   };

class CColorCalibrationDemo;

namespace DemoUtil
   {
   // Principal functions
   void SetDemoEnv           (MIL_ID*             pApplicationId, MIL_ID* pSystemId,   MIL_ID* pDisplayId,   MIL_ID* pDisplayImageId);
   void PrintDemoHeader      ();                  
   void LaunchApplicationCase(MIL_INT             CaseIdx,        MIL_ID  MilSystemId, MIL_ID  MilDisplayId, MIL_ID* pMilDisplayImageId);
   void PrintUseCaseIntro    (ApplicationCaseEnum enApplicationCase);
   void FreeDemoObjects      (MIL_ID              ApplicationId,  MIL_ID  SystemId,    MIL_ID  DisplayId,    MIL_ID  DisplayImageId);
   };

class CColorCalibrationDemo
   {
   public:
      CColorCalibrationDemo (MIL_ID SystemId);
      ~CColorCalibrationDemo();

      // Class principal methods
      void GenerateDemoScenario   (MIL_ID SystemId, ApplicationCaseEnum enApplicationCase);
      void PerformColorCalibration(MIL_ID SystemId, MIL_ID              DisplayId, MIL_ID* pDisplayImageId, ApplicationCaseEnum enApplicationCase);
      void UpdateDisplay          (MIL_ID SystemId, MIL_ID              DisplayId, MIL_ID* pDisplayImageId, ApplicationCaseEnum enApplicationCase, DispalyOptionEnum enDisplayOption, ColorRenderingCaseEnum enShowCase);

      // Helper functions for color calibration scenarios
      void GenerateFoodInspectionCase (MIL_ID SystemId);
      void GeneratePrintInspectionCase(MIL_ID SystemId);
      void GenerateBoardInspectionCase(MIL_ID SystemId);

      // Helper function for defining color sample using grid locater
      void DefineSampleForColorChecker(MIL_ID SystemId, ApplicationCaseEnum enApplicationCase, MIL_ID SampleID, MIL_INT SampleLabelOrIndex);
      
      // Helper functions for color calibration in each case
      void PerformFoodInspectionCase (MIL_ID SystemId, MIL_ID DisplayId, MIL_ID* pDisplayImageId);
      void PerformPrintInspectionCase(MIL_ID SystemId, MIL_ID DisplayId, MIL_ID* pDisplayImageId);
      void PerformBoardInspectionCase(MIL_ID SystemId, MIL_ID DisplayId, MIL_ID* pDisplayImageId);

      // Helper functions for display
      void SetDisplayImage   (MIL_ID SystemId,  MIL_ID              DisplayId,         MIL_ID*           pDisplayImageId, ApplicationCaseEnum    enApplicationCase, DispalyOptionEnum enDisplayOption, ColorRenderingCaseEnum enShowCase);
      void ShowProcessingInfo(MIL_ID SystemId,  MIL_ID              MilDisplayId,      MIL_ID            DisplayImageId,  ApplicationCaseEnum    enApplicationCase, DispalyOptionEnum enDisplayOption, ColorRenderingCaseEnum enShowCase);
      void PrintMessage      (MIL_ID DisplayId, ApplicationCaseEnum enApplicationCase, DispalyOptionEnum enDisplayOption, ColorRenderingCaseEnum enShowCase);

   private:
      // Color context related properties 
      MIL_ID  m_ColorCalibrationContext;
      MIL_INT m_CalibrationMethod;
      MIL_INT m_CalibrationIntent;
      MIL_INT m_ComputeOption;

      // Color sample buffers
      MIL_ID m_ReferenceImage;
      MIL_ID m_TrainingImage;
      MIL_ID m_TrainingImageCalibrated;
      MIL_ID m_ImageToBeCalibrated;
      MIL_ID m_ImageCalibrated;
      MIL_ID m_ReferenceMosaicForHspi;
      MIL_ID m_TrainingMosaicForHspi;
      MIL_ID m_ReferenceImageForApplyHspi;
      
      // Color sample information
      MIL_INT m_SampleLabelOrIndex;
      
      // Display related properties
      MIL_ID             m_TrainingGraListId;
      MIL_INT            m_DisplaySampleSizeX;
      MIL_INT            m_DisplaySampleSizeY;
      MIL_INT            m_LeftSampleTextPosX;
      MIL_INT            m_MiddleSampleTextPosX;
      MIL_INT            m_RightSampleTextPosX;
      MIL_INT            m_ReferenceIndexForMva;
      MIL_DOUBLE         m_ResizeCoefForHspi;
      MIL_CONST_TEXT_PTR m_ReferenceImageLabel;
      MIL_CONST_TEXT_PTR m_SampleImageLabel;
      MIL_CONST_TEXT_PTR m_CalibratedImageLabel;
   };

template <class T> inline T Max(T x, T y) { return ((x >= y) ? x : y); }

// Image file list
static const MIL_INT NUM_MVA_SAMPLES = 6;
static MIL_CONST_TEXT_PTR MVA_SAMPLE_LIST_FILENAME[NUM_MVA_SAMPLES] =
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantCyan.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantGreen.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantYellow.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantWhite.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantViolet.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantMagenta.mim")
   };

static const MIL_INT NUM_HSPI_SAMPLES = 3;
static MIL_CONST_TEXT_PTR HSPI_REFERENCE_LIST_FILENAME[NUM_HSPI_SAMPLES] =
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodReference1.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodReference2.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodReference3.mim"),
   };

static MIL_CONST_TEXT_PTR HSPI_TRAINING_LIST_FILENAME[NUM_HSPI_SAMPLES] =
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodTraining1.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodTraining2.mim"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodTraining3.mim"),
   };

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main function
//
/////////////////////////////////////////////////////////////////////////////////////////////////
int MosMain(void)
   {
   using namespace DemoUtil;

   MIL_ID MilApplicationId;   // MIL application identifier
   MIL_ID MilSystemId;        // MIL system identifier
   MIL_ID MilDisplayId;       // MIL display identifier
   MIL_ID MilDisplayImageId;  // MIL display color image identifier
   
   // Allocation of demo objects
   SetDemoEnv(&MilApplicationId, &MilSystemId, &MilDisplayId, &MilDisplayImageId);

   // Print color relative calibration principals and demo contents
   PrintDemoHeader();

   // Perform and visualize all application cases of color calibration
   for(MIL_INT CaseIdx = 0; CaseIdx < NumOfApplicationCases; CaseIdx++)
      LaunchApplicationCase(CaseIdx, MilSystemId, MilDisplayId, &MilDisplayImageId);
   
   FreeDemoObjects(MilApplicationId, MilSystemId, MilDisplayId, MilDisplayImageId);
   return 0;	
   }

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Namespace DemoUtil
//
/////////////////////////////////////////////////////////////////////////////////////////////////
void DemoUtil::PrintDemoHeader()
   {
   // Print the demo synopsis for color relative calibration.
   MosPrintf(MIL_TEXT("\n|---------------------------------------------------------------------------|\n")); 
   MosPrintf(MIL_TEXT(  "| Color calibration aims to correct color appearance distortion introduced  |\n"));
   MosPrintf(MIL_TEXT(  "| by camera or illuminant changes. The color calibrated image enables better|\n")); 
   MosPrintf(MIL_TEXT(  "| precision for color-based machine vision applications.                    |\n")); 
   MosPrintf(MIL_TEXT(  "|                                                                           |\n"));
   MosPrintf(MIL_TEXT(  "| This demo shows several typical use cases of color-relative calibration.  |\n"));
   MosPrintf(MIL_TEXT(  "|                                                                           |\n"));
   MosPrintf(MIL_TEXT(  "|   Case(1): histogram-based color-relative calibration,                    |\n"));
   MosPrintf(MIL_TEXT(  "|            for food inspection                                            |\n"));
   MosPrintf(MIL_TEXT(  "|                                                                           |\n"));
   MosPrintf(MIL_TEXT(  "|   Case(2): color-to-color based color-relative calibration,               |\n"));
   MosPrintf(MIL_TEXT(  "|            for print inspection                                           |\n"));
   MosPrintf(MIL_TEXT(  "|                                                                           |\n"));
   MosPrintf(MIL_TEXT(  "|   Case(3): global-mean-variance-based color-relative calibration,         |\n"));
   MosPrintf(MIL_TEXT(  "|            for electronic board inspection                                |\n"));
   MosPrintf(MIL_TEXT(  "|                                                                           |\n"));
   MosPrintf(MIL_TEXT(  "|---------------------------------------------------------------------------|\n")); 
   MosPrintf(MIL_TEXT("\nPress any key to start the demo ...\n\n"));
   MosGetch();
   }

void DemoUtil::SetDemoEnv(MIL_ID* pApplicationId, 
                          MIL_ID* pSystemId, 
                          MIL_ID* pDisplayId,
                          MIL_ID* pDisplayImageId)
   {
   // Application allocation
   MappAlloc(M_NULL, M_DEFAULT, pApplicationId);

   // System allocation
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, pSystemId);

   // Display allocation
   *pDisplayImageId = M_NULL;
   MdispAlloc (*pSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, pDisplayId);
   MgraColor  (M_DEFAULT,  M_COLOR_CYAN);
   MgraControl(M_DEFAULT,  M_BACKGROUND_MODE, M_OPAQUE);
   }

void DemoUtil::LaunchApplicationCase(MIL_INT CaseIdx, 
                                     MIL_ID  MilSystemId,
                                     MIL_ID  MilDisplayId,
                                     MIL_ID* pMilDisplayImageId)
   {
   // Allocation of color calibration objects
   CColorCalibrationDemo ColorCalibrationCase(MilSystemId);
   ApplicationCaseEnum   enApplicationCase;

   switch(CaseIdx)
      {
      case 0:
         {
         enApplicationCase = enFoodInspection;
         break;
         }
      
      case 1:
         {
         enApplicationCase = enPrintInspection;
         break;
         }

      case 2:
         {
         enApplicationCase = enBoardInspection;
         break;
         }
         
      default:
         MosPrintf(MIL_TEXT("Invalid application case!"));
         break;
      }

   // Generate scenario of color calibration
   ColorCalibrationCase.GenerateDemoScenario(MilSystemId, enApplicationCase);

   // Perform color calibration and visualize results
   ColorCalibrationCase.PerformColorCalibration(MilSystemId, MilDisplayId, pMilDisplayImageId, enApplicationCase);
   }

void DemoUtil::PrintUseCaseIntro(ApplicationCaseEnum enApplicationCase)
   {
   switch (enApplicationCase)
      {
      case enFoodInspection:
         {
         MosPrintf(MIL_TEXT("\n\nCase(1): histogram-based color-relative calibration,                  \n"));
         MosPrintf(MIL_TEXT(    "         for food inspection.                                         \n"));
         MosPrintf(MIL_TEXT(    "-------------------------------------------------------               \n"));
         MosPrintf(MIL_TEXT("\n  Introduction:                                                         "  ));
         MosPrintf(MIL_TEXT("\n    - A color-relative calibration is performed by providing the color  "  ));
         MosPrintf(MIL_TEXT("\n      distribution information of grabbed images. In this case, the     "  ));
         MosPrintf(MIL_TEXT("\n      contents of reference and training images must be similar.        "  ));
         MosPrintf(MIL_TEXT("\n      However, neither strict pixel-wise alignment between samples      "  ));
         MosPrintf(MIL_TEXT("\n      nor a standard ColorChecker target is required.                   \n"));
         MosPrintf(MIL_TEXT("\n  Color calibration steps:                                              "  ));
         MosPrintf(MIL_TEXT("\n    - User grabs images of food products on conveyor 1 using camera     "  ));
         MosPrintf(MIL_TEXT("\n      1 under illuminant 1. These images define the reference data      "  ));
         MosPrintf(MIL_TEXT("\n      in the color calibration context (see 1st column of the display). \n"));
         MosPrintf(MIL_TEXT("\n    - User grabs images of the same collection of products on           "  ));
         MosPrintf(MIL_TEXT("\n      conveyor 2, which uses camera 2 under illuminant 2. These         "  ));
         MosPrintf(MIL_TEXT("\n      images define the training data in context (see 2nd column of the "  ));
         MosPrintf(MIL_TEXT("\n      display).                                                         \n"));
         MosPrintf(MIL_TEXT("\n    - Then the color-relative calibration is performed to estimate      "  ));
         MosPrintf(MIL_TEXT("\n      the color transform from the training color data to the reference "  ));
         MosPrintf(MIL_TEXT("\n      color data.                                                       \n"));
         MosPrintf(MIL_TEXT("\n    - A transformation is applied to the training data for verification "  ));
         MosPrintf(MIL_TEXT("\n      (see 3rd column of the display).                                  \n"));
         break;                                                                                        
         }                                                                                             
      case enPrintInspection:                                                                          
         {                                                                                             
         MosPrintf(MIL_TEXT("\n\n\nCase(2): color-to-color-based color-relative calibration,           \n"));
         MosPrintf(MIL_TEXT(      "         for print inspection.                                      \n"));
         MosPrintf(MIL_TEXT(      "------------------------------------------------------------        \n"));
         MosPrintf(MIL_TEXT("\n  Introduction:                                                         "  ));
         MosPrintf(MIL_TEXT("\n    - A color-relative calibration is performed providing explicit      "  ));
         MosPrintf(MIL_TEXT("\n      pairs of colors with the help of a ColorChecker target. So in     "  ));
         MosPrintf(MIL_TEXT("\n      a first step, the color data of the reference and training images "  ));
         MosPrintf(MIL_TEXT("\n      may require a realignment before performing the color-relative    "  ));
         MosPrintf(MIL_TEXT("\n      calibration calculation. Note that using a ColorChecker target    "  ));
         MosPrintf(MIL_TEXT("\n      is not mandatory. Real products can be used as long as the        "  ));
         MosPrintf(MIL_TEXT("\n      reference and training images provide paired colors.              \n"));
         MosPrintf(MIL_TEXT("\n  Color calibration steps:                                              "  ));
         MosPrintf(MIL_TEXT("\n    - User grabs ColorChecker target image using camera 1 under         "  ));
         MosPrintf(MIL_TEXT("\n      illuminant 1. The grabbed image defines the reference data in the "  ));
         MosPrintf(MIL_TEXT("\n      color calibration context (see 1st column of the display).        \n"));
         MosPrintf(MIL_TEXT("\n    - User grabs an image of the same ColorChecker target using camera  "  ));
         MosPrintf(MIL_TEXT("\n      2 under illuminant 2. The grabbed image defines the training      "  ));
         MosPrintf(MIL_TEXT("\n      data in context (see 2nd column of the display).                  \n"));
         MosPrintf(MIL_TEXT("\n    - If required, reference and training color data are first aligned  "  ));
         MosPrintf(MIL_TEXT("\n      using MIL tools by locating the color patches (shown by overlaid  "  ));
         MosPrintf(MIL_TEXT("\n      cross symbols).                                                   \n"));
         MosPrintf(MIL_TEXT("\n    - Then the color-relative calibration is performed to estimate      "  ));
         MosPrintf(MIL_TEXT("\n      the color transform from the training color data to the reference "  ));
         MosPrintf(MIL_TEXT("\n      color data.                                                       \n"));
         MosPrintf(MIL_TEXT("\n    - A transformation is applied to the training data for verification "  ));
         MosPrintf(MIL_TEXT("\n      (see 3rd column of the display).                                  \n"));
         break;                                                                                        
         }                                                                                             
      case enBoardInspection:                                                                          
         {                                                                                             
         MosPrintf(MIL_TEXT("\n\n\nCase(3): global-mean-variance-based color-relative calibration,     \n")); 
         MosPrintf(MIL_TEXT(      "         for electronic board inspection                            \n"));
         MosPrintf(MIL_TEXT(      "-----------------------------------------------------------------   \n"));
         MosPrintf(MIL_TEXT("\n  Introduction:                                                         "  ));
         MosPrintf(MIL_TEXT("\n    - A color-relative calibration is performed to remove the global    "  ));
         MosPrintf(MIL_TEXT("\n      color casting/drifting effect of grabbed images. Neither data     "  ));
         MosPrintf(MIL_TEXT("\n      alignment between samples nor similarity of image content is      "  ));
         MosPrintf(MIL_TEXT("\n      required. Only global color distribution features are used.       \n"));
         MosPrintf(MIL_TEXT("\n  Color calibration steps:                                              "  ));
         MosPrintf(MIL_TEXT("\n    - User grabs images of an electronic board under different          "  ));
         MosPrintf(MIL_TEXT("\n      illuminants. A mosaic image is generated using these images to    "  ));
         MosPrintf(MIL_TEXT("\n      show the color casting effect across the different acquisitions   "  ));
         MosPrintf(MIL_TEXT("\n      (see top row of the display).                                     \n"));
         MosPrintf(MIL_TEXT("\n    - One of the images is selected as the reference appearance         "  ));
         MosPrintf(MIL_TEXT("\n      (see reference highlighted with an overlaid blue rectangle).      \n"));
         MosPrintf(MIL_TEXT("\n    - Then the color-relative calibration is performed to estimate      "  ));
         MosPrintf(MIL_TEXT("\n      the color transform from the remaining 5 training color data      "  ));
         MosPrintf(MIL_TEXT("\n      to the selected reference color data.                             \n"));
         MosPrintf(MIL_TEXT("\n    - Transformations are applied to the training data for verification."  ));
         MosPrintf(MIL_TEXT("\n      A new mosaic image is built and displayed using the corrected     "  ));
         MosPrintf(MIL_TEXT("\n      images (see bottom row of the display). The various color         "  ));
         MosPrintf(MIL_TEXT("\n      drifting effects have been removed and a smooth color appearance  "  ));
         MosPrintf(MIL_TEXT("\n      across the new mosaic tiles can be observed.                      \n"));
         break;
         }
      }
   }

void DemoUtil::FreeDemoObjects(MIL_ID ApplicationId, 
                               MIL_ID SystemId, 
                               MIL_ID DisplayId,
                               MIL_ID DisplayImageId)
   {
   // Free display buffer
   MbufFree(DisplayImageId);

   // Free display object
   MdispFree(DisplayId);

   // Free system object
   MsysFree(SystemId);

   // Free application
   MappFree(ApplicationId);
   }

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// class CColorCalibrationDemo
//
/////////////////////////////////////////////////////////////////////////////////////////////////
CColorCalibrationDemo::CColorCalibrationDemo(MIL_ID SystemId)
   {
   // Allocation of color relative calibration context
   McolAlloc(SystemId, 
             M_COLOR_CALIBRATION_RELATIVE, 
             M_DEFAULT, 
             M_DEFAULT, 
             M_DEFAULT, 
             &m_ColorCalibrationContext);

   // Property initialization
   m_CalibrationMethod          = M_HISTOGRAM_BASED;
   m_CalibrationIntent          = M_BALANCE;
   m_ComputeOption              = M_COMPUTE_ITEM_STAT;
   m_ReferenceImage             = M_NULL;
   m_TrainingImage              = M_NULL;
   m_TrainingImageCalibrated    = M_NULL;
   m_ImageToBeCalibrated        = M_NULL;
   m_ImageCalibrated            = M_NULL;
   m_ReferenceMosaicForHspi     = M_NULL;
   m_TrainingMosaicForHspi      = M_NULL;
   m_ReferenceImageForApplyHspi = M_NULL;
   m_SampleLabelOrIndex         = M_SAMPLE_LABEL(1);
   m_TrainingGraListId          = M_NULL;
   m_DisplaySampleSizeX         = M_NULL;
   m_DisplaySampleSizeY         = M_NULL;
   m_LeftSampleTextPosX         = M_NULL;
   m_MiddleSampleTextPosX       = M_NULL;
   m_RightSampleTextPosX        = M_NULL;
   m_ReferenceIndexForMva       = 3;
   m_ResizeCoefForHspi          = 0.5;
   m_ReferenceImageLabel        = MIL_TEXT("");
   m_SampleImageLabel           = MIL_TEXT("");
   m_CalibratedImageLabel       = MIL_TEXT("");
   }

CColorCalibrationDemo::~CColorCalibrationDemo()
   {
   // Free allocated buffers
   if (m_ReferenceImageForApplyHspi)
      MbufFree(m_ReferenceImageForApplyHspi);

   if (m_TrainingMosaicForHspi)
      MbufFree(m_TrainingMosaicForHspi);

   if (m_ReferenceMosaicForHspi)
      MbufFree(m_ReferenceMosaicForHspi);
   
   if (m_ImageCalibrated)
      MbufFree(m_ImageCalibrated);

   if (m_ImageToBeCalibrated)
      MbufFree(m_ImageToBeCalibrated);

   if(m_TrainingImageCalibrated)
      MbufFree(m_TrainingImageCalibrated);

   if(m_TrainingImage)
      MbufFree(m_TrainingImage);

   if (m_ReferenceImage)
      MbufFree(m_ReferenceImage);

   // Free color relative calibration context
   McolFree(m_ColorCalibrationContext);

   // Free graphic object
   if (m_TrainingGraListId)
      MgraFree(m_TrainingGraListId);
   }

void CColorCalibrationDemo::GenerateDemoScenario(MIL_ID SystemId, ApplicationCaseEnum enApplicationCase)
   {
   switch(enApplicationCase)
      {
      case enFoodInspection:
         {
         GenerateFoodInspectionCase(SystemId);
         break;
         }

      case enPrintInspection:
         {
         GeneratePrintInspectionCase(SystemId);
         break;
         }

      case enBoardInspection:
         {
         GenerateBoardInspectionCase(SystemId);
         break;
         }
      
      default:
         {
         MosPrintf(MIL_TEXT("Invalid application case!"));
         break;
         }
      }
   }

void CColorCalibrationDemo::PerformColorCalibration(MIL_ID              SystemId,
                                                    MIL_ID              DisplayId,
                                                    MIL_ID*             pDisplayImageId,
                                                    ApplicationCaseEnum enApplicationCase)
   {
   switch (enApplicationCase)
      {
      case enFoodInspection:
         PerformFoodInspectionCase(SystemId, DisplayId, pDisplayImageId);
         break;

      case enPrintInspection:
         PerformPrintInspectionCase(SystemId, DisplayId, pDisplayImageId);
         break;

      case enBoardInspection:
         PerformBoardInspectionCase(SystemId, DisplayId, pDisplayImageId);
         break;

      default:
         break;
      }
   }

void CColorCalibrationDemo::GenerateFoodInspectionCase(MIL_ID SystemId)
   {
   MIL_CONST_TEXT_PTR ImageFileName;

   m_CalibrationMethod = M_HISTOGRAM_BASED;
   m_CalibrationIntent = M_PRECISION;
   m_ComputeOption     = M_COMPUTE_ITEM_PIXELS;

   // Load first image to be calibrated
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodTarget.mim");
   MbufRestore(ImageFileName, SystemId, &m_ImageToBeCalibrated);
   
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("FastFoodReference3.mim");
   MbufRestore(ImageFileName, SystemId, &m_ReferenceImageForApplyHspi);

   MbufAllocColor(M_DEFAULT_HOST,
                  3, 
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_X, M_NULL),
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_Y, M_NULL),
                  8+M_UNSIGNED, 
                  M_IMAGE+M_PROC,
                  &m_ImageCalibrated);
   MbufClear(m_ImageCalibrated, 0);
   }

void CColorCalibrationDemo::GeneratePrintInspectionCase(MIL_ID SystemId)
   {
   MIL_CONST_TEXT_PTR ImageFileName;

   // Set context parameters
   m_CalibrationMethod = M_COLOR_TO_COLOR;
   m_CalibrationIntent = M_PRECISION;
   m_ComputeOption     = M_COMPUTE_ITEM_PIXELS;

   // Load reference image
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("ColorCheckerReference.mim");
   MbufRestore(ImageFileName, SystemId, &m_ReferenceImage);

   // Load training image
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("ColorCheckerTraining.mim");
   MbufRestore(ImageFileName, SystemId, &m_TrainingImage);

   // Load first image to be calibrated
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("ColorPrintUnderFluorescent.mim");
   MbufRestore(ImageFileName, SystemId, &m_ImageToBeCalibrated);

   MbufAllocColor(M_DEFAULT_HOST,
                  3, 
                  MbufInquire(m_TrainingImage, M_SIZE_X, M_NULL),
                  MbufInquire(m_TrainingImage, M_SIZE_Y, M_NULL),
                  8+M_UNSIGNED, 
                  M_IMAGE+M_PROC,
                  &m_TrainingImageCalibrated);
   MbufClear(m_TrainingImageCalibrated, 0);
   
   MbufAllocColor(M_DEFAULT_HOST,
                  3, 
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_X, M_NULL),
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_Y, M_NULL),
                  8+M_UNSIGNED, 
                  M_IMAGE+M_PROC,
                  &m_ImageCalibrated);
   MbufClear(m_ImageCalibrated, 0);
   }

void CColorCalibrationDemo::GenerateBoardInspectionCase(MIL_ID SystemId)
   {
   m_CalibrationMethod = M_GLOBAL_MEAN_VARIANCE;
   m_CalibrationIntent = M_GENERALIZATION;
   m_ComputeOption     = M_DEFAULT;
   }

void CColorCalibrationDemo::PerformFoodInspectionCase(MIL_ID  SystemId,
                                                      MIL_ID  DisplayId, 
                                                      MIL_ID* pDisplayImageId)
   {
   MIL_CONST_TEXT_PTR ReferenceImageFileName;
   MIL_CONST_TEXT_PTR TrainingImageFileName;
   MIL_INT ImageSizeX        = MbufDiskInquire(HSPI_TRAINING_LIST_FILENAME[0], M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY        = MbufDiskInquire(HSPI_TRAINING_LIST_FILENAME[0], M_SIZE_Y, M_NULL);
   MIL_INT ResizedImageSizeX = (MIL_INT)(ImageSizeX * m_ResizeCoefForHspi);
   MIL_INT ResizedImageSizeY = (MIL_INT)(ImageSizeY * m_ResizeCoefForHspi);
   MbufAllocColor(M_DEFAULT_HOST,
                  3,
                  ResizedImageSizeX,
                  ResizedImageSizeY * NUM_HSPI_SAMPLES,
                  8+M_UNSIGNED,
                  M_IMAGE+M_PROC+M_DISP,
                  &m_ReferenceMosaicForHspi);
   MbufClear(m_ReferenceMosaicForHspi, 0);
   
   MbufAllocColor(M_DEFAULT_HOST,
                  3,
                  ResizedImageSizeX,
                  ResizedImageSizeY * NUM_HSPI_SAMPLES,
                  8+M_UNSIGNED,
                  M_IMAGE+M_PROC+M_DISP,
                  &m_TrainingMosaicForHspi);
   MbufClear(m_TrainingMosaicForHspi, 0);

   McolSetMethod(m_ColorCalibrationContext,
                 m_CalibrationMethod,
                 m_CalibrationIntent,
                 m_ComputeOption,
                 M_DEFAULT);

   for (MIL_INT i = 0; i < NUM_HSPI_SAMPLES; i++)
      {
      // Define sample type
      MIL_INT SampleType = (i == 0) ? M_IMAGE : M_IMAGE + M_ADD_COLOR_TO_SAMPLE;
      
      MIL_ID ReferenceToUse = MbufAllocColor(M_DEFAULT_HOST, 3, ResizedImageSizeX, ResizedImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
      MIL_ID TrainingToUse  = MbufAllocColor(M_DEFAULT_HOST, 3, ResizedImageSizeX, ResizedImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
      MbufClear(ReferenceToUse, 0);
      MbufClear(TrainingToUse,  0);

      // Obtain reference and training samples
      ReferenceImageFileName = HSPI_REFERENCE_LIST_FILENAME[i];
      TrainingImageFileName  = HSPI_TRAINING_LIST_FILENAME [i];
      MbufRestore(ReferenceImageFileName, SystemId, &m_ReferenceImage);
      MbufRestore(TrainingImageFileName,  SystemId, &m_TrainingImage);

      // Define sample into context
      MimResize(m_ReferenceImage, ReferenceToUse, m_ResizeCoefForHspi, m_ResizeCoefForHspi, M_DEFAULT);
      MimResize(m_TrainingImage, TrainingToUse,   m_ResizeCoefForHspi, m_ResizeCoefForHspi, M_DEFAULT);
      McolDefine(m_ColorCalibrationContext, m_ReferenceImage, M_REFERENCE_SAMPLE, SampleType, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      McolDefine(m_ColorCalibrationContext, m_TrainingImage,  M_SAMPLE_LABEL(1),  SampleType, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      // Put sample into mosaic buffer for visualization
      MIL_ID ReferenceMosaicChildId = MbufChild2d(m_ReferenceMosaicForHspi, 0, ResizedImageSizeY*i, ResizedImageSizeX, ResizedImageSizeY, M_NULL);
      MIL_ID TrainingMosaicChildId  = MbufChild2d(m_TrainingMosaicForHspi,  0, ResizedImageSizeY*i, ResizedImageSizeX, ResizedImageSizeY, M_NULL);
      MbufCopy(ReferenceToUse, ReferenceMosaicChildId);
      MbufCopy(TrainingToUse,  TrainingMosaicChildId);
      
      // Free local objects
      MbufFree(TrainingMosaicChildId);  TrainingMosaicChildId  = M_NULL;
      MbufFree(ReferenceMosaicChildId); ReferenceMosaicChildId = M_NULL;
      MbufFree(TrainingToUse);          TrainingToUse          = M_NULL;
      MbufFree(ReferenceToUse);         ReferenceToUse         = M_NULL;
      MbufFree(m_TrainingImage);        m_TrainingImage        = M_NULL;
      MbufFree(m_ReferenceImage);       m_ReferenceImage       = M_NULL;
      }

   MbufAllocColor(M_DEFAULT_HOST,
                  3,
                  MbufInquire(m_TrainingMosaicForHspi, M_SIZE_X, M_NULL),
                  MbufInquire(m_TrainingMosaicForHspi, M_SIZE_Y, M_NULL),
                  8+M_UNSIGNED, 
                  M_IMAGE+M_PROC,
                  &m_TrainingImageCalibrated);
   MbufClear(m_TrainingImageCalibrated, 0);   

   // Perform preprocessing
   DemoUtil::PrintUseCaseIntro(enFoodInspection);
   McolPreprocess(m_ColorCalibrationContext, M_DEFAULT);

   // Visualize training performance
   McolTransform(m_ColorCalibrationContext,
                 m_SampleLabelOrIndex,
                 m_TrainingMosaicForHspi,
                 m_TrainingImageCalibrated,
                 M_DEFAULT);
   UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enFoodInspection, enShowPreprocessingInfo, enNonColorRenderingCase);

   // Visualize the calibration results
   McolTransform(m_ColorCalibrationContext,
                 m_SampleLabelOrIndex,
                 m_ImageToBeCalibrated,
                 m_ImageCalibrated,
                 M_DEFAULT);
   UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enFoodInspection, enShowCalibrationResults, enNonColorRenderingCase);
   }

void CColorCalibrationDemo::PerformPrintInspectionCase(MIL_ID  SystemId, 
                                                       MIL_ID  DisplayId, 
                                                       MIL_ID* pDisplayImageId)
   {
   McolSetMethod(m_ColorCalibrationContext,
                 m_CalibrationMethod,
                 m_CalibrationIntent,
                 m_ComputeOption,
                 M_DEFAULT);

   // Define reference image into context
   DefineSampleForColorChecker(SystemId, enPrintInspection, m_ReferenceImage, M_REFERENCE_SAMPLE);

   // Define training image into context
   DefineSampleForColorChecker(SystemId, enPrintInspection, m_TrainingImage,  m_SampleLabelOrIndex);

   // Perform preprocessing
   DemoUtil::PrintUseCaseIntro(enPrintInspection);
   McolPreprocess(m_ColorCalibrationContext, M_DEFAULT);

   // Visualize training performance
   McolTransform(m_ColorCalibrationContext,
                 m_SampleLabelOrIndex,
                 m_TrainingImage,
                 m_TrainingImageCalibrated,
                 M_DEFAULT);
   UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enPrintInspection, enShowPreprocessingInfo, enNonColorRenderingCase);

   // Visualize the calibration results
   McolTransform(m_ColorCalibrationContext,
                 m_SampleLabelOrIndex,
                 m_ImageToBeCalibrated,
                 m_ImageCalibrated,
                 M_DEFAULT);
   UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enPrintInspection, enShowCalibrationResults, enNonColorRenderingCase);
   }

void CColorCalibrationDemo::PerformBoardInspectionCase(MIL_ID  SystemId,
                                                       MIL_ID  DisplayId, 
                                                       MIL_ID* pDisplayImageId)
   {
   McolSetMethod(m_ColorCalibrationContext,
                 m_CalibrationMethod,
                 m_CalibrationIntent,
                 m_ComputeOption,
                 M_DEFAULT);

   // Define sample into the context
   for (MIL_INT i = 0; i < NUM_MVA_SAMPLES; i++)
      {
      // Add the sample into the context.
      MbufRestore(MVA_SAMPLE_LIST_FILENAME[i], SystemId, &m_TrainingImage);
      McolDefine(m_ColorCalibrationContext, m_TrainingImage, M_DEFAULT, M_IMAGE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MbufFree(m_TrainingImage); m_TrainingImage = M_NULL;
      }

   DemoUtil::PrintUseCaseIntro(enBoardInspection);
   MIL_CONST_TEXT_PTR     ReferenceImageName;
   ColorRenderingCaseEnum enShowCase;
   for (MIL_INT MvaShowCaseIdx = 0; MvaShowCaseIdx < 2; MvaShowCaseIdx++)
      {
      // Loop for all MVA images
      MIL_ID DispOriginalChild    = M_NULL;
      MIL_ID DispCalibratedChild  = M_NULL;
      MIL_ID OriginalImageChild   = M_NULL;
      MIL_ID CalibratedImageChild = M_NULL;

      // Load reference image
      if (m_ReferenceImage)
         MbufFree(m_ReferenceImage);
      ReferenceImageName = (MvaShowCaseIdx == 0) ? EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantWhite.mim") :
                                                   EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantMagenta.mim");
      MbufRestore(ReferenceImageName, SystemId, &m_ReferenceImage);

      // Set the reference index upon which an overlay rectangle will be shown for displaying purpose
      m_ReferenceIndexForMva = (MvaShowCaseIdx == 0) ? 3 /*4th slice*/ : 5 /*2nd slice*/;

      // Set the show case index
      enShowCase = (MvaShowCaseIdx == 0) ? enFirstShowCase : enSecondShowCase;

      if (m_TrainingImageCalibrated)
         MbufFree(m_TrainingImageCalibrated);

      MbufAllocColor(M_DEFAULT_HOST,
                     3,
                     MbufInquire(m_ReferenceImage, M_SIZE_X, M_NULL),
                     MbufInquire(m_ReferenceImage, M_SIZE_Y, M_NULL),
                     8 + M_UNSIGNED,
                     M_IMAGE + M_PROC,
                     &m_TrainingImageCalibrated);
      MbufClear(m_TrainingImageCalibrated, 0);

      // Define reference images into context
      McolDefine(m_ColorCalibrationContext, m_ReferenceImage, M_REFERENCE_SAMPLE, M_IMAGE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      
      // Perform preprocessing
      SetDisplayImage(SystemId, DisplayId, pDisplayImageId, enBoardInspection, enResetDisplay, enShowCase);
      McolPreprocess(m_ColorCalibrationContext, M_DEFAULT);

      // Apply MVA for Color Drifting correction
      MIL_INT ImageSizeX = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_X, M_NULL);
      MIL_INT ImageSizeY = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_Y, M_NULL);
      MIL_INT ImageChildSizeX = ImageSizeX / NUM_MVA_SAMPLES;
      for (MIL_INT i = 0; i < NUM_MVA_SAMPLES; i++)
         {
         m_SampleLabelOrIndex = M_SAMPLE_INDEX(i);

         // Perform MVA color calibration over the training images
         MbufRestore(MVA_SAMPLE_LIST_FILENAME[i], SystemId, &m_TrainingImage);
         McolTransform(m_ColorCalibrationContext, m_SampleLabelOrIndex, m_TrainingImage, m_TrainingImageCalibrated, M_DEFAULT);

         // Allocate child buffers for product display.
         MbufChild2d(m_TrainingImage,           ImageChildSizeX*i, 0,          ImageChildSizeX, ImageSizeY, &OriginalImageChild);
         MbufChild2d(*pDisplayImageId,          ImageChildSizeX*i, 0,          ImageChildSizeX, ImageSizeY, &DispOriginalChild);
         MbufChild2d(m_TrainingImageCalibrated, ImageChildSizeX*i, 0,          ImageChildSizeX, ImageSizeY, &CalibratedImageChild);
         MbufChild2d(*pDisplayImageId,          ImageChildSizeX*i, ImageSizeY, ImageChildSizeX, ImageSizeY, &DispCalibratedChild);

         // Put data to display buffer
         MbufCopy(OriginalImageChild,   DispOriginalChild);
         MbufCopy(CalibratedImageChild, DispCalibratedChild);

         // Release allocate temp buffer
         MbufFree(OriginalImageChild);   OriginalImageChild   = M_NULL;
         MbufFree(CalibratedImageChild); CalibratedImageChild = M_NULL;
         MbufFree(DispCalibratedChild);  DispCalibratedChild  = M_NULL;
         MbufFree(DispOriginalChild);    DispOriginalChild    = M_NULL;
         MbufFree(m_TrainingImage);      m_TrainingImage      = M_NULL;
         }

      UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enBoardInspection, enShowPreprocessingInfo, enShowCase);
      }

   // A case of wild mapping: rendering natural image with the color appearance of the electronic board
   // Remove all defined samples
   McolDefine(m_ColorCalibrationContext, M_NULL, M_ALL, M_DELETE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Load reference image
   MIL_CONST_TEXT_PTR ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("ColorBoardIlluminantWhite.mim");
   if (m_ReferenceImage)
      MbufFree(m_ReferenceImage);
   MbufRestore(ImageFileName, SystemId, &m_ReferenceImage);
   McolDefine(m_ColorCalibrationContext, m_ReferenceImage, M_REFERENCE_SAMPLE, M_IMAGE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Load training image
   ImageFileName = EXAMPLE_IMAGE_PATH MIL_TEXT("OceanNaturalScene.mim");
   if (m_TrainingImage)
      MbufFree(m_TrainingImage);
   MbufRestore(ImageFileName, SystemId, &m_TrainingImage);
   McolDefine(m_ColorCalibrationContext, m_TrainingImage, M_DEFAULT, M_IMAGE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Preprocessing
   //UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enBoardInspection, enShowPreprocessingInfo, enThirdShowCase);
   McolPreprocess(m_ColorCalibrationContext, M_DEFAULT);

   // Use training image as image to be calibrated. 
   // (here re-allocate buffer for image to be allocated to keep the code of setting display and printing information untouched.)
   if (m_ImageToBeCalibrated)
      MbufFree(m_ImageToBeCalibrated);
   MbufRestore(ImageFileName, SystemId, &m_ImageToBeCalibrated);
   MbufAllocColor(M_DEFAULT_HOST,
                  3,
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_X, M_NULL),
                  MbufInquire(m_ImageToBeCalibrated, M_SIZE_Y, M_NULL),
                  8 + M_UNSIGNED,
                  M_IMAGE + M_PROC,
                  &m_ImageCalibrated);
   MbufClear(m_ImageCalibrated, 0);
   McolTransform(m_ColorCalibrationContext, M_SAMPLE_INDEX(0), m_ImageToBeCalibrated, m_ImageCalibrated, M_DEFAULT);
   
   // Update display
   UpdateDisplay(SystemId, DisplayId, pDisplayImageId, enBoardInspection, enShowCalibrationResults, enThirdShowCase);
   }

void CColorCalibrationDemo::UpdateDisplay(MIL_ID                 SystemId,                                          
                                          MIL_ID                 DisplayId, 
                                          MIL_ID*                pDisplayImageId,
                                          ApplicationCaseEnum    enApplicationCase,
                                          DispalyOptionEnum      enDisplayOption,
                                          ColorRenderingCaseEnum enShowCase)
   {
   // Set display dimension
   SetDisplayImage(SystemId, DisplayId, pDisplayImageId, enApplicationCase, enDisplayOption, enShowCase);

   // Feed display data and processing information
   ShowProcessingInfo(SystemId, DisplayId, *pDisplayImageId, enApplicationCase, enDisplayOption, enShowCase);
   }

void CColorCalibrationDemo::SetDisplayImage(MIL_ID                 SystemId, 
                                            MIL_ID                 DisplayId, 
                                            MIL_ID*                pDisplayImageId, 
                                            ApplicationCaseEnum    enApplicationCase, 
                                            DispalyOptionEnum      enDisplayOption,
                                            ColorRenderingCaseEnum enShowCase)
   {
   // Disable display update
   MdispControl(DisplayId, M_UPDATE, M_DISABLE);

   // The first and second show case for electronic board inspection uses the following display.
   if ((enBoardInspection == enApplicationCase) && (enThirdShowCase != enShowCase))
      {
      if (enResetDisplay == enDisplayOption)
         {
         MIL_INT ImageSizeX      = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_X, M_NULL);
         MIL_INT ImageSizeY      = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_Y, M_NULL);
         MIL_INT ImageChildSizeX = ImageSizeX / NUM_MVA_SAMPLES;

         if (*pDisplayImageId)
            MbufFree(*pDisplayImageId);

         // Allocate new display buffer
         MbufAllocColor(SystemId,
                        3,
                        ImageSizeX,
                        ImageSizeY * 2,
                        8 + M_UNSIGNED,
                        M_IMAGE + M_PROC + M_DISP,
                        pDisplayImageId);
         MbufClear(*pDisplayImageId, M_COLOR_GRAY);
         }
      }
   else // enFoodInspection || enPrintInspection || 3rd show case of enBoardInspection
      {
      // Get image size information
      MIL_INT DispSizeXInNeed = M_NULL;
      MIL_INT DispSizeYInNeed = M_NULL;
      MIL_INT LeftImageSizeX  = M_NULL;
      MIL_INT LeftImageSizeY  = M_NULL;

      // Set display image size
      if (enShowCalibrationResults == enDisplayOption)
         {
         m_DisplaySampleSizeX = MbufInquire(m_ImageToBeCalibrated, M_SIZE_X, M_NULL);
         m_DisplaySampleSizeY = MbufInquire(m_ImageToBeCalibrated, M_SIZE_Y, M_NULL);
         LeftImageSizeX       = m_DisplaySampleSizeX;
         LeftImageSizeY       = m_DisplaySampleSizeY;
         } 
      else // enShowPreprocessingInfo
         {
         if (enFoodInspection == enApplicationCase)
            {
            m_DisplaySampleSizeX = MbufInquire(m_TrainingMosaicForHspi, M_SIZE_X, M_NULL);
            m_DisplaySampleSizeY = MbufInquire(m_TrainingMosaicForHspi, M_SIZE_Y, M_NULL);
            LeftImageSizeX       = m_DisplaySampleSizeX;
            LeftImageSizeY       = m_DisplaySampleSizeY;
            }
         else // enPrintInspection
            {
            m_DisplaySampleSizeX = MbufInquire(m_TrainingImage, M_SIZE_X, M_NULL);
            m_DisplaySampleSizeY = MbufInquire(m_TrainingImage, M_SIZE_Y, M_NULL);
            LeftImageSizeX       = MbufInquire(m_ReferenceImage, M_SIZE_X, M_NULL);
            LeftImageSizeY       = MbufInquire(m_ReferenceImage, M_SIZE_Y, M_NULL);
            }
         }

      DispSizeXInNeed = LeftImageSizeX + m_DisplaySampleSizeX + m_DisplaySampleSizeX;
      DispSizeYInNeed = Max(LeftImageSizeY, m_DisplaySampleSizeY);

      // Set overlay text position.
      m_LeftSampleTextPosX   = TEXT_POSITION;
      m_MiddleSampleTextPosX = LeftImageSizeX + TEXT_POSITION;
      m_RightSampleTextPosX  = LeftImageSizeX + m_DisplaySampleSizeX + TEXT_POSITION;

      // For the first time display, allocate a display buffer
      if (M_NULL == *pDisplayImageId)
         {
         MbufAllocColor(SystemId, 
                        3, 
                        DispSizeXInNeed, 
                        DispSizeYInNeed, 
                        8 + M_UNSIGNED, 
                        M_IMAGE + M_PROC + M_DISP, 
                        pDisplayImageId);
         }

      // If the image to be calibrated is not of the same size as training sample, reset display size
      else
         {
         bool NeedToResetDispSize = (DispSizeXInNeed != MbufInquire(*pDisplayImageId, M_SIZE_X, M_NULL) ||
                                     DispSizeYInNeed != MbufInquire(*pDisplayImageId, M_SIZE_Y, M_NULL));
         if (NeedToResetDispSize)
            {
            // Free allocated display buffer
            MbufFree(*pDisplayImageId);

            // Allocate new display buffer
            MbufAllocColor(SystemId,
                           3,
                           DispSizeXInNeed,
                           DispSizeYInNeed,
                           8 + M_UNSIGNED,
                           M_IMAGE + M_PROC + M_DISP,
                           pDisplayImageId);
            }
         }

      MbufClear(*pDisplayImageId, M_COLOR_BLACK);
      }
   }

void CColorCalibrationDemo::ShowProcessingInfo(MIL_ID                 SystemId, 
                                               MIL_ID                 DisplayId, 
                                               MIL_ID                 DisplayImageId, 
                                               ApplicationCaseEnum    enApplicationCase, 
                                               DispalyOptionEnum      enDisplayOption,
                                               ColorRenderingCaseEnum enShowCase)
   {
   // Enable display update and overlay
   MdispSelect(DisplayId,  DisplayImageId);
   MdispControl(DisplayId, M_UPDATE, M_ENABLE);

   // Refresh overlay buffer
   MdispControl(DisplayId, M_OVERLAY, M_DISABLE);
   MdispControl(DisplayId, M_OVERLAY, M_ENABLE);

   if ((enBoardInspection == enApplicationCase) && (enThirdShowCase != enShowCase))
      {
      PrintMessage(DisplayId, enApplicationCase, enDisplayOption, enShowCase);
      }
   else // enFoodInspection || enPrintInspection || 3rd show case of enBoardInspection
      {
      // Show processing images
      MIL_ID  DispImageLeft   = M_NULL;
      MIL_ID  DispImageMiddle = M_NULL;
      MIL_ID  DispImageRight  = M_NULL;
      MIL_INT LeftImageSizeX  = M_NULL;
      MIL_INT LeftImageSizeY  = M_NULL;

      if (enShowCalibrationResults == enDisplayOption)
         {
         LeftImageSizeX = MbufInquire(m_ImageToBeCalibrated, M_SIZE_X, M_NULL);
         LeftImageSizeY = MbufInquire(m_ImageToBeCalibrated, M_SIZE_Y, M_NULL);
         }
      else // enShowPreprocessingInfo
         {
         if (enFoodInspection == enApplicationCase)
            {
            LeftImageSizeX = MbufInquire(m_ReferenceMosaicForHspi, M_SIZE_X, M_NULL);
            LeftImageSizeY = MbufInquire(m_ReferenceMosaicForHspi, M_SIZE_Y, M_NULL);
            }
         else // enPrintInspection || 3rd show case of enBoardInspection
            {
            LeftImageSizeX = MbufInquire(m_ReferenceImage, M_SIZE_X, M_NULL);
            LeftImageSizeY = MbufInquire(m_ReferenceImage, M_SIZE_Y, M_NULL);
            }
         }

      MbufChild2d(DisplayImageId, 0,                                     0, LeftImageSizeX,       LeftImageSizeY,       &DispImageLeft);
      MbufChild2d(DisplayImageId, LeftImageSizeX,                        0, m_DisplaySampleSizeX, m_DisplaySampleSizeY, &DispImageMiddle);
      MbufChild2d(DisplayImageId, LeftImageSizeX + m_DisplaySampleSizeX, 0, m_DisplaySampleSizeX, m_DisplaySampleSizeY, &DispImageRight);

      switch (enDisplayOption)
         {
         case enShowPreprocessingInfo:
           {
           if (enFoodInspection == enApplicationCase)
               {
               MbufCopy(m_ReferenceMosaicForHspi, DispImageLeft);
               MbufCopy(m_TrainingMosaicForHspi,  DispImageMiddle);
               }
            else // enPrintInspection || 3rd show case of enBoardInspection
               {
               MbufCopy(m_ReferenceImage, DispImageLeft);
               MbufCopy(m_TrainingImage,  DispImageMiddle);

               // Annotation of perceptive calibration using graphic list
               if (enPrintInspection == enApplicationCase)
                  {
                  MdispControl(DisplayId, M_ASSOCIATED_GRAPHIC_LIST_ID, m_TrainingGraListId);
                  }
               }
            MbufCopy(m_TrainingImageCalibrated, DispImageRight);
            PrintMessage(DisplayId, enApplicationCase, enShowPreprocessingInfo, enShowCase);

            // Remove the graphic list overlay
            MdispControl(DisplayId, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
            break;
            }

         case enShowCalibrationResults:
            {
            // Display images
            if (enFoodInspection == enApplicationCase)
               {
               MbufCopy(m_ReferenceImageForApplyHspi, DispImageLeft);
               }
            else if (enPrintInspection == enApplicationCase)
               {
               MIL_INT    ReferenceImageSizeX = MbufInquire(m_ReferenceImage, M_SIZE_X, M_NULL);
               MIL_DOUBLE ResizeCoef          = (MIL_DOUBLE)m_DisplaySampleSizeX / (MIL_DOUBLE)ReferenceImageSizeX;
               MimResize(m_ReferenceImage, DispImageLeft, M_FILL_DESTINATION, ResizeCoef, M_DEFAULT);
               }
            else // enBoardInspection
               {
               MbufCopy(m_ReferenceImage, DispImageLeft);
               }

            MbufCopy(m_ImageToBeCalibrated, DispImageMiddle);
            MbufCopy(m_ImageCalibrated,     DispImageRight);

            // Refresh display
            MdispControl(DisplayId, M_UPDATE, M_ENABLE);
            PrintMessage(DisplayId, enApplicationCase, enShowCalibrationResults, enShowCase);
            break;
            }

         default:
            {
            MosPrintf(MIL_TEXT("Invalid display option!"));
            break;
            }
         }

      MdispControl(DisplayId, M_UPDATE, M_ENABLE);

      // Free local buffers
      MbufFree(DispImageRight);
      MbufFree(DispImageMiddle);
      MbufFree(DispImageLeft);
      }
   }

void CColorCalibrationDemo::PrintMessage(MIL_ID                 DisplayId, 
                                         ApplicationCaseEnum    enApplicationCase, 
                                         DispalyOptionEnum      enDisplayOption,
                                         ColorRenderingCaseEnum enShowCase)
   {
   MIL_ID OverlayId;
   MdispInquire(DisplayId, M_OVERLAY_ID, &OverlayId);

   switch(enApplicationCase)
      {
      case enFoodInspection:
         {
         if(enShowPreprocessingInfo == enDisplayOption)
            {
            MosPrintf(MIL_TEXT("\n Press any key to continue...                                     \n"));
            m_ReferenceImageLabel  = MIL_TEXT("Conveyor 1 (reference)");
            m_SampleImageLabel     = MIL_TEXT("Conveyor 2 (training)");
            m_CalibratedImageLabel = MIL_TEXT("Color-calibrated conveyor 2");
            }
         else if(enShowCalibrationResults == enDisplayOption)
            {
            MosPrintf(MIL_TEXT("\n    - The transformation is then applied to newly grabbed images  "  ));
            MosPrintf(MIL_TEXT("\n      on conveyor 2 so that the color image appearances between   "  ));
            MosPrintf(MIL_TEXT("\n      the two conveyors stay coherent.                            \n"));
            MosPrintf(MIL_TEXT("\n Press any key to continue next use case...                       \n"));
            m_ReferenceImageLabel  = MIL_TEXT("Conveyor 1");
            m_SampleImageLabel     = MIL_TEXT("Conveyor 2");
            m_CalibratedImageLabel = MIL_TEXT("Color-calibrated conveyor 2");
            }
         else
            MosPrintf(MIL_TEXT("\nInvalid sample option!"));

         // Update display
         MgraText(M_DEFAULT, OverlayId, m_LeftSampleTextPosX,   TEXT_POSITION, m_ReferenceImageLabel);
         MgraText(M_DEFAULT, OverlayId, m_MiddleSampleTextPosX, TEXT_POSITION, m_SampleImageLabel);
         MgraText(M_DEFAULT, OverlayId, m_RightSampleTextPosX,  TEXT_POSITION, m_CalibratedImageLabel);

         break;
         }

      case enPrintInspection:
         {
         m_ReferenceImageLabel = MIL_TEXT("Reference image (camera 1, illuminant 1)");
         if(enShowPreprocessingInfo == enDisplayOption)
            {
            MosPrintf(MIL_TEXT("\n Press any key to continue...                                    \n"));
            m_SampleImageLabel     = MIL_TEXT("Training Image (camera 2, illuminant 2)");
            m_CalibratedImageLabel = MIL_TEXT("Color-calibrated result");
            }
         else if(enShowCalibrationResults == enDisplayOption)
            {
            MosPrintf(MIL_TEXT("\n    - Another transformation is applied to newly grabbed images  "  ));
            MosPrintf(MIL_TEXT("\n      so that the color image appearances stay coherent.         \n"));
            MosPrintf(MIL_TEXT("\n Press any key to continue next use case...                      \n"));
            m_SampleImageLabel     = MIL_TEXT("Image to be calibrated (camera 2, illuminant 2)");
            m_CalibratedImageLabel = MIL_TEXT("Color-calibrated result");
            }
         else
            MosPrintf(MIL_TEXT("\nInvalid sample option!"));

         // Update display
         MgraText(M_DEFAULT, OverlayId, m_LeftSampleTextPosX,   TEXT_POSITION, m_ReferenceImageLabel);
         MgraText(M_DEFAULT, OverlayId, m_MiddleSampleTextPosX, TEXT_POSITION, m_SampleImageLabel);
         MgraText(M_DEFAULT, OverlayId, m_RightSampleTextPosX,  TEXT_POSITION, m_CalibratedImageLabel);
         break;
         }
      case enBoardInspection:
         {
         if (enShowPreprocessingInfo == enDisplayOption)
            {
            if (enFirstShowCase == enShowCase)
               {
               MosPrintf(MIL_TEXT("\n Press any key to continue...                                 \n"));
               }
            else if (enSecondShowCase == enShowCase)
               {
               MosPrintf(MIL_TEXT("\n    - The same operation is performed, but using a different  "  ));
               MosPrintf(MIL_TEXT("\n      reference image.                                        \n"));
               MosPrintf(MIL_TEXT("\n Press any key to continue...                                 \n"));
               }

            m_SampleImageLabel     = MIL_TEXT(" Mosaic image from 6 different illuminants ");
            m_CalibratedImageLabel = MIL_TEXT(" Mosaic image using color-calibrated results ");

            MIL_INT ImageSizeX = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_X, M_NULL);
            MIL_INT ImageSizeY = MbufDiskInquire(MVA_SAMPLE_LIST_FILENAME[0], M_SIZE_Y, M_NULL);
            MIL_INT ImageChildSizeX = ImageSizeX / NUM_MVA_SAMPLES;

            // Update display
            MgraColor(M_DEFAULT, M_COLOR_BLUE);
            MgraRectAngle(M_DEFAULT, OverlayId, ImageChildSizeX * m_ReferenceIndexForMva, 0, ImageChildSizeX - 1, ImageSizeY, 0, M_DEFAULT);
            MgraColor(M_DEFAULT, M_COLOR_CYAN);

            // Update display
            MgraText(M_DEFAULT, OverlayId, TEXT_POSITION, TEXT_POSITION, m_SampleImageLabel);
            MgraText(M_DEFAULT, OverlayId, TEXT_POSITION, ImageSizeY + TEXT_POSITION, m_CalibratedImageLabel);
            }
         else if (enShowCalibrationResults == enDisplayOption)
            {
            MosPrintf(MIL_TEXT("\n    - Note that this method does not require data alignment nor image     "  ));
            MosPrintf(MIL_TEXT("\n      content similarity. An image with completely different content is   "  ));
            MosPrintf(MIL_TEXT("\n      used to demonstrate the generality of this method. After applying   "  ));
            MosPrintf(MIL_TEXT("\n      color-relative calibration, the natural scene image is rendered with"  ));
            MosPrintf(MIL_TEXT("\n      the global color distribution information of the electronic board   \n"));
            MosPrintf(MIL_TEXT("\n Press any key to terminate...                                            \n"));
            m_ReferenceImageLabel  = MIL_TEXT("Reference image");
            m_SampleImageLabel     = MIL_TEXT("Image of different content");
            m_CalibratedImageLabel = MIL_TEXT("Color-calibrated image");

            // Update display
            MgraText(M_DEFAULT, OverlayId, m_LeftSampleTextPosX,   TEXT_POSITION, m_ReferenceImageLabel);
            MgraText(M_DEFAULT, OverlayId, m_MiddleSampleTextPosX, TEXT_POSITION, m_SampleImageLabel);
            MgraText(M_DEFAULT, OverlayId, m_RightSampleTextPosX,  TEXT_POSITION, m_CalibratedImageLabel);
            }
         else
            MosPrintf(MIL_TEXT("\nInvalid sample option!"));

         break;
         }

      default:
         {
         MosPrintf(MIL_TEXT("\nInvalid application case!"));
         break;;
         }
      }

   MosGetch();
   }

static void DrawCross(MIL_ID GraphicContext, MIL_ID Dest, MIL_INT NbCross, MIL_DOUBLE* pX, MIL_DOUBLE* pY)
   {
   const MIL_DOUBLE CrossLength = 5.0;

   for(MIL_INT c = 0; c < NbCross; c++)
      {
      MgraLine(GraphicContext, Dest, pX[c] - CrossLength, pY[c]              ,
                                     pX[c] + CrossLength, pY[c]              );
      MgraLine(GraphicContext, Dest, pX[c]              , pY[c] - CrossLength,
                                     pX[c]              , pY[c] + CrossLength);
      }
   }

void CColorCalibrationDemo::DefineSampleForColorChecker(MIL_ID              SystemId,
                                                        ApplicationCaseEnum enApplicationCase,
                                                        MIL_ID              SampleId, 
                                                        MIL_INT             SampleLabelOrIndex)
   {
   MIL_INT    NbOfRow     = 10;           
   MIL_INT    NbOfCol     = 14;           
   MIL_INT    PatchSize   = 10;
   MIL_INT    PatchRadius = 5;
   MIL_DOUBLE PatchWidth  = 1.0 / NbOfCol;
   MIL_DOUBLE PatchHeight = 1.0 / NbOfRow;

   // Allocate a calibration context
   MIL_ID CalibrationContext = McalAlloc(SystemId, M_PERSPECTIVE_TRANSFORMATION, M_DEFAULT, M_NULL);

   // See theoretical grid coordinates
   MIL_DOUBLE WorldGridX[4] = { 0.0, 1.0, 0.0, 1.0 };
   MIL_DOUBLE WorldGridY[4] = { 0.0, 0.0, 1.0, 1.0 };
   MIL_DOUBLE WorldGridZ[4] = { 0.0, 0.0, 0.0, 0.0 };
   MIL_DOUBLE PixelGridX[4] = { 0.0, 0.0, 0.0, 0.0 };
   MIL_DOUBLE PixelGridY[4] = { 0.0, 0.0, 0.0, 0.0 };

   // Hard-coded pixel coordinates for reference sample
   if(M_REFERENCE_SAMPLE == SampleLabelOrIndex)
      {
      // 4 corners: (81, 35), (490, 24), (83, 352), (488, 342)
      PixelGridX[0] = 81.0;
      PixelGridX[1] = 490.0;
      PixelGridX[2] = 83.0; 
      PixelGridX[3] = 488.0;

      PixelGridY[0] = 35.0;
      PixelGridY[1] = 24.0;
      PixelGridY[2] = 325.0; 
      PixelGridY[3] = 342.0;

      }
   else // for training sample
      {
      // 4 corners: (40, 28), (484, 34), (43, 345), (476, 342)
      PixelGridX[0] = 40.0;
      PixelGridX[1] = 484.0;
      PixelGridX[2] = 43.0; 
      PixelGridX[3] = 476.0;

      PixelGridY[0] = 28.0;
      PixelGridY[1] = 34.0;
      PixelGridY[2] = 345.0; 
      PixelGridY[3] = 342.0;

      // Allocate a graphic list for annotation
      MgraAllocList(SystemId, M_DEFAULT, &m_TrainingGraListId);
      }

   // Set coordinates
   McalList(CalibrationContext,
            PixelGridX,
            PixelGridY,
            WorldGridX,
            WorldGridY,
            WorldGridZ,
            4,
            M_DEFAULT,
            M_DEFAULT);

   // Associate calibration to image
   McalAssociate(CalibrationContext, SampleId, M_DEFAULT);

   MIL_DOUBLE XPos[NUM_COLOR_PATCH];
   MIL_DOUBLE YPos[NUM_COLOR_PATCH];

   MIL_ID  SampleChildId = MbufChild2d(SampleId, 0, 0, PatchSize, PatchSize, M_NULL);
   MIL_INT OffsetForGraphicList = MbufInquire(SampleId, M_SIZE_X, M_NULL);
   MIL_INT PatchIdx = 0;
   for(MIL_INT RowIdx = 0; RowIdx < NbOfRow; RowIdx++)
      {
      for(MIL_INT ColIdx = 0; ColIdx < NbOfCol; ColIdx++)
         {
         MIL_DOUBLE WorldPosY = (RowIdx + 0.5) * PatchHeight;
         MIL_DOUBLE WorldPosX = (ColIdx + 0.5) * PatchWidth;
         MIL_DOUBLE PixelPosX = 0.0;
         MIL_DOUBLE PixelPosY = 0.0;
         McalTransformCoordinate(SampleId, M_WORLD_TO_PIXEL, WorldPosX, WorldPosY, &PixelPosX, &PixelPosY);

         MbufChildMove(SampleChildId,
                      (MIL_INT)PixelPosX - PatchRadius,
                      (MIL_INT)PixelPosY - PatchRadius,
                       PatchSize,
                       PatchSize,
                       M_DEFAULT);

         MIL_INT SampleType = ( PatchIdx == 0 ) ? M_IMAGE : M_IMAGE + M_ADD_COLOR_TO_SAMPLE;
         McolDefine(m_ColorCalibrationContext, 
                    SampleChildId, 
                    SampleLabelOrIndex, 
                    SampleType, 
                    M_DEFAULT,
                    M_DEFAULT,
                    M_DEFAULT,
                    M_DEFAULT);

         // Set coordinates into graphic list
         XPos[PatchIdx] = PixelPosX + OffsetForGraphicList;
         YPos[PatchIdx] = PixelPosY;
         PatchIdx++;
         }
      }

   if (M_REFERENCE_SAMPLE != SampleLabelOrIndex)
      {
      DrawCross(M_DEFAULT, m_TrainingGraListId, NUM_COLOR_PATCH, XPos, YPos);
      }

   MbufFree(SampleChildId);
   McalFree(CalibrationContext);
   }
