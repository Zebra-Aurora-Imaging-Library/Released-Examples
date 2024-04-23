//*******************************************************************************/
/*
* File name: UtilMatchColormap.cpp
*
* Synopsis:  This example allows you to easily restore a color matching context and
*            interactively display a colormap with the corresponding matched areas.
*            The colormap is provided for a given luminance value which you can
*            interactively change using the keyboard.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>  

/* Util dimensions to generate display and buffers. */
const long COLORMAP_SIZE = 256;
const long COLOR_SAMPLE_SIZE = 32;
const long COLOR_OVERVIEW_MAP_SIZE = 64;
const long DISPLAY_TEXT_SIZE = 20;
const long OVERVIEW_COLMAP_NUMBER = 8;
const long INIT_LUMINANCE_VALUE = 128;

/* Functions to generate the colormap images. */
void InitHSLColormap(MIL_ID MilHSLColormapID);

MIL_INT GetOverviewColormapOffset(MIL_INT Luminance);

void GenColormap(MIL_ID MilHSLColormap,
                 MIL_ID ColprmapImage,
                 MIL_ID Luminance);

/* Utility function and macros. */
void RestoreColorContext(MIL_ID MilSystem,
                         MIL_ID &MilColContext,
                         MIL_ID MilDispImage);

#define STRING_LENGTH_MAX 128
#define MosMin(a, b) (((a) < (b)) ? (a) : (b))
#define MosMax(a, b) (((a) > (b)) ? (a) : (b))

/* Default color context to load. */
#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("FoodInspectionMango/")
static MIL_CONST_TEXT_PTR COLOR_CONTEXT_PATH = EXAMPLE_IMAGE_PATH MIL_TEXT("MangoColor.mcol");

