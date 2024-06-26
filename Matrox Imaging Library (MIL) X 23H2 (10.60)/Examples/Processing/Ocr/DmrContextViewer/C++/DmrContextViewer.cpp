﻿//******************************************************************************
//
// File name: DMRContextViewer.cpp
//
// Synopsis:  This example inquires the settings and the fonts of an
//            interactively restored Dot Matrix Reader (SureDotOCR®) context.
//            The inquired values are either displayed on screen
//            or saved to disk depending on user's choice.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <sstream>

// Output context info. to text file (if chosen)
MIL_CONST_TEXT_PTR OUTPUT_FILE  = M_TEMP_DIR MIL_TEXT("MdmrContext.txt");

// Define the struct of the settings.
class CDMRInquirer
   {
   MIL_INT    _DefaultSystem;
   MIL_DOUBLE _DotDiameter;
   MIL_INT    _ForegroundValue;
   MIL_DOUBLE _ItalicAngle;
   MIL_INT    _ItalicAngleMode;
   MIL_DOUBLE _MaxIntensity;
   MIL_INT    _MaxIntensityMode;
   MIL_INT    _MinContrast;
   MIL_INT    _MinContrastMode;
   MIL_DOUBLE _MinIntensity;
   MIL_INT    _MinIntensityMode;
   MIL_INT    _NbOfFont;
   MIL_INT    _NbOfStringModels;
   MIL_DOUBLE _SpaceSizeMax;
   MIL_INT    _SpaceSizeMaxMode;
   MIL_DOUBLE _SpaceSizeMin;
   MIL_INT    _SpaceSizeMinMode;
   MIL_DOUBLE _StringAngle;
   MIL_INT    _StringAngleMode;
   MIL_INT    _StringAngleInputUnits;
   MIL_DOUBLE _TextBlockHeight;
   MIL_INT    _TextBlockSizeMode;
   MIL_DOUBLE _TextBlockWidth;
   MIL_DOUBLE _TimeOut;

   // String model parameters.
   MIL_DOUBLE _CharAcceptance;
   MIL_INT    _NbOfConstrainedPositions;
   MIL_DOUBLE _StringAcceptance;
   MIL_DOUBLE _StringCertainty;
   MIL_DOUBLE _StringIndexValue;
   MIL_DOUBLE _StringLabelValue;
   MIL_INT    _StringRank;
   MIL_INT    _StringSizeMax;
   MIL_INT    _StringSizeMin;

   // Font parameters.
   MIL_INT    _ConstraintPosition;
   MIL_INT    _ConstraintType;
   MIL_INT    _FontLabel;
   MIL_INT    _FontSizeColumns;
   MIL_INT    _FontSizeRows;
   MIL_INT    _FontSizeTemplate;
   MIL_INT    _NbOfChars;
   MIL_INT    _NbOfPermittedChars;

   // Inquiry functions
   void InquireSettings    (MIL_ID DmrCntxId, MIL_FILE OutputTarget);
   void InquireStringModel (MIL_ID DmrCntxId, MIL_FILE OutputTarget);
   void InquireAndPrintFont(MIL_ID DmrCntxId, MIL_FILE OutputTarget);

   // Helper functions
   static MIL_CONST_TEXT_PTR DefToStr(MIL_INT TheDef);
   static void PrintConstraintInfo(MIL_ID DmrCntxId, MIL_INT ConstraintType, MIL_CONST_TEXT_PTR CharListPtr, MIL_INT ConstraintFontLabel, MIL_FILE OutputTarget);
   void PrintTemplate(MIL_FILE OutputTarget, MIL_UINT8* CharTemplate);

   public:
      // Main inquire function
      void Inquire(MIL_ID DmrCntxId, MIL_FILE OutputTarget);
   };

// Helper class
// RAII-style output file.
class COutputTarget
   {
   MIL_FILE _OutputTarget;
   public:
      COutputTarget();
      ~COutputTarget();
      operator MIL_FILE();
   };

// Example description.
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DMRContextViewer\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to inquire the DMR context settings.\n\n")
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, System, Dot Matrix Reader.\n\n")

             MIL_TEXT("NOTE:\n")
             MIL_TEXT("If you don't have context ready to use,\n")
             MIL_TEXT("there is one preset context for testing purpose:\n")
             MIL_TEXT("\"Matrox Imaging\\Images\\DmrContextViewer\\SampleDmrContextForInquiry.mdmr\"\n\n")

             MIL_TEXT("Press <Enter> to restore a Dot Matrix Reader context from disk.\n"));

   MosGetch();
   }

