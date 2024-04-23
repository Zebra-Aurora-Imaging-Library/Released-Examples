//***************************************************************************************/
//
// File name: CodeDetect.cpp
//
// Synopsis:  This program creates a code reader context using the
//            automatically detected code types from a sample image.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <vector>

///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("CodeDetect\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program creates a code reader context using the\n")
             MIL_TEXT("automatically detected code types from a sample image.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, code.\n\n"));
   }

// Util constants
#define LARGE_SIZE_IMAGE_LIMIT 2000
#define MID_SIZE_IMAGE_LIMIT    700

#define LARGE_SIZE_IMAGE_ZOOM_FACTOR  0.25
#define MID_SIZE_IMAGE_ZOOM_FACTOR    0.5
#define SMALL_SIZE_IMAGE_ZOOM_FACTOR  1.0

#define TITLE_OFFSET_X            5
#define TITLE_OFFSET_Y            5
#define STRING_ELEMENT_OFFSET_Y  10

const MIL_INT NumberOfSampleImages = 4;

static MIL_CONST_TEXT_PTR ImageFilename[NumberOfSampleImages] =
   {
   M_IMAGE_PATH MIL_TEXT("CodeDetect/DetectExample_4codes.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeDetect/DetectExample_3codes.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeDetect/DetectExample_2codes.mim"),
   M_IMAGE_PATH MIL_TEXT("CodeDetect/DetectExample_6codes.mim")
   };

// The detection function requires the number of expected barcodes.
static const MIL_INT NumberOfBarCodesPerImage[NumberOfSampleImages] =
   {4, 3, 2, 6};

//************************************
// Utility sub-functions declaration
//************************************

// COverlay: used to encapsulate drawing operations.
class COverlay
   {
   public:
      COverlay();
      ~COverlay();

      void        FreeMilObjects();
      MIL_ID      GetId() { return m_CurrentGraphicList; }

      void        Reset();
      void        SetTitle(MIL_CONST_TEXT_PTR TitleString);
      MIL_ID      GetBoundingBoxCtx() { return m_BoundingBoxGraCtx; }
      void        WriteTextElement(MIL_DOUBLE TextPosX, MIL_DOUBLE TextPosY, MIL_CONST_TEXT_PTR StringToWrite);

   private:
      //Graphic contexts used in the graphic list
      MIL_ID m_TitleGraCtx;
      MIL_ID m_CodeRelatedTextGraCtx;
      MIL_ID m_BoundingBoxGraCtx;

      //Current graphic list
      MIL_ID m_CurrentGraphicList;

   };

void PrepareDisplayAndAnnotation(MIL_ID MilSystem,
                                 MIL_ID MilSrcImage,
                                 MIL_ID MilDisplay);