int MosMain(void)
{
   MIL_ID  MilApplication,        /* Application identifier.    */
           MilSystem,             /* System identifier.         */
           MilDispImage,          /* Image to display.          */
           MilOverlay,            /* Display overlay buffer.    */
           MilHSLColormap,        /* HSL colormap buffer.       */
           MilColormapChild,      /* Colormap child buffer.     */
           MilColorResChild,      /* Color result child buffer. */
           MilColContext,         /* Color context.             */
           MilColResult,          /* Color result.              */
           MilColSample,          /* Color sample buffer.       */
           MilBlobContext,        /* Blob context.              */
           MilBlobResult,         /* Blob result buffer.        */
           MilDisplay;            /* Display identifier.        */

   MIL_INT Luminance = INIT_LUMINANCE_VALUE,
           Offset, SizeX, SizeY,
           NbSamples,
           Index, Ch;

   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX];

   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("UtilMatchColormap\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example allows you to easily restore a color matching context and\n")
             MIL_TEXT("interactively display a colormap with the corresponding matched areas.\n")
             MIL_TEXT("The colormap is provided for a given luminance value which you can\n")
             MIL_TEXT("interactively change using the keyboard.\n\n"));

   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Allocate the main display buffer. */
   SizeX = 2 * COLORMAP_SIZE + 1;
   SizeY = 2 * DISPLAY_TEXT_SIZE + COLOR_SAMPLE_SIZE + COLORMAP_SIZE + 2 * COLOR_OVERVIEW_MAP_SIZE + 3;
   MbufAllocColor(MilSystem, 3, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &MilDispImage);
   MbufClear(MilDispImage, 0);

   /* Allocate main display child buffers. */
   MbufChild2d(MilDispImage, 0, COLOR_SAMPLE_SIZE + 2 * DISPLAY_TEXT_SIZE, COLORMAP_SIZE, COLORMAP_SIZE, &MilColormapChild);
   MbufChild2d(MilDispImage, COLORMAP_SIZE + 1, COLOR_SAMPLE_SIZE + 2 * DISPLAY_TEXT_SIZE, COLORMAP_SIZE, COLORMAP_SIZE, &MilColorResChild);

   /* Allocate and initialize the HLS buffer used to generate the colormap. */
   MbufAllocColor(MilSystem, 3, COLORMAP_SIZE, COLORMAP_SIZE, 8 + M_UNSIGNED, M_IMAGE + M_PLANAR + M_PROC, &MilHSLColormap);
   InitHSLColormap(MilHSLColormap);

   /* Allocate a color result buffer. */
   McolAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilColResult);

   /* Restore color match context and create the color match overview. */
   RestoreColorContext(MilSystem, MilColContext, MilDispImage);

   /* Inquire the number of color samples. */
   McolInquire(MilColContext, M_CONTEXT, M_NUMBER_OF_SAMPLES + M_TYPE_MIL_INT, &NbSamples);

   /* Enable the display of overlay and retrieve the overlay buffer. */
   MdispSelect(MilDisplay, MilDispImage);
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlay);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Add display annotations. */
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilDispImage, 0, 2, MIL_TEXT("Context samples average colors:"));
   MgraText(M_DEFAULT, MilDispImage, 0, COLOR_SAMPLE_SIZE + DISPLAY_TEXT_SIZE + 2, MIL_TEXT("Luminance = "));
   
   /* Allocate a buffer and blob ressources to display contours of the mathed areas. */
   MbufAlloc2d(MilSystem, COLORMAP_SIZE, COLORMAP_SIZE, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilColSample);
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);
   MblobControl(MilBlobContext, M_SAVE_RUNS, M_ENABLE);

   /* Modify the colormap according to the arrow keys and update it. */
   MosPrintf(MIL_TEXT("To interact with the utility, press the:\n"));
   MosPrintf(MIL_TEXT(". Left or Down key, which decreases the luminance value.\n"));
   MosPrintf(MIL_TEXT(". Right or Up key, which increases the luminance value.\n"));
   MosPrintf(MIL_TEXT(". 'N' or 'n' key, which restores a new color matching context.\n"));
   MosPrintf(MIL_TEXT(". <Enter> key, which terminates the program.\n\n"));

   Ch = 0;
   while (Ch != '\r')
      {
      switch (Ch)
         {
         /* Left/down arrow: decrease the luminance value. */
         case 0x4B:
         case 0x50:
            { Luminance -= 1; break; }

         /* Right/up arrow: increase the luminance value. */
         case 0x4D:
         case 0x48:
            { Luminance += 1; break; }

         /* 'N'/'n' to restore a new color match context. */
         case 'N':
         case 'n':
            {
            /* Restore a new color match context and create the color match overview. */
            McolFree(MilColContext);
            RestoreColorContext(MilSystem, MilColContext, MilDispImage);
            /* Inquire the number of color samples. */
            McolInquire(MilColContext, M_CONTEXT, M_NUMBER_OF_SAMPLES + M_TYPE_MIL_INT, &NbSamples);
            /* Reset the luminance value. */
            Luminance = INIT_LUMINANCE_VALUE;
            }
         }

      /* Saturate the luminace value and update the displayed value. */
      Luminance = MosMin(Luminance, 255);
      Luminance = MosMax(Luminance, 0);

      MgraColor(M_DEFAULT, M_COLOR_WHITE);
      MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%d  "), (int)Luminance);
      MgraText(M_DEFAULT, MilDispImage, 100, COLOR_SAMPLE_SIZE + DISPLAY_TEXT_SIZE + 2, Text);

      /* Generate the map of colors. */
      GenColormap(MilHSLColormap, MilColormapChild, Luminance);

      /* Match the colors in the map of colors. */
      McolMatch(MilColContext, MilColormapChild, M_DEFAULT, M_NULL, MilColResult, M_DEFAULT);

      /* Display the matched color result per pixel. */ 
      McolDraw(M_DEFAULT, MilColResult, MilColorResChild, M_DRAW_PIXEL_MATCH_USING_COLOR, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      /* Display the contours of the matched areas in the colormap. */
      MgraColor(M_DEFAULT, M_COLOR_WHITE);      
      for (Index = 0; Index<NbSamples; Index++)
         {
         MbufClear(MilColSample, 0);
         McolDraw(M_DEFAULT, MilColResult, MilColSample, M_DRAW_PIXEL_MATCH_USING_LABEL, M_DEFAULT, M_SAMPLE_INDEX(Index), M_DEFAULT);
         MblobCalculate(MilBlobContext, MilColSample, M_NULL, MilBlobResult);
         MblobDraw(M_DEFAULT, MilBlobResult, MilColormapChild, M_DRAW_BLOBS_CONTOUR, M_DEFAULT, M_DEFAULT);
         }

      /* Update the rectangle in the colormap overview display. */
      MgraColor(M_DEFAULT, M_COLOR_WHITE);
      Offset = GetOverviewColormapOffset(Luminance);
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
      MgraRect(M_DEFAULT, MilOverlay, 
               Offset, COLOR_SAMPLE_SIZE + 2 * DISPLAY_TEXT_SIZE + COLORMAP_SIZE + 1, 
               Offset + (2 * COLORMAP_SIZE) / OVERVIEW_COLMAP_NUMBER - 1, 
               COLOR_SAMPLE_SIZE + 2 * DISPLAY_TEXT_SIZE + COLORMAP_SIZE + 2 * COLOR_OVERVIEW_MAP_SIZE + 1);

      /* If its an arrow key, get the second code. */
      if ((Ch = MosGetch()) == 0xE0)
         Ch = MosGetch();
      }

   /* Free allocated objects. */
   MbufFree(MilColormapChild);
   MbufFree(MilColorResChild);
   MbufFree(MilDispImage);
   MbufFree(MilHSLColormap);
   MbufFree(MilColSample);
   McolFree(MilColContext);
   McolFree(MilColResult);
   MblobFree(MilBlobContext);
   MblobFree(MilBlobResult); 

   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   };