// Main.
int MosMain(void)
   {
   PrintHeader();

   // Allocate the application.
   const MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);

   // Alloc the system
   const MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);

   // Interactively restore a Dot Matrix Reader context.
   MappControl(M_ERROR, M_PRINT_DISABLE);
   const MIL_ID DmrCntxId = MdmrRestore(M_INTERACTIVE, MilSystem, M_DEFAULT, M_NULL);
   MappControl(M_ERROR, M_PRINT_ENABLE);

   if (DmrCntxId != M_NULL)
      {
      // Set output to screen or text file
      COutputTarget OutputTarget;
      
      if (MIL_FILE(OutputTarget))
         {
         // Inquire DMR context
         CDMRInquirer DMRInquirer;
         DMRInquirer.Inquire(DmrCntxId, OutputTarget);
         }

      // Free DMR context
      MdmrFree(DmrCntxId);
      }
   else
      {
      MosPrintf(MIL_TEXT("\nError loading context file.\n"));
      MosPrintf(MIL_TEXT("Please make sure you have a MIL Full license.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
      MosGetch();
      }

   // Free system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

// CDMRSetting member function definitions.

// Function to inquire settings.
void CDMRInquirer::InquireSettings(MIL_ID DmrCntxId, MIL_FILE OutputTarget)
   {
   MosFprintf(OutputTarget, MIL_TEXT("-------------------------------------------\n")
                            MIL_TEXT("Context Settings:\n")
                            MIL_TEXT("-------------------------------------------\n"));

   // Inquiry on dot diameters
   MdmrInquire(DmrCntxId, M_DOT_DIAMETER + M_TYPE_MIL_DOUBLE, &_DotDiameter);
   if (M_DEFAULT == _DotDiameter)
      {
      MdmrInquire(DmrCntxId, M_DOT_DIAMETER + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_DotDiameter);
      MosFprintf(OutputTarget, MIL_TEXT("M_DOT_DIAMETER             = M_DEFAULT (%s)\n"), DefToStr(MIL_INT(_DotDiameter)));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_DOT_DIAMETER             = %.2f\n"), _DotDiameter);
      }

   // Inquiry on foreground values
   MdmrInquire(DmrCntxId, M_FOREGROUND_VALUE + M_TYPE_MIL_INT, &_ForegroundValue);
   if (M_DEFAULT == _ForegroundValue)
      {
      MdmrInquire(DmrCntxId, M_FOREGROUND_VALUE + M_TYPE_MIL_INT + M_DEFAULT, &_ForegroundValue);
      MosFprintf(OutputTarget, MIL_TEXT("M_FOREGROUND_VALUE         = M_DEFAULT (%s)\n"), DefToStr(_ForegroundValue));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_FOREGROUND_VALUE         = %s\n"), DefToStr(_ForegroundValue));
      }

   // Inquiry on M_ITALIC_ANGLE
   MdmrInquire(DmrCntxId, M_ITALIC_ANGLE + M_TYPE_MIL_DOUBLE, &_ItalicAngle);
   if (M_DEFAULT == _ItalicAngle)
      {
      MdmrInquire(DmrCntxId, M_ITALIC_ANGLE + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_ItalicAngle);
      MosFprintf(OutputTarget, MIL_TEXT("M_ITALIC_ANGLE             = M_DEFAULT (%.2f)\n"), _ItalicAngle);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_ITALIC_ANGLE             = %.2f\n"), _ItalicAngle);
      }

   // Inquiry on M_ITALIC_ANGLE_MODE
   MdmrInquire(DmrCntxId, M_ITALIC_ANGLE_MODE + M_TYPE_MIL_INT, &_ItalicAngleMode);
   if (M_DEFAULT == _ItalicAngleMode)
      {
      MdmrInquire(DmrCntxId, M_ITALIC_ANGLE_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_ItalicAngleMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_ITALIC_ANGLE_MODE        = M_DEFAULT (%s)\n"), DefToStr(_ItalicAngleMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_ITALIC_ANGLE_MODE        = %s\n"), DefToStr(_ItalicAngleMode));
      }

   // Inquiry on maximum pixel intensity
   MdmrInquire(DmrCntxId, M_MAX_INTENSITY + M_TYPE_MIL_DOUBLE, &_MaxIntensity);
   if (M_DEFAULT == _MaxIntensity)
      {
      MdmrInquire(DmrCntxId, M_MAX_INTENSITY + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_MaxIntensity);
      MosFprintf(OutputTarget, MIL_TEXT("M_MAX_INTENSITY            = M_DEFAULT (%.2f)\n"), _MaxIntensity);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MAX_INTENSITY            = %.2f\n"), _MaxIntensity);
      }

   // Inquiry on max pixel intensity mode
   MdmrInquire(DmrCntxId, M_MAX_INTENSITY_MODE + M_TYPE_MIL_INT, &_MaxIntensityMode);
   if (M_DEFAULT == _MaxIntensityMode)
      {
      MdmrInquire(DmrCntxId, M_MAX_INTENSITY_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_MaxIntensityMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_MAX_INTENSITY_MODE       = M_DEFAULT (%s)\n"), DefToStr(_MaxIntensityMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MAX_INTENSITY_MODE       = %s\n"), DefToStr(_MaxIntensityMode));
      }

   // Inquiry on min contrast
   MdmrInquire(DmrCntxId, M_MIN_CONTRAST + M_TYPE_MIL_INT, &_MinContrast);
   if (M_DEFAULT == _MinContrast)
      {
      MdmrInquire(DmrCntxId, M_MIN_CONTRAST + M_TYPE_MIL_INT + M_DEFAULT, &_MinContrast);
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_CONTRAST             = M_DEFAULT (%d)\n"), (long) _MinContrast);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_CONTRAST             = %d\n"), (long) _MinContrast);
      }

   // Inquiry on min contrast mode
   MdmrInquire(DmrCntxId, M_MIN_CONTRAST_MODE + M_TYPE_MIL_INT, &_MinContrastMode);
   if (M_DEFAULT == _MinContrastMode)
      {
      MdmrInquire(DmrCntxId, M_MIN_CONTRAST_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_MinContrastMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_CONTRAST_MODE        = M_DEFAULT (%s)\n"), DefToStr(_MinContrastMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_CONTRAST_MODE        = %s\n"), DefToStr(_MinContrastMode));
      }

   // Inquiry on minimum pixel intensity
   MdmrInquire(DmrCntxId, M_MIN_INTENSITY + M_TYPE_MIL_DOUBLE, &_MinIntensity);
   if (M_DEFAULT == _MinIntensity)
      {
      MdmrInquire(DmrCntxId, M_MIN_INTENSITY + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_MinIntensity);
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_INTENSITY            = M_DEFAULT (%.2f)\n"), _MinIntensity);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_INTENSITY            = %.2f\n"), _MinIntensity);
      }

   // Inquiry on min pixel intensity mode
   MdmrInquire(DmrCntxId, M_MIN_INTENSITY_MODE + M_TYPE_MIL_INT, &_MinIntensityMode);
   if (M_DEFAULT == _MinIntensityMode)
      {
      MdmrInquire(DmrCntxId, M_MIN_INTENSITY_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_MinIntensityMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_INTENSITY_MODE       = M_DEFAULT (%s)\n"), DefToStr(_MinIntensityMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_MIN_INTENSITY_MODE       = %s\n"), DefToStr(_MinIntensityMode));
      }

   // Inquiry on M_SPACE_SIZE_MAX
   MdmrInquire(DmrCntxId, M_SPACE_SIZE_MAX + M_TYPE_MIL_DOUBLE, &_SpaceSizeMax);
   if (M_DEFAULT == _SpaceSizeMax)
      {
      MdmrInquire(DmrCntxId, M_SPACE_SIZE_MAX + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_SpaceSizeMax);
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MAX           = M_DEFAULT (%.2f)\n"), _SpaceSizeMax);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MAX           = %.2f\n"), _SpaceSizeMax);
      }

   // Inquiry on M_SPACE_SIZE_MAX_MODE
   MdmrInquire(DmrCntxId, M_SPACE_SIZE_MAX_MODE + M_TYPE_MIL_INT, &_SpaceSizeMaxMode);
   if (M_DEFAULT == _SpaceSizeMaxMode)
      {
      MdmrInquire(DmrCntxId, M_SPACE_SIZE_MAX_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_SpaceSizeMaxMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MAX_MODE      = M_DEFAULT (%s)\n"), DefToStr(_SpaceSizeMaxMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MAX_MODE      = %s\n"), DefToStr(_SpaceSizeMaxMode));
      }

   // Inquiry on M_SPACE_SIZE_MIN
   MdmrInquire(DmrCntxId, M_SPACE_SIZE_MIN + M_TYPE_MIL_DOUBLE, &_SpaceSizeMin);
   if (M_DEFAULT == _SpaceSizeMin)
      {
      MdmrInquire(DmrCntxId, M_SPACE_SIZE_MIN + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_SpaceSizeMin);
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MIN           = M_DEFAULT (%.2f)\n"), _SpaceSizeMin);
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MIN           = %.2f\n"), _SpaceSizeMin);
      }

   // Inquiry on M_SPACE_SIZE_MIN_MODE
   MdmrInquire(DmrCntxId, M_SPACE_SIZE_MIN_MODE + M_TYPE_MIL_INT, &_SpaceSizeMinMode);
   if (M_DEFAULT == _SpaceSizeMinMode)
      {
      MdmrInquire(DmrCntxId, M_SPACE_SIZE_MIN_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_SpaceSizeMinMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MIN_MODE      = M_DEFAULT (%s)\n"), DefToStr(_SpaceSizeMinMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_SPACE_SIZE_MIN_MODE      = %s\n"), DefToStr(_SpaceSizeMinMode));
      }

   // Inquiry on M_STRING_ANGLE
   MdmrInquire(DmrCntxId, M_STRING_ANGLE + M_TYPE_MIL_DOUBLE, &_StringAngle);
   if (M_DEFAULT == _StringAngle)
      {
      MdmrInquire(DmrCntxId, M_STRING_ANGLE + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_StringAngle);
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE             = M_DEFAULT (%.2f)\n"), _StringAngle);
      }
   else if (M_ACCORDING_TO_REGION == _StringAngle)
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE             = %s\n"), DefToStr(MIL_INT(_StringAngle)));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE             = %.2f\n"), _StringAngle);
      }

   // Inquiry on M_STRING_ANGLE_INPUT_UNITS
   MdmrInquire(DmrCntxId, M_STRING_ANGLE_INPUT_UNITS + M_TYPE_MIL_INT, &_StringAngleInputUnits);
   if (M_DEFAULT == _StringAngleInputUnits)
      {
      MdmrInquire(DmrCntxId, M_STRING_ANGLE_INPUT_UNITS + M_TYPE_MIL_INT + M_DEFAULT, &_StringAngleInputUnits);
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE_INPUT_UNITS = M_DEFAULT (%s)\n"), DefToStr(_StringAngleInputUnits));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE_INPUT_UNITS = %s\n"), DefToStr(_StringAngleInputUnits));
      }

   // Inquiry on M_STRING_ANGLE_MODE
   MdmrInquire(DmrCntxId, M_STRING_ANGLE_MODE + M_TYPE_MIL_INT, &_StringAngleMode);
   if (M_DEFAULT == _StringAngleMode)
      {
      MdmrInquire(DmrCntxId, M_STRING_ANGLE_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_StringAngleMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE_MODE        = M_DEFAULT (%s)\n"), DefToStr(_StringAngleMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ANGLE_MODE        = %s\n"), DefToStr(_StringAngleMode));
      }

   // Inquiry on the height of the rectangular region in which to read strings
   MdmrInquire(DmrCntxId, M_TEXT_BLOCK_HEIGHT + M_TYPE_MIL_DOUBLE, &_TextBlockHeight);
   if (M_DEFAULT == _TextBlockHeight)
      {
      MdmrInquire(DmrCntxId, M_TEXT_BLOCK_HEIGHT + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_TextBlockHeight);
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_HEIGHT        = M_DEFAULT (%s)\n"), DefToStr(MIL_INT(_TextBlockHeight)));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_HEIGHT        = %.2f\n"), _TextBlockHeight);
      }

   // Inquiry on M_TEXT_BLOCK_SIZE_MODE
   MdmrInquire(DmrCntxId, M_TEXT_BLOCK_SIZE_MODE + M_TYPE_MIL_INT, &_TextBlockSizeMode);
   if (M_DEFAULT == _TextBlockSizeMode)
      {
      MdmrInquire(DmrCntxId, M_TEXT_BLOCK_SIZE_MODE + M_TYPE_MIL_INT + M_DEFAULT, &_TextBlockSizeMode);
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_SIZE_MODE     = M_DEFAULT (%s)\n"), DefToStr(_TextBlockSizeMode));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_SIZE_MODE     = %s\n"), DefToStr(_TextBlockSizeMode));
      }

   // Inquiry on width of the rectangular region in which to read strings
   MdmrInquire(DmrCntxId, M_TEXT_BLOCK_WIDTH + M_TYPE_MIL_DOUBLE, &_TextBlockWidth);
   if (M_DEFAULT == _TextBlockWidth)
      {
      MdmrInquire(DmrCntxId, M_TEXT_BLOCK_WIDTH + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_TextBlockWidth);
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_WIDTH         = M_DEFAULT (%s)\n"), DefToStr(MIL_INT(_TextBlockWidth)));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_TEXT_BLOCK_WIDTH         = %.2f\n"), _TextBlockWidth);
      }

   // Inquiry on number of fonts
   MdmrInquire(DmrCntxId, M_NUMBER_OF_FONTS + M_TYPE_MIL_INT, &_NbOfFont);
   MosFprintf(OutputTarget, MIL_TEXT("M_NUMBER_OF_FONTS          = %d\n"), (long) _NbOfFont);

   // Inquiry on number of string models
   _NbOfStringModels = MdmrInquire(DmrCntxId, M_NUMBER_OF_STRING_MODELS, M_NULL);
   MosFprintf(OutputTarget, MIL_TEXT("M_NUMBER_OF_STRING_MODELS  = %d\n"), (long) _NbOfStringModels);

   // Inquiry on time out
   MdmrInquire(DmrCntxId, M_TIMEOUT + M_TYPE_MIL_DOUBLE, &_TimeOut);
   if (M_DEFAULT == _TimeOut)
      {
      MdmrInquire(DmrCntxId, M_TIMEOUT + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_TimeOut);
      MosFprintf(OutputTarget, MIL_TEXT("M_TIMEOUT                  = M_DEFAULT (%.2f)\n"), _TimeOut);
      }
   else if (M_DISABLE == _TimeOut)
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_TIMEOUT                  = %s\n"), DefToStr(MIL_INT(_TimeOut)));
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("M_TIMEOUT                  = %.2f\n"), _TimeOut);
      }

   // Pause for next section
   if (MIL_STDOUT == OutputTarget)
      {
      MosPrintf(MIL_TEXT("\nPress <Enter> to show string model info.\n\n"));
      MosGetch();
      }
   }

