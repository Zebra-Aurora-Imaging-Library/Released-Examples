//*************************************************************************************
//
// File name: ClassPrintedChar.cpp
//
// Synopsis:  This example demonstrates the application of the classification
//            module to an OCR application. 
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved.

#include <mil.h>
#include <algorithm>

using namespace std;

// Path definitions.
#define EXAMPLE_IMAGE_DIR_PATH   M_IMAGE_PATH MIL_TEXT("/Classification/PrintedChar/")
#define EXAMPLE_CLASS_CTX_PATH   EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("MatroxNet_PrintedCharEx.mclass")
#define EXAMPLE_STR_ELEM_PATH    EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("StructElement.mim")
#define TARGET_IMAGE_DIR_PATH    EXAMPLE_IMAGE_DIR_PATH MIL_TEXT("Products/")

// Use the images from the example folder by default.
#define USE_EXAMPLE_IMAGE_FOLDER

// Util constants.
#define DISP_WINDOW_SIZE_X           600
#define DISP_WINDOW_SIZE_Y           250
#define DISP_BAR_SIZE_X              110

#define START_Y_TILE                 130
#define START_Y_CHAR                 190
#define START_Y_SCORE                210

#define BUFFERING_SIZE_MAX           10
#define NORMALIZE_CHAR_WINDOW_SIZE   32
#define NORMALIZE_CHAR_WINDOW_MARGIN 9
#define INTENSITY_OFFSET             5
#define BUFF_SIZE                    256

// Struct declarations.
struct charBox
   {
   MIL_INT BBoxStx,
      BBoxSty,
      BBoxEnx,
      BBoxEny;
   };


#ifdef USE_EXAMPLE_IMAGE_FOLDER
#define SYSTEM_TO_USE M_SYSTEM_HOST
#define DCF_TO_USE TARGET_IMAGE_DIR_PATH
#else
#define SYSTEM_TO_USE M_SYSTEM_DEFAULT
#define DCF_TO_USE MIL_TEXT("M_DEFAULT")
#endif 

// Function declarations.
void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_ID  ClassCtx,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilOverlay);

void ResetDisplay(MIL_ID MilDisplay,
                  MIL_ID MilBlobRes,
                  MIL_ID MilImage,
                  MIL_ID MilDispImage);

MIL_DOUBLE Median(vector<MIL_INT> scores);

vector<charBox> BuildBboxDict(MIL_ID img,
                              vector<MIL_INT> &MilBoxMinX,
                              vector<MIL_INT> &MilBoxMaxX,
                              vector<MIL_INT> &MilBoxMinY,
                              vector<MIL_INT> &MilBoxMaxY);

void GetBbox(MIL_ID MilSystem,
             MIL_ID MilImage,
             MIL_ID MimCtx,
             MIL_ID MilBlobCtx,
             MIL_ID MilBlobRes,
             MIL_ID MilAdaptiveCtx,
             vector<MIL_INT> &MilBoxMinX,
             vector<MIL_INT> &MilBoxMaxX,
             vector<MIL_INT> &MilBoxMinY,
             vector<MIL_INT> &MilBoxMaxY);

void Saturate(MIL_INT &sx,
              MIL_INT &sy,
              MIL_INT &ex,
              MIL_INT &ey,
              MIL_INT w,
              MIL_INT h);

void GetNormalizedChar(MIL_ID MilSystem,
                       MIL_ID MilClassCtx,
                       MIL_ID MilClassRes,
                       MIL_ID MilImage,
                       MIL_ID MilDestImage,
                       MIL_INT StartX,
                       MIL_INT StartY,
                       MIL_INT EndX,
                       MIL_INT EndY);

void UpdateDispChar(MIL_ID MilDispImage,
                    MIL_ID MilPredImage,
                    MIL_ID MilOverlayImg,
                    MIL_STRING ReadChar,
                    MIL_DOUBLE BestScore,
                    MIL_INT StringLenght,
                    MIL_INT id);

void ProcessStatus(MIL_INT Status);

