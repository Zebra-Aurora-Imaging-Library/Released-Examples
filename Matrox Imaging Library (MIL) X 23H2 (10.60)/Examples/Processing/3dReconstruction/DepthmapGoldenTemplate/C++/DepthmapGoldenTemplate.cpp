//***************************************************************************************/
// 
// File name: DepthmapGoldenTemplate.cpp  
//
// Synopsis:  This program contains an example of 3D surface analysis to detect
//            extra of missing material compared to a refence 3d surface.
//            See the PrintHeader() function below for detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include "mil.h"
#include <iostream>
#include <algorithm>

//*****************************************************************************
// Print the header.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DepthmapGoldenTemplate\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to detect various defects of\n")
             MIL_TEXT("a 3D object compared to a perfect reference of the object.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, System, Display, Buffer,\n")
             MIL_TEXT("Graphics, Image processing, 3D Image Processing.\n\n"));
   }

// Source image paths.
#define REFERENCE_DEPTHMAP_PATH   M_IMAGE_PATH MIL_TEXT("DepthmapGoldenTemplate/Reference.mim")
#define TARGET_DEPTHMAP_PATH      M_IMAGE_PATH MIL_TEXT("DepthmapGoldenTemplate/Target.mim")

// Util definitions and functions.
class Color
   {

   public:
   MIL_INT RED;   // red component
   MIL_INT GREEN; // green component
   MIL_INT BLUE;  // blue component

      Color(MIL_INT Red, MIL_INT Green, MIL_INT Blue)
         {
         RED = Red;
         GREEN = Green;
         BLUE = Blue;
         }
   };

Color Red   (255, 0  , 0  ), 
      Yellow(255, 132, 9  ),
      Green (77 , 232, 0  ),
      Blue  (0  , 0  , 255),
      Cyan  (0  , 255, 255),
      White (255, 255, 255);

// Defects detection parameters.
#define MAX_INTENSITY         255
#define MID_INTENSITY         127
#define MIN_INTENSITY         0

#define MAX_DEPTH_VALUE       65535
#define MID_DEPTH_VALUE       32767
#define MIN_DEPTH_VALUE       0

#define DEFECT_THRESHOLD      10
#define ARITH_DIST_NN         10        

#define IMAGE_WIDTH           600
#define IMAGE_HEIGHT          960

#define WINDOW_WIDTH          420
#define WINDOW_HEIGHT         480

#define LEGEND_WIDTH          240
#define LEGEND_MARGIN_X       60
#define LEGEND_MARGIN_Y       100
                             
#define WINDOW_BAR_SIZE       30
#define WINDOW_GAP            15

#define RANGE_LOW             32500
#define RANGE_HIGH            33000

static MIL_INT Sx;
static MIL_INT Sy;

void PrintHeader();

void generateLUT(MIL_ID &MilLUT);
void generateHeightMap(MIL_ID MilSystem, MIL_ID MilReferenceModel, MIL_ID MilSceneTarget, MIL_ID &MilDefect, MIL_ID &MilDefectMask, MIL_ID &MilHeightMapImage);
void generateHeightMapLegend(MIL_ID MilSystem, MIL_ID LUT, MIL_ID MilLegendImage);
void generateLutColorWithInvalidDepth(MIL_ID MilLut, MIL_UINT8 InvalidDepthColor);

void generateValidityMap(MIL_ID MilSystem, MIL_ID MilDefect, MIL_ID &MilValidityImage, MIL_ID &MilDefectMask);
void generateValidityLegend(MIL_ID MilImage);

void Display(MIL_ID MilSystem, MIL_ID MilReferenceImage, MIL_ID MilTargetImage, MIL_ID  MilHeightMapImage, MIL_ID MilValidityImage, MIL_UINT SizeX, MIL_UINT SizeY);


