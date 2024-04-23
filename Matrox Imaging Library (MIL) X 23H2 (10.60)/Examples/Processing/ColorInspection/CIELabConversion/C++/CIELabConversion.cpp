/************************************************************************************/
/*
 * File name: CIELabConversion.cpp 
 *
 * Synopsis:  This program demonstrates how to convert a grabbed image
 *            to the CIELab color space.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

#define STRING_LENGTH_MAX     128
#define COLOR_PICKER_SIZE       6

#define  INIT_COLOR_PICKER_POSX 526
#define  INIT_COLOR_PICKER_POSY 275

/* Example functions prototypes. */
void ConvertToCIELab(MIL_ID MilSystem, 
                     MIL_ID MilDisplay, 
                     MIL_CONST_TEXT_PTR ReferenceFilename, 
                     MIL_CONST_TEXT_PTR ColorCalibrationGridFilename, 
                     MIL_CONST_TEXT_PTR TargetFilename);

/* Source images file name. */
#define CALIBRATION_IMAGE  M_IMAGE_PATH MIL_TEXT("CIELabConversion/ColorCalibrationGrid.tif")
#define REFERENCE_IMAGE    M_IMAGE_PATH MIL_TEXT("CIELabConversion/ColorCalibrationReference.mim") 
#define GRABBED_IMAGE      M_IMAGE_PATH MIL_TEXT("CIELabConversion/Colorful.tif")

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("CIELabConversion\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This example demonstrates how to convert the color data of a grabbed image\n")
      MIL_TEXT("from RGB to the CIELab color space.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, buffer, color analysis, display, graphic,\n")
      MIL_TEXT("image processing, system.\n\n"));
   }

/*---------------------------------------------------------------------------*/
int MosMain(void)
   {
   PrintHeader();

   MIL_ID MilApplication,  /* Application identifier. */
          MilSystem,       /* System identifier.      */
          MilDisplay;      /* Display identifier.     */
          
   /* Allocations. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Set display properties. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);

   /* Run the projection visualization example. */
   ConvertToCIELab(MilSystem, MilDisplay, REFERENCE_IMAGE, CALIBRATION_IMAGE, GRABBED_IMAGE);
         
   /* Free objects. */
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);

   return 0;
   }

/* Data structure to be passed to the hook function. */
struct HookData
   {
   MIL_ID      MilSystem;
   MIL_ID      MilDisplay;
   MIL_ID      MilGraphicList;
   MIL_ID      DisplayBuffer;
   MIL_ID      SrcImage;
   MIL_ID      ResultingImage;
   MIL_ID      DistanceImage;
   MIL_ID      MaskImage;
   MIL_ID      FilteredImage;
   MIL_INT     SrcSizeX;
   MIL_INT     SrcSizeY;   
   MIL_DOUBLE* Threshold;
   };

/* This function segments an image based on a threshold and a distance image. */
void UpdateFilteredImage(MIL_ID MilDisplay,
                         MIL_ID MilGraphicList,
                         MIL_ID SrcImage, 
                         MIL_ID DistanceImage, 
                         MIL_ID Mask, 
                         MIL_ID Destination, 
                         MIL_DOUBLE Threshold,
                         MIL_INT AnnotationPosY)
   {
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX];

   MimBinarize(DistanceImage, Mask, M_LESS_OR_EQUAL, Threshold, M_NULL);
   MimArith(SrcImage, Mask, Destination, M_AND);

   /* Identify images. */
   MgraText(M_DEFAULT,Destination,0,0,MIL_TEXT("Segmented RGB image"));
   MgraText(M_DEFAULT,SrcImage,0,0,MIL_TEXT("RGB image"));
      
   /* Display threshold values on the display. */
   MosSprintf(Text,
              STRING_LENGTH_MAX, 
              MIL_TEXT("(CIE76) Delta E threshold: %f"), Threshold);
   MgraText(M_DEFAULT,MilGraphicList,10,AnnotationPosY,Text);
   MosPrintf(MIL_TEXT("%s. \r"), Text);

   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   }