//************************************
// DETECT CODES
//************************************
void DetectCodes(MIL_ID               SystemId,
                 MIL_ID               Image,
                 MIL_INT              NbBarCodeInImage,
                 std::vector<MIL_INT> ListOfCodeTypeToDetect,
                 MIL_ID               &MilCodeResult,
                 std::vector<MIL_INT> &CodesDetectedType, 
                 COverlay             &DetectOverlay, 
                 COverlay             &ReadOverlay)
   {
   MIL_INT NbBarCodeDetected  = 0;
   MIL_INT CodeTypeStringSize = 0;
   std::vector<MIL_TEXT_CHAR> CodeTypeString;

   // Allocate a code result object for code type detection, if not already done
   if(MilCodeResult != M_NULL)
      {
      McodeFree(MilCodeResult);
      MilCodeResult = M_NULL;
      }
   if(MilCodeResult == M_NULL)
      MilCodeResult = McodeAllocResult(SystemId, M_CODE_DETECT_RESULT, M_NULL);

   MosPrintf(MIL_TEXT("Detecting code types..."));
   DetectOverlay.SetTitle(MIL_TEXT("CODE TYPES DETECTION"));

   // Detect the codes present in the image.
   McodeDetect(Image,
               (MIL_INT)ListOfCodeTypeToDetect.size(),
               &ListOfCodeTypeToDetect[0],
               NbBarCodeInImage,
               M_DEFAULT,
               M_DEFAULT,
               MilCodeResult);

   MosPrintf(MIL_TEXT(" Done!\n\n"));

   // Retrieve the number of automatically detected codes.
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbBarCodeDetected);
   MosPrintf(MIL_TEXT("%i barcodes detected on %i expected barcodes\n"), NbBarCodeDetected, NbBarCodeInImage);

   // Retrieve some detection results.
   if(NbBarCodeDetected > 0)
      {
      //Retrieve code types of codes detected.
      CodesDetectedType.resize(NbBarCodeDetected);
      McodeGetResult(MilCodeResult, M_ALL, M_GENERAL, M_CODE_TYPE + M_TYPE_MIL_INT, &CodesDetectedType[0]);

      // Display annotations.
      for(MIL_INT jj = 0; jj < NbBarCodeDetected; jj++)
         {
         // Display the bounding box of the detected codes.
         McodeDraw(DetectOverlay.GetBoundingBoxCtx(), MilCodeResult, DetectOverlay.GetId(), M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);

         // Retrieve the string of the code type.
         McodeGetResult(MilCodeResult, jj, M_GENERAL, M_CODE_TYPE_NAME + M_STRING_SIZE + M_TYPE_MIL_INT, &CodeTypeStringSize);
         CodeTypeString.resize(CodeTypeStringSize);
         McodeGetResult(MilCodeResult, jj, M_GENERAL, M_CODE_TYPE_NAME + M_TYPE_TEXT_CHAR, &CodeTypeString[0]);
         MosPrintf(MIL_TEXT("Type detected:  %s\n"), &CodeTypeString[0]);

         // Annotate code type both in the Detect's graphic list and in the Read's graphic list.
         double DrawPosX;
         double DrawPosY;
         McodeGetResult(MilCodeResult, jj, M_GENERAL, M_BOTTOM_RIGHT_X, &DrawPosX);
         McodeGetResult(MilCodeResult, jj, M_GENERAL, M_BOTTOM_RIGHT_Y, &DrawPosY);
         DrawPosY += STRING_ELEMENT_OFFSET_Y;

         DetectOverlay.WriteTextElement(DrawPosX, DrawPosY, &CodeTypeString[0]);
         ReadOverlay.WriteTextElement(DrawPosX, DrawPosY, &CodeTypeString[0]);
         }
      }
   }

//************************************
// READ CODES
//************************************
void ReadCodesAndOutputString(MIL_ID   SystemId, 
                              MIL_ID   Image, 
                              MIL_ID   CodeContext, 
                              COverlay &ReadOverlay)
   {
   MIL_INT SizeDecodedString = 0;
   MIL_INT NumberCodesRead   = 0;
   std::vector<MIL_TEXT_CHAR> DecodedString;
   
   // Allocate a code result for code reading.
   MIL_ID ReadCodeResultId = McodeAllocResult(SystemId, M_DEFAULT, M_NULL);

   // Set the context to read all code occurrences.
   McodeControl(CodeContext, M_NUMBER, M_ALL);

   MosPrintf(MIL_TEXT("Reading codes..."));
   ReadOverlay.SetTitle(MIL_TEXT("READING CODES"));

   // Reading the codes.
   McodeRead(CodeContext, Image, ReadCodeResultId);
   MosPrintf(MIL_TEXT(" Done!\n\n"));

   // Retrieving the number of read codes.
   McodeGetResult(ReadCodeResultId, M_GENERAL, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NumberCodesRead);
   MosPrintf(MIL_TEXT("%i barcodes read\n"), NumberCodesRead);

   // Retrieving and displaying the decoded strings.
   for(MIL_INT jj = 0; jj < NumberCodesRead; jj++)
      {
      // Draw the bounding box of the read code.
      McodeDraw(ReadOverlay.GetBoundingBoxCtx(), ReadCodeResultId, ReadOverlay.GetId(), M_DRAW_BOX, jj, M_GENERAL, M_DEFAULT);

      // Retrieving the string.
      McodeGetResult(ReadCodeResultId, jj, M_GENERAL, M_STRING + M_STRING_SIZE + M_TYPE_MIL_INT, &SizeDecodedString);
      DecodedString.resize(SizeDecodedString);
      McodeGetResult(ReadCodeResultId, jj, M_GENERAL, M_STRING, &DecodedString[0]);
      MosPrintf(MIL_TEXT("Decoded string:  %s\n"), &DecodedString[0]);

      // Annotate.
      double DrawPosX;
      double DrawPosY;
      McodeGetResult(ReadCodeResultId, jj, M_GENERAL, M_POSITION_X, &DrawPosX);
      McodeGetResult(ReadCodeResultId, jj, M_GENERAL, M_POSITION_Y, &DrawPosY);
      DrawPosY += STRING_ELEMENT_OFFSET_Y;

      ReadOverlay.WriteTextElement(DrawPosX, DrawPosY, &DecodedString[0]);
      }

   // Release allocated object.
   McodeFree(ReadCodeResultId);
   }