// Main function
int MosMain()
   {
   MIL_ID MilApplication,     /* Application Identifier  */
          MilSystem,          /* System Identifier       */
          MilLut,             /* LUT buffer Identifier   */
          MilReferenceImage,  /* Image buffer Identifier */
          MilTargetImage,     /* Image buffer Identifier */
          MilHeightMapImage,  /* Image buffer Identifier */
          MilValidityImage,   /* Image buffer Identifier */
          MilReferenceModel,  /* Image buffer Identifier */
          MilSceneTarget,     /* Image buffer Identifier */
          MilDefect,          /* Image buffer Identifier */
          MilDefectMask;      /* Image buffer Identifier */

   PrintHeader();

   // Allocate MIL objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   Sx = MbufDiskInquire(REFERENCE_DEPTHMAP_PATH, M_SIZE_X, M_NULL);
   Sy = MbufDiskInquire(REFERENCE_DEPTHMAP_PATH, M_SIZE_Y, M_NULL);

   MbufAlloc2d(MilSystem, Sx, Sy, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MilReferenceModel);
   MbufAlloc2d(MilSystem, Sx, Sy, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MilSceneTarget);
   MbufAlloc2d(MilSystem, Sx, Sy, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MilDefect);
   MbufAlloc2d(MilSystem, Sx, Sy,  8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilDefectMask);

   MbufAllocColor(MilSystem, 3, Sx, Sy, 8 + M_UNSIGNED, M_IMAGE + M_PROC+ M_DISP, &MilReferenceImage);
   MbufAllocColor(MilSystem, 3, Sx, Sy, 8 + M_UNSIGNED, M_IMAGE + M_PROC+ M_DISP, &MilTargetImage);
   MbufAllocColor(MilSystem, 3, Sx, Sy, 8 + M_UNSIGNED, M_IMAGE + M_PROC+ M_DISP, &MilHeightMapImage);
   MbufAllocColor(MilSystem, 3, Sx, Sy, 8 + M_UNSIGNED, M_IMAGE + M_PROC+ M_DISP, &MilValidityImage);
   
   MbufAllocColor(MilSystem, 3, MAX_DEPTH_VALUE + 1, 1, 8 + M_UNSIGNED, M_LUT, &MilLut);

   // Importing the depthmaps.
   MbufImport(REFERENCE_DEPTHMAP_PATH, M_MIL_TIFF + M_WITH_CALIBRATION, M_LOAD, MilSystem, &MilReferenceModel);
   MbufImport(TARGET_DEPTHMAP_PATH, M_MIL_TIFF + M_WITH_CALIBRATION, M_LOAD, MilSystem, &MilSceneTarget);

   MosPrintf(MIL_TEXT("The reference depth map window shows the reference object without defects.\n")
             MIL_TEXT("The target depth map window shows the 3D scan of the object to be compared\n")
             MIL_TEXT("with the reference.\n")
             MIL_TEXT("The height map of differences window and the validity map window show the\n")
             MIL_TEXT("results of the robust arithmetic operations, done by 3dim, which are used\n")
             MIL_TEXT("to compare depth maps.\n\n")

             MIL_TEXT("The validity map lets you to distinguish between the following:\n")
             MIL_TEXT("   - Pass areas (green), where data is available in both depth maps and\n")
             MIL_TEXT("     there is no defect.\n\n")
             MIL_TEXT("   - Bump areas (red), where data is available in both depthmaps\n")
             MIL_TEXT("     but the elevation in the target is higher than reference.\n\n")
             MIL_TEXT("   - Underfill areas (blue), where data is available in both depth maps\n") 
             MIL_TEXT("     but the elevation in the target is lower than in the reference.\n\n")
             MIL_TEXT("   - Missing reference data areas (cyan),\n")
             MIL_TEXT("     where data is available in the target only.\n\n")
             MIL_TEXT("   - Missing target data areas (orange),\n")
             MIL_TEXT("     where data is available in the reference only.\n\n")
             MIL_TEXT("   - Missing data areas (white),\n")
             MIL_TEXT("     where data is unavailable in both the reference and the target.\n\n"));

   // Generate the pseudo-color Look Up Table.
   generateLutColorWithInvalidDepth(MilLut, MAX_INTENSITY);

   // Using the Look Up table to map the colors for the reference depthmap.
   MimLutMap(MilReferenceModel, MilReferenceImage, MilLut);

   // Using the Look Up table to map the colors for the target depthmap.
   MimLutMap(MilSceneTarget, MilTargetImage, MilLut);

   // Generating the height map of differences.
   generateHeightMap(MilSystem, MilReferenceModel, MilSceneTarget, MilDefect, MilDefectMask, MilHeightMapImage);

   // Generating the validity map.
   generateValidityMap(MilSystem, MilDefect, MilValidityImage, MilDefectMask);

   // Create and update the display of maps.
   Display(MilSystem, MilReferenceImage, MilTargetImage, MilHeightMapImage, MilValidityImage, Sx, Sy);

   // Free the MIL objects
   MbufFree(MilDefect);
   MbufFree(MilDefectMask);

   MbufFree(MilReferenceModel);
   MbufFree(MilSceneTarget);

   MbufFree(MilReferenceImage);
   MbufFree(MilTargetImage);

   MbufFree(MilHeightMapImage);
   MbufFree(MilValidityImage);

   MbufFree(MilLut);

   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

//*****************************************************************************
// Generate the look up table.
//*****************************************************************************
void generateLUT(MIL_ID &MilLUT)
   {
   MIL_ID LutRedChild,
          LutGreenChild,
          LutBlueChild;

   // Clear the LUT.
   MbufClear(MilLUT, MIN_INTENSITY);

   // Allocate the LUT child buffers.
   MbufChildColor(MilLUT, M_RED  , &LutRedChild);
   MbufChildColor(MilLUT, M_GREEN, &LutGreenChild);
   MbufChildColor(MilLUT, M_BLUE , &LutBlueChild);
   
   // Generate the LUT for bump defects (reddish).
   MgenLutRamp(LutRedChild  , MID_DEPTH_VALUE, MID_INTENSITY, RANGE_HIGH, MAX_INTENSITY);
   MgenLutRamp(LutGreenChild, MID_DEPTH_VALUE, MID_INTENSITY, RANGE_HIGH, MIN_INTENSITY);
   MgenLutRamp(LutBlueChild , MID_DEPTH_VALUE, MID_INTENSITY, RANGE_HIGH, MIN_INTENSITY);

   MgenLutRamp(LutRedChild  , RANGE_HIGH, MAX_INTENSITY, MAX_DEPTH_VALUE, MAX_INTENSITY);
   MgenLutRamp(LutGreenChild, RANGE_HIGH, MIN_INTENSITY, MAX_DEPTH_VALUE, MIN_INTENSITY);
   MgenLutRamp(LutBlueChild , RANGE_HIGH, MIN_INTENSITY, MAX_DEPTH_VALUE, MIN_INTENSITY);

   // Generate the LUT for underfill defects (bluish).
   MgenLutRamp(LutRedChild  , RANGE_LOW, MIN_INTENSITY, MID_DEPTH_VALUE, MID_INTENSITY);
   MgenLutRamp(LutGreenChild, RANGE_LOW, MIN_INTENSITY, MID_DEPTH_VALUE, MID_INTENSITY);
   MgenLutRamp(LutBlueChild , RANGE_LOW, MAX_INTENSITY, MID_DEPTH_VALUE, MID_INTENSITY);

   MgenLutRamp(LutRedChild  , MIN_DEPTH_VALUE, MIN_INTENSITY, RANGE_LOW, MIN_INTENSITY);
   MgenLutRamp(LutGreenChild, MIN_DEPTH_VALUE, MIN_INTENSITY, RANGE_LOW, MIN_INTENSITY);
   MgenLutRamp(LutBlueChild , MIN_DEPTH_VALUE, MAX_INTENSITY, RANGE_LOW, MAX_INTENSITY);

   // Map No-Data to white color.
   std::vector<MIL_UINT8> MaxInt( 1 );
   MaxInt[0] = MAX_INTENSITY;
   MbufPut1d(LutRedChild,   MAX_DEPTH_VALUE, 1, MaxInt);
   MbufPut1d(LutGreenChild, MAX_DEPTH_VALUE, 1, MaxInt);
   MbufPut1d(LutBlueChild,  MAX_DEPTH_VALUE, 1, MaxInt);

   // Free the childs.
   MbufFree(LutRedChild);
   MbufFree(LutGreenChild);
   MbufFree(LutBlueChild);

   return;
   }

//*****************************************************************************
// Generate the Legend for the height map using Red and Blue colors. 
//*****************************************************************************
void generateHeightMapLegend(MIL_ID MilSystem, MIL_ID LUT, MIL_ID MilLegendImage)
   {
   MIL_ID MilLutLegend,
          MilLegend;
   
   // Number of values in the depth map.
   MIL_INT NUM_DEPTHMAP_VALUES = RANGE_HIGH - RANGE_LOW + 1;

   // Retreive the legend size.
   MIL_INT LegendSizeX = MbufInquire(MilLegendImage, M_SIZE_X, M_NULL);
   MIL_INT LegendSizeY = MbufInquire(MilLegendImage, M_SIZE_Y, M_NULL);

   // Allocate the legend buffer.
   MbufAllocColor(MilSystem, 3, LegendSizeX, LegendSizeY, 16 + M_UNSIGNED, M_IMAGE + M_PROC, &MilLegend);

   // Allocate the look up table for legend.
   MbufAllocColor(MilSystem, 3, 1, NUM_DEPTHMAP_VALUES, 16 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &MilLutLegend);

   // Write a linear ramp in an array.
   MIL_UINT16* pLUTLegend = new MIL_UINT16[NUM_DEPTHMAP_VALUES];
   for(MIL_UINT16 i = 0; i < NUM_DEPTHMAP_VALUES; i++)
      {
      pLUTLegend[i] = RANGE_HIGH - i;
      }

   // Put linear ramp in the legend buffer.
   MbufPutColor(MilLutLegend, M_PLANAR, M_RED  , pLUTLegend);
   MbufPutColor(MilLutLegend, M_PLANAR, M_GREEN, pLUTLegend);
   MbufPutColor(MilLutLegend, M_PLANAR, M_BLUE , pLUTLegend);

   // Resize the 1D buffer to fit into the main legend.
   MimResize(MilLutLegend, MilLegend, M_FILL_DESTINATION, M_FILL_DESTINATION, M_BILINEAR);

   // Apply the look up table.
   MimLutMap(MilLegend, MilLegendImage, LUT);

   MIL_TEXT_CHAR mmTxt[16];

   MgraFont(M_DEFAULT, M_FONT_DEFAULT_LARGE);
   MgraFontScale(M_DEFAULT, 1.5, 1.5);

   MgraControl(M_DEFAULT, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);

   // Add high legend text.
   MosSprintf(mmTxt, 16, MIL_TEXT(" + "));
   MgraText(M_DEFAULT, MilLegendImage, LegendSizeX / 2, 10, mmTxt);
   
   // Add low legend text.
   MosSprintf(mmTxt, 16, MIL_TEXT(" - "));
   MgraText(M_DEFAULT, MilLegendImage, LegendSizeX / 2, LegendSizeY - 50, mmTxt);
   MgraFontScale(M_DEFAULT, 1, 1);

   // Free the buffer. 
   MbufFree(MilLutLegend);
   MbufFree(MilLegend);

   // Release the memory.
   delete[] pLUTLegend; 

   return;
   }

//*****************************************************************************
// Set LUT value associated to M_INVALID_POINT to a gray level value.
//*****************************************************************************
void generateLutColorWithInvalidDepth(MIL_ID MilLut, MIL_UINT8 InvalidDepthColor)
   {
   // Create LUT and replace the last value with a special color for invalid points.
   MgenLutFunction(MilLut, M_COLORMAP_JET+M_LAST_GRAY, M_DEFAULT, InvalidDepthColor, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   return;
   }

//*****************************************************************************
// Compute the distance between two 3d Models and generate the heightmap.
//*****************************************************************************
void generateHeightMap(MIL_ID MilSystem, 
                       MIL_ID MilReferenceModel,
                       MIL_ID MilSceneTarget,
                       MIL_ID &MilDefect, 
                       MIL_ID &MilDefectMask, 
                       MIL_ID &MilHeightMapImage)
   {
   MIL_ID MilLUT,
          MilLegendChild;

   // Allocation the LUT buffer.
   MbufAllocColor(MilSystem, 3, MAX_DEPTH_VALUE + 1, 1, 8 + M_UNSIGNED, M_LUT, &MilLUT);

   // Compare the two depthmaps for differences.
   M3dimArith(MilSceneTarget, MilReferenceModel, MilDefect, M_NULL, M_DIST_NN_SIGNED(ARITH_DIST_NN),M_DEFAULT, M_FIT_SCALES);

   // Calculate the validity map.
   M3dimArith(MilSceneTarget, MilReferenceModel, MilDefectMask, M_NULL, M_VALIDITY_MAP, M_DEFAULT, M_DEFAULT);

   // Create the look up table.
   generateLUT(MilLUT);

   // Apply the look up table.
   MimLutMap(MilDefect, MilHeightMapImage, MilLUT);

   // Consider no data as no defect. 
   MbufClearCond(MilHeightMapImage, MAX_INTENSITY, MAX_INTENSITY, MAX_INTENSITY, MilDefectMask, M_NOT_EQUAL, M_BOTH_SRC_VALID_LABEL);

   // Generate the legend for the height map display.
   MbufChildColor2d(MilHeightMapImage,
                    M_ALL_BANDS,
                    IMAGE_WIDTH + LEGEND_MARGIN_X,
                    LEGEND_MARGIN_Y, 
                    LEGEND_WIDTH - (2 * LEGEND_MARGIN_X),
                    IMAGE_HEIGHT - (2 * LEGEND_MARGIN_Y),
                    &MilLegendChild);

   generateHeightMapLegend(MilSystem, MilLUT, MilLegendChild);

   // Free the MIL buffers
   MbufFree(MilLegendChild);
   MbufFree(MilLUT);

   return;
   }

//*****************************************************************************
// Generate the validity map.
//*****************************************************************************
void generateValidityMap(MIL_ID MilSystem, 
                         MIL_ID MilDefect, 
                         MIL_ID &MilValidityImage, 
                         MIL_ID &MilDefectMask)
   {

   // Mil Objects
   MIL_ID MilConvexMask,
          MilConcaveMask,
          MilLegendChild;

   MbufAlloc2d(MilSystem, Sx, Sy, 1 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilConvexMask);
   MbufAlloc2d(MilSystem, Sx, Sy, 1 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilConcaveMask);

   // Consider no data as no defect. 
   MbufClearCond(MilDefect, MID_DEPTH_VALUE, M_NULL, M_NULL, MilDefectMask, M_NOT_EQUAL, M_BOTH_SRC_VALID_LABEL);

   // Threshold the defects.
   const MIL_DOUBLE DefectIsPresent = 1;
   MimBinarize(MilDefect, MilConvexMask, M_FIXED + M_GREATER, MID_DEPTH_VALUE + DEFECT_THRESHOLD, M_NULL);
   MimBinarize(MilDefect, MilConcaveMask, M_FIXED + M_LESS  , MID_DEPTH_VALUE - DEFECT_THRESHOLD, M_NULL);

   //  Pass (Green).
   MbufClearCond(MilValidityImage, (MIL_DOUBLE)Green.RED, (MIL_DOUBLE)Green.GREEN, (MIL_DOUBLE)Green.BLUE, MilDefectMask, M_EQUAL, M_BOTH_SRC_VALID_LABEL);

   // No target Data (Blue).
   MbufClearCond(MilValidityImage, (MIL_DOUBLE)Yellow.RED, (MIL_DOUBLE)Yellow.GREEN, (MIL_DOUBLE)Yellow.BLUE, MilDefectMask, M_EQUAL, M_ONLY_SRC2_VALID_LABEL);

   //  No reference data (Cyan).
   MbufClearCond(MilValidityImage, (MIL_DOUBLE)Cyan.RED, (MIL_DOUBLE)Cyan.GREEN, (MIL_DOUBLE)Cyan.BLUE, MilDefectMask, M_EQUAL, M_ONLY_SRC1_VALID_LABEL);

   // No data (White).
   MbufClearCond(MilValidityImage, 255, 255, 255, MilDefectMask, M_EQUAL, M_NO_SRC_VALID_LABEL);

   // Bump (Red).
   MbufClearCond(MilValidityImage, (MIL_DOUBLE)Red.RED, (MIL_DOUBLE)Red.GREEN, (MIL_DOUBLE)Red.BLUE, MilConvexMask, M_EQUAL, DefectIsPresent);

   // Underfill (Yellow).
   MbufClearCond(MilValidityImage, (MIL_DOUBLE)Blue.RED, (MIL_DOUBLE)Blue.GREEN, (MIL_DOUBLE)Blue.BLUE, MilConcaveMask, M_EQUAL, DefectIsPresent);

   // Generate the legend for the validity map display.
   MbufChild2d(MilValidityImage, IMAGE_WIDTH, 0, LEGEND_WIDTH, IMAGE_HEIGHT, &MilLegendChild);
   generateValidityLegend(MilLegendChild);

   MbufFree(MilLegendChild);
   MbufFree(MilConcaveMask);
   MbufFree(MilConvexMask);

   return;
   }

//*****************************************************************************
// Generate the validity map legend.
//*****************************************************************************
void generateValidityLegend(MIL_ID MilImage)
   {
   MbufClear(MilImage, M_COLOR_WHITE);

   MIL_INT SizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   MIL_DOUBLE Y_START   = (15.0 * (MIL_DOUBLE)SizeY) / 100.0,
              Y_SIZE    = (10.0 * (MIL_DOUBLE)SizeY) / 100.0,
              Y_GAP     = ( 4.0 * (MIL_DOUBLE)SizeY) / 100.0,
              Y_TXT_TOP = ( 3.0 * (MIL_DOUBLE)SizeY) / 100.0,
              Y_TXT_MID = ( 5.0 * (MIL_DOUBLE)SizeY) / 100.0,
              Y_TXT_DWN = ( 7.0 * (MIL_DOUBLE)SizeY) / 100.0,
              X_START   = (20.0 * (MIL_DOUBLE)SizeX) / 100.0,
              X_SIZE    = (70.0 * (MIL_DOUBLE)SizeX) / 100.0,
              X_TXT     = (35.0 * (MIL_DOUBLE)SizeX) / 100.0;

   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);
   MgraControl(M_DEFAULT, M_TEXT_ALIGN_VERTICAL, M_CENTER);

   // Draw the bump section.
   MgraColor(M_DEFAULT, M_RGB888(Red.RED, Red.GREEN, Red.BLUE));
   MgraRectFill(M_DEFAULT, MilImage, X_START, Y_START, X_START + X_SIZE, Y_START + Y_SIZE);
   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_MID, MIL_TEXT("Bump"));

   // Draw the underfill section.
   Y_START += Y_SIZE + Y_GAP;
   MgraColor(M_DEFAULT, M_RGB888(Blue.RED, Blue.GREEN, Blue.BLUE));
   MgraRectFill(M_DEFAULT, MilImage, X_START, Y_START, X_START + X_SIZE, Y_START + Y_SIZE);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_MID, MIL_TEXT("Underfill"));

   // Draw the pass section.
   Y_START += Y_SIZE + Y_GAP;
   MgraColor(M_DEFAULT, M_RGB888(Green.RED, Green.GREEN, Green.BLUE));
   MgraRectFill(M_DEFAULT, MilImage, X_START, Y_START, X_START + X_SIZE, Y_START + Y_SIZE);
   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_MID, MIL_TEXT("Pass"));

   // Draw the reference no data section.
   Y_START += Y_SIZE + Y_GAP;
   MgraColor(M_DEFAULT, M_RGB888(Cyan.RED, Cyan.GREEN, Cyan.BLUE));
   MgraRectFill(M_DEFAULT, MilImage, X_START, Y_START, X_START + X_SIZE, Y_START + Y_SIZE);
   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_TOP, MIL_TEXT("Reference"));
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_DWN, MIL_TEXT("Missing"));

   // Draw the target no data section.
   Y_START += Y_SIZE + Y_GAP;
   MgraColor(M_DEFAULT, M_RGB888(Yellow.RED, Yellow.GREEN, Yellow.BLUE));
   MgraRectFill(M_DEFAULT, MilImage, X_START, Y_START, X_START + X_SIZE, Y_START + Y_SIZE);
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_TOP, MIL_TEXT("Target"));
   MgraText(M_DEFAULT, MilImage, X_START + X_TXT, Y_START + Y_TXT_DWN, MIL_TEXT("Missing"));

   return;
   }