// Function to inquire string models
void CDMRInquirer::InquireStringModel(MIL_ID DmrCntxId, MIL_FILE OutputTarget)
   {
   if ((_NbOfStringModels == 0) && (MIL_STDOUT == OutputTarget))
      {
      MosFprintf(OutputTarget, MIL_TEXT("\nNo string models are defined. Press <Enter> to show font info.\n\n"));
      MosGetch();
      return;
      }

   for (MIL_INT StrIndex = 0; StrIndex < _NbOfStringModels; StrIndex++)
      {
      MosFprintf(OutputTarget, MIL_TEXT("\n-------------------------------------------\n"));
      MosFprintf(OutputTarget, MIL_TEXT("String model %d settings:\n"), (long) StrIndex);
      MosFprintf(OutputTarget, MIL_TEXT("-------------------------------------------\n"));

      // Inquiry on character's acceptance level
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_CHAR_ACCEPTANCE + M_TYPE_MIL_DOUBLE, &_CharAcceptance);
      if (M_DEFAULT == _CharAcceptance)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_CHAR_ACCEPTANCE + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_CharAcceptance);
         MosFprintf(OutputTarget, MIL_TEXT("M_CHAR_ACCEPTANCE                 = M_DEFAULT (%.2f)\n"), _CharAcceptance);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_CHAR_ACCEPTANCE                 = %.2f\n"), _CharAcceptance);
         }

      // Inquiry on constraints for different positions of string model
      // Inquiry on number of explicitly constrained positions
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_NUMBER_OF_CONSTRAINED_POSITIONS + M_TYPE_MIL_INT, &_NbOfConstrainedPositions);
      MosFprintf(OutputTarget, MIL_TEXT("M_NUMBER_OF_CONSTRAINED_POSITIONS = %d\n"), (long) _NbOfConstrainedPositions);

      // Inquiry on string score acceptance level
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_ACCEPTANCE + M_TYPE_MIL_DOUBLE, &_StringAcceptance);
      if (M_DEFAULT == _StringAcceptance)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_ACCEPTANCE + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_StringAcceptance);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ACCEPTANCE               = M_DEFAULT (%.2f)\n"), _StringAcceptance);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_ACCEPTANCE               = %.2f\n"), _StringAcceptance);
         }

      // Inquiry on string certainty level
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_CERTAINTY + M_TYPE_MIL_DOUBLE, &_StringCertainty);
      if (M_DEFAULT == _StringCertainty)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_CERTAINTY + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_StringCertainty);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_CERTAINTY                = M_DEFAULT (%.2f)\n"), _StringCertainty);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_CERTAINTY                = %.2f\n"), _StringCertainty);
         }

      // Inquiry on string index value
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_INDEX_VALUE + M_TYPE_MIL_DOUBLE, &_StringIndexValue);
      if (M_DEFAULT == _StringIndexValue)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_INDEX_VALUE + M_TYPE_MIL_DOUBLE + M_DEFAULT, &_StringIndexValue);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_INDEX_VALUE              = M_DEFAULT (%s)\n"), DefToStr(MIL_INT(_StringIndexValue)));
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_INDEX_VALUE              = %.2f\n"), _StringIndexValue);
         }

      // Inquiry on string label value
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_LABEL_VALUE + M_TYPE_MIL_DOUBLE, &_StringLabelValue);
      MosFprintf(OutputTarget, MIL_TEXT("M_STRING_LABEL_VALUE              = %.2f\n"), _StringLabelValue);

      // Inquiry on string rank
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_RANK + M_TYPE_MIL_INT, &_StringRank);
      if (M_DEFAULT == _StringRank)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_RANK + M_TYPE_MIL_INT + M_DEFAULT, &_StringRank);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_RANK                     = M_DEFAULT (%d)\n"), (long) _StringRank);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_RANK                     = %d\n"), (long) _StringRank);
         }

      // Inquiry on maximum number of characters in string
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_SIZE_MAX + M_TYPE_MIL_INT, &_StringSizeMax);
      if (M_DEFAULT == _StringSizeMax)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_SIZE_MAX + M_TYPE_MIL_INT + M_DEFAULT, &_StringSizeMax);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MAX                 = M_DEFAULT (%s)\n"), DefToStr(_StringSizeMax));
         }
      else if (M_INVALID == _StringSizeMax)
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MAX                 = %s\n"), DefToStr(_StringSizeMax));
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MAX                 = %d\n"), (long) _StringSizeMax);
         }

      // Inquiry on minimum number of characters in string
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_SIZE_MIN + M_TYPE_MIL_INT, &_StringSizeMin);
      if (M_DEFAULT == _StringSizeMin)
         {
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_STRING_SIZE_MIN + M_TYPE_MIL_INT + M_DEFAULT, &_StringSizeMin);
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MIN                 = M_DEFAULT (%.d)\n"), (long) _StringSizeMin);
         }
      else if (M_INVALID == _StringSizeMin)
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MIN                 = %s\n"), DefToStr(_StringSizeMin));
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_STRING_SIZE_MIN                 = %d\n"), (long) _StringSizeMin);
         }

      if (_NbOfConstrainedPositions)
         {
         MosFprintf(OutputTarget, MIL_TEXT("\nThe %d constraints for specific positions in string are:\n"), (long) _NbOfConstrainedPositions);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("\nThere is no constraints for specific positions in string model.\n\n"));
         }

      for (MIL_INT CIndex = 0; CIndex < _NbOfConstrainedPositions; ++CIndex)
         {
         // Inquiry on the constraint position
         MIL_INT ConstraintPosition;
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_POSITION_CONSTRAINED_ORDER(CIndex), M_DEFAULT, M_POSITION + M_TYPE_MIL_INT, &ConstraintPosition);
         MosFprintf(OutputTarget, MIL_TEXT("\nPosition %d: \n"), (long) ConstraintPosition);

         // Inquiry on permitted characters for this constraint position
         MIL_INT NbEntries;
         MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_POSITION_CONSTRAINED_ORDER(CIndex), M_DEFAULT, M_NUMBER_OF_PERMITTED_CHARS_ENTRIES + M_TYPE_MIL_INT, &NbEntries);
         if (NbEntries > 0)
            {
            for (MIL_INT j = 0; j<NbEntries; j++)
               {
               // Inquiry on the label of the font
               MIL_INT ConstraintFontLabel;
               MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_POSITION_CONSTRAINED_ORDER(CIndex), j, M_FONT_LABEL_VALUE + M_TYPE_MIL_INT, &ConstraintFontLabel);

               // Inquiry on the type of the constraint
               MIL_INT ConstraintType;
               MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_POSITION_CONSTRAINED_ORDER(CIndex), j, M_TYPE + M_TYPE_MIL_INT, &ConstraintType);

               // Inquiry on the char list
               MIL_STRING CharList;
               if (M_CHAR_LIST == ConstraintType)
                  {
                  MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_POSITION_CONSTRAINED_ORDER(CIndex), j, M_CHAR_LIST, CharList);
                  }

               // Print out to target the constraint info.
               PrintConstraintInfo(DmrCntxId, ConstraintType, &CharList[0], ConstraintFontLabel, OutputTarget);
               }
            }
         }

      // Inquiry on permitted character entries
      MosFprintf(OutputTarget, MIL_TEXT("\nThe implicit constraint is:\n\n"));
      MIL_INT NbEntries;
      MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, M_DEFAULT, M_NUMBER_OF_PERMITTED_CHARS_ENTRIES + M_TYPE_MIL_INT, &NbEntries);
      if (NbEntries > 0)
         {
         for (MIL_INT j = 0; j < NbEntries; j++)
            {
            // Inquiry on label of the font
            MIL_INT ConstraintFontLabel;
            MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, j, M_FONT_LABEL_VALUE + M_TYPE_MIL_INT, &ConstraintFontLabel);

            // Inquiry on the type of the constraint
            MIL_INT ConstraintType;
            MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, j, M_TYPE + M_TYPE_MIL_INT, &ConstraintType);

            // Inquiry on the char list
            MIL_STRING CharList;
            if (M_CHAR_LIST == ConstraintType)
               {
               MdmrInquireStringModel(DmrCntxId, M_STRING_INDEX(StrIndex), M_DEFAULT, j, M_CHAR_LIST, CharList);
               }

            // Print out to target the constraint info.
            PrintConstraintInfo(DmrCntxId, ConstraintType, &CharList[0], ConstraintFontLabel, OutputTarget);
            }
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("Any character from any font is permitted (default behavior).\n"));
         }
      }

   if (MIL_STDOUT == OutputTarget)
      {
      MosFprintf(OutputTarget, MIL_TEXT("\nPress <Enter> to show font info.\n\n"));
      MosGetch();
      }
   }