int MosMain()
   {
   // Allocate the MIL application, system, and display.
   MIL_ID MilApplication   = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem        = M_DEFAULT_HOST;
   MIL_ID MilDisplay       = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MIL_ID MilCodeContext   = M_NULL;
   MIL_ID MilCodeResult    = M_NULL;

   // Allocate a code reader result object for code type detection
   MilCodeResult = McodeAllocResult(MilSystem, M_CODE_DETECT_RESULT, M_NULL);

   // Allocate a code reader context object that will be used for the reading 
   MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);
   
   std::vector<MIL_INT> CodesDetectedType;
   COverlay DetectOverlay;
   COverlay ReadOverlay;

   // The detection can be constrained to a category or a list of code types.
   std::vector<MIL_INT> ListOfCodeTypes[NumberOfSampleImages];
   ListOfCodeTypes[0].push_back(M_SUPPORTED_CODE_TYPES_DETECT); // will search in all code types supported by the detect (equivalent to M_DEFAULT)
   ListOfCodeTypes[1].push_back(M_SUPPORTED_CODE_TYPES_DETECT); // will search in all code types supported by the detect (equivalent to M_DEFAULT)
   ListOfCodeTypes[2].push_back(M_CODE39);                      // Restrict the detection to these 4 code types
   ListOfCodeTypes[2].push_back(M_CODE93);                      // Restrict the detection to these 4 code types
   ListOfCodeTypes[2].push_back(M_CODE128);                     // Restrict the detection to these 4 code types
   ListOfCodeTypes[2].push_back(M_EAN13);                       // Restrict the detection to these 4 code types
   ListOfCodeTypes[3].push_back(M_SUPPORTED_CODE_TYPES_DETECT); // will search in all code types supported by the detect (equivalent to M_DEFAULT)

   // Print the example header.
   PrintHeader();

   MosPrintf(MIL_TEXT("Starting automatic code type detection:\n\n"));

   for (MIL_INT ii = 0; ii < NumberOfSampleImages; ii++)
      {
      // Restore the image.
      MIL_ID MilSrcImage = MbufRestore(ImageFilename[ii], MilSystem, M_NULL);
      MosPrintf(MIL_TEXT("Image %i\n\n"), ii);

      // Reset the overlays.
      DetectOverlay.Reset();
      ReadOverlay.Reset();

      // Reset the display.
      PrepareDisplayAndAnnotation(MilSystem, MilSrcImage, MilDisplay);

      /////////////////////////////////////////////////////
      // Detect the types of codes present in the image. //
      /////////////////////////////////////////////////////
      MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, DetectOverlay.GetId());
      CodesDetectedType.clear();

      DetectCodes(MilSystem, MilSrcImage, NumberOfBarCodesPerImage[ii], ListOfCodeTypes[ii], MilCodeResult, CodesDetectedType, DetectOverlay, ReadOverlay);

      MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
      MosGetch();

      //////////////////////////////////////////////////////////////////////
      // Populating the code reader context with the detected code types. //
      //////////////////////////////////////////////////////////////////////
      MosPrintf(MIL_TEXT("Populating a context from the detected code types..."));
      if(MilCodeContext != M_NULL)
         {
         McodeModel(MilCodeContext, M_RESET_FROM_DETECTED_RESULTS, M_NULL, M_ALL, MilCodeResult, M_NULL);
         }
      MosPrintf(MIL_TEXT(" Done!\n\n"));

      ////////////////////////////////////
      // Using the code reader context. //
      ////////////////////////////////////
      MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, ReadOverlay.GetId());
      ReadCodesAndOutputString(MilSystem, MilSrcImage, MilCodeContext, ReadOverlay);

      MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
      MosGetch();

      MbufFree(MilSrcImage);
      } // End for each image

   MosPrintf(MIL_TEXT("\nPress <Enter> to terminate.\n\n"));
   MosGetch();

   // Release allocated object.
   DetectOverlay.FreeMilObjects();
   ReadOverlay.FreeMilObjects();
   MdispFree(MilDisplay);
   if(MilCodeResult != M_NULL)
      McodeFree(MilCodeResult);
   if(MilCodeContext != M_NULL)
      McodeFree(MilCodeContext);
   if (MilSystem != M_DEFAULT_HOST)
      MsysFree(MilSystem);
   
   MappFree(MilApplication);

   return 0;
   }