//*****************************************************************************
// Setup the display.
//*****************************************************************************
void Display(MIL_ID MilSystem, 
             MIL_ID MilReferenceImage, 
             MIL_ID MilTargetImage, 
             MIL_ID  MilHeightMapImage, 
             MIL_ID MilValidityImage, 
             MIL_UINT SizeX, 
             MIL_UINT SizeY)
   {

   MIL_ID MilReferenceDisplay,
          MilTargetDisplay,
          MilHeightMapDisplay,
          MilValidityDisplay;

   // Allocate displays.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilReferenceDisplay);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilTargetDisplay);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilHeightMapDisplay);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilValidityDisplay);

   // Set initial position of the windows.
   MdispControl(MilReferenceDisplay, M_WINDOW_INITIAL_POSITION_X, 0);
   MdispControl(MilReferenceDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);

   MdispControl(MilTargetDisplay, M_WINDOW_INITIAL_POSITION_X, WINDOW_WIDTH + WINDOW_GAP);
   MdispControl(MilTargetDisplay, M_WINDOW_INITIAL_POSITION_Y, 0);

   MdispControl(MilHeightMapDisplay, M_WINDOW_INITIAL_POSITION_X, 0);
   MdispControl(MilHeightMapDisplay, M_WINDOW_INITIAL_POSITION_Y, WINDOW_HEIGHT + WINDOW_BAR_SIZE + WINDOW_GAP);

   MdispControl(MilValidityDisplay, M_WINDOW_INITIAL_POSITION_X, WINDOW_WIDTH + WINDOW_GAP);
   MdispControl(MilValidityDisplay, M_WINDOW_INITIAL_POSITION_Y, WINDOW_HEIGHT + WINDOW_BAR_SIZE + WINDOW_GAP);

   // Set the name of windows.
   MdispControl(MilReferenceDisplay, M_TITLE, MIL_TEXT("Reference Depthmap"));
   MdispControl(MilTargetDisplay   , M_TITLE, MIL_TEXT("Target Depthmap"));
   MdispControl(MilHeightMapDisplay, M_TITLE, MIL_TEXT("Height Map of Differences"));
   MdispControl(MilValidityDisplay , M_TITLE, MIL_TEXT("Validity Map"));

   // Set size of windows.
   MIL_DOUBLE ZoomX = WINDOW_WIDTH  / (MIL_DOUBLE)SizeX;
   MIL_DOUBLE ZoomY = WINDOW_HEIGHT / (MIL_DOUBLE)SizeY;

   MdispZoom(MilReferenceDisplay, ZoomX, ZoomY);
   MdispZoom(MilTargetDisplay   , ZoomX, ZoomY);
   MdispZoom(MilHeightMapDisplay, ZoomX, ZoomY);
   MdispZoom(MilValidityDisplay , ZoomX, ZoomY);

   MdispSelect(MilValidityDisplay , MilValidityImage);
   MdispSelect(MilHeightMapDisplay, MilHeightMapImage);
   MdispSelect(MilTargetDisplay   , MilTargetImage);
   MdispSelect(MilReferenceDisplay, MilReferenceImage);

   // Wait for user.
   MosPrintf(MIL_TEXT("Press <enter> to end.\n"));
   MosGetchar();

   MdispFree(MilReferenceDisplay);
   MdispFree(MilTargetDisplay   );
   MdispFree(MilHeightMapDisplay);
   MdispFree(MilValidityDisplay );
   
   return;
   }
