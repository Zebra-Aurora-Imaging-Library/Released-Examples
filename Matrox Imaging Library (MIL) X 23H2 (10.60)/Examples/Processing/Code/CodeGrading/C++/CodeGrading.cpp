﻿/***************************************************************************************/
/* 
* File name: CodeGrading.cpp  
*
* Synopsis:  This program contains an example of code grading for different types of 
*            linear Code, Composite code and Cross-Row, using the Code Reader module.
*            See the PrintHeader() function below for a detailed description.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

#define SAVE_PATH MIL_TEXT("")

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("CodeGrading\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program grades different types of Linear codes, Composite codes,\n"));
   MosPrintf(MIL_TEXT("Cross-row codes and 2D matrix codes.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, graphic, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/* Number of Linear codes */
static const MIL_INT NUMBER_OF_IMAGES               = 14L;

/* Height of Buffer used for reflectance profile */
static const MIL_INT REFLECTANCE_PROFILE_HEIGHT     = 256L;


/* Linear Code images */
static MIL_CONST_TEXT_PTR CodeFileName[NUMBER_OF_IMAGES] =
   {
   M_IMAGE_PATH MIL_TEXT("CodeGrading/UPCA_Decode.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/UPCA_SC.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/UPCE_Rmin.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/EAN14_ECmin.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/Code39_Modulation.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/UPCA_Defects.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/UPCE_Decodability.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/Code93_QuietZone.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/PDF417_UEC.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/TruncatedPDF417_2DDecodability.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/PDF417_2DDefects.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/Composite_Defects.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/DataMatrix.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeGrading/QrCode.mim")
   };


static const MIL_INT CodeType[NUMBER_OF_IMAGES] = 
   {
   M_UPC_A,
   M_UPC_A,
   M_UPC_E,
   M_EAN14,
   M_CODE39,
   M_UPC_A,
   M_UPC_E,
   M_CODE93,
   M_PDF417,
   M_TRUNCATED_PDF417,
   M_PDF417,
   M_COMPOSITECODE,
   M_DATAMATRIX,
   M_QRCODE
   };

/* Result Per Scan Reflectance profile */
struct SCAN_REFLECTANCE_PROFILE_RESULT_ST
   {
   
   MIL_DOUBLE ScanReflectanceProfileGrade;
   MIL_INT DecodeGrade;
   MIL_DOUBLE SymbolContrastGrade;     
   MIL_INT ReflectanceMinimumGrade;
   MIL_INT EdgeContrastMinimumGrade;
   MIL_DOUBLE ModulationGrade;
   MIL_DOUBLE DefectsGrade;
   MIL_DOUBLE DecodabilityGrade;
   MIL_INT QuietZoneGrade;
   MIL_INT GuardPatternGrade;
   MIL_INT WideToNarrowRatioGrade;
   MIL_INT InterCharacterGapGrade;

   double  SymbolContrast;     
   double  ReflectanceMinimum;
   double  EdgeContrastMinimum;
   double  Modulation;
   double  Defects;
   double  Decodability;
   double  QuietZone;
   double  GuardPattern;
   double  WideToNarrowRatio;
   double  InterCharacterGap;
   double  MinimumReflectanceMargin;
   double  EdgeDeterminationWarning;
   };

struct ROW_RESULT_ST
   {
   double RowOverallGrade;
   SCAN_REFLECTANCE_PROFILE_RESULT_ST *ScanResults;
   };

class MATRIX_RESULT_ST
   {
public:
   MATRIX_RESULT_ST();

   MIL_INT DecodeGrade;
   MIL_INT UnusedErrorCorrectionGrade;
   MIL_INT AxialNonUniformityGrade;
   MIL_INT GridNonUniformityGrade;
   MIL_INT FixedPatternDamageGrade;
   MIL_INT FormatInformationGrade;
   MIL_INT VersionInformationGrade;
   MIL_INT SymbolContrastGrade;
   MIL_INT ModulationGrade;
   MIL_INT CellModulationGrade;
   MIL_INT CellContrastGrade;
   MIL_INT MinimumReflectanceGrade;
   
   double UnusedErrorCorrection;
   double PrintGrowth;
   double AxialNonUniformity;
   double GridNonUniformity;
   double SymbolContrast;
   double CellContrast;
   double MinimumReflectance;
   };

class SEMIT10_RESULT_ST
   {
   public:
      SEMIT10_RESULT_ST();

      double P1X;
      double P1Y;
      double P2X;
      double P2Y;
      double P3X;
      double P3Y;
      double P4X;
      double P4Y;
      MIL_INT NbColumns;
      MIL_INT NbRows;
      double SymbolContrast;
      double SymbolContrastSNR;
      double HorizontalMarkGrowth;
      double VerticalMarkGrowth;
      double CellWidth;
      double CellHeight;
      double HorizontalMarkMisplacement;
      double VerticalMarkMisplacement;
      MIL_INT NumberOfInterleavedBlocks;
      std::vector<double> UnusedErrorCorrection;
      double CellDefects;
      double FinderPatternDefects;
   };

/* Container of dynamic 2D array */
class GRADE_ARRAY
   {
   public:
      GRADE_ARRAY();
      ~GRADE_ARRAY();

      void AllocateRows(MIL_INT NumberOfRows);
      void AllocateScans(MIL_INT RowIndex, MIL_INT NumberOfScans);
      SCAN_REFLECTANCE_PROFILE_RESULT_ST *GetScanResultPtr(MIL_INT RowIndex, MIL_INT ScanIndex);
      const SCAN_REFLECTANCE_PROFILE_RESULT_ST &GetScanResult(MIL_INT RowIndex, MIL_INT ScanIndex) const;
      ROW_RESULT_ST *GetRowResultPtr(MIL_INT RowIndex);
      void Free();   

   private:
      ROW_RESULT_ST *m_RowResults;
      MIL_INT m_NumberOfRows;
   };

/* Grade result structure */
class GRADE_RESULT_ST
   {
public:
   GRADE_RESULT_ST();

   MIL_INT CodeType;
   MIL_INT Encoding;

   double OverallGrade;

   /* Worst grade */
   MIL_INT WorstScanIndex;
   MIL_INT WorstRowIndex;
   MIL_INT WorstScanIndexOffset;
   MIL_DOUBLE WorstGrade;

   /* Result Per Row */
   GRADE_ARRAY RowResults;

   /* Results specific to Cross-row component */
   double  StartStopGrade;
   MIL_INT CodewordYieldGrade;  
   MIL_INT ModulationGrade;
   MIL_INT DecodabilityGrade;
   MIL_INT DefectsGrade;
   MIL_INT UnusedErrorCorrectionGrade;
   double  CodewordYield;
   double  UnusedErrorCorrection;

   /* Results specific to 2D matrix */
   MATRIX_RESULT_ST MatrixResults;

   /* Results specific for SemiT10 grading */
   SEMIT10_RESULT_ST SemiT10Results;

   void InitializeWorstGrade();
   void UpdateWorstGrade(MIL_INT RowIndex, MIL_INT ScanIndex, MIL_INT ScanIndexOffset, MIL_DOUBLE Grade);
   MIL_INT GetWorstCrossRowGrade();
   };

enum CodeTypeCategory
   {
   IS_1D_CODE,
   IS_CROSS_ROW_CODE,
   IS_COMPOSITE_CODE,
   IS_2D_MATRIX_CODE,
   IS_NOT_SUPPORTED_BY_THIS_EXAMPLE
   };

#define min(a,b)    (((a) < (b)) ? (a) : (b))

/* Maximum length of the string to read. */
#define STRING_LENGTH_MAX              70L


/* Function declarations. */

/* General grading function. */
void CodeGrading(MIL_ID MilSystem, MIL_ID MilSrcImage, MIL_ID MilDisplay, MIL_INT CodeType, bool GradeAfterRead, bool SemiT10Grading, bool PreviousGradingEdition);

/* Sub-functions that extract the grading results from the MIL code result object. */
void GetAndDisplayResultsFor1DCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor);
void GetAndDisplayResultsForCrossRowCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor);
void GetAndDisplayResultsFor2DMatrixCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage);
void GetAndDisplaySemiT10Results(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage);

void GetResultForRow(MIL_ID MilCodeResult, MIL_INT OccurrenceIndex, MIL_INT RowIndex, MIL_INT ScanIndexOffset, MIL_INT NumberOfScans, GRADE_RESULT_ST* GradingResult);

/* Sub-functions that display and print results. */
void DisplayScanReflectanceProfileOfWorstGrade(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor, const GRADE_RESULT_ST& GradingResult);
void PrintScanWorstGrade(const GRADE_RESULT_ST& GradingResult);
void PrintCrossRowGrade(const GRADE_RESULT_ST& GradingResult, MIL_INT WorstGrade);
void Print2DMatrixGrade(const GRADE_RESULT_ST& GradingResult);
void PrintSemiT10Grade(const GRADE_RESULT_ST& GradingResult);

/* Utility sub-functions. */
CodeTypeCategory GetCodeTypeCategory(MIL_INT CodeType);
MIL_INT Get1DCodeTypeOfCompositeCode(MIL_INT Encoding);
MIL_CONST_TEXT_PTR GetCodeTypeString(const GRADE_RESULT_ST& GradingResult);
MIL_CONST_TEXT_PTR GetGradingStandardEditionString(MIL_INT GradingStandardEdition);
MIL_CONST_TEXT_PTR GetGradeString(double Grade);