// Function to inquire and print fonts.
void CDMRInquirer::InquireAndPrintFont(MIL_ID DmrCntxId, MIL_FILE OutputTarget)
   {
   if (_NbOfFont == 0)
      {
      MosFprintf(OutputTarget, MIL_TEXT("\nNo fonts are defined. Press <Enter> to continue.\n\n"));
      return;
      }

   for (MIL_INT FontIndex = 0; FontIndex < _NbOfFont; FontIndex++)
      {
      MosFprintf(OutputTarget, MIL_TEXT("\n-------------------------------------------\n"));
      MosFprintf(OutputTarget, MIL_TEXT("Font Settings:\n"));
      MosFprintf(OutputTarget, MIL_TEXT("-------------------------------------------\n\n"));

      //Inquiry on font label
      MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_LABEL_VALUE + M_TYPE_MIL_INT, &_FontLabel);
      MosFprintf(OutputTarget, MIL_TEXT("M_FONT_LABEL_VALUE   = %d\n"), (long) _FontLabel);

      //Inquiry on the total number of the characters.
      MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_NUMBER_OF_CHARS + M_TYPE_MIL_INT, &_NbOfChars);
      MosFprintf(OutputTarget, MIL_TEXT("M_NUMBER_OF_CHARS    = %d\n"), (long) _NbOfChars);

      //Inquiry on the column and row number of the characters.
      MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_SIZE_COLUMNS + M_TYPE_MIL_INT, &_FontSizeColumns);
      if (_FontSizeColumns == M_DEFAULT)
         {
         MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_SIZE_COLUMNS + M_DEFAULT + M_TYPE_MIL_INT, &_FontSizeColumns);
         MosFprintf(OutputTarget, MIL_TEXT("M_FONT_SIZE_COLUMNS  = M_DEFAULT (%d)\n"), (long) _FontSizeColumns);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_FONT_SIZE_COLUMNS  = %d\n"), (long) _FontSizeColumns);
         }

      MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_SIZE_ROWS + M_TYPE_MIL_INT, &_FontSizeRows);
      if (_FontSizeRows == M_DEFAULT)
         {
         MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_SIZE_ROWS + M_DEFAULT + M_TYPE_MIL_INT, &_FontSizeRows);
         MosFprintf(OutputTarget, MIL_TEXT("M_FONT_SIZE_ROWS     = M_DEFAULT (%d)\n"), (long) _FontSizeRows);
         }
      else
         {
         MosFprintf(OutputTarget, MIL_TEXT("M_FONT_SIZE_ROWS     = %d\n"), (long) _FontSizeRows);
         }

      // Inquiry on template size
      MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), M_DEFAULT, M_NULL, M_FONT_SIZE_TEMPLATE + M_TYPE_MIL_INT, &_FontSizeTemplate);
      MosFprintf(OutputTarget, MIL_TEXT("M_FONT_SIZE_TEMPLATE = %d\n\n"), (long) _FontSizeTemplate);

      // Inquiry on font character
      for (MIL_INT i = 0; i < _NbOfChars; i++)
         {
         // Inquire the length of the character name, including the terminating null character.
         MIL_STRING CharName;
         MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), i, M_NULL, M_CHAR_NAME + M_HEX_UTF16_FOR_NON_BASIC_LATIN, CharName);

         // Inquiry on the character template.
         std::vector<MIL_UINT8> CharTemplate;
         MdmrInquireFont(DmrCntxId, M_FONT_INDEX(FontIndex), i, M_NULL, M_CHAR_TEMPLATE, CharTemplate);

         //Print out each character in the following format:
         MosFprintf   (OutputTarget, MIL_TEXT("Char name = %s\n\n"), &CharName[0]);
         PrintTemplate(OutputTarget, &CharTemplate[0]);
         }
      }
   }