/* RestoreColorContext utility function: 
*  - to restore and to preprocess a color context.
*  - to retrieve and display context color samples.
*  - to retrieve and display an overview of the matched colors.
*/
void RestoreColorContext(MIL_ID MilSystem,
                         MIL_ID &MilColContext,
                         MIL_ID MilDispImage)
{
   MIL_ID MilColBuffer,
          MilColResult,
          MilHSLColormap,
          MilUtilChild,
          MilOverviewMap;

   MIL_INT Luminance,
           NbSamples,
           Offset,
           SizePerSample,
           Index;

   MIL_INT Rvalue,
           Gvalue,
           Bvalue;

   MIL_TEXT_CHAR MyChar = ' ';
   MosPrintf(MIL_TEXT("Press 'D' to restore a default context, or\n")
             MIL_TEXT("press another key to select a new context.\n\n"));
   
   // Get the last character.
   MyChar = (MIL_TEXT_CHAR)MosGetch();

   if (MyChar == 'd' || MyChar == 'D')
      {
      McolRestore(COLOR_CONTEXT_PATH, MilSystem, M_DEFAULT, &MilColContext);
      }
   else
      {
      /* Restore and preprocess a color match context. */
      MosPrintf(MIL_TEXT("Select a new color matching context to restore.\n")
                MIL_TEXT("<Cancel> will restore a default context.\n\n"));

      MappControl(M_ERROR, M_PRINT_DISABLE);
      McolRestore(M_INTERACTIVE, MilSystem, M_DEFAULT, &MilColContext);
      MappControl(M_ERROR, M_PRINT_ENABLE);

      /* Restore a default context if needed. */
      if (MilColContext == M_NULL)
         McolRestore(COLOR_CONTEXT_PATH, MilSystem, M_DEFAULT, &MilColContext);
      }

   McolControl(MilColContext, M_DEFAULT, M_OUTLIER_DRAW_COLOR, 0);
   McolPreprocess(MilColContext, M_DEFAULT);

   /* Display the color context samples average colors. */
   McolInquire(MilColContext, M_CONTEXT, M_NUMBER_OF_SAMPLES + M_TYPE_MIL_INT, &NbSamples);
   SizePerSample = (2 * COLORMAP_SIZE) / NbSamples;

   MgraColor(M_DEFAULT, M_COLOR_BLACK);
   for (Index = 0; Index<NbSamples; Index++)
      {
      McolInquire(MilColContext, M_SAMPLE_INDEX(Index), M_SAMPLE_8BIT_AVERAGE_COLOR_BAND_0 + M_TYPE_MIL_INT, &Rvalue);
      McolInquire(MilColContext, M_SAMPLE_INDEX(Index), M_SAMPLE_8BIT_AVERAGE_COLOR_BAND_1 + M_TYPE_MIL_INT, &Gvalue);
      McolInquire(MilColContext, M_SAMPLE_INDEX(Index), M_SAMPLE_8BIT_AVERAGE_COLOR_BAND_2 + M_TYPE_MIL_INT, &Bvalue);
      MbufChild2d(MilDispImage, Index * SizePerSample, DISPLAY_TEXT_SIZE, SizePerSample, COLOR_SAMPLE_SIZE, &MilUtilChild);
      MbufClear(MilUtilChild, M_RGB888(Rvalue, Gvalue, Bvalue));
      MgraLine(M_DEFAULT, MilDispImage, Index * SizePerSample, DISPLAY_TEXT_SIZE, Index * SizePerSample, DISPLAY_TEXT_SIZE + COLOR_SAMPLE_SIZE);
      MbufFree(MilUtilChild);
      }

   /* Generate the overview color and result maps. */
   McolAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilColResult);
   MbufAllocColor(MilSystem, 3, 2 * COLORMAP_SIZE + 1, 2 * COLOR_OVERVIEW_MAP_SIZE, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilOverviewMap);   
   MbufAllocColor(MilSystem, 3, COLORMAP_SIZE, COLORMAP_SIZE, 8 + M_UNSIGNED, M_IMAGE + M_PLANAR + M_PROC, &MilHSLColormap);
   MbufAllocColor(MilSystem, 3, COLORMAP_SIZE, COLORMAP_SIZE, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &MilColBuffer);
   MbufClear(MilOverviewMap, M_COLOR_BLACK);

   InitHSLColormap(MilHSLColormap);

   for (Index = 0; Index<OVERVIEW_COLMAP_NUMBER; Index++)
      {
      Luminance = (MIL_INT)((Index + 0.5) * 255 / OVERVIEW_COLMAP_NUMBER);
      Offset = GetOverviewColormapOffset(Luminance);

      GenColormap(MilHSLColormap, MilColBuffer, Luminance);

      MbufChild2d(MilOverviewMap, Offset, 0, (2 * COLORMAP_SIZE) / OVERVIEW_COLMAP_NUMBER, COLOR_OVERVIEW_MAP_SIZE, &MilUtilChild);
      MimResize(MilColBuffer, MilUtilChild, M_FILL_DESTINATION, M_FILL_DESTINATION, M_DEFAULT);
      MbufFree(MilUtilChild);

      McolMatch(MilColContext, MilColBuffer, M_DEFAULT, M_NULL, MilColResult, M_DEFAULT);
      McolDraw(M_DEFAULT, MilColResult, MilColBuffer, M_DRAW_PIXEL_MATCH_USING_COLOR, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      MbufChild2d(MilOverviewMap, Offset, COLOR_OVERVIEW_MAP_SIZE, (2 * COLORMAP_SIZE) / OVERVIEW_COLMAP_NUMBER, COLOR_OVERVIEW_MAP_SIZE, &MilUtilChild);
      MimResize(MilColBuffer, MilUtilChild, M_FILL_DESTINATION, M_FILL_DESTINATION, M_DEFAULT);
      MbufFree(MilUtilChild);
      
      MbufCopyColor2d(MilOverviewMap, MilDispImage, M_ALL_BANDS, 0, 0, M_ALL_BANDS, 0, 2 * DISPLAY_TEXT_SIZE + COLOR_SAMPLE_SIZE + COLORMAP_SIZE + 2, 2 * COLORMAP_SIZE + 1, 2 * COLOR_OVERVIEW_MAP_SIZE -1);
      }

   /* Release allocated ressources.*/
   MbufFree(MilColBuffer);
   MbufFree(MilHSLColormap);
   MbufFree(MilOverviewMap);
   McolFree(MilColResult);
   };