void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, double  Grade);
void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, MIL_INT Grade);
void PrintGrade(MIL_CONST_TEXT_PTR Text, double  Grade);
void PrintGrade(MIL_CONST_TEXT_PTR Text, MIL_INT Grade);
void PrintValue(MIL_CONST_TEXT_PTR Text, double  Grade);



/*****************************************************************************
Main.
*****************************************************************************/
int MosMain(void)
   {
   MIL_ID      MilApplication,         /* Application identifier.                      */
               MilSystem,              /* System identifier.                           */
               MilDisplay;             /* Display identifier.                          */

   bool        GradeAfterRead;         /* Read first then grade using the read result  */
   bool        SemiT10Grading;         /* Grade according to Semi T10-0701 */
   bool        PreviousGradingEdition; /* Grade according to previous edition of 15416 */

   /* Allocate MIL objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Print Header. */
   PrintHeader();

   /********************/
   /* BARCODE GRADING: */
   /********************/
   GradeAfterRead = false;
   PreviousGradingEdition = false;

   for(MIL_INT ii = 0; ii < NUMBER_OF_IMAGES; ii++)
      {
      /* Restore source image into an automatically allocated image buffer. */
      MIL_ID MilSrcImage;
      MbufRestore(CodeFileName[ii], MilSystem, &MilSrcImage);

      /* For the last image, the code is read first, then it is graded */
      if (ii==NUMBER_OF_IMAGES-1)
         GradeAfterRead = true;

      /* For the first image, grade according to ISO/IEC 15416:2000 */
      if(ii == 0)
         PreviousGradingEdition = true;
      else
         PreviousGradingEdition = false;

      SemiT10Grading = false;
      CodeGrading(MilSystem, MilSrcImage, MilDisplay, CodeType[ii], GradeAfterRead, SemiT10Grading, PreviousGradingEdition);

      if(CodeType[ii] == M_DATAMATRIX)
         {
         SemiT10Grading = true;
         CodeGrading(MilSystem, MilSrcImage, MilDisplay, CodeType[ii], GradeAfterRead, SemiT10Grading, PreviousGradingEdition);
         }

      /* Free source image. */
      MbufFree(MilSrcImage);
      }

   /* Free other allocations. */
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);


   return 0;
   }

/************************************************************************
  CodeGrading

  Calculate the grading results of a code using MIL; display and
  print the results.
************************************************************************/
void CodeGrading(MIL_ID MilSystem, MIL_ID MilSrcImage, MIL_ID MilDisplay, MIL_INT CodeType, bool GradeAfterRead, bool SemiT10Grading, bool PreviousGradingEdition)
   {
   /* Allocate a display buffer and show the source image. */
   MIL_INT SrcImageWidth, SrcImageHeight, SrcImageType;
   MbufInquire(MilSrcImage, M_SIZE_X, &SrcImageWidth);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SrcImageHeight);
   MbufInquire(MilSrcImage, M_TYPE,   &SrcImageType);

   MIL_ID MilDisplayImage;
   MbufAlloc2d(MilSystem, 
               SrcImageWidth,
               SrcImageHeight + REFLECTANCE_PROFILE_HEIGHT,
               SrcImageType,
               M_IMAGE+M_PROC+M_DISP, 
               &MilDisplayImage);

   MbufClear(MilDisplayImage, 0L);
   MbufCopy(MilSrcImage, MilDisplayImage);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilDisplayImage);

   /* Prepare for overlay annotations. */
   MIL_ID MilOverlayImage;
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Determine the type categories of the code. */
   CodeTypeCategory ComponentType = GetCodeTypeCategory(CodeType);
   if(ComponentType == IS_NOT_SUPPORTED_BY_THIS_EXAMPLE)
      {
      /* Free source image. */
      MbufFree(MilDisplayImage);
      MbufFree(MilSrcImage);
      }

   /* Allocate CODE objects. */
   MIL_ID MilCodeContext;
   McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilCodeContext);
   McodeControl(MilCodeContext, M_TIMEOUT, 10000.0);
   MIL_ID ModelId = McodeModel(MilCodeContext, M_ADD, CodeType, M_NULL, M_DEFAULT, M_NULL);

   if(PreviousGradingEdition)
      {
      /* Grade according to ISO/IEC 15416:2000. */
      if(ComponentType == IS_1D_CODE)
         McodeControl(ModelId, M_GRADING_STANDARD_EDITION, M_ISO_15416_2000);
      else
         McodeControl(ModelId, M_GRADING_STANDARD_EDITION, M_ISO_15415_2011_15416_2000);
      }
   else
      {
      /* Grade according to latest edition of ISO/IEC or SEMI. */
      McodeControl(ModelId, M_GRADING_STANDARD_EDITION, M_DEFAULT);

      /* Grade according to Semi T10 Standard. */
      if(SemiT10Grading)
         {
         McodeControl(ModelId, M_DECODE_ALGORITHM, M_CODE_DEFORMED);
         McodeControl(MilCodeContext, M_GRADING_STANDARD, M_SEMI_T10_GRADING);
         }
      }

   /* Prepare bar code result buffer */
   MIL_ID MilCodeResult;
   McodeAllocResult(MilSystem, M_DEFAULT, &MilCodeResult);

   McodeControl(MilCodeContext, M_INITIALIZATION_MODE, M_IMPROVED_RECOGNITION);

   /* Grade codes from image. */
   if (GradeAfterRead)
      {
      MIL_ID MilReadCodeResult;
      MIL_INT CodeNb;

      /* Allocate a result buffer for the read operation */
      McodeAllocResult(MilSystem, M_DEFAULT, &MilReadCodeResult);

      /* Read */
      McodeRead(MilCodeContext, MilSrcImage, MilReadCodeResult);

      /* Check that a code was successfully read before calling the grading process */
      McodeGetResult(MilReadCodeResult, M_GENERAL, M_GENERAL, M_NUMBER+M_TYPE_MIL_INT, &CodeNb);
      if (CodeNb == 1)
         {
         MIL_TEXT_CHAR  BarcodeString[STRING_LENGTH_MAX];
         /* Get the decoded string (for display purposes only, not required for grading) */
         McodeGetResult(MilReadCodeResult, 0, M_GENERAL, M_STRING, BarcodeString);

         MosPrintf(MIL_TEXT("\nThe result object of a previously read code can be used as input for\n"));
         MosPrintf(MIL_TEXT("a subsequent grade operation. The internal read operation of the grade\n"));
         MosPrintf(MIL_TEXT("operation is then skipped.\n\n"));
         MosPrintf(MIL_TEXT("The code is first read: %s\nThe code is then graded using the read result:\n"), BarcodeString);

         /* Grade using McodeRead results to save time. */
         McodeGrade(MilCodeContext, MilSrcImage, MilReadCodeResult, M_DEFAULT, MilCodeResult, M_DEFAULT);

         }
      /* Free the read result buffer*/
      McodeFree(MilReadCodeResult);
      }
   else
      McodeGrade(MilCodeContext, MilSrcImage, M_NULL, M_DEFAULT, MilCodeResult, M_DEFAULT);

   /* Get grading status. */
   MIL_INT Status;
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &Status);

   /* Check if the gradation operation was successful. */
   if(Status == M_STATUS_GRADE_OK)
      {
      /* Show that read results are available after grading even when no McodeRead call is 
         made before. Show only with first code. */
      if(!GradeAfterRead)
         {
         MIL_INT CodeNb;
         McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &CodeNb);
         if(CodeNb == 1)
            {
            MIL_TEXT_CHAR  BarcodeString[STRING_LENGTH_MAX];
            /* Get the decoded string. */
            McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, BarcodeString);

            MosPrintf(MIL_TEXT("\nThe output result object of McodeGrade can be used to get reading \n"));
            MosPrintf(MIL_TEXT("results directly without calling McodeRead.\n\n"));
            MosPrintf(MIL_TEXT("The code read is: %s\n\n"), BarcodeString);
            }
         }

      MgraColor(M_DEFAULT, M_COLOR_GREEN);

      if(ComponentType != IS_2D_MATRIX_CODE)
         {
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_SCAN_PROFILES, M_ALL, M_ALL, M_DEFAULT);
         }
      else
         {
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX + M_DRAW_POSITION + M_DRAW_QUIET_ZONE, M_ALL, M_GENERAL, M_DEFAULT);
         }

      MIL_INT GradingStandardEdition;
      McodeInquire(ModelId, M_GRADING_STANDARD_EDITION + M_TYPE_MIL_INT, &GradingStandardEdition);
      if(GradingStandardEdition == M_DEFAULT)
         McodeInquire(ModelId, M_GRADING_STANDARD_EDITION + M_DEFAULT + M_TYPE_MIL_INT, &GradingStandardEdition);

      MosPrintf(MIL_TEXT("Grading Standard Used:   %s\n"), GetGradingStandardEditionString(GradingStandardEdition));

      switch(ComponentType)
         {
         case IS_1D_CODE:
            GetAndDisplayResultsFor1DCode(MilSystem, MilCodeResult, MilOverlayImage, 0);
            break;
         case IS_CROSS_ROW_CODE:
            GetAndDisplayResultsForCrossRowCode(MilSystem, MilCodeResult, MilOverlayImage, 0);
            break;
         case IS_COMPOSITE_CODE:
            GetAndDisplayResultsFor1DCode(MilSystem, MilCodeResult, MilOverlayImage, M_LINEAR_COMPONENT);
            MosPrintf(MIL_TEXT("Press <Enter> to get the 2D part.\n\n"));
            MosGetch();
            GetAndDisplayResultsForCrossRowCode(MilSystem, MilCodeResult, MilOverlayImage, M_2D_COMPONENT);
            break;

         case IS_2D_MATRIX_CODE:
            if(SemiT10Grading)
               GetAndDisplaySemiT10Results(MilSystem, MilCodeResult, MilOverlayImage);
            else
               GetAndDisplayResultsFor2DMatrixCode(MilSystem, MilCodeResult, MilOverlayImage);
            break;
         }

      /* Saves a report containing most of the results from a grade operation as a flat text file. */
      MIL_TEXT_PTR pOutFilename = const_cast<MIL_TEXT_PTR>(SAVE_PATH MIL_TEXT("ReportFile.txt"));
      McodeStream(pOutFilename, M_NULL, M_SAVE_REPORT, M_FILE, M_DEFAULT, M_DEFAULT, &MilCodeResult, M_NULL);
      MosPrintf(MIL_TEXT("A grading report was saved in ReportFile.txt\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("Code grading operation failed.\n\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free Display image. */
   MbufFree(MilDisplayImage);

   /* Free objects. */
   McodeFree(MilCodeResult);
   McodeFree(MilCodeContext);
   }


/************************************************************************
  GetAndDisplayResultsFor1DCode

  Extracts the grading results for a 1D code from a MIL code result
  and display them.
************************************************************************/
void GetAndDisplayResultsFor1DCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor)
   {
   MIL_INT ScanIndexOffset = 0;
   MIL_INT OccurrenceIndex = 0;
   GRADE_RESULT_ST GradingResult;
   
   /* Initialize the worst grade. */
   GradingResult.InitializeWorstGrade();

   /* Get the code type. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &(GradingResult.CodeType));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_ENCODING  + M_TYPE_MIL_INT, &(GradingResult.Encoding));

   if(GradingResult.CodeType == M_COMPOSITECODE)
      {
      GradingResult.CodeType = Get1DCodeTypeOfCompositeCode(GradingResult.Encoding);
      }

   /* Get the overall grade. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_OVERALL_SYMBOL_GRADE + Accessor, &(GradingResult.OverallGrade));

   /* Get number of Rows. */
   /* Note, Accessor is used for composite code to identify LINEAR part (if part of a composite code) */
   MIL_INT NumberOfRows;
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_NUMBER_OF_ROWS + Accessor + M_TYPE_MIL_INT, &NumberOfRows);
   
   /* Allocate memory for each row. */
   GradingResult.RowResults.AllocateRows(NumberOfRows);

   /* Get results for each row. */
   for(MIL_INT RowIndex = 0; RowIndex < NumberOfRows; RowIndex++)
      {
      /* Get number of scans per row. */
      MIL_INT NumberOfScans;
      McodeGetResult(MilCodeResult, OccurrenceIndex, RowIndex, M_ROW_NUMBER_OF_SCANS + Accessor + M_TYPE_MIL_INT, &NumberOfScans);

      /* Allocate memory to hold results of scan. */
      GradingResult.RowResults.AllocateScans(RowIndex, NumberOfScans);

      /* Get grading of the current row. */
      GetResultForRow(MilCodeResult, OccurrenceIndex, RowIndex, ScanIndexOffset, Accessor, &GradingResult);

      /* Increment the scan profile index. */
      ScanIndexOffset += NumberOfScans;
      }

   /* Display scan reflectance profile. */
   DisplayScanReflectanceProfileOfWorstGrade(MilSystem, MilCodeResult, MilOverlayImage, Accessor, GradingResult);

   /* Print worst grade. */
   PrintScanWorstGrade(GradingResult);

   /* Free allocated memory. */
   GradingResult.RowResults.Free();
   }

