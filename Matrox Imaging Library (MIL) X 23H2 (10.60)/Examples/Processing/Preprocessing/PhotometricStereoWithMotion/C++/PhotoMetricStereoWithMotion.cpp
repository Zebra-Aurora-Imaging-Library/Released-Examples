//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: PhotoMetricStereoWithMotion.cpp
// 
// Synopsis:  This example demonstrates the use of photometric stereo technology
//            for enhancing the embossed characters of a moving object.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#include <vector>

#define IMAGE_DIR M_IMAGE_PATH MIL_TEXT("PhotometricStereoWithMotion/")

// Lighting information.
const MIL_INT    NB_IMG                = 4;
const MIL_DOUBLE ZENITH_ANGLE [NB_IMG] = { 60.0,   60.0,   60.0,  60.0 }; // in degrees
const MIL_DOUBLE AZIMUTH_ANGLE[NB_IMG] = {  0.0,   90.0,  180.0, 270.0 }; // in degrees
const MIL_DOUBLE LIGHT_NORM   [NB_IMG] = {  1.0,    1.0,    1.0,   1.0 };

static MIL_CONST_TEXT_PTR ILLUSTRATION_PATH = IMAGE_DIR MIL_TEXT("/LightOrientations.png");
static const MIL_INT      ILLUSTRATION_DISPLAY_OFFSET_X = 660;
static const MIL_INT      FIND_MODEL_MARGIN = 250;
static const MIL_DOUBLE   SHAPE_SMOOTHNESS_VALUE = 70.0;

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("PhotometricStereoWithMotion\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates the use of photometric stereo technology\n")
             MIL_TEXT("for enhancing the embossed characters on a moving object.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display, image processing,\n")
             MIL_TEXT("registration, pattern matching, system.                      \n\n"));

   MosPrintf(MIL_TEXT("Press any key to continue.\n\n"));
   MosGetch();
   }

