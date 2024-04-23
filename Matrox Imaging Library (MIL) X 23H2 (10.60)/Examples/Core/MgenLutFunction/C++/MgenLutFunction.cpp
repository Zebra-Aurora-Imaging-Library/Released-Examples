//*******************************************************************************/
//
// File name: MgenLutFunction.cpp
//
// Synopsis:  This program shows how MgenLutFunction() can generate various
//            LUT (lookup table) profiles using:
//
//             1 - Custom mathematical functions to change the dynamic
//                 of an image.
//
//             2 - A specific color-map to enhance display.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//********************************************************************************/

#include <mil.h> 
#include <math.h>

// Source image file specifications.
#define IMAGE_FILE1                     M_IMAGE_PATH MIL_TEXT("CircuitsBoard.mim")
#define IMAGE_FILE2                     M_IMAGE_PATH MIL_TEXT("Candy.mim")

// Example functions declarations.
void CustomMathLut(MIL_ID MilSystem, MIL_ID MilDisplay);
void PseudoColorMap(MIL_ID MilSystem, MIL_ID MilDisplay);
void HueColorMap(MIL_ID MilSystem, MIL_ID MilDisplay);

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("MgenLutFunction\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program shows how MgenLutFunction() can generate\n")
      MIL_TEXT("various LUT (lookup table) profiles using:\n\n")
      MIL_TEXT("\t1 - Custom mathematical functions to change\n")
      MIL_TEXT("\t    the dynamic of an image.\n\n")
      MIL_TEXT("\t2 - A specific color-map to enhance display.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("image processing, data generation.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }


//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID   MilApplication,      // Application identifier.
            MilSystem,           // System identifier.
            MilDisplay;          // Display identifier.

   // Allocate defaults.
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, M_NULL, M_NULL);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Print header.
   PrintHeader();

   // Run the example using a custom mathematical function.
   CustomMathLut(MilSystem, MilDisplay);

   // Run the example using a specified color-map.
   PseudoColorMap(MilSystem, MilDisplay);

   // Run the example using a HUE color-map.
   HueColorMap(MilSystem, MilDisplay);

   // Free allocated resources.
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