// Function to inquire DMR context.
void CDMRInquirer::Inquire(MIL_ID DmrCntxId, MIL_FILE OutputTarget)
   {
   // Inquire the context settings.
   InquireSettings(DmrCntxId, OutputTarget);

   // Inquire the string models info.
   InquireStringModel(DmrCntxId, OutputTarget);

   // Inquire the font info.
   InquireAndPrintFont(DmrCntxId, OutputTarget);
   }

// Function to output model constraint information.
void CDMRInquirer::PrintConstraintInfo(MIL_ID DmrCntxId, MIL_INT ConstraintType, MIL_CONST_TEXT_PTR CharListPtr, MIL_INT ConstraintFontLabel, MIL_FILE OutputTarget)
   {
   std::basic_stringstream<MIL_TEXT_CHAR> FontLabel;

   if (M_ANY == ConstraintFontLabel)
      {
      FontLabel << MIL_TEXT("M_ANY");
      }
   else
      {
      FontLabel << ConstraintFontLabel;
      }

   if (M_CHAR_LIST == ConstraintType)
      {
      MosFprintf(OutputTarget, MIL_TEXT("\"%s\" from font %s\n"), CharListPtr, FontLabel.str().c_str());
      }
   else
      {
      MosFprintf(OutputTarget, MIL_TEXT("%s from font %s\n"), DefToStr(ConstraintType), FontLabel.str().c_str());
      }
   }

