﻿/*************************************************************************************/
/* 
 * File name: DxfModelFinding.cpp
 * 
 * Synopsis:  This example shows how to match a CAD model to a target image 
 *            using Model Finder.
 *            In the first example, a calibration context is used to convert the model 
 *            from world units to pixel units.
 *            In the second example, the M_PIXEL_SCALE control is used for this conversion. 
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Example function declarations. */
void DxfWithCalibrationContextExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void DxfWithPixelScaleExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* DXF file used to define the model. */
#define DXF_FILE_MODEL_PATH M_IMAGE_PATH MIL_TEXT("DxfModelFinding/Model.dxf")

/* Target image. */
#define TARGET_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("SingleTarget.mim")

/*****************************************************************************/
/* Example description.                                        
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("DxfModelFinding\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to match a CAD model to a target image\n"));
   MosPrintf(MIL_TEXT("using Model Finder.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Display, Graphics, Model Finder, Calibration.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

/*****************************************************************************/
/* Main. 
/*****************************************************************************/
int MosMain(void)
   {
   /* Print example information in console. */
   PrintHeader();

   MIL_ID MilApplication,     /* Application identifier. */
          MilSystem,          /* System Identifier.      */
          MilDisplay;         /* Display identifier.     */

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Run first example. */
   DxfWithCalibrationContextExample(MilSystem, MilDisplay);

   /* Run second example. */
   DxfWithPixelScaleExample(MilSystem, MilDisplay);

   /* Free defaults. */    
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

/*****************************************************************************/
/* Example 1: Model defined using a DXF file and a calibration context.*/

void DxfWithCalibrationContextExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilTargetImage,                        /* Target image buffer identifier. */
               ModelFinderContext,                    /* Model Finder context.           */
               ModelFinderResult,                     /* Result identifier.              */
               CalibrationContext,                    /* Calibration context identifier. */
               GraphicList;                           /* Graphic list identifier.        */
   MIL_DOUBLE  ModelDrawColor       = M_COLOR_RED,    /* Model draw color.               */
               CalibrationDrawColor = M_COLOR_CYAN,   /* Calibration draw color.         */
               PositionDrawColor    = M_COLOR_BLUE;   /* Position draw color.            */

   /* Load target image. */
   MbufImport(TARGET_IMAGE_PATH, M_DEFAULT, M_RESTORE + M_NO_GRAB + M_NO_COMPRESS, MilSystem, 
              &MilTargetImage);
   MdispSelect(MilDisplay, MilTargetImage);

   /* Define model for Model Finder context from a DXF file. */
   MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &ModelFinderContext);
   MmodDefineFromFile(ModelFinderContext, M_DXF_FILE, DXF_FILE_MODEL_PATH, M_DEFAULT);

   /* Allocate a result buffer. */
   MmodAllocResult(MilSystem, M_DEFAULT, &ModelFinderResult);

   /* A calibration context could be restored using McalRestore or could be established using 
      McalGrid, McalList or McalUniform. Here, a uniform calibration is used. */
   McalAlloc(MilSystem, M_UNIFORM_TRANSFORMATION, M_DEFAULT, &CalibrationContext);
   McalUniform(CalibrationContext, 0.0, 0.0, 0.75, 0.75, 0.0, M_DEFAULT);

   /* Associate the calibration to the model and the target image. */
   McalAssociate(CalibrationContext, MilTargetImage, M_DEFAULT);
   MmodControl(ModelFinderContext, 0, M_ASSOCIATED_CALIBRATION, CalibrationContext);

   /* Preprocess the search context and find the target. */
   MmodPreprocess(ModelFinderContext, M_DEFAULT);
   MmodFind(ModelFinderContext, MilTargetImage, ModelFinderResult);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Draw results */
   MgraColor(M_DEFAULT, ModelDrawColor);
   MmodDraw(M_DEFAULT, ModelFinderResult, GraphicList,
            M_DRAW_EDGES + M_DRAW_BOX, 0, M_DEFAULT);
   MgraColor(M_DEFAULT, PositionDrawColor);
   MmodDraw(M_DEFAULT, ModelFinderResult, GraphicList,
            M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
   MgraColor(M_DEFAULT, CalibrationDrawColor);
   McalDraw(M_DEFAULT, CalibrationContext, GraphicList,
            M_DRAW_ABSOLUTE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

   /* Pause to show the model. */
   MosPrintf(MIL_TEXT("Solution 1:\n"));
   MosPrintf(MIL_TEXT("----------\n"));
   MosPrintf(MIL_TEXT("A calibration context is used to map the target\n"));
   MosPrintf(MIL_TEXT("image to the world system of the CAD model.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MbufFree(MilTargetImage);
   MmodFree(ModelFinderContext);
   MmodFree(ModelFinderResult);
   McalFree(CalibrationContext);
   MgraFree(GraphicList);
   }

/*****************************************************************************/
/* Example 2: Model defined using a DXF file and the control M_PIXEL_SCALE.  */

void DxfWithPixelScaleExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilTargetImage,                     /* Target image buffer identifier. */
               ModelFinderContext,                 /* Model Finder context.           */
               ModelFinderResult,                  /* Result identifier.              */
               GraphicList;                        /* Graphic list identifier.        */
   MIL_DOUBLE  ModelDrawColor    = M_COLOR_RED,    /* Model draw color.               */
               PositionDrawColor = M_COLOR_BLUE;   /* Position draw color.            */

   /* Load target image. */
   MbufImport(TARGET_IMAGE_PATH, M_DEFAULT, M_RESTORE + M_NO_GRAB + M_NO_COMPRESS, MilSystem,
              &MilTargetImage);
   MdispSelect(MilDisplay, MilTargetImage);

   /* Define model for Model Finder context from a DXF file. */
   MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &ModelFinderContext);
   MmodDefineFromFile(ModelFinderContext, M_DXF_FILE, DXF_FILE_MODEL_PATH, M_DEFAULT);

   /* Allocate a result buffer. */
   MmodAllocResult(MilSystem, M_DEFAULT, &ModelFinderResult);

   /* Set pixel scale value. */
   MmodControl(ModelFinderContext, 0, M_PIXEL_SCALE, 1.33);

   /* Preprocess the search context and find the target. */
   MmodPreprocess(ModelFinderContext, M_DEFAULT);
   MmodFind(ModelFinderContext, MilTargetImage, ModelFinderResult);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Draw results */
   MgraColor(M_DEFAULT, ModelDrawColor);
   MmodDraw(M_DEFAULT, ModelFinderResult, GraphicList,
            M_DRAW_EDGES + M_DRAW_BOX, 0, M_DEFAULT);
   MgraColor(M_DEFAULT, PositionDrawColor);
   MmodDraw(M_DEFAULT, ModelFinderResult, GraphicList,
            M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);

   /* Pause to show the model. */
   MosPrintf(MIL_TEXT("Solution 2:\n"));
   MosPrintf(MIL_TEXT("----------\n"));
   MosPrintf(MIL_TEXT("If the mapping between the CAD model and the target image is uniform,\n"));
   MosPrintf(MIL_TEXT("the control M_PIXEL_SCALE can be used to specify the pixel to world ratio\n"));
   MosPrintf(MIL_TEXT("without the need of a calibration context.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free MIL objects. */
   MbufFree(MilTargetImage);
   MmodFree(ModelFinderContext);
   MmodFree(ModelFinderResult);
   MgraFree(GraphicList);
   }