/************************************************************************
  GetResultForRow

  Extracts the grading results for a row of a 1D code from a 
  MIL code result.
************************************************************************/
void GetResultForRow(MIL_ID MilCodeResult, MIL_INT OccurrenceIndex, MIL_INT RowIndex, MIL_INT ScanIndexOffset, MIL_INT Accessor, GRADE_RESULT_ST* GradingResult)
   {
   /* Get number of scans per row. */
   MIL_INT NumberOfScans;
   McodeGetResult(MilCodeResult, OccurrenceIndex, RowIndex, M_ROW_NUMBER_OF_SCANS + Accessor + M_TYPE_MIL_INT, &NumberOfScans);
  
   ROW_RESULT_ST *CurrentRowResult = GradingResult->RowResults.GetRowResultPtr(RowIndex);

   /* Save the Row Overall Grade. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, RowIndex, M_ROW_OVERALL_GRADE + Accessor, &(CurrentRowResult->RowOverallGrade));

   /* Get results for each scan in this row. */
   for(MIL_INT ScanIndex = 0; ScanIndex < NumberOfScans; ScanIndex++)
      {
      SCAN_REFLECTANCE_PROFILE_RESULT_ST *CurrentScanResult = GradingResult->RowResults.GetScanResultPtr(RowIndex, ScanIndex);

      /* Get some values and grading. */
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_REFLECTANCE_PROFILE_GRADE + Accessor, &(CurrentScanResult->ScanReflectanceProfileGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_DECODE_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->DecodeGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_SYMBOL_CONTRAST_GRADE + Accessor, &(CurrentScanResult->SymbolContrastGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_REFLECTANCE_MINIMUM_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->ReflectanceMinimumGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_EDGE_CONTRAST_MINIMUM_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->EdgeContrastMinimumGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_MODULATION_GRADE + Accessor, &(CurrentScanResult->ModulationGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_DEFECTS_GRADE + Accessor, &(CurrentScanResult->DefectsGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_DECODABILITY_GRADE + Accessor, &(CurrentScanResult->DecodabilityGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_QUIET_ZONE_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->QuietZoneGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_GUARD_PATTERN_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->GuardPatternGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_WIDE_TO_NARROW_RATIO_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->WideToNarrowRatioGrade));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_INTERCHARACTER_GAP_GRADE + Accessor + M_TYPE_MIL_INT, &(CurrentScanResult->InterCharacterGapGrade));
                                                    
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_SYMBOL_CONTRAST + Accessor, &(CurrentScanResult->SymbolContrast));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_REFLECTANCE_MINIMUM + Accessor, &(CurrentScanResult->ReflectanceMinimum));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_EDGE_CONTRAST_MINIMUM + Accessor, &(CurrentScanResult->EdgeContrastMinimum));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_MODULATION + Accessor, &(CurrentScanResult->Modulation));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_DEFECTS + Accessor, &(CurrentScanResult->Defects));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_DECODABILITY + Accessor, &(CurrentScanResult->Decodability));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_GUARD_PATTERN + Accessor, &(CurrentScanResult->GuardPattern));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_QUIET_ZONE + Accessor, &(CurrentScanResult->QuietZone));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_WIDE_TO_NARROW_RATIO + Accessor, &(CurrentScanResult->WideToNarrowRatio));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_INTERCHARACTER_GAP + Accessor, &(CurrentScanResult->InterCharacterGap));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_MINIMUM_REFLECTANCE_MARGIN + Accessor, &(CurrentScanResult->MinimumReflectanceMargin));
      McodeGetResult(MilCodeResult, OccurrenceIndex, ScanIndex + ScanIndexOffset, M_SCAN_EDGE_DETERMINATION_WARNING + Accessor, &(CurrentScanResult->EdgeDeterminationWarning));

      /* Remember which scan has the worst grade in the whole code. */
      GradingResult->UpdateWorstGrade(RowIndex, ScanIndex, ScanIndexOffset, CurrentScanResult->ScanReflectanceProfileGrade);
      }
   }

/************************************************************************
  DisplayScanReflectanceProfileOfWorstGrade

  Display the scan reflectance profile of the scan that obtained the
  worst grade in the code.
************************************************************************/
void DisplayScanReflectanceProfileOfWorstGrade(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor, const GRADE_RESULT_ST& GradingResult)
   {
   MIL_INT WorstScanIndex = GradingResult.WorstScanIndex + GradingResult.WorstScanIndexOffset;

   /* Draw the position of the scan that had the worst grade. */
   MgraColor(M_DEFAULT, M_COLOR_RED);
   McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_SCAN_PROFILES + Accessor, 0, WorstScanIndex, M_DEFAULT);
   MIL_INT SizeX = 0;                             /* Size X */
   MIL_INT SizeY = REFLECTANCE_PROFILE_HEIGHT;    /* Size Y */

   /* Allocate buffer to hold the draw of the scan reflectance profile.
      It will be allocated as a child in the bottom of the displayed
      image. */
   MIL_INT OverlayHeight, OverlayWidth;
   MbufInquire(MilOverlayImage, M_SIZE_Y, &OverlayHeight);
   MbufInquire(MilOverlayImage, M_SIZE_X, &OverlayWidth);

   MIL_ID MilOverlayImageChild;
   MbufChild2d(MilOverlayImage, 0, OverlayHeight - REFLECTANCE_PROFILE_HEIGHT, OverlayWidth, REFLECTANCE_PROFILE_HEIGHT, &MilOverlayImageChild);

   /* Draw the scan reflectance profile. */
   MbufClear(MilOverlayImageChild, 255);
   MgraColor(M_DEFAULT, 0);
   McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImageChild, M_DRAW_REFLECTANCE_PROFILE + Accessor, 0, WorstScanIndex, M_DEFAULT);
   MbufFree(MilOverlayImageChild);
   }


