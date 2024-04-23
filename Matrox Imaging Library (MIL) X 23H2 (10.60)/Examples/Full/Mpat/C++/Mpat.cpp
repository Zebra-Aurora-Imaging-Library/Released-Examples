﻿/***************************************************************************************/
/*
* File name: MPat.cpp
*
* Synopsis:  This program contains 4 examples of the pattern matching module:
*
*            The first example defines a model and then searches for it in a shifted
*            version of the image (without rotation).
*
*            The second example defines a regular model and then searches for it in a
*            rotated version of the image using search angle range.
*
*            The third example defines a rotated model at certain angle and then
*            searches for it in a rotated version of the image.
*
*            The fourth example automatically allocates a model in a wafer image and 
*            finds its horizontal and vertical displacement.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <math.h>

/* Example functions declarations. */
void SearchModelExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void SearchModelAngleRangeExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void SearchModelAtAngleExample(MIL_ID MilSystem, MIL_ID MilDisplay);
void AutoAllocationModelExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/*****************************************************************************
 Main.
 *****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,     /* Application identifier. */
          MilSystem,          /* System identifier.      */
          MilDisplay;         /* Display identifier.     */

   MosPrintf(MIL_TEXT("\nGRAYSCALE PATTERN MATCHING:\n"));
   MosPrintf(MIL_TEXT("---------------------------\n\n"));

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Run the search at 0 degree example. */
   SearchModelExample(MilSystem, MilDisplay);

   /* Run the search over 360 degrees example. */
   SearchModelAngleRangeExample(MilSystem, MilDisplay);

   /* Run the search rotated model example. */
   SearchModelAtAngleExample(MilSystem, MilDisplay);

   /* Run the automatic model allocation example. */
   AutoAllocationModelExample(MilSystem, MilDisplay);

   /* Free defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }


/*****************************************************************************/
/* Find model in shifted version of the image example. */

/* Source image file name. */
#define FIND_IMAGE_FILE    M_IMAGE_PATH MIL_TEXT("CircuitsBoard.mim")

/* Model position and size. */
#define FIND_MODEL_X_POS   153L
#define FIND_MODEL_Y_POS   132L
#define FIND_MODEL_WIDTH   128L
#define FIND_MODEL_HEIGHT  128L
#define FIND_MODEL_X_CENTER   (FIND_MODEL_X_POS+(FIND_MODEL_WIDTH -1)/2.0)
#define FIND_MODEL_Y_CENTER   (FIND_MODEL_Y_POS+(FIND_MODEL_HEIGHT-1)/2.0)

/* Target image shifting values. */
#define FIND_SHIFT_X        4.5
#define FIND_SHIFT_Y        7.5

/* Minimum match score to determine acceptability of model (default). */
#define FIND_MODEL_MIN_MATCH_SCORE 70.0

/* Minimum accuracy for the search. */
#define FIND_MODEL_MIN_ACCURACY    0.1

/* Absolute value macro. */
#define AbsoluteValue(x) (((x) < 0.0) ? -(x) : (x))

void SearchModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage,                /* Image buffer identifier.  */
              GraphicList,             /* Graphic list identifier.  */
              ContextId,               /* ContextId identifier.     */
              Result;                  /* Result identifier.        */
   MIL_INT    NumResults;              /* Number of results found   */
   MIL_DOUBLE XOrg = 0.0, YOrg = 0.0;  /* Original model position.  */
   MIL_DOUBLE x    = 0.0, y    = 0.0;  /* Model position.           */
   MIL_DOUBLE ErrX = 0.0, ErrY = 0.0;  /* Model error position.     */
   MIL_DOUBLE Score = 0.0;             /* Model correlation score.  */
   MIL_DOUBLE Time  = 0.0;             /* Model search time.        */
   MIL_DOUBLE AnnotationColor = M_COLOR_GREEN; /* Drawing color.    */

   /* Restore source image into an automatically allocated image buffer. */
   MbufRestore(FIND_IMAGE_FILE, MilSystem, &MilImage);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a normalized pattern matching context. */
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &ContextId);

   /* Define a regular model. */
   MpatDefine(ContextId, M_REGULAR_MODEL, MilImage, FIND_MODEL_X_POS,
              FIND_MODEL_Y_POS, FIND_MODEL_WIDTH, FIND_MODEL_HEIGHT, M_DEFAULT);

   /* Set the search accuracy to high. */
   MpatControl(ContextId, M_DEFAULT, M_ACCURACY, M_HIGH);

   /* Set the search model speed to high. */
   MpatControl(ContextId, M_DEFAULT, M_SPEED, M_HIGH);

   /* Preprocess the model. */
   MpatPreprocess(ContextId, M_DEFAULT, MilImage);

   /* Draw a box around the model in the model image. */
   MgraColor(M_DEFAULT, AnnotationColor);
   MpatDraw(M_DEFAULT, ContextId, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
                                                                  M_DEFAULT, M_ORIGINAL);
   /* Pause to show the original image and model position. */
   MosPrintf(MIL_TEXT("\nA %ldx%ld model was defined in the source image.\n"),
                             FIND_MODEL_WIDTH, FIND_MODEL_HEIGHT);
   MosPrintf(MIL_TEXT("It will be found in an image shifted ")
             MIL_TEXT("by %.2f in X and %.2f in Y.\n"), FIND_SHIFT_X, FIND_SHIFT_Y);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Clear annotations. */
   MgraClear(M_DEFAULT, GraphicList);

   /* Translate the image on a subpixel level. */
   MimTranslate(MilImage, MilImage, FIND_SHIFT_X, FIND_SHIFT_Y, M_DEFAULT);

   /* Allocate result buffer. */
   MpatAllocResult(MilSystem, M_DEFAULT, &Result);

   /* Dummy first call for bench measure purpose only (bench stabilization,
      cache effect, etc...). This first call is NOT required by the application. */
   MpatFind(ContextId, MilImage, Result);
   MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

   /* Find the model in the target buffer. */
   MpatFind(ContextId, MilImage, Result);

   /* Read the time spent in MpatFind. */
   MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

   /* If one model was found above the acceptance threshold. */
   MpatGetResult(Result, M_GENERAL, M_NUMBER+M_TYPE_MIL_INT, &NumResults);
   if(NumResults == 1L)
      {
      /* Read results and draw a box around the model occurrence. */
      MpatGetResult(Result, M_DEFAULT, M_POSITION_X, &x);
      MpatGetResult(Result, M_DEFAULT, M_POSITION_Y, &y);
      MpatGetResult(Result, M_DEFAULT, M_SCORE, &Score);
      MgraColor(M_DEFAULT, AnnotationColor);
      MpatDraw(M_DEFAULT, Result, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
                                                              M_DEFAULT, M_DEFAULT);

      /* Calculate the position errors in X and Y and inquire original model position. */
      ErrX = fabs((FIND_MODEL_X_CENTER + FIND_SHIFT_X) - x);
      ErrY = fabs((FIND_MODEL_Y_CENTER + FIND_SHIFT_Y) - y);
      MpatInquire(ContextId, M_DEFAULT, M_ORIGINAL_X, &XOrg);
      MpatInquire(ContextId, M_DEFAULT, M_ORIGINAL_Y, &YOrg);

      /* Print out the search result of the model in the original image. */
      MosPrintf(MIL_TEXT("Search results:\n"));
      MosPrintf(MIL_TEXT("---------------------------------------------------\n"));
      MosPrintf(MIL_TEXT("The model is found to be shifted by \tX:%.2f, Y:%.2f.\n"),
                                                                    x - XOrg, y - YOrg);
      MosPrintf(MIL_TEXT("The model position error is \t\tX:%.2f, Y:%.2f\n"),
                                                                 ErrX, ErrY);
      MosPrintf(MIL_TEXT("The model match score is \t\t%.1f\n"), Score);
      MosPrintf(MIL_TEXT("The search time is \t\t\t%.3f ms\n\n"), Time*1000.0);

      /* Verify the results. */
      if (
          (AbsoluteValue((x - XOrg) - FIND_SHIFT_X) > FIND_MODEL_MIN_ACCURACY) ||
          (AbsoluteValue((y - YOrg) - FIND_SHIFT_Y) > FIND_MODEL_MIN_ACCURACY) ||
          (Score                              < FIND_MODEL_MIN_MATCH_SCORE)
         )
         MosPrintf(MIL_TEXT("Results verification error !\n"));
      }
   else
      MosPrintf(MIL_TEXT("Model not found !\n"));

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Clear annotations. */
   MgraClear(M_DEFAULT, GraphicList);

   /* Free all allocations. */
   MgraFree(GraphicList);
   MpatFree(Result);
   MpatFree(ContextId);
   MbufFree(MilImage);
   }