// Function to translate the definitions of the returned parameters to strings.
MIL_CONST_TEXT_PTR CDMRInquirer::DefToStr(MIL_INT TheDef)
   {
   switch (TheDef)
      {
      case M_ACCORDING_TO_REGION: return MIL_TEXT("M_ACCORDING_TO_REGION");
      case M_ANGLE:               return MIL_TEXT("M_ANGLE");
      case M_ANY:                 return MIL_TEXT("M_ANY");
      case M_AUTO:                return MIL_TEXT("M_AUTO");
      case M_CHAR_WIDTH_FACTOR:   return MIL_TEXT("M_CHAR_WIDTH_FACTOR");
      case M_DEFAULT:             return MIL_TEXT("M_DEFAULT");
      case M_DIGITS:              return MIL_TEXT("M_DIGITS");
      case M_DISABLE:             return MIL_TEXT("M_DISABLE");
      case M_ENABLE:              return MIL_TEXT("M_ENABLE");
      case M_FALSE:               return MIL_TEXT("M_FALSE");
      case M_FOREGROUND_BLACK:    return MIL_TEXT("M_FOREGROUND_BLACK");
      case M_FOREGROUND_WHITE:    return MIL_TEXT("M_FOREGROUND_WHITE");
      case M_INVALID:             return MIL_TEXT("M_INVALID");
      case M_LETTERS:             return MIL_TEXT("M_LETTERS");
      case M_LETTERS_LOWERCASE:   return MIL_TEXT("M_LETTERS_LOWERCASE");
      case M_LETTERS_UPPERCASE:   return MIL_TEXT("M_LETTERS_UPPERCASE");
      case M_ORIENTATION:         return MIL_TEXT("M_ORIENTATION");
      case M_PIXEL:               return MIL_TEXT("M_PIXEL");
      case M_SPACE:               return MIL_TEXT("M_SPACE");
      case M_USER_DEFINED:        return MIL_TEXT("M_USER_DEFINED");
      case M_WORLD:               return MIL_TEXT("M_WORLD");
      default:                    return MIL_TEXT("UNKNOWN");
      }
   }