//////////////////////////////////////////////////////////////////////////////////////////
int MosMain()
   {
   // Allocate MIL objects.
   MIL_ID MilApplicationId = MappAlloc (M_DEFAULT, M_NULL);
   MIL_ID MilSystemId      = MsysAlloc (M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplayId     = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(MilDisplayId, M_VIEW_MODE, M_AUTO_SCALE);
   MIL_ID MilGraList = MgraAllocList(MilSystemId, M_DEFAULT, M_NULL);
   MdispControl(MilDisplayId, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   // Show illustration of light orientations.
   MIL_ID IllustrationDispId  = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_PATH, MilSystemId, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Light orientations"));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_DISPLAY_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);
   
   PrintHeader();

   // Restore images.
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MIL_ID StartImage, EndImage;
   MIL_TEXT_CHAR ImgName[256];
   MosSprintf(ImgName, 256, MIL_TEXT("%sFrameStart.mim"), IMAGE_DIR);
   MbufRestore(ImgName, MilSystemId, &StartImage);
   MdispSelect(MilDisplayId, StartImage);
   MosPrintf(MIL_TEXT("The start image, taken with all lights on, is displayed.\n")
             MIL_TEXT("Press any key to continue.\n\n"));
   MosGetch();

   std::vector<MIL_ID> SourceImageVect(NB_IMG, M_NULL);
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MosSprintf(ImgName, 256, MIL_TEXT("%sFrame%d.mim"), IMAGE_DIR, i);
      MbufRestore(ImgName, MilSystemId, &SourceImageVect[i]);
      MdispSelect(MilDisplayId, SourceImageVect[i]);
      MosPrintf(MIL_TEXT("The next image, with directional illumination, is displayed.\n")
                MIL_TEXT("Press any key to continue.\n\n")); 
      MosGetch();
      }

   MosSprintf(ImgName, 256, MIL_TEXT("%sFrameEnd.mim"), IMAGE_DIR);
   MbufRestore(ImgName, MilSystemId, &EndImage);
   MdispSelect(MilDisplayId, EndImage);
   MosPrintf(MIL_TEXT("The last image, taken with all lights on, is displayed.\n\n"));

   // Retrieve source image sizes.
   MIL_INT SizeX, SizeY;
   MbufInquire(StartImage, M_SIZE_X, &SizeX);
   MbufInquire(StartImage, M_SIZE_Y, &SizeY);

   // Motion estimation using a pattern matching tool using the start and end images taken using all lights on.
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////
   MIL_INT    NumResults;
   MIL_DOUBLE MotionX = 0.0;
   MIL_DOUBLE MotionY = 0.0;

   // Allocate a pattern matching tool and define a model from the start image.
   MIL_ID PatContextId = MpatAlloc(MilSystemId, M_NORMALIZED, M_DEFAULT, M_NULL);
   MIL_ID PatResultId = MpatAllocResult(MilSystemId, M_DEFAULT, M_NULL);
   MpatDefine(PatContextId, M_REGULAR_MODEL, StartImage, FIND_MODEL_MARGIN, FIND_MODEL_MARGIN, SizeX - 2 * FIND_MODEL_MARGIN, SizeY - 2 * FIND_MODEL_MARGIN, M_DEFAULT);
   MpatPreprocess(PatContextId, M_DEFAULT, M_NULL);
   
   // Find the model in the end image and calculate the motion per image.
   MpatFind(PatContextId, EndImage, PatResultId);
   MpatGetResult(PatResultId, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NumResults);

   std::vector<MIL_ID> TranslatedImageVect(NB_IMG, M_NULL);
   if (NumResults == 1L)
      {
      MIL_DOUBLE posx, posy;
      MpatGetResult(PatResultId, M_DEFAULT, M_POSITION_X, &posx);
      MpatGetResult(PatResultId, M_DEFAULT, M_POSITION_Y, &posy);
      MIL_DOUBLE ImageCenterX = 0.5 * (SizeX - 1);
      MIL_DOUBLE ImageCenterY = 0.5 * (SizeY - 1);
      MotionX = (posx - ImageCenterX);
      MotionY = (posy - ImageCenterY);

      MIL_INT TranslatedSizeX = (SizeX - MIL_INT(MotionX + 0.5));
      MIL_INT TranslatedSizeY = (SizeY - MIL_INT(MotionY + 0.5));

      // Copy the first image.
      TranslatedImageVect[0] = MbufClone(SourceImageVect[0], M_DEFAULT, TranslatedSizeX, TranslatedSizeY, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_NULL);

      // Motion compensation. Translate all directionnal lights images on the first one.
      for (MIL_INT i = 1; i < NB_IMG; ++i)
         {
         TranslatedImageVect[i] = 
            MbufClone(SourceImageVect[i], M_DEFAULT, TranslatedSizeX, TranslatedSizeY, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_NULL);

         MimTranslate(SourceImageVect[i], 
                      TranslatedImageVect[i], 
                      -i*MotionX / (MIL_DOUBLE)(NB_IMG + 1.0), 
                      -i*MotionY / (MIL_DOUBLE)(NB_IMG + 1.0), 
                      M_BILINEAR);
         }

      // Display motion vector.
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      MgraVectors(M_DEFAULT, MilGraList, 1, &ImageCenterX, &ImageCenterY, &MotionX, &MotionY, M_ABSOLUTE, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("The motion vector, in green, has been estimated using the first and last\nimages:\n")
                MIL_TEXT(" - X displacement: %f pixels\n")
                MIL_TEXT(" - Y displacement: %f pixels\n")
                MIL_TEXT("Press any key to continue.\n\n"), MotionX, MotionY);
      MosGetch();

      MgraClear(M_DEFAULT, MilGraList);

      // Perform the photometric stereo.
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////
      MIL_ID PsContextId = MregAlloc(MilSystemId, M_PHOTOMETRIC_STEREO, M_DEFAULT, M_NULL);
      MIL_ID PsResultId = MregAllocResult(MilSystemId, M_PHOTOMETRIC_STEREO_RESULT, M_NULL);

      // Setup the lighting configuration.
      MregControl(PsContextId, M_ALL, M_LIGHT_VECTOR_TYPE, M_SPHERICAL);
      for (MIL_INT i = 0; i < NB_IMG; ++i)
         {
         MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_1, ZENITH_ANGLE [i]);
         MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_2, AZIMUTH_ANGLE[i]);
         MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_3, LIGHT_NORM   [i]);
         }

      // Non uniform illumination requires image correction.
      MregControl(PsContextId, M_CONTEXT, M_NON_UNIFORMITY_CORRECTION, M_AUTO);
      // Object surface is not of constant albedo (printed characters).
      MregControl(PsContextId, M_CONTEXT, M_SHAPE_NORMALIZATION, M_ENABLE);
      // Increase the smoothness to well reconstruct thick embossed characters.
      MregControl(PsContextId, M_CONTEXT, M_SHAPE_SMOOTHNESS, SHAPE_SMOOTHNESS_VALUE);
      // Request the shape image to be calculated.
      MregControl(PsContextId, M_CONTEXT, M_LOCAL_SHAPE, M_ENABLE);

      MregCalculate(PsContextId, &TranslatedImageVect[0], PsResultId, NB_IMG, M_DEFAULT);

      MIL_ID PsResultImageId = MbufAlloc2d(MilSystemId, TranslatedSizeX, TranslatedSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_NULL);
      MregDraw(M_DEFAULT, PsResultId, PsResultImageId, M_DRAW_LOCAL_SHAPE_IMAGE, M_DEFAULT, M_DEFAULT);
      MdispSelect(MilDisplayId, PsResultImageId);
      MosPrintf(MIL_TEXT("Photometric stereo registration is applied to the images.\n")
                MIL_TEXT("The local shape image result is displayed.\n")
                MIL_TEXT("Press any key to continue.\n\n"));
      MosGetch();

      MIL_ID BinAdaptCtxId = MimAlloc(MilSystemId, M_BINARIZE_ADAPTIVE_CONTEXT, M_DEFAULT, &BinAdaptCtxId);
      MimControl(BinAdaptCtxId, M_THRESHOLD_MODE, M_NIBLACK);
      MimControl(BinAdaptCtxId, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);
      MIL_ID MilLinearFilterIIRContext = MimAlloc(MilSystemId, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, M_NULL);
      MimControl(MilLinearFilterIIRContext, M_FILTER_TYPE, M_SHEN);
      MimControl(MilLinearFilterIIRContext, M_FILTER_RESPONSE_TYPE, M_STEP);
      MimDifferential(MilLinearFilterIIRContext, PsResultImageId, M_NULL, M_NULL, M_NULL, PsResultImageId, M_NULL, M_DEFAULT, M_SHARPEN, M_DEFAULT);
      MimFree(MilLinearFilterIIRContext);
      MimBinarizeAdaptive(BinAdaptCtxId, PsResultImageId, M_NULL, M_NULL, PsResultImageId, M_NULL, M_DEFAULT);
      MimArith(PsResultImageId, M_NULL, PsResultImageId, M_NOT);
      MdispSelect(MilDisplayId, PsResultImageId);
      MosPrintf(MIL_TEXT("The shape image has been enhanced and\n")
                MIL_TEXT("segmented using adaptive binarization.\n")
                MIL_TEXT("Press any key to continue.\n\n"));
      MosGetch();

      MregDraw(M_DEFAULT, PsResultId, PsResultImageId, M_DRAW_ALBEDO_IMAGE, M_DEFAULT, M_DEFAULT);
      MdispSelect(MilDisplayId, PsResultImageId);
      MosPrintf(MIL_TEXT("The albedo image is displayed.\n")
                MIL_TEXT("Press any key to continue.\n\n"));
      MosGetch();

      // Free MIL objects.
      MimFree(BinAdaptCtxId);
      MregFree(PsContextId);
      MregFree(PsResultId);
      MbufFree(PsResultImageId);
      }
   else
      {
      MosPrintf(MIL_TEXT("Motion estimation failed.\n"));
      MosPrintf(MIL_TEXT("Press any key to end.\n"));
      MosGetch();
      }

   // Free MIL objects.
   MpatFree(PatContextId);
   MpatFree(PatResultId);

   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   MbufFree(StartImage);
   MbufFree(EndImage);
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MbufFree(TranslatedImageVect[i]);
      MbufFree(SourceImageVect[i]);
      }

   MgraFree(MilGraList);
   MdispFree(MilDisplayId);
   MsysFree (MilSystemId);
   MappFree (MilApplicationId);
   }