///****************************************************************************
//    Main.
///****************************************************************************
int main(void)
   {
   MIL_ID MilApplication,     // MIL application identifier.
      MilSystem,              // MIL system identifier.
      MilDisplay,             // MIL display identifier.
      MilOverlay,             // MIL overlay identifier.
      MilImage,               // MIL image identifier.
      MilPreProc,             // MIL image identifier.
      MilDispImage,           // MIL image identifier.
      MilMimCtx,              // MIL Image Processing context. 
      MilClassCtx,            // MIL Classification Context.
      MilClassRes,            // MIL Classification Result.
      MilBlobCtx,             // MIL Blob Context.
      MilBlobRes,             // MIL Blob Result.
      MilAdaptiveCtx,         // MIL Adaptive Binarization Context.
      MilPredInpImg,          // MIL image identifier.
      MilStructElement;       // MIL structured element.

   MIL_INT NumberOfCategories,
      InputSizeX,
      InputSizeY,
      InputType,
      BestClass;

   MIL_DOUBLE BestScore;

   std::vector<MIL_STRING> Images;
   Images.reserve(16);
   Images.push_back(MIL_TEXT("Img_01.bmp"));
   Images.push_back(MIL_TEXT("Img_02.bmp"));
   Images.push_back(MIL_TEXT("Img_03.bmp"));
   Images.push_back(MIL_TEXT("Img_04.bmp"));
   Images.push_back(MIL_TEXT("Img_05.bmp"));
   Images.push_back(MIL_TEXT("Img_06.bmp"));
   Images.push_back(MIL_TEXT("Img_07.bmp"));
   Images.push_back(MIL_TEXT("Img_08.bmp"));
   Images.push_back(MIL_TEXT("Img_09.bmp"));
   Images.push_back(MIL_TEXT("Img_10.bmp"));

   // Print the example synopsis.
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("ClassPrintedChar\n\n"));
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the application of the classification module\n"));
   MosPrintf(MIL_TEXT("in OCR. Characters are first detected and pre-processed. A pre-trained\n"));
   MosPrintf(MIL_TEXT("classification context is then used to identify the character.\n"));

   MosPrintf(MIL_TEXT("\n"));
   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Classification, Blob Analysis, Buffer, Display, Graphics, Image Processing.\n\n"));

   // Allocate MIL objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, SYSTEM_TO_USE, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);

   // Wait for user.
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetchar();

   // Load structured element.
   MbufImport(EXAMPLE_STR_ELEM_PATH, M_DEFAULT, M_RESTORE + M_NO_GRAB + M_NO_COMPRESS, MilSystem, &MilStructElement);

   // Blob allocation and controls.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobCtx);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobRes);

   MblobControl(MilBlobCtx, M_FOREGROUND_VALUE, M_ZERO);
   MblobControl(MilBlobCtx, M_BOX, M_ENABLE);
   MblobControl(MilBlobCtx, M_SORT1, M_BOX_X_MAX);
   MblobControl(MilBlobCtx, M_SORT1_DIRECTION, M_SORT_UP);

   // Adaptive Binarization allocation and controls.
   MimAlloc(MilSystem, M_BINARIZE_ADAPTIVE_CONTEXT, M_DEFAULT, &MilAdaptiveCtx);
   MimControl(MilAdaptiveCtx, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);
   MimControl(MilAdaptiveCtx, M_AVERAGE_MODE, M_GAUSSIAN);
   MimControl(MilAdaptiveCtx, M_MINIMUM_CONTRAST, 4);
   MimControl(MilAdaptiveCtx, M_LOCAL_DIMENSION, 40);

   // Image Processnig context to perform smoothing operation
   MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, &MilMimCtx);
   MimControl(MilMimCtx, M_FILTER_OPERATION, M_SMOOTH);
   MimControl(MilMimCtx, M_FILTER_TYPE, M_DERICHE);

   // Restore Classification context.
   MosPrintf(MIL_TEXT("Restoring the classification context from file.."));
   MclassRestore(EXAMPLE_CLASS_CTX_PATH, MilSystem, M_DEFAULT, &MilClassCtx);
   MosPrintf(MIL_TEXT("."));

   // Preprocess the context.
   MclassPreprocess(MilClassCtx, M_DEFAULT);
   MosPrintf(MIL_TEXT(".ready.\n"));

   // Inquire classification context information. 
   MclassInquire(MilClassCtx, M_CONTEXT, M_NUMBER_OF_CLASSES + M_TYPE_MIL_INT, &NumberOfCategories);
   MclassInquire(MilClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_X + M_TYPE_MIL_INT, &InputSizeX);
   MclassInquire(MilClassCtx, M_DEFAULT_SOURCE_LAYER, M_SIZE_Y + M_TYPE_MIL_INT, &InputSizeY);
   MclassInquire(MilClassCtx, M_DEFAULT_SOURCE_LAYER, M_TYPE + M_TYPE_MIL_INT, &InputType);

   // Inquire and print source layer information.
   MosPrintf(MIL_TEXT(" - The classifier was trained to recognize %d categories.\n"), NumberOfCategories);
   MosPrintf(MIL_TEXT(" - The classifier was trained for %dx%d source images.\n\n"), InputSizeX, InputSizeY);

   // Allocate a classification result buffer.
   MclassAllocResult(MilSystem, M_PREDICT_CNN_RESULT, M_DEFAULT, &MilClassRes);

   // Allocate buffer used for prediction. 
   MbufAlloc2d(MilSystem, InputSizeX, InputSizeY, InputType, M_IMAGE + M_PROC, &MilPredInpImg);

   // Setup the example display.
   SetupDisplay(MilSystem, MilDisplay, MilClassCtx, MilDispImage, MilOverlay);

   for(size_t i = 0; i < Images.size(); i++)
      {
      // Stop updating display.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Load images.
      MIL_STRING filename = DCF_TO_USE + Images[i];
      MbufRestore(filename, MilSystem, &MilImage);

      // Preprocessing.
      MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &MilPreProc);
      MimMorphic(MilImage, MilPreProc, MilStructElement, M_BOTTOM_HAT, 1, M_GRAYSCALE);

      // Remove background noise.
      MimArith(MilPreProc, INTENSITY_OFFSET, MilPreProc, M_SUB_CONST + M_SATURATION);

      // Note: extra step, solely for improving visualization.
      MimArith(MilPreProc, M_NULL, MilPreProc, M_NOT);

      vector<MIL_INT> MilBoxMinX,
         MilBoxMaxX,
         MilBoxMinY,
         MilBoxMaxY;

      // Get Bounding box of each character in the string. 
      GetBbox(MilSystem,
              MilImage,
              MilMimCtx,
              MilBlobCtx,
              MilBlobRes,
              MilAdaptiveCtx,
              MilBoxMinX,
              MilBoxMaxX,
              MilBoxMinY,
              MilBoxMaxY);

      // Normalize Bounding Boxes.
      auto CharDictionary = BuildBboxDict(MilImage,
                                          MilBoxMinX,
                                          MilBoxMaxX,
                                          MilBoxMinY,
                                          MilBoxMaxY);

      // Reset the display for new string.
      ResetDisplay(MilDisplay, MilBlobRes, MilImage, MilDispImage);

      MIL_INT charId = 0;
      MIL_STRING readChar;

      MosPrintfA("\n  String: ");

      for(auto charInf : CharDictionary)
         {
         GetNormalizedChar(MilSystem,
                           MilClassCtx,
                           MilClassRes,
                           MilPreProc,
                           MilPredInpImg,
                           charInf.BBoxStx,
                           charInf.BBoxSty,
                           charInf.BBoxEnx,
                           charInf.BBoxEny);

         // Classify the extracted character.
         MclassPredict(MilClassCtx, MilPredInpImg, MilClassRes, M_DEFAULT);

         MIL_INT Status {};
         MclassGetResult(MilClassRes, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);
         ProcessStatus(Status);

         // Get prediction result.
         MclassGetResult(MilClassRes, M_DEFAULT, M_BEST_CLASS_INDEX + M_TYPE_MIL_INT, &BestClass);
         MclassGetResult(MilClassRes, M_DEFAULT, M_BEST_CLASS_SCORE, &BestScore);

         // Get the character.
         MclassInquire(MilClassCtx, M_CLASS_INDEX(BestClass), M_CLASS_NAME, readChar);

         // Update the display with the new read character. 
         UpdateDispChar(MilDispImage,
                        MilPredInpImg,
                        MilOverlay,
                        readChar,
                        BestScore,
                        CharDictionary.size(),
                        charId);

         charId++;
         MosPrintfA("%s", readChar.c_str());
         }
      MosPrintfA("\n\n");

      // Update display.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
      MosGetch();

      // Free allocated buffers. 
      MbufFree(MilPreProc);
      MbufFree(MilImage);
      }

   // Free allocated resources.
   MbufFree(MilDispImage);
   MbufFree(MilPredInpImg);
   MbufFree(MilStructElement);

   MclassFree(MilClassRes);
   MclassFree(MilClassCtx);

   MblobFree(MilBlobCtx);
   MblobFree(MilBlobRes);

   MimFree(MilMimCtx);
   MimFree(MilAdaptiveCtx);

   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