/************************************************************************
  PrintScanWorstGrade

  Print the grading results of the reflectance profile of the scan 
  that obtained the worst grade in the code.
************************************************************************/
void PrintScanWorstGrade(const GRADE_RESULT_ST& GradingResult)
   {
   MosPrintf(MIL_TEXT("Code Type:               %s\n"), GetCodeTypeString(GradingResult));
   PrintGrade(MIL_TEXT("Overall Symbol Grade:    "), GradingResult.OverallGrade);
   MosPrintf(MIL_TEXT("Defect Row: #%d Scan reflectance profile: #%d\n\n"), (int)GradingResult.WorstRowIndex, (int)GradingResult.WorstScanIndex);

   const SCAN_REFLECTANCE_PROFILE_RESULT_ST &CurrentScanResult = GradingResult.RowResults.GetScanResult(GradingResult.WorstRowIndex, GradingResult.WorstScanIndex);

   PrintGrade(MIL_TEXT("  Scan reflectance profile grade: "), CurrentScanResult.ScanReflectanceProfileGrade);
   MosPrintf(MIL_TEXT("  ----------------------------------------------\n"));
   MosPrintf(MIL_TEXT("  Parameter                         Value      Grade\n"));
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n"));

   PrintGrade(MIL_TEXT("    Decode                                     "), CurrentScanResult.DecodeGrade);
   PrintValueAndGrade(MIL_TEXT("    Symbol Contrast (SC)            "), CurrentScanResult.SymbolContrast     , CurrentScanResult.SymbolContrastGrade     );
   PrintValueAndGrade(MIL_TEXT("    Minimum reflectance (Rmin)      "), CurrentScanResult.ReflectanceMinimum , CurrentScanResult.ReflectanceMinimumGrade );
   PrintValueAndGrade(MIL_TEXT("    Edge contrast minimum (ECmin)   "), CurrentScanResult.EdgeContrastMinimum, CurrentScanResult.EdgeContrastMinimumGrade);
   PrintValueAndGrade(MIL_TEXT("    Modulation (MOD)                "), CurrentScanResult.Modulation         , CurrentScanResult.ModulationGrade         );
   PrintValueAndGrade(MIL_TEXT("    Defects                         "), CurrentScanResult.Defects            , CurrentScanResult.DefectsGrade            );
   PrintValueAndGrade(MIL_TEXT("    Decodability (V)                "), CurrentScanResult.Decodability       , CurrentScanResult.DecodabilityGrade       );
   PrintValueAndGrade(MIL_TEXT("    Quiet Zone                      "), CurrentScanResult.QuietZone          , CurrentScanResult.QuietZoneGrade          );
   PrintValueAndGrade(MIL_TEXT("    Interior guard [in Z]           "), CurrentScanResult.GuardPattern       , CurrentScanResult.GuardPatternGrade       );
   PrintValueAndGrade(MIL_TEXT("    Wide/Narrow ratio [in Z]        "), CurrentScanResult.WideToNarrowRatio  , CurrentScanResult.WideToNarrowRatioGrade  );
   PrintValueAndGrade(MIL_TEXT("    Intercharacter gap [in Z]       "), CurrentScanResult.InterCharacterGap  , CurrentScanResult.InterCharacterGapGrade  );

   PrintValue(MIL_TEXT("    Minimum reflectance margin      "), CurrentScanResult.MinimumReflectanceMargin);

   if(CurrentScanResult.EdgeDeterminationWarning == M_YES)
      MosPrintf(MIL_TEXT("    Edge determination warning       Yes\n"));
   else if(CurrentScanResult.EdgeDeterminationWarning == M_NO)
      MosPrintf(MIL_TEXT("    Edge determination warning       No\n"));

   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n\n"));

   if ((GradingResult.WorstGrade + 0.5) >= M_CODE_GRADE_A)
      {
      MosPrintf(MIL_TEXT("The scan reflectance profile grades are perfect.\n"));
      }
   else
      {
      if (GradingResult.WorstGrade == M_CODE_GRADE_F)
         MosPrintf(MIL_TEXT("Grading failure:\n\n"));
      else
         MosPrintf(MIL_TEXT("Grading warning:\n\n"));

      if (CurrentScanResult.SymbolContrastGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the symbol contrast grade,\n"));
         MosPrintf(MIL_TEXT("the difference between the highest and the lowest intensity\n"));
         MosPrintf(MIL_TEXT("values (=Rmax-Rmin), in at least one scan line (displayed in red),\n"));
         MosPrintf(MIL_TEXT("is too small.\n\n"));
         }
      if (CurrentScanResult.ReflectanceMinimumGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the minimum reflectance grade,\n"));
         MosPrintf(MIL_TEXT("the lowest intensity value (Rmin) should not be more than\n"));
         MosPrintf(MIL_TEXT("0.5x the maximum intensity value (Rmax) in at least one scan\n"));
         MosPrintf(MIL_TEXT("profile (displayed in red).\n\n"));

         }
      if (CurrentScanResult.EdgeContrastMinimumGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Edge contrast minimum grade,\n"));
         MosPrintf(MIL_TEXT("the smallest intensity difference (=Rs-Rb) of adjoining\n"));
         MosPrintf(MIL_TEXT("elements of a scan profile (displayed in red) is too small.\n\n"));
         }
      if (CurrentScanResult.ModulationGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the modulation grade (=ECmin/SC),\n"));
         MosPrintf(MIL_TEXT("the ratio between the minimum edge contrast and the symbol\n"));
         MosPrintf(MIL_TEXT("contrast is too small in at least one scan profile (displayed in red)\n\n"));
         }
      if (CurrentScanResult.DefectsGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the defects grade (=ERNmax/SC),\n"));
         MosPrintf(MIL_TEXT("intensity irregularities found in the barcode elements, including its\n"));
         MosPrintf(MIL_TEXT("quiet zone, are too important in at least one scan profile (displayed in red).\n\n"));
         }
      if (CurrentScanResult.DecodabilityGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the decodabilty grade measures,\n"));
         MosPrintf(MIL_TEXT("  V = absolute value of ( (RT - M) / (RT - A) )\n"));
         MosPrintf(MIL_TEXT("where: (RT - M) represents the remaining margin not used by the printing\n"));
         MosPrintf(MIL_TEXT("                variation,\n"));
         MosPrintf(MIL_TEXT("   and (RT - A) represents the total theoretical margin based on the\n"));
         MosPrintf(MIL_TEXT("                ideal measurement of the element(s).\n\n"));
         }
      if ((CurrentScanResult.QuietZoneGrade == GradingResult.WorstGrade) ||
         (CurrentScanResult.GuardPatternGrade == GradingResult.WorstGrade))
         {
         MosPrintf(MIL_TEXT("- as reported by the quiet zone grade, the required blank\n"));
         MosPrintf(MIL_TEXT("space before and after the bar code is not respected in at least one scan\n"));
         MosPrintf(MIL_TEXT("profile (displayed in red).\n\n"));
         }
      if (CurrentScanResult.DecodeGrade == GradingResult.WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the decode grade, one of the following criteria\n"));
         MosPrintf(MIL_TEXT("is probably not respected: character encoding, start/stop patterns,\n"));
         MosPrintf(MIL_TEXT("check digits, quiet zones, or inter-character gaps. In this image, some bars,\n"));
         MosPrintf(MIL_TEXT("along at least one scan profile (displayed in red), are too thin.\n\n"));
         }
      }
   }

/************************************************************************
  GetAndDisplayResultsForCrossRowCode

  Extracts the grading results for a cross row code from a MIL code result
  and display them.
************************************************************************/
void GetAndDisplayResultsForCrossRowCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage, MIL_INT Accessor)
   {
   MIL_INT ScanIndexOffset = 0;
   MIL_INT OccurrenceIndex = 0;
   GRADE_RESULT_ST GradingResult;
   
   /* Initialize the worst grade. */
   GradingResult.InitializeWorstGrade();

   /* Get the code type. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &(GradingResult.CodeType));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_ENCODING  + M_TYPE_MIL_INT, &(GradingResult.Encoding));

   if(GradingResult.CodeType == M_COMPOSITECODE)
      {
      if(GradingResult.Encoding == M_ENC_GS1_128_PDF417)
         GradingResult.CodeType = M_PDF417;
      else
         GradingResult.CodeType = M_MICROPDF417;
      }

   /* Get the global code grading results. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_OVERALL_SYMBOL_GRADE + Accessor, &(GradingResult.OverallGrade));

   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_START_STOP_PATTERN_GRADE + Accessor, &(GradingResult.StartStopGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODEWORD_YIELD_GRADE + Accessor + M_TYPE_MIL_INT, &(GradingResult.CodewordYieldGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_MODULATION_GRADE + Accessor + M_TYPE_MIL_INT, &(GradingResult.ModulationGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_DECODABILITY_GRADE + Accessor + M_TYPE_MIL_INT, &(GradingResult.DecodabilityGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_DEFECTS_GRADE + Accessor + M_TYPE_MIL_INT, &(GradingResult.DefectsGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_UNUSED_ERROR_CORRECTION_GRADE + Accessor + M_TYPE_MIL_INT, &(GradingResult.UnusedErrorCorrectionGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODEWORD_YIELD + Accessor, &(GradingResult.CodewordYield));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_UNUSED_ERROR_CORRECTION + Accessor, &(GradingResult.UnusedErrorCorrection));

   /* Get number of Rows*/
   /* Note, accessor is used for composite code to identify 2D part. */

   MIL_INT NumberOfRows;
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_NUMBER_OF_ROWS + Accessor + M_TYPE_MIL_INT, &NumberOfRows);

   /* Allocate memory for each row. */
   GradingResult.RowResults.AllocateRows(NumberOfRows);

   for(MIL_INT RowIndex = 0; RowIndex < NumberOfRows; RowIndex++)
      {
      /* Get number of scans per row. */
      MIL_INT NumberOfScans;
      McodeGetResult(MilCodeResult, OccurrenceIndex, RowIndex, M_ROW_NUMBER_OF_SCANS + Accessor + M_TYPE_MIL_INT, &NumberOfScans);

      /* Allocate memory to hold results of scan. */
      GradingResult.RowResults.AllocateScans(RowIndex, NumberOfScans);

      /* Get grading of the current row. */
      GetResultForRow(MilCodeResult, OccurrenceIndex, RowIndex, ScanIndexOffset, Accessor, &GradingResult);

      /* Increment the scan profile index. */
      ScanIndexOffset += NumberOfScans;
      }

   /* Display scan reflectance profile. */
   DisplayScanReflectanceProfileOfWorstGrade(MilSystem, MilCodeResult, MilOverlayImage, Accessor, GradingResult);

   /* Print worst scan result. */
   PrintScanWorstGrade(GradingResult);

   /* Print worst Cross-Row result. */
   MIL_INT WorstGrade = GradingResult.GetWorstCrossRowGrade();
   PrintCrossRowGrade(GradingResult, WorstGrade);

   /* Free allocated memory. */
   GradingResult.RowResults.Free();
   }