/* InitHSLColormap utility function:
*  - to initialize the Hue band with an horizontal linear ramp.
*  - to initialize the Saturation band with a vertical linear ramp.
*/
void InitHSLColormap(MIL_ID MilHSLColormapID)
   {
   MIL_INT i, j;
   MIL_UINT8 pData[COLORMAP_SIZE][COLORMAP_SIZE];

   // Fill the Hue band
   for (j = 0; j < COLORMAP_SIZE; j++)
      for (i = 0; i < COLORMAP_SIZE; i++)
         pData[i][j] = (MIL_UINT8)(j);
   MbufPutColor(MilHSLColormapID, M_PLANAR, M_RED, pData);

   // Fill the Saturation band
   for (j = 0; j < COLORMAP_SIZE; j++)
      for (i = 0; i < COLORMAP_SIZE; i++)
         pData[i][j] = (MIL_UINT8)(i);
   MbufPutColor(MilHSLColormapID, M_PLANAR, M_GREEN, pData);
   };

/* GenColormap utility function:
*  - to generate an RGB colormap for a given Luminance value.
*/
void GenColormap(MIL_ID MilHSLColormap, MIL_ID ColormapImage, MIL_ID Luminance)
   {
   // Clear the Luminance band with the provided value.
   MIL_ID BlueBand;
   MbufChildColor(MilHSLColormap, M_BLUE, &BlueBand);
   MbufClear(BlueBand, (MIL_DOUBLE)Luminance);
   MbufFree(BlueBand);
   // Convert the HSL buffer to RGB.
   MimConvert(MilHSLColormap, ColormapImage, M_HSL_TO_RGB);
   };

/* GetOverviewColormapOffset utility function:
*  - to retrieve the offset of an overview colormap for a given Luminance.
*/
MIL_INT GetOverviewColormapOffset(MIL_INT Luminance)
   {
   MIL_INT Index = (MIL_INT)(Luminance*OVERVIEW_COLMAP_NUMBER / 256);
   return Index * ((2 * COLORMAP_SIZE) / OVERVIEW_COLMAP_NUMBER);
   };