/*****************************************************************************/
/* Find rotated model example. */

/* Source image file name. */
#define ROTATED_FIND_IMAGE_FILE      M_IMAGE_PATH MIL_TEXT("CircuitsBoard.mim")

/* Image rotation values. */
#define ROTATED_FIND_ROTATION_DELTA_ANGLE  10
#define ROTATED_FIND_ROTATION_ANGLE_STEP   1
#define ROTATED_FIND_RAD_PER_DEG           0.01745329251

/* Model position and size. */
#define ROTATED_FIND_MODEL_X_POS           153L
#define ROTATED_FIND_MODEL_Y_POS           132L
#define ROTATED_FIND_MODEL_WIDTH           128L
#define ROTATED_FIND_MODEL_HEIGHT          128L

#define ROTATED_FIND_MODEL_X_CENTER           ROTATED_FIND_MODEL_X_POS+ \
                                             (ROTATED_FIND_MODEL_WIDTH -1)/2.0
#define ROTATED_FIND_MODEL_Y_CENTER           ROTATED_FIND_MODEL_Y_POS+ \
                                             (ROTATED_FIND_MODEL_HEIGHT-1)/2.0


/* Minimum accuracy for the search position. */
#define ROTATED_FIND_MIN_POSITION_ACCURACY     0.10

/* Minimum accuracy for the search angle. */
#define ROTATED_FIND_MIN_ANGLE_ACCURACY        0.25

/* Angle range to search. */
#define ROTATED_FIND_ANGLE_DELTA_POS           ROTATED_FIND_ROTATION_DELTA_ANGLE
#define ROTATED_FIND_ANGLE_DELTA_NEG           ROTATED_FIND_ROTATION_DELTA_ANGLE

/* Prototypes of utility functions. */
void RotateModelCenter(MIL_ID Buffer,
                       MIL_DOUBLE *X,
                       MIL_DOUBLE *Y,
                       MIL_DOUBLE Angle);

MIL_DOUBLE CalculateAngleDist(MIL_DOUBLE Angle1,
                              MIL_DOUBLE Angle2);


void SearchModelAngleRangeExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilSourceImage,              /* Model  image buffer identifier.   */
               MilTargetImage,              /* Target image buffer identifier.   */
               MilDisplayImage,             /* Target image buffer identifier.   */
               GraphicList,                 /* Graphic list.                     */
               MilContextId,                /* Model identifier.                 */
               MilResult;                   /* Result identifier.                */
   MIL_DOUBLE  RealX       = 0.0,           /* Model real position in x.         */
               RealY       = 0.0,           /* Model real position in y.         */
               RealAngle   = 0.0,           /* Model real angle.                 */
               X           = 0.0,           /* Model position in x found.        */
               Y           = 0.0,           /* Model position in y found.        */
               Angle       = 0.0,           /* Model angle found.                */
               Score       = 0.0,           /* Model correlation score.          */
               Time        = 0.0,           /* Model search time.                */
               ErrX        = 0.0,           /* Model error position in x.        */
               ErrY        = 0.0,           /* Model error position in y.        */
               ErrAngle    = 0.0,           /* Model error angle.                */
               SumErrX     = 0.0,           /* Model total error position in x.  */
               SumErrY      = 0.0,          /* Model total error position in y.  */
               SumErrAngle  = 0.0,          /* Model total error angle.          */
               SumTime      = 0.0;          /* Model total search time.          */
   MIL_INT     NumResults;                  /* Number of results found           */
   MIL_INT     NbFound      = 0;            /* Number of models found.           */
   MIL_DOUBLE  AnnotationColor = M_COLOR_GREEN; /* Drawing color.                */

   /* Load target image into image buffers and display it. */
   MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, &MilSourceImage);
   MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, &MilTargetImage);
   MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, &MilDisplayImage);
   MdispSelect(MilDisplay, MilDisplayImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a normalized pattern matching context. */
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &MilContextId);

   /* Define a regular model. */
   MpatDefine(MilContextId, M_REGULAR_MODEL + M_CIRCULAR_OVERSCAN, MilSourceImage,
              ROTATED_FIND_MODEL_X_POS, ROTATED_FIND_MODEL_Y_POS,
              ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT, M_DEFAULT);

   /* Set the search model speed. */
   MpatControl(MilContextId, M_DEFAULT, M_SPEED, M_MEDIUM);

   /* Set the position search accuracy. */
   MpatControl(MilContextId, M_DEFAULT, M_ACCURACY, M_HIGH);

   /* Activate the search model angle mode. */
   MpatControl(MilContextId, M_DEFAULT, M_SEARCH_ANGLE_MODE, M_ENABLE);

   /* Set the search model range angle. */
   MpatControl(MilContextId, M_DEFAULT, M_SEARCH_ANGLE_DELTA_NEG, ROTATED_FIND_ANGLE_DELTA_NEG);
   MpatControl(MilContextId, M_DEFAULT, M_SEARCH_ANGLE_DELTA_POS, ROTATED_FIND_ANGLE_DELTA_POS);

   /* Set the search model angle accuracy. */
   MpatControl(MilContextId, M_DEFAULT, M_SEARCH_ANGLE_ACCURACY, ROTATED_FIND_MIN_ANGLE_ACCURACY);

   /* Set the search model angle interpolation mode to bilinear. */
   MpatControl(MilContextId, M_DEFAULT, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);

   /* Preprocess the model. */
   MpatPreprocess(MilContextId, M_DEFAULT, MilSourceImage);

   /* Allocate a result buffer. */
   MpatAllocResult(MilSystem, M_DEFAULT, &MilResult);

   /* Draw the original model position. */
   MpatDraw(M_DEFAULT, MilContextId, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
                                                          M_DEFAULT, M_ORIGINAL);

   /* Pause to show the original image and model position. */
   MosPrintf(MIL_TEXT("\nA %ldx%ld model was defined in the source image.\n"),
                          ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT);
   MosPrintf(MIL_TEXT("It will be searched in images rotated from %d degree ")
             MIL_TEXT("to %d degree.\n"), -ROTATED_FIND_ROTATION_DELTA_ANGLE,
                                            ROTATED_FIND_ROTATION_DELTA_ANGLE);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Dummy first call for bench measure purpose only (bench stabilization,
   cache effect, etc...). This first call is NOT required by the application. */
   MpatFind(MilContextId, MilSourceImage, MilResult);

   /* If the model was found above the acceptance threshold. */
   MpatGetResult(MilResult, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT, &NumResults);
   if(NumResults == 1L)
      {
      /* Search for the model in images at different angles. */
      RealAngle = ROTATED_FIND_ROTATION_DELTA_ANGLE;
      while(RealAngle >= -ROTATED_FIND_ROTATION_DELTA_ANGLE)
         {
         /* Rotate the image from the model image to target image. */
         MimRotate(MilSourceImage, MilTargetImage, RealAngle, M_DEFAULT,
                   M_DEFAULT, M_DEFAULT, M_DEFAULT, M_BILINEAR + M_OVERSCAN_CLEAR);

         /* Reset the timer. */
         MappTimer(M_DEFAULT, M_TIMER_RESET + M_SYNCHRONOUS, M_NULL);

         /* Find the model in the target image. */
         MpatFind(MilContextId, MilTargetImage, MilResult);
         
         /* Read the time spent in MpatFind. */
         MappTimer(M_DEFAULT, M_TIMER_READ + M_SYNCHRONOUS, &Time);

         /* Clear the annotations. */
         MgraClear(M_DEFAULT, GraphicList);

         /* If one model was found above the acceptance threshold. */
         MpatGetResult(MilResult, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT, &NumResults);
         if(NumResults == 1L)
            {
            /* Read results and draw a box around the model occurrence. */
            MpatGetResult(MilResult, M_DEFAULT, M_POSITION_X, &X);
            MpatGetResult(MilResult, M_DEFAULT, M_POSITION_Y, &Y);
            MpatGetResult(MilResult, M_DEFAULT, M_ANGLE, &Angle);
            MpatGetResult(MilResult, M_DEFAULT, M_SCORE, &Score);

            MgraColor(M_DEFAULT, AnnotationColor);
            MpatDraw(M_DEFAULT, MilResult, GraphicList,
                     M_DRAW_BOX + M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);

            MbufCopy(MilTargetImage, MilDisplayImage);

            /* Calculate the angle error and the position errors for statistics. */
            ErrAngle = CalculateAngleDist(Angle, RealAngle);

            RotateModelCenter(MilSourceImage, &RealX, &RealY, RealAngle);
            ErrX = fabs(X - RealX);
            ErrY = fabs(Y - RealY);

            SumErrAngle += ErrAngle;
            SumErrX += ErrX;
            SumErrY += ErrY;
            SumTime += Time;
            NbFound++;

            /* Verify the precision for the position and the angle. */
            if ((ErrX > ROTATED_FIND_MIN_POSITION_ACCURACY) ||
                (ErrY > ROTATED_FIND_MIN_POSITION_ACCURACY) ||
                (ErrAngle > ROTATED_FIND_MIN_ANGLE_ACCURACY))
               {
               MosPrintf(MIL_TEXT("Model accuracy error at angle %.1f !\n\n"), RealAngle);
               MosPrintf(MIL_TEXT("Errors are X:%.3f, Y:%.3f and Angle:%.2f\n\n"),
                                                           ErrX, ErrY, ErrAngle);
               MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
               MosGetch();
               }
            }
         else
            {
            MosPrintf(MIL_TEXT("Model was not found at angle %.1f !\n\n"), RealAngle);
            MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();
            }

         RealAngle -= ROTATED_FIND_ROTATION_ANGLE_STEP;
         }

      /* Print out the search result statistics */
      /* of the models found in rotated images. */
      MosPrintf(MIL_TEXT("\nSearch statistics for the model ")
                MIL_TEXT("found in the rotated images.\n"));
      MosPrintf(MIL_TEXT("------------------------------")
                MIL_TEXT("------------------------------\n"));
      MosPrintf(MIL_TEXT("The average position error is \t\tX:%.3f, Y:%.3f\n"),
                                                      SumErrX / NbFound, SumErrY / NbFound);
      MosPrintf(MIL_TEXT("The average angle error is \t\t%.3f\n"), SumErrAngle / NbFound);
      MosPrintf(MIL_TEXT("The average search time is \t\t%.3f ms\n\n"),
                                                          SumTime*1000.0 / NbFound);
      }
   else
      {
      MosPrintf(MIL_TEXT("Model was not found!\n\n"));
      }

   /* Wait for a key to be pressed. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MgraFree(GraphicList);
   MpatFree(MilResult);
   MpatFree(MilContextId);
   MbufFree(MilTargetImage);
   MbufFree(MilSourceImage);
   MbufFree(MilDisplayImage);
   }


/* Calculate the rotated center of the model to compare the accuracy with
*  the center of the occurrence found during pattern matching.
*/
void RotateModelCenter(MIL_ID Buffer,
                       MIL_DOUBLE *X,
                       MIL_DOUBLE *Y,
                       MIL_DOUBLE Angle)
   {
   MIL_INT BufSizeX = MbufInquire(Buffer, M_SIZE_X, M_NULL);
   MIL_INT BufSizeY = MbufInquire(Buffer, M_SIZE_Y, M_NULL);
   MIL_DOUBLE RadAngle = Angle * ROTATED_FIND_RAD_PER_DEG;
   MIL_DOUBLE CosAngle = cos(RadAngle);
   MIL_DOUBLE SinAngle = sin(RadAngle);

   MIL_DOUBLE OffSetX = (BufSizeX - 1) / 2.0F;
   MIL_DOUBLE OffSetY = (BufSizeY - 1) / 2.0F;

   *X = (ROTATED_FIND_MODEL_X_CENTER - OffSetX)*CosAngle +
        (ROTATED_FIND_MODEL_Y_CENTER - OffSetY)*SinAngle + OffSetX;
   *Y = (ROTATED_FIND_MODEL_Y_CENTER - OffSetY)*CosAngle -
        (ROTATED_FIND_MODEL_X_CENTER - OffSetX)*SinAngle + OffSetY;
   }