/************************************************************************
  PrintCrossRowGrade

  Print Cross-Row code verification result and the grading results for
  the scan that obtained the worst grade.
************************************************************************/
void PrintCrossRowGrade(const GRADE_RESULT_ST& GradingResult, MIL_INT WorstGrade)
   {
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("  Parameter                         Value      Grade\n"));
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n"));

   PrintGrade(MIL_TEXT("    Start/Stop Pattern                         "), GradingResult.StartStopGrade);
   PrintValueAndGrade(MIL_TEXT("    Codeword Yield                  "), GradingResult.CodewordYield, GradingResult.CodewordYieldGrade);
   PrintGrade(MIL_TEXT("    Modulation                                 "), GradingResult.ModulationGrade);
   PrintGrade(MIL_TEXT("    Decodability                               "), GradingResult.DecodabilityGrade);
   PrintGrade(MIL_TEXT("    Defects                                    "), GradingResult.DefectsGrade);
   PrintValueAndGrade(MIL_TEXT("    Unused Error Correction         "), GradingResult.UnusedErrorCorrection, GradingResult.UnusedErrorCorrectionGrade);
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n\n"));

   if (WorstGrade == M_CODE_GRADE_A)
      {
      MosPrintf(MIL_TEXT("The code is perfect.\n"));
      }
   else
      {
      if (WorstGrade == M_CODE_GRADE_F)
         MosPrintf(MIL_TEXT("Grading failure:\n\n"));
      else
         MosPrintf(MIL_TEXT("Grading warning:\n\n"));

      if (GradingResult.CodewordYieldGrade == WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the codeword yield grade, the codeword yield\n"));
         MosPrintf(MIL_TEXT("result determines how well the code can be read at an angle relative\n"));
         MosPrintf(MIL_TEXT("to the horizontal and vertical axis of code. When all other results\n"));
         MosPrintf(MIL_TEXT("are good, a poor codeword yield result can indicate a problem along\n"));
         MosPrintf(MIL_TEXT("the Y-axis of the code.\n\n"));
         }
      if (GradingResult.ModulationGrade == WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the modulation grade,\n"));
         MosPrintf(MIL_TEXT("modulation is the ratio of the minimum edge contrast to symbol\n"));
         MosPrintf(MIL_TEXT("contrast within the code.\n\n"));
         }
      if (GradingResult.DecodabilityGrade == WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the codeword decodability grade,\n"));
         MosPrintf(MIL_TEXT("the print quality of each codeword relative is too poor.\n"));
         MosPrintf(MIL_TEXT("Note that this grade does not take into account any start/stop patterns.\n\n"));
         }
      if (GradingResult.DefectsGrade == WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the codeword defects grade,\n"));
         MosPrintf(MIL_TEXT("the deviation relative to the expected signal that denotes a\n"));
         MosPrintf(MIL_TEXT("codeword in the code is too large. The larger the result, the\n"));
         MosPrintf(MIL_TEXT("greater the defect in the codeword, and the less likely that the\n"));
         MosPrintf(MIL_TEXT("codeword can be decoded without error.\n\n"));
         }
      if (GradingResult.UnusedErrorCorrectionGrade == WorstGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the unused error correction grade, the ratio\n"));
         MosPrintf(MIL_TEXT("between the unused error correction and the total number of error correction\n"));
         MosPrintf(MIL_TEXT("available in the code is too low. Damages in the code have eroded the \n"));
         MosPrintf(MIL_TEXT("\"reading safety margin\" that the error correction provides.\n\n"));
         }
      }

   }


/************************************************************************
  GetAndDisplayResultsFor2DMatrixCode

  Extracts the grading results for a 2D Matrix code from a MIL code result
  and display them.
************************************************************************/
void GetAndDisplayResultsFor2DMatrixCode(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage)
   {
   MIL_INT ScanIndexOffset = 0;
   MIL_INT OccurrenceIndex = 0;
   GRADE_RESULT_ST GradingResult;
   
   /* Initialize the worst grade. */
   GradingResult.InitializeWorstGrade();

   /* Get the code type. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &(GradingResult.CodeType));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_ENCODING  + M_TYPE_MIL_INT, &(GradingResult.Encoding));

   /* Get the global code grading results. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_OVERALL_SYMBOL_GRADE, &(GradingResult.OverallGrade));

   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_DECODE_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.DecodeGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_PRINT_GROWTH, &(GradingResult.MatrixResults.PrintGrowth));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_UNUSED_ERROR_CORRECTION_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.UnusedErrorCorrectionGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_UNUSED_ERROR_CORRECTION, &(GradingResult.MatrixResults.UnusedErrorCorrection));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_AXIAL_NONUNIFORMITY_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.AxialNonUniformityGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_AXIAL_NONUNIFORMITY, &(GradingResult.MatrixResults.AxialNonUniformity));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_GRID_NONUNIFORMITY_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.GridNonUniformityGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_GRID_NONUNIFORMITY, &(GradingResult.MatrixResults.GridNonUniformity));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_FIXED_PATTERN_DAMAGE_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.FixedPatternDamageGrade));
   
   /* Available only for ISO/IEC:15415 grading standard. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_SYMBOL_CONTRAST_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.SymbolContrastGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_SYMBOL_CONTRAST, &(GradingResult.MatrixResults.SymbolContrast));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_MODULATION_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.ModulationGrade));

   /* Available only for Qr Code. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_FORMAT_INFORMATION_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.FormatInformationGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_VERSION_INFORMATION_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.VersionInformationGrade));

   /* Available only for ISO DPM grading standard. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_MODULATION_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.CellModulationGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_CONTRAST_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.CellContrastGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_CONTRAST, &(GradingResult.MatrixResults.CellContrast));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_MINIMUM_REFLECTANCE_GRADE + M_TYPE_MIL_INT, &(GradingResult.MatrixResults.MinimumReflectanceGrade));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_MINIMUM_REFLECTANCE, &(GradingResult.MatrixResults.MinimumReflectance));

   /* Print the worst 2D Matrix result */
   Print2DMatrixGrade(GradingResult);
   }