// Function to print the font to output or a text file.
void CDMRInquirer::PrintTemplate(MIL_FILE OutputTarget, MIL_UINT8* CharTemplate)
   {
   // Note:
   //                      00 00 FF 00 00                         *
   //                      00 FF 00 FF 00                      *     *
   //      Print to file   FF 00 00 00 FF    or to screen   *           *
   //     -------------->  FF 00 00 00 FF   ------------->  *           *
   //                      FF FF FF FF FF                   *  *  *  *  *
   //                      FF 00 00 00 FF                   *           *
   //                      FF 00 00 00 FF                   *           *
   //
   for (MIL_INT RowIndex = 0; RowIndex < _FontSizeRows; RowIndex++)
      {
      for (MIL_INT ColIndex = 0; ColIndex < _FontSizeColumns; ColIndex++)
         {
         if (MIL_STDOUT == OutputTarget)
            {
            MosFprintf(OutputTarget, MIL_TEXT("%s "), CharTemplate[RowIndex*_FontSizeColumns + ColIndex] == 0 ? MIL_TEXT(" ") : MIL_TEXT("*"));
            }
         else
            {
            MosFprintf(OutputTarget, MIL_TEXT("%s "), CharTemplate[RowIndex*_FontSizeColumns + ColIndex] == 0 ? MIL_TEXT("00") : MIL_TEXT("FF"));
            }
         }
      MosFprintf(OutputTarget, MIL_TEXT("\n"));
      }
   MosFprintf(OutputTarget, (MIL_TEXT("\n\n")));
   }

