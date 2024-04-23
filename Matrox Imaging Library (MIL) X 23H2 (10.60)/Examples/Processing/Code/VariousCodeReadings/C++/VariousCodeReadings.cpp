﻿/***************************************************************************************/
/* 
* File name: VariousCodeReadings.cpp  
*
* Synopsis:  This program contains examples of code reading operations for different 
*            types of codes under various conditions.
*            See the PrintHeader() function below for detailed description.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>


///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("VariousCodeReadings\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program reads different types of codes,\n")
             MIL_TEXT("under various conditions.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, calibration, code.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//**************************************
// CODE FOREGROUND COLOR declarations
//**************************************

static MIL_CONST_TEXT_PTR CodeForegroundColorFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/BlackAndWhiteDatamatrix.mim");

void CodeForegroundColor(MIL_CONST_TEXT_PTR SrcFilename, 
                         MIL_ID MilSystem, 
                         MIL_ID MilDisplay);

//******************************
// CODE ROTATION declarations
//******************************

static MIL_CONST_TEXT_PTR CodeRotationFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/GS1Databar.mim");

void CodeRotation(MIL_CONST_TEXT_PTR SrcFilename, 
                  MIL_ID MilSystem, 
                  MIL_ID MilDisplay);


//******************************************
// LINEAR CODE SCANLINE SCORES declarations
//******************************************

static MIL_CONST_TEXT_PTR CodeScanLineScoresFileName = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/Code128_ScanScore.mim");

void LinearCodeScanLineScores(MIL_CONST_TEXT_PTR SrcFilename,
                              MIL_ID MilSystem,
                              MIL_ID MilDisplay);

//*********************************
// CODE DEFORMATION declarations
//*********************************

const MIL_INT NUMBER_GRID_ROWS    = 19;
const MIL_INT NUMBER_GRID_COLUMNS = 19;

static MIL_CONST_TEXT_PTR CalDeformationFilename  = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/CalibrationQRCode.mim");
static MIL_CONST_TEXT_PTR CodeDeformationFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/DeformedQRCode.mim");

void CodeDeformation(MIL_CONST_TEXT_PTR SrcFilename, 
                     MIL_CONST_TEXT_PTR GridFilename, 
                     MIL_INT RowNumber, 
                     MIL_INT ColumNumber, 
                     MIL_ID MilSystem, 
                     MIL_ID MilDisplay);

//*********************************
// CODE UNEVEN GRID declarations
//*********************************

const MIL_INT CodeUnevenGridNumber = 5;

static MIL_CONST_TEXT_PTR CodeUnevenGridFilename[CodeUnevenGridNumber] = 
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix1.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix2.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix3.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix4.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/UnevenGridDatamatrix5.mim")
   };

void CodeUnevenGrid(MIL_CONST_TEXT_PTR SrcFilename, 
                    MIL_ID MilSystem, 
                    MIL_ID MilDisplay);

//***********************************************
// CODE ASPECT RATIO AND SHEARING declarations
//***********************************************

static MIL_CONST_TEXT_PTR CodeAspectRatioAndShearingFilename = M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/SampleQRCode.mim");

void CodeAspectRatioAndShearing(MIL_CONST_TEXT_PTR SrcFilename, 
                                MIL_ID MilSystem, 
                                MIL_ID MilDisplay);

//****************************************
//* CODE FLIPPED DATAMATRIX declarations
//****************************************

const MIL_INT CodeFlippedDatamatrixNumber = 2;

static MIL_CONST_TEXT_PTR CodeFlippedDatamatrixFilename[CodeFlippedDatamatrixNumber] = 
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/FlippedDatamatrix1.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/FlippedDatamatrix2.mim")
   };

void CodeFlippedDatamatrix(MIL_CONST_TEXT_PTR SrcFilename, 
                           MIL_ID MilSystem, 
                           MIL_ID MilDisplay);


//*****************************************************
//* CODE EXTENDED RECTANGULAR DATAMATRIX declarations
//*****************************************************

const MIL_INT CodeDMRENumber = 2;

static MIL_CONST_TEXT_PTR CodeDMREFilename[CodeDMRENumber] =
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/DMRE1.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/DMRE2.mim")
   };

void CodeExtendedRectangularDatamatrix(MIL_CONST_TEXT_PTR SrcFilename,
                                       MIL_ID MilSystem,
                                       MIL_ID MilDisplay);

//****************************************
//* CODE Character Set ECIs declarations
//****************************************

const MIL_INT CodeECINumber = 2;

static MIL_CONST_TEXT_PTR CodeECIFilename[CodeECINumber] =
   {
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/ECIQRCode.mim"),
   M_IMAGE_PATH MIL_TEXT("VariousCodeReadings/ECIAztecCode.mim")
   };

void CodeECI(MIL_CONST_TEXT_PTR SrcFilename,
             MIL_INT CodeType,
             MIL_ID MilSystem,
             MIL_ID MilDisplay);


//************************************
// Utility sub-functions declarations
//************************************

void AllocDisplayImage(MIL_ID MilSystem, 
                       MIL_ID MilSrcImage, 
                       MIL_ID MilDisplay, 
                       MIL_ID& MilDispProcImage, 
                       MIL_ID& MilOverlayImage);

void RetrieveAndDrawCode(MIL_ID     MilCodeResult, 
                         MIL_ID     MilDisplay, 
                         MIL_ID     MilOverlayImage, 
                         MIL_DOUBLE DrawPosX,
                         MIL_DOUBLE DrawPosY,
                         bool       DrawBox, 
                         bool       DrawCode);

//*****************************************************************************
// Main
//*****************************************************************************
int MosMain(void)
   {
   MIL_INT ii;

   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay     = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);

   // Print Header.
   PrintHeader();

   //************************
   // CODE FOREGROUND COLOR    
   //************************
   CodeForegroundColor(CodeForegroundColorFilename, MilSystem, MilDisplay);

   //*****************
   // CODE ROTATION 
   //*****************
   CodeRotation(CodeRotationFilename, MilSystem, MilDisplay);

   //****************************
   // LINEAR CODE SCANLINE SCORES
   //****************************
   LinearCodeScanLineScores(CodeScanLineScoresFileName, MilSystem, MilDisplay);

   //*******************
   // CODE DEFORMATION 
   //*******************
   CodeDeformation(CodeDeformationFilename, CalDeformationFilename, 19, 19, MilSystem, MilDisplay);

   //****************************
   // CODE UNVEN GRID DISTORTION
   //****************************
   for(ii=0; ii<CodeUnevenGridNumber; ii++)
      CodeUnevenGrid(CodeUnevenGridFilename[ii], MilSystem, MilDisplay);

   //*********************************
   // CODE ASPECT RATIO AND SHEARING
   //*********************************
   CodeAspectRatioAndShearing(CodeAspectRatioAndShearingFilename, MilSystem, MilDisplay);

   //*************************
   // CODE FLIPPED DATAMTRIX
   //*************************
   for(ii=0; ii<CodeFlippedDatamatrixNumber; ii++)
      CodeFlippedDatamatrix(CodeFlippedDatamatrixFilename[ii], MilSystem, MilDisplay);

   //**************************************
   // CODE EXTENDED RECTANGULAR DATAMTRIX
   //**************************************
   for(ii = 0; ii < CodeDMRENumber; ii++)
      CodeExtendedRectangularDatamatrix(CodeDMREFilename[ii], MilSystem, MilDisplay);

   //**************************
   // CODE Character Set ECIs
   //**************************
   MgraFont(M_DEFAULT, MIL_FONT_NAME(M_FONT_DEFAULT_TTF));

   for(ii = 0; ii < CodeECINumber; ii++)
      {
      CodeECI(CodeECIFilename[ii], (ii == 0 ) ? M_QRCODE : M_AZTEC, MilSystem, MilDisplay);
      }

   // Free other allocations.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//************************
// CODE FOREGROUND COLOR  
//************************
void CodeForegroundColor(MIL_CONST_TEXT_PTR SrcFilename, 
                         MIL_ID MilSystem, 
                         MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING BLACK AND WHITE FOREGROUND COLOR CODES]\n\n")

             MIL_TEXT("In this example,two codes of opposite color are read\n")
             MIL_TEXT("by setting the foreground color property.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Enable the presearch mode.
   McodeControl(MilCodeModel, M_USE_PRESEARCH, M_STAT_BASE);

   // Set the foreground color to black.
   McodeControl(MilCodeModel, M_FOREGROUND_VALUE, M_FOREGROUND_BLACK);
   
   // Read the code and display the result.
   McodeRead(MilCodeContext, MilSrcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.25*SizeX, 10, true, false);

   // Set the foreground color to white.
   McodeControl(MilCodeModel, M_FOREGROUND_VALUE, M_FOREGROUND_WHITE);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilSrcImage, MilCodeResult);

   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.75*SizeX, 10, true, false);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   }

//*****************
// CODE ROTATION 
//*****************
void CodeRotation(MIL_CONST_TEXT_PTR SrcFilename, 
                  MIL_ID MilSystem, 
                  MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING A ROTATED CODE]\n\n")

             MIL_TEXT("In this example, a linear code is read at any angle\n")
             MIL_TEXT("by increasing the settings of the search angle range.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_GS1_DATABAR, M_NULL, M_DEFAULT, M_NULL);

   // Set search angle range.
   McodeControl(MilCodeModel, M_SEARCH_ANGLE_DELTA_NEG, 180);
   McodeControl(MilCodeModel, M_SEARCH_ANGLE_DELTA_POS, 180);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Rotate the image, then read and display the result.
   for(MIL_INT ii=5; ii<=360; ii+=5)
      {
      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear overlay.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Rotate the image.
      MimRotate(MilSrcImage, MilDispProcImage, (MIL_DOUBLE)(ii), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_BILINEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//****************************
// LINEAR CODE SCANLINE SCORES
//****************************
void LinearCodeScanLineScores(MIL_CONST_TEXT_PTR SrcFilename,
                              MIL_ID MilSystem,
                              MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING A LINEAR CODE AND DISPLAYING SCANLINES]\n\n")

             MIL_TEXT("In this example, a linear code is read. We then\n")
             MIL_TEXT("display the ScanLines that were decoded from it,\n"));
             MIL_TEXT("along with their scores.\n\n");
             
   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage;        // Display and destination buffer.
   MIL_ID MilOverlayImage;         // Overlay buffer.

   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_CODE128, M_NULL, M_DEFAULT, M_NULL);

   //Set some control.
   McodeControl(MilCodeContext, M_POSITION_ACCURACY, M_HIGH);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, false);

   //Display Read Score.
   MIL_DOUBLE ReadScore = 0;
   McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SCORE, &ReadScore);

   const MIL_INT TEXT_SIZE = 256;
   MIL_TEXT_CHAR OutputText[TEXT_SIZE];
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Read Score: %.2f"), ReadScore);
   MosPrintf(MIL_TEXT("Code 128 decoded with a Read Score of %.2f\n"), ReadScore);

   // Draw read string.
   MgraColor(M_DEFAULT, M_COLOR_CYAN);
   MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   MgraText(M_DEFAULT, MilOverlayImage, 0.5*SizeX, 25, OutputText);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Draw Decoded Scan Lines.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_DECODED_SCANS, 0, M_ALL, M_DEFAULT);
   //Retrieve decoded scan scores.
   std::vector<MIL_DOUBLE> DecodedScanScores;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_SCORE, DecodedScanScores);
   //Retrieve decoded scan Start positions.
   std::vector<MIL_INT> DecodedScansStartX;
   std::vector<MIL_INT> DecodedScansStartY;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_START_X, DecodedScansStartX);
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_START_Y, DecodedScansStartY);
   //Retrieve decoded scan End positions.
   std::vector<MIL_INT> DecodedScansEndX;
   std::vector<MIL_INT> DecodedScansEndY;
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_END_X, DecodedScansEndX);
   McodeGetResult(MilCodeResult, 0, M_ALL, M_DECODED_SCANS_END_Y, DecodedScansEndY);


   //Display ScanLine Index and scores.
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraBackColor(M_DEFAULT, M_COLOR_WHITE);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_VERTICAL, M_CENTER);

   //Display column titles.
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("ScanLine"));
   MgraText(M_DEFAULT, MilOverlayImage, 10, 70, OutputText);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Index"));
   MgraText(M_DEFAULT, MilOverlayImage, 10, 85, OutputText);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("ScanLine"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX - 10, 70, OutputText);
   MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("Scores"));
   MgraText(M_DEFAULT, MilOverlayImage, SizeX - 10, 85, OutputText);

   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);

   MosPrintf(MIL_TEXT("%i ScanLines were decoded, here's their scores:\n"), DecodedScanScores.size());

   for(MIL_UINT i = 0; i < DecodedScanScores.size(); i++)
      {
      //Display ScanLine Index.
      MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("%i"), i);
      MgraText(M_DEFAULT, MilOverlayImage, DecodedScansStartX[i]-10, DecodedScansStartY[i], OutputText);

      //Display ScanLine Score.
      MosSprintf(OutputText, TEXT_SIZE, MIL_TEXT("%.2f"), DecodedScanScores[i]);
      MgraText(M_DEFAULT, MilOverlayImage, DecodedScansEndX[i] + 10, DecodedScansEndY[i], OutputText);

      MosPrintf(MIL_TEXT("ScanLine[%i] Score = %.2f\n"), i, DecodedScanScores[i]);
      }
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   }

//*******************
// CODE DEFORMATION 
//*******************
void CodeDeformation(MIL_CONST_TEXT_PTR SrcFilename, 
                     MIL_CONST_TEXT_PTR GridFilename, 
                     MIL_INT RowNumber, 
                     MIL_INT ColumNumber, 
                     MIL_ID MilSystem, 
                     MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[RECTIFYING AND READING A DISTORTED CODE]\n\n")

             MIL_TEXT("In this example, a distorted code printed on a given non planar surface\n")
             MIL_TEXT("is read by calibrating and correcting the image of the printing surface.\n\n"));

   // Restore the grid image.
   MIL_ID MilSrcImage = MbufRestore(GridFilename, MilSystem, M_NULL);

   // Allocate a calibration.
   MIL_ID MilCalContext = McalAlloc(MilSystem, M_LINEAR_INTERPOLATION, M_DEFAULT, M_NULL);

   // Calibrate from the grid image.
   McalGrid(MilCalContext, MilSrcImage, 0, 0, 0, NUMBER_GRID_ROWS, NUMBER_GRID_COLUMNS, 1, 1, M_DEFAULT, M_CHESSBOARD_GRID);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   // Display the calibration result.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   McalDraw(M_DEFAULT, MilSrcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);
   
   MosPrintf(MIL_TEXT("The image of the surface is calibrated using a chessboard grid.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the calibration image and the display image.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);

   // Restore the image.
   MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Associate the calibration.
   McalAssociate(MilCalContext, MilSrcImage, M_DEFAULT);

   // Allocate a display image.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   // Retrieve image info.
   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   // Display the calibration result.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   McalDraw(M_DEFAULT, MilSrcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The image of the distorted code is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Clear the overlay image.
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   // Transform the image.
   McalTransformImage(MilSrcImage, MilDispProcImage, MilCalContext, M_BILINEAR, M_DEFAULT, M_WARP_IMAGE + M_CLIP);

   // Display the calibration result.
   McalDraw(M_DEFAULT, MilDispProcImage, MilOverlayImage, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The transformed image of the code is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_QRCODE, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 0.5*SizeY, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   McalFree(MilCalContext);
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//****************************
// CODE UNVEN GRID DISTORTION
//****************************
void CodeUnevenGrid(MIL_CONST_TEXT_PTR SrcFilename, 
                    MIL_ID MilSystem, 
                    MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING UNEVEN GRID STEP DISTORTED DATAMATRIX]\n\n")

             MIL_TEXT("In this example, an uneven grid step distorted datamatrix is\n")
             MIL_TEXT("read by enabling the reading with distortion capability.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Set the foreground color to white.
   McodeControl(MilCodeModel, M_FOREGROUND_VALUE, M_FOREGROUND_WHITE);
   
   // Set the presearch to stat base.
   McodeControl(MilCodeModel, M_USE_PRESEARCH, M_STAT_BASE);

   // Set the decode algorithm to code deformed.
   McodeControl(MilCodeModel, M_DECODE_ALGORITHM, M_CODE_DEFORMED);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//*********************************
// CODE ASPECT RATIO AND SHEARING
//*********************************
void CodeAspectRatioAndShearing(MIL_CONST_TEXT_PTR SrcFilename, 
                                MIL_ID MilSystem, 
                                MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING Qrcode DISTORTED BY ASPECT RATIO AND SHEARING]\n\n")

             MIL_TEXT("In this example, a QrCode is read even if it has an aspect ratio\n")
             MIL_TEXT("different than 1 or if it has shearing.\n\n"));

   MIL_INT ii;

   const MIL_DOUBLE StepValue  = 0.01;
   const MIL_INT    Iterations = 10;

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_QRCODE, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a warp matrix and initialize it as the identity matrix
   MIL_ID MilWarpMatrix = MbufAlloc2d(MilSystem, 3, 3, 32+M_FLOAT, M_ARRAY, M_NULL);
   MgenWarpParameter(M_NULL, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, 0, 0);

   for(ii=0; ii<Iterations; ii++)
      {
      // Increase aspect ratio.
      MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_SCALE, 1.0, 1.0+StepValue);

      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear the overlay image.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Apply the transformation.
      MimWarp(MilSrcImage, MilDispProcImage, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR+M_OVERSCAN_CLEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

      // Enable display update.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   for(ii=0; ii<Iterations; ii++)
      {
      // Increase the shearing in X.
      MgenWarpParameter(MilWarpMatrix, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_SHEAR_X, StepValue, M_DEFAULT);
 
      // Disable display update.
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      // Clear the overlay image.
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      // Apply the transformation.
      MimWarp(MilSrcImage, MilDispProcImage, MilWarpMatrix, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR+M_OVERSCAN_CLEAR);

      // Read the code and display the result.
      McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
      RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

      // Enable display update.
      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilWarpMatrix);
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };

//*************************
// CODE FLIPPED DATAMTRIX
//*************************
void CodeFlippedDatamatrix(MIL_CONST_TEXT_PTR SrcFilename, 
                           MIL_ID MilSystem, 
                           MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING FLIPPED DATAMATRIX]\n\n")

             MIL_TEXT("In this example, a flipped datamatrix is\n")
             MIL_TEXT("read by enabling the flip capability.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Set the presearch to stat base.
   McodeControl(MilCodeModel, M_USE_PRESEARCH, M_STAT_BASE);

   // Set decode algorithm to code deformed.
   McodeControl(MilCodeModel, M_DECODE_ALGORITHM, M_CODE_DEFORMED);

   // Set the code flip to any.
   McodeControl(MilCodeModel, M_CODE_FLIP, M_ANY);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };


//**************************************
// CODE EXTENDED RECTANGULAR DATAMTRIX
//**************************************
void CodeExtendedRectangularDatamatrix(MIL_CONST_TEXT_PTR SrcFilename,
                                       MIL_ID MilSystem,
                                       MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING EXTENDED RECTANGULAR DATAMATRIX]\n\n")

             MIL_TEXT("In this example, an extended rectangular datamatrix is\n")
             MIL_TEXT("read automatically using a M_DATAMATRIX code model.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };


void CodeECI(MIL_CONST_TEXT_PTR SrcFilename,
             MIL_INT CodeType,
             MIL_ID MilSystem,
             MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[READING the encodable character set, Extended Channel Interpretation (ECIs)]\n\n")

             MIL_TEXT("In this example, a bar code encoded with the ECI character set is read \n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Retrieve image info.
   MIL_INT SizeX;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);

   // Allocate a code context.
   MIL_ID MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);

   // Set speed to M_LOW.
   McodeControl(MilCodeContext, M_SPEED, M_LOW);

   // Allocate a code result.
   MIL_ID MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_NULL);

   // Add a code model.
   MIL_ID MilCodeModel = McodeModel(MilCodeContext, M_ADD, CodeType, M_NULL, M_DEFAULT, M_NULL);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, true, true);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Enable the raw data string format.
   McodeControl(MilCodeResult, M_STRING_FORMAT, M_RAW_DATA);
   
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   // Read the code and display the result.
   McodeRead(MilCodeContext, MilDispProcImage, MilCodeResult);
   RetrieveAndDrawCode(MilCodeResult, MilDisplay, MilOverlayImage, 0.5*SizeX, 10, false, false);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   McodeFree(MilCodeContext);
   McodeFree(MilCodeResult);
   };


//************************************
// Utility sub-functions definitions
//************************************

void RetrieveAndDrawCode(MIL_ID     MilCodeResult, 
                         MIL_ID     MilDisplay, 
                         MIL_ID     MilOverlayImage, 
                         MIL_DOUBLE DrawPosX,
                         MIL_DOUBLE DrawPosY,
                         bool       DrawBox, 
                         bool       DrawCode)
   {
   const MIL_INT DispOffsetY = 30;

   // Get decoding status.
   MIL_INT ReadStatus;
   McodeGetResult(MilCodeResult, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &ReadStatus);

   // Check if the decode operation was successful.
   if (ReadStatus == M_STATUS_READ_OK)
      {
      // Get decoded string.
      MIL_INT ResultStringSize;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING + M_STRING_SIZE+M_TYPE_MIL_INT, &ResultStringSize);
      MIL_TEXT_CHAR* ResultString = new MIL_TEXT_CHAR[ResultStringSize];
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_STRING, ResultString);

      MIL_INT ECIFlag;
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_IS_ECI + M_TYPE_MIL_INT, &ECIFlag);

      if(ECIFlag == M_FALSE)
         {
         // Replace non printable characters with space.
         MIL_INT ii;
         for(ii = 0; ResultString[ii] != MIL_TEXT('\0'); ii++)
            {
            if((ResultString[ii] < MIL_TEXT('0')) || (ResultString[ii] > MIL_TEXT('z')))
               ResultString[ii] = MIL_TEXT(' ');
            }
         }

      // Add prefix to the string.
      MIL_CONST_TEXT_PTR PrefixString = MIL_TEXT("Read code: ");
      MIL_INT OutputStringSize = ResultStringSize + MosStrlen(PrefixString);
      MIL_TEXT_CHAR* OutputString = new MIL_TEXT_CHAR[OutputStringSize]; // Array of characters to draw.
      MosSprintf(OutputString, OutputStringSize, MIL_TEXT("%s%s"), PrefixString, ResultString);

      // Draw read string.
      MgraColor(M_DEFAULT, M_COLOR_CYAN); 
      MgraBackColor(M_DEFAULT, M_COLOR_GRAY);
      MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
      MgraText(M_DEFAULT, MilOverlayImage, DrawPosX, DrawPosY, OutputString);

      // Draw a box around the code.
      if(DrawBox)
         {
         MgraColor(M_DEFAULT, M_COLOR_GREEN); 
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_BOX, 0, M_GENERAL, M_DEFAULT);
         }

      if(DrawCode)
         {
         MgraColor(M_DEFAULT, M_COLOR_RED); 
         McodeDraw(M_DEFAULT, MilCodeResult, MilOverlayImage, M_DRAW_CODE, 0, M_GENERAL, M_DEFAULT);
         }

      // Retrieve basic results.
      MIL_DOUBLE PositionX, PositionY, SizeX, SizeY;
      McodeControl(MilCodeResult, M_RESULT_OUTPUT_UNITS, M_PIXEL);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_POSITION_X, &PositionY);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_X,     &SizeX);
      McodeGetResult(MilCodeResult, 0, M_GENERAL, M_SIZE_Y,     &SizeY);
      
      MosPrintf(MIL_TEXT("Reading was successful.\n\n"));
      MosPrintf(MIL_TEXT(" - %s\n"), OutputString);
      MosPrintf(MIL_TEXT(" - Position: (%.2f, %.2f)\n"), PositionX, PositionY);
      MosPrintf(MIL_TEXT(" - Dimensions: (%.2f x %.2f)\n\n"), SizeX, SizeY);
	  
	  delete [] ResultString;
	  delete [] OutputString;
      }
   else
      {
      MosPrintf(MIL_TEXT("Code read operation failed.\n\n"));
      }
   }

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