//************************************
// Utility sub-functions definitions
//************************************

//************************************
// SETUP DISPLAY
//************************************
void PrepareDisplayAndAnnotation(MIL_ID MilSystem,
                                 MIL_ID MilSrcImage,
                                 MIL_ID MilDisplay)
   {
   // Retrieve the source image size.
   MIL_INT SrcSizeX, SrcSizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SrcSizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SrcSizeY);


   if (SrcSizeX > LARGE_SIZE_IMAGE_LIMIT && SrcSizeY > LARGE_SIZE_IMAGE_LIMIT)
      {
      MdispZoom(MilDisplay, LARGE_SIZE_IMAGE_ZOOM_FACTOR, LARGE_SIZE_IMAGE_ZOOM_FACTOR);
      }
   else if(SrcSizeX > MID_SIZE_IMAGE_LIMIT && SrcSizeY > MID_SIZE_IMAGE_LIMIT)
      {
      MdispZoom(MilDisplay, MID_SIZE_IMAGE_ZOOM_FACTOR, MID_SIZE_IMAGE_ZOOM_FACTOR);
      }
   else
      {
      MdispZoom(MilDisplay, SMALL_SIZE_IMAGE_ZOOM_FACTOR, SMALL_SIZE_IMAGE_ZOOM_FACTOR);
      }

   // Display the image buffer.
   MdispSelect(MilDisplay, MilSrcImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   }


//***************************************************************************
// COverlay member functions
//***************************************************************************
COverlay::COverlay()
   {
   //Allocate different graphic elements
   m_CurrentGraphicList    = MgraAllocList(M_DEFAULT_HOST, M_DEFAULT, M_NULL);
   m_TitleGraCtx           = MgraAlloc(M_DEFAULT_HOST, M_NULL);
   m_CodeRelatedTextGraCtx = MgraAlloc(M_DEFAULT_HOST, M_NULL);
   m_BoundingBoxGraCtx     = MgraAlloc(M_DEFAULT_HOST, M_NULL);

   //Set graphic elements context colors
   MgraColor(m_TitleGraCtx, M_COLOR_CYAN);
   MgraBackColor(m_TitleGraCtx, M_COLOR_GRAY);
   MgraControl(m_TitleGraCtx, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);

   MgraColor(m_CodeRelatedTextGraCtx, M_COLOR_CYAN);
   MgraBackColor(m_CodeRelatedTextGraCtx, M_COLOR_GRAY);
   MgraControl(m_CodeRelatedTextGraCtx, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);

   MgraColor(m_BoundingBoxGraCtx, M_COLOR_GREEN);
   }

COverlay::~COverlay()
   {
   FreeMilObjects();
   }

void COverlay::FreeMilObjects()
   {
   if(m_CurrentGraphicList)
      {
      MgraFree(m_CurrentGraphicList);
      m_CurrentGraphicList = M_NULL;
      }

   if(m_TitleGraCtx)
      {
      MgraFree(m_TitleGraCtx);
      m_TitleGraCtx = M_NULL;
      }

   if(m_CodeRelatedTextGraCtx)
      {
      MgraFree(m_CodeRelatedTextGraCtx);
      m_CodeRelatedTextGraCtx = M_NULL;
      }

   if(m_BoundingBoxGraCtx)
      {
      MgraFree(m_BoundingBoxGraCtx);
      m_BoundingBoxGraCtx = M_NULL;
      }
   }

void COverlay::Reset()
   {
   if(m_CurrentGraphicList)
      MgraClear(M_DEFAULT, m_CurrentGraphicList);
   }

void COverlay::SetTitle(MIL_CONST_TEXT_PTR TitleString)
   {
   MgraText(m_TitleGraCtx, m_CurrentGraphicList, TITLE_OFFSET_X, TITLE_OFFSET_Y, TitleString);
   }

void COverlay::WriteTextElement(MIL_DOUBLE TextPosX, MIL_DOUBLE TextPosY, MIL_CONST_TEXT_PTR StringToWrite)
   {
   MgraText(m_CodeRelatedTextGraCtx, m_CurrentGraphicList, TextPosX, TextPosY, StringToWrite);
   }