/************************************************************************
  Print2DMatrixGrade

  Print the 2D Matrix code verification result and the grading results.
************************************************************************/
void Print2DMatrixGrade(const GRADE_RESULT_ST& GradingResult)
   {
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("  Parameter                         Value      Grade\n"));
   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n"));

   PrintGrade(MIL_TEXT("    Decode Grade                               "), GradingResult.MatrixResults.DecodeGrade);
   PrintValueAndGrade(MIL_TEXT("    Unused Error Correction         "), GradingResult.MatrixResults.UnusedErrorCorrection, GradingResult.MatrixResults.UnusedErrorCorrectionGrade);
   PrintValue(MIL_TEXT("    Print Growth                    "), GradingResult.MatrixResults.PrintGrowth);
   PrintValueAndGrade(MIL_TEXT("    Axial Non-Uniformity            "), GradingResult.MatrixResults.AxialNonUniformity, GradingResult.MatrixResults.AxialNonUniformityGrade);
   PrintValueAndGrade(MIL_TEXT("    Grid Non-Uniformity             "), GradingResult.MatrixResults.GridNonUniformity, GradingResult.MatrixResults.GridNonUniformityGrade);
   PrintGrade(MIL_TEXT("    Fixed Pattern Damage                       "), GradingResult.MatrixResults.FixedPatternDamageGrade);
   PrintGrade(MIL_TEXT("    Format Information                         "), GradingResult.MatrixResults.FormatInformationGrade);
   PrintGrade(MIL_TEXT("    Version Information                        "), GradingResult.MatrixResults.VersionInformationGrade);
   PrintValueAndGrade(MIL_TEXT("    Symbol Contrast                 "), GradingResult.MatrixResults.SymbolContrast, GradingResult.MatrixResults.SymbolContrastGrade);
   PrintGrade(MIL_TEXT("    Modulation Grade                           "), GradingResult.MatrixResults.ModulationGrade);
   PrintValueAndGrade(MIL_TEXT("    Cell Contrast                   "), GradingResult.MatrixResults.CellContrast, GradingResult.MatrixResults.CellContrastGrade);
   PrintValueAndGrade(MIL_TEXT("    Minimum Reflectance             "), GradingResult.MatrixResults.MinimumReflectance, GradingResult.MatrixResults.MinimumReflectanceGrade);
   PrintGrade(MIL_TEXT("    Cell Modulation Grade                      "), GradingResult.MatrixResults.CellModulationGrade);

   MosPrintf(MIL_TEXT("  ----------------------------------------------------\n\n"));

   if (GradingResult.OverallGrade == M_CODE_GRADE_A)
      {
      MosPrintf(MIL_TEXT("The code is perfect.\n"));
      }
   else
      {
      if (GradingResult.OverallGrade == M_CODE_GRADE_F)
         MosPrintf(MIL_TEXT("Grading failure:\n\n"));
      else
         MosPrintf(MIL_TEXT("Grading warning:\n\n"));

      if (GradingResult.MatrixResults.DecodeGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the decode grade, the print quality\n"));
         MosPrintf(MIL_TEXT("of the symbol is too poor to be readable.\n"));
         }
      if (GradingResult.MatrixResults.UnusedErrorCorrectionGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the unused error correction grade, the ratio\n"));
         MosPrintf(MIL_TEXT("between the unused error correction and the total number of error corrections\n"));
         MosPrintf(MIL_TEXT("available in the code is too low. Damages in the code have eroded the \n"));
         MosPrintf(MIL_TEXT("\"reading safety margin\" that the error correction provides.\n\n"));
         }
      if (GradingResult.MatrixResults.SymbolContrastGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the symbol contrast grade,\n"));
         MosPrintf(MIL_TEXT("the difference between the highest and the lowest intensity\n"));
         MosPrintf(MIL_TEXT("values (=Rmax-Rmin) is too small.\n\n"));
         }
      if (GradingResult.MatrixResults.ModulationGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the modulation grade,\n"));
         MosPrintf(MIL_TEXT("modulation is a measure of the uniformity of reflectance of\n"));
         MosPrintf(MIL_TEXT("the dark and light modules, respectively.\n\n"));
         }
      if (GradingResult.MatrixResults.CellContrastGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the cell contrast grade,\n"));
         MosPrintf(MIL_TEXT("CC = (MLtarget MDtarget) / MLtarget\n"));
         MosPrintf(MIL_TEXT("where: MLtarget represents the mean of the light cell from the\n"));
         MosPrintf(MIL_TEXT("                final grid-point histogram of the symbol,\n"));
         MosPrintf(MIL_TEXT("  and  MDtarget represents the mean of the dark cell from the\n"));
         MosPrintf(MIL_TEXT("                final grid-point histogram of the symbol.\n\n"));
         }
      if (GradingResult.MatrixResults.MinimumReflectanceGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the minimum reflectance grade,\n"));
         MosPrintf(MIL_TEXT("Rtarget = Rcal x (SRcal/SRtarget) x (MLtarget/MLcal)\n"));
         MosPrintf(MIL_TEXT("where: MLtarget represents the mean of the light cell from the\n"));
         MosPrintf(MIL_TEXT("                final grid-point histogram of the symbol.\n"));
         MosPrintf(MIL_TEXT("       SRtarget represents the value of System Response parameters\n"));
         MosPrintf(MIL_TEXT("                used to create an image of the symbol.\n"));
         MosPrintf(MIL_TEXT("       MLcal    represents the mean of the light cell from a\n"));
         MosPrintf(MIL_TEXT("                histogram of the calibrated standard.\n"));
         MosPrintf(MIL_TEXT("       SRcal    represents the value of the System Response parameters\n"));
         MosPrintf(MIL_TEXT("                used to create an image of the calibrated standard.\n"));
         MosPrintf(MIL_TEXT("       Rcal     represents the reflectance value Rmax from a\n"));
         MosPrintf(MIL_TEXT("                calibrated standard.\n\n"));
         }
      if (GradingResult.MatrixResults.CellModulationGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the cell modulation grade,\n"));
         MosPrintf(MIL_TEXT("modulation is a measure of the uniformity of reflectance of\n"));
         MosPrintf(MIL_TEXT("the dark and light cells, respectively.\n\n"));
         }
      if (GradingResult.MatrixResults.AxialNonUniformityGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Axial Non-Uniformity grade,\n"));
         MosPrintf(MIL_TEXT("Axial Non-Uniformity is a measure of how much the sampling point spacing\n"));
         MosPrintf(MIL_TEXT("differs from one axis to another, namely:\n"));
         MosPrintf(MIL_TEXT("      AN = abs(XAVG - YAVG) / ((XAVG + YAVG) / 2)\n"));
         MosPrintf(MIL_TEXT("where: XAVG and YAVG are average spacing along each axis,\n"));
         MosPrintf(MIL_TEXT("   and abs() yields the absolute value.\n\n"));
         }
      if (GradingResult.MatrixResults.GridNonUniformityGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Grid Non-Uniformity grade,\n"));
         MosPrintf(MIL_TEXT("Grid Non-Uniformity is a measure of the largest vector deviation of the\n"));
         MosPrintf(MIL_TEXT("grid intersections, determined by the reference decode algorithm, from\n"));
         MosPrintf(MIL_TEXT("an ideal theoretical position.\n\n"));
         }
      if (GradingResult.MatrixResults.FixedPatternDamageGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Fixed Pattern Damage grade,\n"));
         MosPrintf(MIL_TEXT("the number of module errors (modules that appear as the inverse\n"));
         MosPrintf(MIL_TEXT("of the intended color or that have a bad modulation) in the finder pattern,\n"));
         MosPrintf(MIL_TEXT(" quiet zone, timing, and other fixed patterns, is too high.\n\n"));
         }
      if (GradingResult.MatrixResults.FormatInformationGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Format Information grade,\n"));
         MosPrintf(MIL_TEXT("the number of module errors (modules that appear as the inverse\n"));
         MosPrintf(MIL_TEXT("of the intended color) in each block of format information is\n"));
         MosPrintf(MIL_TEXT("too high.\n\n"));
         }
      if (GradingResult.MatrixResults.VersionInformationGrade == GradingResult.OverallGrade)
         {
         MosPrintf(MIL_TEXT("- as reported by the Version Information grade,\n"));
         MosPrintf(MIL_TEXT("the number of module errors (modules that appear as the inverse\n"));
         MosPrintf(MIL_TEXT("of the intended color) in each block of version information is\n"));
         MosPrintf(MIL_TEXT("too high.\n\n"));
         }
      }
   }

 /************************************************************************
  GetAndDisplaySemiT10Results

  Extracts SemiT10 grading results from a MIL code result
  and display them.
************************************************************************/
void GetAndDisplaySemiT10Results(MIL_ID MilSystem, MIL_ID MilCodeResult, MIL_ID MilOverlayImage)
   {
   MIL_INT ScanIndexOffset = 0;
   MIL_INT OccurrenceIndex = 0;
   GRADE_RESULT_ST GradingResult;

   /* Get the code type. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &(GradingResult.CodeType));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_ENCODING + M_TYPE_MIL_INT, &(GradingResult.Encoding));

   /* Available only for SEMI T10 grading standard. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P1_X, &(GradingResult.SemiT10Results.P1X));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P1_Y, &(GradingResult.SemiT10Results.P1Y));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P2_X, &(GradingResult.SemiT10Results.P2X));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P2_Y, &(GradingResult.SemiT10Results.P2Y));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P3_X, &(GradingResult.SemiT10Results.P3X));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P3_Y, &(GradingResult.SemiT10Results.P3Y));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P4_X, &(GradingResult.SemiT10Results.P4X));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CORNER_P4_Y, &(GradingResult.SemiT10Results.P4Y));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_SYMBOL_CONTRAST, &(GradingResult.SemiT10Results.SymbolContrast));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_SYMBOL_CONTRAST_SNR, &(GradingResult.SemiT10Results.SymbolContrastSNR));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_HORIZONTAL_MARK_GROWTH, &(GradingResult.SemiT10Results.HorizontalMarkGrowth));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_VERTICAL_MARK_GROWTH, &(GradingResult.SemiT10Results.VerticalMarkGrowth));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_WIDTH, &(GradingResult.SemiT10Results.CellWidth));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_HEIGHT, &(GradingResult.SemiT10Results.CellHeight));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_HORIZONTAL_MARK_MISPLACEMENT, &(GradingResult.SemiT10Results.HorizontalMarkMisplacement));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_VERTICAL_MARK_MISPLACEMENT, &(GradingResult.SemiT10Results.VerticalMarkMisplacement));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_NUMBER_OF_INTERLEAVED_BLOCKS + M_TYPE_MIL_INT, &(GradingResult.SemiT10Results.NumberOfInterleavedBlocks));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_UNUSED_ERROR_CORRECTION, GradingResult.SemiT10Results.UnusedErrorCorrection);
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_DEFECTS, &(GradingResult.SemiT10Results.CellDefects));
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_FINDER_PATTERN_DEFECTS, &(GradingResult.SemiT10Results.FinderPatternDefects));

   /* Result Type M_CELL_NUMBER_X corresponds to Number of columns in SEMI T10 grading standard. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_NUMBER_X + M_TYPE_MIL_INT, &(GradingResult.SemiT10Results.NbColumns));

   /* Result Type M_CELL_NUMBER_Y corresponds to Number of rows in SEMI T10 grading standard. */
   McodeGetResult(MilCodeResult, OccurrenceIndex, M_GENERAL, M_CELL_NUMBER_Y + M_TYPE_MIL_INT, &(GradingResult.SemiT10Results.NbRows));

   /* Print SemiT10 result */
   PrintSemiT10Grade(GradingResult);
   }