void CustomMathLut(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilDispImage,              // Display image buffer identifier for both source and result images.
            MilLeftSubImage,           // Sub-image buffer identifier for original image.
            MilRightSubImage,          // Sub-image buffer identifier for processed image.
            MilMonoLut;                // LUT through which to map source input values.

   MIL_DOUBLE a, b, c;                 // The constants for the mathematical function.

   MIL_INT  SizeX,                     // Source image size X.
            SizeY;                     // Source image size Y.

   const MIL_INT ImageMinValue = 0;    // Source image min value.
   const MIL_INT ImageMaxValue = 255;  // Source image max value.

   MosPrintf(MIL_TEXT("1 -Image dynamic modification using a custom LUT.\n")
             MIL_TEXT("-------------------------------------------------\n\n"));

   // Retrieving source size.
   MbufDiskInquire(IMAGE_FILE1, M_SIZE_X, &SizeX);
   MbufDiskInquire(IMAGE_FILE1, M_SIZE_Y, &SizeY);

   // Allocate a display buffer twice the size of the source image and display it.
   MbufAlloc2d(MilSystem,  SizeX * 2, SizeY , 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilDispImage);
   MbufClear(MilDispImage, 0L);
   MdispSelect(MilDisplay, MilDispImage);

   // Define the left and right part of the display buffer as two child buffers,
   // to display the source and result images side by side.
   MbufChild2d(MilDispImage, 0L, 0L, SizeX, SizeY, &MilLeftSubImage);
   MbufChild2d(MilDispImage, SizeX, 0L, SizeX, SizeY, &MilRightSubImage);

   // Restore and display the source image.
   MbufLoad(IMAGE_FILE1, MilLeftSubImage);
   MosPrintf(MIL_TEXT("An 8-bit monochrome image is loaded and displayed.\n\n"));

   // Allocate a buffer to store lookup table data. 
   MbufAlloc1d(MilSystem, 256, 8 + M_UNSIGNED, M_LUT, &MilMonoLut);

   // Filling the LUT with a power function
   //       ^
   //       |       ++++++
   //       |    +++
   //       |  ++
   //       | +
   //       |+
   //       +------------>
   a = ImageMaxValue/log10((MIL_DOUBLE)ImageMaxValue), b = 10, c =0;;
   MgenLutFunction(MilMonoLut, M_LOG, a, b, c, ImageMinValue, (MIL_DOUBLE)ImageMinValue, ImageMaxValue);
   MimLutMap(MilLeftSubImage, MilRightSubImage, MilMonoLut);
   
   MosPrintf(MIL_TEXT("The source image intensities were transformed with\n"));
   MosPrintf(MIL_TEXT("a LUT that was filled using a M_LOG function.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Filling the LUT with a power function
   //       ^
   //       |          +
   //       |          +
   //       |        ++
   //       |     +++
   //       |+++++
   //       +------------>
   a = 1.0 / (MIL_DOUBLE)ImageMaxValue, b = 2, c = 0;
   MgenLutFunction(MilMonoLut, M_POWER, a, b, c, ImageMinValue, (MIL_DOUBLE)ImageMinValue, ImageMaxValue);
   MimLutMap(MilLeftSubImage, MilRightSubImage, MilMonoLut);
   
   MosPrintf(MIL_TEXT("The source image intensities were transformed with\n"));
   MosPrintf(MIL_TEXT("a LUT that was filled using a M_POWER function.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Filling the LUT with a piecewise linear function
   //       ^
   //       |    ++
   //       |   +  +
   //       |  +    +
   //       | +      +
   //       |+        +
   //       +------------>
   MIL_DOUBLE InflectionValue = (ImageMinValue + ImageMaxValue) / 2;
   MgenLutRamp(MilMonoLut, ImageMinValue, (MIL_DOUBLE)ImageMinValue, (MIL_INT)InflectionValue, (MIL_DOUBLE)ImageMaxValue);
   MgenLutRamp(MilMonoLut,  (MIL_INT)InflectionValue, (MIL_DOUBLE)ImageMaxValue, ImageMaxValue, (MIL_DOUBLE)ImageMinValue);
   MimLutMap(MilLeftSubImage, MilRightSubImage, MilMonoLut);

   MosPrintf(MIL_TEXT("The source image intensities were transformed with\n"));
   MosPrintf(MIL_TEXT("a LUT that was filled using a piecewise linear function.\n\n"));
   MosGetch();

   // Free the allocated objects.
   MbufFree(MilMonoLut);
   MbufFree(MilRightSubImage);
   MbufFree(MilLeftSubImage);
   MbufFree(MilDispImage);
   }

void PseudoColorMap(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilDispImage,          // Display image buffer identifier for both source and result images.
            MilMonoImage,          // 8-bit source image buffer identifier.
            MilLeftSubImage,       // Sub-image buffer identifier for original image.
            MilRightSubImage,      // Sub-image buffer identifier for processed image.
            MilLut;                // LUT through which to map source input values.

   MIL_INT  SizeX, 
            SizeY;

   MosPrintf(MIL_TEXT("2 -Pseudo-color display using a color-map LUT.\n")
             MIL_TEXT("----------------------------------------------\n\n"));

   // Restore the source monochrome image into a 8-bit buffer for processing.
   MbufRestore(IMAGE_FILE1, MilSystem, &MilMonoImage); 

   // Retrieving source size.
   MbufInquire(MilMonoImage, M_SIZE_X, &SizeX);
   MbufInquire(MilMonoImage, M_SIZE_Y, &SizeY);

   // Allocate a color display buffer twice the size of the source image.
   MbufAllocColor(MilSystem, 3, SizeX * 2, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilDispImage);
   MbufClear(MilDispImage, 0L);
   MdispSelect(MilDisplay, MilDispImage);

   // Define the left and right part of the display buffer as two child buffers,
   // to display the source monochrome and result color images side by side.
   MbufChildColor2d(MilDispImage, M_ALL_BANDS, 0L, 0L, SizeX, SizeY, &MilLeftSubImage);
   MbufChildColor2d(MilDispImage, M_ALL_BANDS, SizeX, 0L, SizeX, SizeY, &MilRightSubImage);

   // Restore and display the source image.
   MbufCopy(MilMonoImage, MilLeftSubImage);
   MosPrintf(MIL_TEXT("An 8-bit monochrome image is loaded and displayed.\n\n"));

   // Allocate a color LUT buffer for color mapping.
   MbufAllocColor(MilSystem, 3, 256, 1, 8 + M_UNSIGNED, M_LUT, &MilLut);

   // Fill the LUT buffer with a hot color-map.
   MgenLutFunction(MilLut, M_COLORMAP_HOT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The image is displayed using a M_COLORMAP_HOT LUT.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fill the LUT buffer with a jet color-map.
   MgenLutFunction(MilLut, M_COLORMAP_JET, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The image is displayed using a M_COLORMAP_JET LUT.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fill the LUT buffer with a spectrum color-map.
   MgenLutFunction(MilLut, M_COLORMAP_SPECTRUM, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The image is displayed using a M_COLORMAP_SPECTRUM LUT.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fill the LUT buffer with a turbo color-map.
   MgenLutFunction(MilLut, M_COLORMAP_TURBO, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The image is displayed using a M_COLORMAP_TURBO LUT.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the allocations.
   MbufFree(MilLut);
   MbufFree(MilRightSubImage);
   MbufFree(MilLeftSubImage);
   MbufFree(MilMonoImage);
   MbufFree(MilDispImage);
   }

void HueColorMap(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilDispImage,          // Display image buffer identifier for both source and result images.
            MilMonoImage,          // 8-bit image buffer identifier to extract the hue component.
            MilLeftSubImage,       // Sub-image buffer identifier for original image.
            MilRightSubImage,      // Sub-image buffer identifier for processed image.
            MilLut;                // LUT through which to map source input values.

   MIL_INT  SizeX, 
            SizeY;

   MosPrintf(MIL_TEXT("3 -Hue component display using a M_COLORMAP_HUE LUT.\n")
             MIL_TEXT("---------------------------------------------------\n\n"));

   // Retrieving source size.
   MbufDiskInquire(IMAGE_FILE2, M_SIZE_X, &SizeX);
   MbufDiskInquire(IMAGE_FILE2, M_SIZE_Y, &SizeY);

   // Allocate a color display buffer twice the size of the source image.
   MbufAllocColor(MilSystem, 3, SizeX * 2, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilDispImage);
   MbufClear(MilDispImage, 0L);
   MdispSelect(MilDisplay, MilDispImage);

   // Define the left and right part of the display buffer as two child buffers,
   // to display the source monochrome and result color images side by side.
   MbufChildColor2d(MilDispImage, M_ALL_BANDS, 0L, 0L, SizeX, SizeY, &MilLeftSubImage);
   MbufChildColor2d(MilDispImage, M_ALL_BANDS, SizeX, 0L, SizeX, SizeY, &MilRightSubImage);

   // Allocate a 8-bit monochrome buffer to extract the hue component.
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilMonoImage);

   // Restore and display the source color image.
   MbufLoad(IMAGE_FILE2, MilLeftSubImage);
   MosPrintf(MIL_TEXT("A color image is loaded and displayed.\n\n"));

   // Apply a RGB to hue conversion.
   MimConvert(MilLeftSubImage, MilMonoImage, M_RGB_TO_H);

   // Allocate a color LUT buffer for color mapping.
   MbufAllocColor(MilSystem, 3, 256, 1, 8 + M_UNSIGNED, M_LUT, &MilLut);

   // Fill the LUT buffer with a hue color-map.
   MgenLutFunction(MilLut, M_COLORMAP_HUE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The hue component of the image is displayed using  \n"));
   MosPrintf(MIL_TEXT("a M_COLORMAP_HUE LUT.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Fill the LUT buffer with a flipped hue color-map.
   MgenLutFunction(MilLut, M_COLORMAP_HUE + M_FLIP, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MilMonoImage, MilRightSubImage, MilLut);

   MosPrintf(MIL_TEXT("The hue component of the image is displayed using \n"));
   MosPrintf(MIL_TEXT("a M_COLORMAP_HUE + M_FLIP LUT.\n\n"));
   MosPrintf(MIL_TEXT("The M_FLIP feature reverse the sequence of colors \n"));
   MosPrintf(MIL_TEXT("in the M_COLORMAP_HUE.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Free the allocated objects.
   MbufFree(MilMonoImage);
   MbufFree(MilLut);
   MbufFree(MilRightSubImage);
   MbufFree(MilLeftSubImage);
   MbufFree(MilDispImage);
   }