// COutputTarget member function definitions

// Open output file to screen or text file.
COutputTarget::COutputTarget()
   {
   // Let user choose to print or save the context content to a text file.
   MIL_INT ch;
   do
      {
      MosPrintf(
         MIL_TEXT("Press <1> to print the context content to screen.\n")
         MIL_TEXT("Press <2> to save the context content to a text file.\n\n"));
      ch = MosGetch();
      } while(ch != '1' && ch != '2');

      if(ch == '1')
         {
         _OutputTarget = MIL_STDOUT;
         }
      else
         {
         _OutputTarget = MosFopen(OUTPUT_FILE, MIL_CONST_TEXT_PTR("w"));
         if(_OutputTarget == M_NULL)
            {
            MosPrintf(MIL_TEXT("File cannot be created.\n"));
            }
         }
   }

// Close output file.
COutputTarget::~COutputTarget()
   {
   if(_OutputTarget && MIL_STDOUT != _OutputTarget)
      {
      MosFclose(_OutputTarget);
      MosPrintf(MIL_TEXT("MdmrContext.txt has been saved in the temp folder.\n"));
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to finish.\n\n"));
   MosGetch();
   }

// Type cast to MIL_FILE
COutputTarget::operator MIL_FILE()
   {
   return _OutputTarget;
   }