void ProcessStatus(MIL_INT Status)
   {
   if(Status != M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("The prediction failed to complete.\n"));
      MosPrintf(MIL_TEXT("The status returned was: "));
      switch(Status)
         {
         default:
         case M_INTERNAL_ERROR:
            MosPrintf(MIL_TEXT("M_INTERNAL_ERROR\n"));
            break;
         case M_PREDICT_NOT_PERFORMED:
            MosPrintf(MIL_TEXT("M_PREDICT_NOT_PERFORMED\n"));
            break;
         case M_CURRENTLY_PREDICTING:
            MosPrintf(MIL_TEXT("M_CURRENTLY_PREDICTING\n"));
            break;
         case M_STOPPED_BY_REQUEST:
            MosPrintf(MIL_TEXT("M_STOPPED_BY_REQUEST\n"));
            break;
         case M_TIMEOUT_REACHED:
            MosPrintf(MIL_TEXT("M_TIMEOUT_REACHED\n"));
            break;
         case M_NOT_ENOUGH_MEMORY:
            MosPrintf(MIL_TEXT("M_NOT_ENOUGH_MEMORY\n"));
            break;
         }
      }
   }

// Set up the display.
void SetupDisplay(MIL_ID  MilSystem,
                  MIL_ID  MilDisplay,
                  MIL_ID  ClassCtx,
                  MIL_ID  &MilDispImage,
                  MIL_ID  &MilOverlay
)
   {
   // Allocate a color buffer.
   MbufAllocColor(MilSystem, 3, DISP_WINDOW_SIZE_X, DISP_WINDOW_SIZE_Y, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilDispImage);

   MbufClear(MilDispImage, M_COLOR_BLACK);

   // Display the window with black color.
   MdispSelect(MilDisplay, MilDispImage);

   // Prepare for overlay annotations.
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   }