/* This function is called when a keyboard event is trapped to allow the threshold value to be updated. */
MIL_INT MFTYPE TrapKeyboard( MIL_INT HookType, MIL_ID EventId, void *UserDataPtr) 
   {
   HookData* HookDataPtr  = (HookData*)UserDataPtr;
   MIL_ID  MilSystem      = HookDataPtr->MilSystem;
   MIL_ID  MilDisplay     = HookDataPtr->MilDisplay;
   MIL_ID  MilGraphicList = HookDataPtr->MilGraphicList;
   MIL_ID  DisplayBuffer  = HookDataPtr->DisplayBuffer;
   MIL_ID  SrcImage       = HookDataPtr->SrcImage;
   MIL_ID  LabImage       = HookDataPtr->ResultingImage;
   MIL_ID  DistanceImage  = HookDataPtr->DistanceImage;
   MIL_ID  MaskImage      = HookDataPtr->MaskImage;
   MIL_ID  FilteredImage  = HookDataPtr->FilteredImage;
   MIL_INT SrcSizeX       = HookDataPtr->SrcSizeX;
   MIL_INT SrcSizeY       = HookDataPtr->SrcSizeY;   
   MIL_DOUBLE& Threshold  = *(HookDataPtr->Threshold);

   MIL_INT KeyVal;
   MdispGetHookInfo(EventId, M_MIL_KEY_VALUE, &KeyVal);

   switch (KeyVal)
         {
         /* Right/up arrow: increase the threshold value. */
         case M_KEY_ARROW_RIGHT:
            { 
            Threshold += 0.5;
            break;
            }
         case M_KEY_ARROW_UP:
            { 
            Threshold += 5;             
            break; 
            }

         /* Left/down arrow: decrease the threshold value. */
         case M_KEY_ARROW_LEFT:
            {
            if (Threshold > 0)
               Threshold -= 0.5; 
            break; 
            }
         case M_KEY_ARROW_DOWN:
            {
            if (Threshold >= 5)
               Threshold -= 5; 
            else
               Threshold = 0;
            break; 
            }
         }

   /* Update segmentation based on new threshold. */
   UpdateFilteredImage(MilDisplay, MilGraphicList, SrcImage, DistanceImage, MaskImage,  FilteredImage, Threshold, SrcSizeY);

   return 0;
   }

/* This function is called when a mouse event is trapped to allow the reference color to be changed. */
MIL_INT MFTYPE SelectColor(MIL_INT HookType, MIL_ID EventId, void *UserDataPtr) 
   {
   MIL_DOUBLE PosX, PosY;

   HookData* HookDataPtr  = (HookData*)UserDataPtr;
   MIL_ID  MilSystem      = HookDataPtr->MilSystem;
   MIL_ID  MilDisplay     = HookDataPtr->MilDisplay;
   MIL_ID  MilGraphicList = HookDataPtr->MilGraphicList;
   MIL_ID  DisplayBuffer  = HookDataPtr->DisplayBuffer;
   MIL_ID  SrcImage       = HookDataPtr->SrcImage;
   MIL_ID  LabImage       = HookDataPtr->ResultingImage;
   MIL_ID  DistanceImage  = HookDataPtr->DistanceImage;
   MIL_ID  MaskImage      = HookDataPtr->MaskImage;
   MIL_ID  FilteredImage  = HookDataPtr->FilteredImage;
   MIL_INT SrcSizeX       = HookDataPtr->SrcSizeX;
   MIL_INT SrcSizeY       = HookDataPtr->SrcSizeY;   
   MIL_DOUBLE Threshold   = *(HookDataPtr->Threshold);

   /* Allocate a statistics tool and child. */
   MIL_ID MilStatChild   = MbufChild2d(LabImage, 0, 0, COLOR_PICKER_SIZE, COLOR_PICKER_SIZE, M_NULL);
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilStatResult  = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);
   MimControl(MilStatContext, M_STAT_MEAN, M_ENABLE);

   MIL_INT Type    = MbufInquire(DisplayBuffer, M_TYPE, M_NULL);

   /* If this is an event, inquire the mouse position in the displayed buffer. */
   if (EventId != M_NULL)
      {
      MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_X, &PosX);
      MdispGetHookInfo(EventId, M_MOUSE_POSITION_BUFFER_Y, &PosY);
      }
   else
      {
      /* Otherwise, use the default position. */
      PosX = INIT_COLOR_PICKER_POSX;
      PosY = INIT_COLOR_PICKER_POSY;
      }
   
   /* Verify that the selected pixel is within the Lab image. */
   if ((PosX < SrcSizeX) && (PosY < SrcSizeY))
      {
      MIL_INT AverageColor = 0;

      /* Allocate a temporary array. */
      MIL_ID ArrayId = MbufAlloc1d(M_DEFAULT_HOST, 3L, Type, M_ARRAY, M_NULL);

      /* Set the child at the picked location. */
      MbufChildMove(MilStatChild, 
                    (MIL_INT)(PosX - COLOR_PICKER_SIZE), 
                    (MIL_INT)(PosY - COLOR_PICKER_SIZE), 
                    2 * COLOR_PICKER_SIZE, 
                    2 * COLOR_PICKER_SIZE, 
                    M_CLIP);

      /* Calculate the average color for each component. */
      for (MIL_INT BandIndex = 0; BandIndex < 3; BandIndex++)
         {
         MIL_ID LabBand = MbufChildColor(MilStatChild, BandIndex, M_NULL);
         MimStatCalculate(MilStatContext, LabBand, MilStatResult, M_DEFAULT);
         MimGetResult(MilStatResult, M_STAT_MEAN + M_TYPE_MIL_INT, &AverageColor);
         MbufPut2d(ArrayId, BandIndex, 0L, 1L, 1L, &AverageColor);
         MbufFree(LabBand);
         }

      /* Compute the distance from the selected color to each pixel in the CIELab color space. */
      McolDistance(LabImage, ArrayId, DistanceImage, M_NULL, M_NULL, M_DELTA_E, M_NO_NORMALIZE, M_DEFAULT);

      /* Mark selected position. */
      MgraClear(M_DEFAULT, MilGraphicList);

      /* Update segmentation based on the new reference color. */
      UpdateFilteredImage(MilDisplay, MilGraphicList, SrcImage, DistanceImage, MaskImage, FilteredImage, Threshold, SrcSizeY);

      MgraRect(M_DEFAULT, MilGraphicList, 
               PosX - COLOR_PICKER_SIZE, 
               PosY - COLOR_PICKER_SIZE, 
               PosX + COLOR_PICKER_SIZE, 
               PosY + COLOR_PICKER_SIZE);
      
      /* Release allocated objects. */      
      MbufFree(ArrayId);
      }

   /* Release allocated objects. */
   MbufFree(MilStatChild);
   MimFree(MilStatContext);
   MimFree(MilStatResult);

   return 0;
   }