/* Calculate the absolute difference between the real angle
 * and the angle found.
 */
MIL_DOUBLE CalculateAngleDist(MIL_DOUBLE Angle1, MIL_DOUBLE Angle2)
   {
   MIL_DOUBLE dist = fabs(Angle1 - Angle2);

   while(dist >= 360.0)
      dist -= 360.0;

   if(dist > 180.0)
      dist = 360.0 - dist;

   return dist;
   }

/******************************************************/
/* Find the rotated model in a rotated image example. */
/******************************************************/
void SearchModelAtAngleExample(MIL_ID MilSystem, MIL_ID MilDisplay)
{
   MIL_ID       MilSourceImage,                    /* Model image buffer identifier.    */
                MilTargetImage,                    /* Target image buffer identifier.   */
                ContextId,                         /* Context identifier.               */
                GraphicList,                       /* Graphic list.                     */
                MilResult;                         /* Result identifier.                */
   MIL_DOUBLE   Time        = 0.0;                 /* Model search time.                */
   MIL_INT      NbFound     = 0;                   /* Number of models found.           */
   MIL_DOUBLE   AnnotationColor = M_COLOR_GREEN;   /* Drawing color.                    */

   /* Load the source image and display it. */
   MbufRestore(ROTATED_FIND_IMAGE_FILE, MilSystem, &MilSourceImage);
   MdispSelect(MilDisplay, MilSourceImage);

   /* Allocate the target image. */
   MbufAlloc2d(MilSystem, MbufInquire(MilSourceImage, M_SIZE_X, M_NULL),
      MbufInquire(MilSourceImage, M_SIZE_Y, M_NULL), 
      8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MilTargetImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Allocate a normalized grayscale model. */
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &ContextId);

   /* Define a regular model. */
   MpatDefine(ContextId, M_REGULAR_MODEL, MilSourceImage,
              ROTATED_FIND_MODEL_X_POS, ROTATED_FIND_MODEL_Y_POS,
              ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT, M_DEFAULT);

   /* Activate the search model angle mode. */
   MpatControl(ContextId, M_DEFAULT, M_SEARCH_ANGLE_MODE, M_ENABLE);

   /* Disable the search model range angle. */
   MpatControl(ContextId, M_DEFAULT, M_SEARCH_ANGLE_DELTA_NEG, 0);
   MpatControl(ContextId, M_DEFAULT, M_SEARCH_ANGLE_DELTA_POS, 0);

   /* Set a specific angle. */
   MpatControl(ContextId, 0, M_SEARCH_ANGLE, ROTATED_FIND_ROTATION_DELTA_ANGLE);

   /* Preprocess the model. */
   MpatPreprocess(ContextId, M_DEFAULT, MilSourceImage);

   /* Allocate a result buffer. */
   MpatAllocResult(MilSystem, M_DEFAULT, &MilResult);

   /* Draw the original model position. */
   MpatDraw(M_DEFAULT, ContextId, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
            M_DEFAULT, M_ORIGINAL);

   /* Pause to show the original image and model position. */
   MosPrintf(MIL_TEXT("\nA %ldx%ld model was defined in the source image.\n"),
             ROTATED_FIND_MODEL_WIDTH, ROTATED_FIND_MODEL_HEIGHT);
   MosPrintf(MIL_TEXT("It will be searched in an image rotated at %d degrees.\n"),
             -ROTATED_FIND_ROTATION_DELTA_ANGLE);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Rotate the source image -10 degrees. */
   MimRotate(MilSourceImage, MilTargetImage, ROTATED_FIND_ROTATION_DELTA_ANGLE, M_DEFAULT, 
             M_DEFAULT, M_DEFAULT, M_DEFAULT, M_BILINEAR + M_OVERSCAN_CLEAR);

   MdispSelect(MilDisplay, MilTargetImage);

   /* Dummy first call for bench measure purpose only (bench stabilization, 
   cache effect, etc...). This first call is NOT required by the application. */
   MpatFind(ContextId, MilTargetImage, MilResult);

   /* Reset the timer. */
   MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);

   MpatFind(ContextId, MilTargetImage, MilResult);
 
   /* Read the time spent in MpatFind(). */
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);

   /* Clear the annotations. */
   MgraClear(M_DEFAULT, GraphicList);

   MpatGetResult(MilResult, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT, &NbFound);
   if (NbFound== 1L)
      {
      MgraColor(M_DEFAULT, AnnotationColor);
      MpatDraw(M_DEFAULT, MilResult, GraphicList, M_DRAW_BOX+M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
      MosPrintf(MIL_TEXT("A search model at a specific angle has been found in the rotated image.\n"));
      MosPrintf(MIL_TEXT("The search time is %.3f ms.\n\n"), Time*1000.0);
      }
   else
      {
      MosPrintf(MIL_TEXT("Model was not found!\n\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Disable the overlay display. */
   MdispControl(MilDisplay, M_OVERLAY_SHOW, M_DISABLE);

   /* Free all allocations. */
   MpatFree(MilResult);
   MpatFree(ContextId);
   MgraFree(GraphicList);
   MbufFree(MilTargetImage);
   MbufFree(MilSourceImage);
}

/*****************************************************************************/
/* Automatic model allocation example. */

/* Source and target images file specifications. */
#define AUTO_MODEL_IMAGE_FILE         M_IMAGE_PATH MIL_TEXT("Wafer.mim")
#define AUTO_MODEL_TARGET_IMAGE_FILE  M_IMAGE_PATH MIL_TEXT("WaferShifted.mim")

/* Model width and height. */
#define AUTO_MODEL_WIDTH   64L
#define AUTO_MODEL_HEIGHT  64L

void AutoAllocationModelExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage,                        /* Image buffer identifier.     */
              MilSubImage,                     /* Sub-image buffer identifier. */
              GraphicList,                     /* Graphic list.                */
              ContextId,                       /* Model identifier.            */
              Result;                          /* Result buffer identifier.    */
   MIL_INT    AllocError;                      /* Allocation error variable.   */
   MIL_INT    NumResults;                      /* Number of results found      */
   MIL_INT    ImageWidth, ImageHeight;         /* Target image dimensions      */
   MIL_DOUBLE OrgX = 0.0, OrgY = 0.0;          /* Original center of model.    */
   MIL_DOUBLE x = 0.0, y = 0.0, Score = 0.0;   /* Result variables.            */
   MIL_DOUBLE AnnotationColor = M_COLOR_GREEN; /* Drawing color.               */ 

   /* Load model image into an image buffer. */
   MbufRestore(AUTO_MODEL_IMAGE_FILE, MilSystem, &MilImage);

   /* Display the image. */
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &GraphicList);

   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraphicList);

   /* Restrict the region to be processed to the bottom right corner of the image. */
   MbufInquire(MilImage, M_SIZE_X, &ImageWidth);
   MbufInquire(MilImage, M_SIZE_Y, &ImageHeight);
   MbufChild2d(MilImage, ImageWidth/2, ImageHeight/2,
                         ImageWidth/2, ImageHeight/2, &MilSubImage);

   /* Add an offset to the drawings so they are aligned with
      the processed child image. */
   MgraControl(M_DEFAULT, M_DRAW_OFFSET_X, (MIL_DOUBLE)-(ImageWidth/2));
   MgraControl(M_DEFAULT, M_DRAW_OFFSET_Y, (MIL_DOUBLE)-(ImageHeight/2));

   /* Automatically allocate a normalized grayscale type pattern matching context. */
   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &ContextId);

   /* Define a unique model*/
   MpatDefine(ContextId, M_AUTO_MODEL, MilSubImage, M_DEFAULT, M_DEFAULT,
              AUTO_MODEL_WIDTH, AUTO_MODEL_HEIGHT, M_DEFAULT);

   /* Set the search accuracy to high. */
   MpatControl(ContextId, M_DEFAULT, M_ACCURACY, M_HIGH);

   /* Check for that model allocation was successful. */
   MappGetError(M_DEFAULT, M_CURRENT, &AllocError);
   if(!AllocError)
      {
      /* Draw a box around the model. */
      MgraColor(M_DEFAULT, AnnotationColor);
      MpatDraw(M_DEFAULT, ContextId, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
                                                                  M_DEFAULT, M_ORIGINAL);
      MosPrintf(MIL_TEXT("A model was automatically defined in the image.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Clear the annotations. */
      MgraClear(M_DEFAULT, GraphicList);

      /* Load target image into an image buffer. */
      MbufLoad(AUTO_MODEL_TARGET_IMAGE_FILE, MilImage);

      /* Allocate result. */
      MpatAllocResult(MilSystem, M_DEFAULT, &Result);

      /* Preprocess the model. */
      MpatPreprocess(ContextId, M_DEFAULT, MilSubImage);

      /* Find model. */
      MpatFind(ContextId, MilSubImage, Result);

      /* If one model was found above the acceptance threshold set. */
      MpatGetResult(Result, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT, &NumResults);
      if(NumResults == 1L)
         {
         /* Get results. */
         MpatGetResult(Result, M_DEFAULT, M_POSITION_X, &x);
         MpatGetResult(Result, M_DEFAULT, M_POSITION_Y, &y);
         MpatGetResult(Result, M_DEFAULT, M_SCORE, &Score);

         /* Draw a box around the occurrence. */
         MgraColor(M_DEFAULT, AnnotationColor);
         MpatDraw(M_DEFAULT, Result, GraphicList, M_DRAW_BOX + M_DRAW_POSITION,
                                                                        M_DEFAULT, M_DEFAULT);

         /* Analyze and print results. */
         MpatInquire(ContextId, M_DEFAULT, M_ORIGINAL_X, &OrgX);
         MpatInquire(ContextId, M_DEFAULT, M_ORIGINAL_Y, &OrgY);
         MosPrintf(MIL_TEXT("An image misaligned by 50 pixels in X and 20 pixels ")
                                                   MIL_TEXT("in Y was loaded.\n\n"));
         MosPrintf(MIL_TEXT("The image is found to be shifted by %.2f in X, ")
                                    MIL_TEXT("and %.2f in Y.\n"), x - OrgX, y - OrgY);
         MosPrintf(MIL_TEXT("Model match score is %.1f percent.\n"), Score);
         MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
         MosGetch();
         }
      else
         {
         MosPrintf(MIL_TEXT("Error: Pattern not found properly.\n"));
         MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
         MosGetch();
         }

      /* Free result buffer and model. */
      MpatFree(Result);
      MpatFree(ContextId);
      }
   else
      {
      MosPrintf(MIL_TEXT("Error: Automatic model definition failed.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   /* Remove the drawing offset. */
   MgraControl(M_DEFAULT, M_DRAW_OFFSET_X, 0.0);
   MgraControl(M_DEFAULT, M_DRAW_OFFSET_Y, 0.0);

   /* Free the graphic list. */
   MgraFree(GraphicList);

   /* Free child buffer and defaults. */
   MbufFree(MilSubImage);
   MbufFree(MilImage);
   }