// Reset the display for next image. 
void ResetDisplay(MIL_ID MilDisplay,
                  MIL_ID MilBlobRes,
                  MIL_ID MilImage,
                  MIL_ID MilDispImage)
   {
   MIL_ID MilDispChild,
      MilImageChild;

   MIL_INT Sx = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT Sy = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   MIL_INT offX = DISP_BAR_SIZE_X + (DISP_WINDOW_SIZE_X - DISP_BAR_SIZE_X - Sx) / 2;
   MIL_INT offY = 30;

   // Set background to bright gray.
   MbufClear(MilDispImage, M_COLOR_BRIGHT_GRAY);

   // Clear display overlay.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   // Change background color of the side bar. 
   MbufChild2d(MilDispImage, 0, 0, DISP_BAR_SIZE_X, DISP_WINDOW_SIZE_Y, &MilDispChild);
   MbufClear(MilDispChild, M_COLOR_GRAY);

   // Labels.
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraText(M_DEFAULT, MilDispChild, 8, offY + 30, MIL_TEXT("Input Image"));
   MgraText(M_DEFAULT, MilDispChild, 8, START_Y_TILE + 25, MIL_TEXT("   Tiles"));
   MgraText(M_DEFAULT, MilDispChild, 8, START_Y_CHAR, MIL_TEXT(" Characters"));
   MgraText(M_DEFAULT, MilDispChild, 8, START_Y_SCORE, MIL_TEXT("   Scores"));

   // Copy the string into display image.
   MbufChild2d(MilDispImage, offX, offY, Sx, Sy, &MilImageChild);
   MbufCopy(MilImage, MilImageChild);

   // Draw characters' boxes. 
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MblobDraw(M_DEFAULT, MilBlobRes, MilImageChild, M_DRAW_BOX, M_INCLUDED_BLOBS, M_DEFAULT);

   // Free allocated buffers.
   MbufFree(MilDispChild);
   MbufFree(MilImageChild);
   }