/*********************************************************/
/*  Show different projection results on a single graph. */
void ConvertToCIELab(MIL_ID MilSystem, 
                     MIL_ID MilDisplay, 
                     MIL_CONST_TEXT_PTR ReferenceFilename, 
                     MIL_CONST_TEXT_PTR ColorCalibrationGridFilename,
                     MIL_CONST_TEXT_PTR TargetFilename)
   {
   MIL_ID MilImage,                      /* Image buffer identifier.   */
          MilReferenceImage,             /* Image buffer identifier.   */   
          MilCalibrationImage,           /* Image buffer identifier.   */
          MilLabImage,                   /* Image buffer identifier.   */
          MaskImage,                     /* Image buffer identifier.   */
          DistanceImage,                 /* Image buffer identifier.   */
          MilGraphicList,                /* Graphic list identifier.   */
          MilSubImage00,                 /* Child buffer identifier.   */
          MilSubImage01,                 /* Child buffer identifier.   */
          ReferenceColorChild,           /* Child buffer identifier.   */
          CalibrationColorChild,         /* Child buffer identifier.   */
          MilColorCalibrationContext;    /* Color calibration context. */

   MIL_INT Type,SizeX, SizeY;
   
   /* Inquire the image Size and Type. */
   MbufDiskInquire(TargetFilename, M_SIZE_X, &SizeX);
   MbufDiskInquire(TargetFilename, M_SIZE_Y, &SizeY);
   MbufDiskInquire(TargetFilename, M_TYPE,   &Type);

   /* Allocate a display buffer and clear it. */
   MbufAllocColor(MilSystem, 3L, SizeX * 2, SizeY + 20,
      Type, M_IMAGE+M_PROC+M_DISP, &MilImage);
   MbufClear(MilImage, 0L);

   /* Allocate a CIELab buffer. */
   MbufAllocColor(MilSystem, 3L, SizeX, SizeY,
      Type, M_IMAGE+M_PROC+M_DISP, &MilLabImage);

   /* Allocate a buffer to compute color distance. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT, M_IMAGE + M_PROC, &DistanceImage);

   /* Allocate mask images. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8L, M_IMAGE + M_PROC, &MaskImage);

   /* Allocate reference image. */
   MbufRestore(ReferenceFilename, MilSystem, &MilReferenceImage);

   /* Allocate color calibration grid image. */
   MbufRestore(ColorCalibrationGridFilename, MilSystem, &MilCalibrationImage);
   
   MbufChild2d(MilReferenceImage,   0, 0, 1, 1, &ReferenceColorChild);
   MbufChild2d(MilCalibrationImage, 0, 0, 1, 1, &CalibrationColorChild);

   /* Display the image buffer and prepare for overlay annotations. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   MdispSelect (MilDisplay, MilImage);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   

   /* Allocate child buffers. */
   MbufChild2d(MilImage, 0L,    0L, SizeX, SizeY, &MilSubImage00);
   MbufChild2d(MilImage, SizeX, 0L, SizeX, SizeY, &MilSubImage01);

   MbufCopy(MilCalibrationImage, MilSubImage00);

   // Allocation of color relative calibration context
   McolAlloc(MilSystem, 
             M_COLOR_CALIBRATION_RELATIVE, 
             M_DEFAULT, 
             M_DEFAULT, 
             M_DEFAULT, 
             &MilColorCalibrationContext);

   /* Set color calibration method. */
   McolSetMethod(MilColorCalibrationContext,
                 M_COLOR_TO_COLOR,
                 M_PRECISION,
                 M_COMPUTE_ITEM_PIXELS,
                 M_DEFAULT);
   
   /* Define the calibration coordinates. */
   const MIL_INT NbSampleRows    = 4;
   const MIL_INT NbSampleColumns = 6;

   const MIL_INT ReferenceSampleOffset = 10;
   const MIL_INT ReferenceSampleSize   = 73;

   MIL_INT ReferenceOffsetX = ReferenceSampleOffset;
   MIL_INT ReferenceOffsetY = ReferenceSampleOffset;

   const MIL_INT CalibrationSampleOffset = 110;
   const MIL_INT CalibrationSampleSize   = 60;

   MIL_INT CalibrationOffsetX = 30;
   MIL_INT CalibrationOffsetY = 64;
   
   MIL_INT ColorSampleType = M_IMAGE;

   /* Define the samples. */
   for (MIL_INT samplerow = 0; samplerow < NbSampleRows; ++samplerow )
      {      
      for (MIL_INT samplecolumn = 0; samplecolumn < NbSampleColumns; ++samplecolumn )
         {
         MbufChildMove(ReferenceColorChild, 
                       ReferenceOffsetX, 
                       ReferenceOffsetY, 
                       ReferenceSampleSize, 
                       ReferenceSampleSize,
                       M_DEFAULT);

         MbufChildMove(CalibrationColorChild, 
                       CalibrationOffsetX, 
                       CalibrationOffsetY, 
                       CalibrationSampleSize, 
                       CalibrationSampleSize,
                       M_DEFAULT);

         McolDefine(MilColorCalibrationContext, ReferenceColorChild,   M_REFERENCE_SAMPLE, ColorSampleType, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         McolDefine(MilColorCalibrationContext, CalibrationColorChild,  M_SAMPLE_LABEL(1), ColorSampleType, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

         ReferenceOffsetX   += ReferenceSampleOffset   + ReferenceSampleSize;
         CalibrationOffsetX += CalibrationSampleOffset;
         ColorSampleType |= M_ADD_COLOR_TO_SAMPLE ;
         }
      ReferenceOffsetX    = ReferenceSampleOffset;
      CalibrationOffsetX  = 30;
      ReferenceOffsetY   += ReferenceSampleOffset + ReferenceSampleSize;
      CalibrationOffsetY += CalibrationSampleOffset;
      }

   /* Preprocess the color context. */
   McolPreprocess(MilColorCalibrationContext, M_DEFAULT);

   /* Transform color space to sRGB. */
   McolTransform(MilColorCalibrationContext, M_SAMPLE_LABEL(1), MilSubImage00, MilSubImage01, M_DEFAULT);

   MgraClear(M_DEFAULT, MilGraphicList);
   MgraText(M_DEFAULT, MilGraphicList,     0, 0, MIL_TEXT("RGB color calibration image"));
   MgraText(M_DEFAULT, MilGraphicList, SizeX, 0, MIL_TEXT("sRGB color calibrated image"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("Relative color calibration is used to determine the transformation\n")
             MIL_TEXT("from RBG to sRGB. The color grid is then converted to sRGB.\n"));
   MosPrintf(MIL_TEXT("Press enter to continue.\n\n"));
   MosGetch();

   /* Load a noisy image. */
   MbufLoad(TargetFilename, MilSubImage00);

   /* Transform color space to sRGB. */
   McolTransform(MilColorCalibrationContext, M_SAMPLE_LABEL(1), MilSubImage00, MilSubImage01, M_DEFAULT);

   MgraClear(M_DEFAULT, MilGraphicList);
   MgraText(M_DEFAULT, MilGraphicList,     0, 0, MIL_TEXT("RGB image"));
   MgraText(M_DEFAULT, MilGraphicList, SizeX, 0, MIL_TEXT("sRGB image"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("A new target image is grabbed.\n"));
   MosPrintf(MIL_TEXT("Relative color calibration is applied to convert the image to the sRGB\ncolor space.\n\n"));
   MosPrintf(MIL_TEXT("Press enter to continue.\n\n"));
   MosGetch();

   /* Transform color space to CIELab. */
   MimConvert(MilSubImage01, MilLabImage, M_SRGB_LINEAR_TO_LAB);

   /* Print a message. */
   MosPrintf(MIL_TEXT("The sRGB image is converted to the perceptually uniform CIELab color space.\n"));

   /* Print a message. */
   MosPrintf(MIL_TEXT("The source image is then segmented by selecting a color and a distance\n")
             MIL_TEXT("in the CIELab perceptual color space.\n\n"));  

   MosPrintf(MIL_TEXT("The threshold value is displayed at the bottom of the screen and can be\n")
             MIL_TEXT("modified using the following key bindings:\n"));
   MosPrintf(MIL_TEXT("\t-Up or right arrow to increase the threshold\n"));
   MosPrintf(MIL_TEXT("\t-Down or left arrow to decrease the threshold\n\n"));
   MosPrintf(MIL_TEXT("Note that you can pick a new reference color by clicking in the left panel.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));

   /* Define initial threshold. */
   MIL_DOUBLE Threshold = 10.0;

   MIL_INT Ch = 0;
   
   MgraClear(M_DEFAULT, MilGraphicList);
   HookData HookDataStruct;
 
   HookDataStruct.MilSystem      = MilSystem;
   HookDataStruct.MilDisplay     = MilDisplay;
   HookDataStruct.MilGraphicList = MilGraphicList;
   HookDataStruct.DisplayBuffer  = MilImage;
   HookDataStruct.ResultingImage = MilLabImage;
   HookDataStruct.DistanceImage  = DistanceImage;
   HookDataStruct.FilteredImage  = MilSubImage01;
   HookDataStruct.SrcImage       = MilSubImage00;
   HookDataStruct.MaskImage      = MaskImage;
   HookDataStruct.SrcSizeX       = SizeX;
   HookDataStruct.SrcSizeY       = SizeY;
   HookDataStruct.Threshold      = &Threshold;

   SelectColor(M_NULL, M_NULL, &HookDataStruct);

   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP, SelectColor, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_KEY_DOWN, TrapKeyboard, &HookDataStruct);

   while (Ch != '\r')
      {
      switch (Ch)
         {
         /* Right/up arrow: increase the threshold value. */
         case 0x48:
            {
            Threshold += 5;             
            break; 
            }
         case 0x4D:
            { 
            Threshold += 0.5;             
            break; 
            }

         /* Left/down arrow: decrease the threshold value. */
         case 0x50:
            { 
            if (Threshold >= 5)
               Threshold -= 5;
            else
               Threshold = 0;
            break; 
            }
         case 0x4B:
            { 
            if (Threshold > 0)
               Threshold -= 0.5; 
            break; 
            }
         }

      /* Update segmentation based on new threshold. */
      UpdateFilteredImage(MilDisplay, MilGraphicList, MilSubImage00, DistanceImage, MaskImage, MilSubImage01, Threshold, SizeY);

      /* Get next command. */
      Ch = MosGetch();
      }
   MosPrintf(MIL_TEXT("\n"));

   /* Unhook functions from display. */
   MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, SelectColor, &HookDataStruct);
   MdispHookFunction(MilDisplay, M_KEY_DOWN + M_UNHOOK, TrapKeyboard, &HookDataStruct);

   /* Free identifiers. */
   MgraFree(MilGraphicList);
   McolFree(MilColorCalibrationContext);
   MbufFree(CalibrationColorChild);
   MbufFree(ReferenceColorChild);  
   MbufFree(MilSubImage00);
   MbufFree(MilSubImage01);
   MbufFree(DistanceImage);
   MbufFree(MaskImage);
   MbufFree(MilLabImage);
   MbufFree(MilCalibrationImage);
   MbufFree(MilReferenceImage);
   MbufFree(MilImage);
   }