/************************************************************************
  PrintSemiT10Grade

  Print SemiT10 grading results.
************************************************************************/
void PrintSemiT10Grade(const GRADE_RESULT_ST& GradingResult)
   {
   MosPrintf(MIL_TEXT("  -------------------------------------------------------\n"));
   MosPrintf(MIL_TEXT("  Parameter                                 Value        \n"));
   MosPrintf(MIL_TEXT("  -------------------------------------------------------\n"));

   MosPrintf(MIL_TEXT("    P1                                       (%7.4f ; %7.4f) \n"), GradingResult.SemiT10Results.P1X, GradingResult.SemiT10Results.P1Y);
   MosPrintf(MIL_TEXT("    P2                                       (%7.4f ; %7.4f) \n"), GradingResult.SemiT10Results.P2X, GradingResult.SemiT10Results.P2Y);
   MosPrintf(MIL_TEXT("    P3                                       (%7.4f ; %7.4f) \n"), GradingResult.SemiT10Results.P3X, GradingResult.SemiT10Results.P3Y);
   MosPrintf(MIL_TEXT("    P4                                       (%7.4f ; %7.4f) \n"), GradingResult.SemiT10Results.P4X, GradingResult.SemiT10Results.P4Y);
   MosPrintf(MIL_TEXT("    Number of Columns                         %d \n"), GradingResult.SemiT10Results.NbColumns);
   MosPrintf(MIL_TEXT("    Number of Rows                            %d \n"), GradingResult.SemiT10Results.NbRows);

   MosPrintf(MIL_TEXT("    Symbol Contrast                           %7.4f %%\n"), GradingResult.SemiT10Results.SymbolContrast);

   if(GradingResult.SemiT10Results.SymbolContrastSNR != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Symbol Contrast SNR                       %7.4f \n"), GradingResult.SemiT10Results.SymbolContrastSNR);
   else
      MosPrintf(MIL_TEXT("    Symbol Contrast SNR                       Not Computable \n"));

   if(GradingResult.SemiT10Results.HorizontalMarkGrowth != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Horizontal Mark Growth                    %7.4f %%\n"), GradingResult.SemiT10Results.HorizontalMarkGrowth);
   else
      MosPrintf(MIL_TEXT("    Horizontal Mark Growth                    Not Computable \n"));

   if(GradingResult.SemiT10Results.VerticalMarkGrowth != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Vertical Mark Growth                      %7.4f %%\n"), GradingResult.SemiT10Results.VerticalMarkGrowth);
   else
      MosPrintf(MIL_TEXT("    Vertical Mark Growth                      Not Computable \n"));

   MosPrintf(MIL_TEXT("    Cell Width                                %7.4f \n"), GradingResult.SemiT10Results.CellWidth);
   MosPrintf(MIL_TEXT("    Cell Height                               %7.4f \n"), GradingResult.SemiT10Results.CellHeight);

   if(GradingResult.SemiT10Results.HorizontalMarkMisplacement != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Horizontal Mark Misplacement              %7.4f %%\n"), GradingResult.SemiT10Results.HorizontalMarkMisplacement);
   else
      MosPrintf(MIL_TEXT("    Horizontal Mark Misplacement              Not Computable \n"));

   if(GradingResult.SemiT10Results.VerticalMarkMisplacement != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Vertical Mark Misplacement                %7.4f %%\n"), GradingResult.SemiT10Results.VerticalMarkMisplacement);
   else
      MosPrintf(MIL_TEXT("    Vertical Mark Misplacement                Not Computable \n"));

   for(MIL_INT ii = 0; ii < (MIL_INT)GradingResult.SemiT10Results.UnusedErrorCorrection.size(); ii++)
      {
      MosPrintf(MIL_TEXT("    Unused Error Correction Block #%d         %7.4f %%\n"), ii, GradingResult.SemiT10Results.UnusedErrorCorrection[ii]);
      }

   if(GradingResult.SemiT10Results.CellDefects != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Cell Defects                              %7.4f %%\n"), GradingResult.SemiT10Results.CellDefects);
   else
      MosPrintf(MIL_TEXT("    Cell Defects                              Not Computable \n"));

   if(GradingResult.SemiT10Results.FinderPatternDefects != M_CODE_GRADE_NOT_COMPUTABLE)
      MosPrintf(MIL_TEXT("    Finder Pattern Defects                    %7.4f %%\n"), GradingResult.SemiT10Results.FinderPatternDefects);
   else
      MosPrintf(MIL_TEXT("    Finder Pattern Defects                    Not Computable \n"));

   MosPrintf(MIL_TEXT("  -------------------------------------------------------\n"));

   }

/************************************************************************
  GetCodeTypeCategory
************************************************************************/
CodeTypeCategory GetCodeTypeCategory(MIL_INT CodeType)
   {  
   CodeTypeCategory ComponentType = IS_NOT_SUPPORTED_BY_THIS_EXAMPLE;

   switch(CodeType)
      {
      case M_COMPOSITECODE:
         ComponentType = IS_COMPOSITE_CODE;

         MosPrintf(MIL_TEXT("COMPOSITE BAR CODE GRADING:\n"));
         MosPrintf(MIL_TEXT("===========================\n"));
         break;

      case M_CODE39:
      case M_EAN13:
      case M_INDUSTRIAL25:
      case M_INTERLEAVED25:
      case M_CODE128:
      case M_GS1_128:
      case M_EAN14:
      case M_BC412:
      case M_CODABAR:
      case M_UPC_A:
      case M_UPC_E:
      case M_GS1_DATABAR:
      case M_EAN8:
      case M_CODE93:
         ComponentType = IS_1D_CODE;

         MosPrintf(MIL_TEXT("LINEAR BAR CODE GRADING:\n"));
         MosPrintf(MIL_TEXT("========================\n"));
         break;

      case M_PDF417:
      case M_MICROPDF417:
      case M_TRUNCATED_PDF417:
         ComponentType = IS_CROSS_ROW_CODE;

         MosPrintf(MIL_TEXT("CROSS-ROW BAR CODE GRADING:\n"));
         MosPrintf(MIL_TEXT("===========================\n"));
         break;

      case M_DATAMATRIX:
      case M_QRCODE:
         ComponentType = IS_2D_MATRIX_CODE;

         MosPrintf(MIL_TEXT("2D MATRIX CODE GRADING:\n"));
         MosPrintf(MIL_TEXT("=======================\n"));
         break;
      
      case M_MAXICODE:
      case M_MICROQRCODE:
         MosPrintf(MIL_TEXT("Not all supported yet.\n"));
         MosGetch();
         break;
      
      default:
         MosPrintf(MIL_TEXT("Symbology does not support grading.\n"));
         MosGetch();
         break;
      }

   return ComponentType;
   }

/************************************************************************
  Get1DCodeTypeOfCompositeCode
************************************************************************/
MIL_INT Get1DCodeTypeOfCompositeCode(MIL_INT Encoding)
   {
   MIL_INT CodeType = 0;
   switch(Encoding)
      {
      case M_ENC_EAN13:                 CodeType = M_EAN13;   break;
      case M_ENC_EAN8:                  CodeType = M_EAN8;    break;
      case M_ENC_UPCA:                  CodeType = M_UPC_A;   break;
      case M_ENC_UPCE:                  CodeType = M_UPC_E;   break;

      case M_ENC_GS1_128_PDF417:      
      case M_ENC_GS1_128_MICROPDF417:   CodeType = M_GS1_128; break;

      case M_ENC_GS1_DATABAR_OMNI:                 
      case M_ENC_GS1_DATABAR_TRUNCATED:       
      case M_ENC_GS1_DATABAR_LIMITED:           
      case M_ENC_GS1_DATABAR_EXPANDED:          
      case M_ENC_GS1_DATABAR_STACKED:         
      case M_ENC_GS1_DATABAR_STACKED_OMNI:    
      case M_ENC_GS1_DATABAR_EXPANDED_STACKED:  CodeType = M_GS1_DATABAR; break;         
      }
   return CodeType;
   }

/************************************************************************
  GetCodeTypeString
************************************************************************/
MIL_CONST_TEXT_PTR GetCodeTypeString(const GRADE_RESULT_ST& GradingResult)
   {
   switch(GradingResult.CodeType)
      {
      case M_CODE39:                return MIL_TEXT("Code 39");                         break;
      case M_DATAMATRIX:            return MIL_TEXT("DataMatrix");                      break;
      case M_EAN13:                 return MIL_TEXT("EAN13");                           break;
      case M_MAXICODE:              return MIL_TEXT("Maxicode");                        break;
      case M_INDUSTRIAL25:          return MIL_TEXT("Industrial 2 of 5");               break;
      case M_INTERLEAVED25:         return MIL_TEXT("Interleaved 2 of 5");              break;
      case M_BC412:                 return MIL_TEXT("BC412");                           break;
      case M_CODABAR:               return MIL_TEXT("Codabar");                         break;
      case M_PDF417:                return MIL_TEXT("PDF417");                          break;
      case M_POSTNET:               return MIL_TEXT("Postnet");                         break;
      case M_PLANET:                return MIL_TEXT("Planet");                          break;
      case M_4_STATE:               return MIL_TEXT("4-State");                         break;
      case M_UPC_A:                 return MIL_TEXT("UPC-A");                           break;
      case M_UPC_E:                 return MIL_TEXT("UPC-E");                           break;
      case M_PHARMACODE:            return MIL_TEXT("Pharmacode");                      break;
      case M_EAN8:                  return MIL_TEXT("EAN8");                            break;
      case M_MICROPDF417:           return MIL_TEXT("MicroPDF417");                     break;
      case M_COMPOSITECODE:         return MIL_TEXT("Composite code");                  break;
      case M_GS1_128:               return MIL_TEXT("UCC/EAN/GS1-128");                 break;
      case M_QRCODE:                return MIL_TEXT("QRcode");                          break;
      case M_CODE93:                return MIL_TEXT("Code 93");                         break;
      case M_TRUNCATED_PDF417:      return MIL_TEXT("Truncated PDF417");                break;
      case M_EAN14:                 return MIL_TEXT("EAN14");                           break;

      case M_CODE128:
         if((GradingResult.Encoding == M_ENC_GS1_128_PDF417) ||
            (GradingResult.Encoding == M_ENC_GS1_128_MICROPDF417))
            return MIL_TEXT("UCC/EAN/GS1-128");
         else
            return MIL_TEXT("Code 128");                        
         break;

      case M_GS1_DATABAR:
         switch(GradingResult.Encoding)
            {
            case M_ENC_GS1_DATABAR_OMNI:                 return MIL_TEXT("GS1-DATABAR");                          break;
            case M_ENC_GS1_DATABAR_TRUNCATED:            return MIL_TEXT("GS1-DATABAR Truncated");                break;
            case M_ENC_GS1_DATABAR_LIMITED:              return MIL_TEXT("GS1-DATABAR Limited");                  break;
            case M_ENC_GS1_DATABAR_EXPANDED:             return MIL_TEXT("GS1-DATABAR Expanded");                 break;
            case M_ENC_GS1_DATABAR_STACKED:              return MIL_TEXT("GS1-DATABAR Stacked");                  break;
            case M_ENC_GS1_DATABAR_STACKED_OMNI:         return MIL_TEXT("GS1-DATABAR Stacked Omnidirectional");  break;
            case M_ENC_GS1_DATABAR_EXPANDED_STACKED:     return MIL_TEXT("GS1-DATABAR Expanded Stacked");         break;
            default:                                     return MIL_TEXT("GS1-DATABAR code");                     break;
            }
         break;

      default:                      
         return MIL_TEXT("Unavailable");                     break;
      }
   return MIL_TEXT("Unavailable");
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
  PrintGrade
************************************************************************/
void PrintGrade(MIL_CONST_TEXT_PTR Text, double Grade)
   {
   if (Grade != M_CODE_GRADE_NOT_AVAILABLE)
      MosPrintf(MIL_TEXT("%s%-1.1f (%s)\n"), Text, Grade, GetGradeString(Grade));
   }

void PrintGrade(MIL_CONST_TEXT_PTR Text, MIL_INT Grade)
   {
   PrintGrade(Text, static_cast<double>(Grade));
   }

/************************************************************************
  PrintValue
************************************************************************/
void PrintValue(MIL_CONST_TEXT_PTR Text, double Value)
   {
   MosPrintf(MIL_TEXT("%s%7.4f\n"), Text, Value);
   }

/************************************************************************
  PrintValueAndGrade
************************************************************************/
void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, double Grade)
   {
   if (Grade != M_CODE_GRADE_NOT_AVAILABLE)
      MosPrintf(MIL_TEXT("%s%7.4f    %-1.1f (%s)\n"), Text, Value, Grade, GetGradeString(Grade));
   }

void PrintValueAndGrade(MIL_CONST_TEXT_PTR Text, double Value, MIL_INT Grade)
   {
   PrintValueAndGrade(Text, Value, static_cast<double>(Grade));
   }


/************************************************************************
  GRADE_ARRAY member functions
************************************************************************/
GRADE_ARRAY::GRADE_ARRAY()
   {
   m_RowResults = NULL;
   m_NumberOfRows = 0;
   }

GRADE_ARRAY::~GRADE_ARRAY()
   {
   Free();
   }

void GRADE_ARRAY::AllocateRows(MIL_INT NumberOfRows)
   {
   /* Allocate memory for rows. */
   Free();
   m_RowResults = new ROW_RESULT_ST [NumberOfRows];
   m_NumberOfRows = NumberOfRows;

   for(int ii = 0; ii < m_NumberOfRows; ii++)
      {
      m_RowResults[ii].RowOverallGrade = M_CODE_GRADE_NOT_AVAILABLE;
      m_RowResults[ii].ScanResults = NULL;
      }
   }


void GRADE_ARRAY::AllocateScans(MIL_INT RowIndex, MIL_INT NumberOfScans)
   {
   /* Allocate memory for scans of a given row. */
   m_RowResults[RowIndex].ScanResults = new SCAN_REFLECTANCE_PROFILE_RESULT_ST[NumberOfScans];
   }
   

void GRADE_ARRAY::Free()
   {
   /* Free all allocated memory */
   for(int ii = 0; ii < m_NumberOfRows; ii++ )
      delete [] m_RowResults[ii].ScanResults;

   delete [] m_RowResults;

   m_RowResults = NULL;
   m_NumberOfRows = 0;
   }

SCAN_REFLECTANCE_PROFILE_RESULT_ST *GRADE_ARRAY::GetScanResultPtr(MIL_INT RowIndex, MIL_INT ScanIndex)
   {
   return &(m_RowResults[RowIndex].ScanResults[ScanIndex]);
   }

const SCAN_REFLECTANCE_PROFILE_RESULT_ST &GRADE_ARRAY::GetScanResult(MIL_INT RowIndex, MIL_INT ScanIndex) const
   {
   return m_RowResults[RowIndex].ScanResults[ScanIndex];
   }

ROW_RESULT_ST *GRADE_ARRAY::GetRowResultPtr(MIL_INT RowIndex)
   {
   return &(m_RowResults[RowIndex]);
   }

/************************************************************************
MATRIX_RESULT_ST member functions
************************************************************************/
MATRIX_RESULT_ST::MATRIX_RESULT_ST()
   {
   DecodeGrade = 0;
   UnusedErrorCorrectionGrade = 0;
   AxialNonUniformityGrade = 0;
   GridNonUniformityGrade = 0;
   FixedPatternDamageGrade = 0;
   FormatInformationGrade = 0;
   VersionInformationGrade = 0;
   SymbolContrastGrade = 0;
   ModulationGrade = 0;
   CellModulationGrade = 0;
   CellContrastGrade = 0;
   MinimumReflectanceGrade = 0;

   UnusedErrorCorrection = 0;
   PrintGrowth = 0;
   AxialNonUniformity = 0;
   GridNonUniformity = 0;
   SymbolContrast = 0;
   CellContrast = 0;
   MinimumReflectance = 0;
   }

/************************************************************************
SEMIT10_RESULT_ST member functions
************************************************************************/
SEMIT10_RESULT_ST::SEMIT10_RESULT_ST()
   {
   P1X = 0;
   P1Y = 0;
   P2X = 0;
   P2Y = 0;
   P3X = 0;
   P3Y = 0;
   P4X = 0;
   P4Y = 0;
   NbColumns = 0;
   NbRows = 0;
   SymbolContrast = 0;
   SymbolContrastSNR = 0;
   HorizontalMarkGrowth = -1;
   VerticalMarkGrowth = -1;
   CellWidth = 0;
   CellHeight = 0;
   HorizontalMarkMisplacement = -1;
   VerticalMarkMisplacement = -1;
   NumberOfInterleavedBlocks = 0;
   UnusedErrorCorrection.resize(0);
   CellDefects = -1;
   FinderPatternDefects = -1;
   }

/************************************************************************
  GRADE_RESULT_ST member functions
************************************************************************/
GRADE_RESULT_ST::GRADE_RESULT_ST()
   {
   CodeType = 0;
   Encoding = 0;

   OverallGrade = 0;

   /* Worst grade. */
   WorstScanIndex = 0;
   WorstRowIndex = 0;
   WorstScanIndexOffset = 0;
   WorstGrade = 0;

   /* Result Per Row. */
   InitializeWorstGrade();

   /* Results specific to Cross-row component. */
   StartStopGrade = 0;
   CodewordYieldGrade = 0;
   ModulationGrade = 0;
   DecodabilityGrade = 0;
   DefectsGrade = 0;
   UnusedErrorCorrectionGrade = 0;
   CodewordYield = 0;
   UnusedErrorCorrection = 0;
   }

void GRADE_RESULT_ST::InitializeWorstGrade()
   {
   WorstGrade           = M_CODE_GRADE_NOT_AVAILABLE;
   WorstScanIndex       = 0;
   WorstScanIndexOffset = 0;
   WorstRowIndex        = 0;
   }

void GRADE_RESULT_ST::UpdateWorstGrade(MIL_INT RowIndex, MIL_INT ScanIndex, MIL_INT ScanIndexOffset, MIL_DOUBLE Grade)
   {
   if((Grade < WorstGrade) || (Grade == M_CODE_GRADE_NOT_AVAILABLE))
      {
      WorstGrade           = Grade;
      WorstScanIndex       = ScanIndex;
      WorstRowIndex        = RowIndex;
      WorstScanIndexOffset = ScanIndexOffset;
      }
   }

MIL_INT GRADE_RESULT_ST::GetWorstCrossRowGrade()
   {
   MIL_INT WorstGrade = CodewordYieldGrade;
   MIL_INT CurrentWorstGrade;

   CurrentWorstGrade = min(ModulationGrade, DecodabilityGrade);
   WorstGrade = min(CurrentWorstGrade, WorstGrade);
   CurrentWorstGrade = min(DefectsGrade, UnusedErrorCorrectionGrade);
   WorstGrade = min( CurrentWorstGrade, WorstGrade);

   return WorstGrade;
   }