// Display a character.
void UpdateDispChar(MIL_ID MilDispImage,
                    MIL_ID MilPredImage,
                    MIL_ID MilOverlayImg,
                    MIL_STRING ReadChar,
                    MIL_DOUBLE BestScore,
                    MIL_INT StringLength,
                    MIL_INT id)
   {
   MIL_INT PredSizeX = MbufInquire(MilPredImage, M_SIZE_X, M_NULL);
   MIL_INT PredSizeY = MbufInquire(MilPredImage, M_SIZE_Y, M_NULL);

   // Margin between two windows. 
   MIL_INT Margin = 5;

   // Window Length of each character.
   MIL_INT cwl = (PredSizeX + (2 * Margin));

   // Required Space up to that character. 
   MIL_INT ReqSpace = StringLength * cwl;

   // Extra Space available.
   MIL_INT ExtraSpace = DISP_WINDOW_SIZE_X - DISP_BAR_SIZE_X - ReqSpace;

   // Start pixel.
   MIL_INT sx = DISP_BAR_SIZE_X + (ExtraSpace / 2) + Margin + (id * (cwl));
   MIL_INT sy = 130;

   // End pixel. 
   MIL_INT ex = sx + PredSizeX;
   MIL_INT ey = sy + PredSizeY;

   // Draw box around character.
   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   MgraRect(M_DEFAULT, MilDispImage, sx - 1, sy - 1, ex, ey);

   // Copy the image into the box.
   MbufCopyColor2d(MilPredImage, MilDispImage, M_ALL_BANDS, 0, 0, M_ALL_BANDS, sx, sy, PredSizeX, PredSizeY);

   // Print the classification result.
   MIL_TEXT_CHAR Char_text[256],
      Accuracy_text[256];

   MosSprintf(Char_text, BUFF_SIZE, MIL_TEXT("   %s"), ReadChar.c_str());
   MosSprintf(Accuracy_text, BUFF_SIZE, MIL_TEXT(" %.0lf%%"), BestScore);

   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);

   MgraText(M_DEFAULT, MilOverlayImg, sx, START_Y_CHAR, Char_text);
   MgraText(M_DEFAULT, MilOverlayImg, sx, START_Y_SCORE, Accuracy_text);

   return;
   }

// Returns median of a vector.
MIL_DOUBLE Median(vector<MIL_INT> scores)
   {
   MIL_INT size = scores.size();

   // Empty vector.
   if(size == 0)
      return 0.0;

   sort(scores.begin(), scores.end());

   // Even size.
   if(size % 2 == 0)
      return MIL_DOUBLE(scores[size / 2 - 1] + scores[size / 2]) / 2;

   // Odd size.
   return MIL_DOUBLE(scores[size / 2]);
   }

// Build the optimized bounding box of a character. 
vector<charBox> BuildBboxDict(MIL_ID img,
                              vector<MIL_INT> &MilBoxMinX,
                              vector<MIL_INT> &MilBoxMaxX,
                              vector<MIL_INT> &MilBoxMinY,
                              vector<MIL_INT> &MilBoxMaxY)
   {
   vector<charBox> charDict;
   vector<MIL_INT> wBox, hBox;
   MIL_INT max_margin_pix;

   // Number of extracted characters. 
   MIL_INT boxes = MilBoxMinX.size();

   MIL_INT imgSy = MbufInquire(img, M_SIZE_Y, M_NULL);

   MIL_INT bboxBaseline = 0;
   MIL_INT bboxTopline = imgSy;

   for(MIL_INT ibox = 0; ibox < boxes; ibox++)
      {
      // Find the base-line and top-line of the string. 
      bboxBaseline = max(bboxBaseline, MilBoxMaxY[ibox]);
      bboxTopline = min(bboxTopline, MilBoxMinY[ibox]);

      // Calculate the width and height of each character.
      hBox.push_back(MilBoxMaxY[ibox] - MilBoxMinY[ibox]);
      wBox.push_back(MilBoxMaxX[ibox] - MilBoxMinX[ibox]);
      }

   // Get heights median.
   MIL_DOUBLE h_med = Median(hBox);

   //
   for(MIL_INT ibox = 0; ibox < boxes; ibox++)
      {
      MIL_BOOL BREAK_IT = false;

      MIL_INT w_margin_pix = max(2, int(0.5 + (wBox[ibox]) * NORMALIZE_CHAR_WINDOW_MARGIN / 200.0));
      MIL_INT h_margin_pix = max(2, int(0.5 + (hBox[ibox]) * NORMALIZE_CHAR_WINDOW_MARGIN / 200.0));

      max_margin_pix = max(w_margin_pix, h_margin_pix);

      // Detect punctuations.
      if(hBox[ibox] < 0.5 * h_med)
         {
         MilBoxMaxY[ibox] = bboxBaseline;
         MilBoxMinY[ibox] = bboxTopline;
         }
      else
         {
         MilBoxMaxY[ibox] = min(MilBoxMaxY[ibox] + max_margin_pix, imgSy);
         MilBoxMinY[ibox] = max(MilBoxMinY[ibox] - max_margin_pix, MIL_INT(0));
         }
      }

   vector<MIL_INT> box_margin;

   for(MIL_INT ibox = 0; ibox < boxes; ibox++)
      {
      MIL_INT lmargin = max_margin_pix;
      MIL_INT rmargin = max_margin_pix;

      // Limit the margin to the space between characters. 
      if(ibox > 0)
         lmargin = min(max_margin_pix, MilBoxMinX[ibox] - MilBoxMaxX[ibox - 1]);
      if(ibox < boxes - 1)
         rmargin = min(max_margin_pix, MilBoxMinX[ibox + 1] - MilBoxMaxX[ibox]);

      MIL_INT cur_margin = min(lmargin, rmargin);
      box_margin.push_back(cur_margin);
      }

   // Add margin.
   for(MIL_INT ibox = 0; ibox < boxes; ibox++)
      {
      MilBoxMinX[ibox] -= box_margin[ibox];
      MilBoxMaxX[ibox] += box_margin[ibox];
      }

   // Convert box information to charBox format. 
   for(MIL_INT ibox = 0; ibox < boxes; ibox++)
      {
      charBox new_char;
      new_char.BBoxStx = MilBoxMinX[ibox];
      new_char.BBoxSty = MilBoxMinY[ibox];
      new_char.BBoxEnx = MilBoxMaxX[ibox];
      new_char.BBoxEny = MilBoxMaxY[ibox];
      charDict.push_back(new_char);
      }

   return charDict;
   }

// Get Bounding Box of the character. 
void GetBbox(MIL_ID MilSystem,
             MIL_ID MilImage,
             MIL_ID MilMimCtx,
             MIL_ID MilBlobCtx,
             MIL_ID MilBlobRes,
             MIL_ID MilAdaptiveCtx,
             vector<MIL_INT> &MilBoxMinX,
             vector<MIL_INT> &MilBoxMaxX,
             vector<MIL_INT> &MilBoxMinY,
             vector<MIL_INT> &MilBoxMaxY)
   {
   MIL_ID MilPrepImg,
      MilBinarized,
      MilThreshImg;

   MIL_INT MilBlobCount;

   MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_IMAGE + M_PROC, M_DEFAULT, &MilPrepImg);
   MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_IMAGE + M_PROC, M_DEFAULT, &MilBinarized);
   MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_IMAGE + M_PROC, M_DEFAULT, &MilThreshImg);

   // Preprocess the input image.
   MimConvolve(MilImage, MilPrepImg, MilMimCtx);

   MimRemap(M_DEFAULT, MilPrepImg, MilPrepImg, M_FIT_SRC_DATA);
   MimOpen(MilPrepImg, MilPrepImg, 1, M_GRAYSCALE);

   // Adaptive binarization to find the characters. 
   MimBinarizeAdaptive(MilAdaptiveCtx, MilPrepImg, M_NULL, M_NULL, MilBinarized, MilThreshImg, M_DEFAULT);

   // Use blob to get the bounding box. 
   MblobCalculate(MilBlobCtx, MilBinarized, M_NULL, MilBlobRes);
   MblobSelect(MilBlobRes, M_EXCLUDE, M_AREA, M_LESS, 80, M_NULL);

   MblobGetResult(MilBlobRes, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &MilBlobCount);
   MilBoxMinX.resize(MilBlobCount);
   MilBoxMaxX.resize(MilBlobCount);
   MilBoxMinY.resize(MilBlobCount);
   MilBoxMaxY.resize(MilBlobCount);

   MblobGetResult(MilBlobRes, M_INCLUDED_BLOBS, M_BOX_X_MIN, MilBoxMinX);
   MblobGetResult(MilBlobRes, M_INCLUDED_BLOBS, M_BOX_X_MAX, MilBoxMaxX);
   MblobGetResult(MilBlobRes, M_INCLUDED_BLOBS, M_BOX_Y_MIN, MilBoxMinY);
   MblobGetResult(MilBlobRes, M_INCLUDED_BLOBS, M_BOX_Y_MAX, MilBoxMaxY);


   // Free objects.
   MbufFree(MilPrepImg);
   MbufFree(MilBinarized);
   MbufFree(MilThreshImg);
   }

void Saturate(MIL_INT &sx,
              MIL_INT &sy,
              MIL_INT &ex,
              MIL_INT &ey,
              MIL_INT w,
              MIL_INT h)
   {
   sx = min(w, max(sx, MIL_INT(0)));
   sy = min(h, max(sy, MIL_INT(0)));
   ex = min(w, max(ex, MIL_INT(0)));
   ey = min(h, max(ey, MIL_INT(0)));
   return;
   }

void GetNormalizedChar(MIL_ID MilSystem,
                       MIL_ID MilClassCtx,
                       MIL_ID MilClassRes,
                       MIL_ID MilImage,
                       MIL_ID MilDestImage,
                       MIL_INT StartX,
                       MIL_INT StartY,
                       MIL_INT EndX,
                       MIL_INT EndY)
   {
   MIL_ID MilImgChild,
      MilDestChild;

   // Image Size.
   MIL_INT ImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Box Size.
   MIL_INT SizeX = EndX - StartX;
   MIL_INT SizeY = EndY - StartY;

   // Conversion ratio.
   MIL_DOUBLE r = min((double(NORMALIZE_CHAR_WINDOW_SIZE) / double(SizeX)), (double(NORMALIZE_CHAR_WINDOW_SIZE) / double(SizeY)));

   // Make sure the box resides inside the image. 
   Saturate(StartX, StartY, EndX, EndY, ImageSizeX, ImageSizeY);

   // Crop the character from main image.
   MbufChild2d(MilImage, StartX, StartY, SizeX, SizeY, &MilImgChild);

   // Clear destination buffers with padding color.
   MbufClear(MilDestImage, M_COLOR_WHITE);

   // Copy destination.
   MbufChild2d(MilDestImage,
               MIL_INT((NORMALIZE_CHAR_WINDOW_SIZE - SizeX * r) / 2) + NORMALIZE_CHAR_WINDOW_MARGIN,
               MIL_INT((NORMALIZE_CHAR_WINDOW_SIZE - SizeY * r) / 2) + NORMALIZE_CHAR_WINDOW_MARGIN,
               MIL_INT(SizeX * r),
               MIL_INT(SizeY * r),
               &MilDestChild);

   // Resize and copy the character into destination image. 
   MimResize(MilImgChild, MilDestChild, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BICUBIC);

   // Free allocated buffers. 
   MbufFree(MilImgChild);
   MbufFree(MilDestChild);

   return;
   }
