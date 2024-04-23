/***************************************************************************************/
/*
/* File name: GumPackInspection.cpp
/*
/* Synopsis:  This example demonstrates how to verify the presence and absence of gums
/*            from a stack of gum packs acquired using an extended depth of field camera
/*            such as the Ricoh EDOF camera.
/*
/* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
/* All Rights Reserved
*/

#include <mil.h>

/* MIL image file specifications. */  
#define NUM_IMAGES            3
#define EXAMPLE_IMAGE_PATH    M_IMAGE_PATH       MIL_TEXT("GumPackInspection/")
#define IMAGE_FILES           EXAMPLE_IMAGE_PATH MIL_TEXT("Image%d.mim")
#define IMAGE_NAME_MAX_LENGTH 200

/* Define the struct of fixturing and gum size data. */
struct SFixturingData
   {
   MIL_DOUBLE   FixturingOffsetX;
   MIL_DOUBLE   FixturingOffsetY;
   MIL_DOUBLE   FixturingAngle;
   MIL_DOUBLE   GumSpacingX;
   MIL_DOUBLE   GumSpacingY;
   MIL_DOUBLE   GumWidth;
   MIL_DOUBLE   GumHeight;
   };

/* Define the struct of measurement markers. */
struct SFixturingMarkers
{
   MIL_ID   VerticalLeftBoundaryMarker;
   MIL_ID   HorizontalBoundaryStripeMarker;
   MIL_ID   RowStripeMarker;
   MIL_ID   FirstColumnStripeMarker;
};

/* Define the row and column number of the gum pack. */
#define GUM_COL_NUM                     6
#define GUM_ROW_NUM                     2

/* Define the percentage of the gum area that are used for MimStatCalculate(). */
#define GUM_WIDTH_PERCENTAGE            0.6
#define GUM_HEIGHT_PERCENTAGE           0.8

/* Define the minimum standard deviation to be as a presence. */
#define MIN_STANDRAD_DEVIATION          20

/* Define percentile value - the upper and lower percentage of the histogram data to remove for statistics. */
#define PERCENTILE_VALUE                10

/* Function to locate and measure each gum pack */
void LocateandMeasureObject(MIL_ID MilSourceImage, SFixturingMarkers* FixturingMarkers, SFixturingData* FixturingData);

/* Example description. */
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("GumPackInspection\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to verify the presence and absence of gums\n")
             MIL_TEXT("from a stack of gum packs acquired using an extended depth of field camera.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("application, buffer, calibration, display, graphics, image processing,\n")
             MIL_TEXT("measurement, system.\n"));

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();
   }

/* Main function. */
int main(void)
   {
   MIL_ID      MilApplication       = M_NULL,   /* MIL application identifier.                              */
               MilSystem            = M_NULL,   /* MIL system identifier.                                   */
               MilDisplay           = M_NULL,   /* MIL display identifier.                                  */
               MilSourceImage       = M_NULL,   /* MIL source image buffer identifier.                      */
               MilDestinationImage  = M_NULL,   /* MIL image buffer destination identifier for warping.     */
               MilChildImage        = M_NULL,   /* MIL child image of each gum area for statistics.         */
               MilMaskImage         = M_NULL,   /* MIL mask image buffer to draw the mask for statistics.   */
               MilMaskChildImage    = M_NULL,   /* MIL child of the mask image buffer                       */
               MilBinImage1         = M_NULL,   /* MIL binary image for statistics.                         */
               MilBinImage2         = M_NULL,   /* MIL binary image for statistics.                         */
               MilGraphicList       = M_NULL,   /* MIL graphicList buffer identifier.                       */
               MilOverlayImage      = M_NULL,   /* MIL overlay buffer identifier.                           */
               MilStatContext       = M_NULL,   /* MIL identifier of the statistic context buffer.          */
               MilStatResult        = M_NULL;   /* MIL identifier of the statistic result buffer.           */

   MIL_DOUBLE  StdDiv;                          /* Standard deviation value from the statistics result      */

   SFixturingMarkers FixturingMarkers;          /* The struct of all measurement markers.                   */
   SFixturingData    FixturingData;             /* The struct of the data for fixturing.                    */

   MIL_INT           i,
                     PassNumber, FailNumber,    /* the number of gums passed and failed.                    */
                     LowThreshold, HighThreshold;

   MIL_TEXT_CHAR     SourceImageNames[NUM_IMAGES][IMAGE_NAME_MAX_LENGTH];   /* Source image name array.     */


   /* Allocate objects. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   MdispZoom(MilDisplay, 0.5, 0.5);

   PrintHeader();

   /* Allocate a graphic list to hold the sub-pixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   /* Associate the graphic list to the display. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);

   /* Load source image names. */
   for (i = 0; i < NUM_IMAGES; i++)
      MosSprintf(SourceImageNames[i],IMAGE_NAME_MAX_LENGTH, IMAGE_FILES, i+1);

   /* Measurement edge marker to measure the left side of the gum pack.             */
   MmeasAllocMarker(MilSystem, M_EDGE, M_DEFAULT, &(FixturingMarkers.VerticalLeftBoundaryMarker));
   /* Measurement stripe marker to measure the top and bottom side of the gum pack. */
   MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, &(FixturingMarkers.HorizontalBoundaryStripeMarker));
   /* Measurement multi-stripe marker to measure the top and bottom row of the gum. */
   MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, &(FixturingMarkers.RowStripeMarker));
   /* Measurement multi-stripe marker to measure the two gum of the first column.   */
   MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, &(FixturingMarkers.FirstColumnStripeMarker));

   /* Inquire the source image size. */
   MIL_INT SizeX    = MbufDiskInquire(SourceImageNames[0], M_SIZE_X, M_NULL);
   MIL_INT SizeY    = MbufDiskInquire(SourceImageNames[0], M_SIZE_Y, M_NULL);

   /* Allocate a source image buffer. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MilSourceImage);

   /* Allocate the statistic context and result buffer. */
   MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);
   /* Enable the standard deviation for statistic calculation. */
   MimControl(MilStatContext, M_STAT_STANDARD_DEVIATION, M_ENABLE);

   for(i =0; i<NUM_IMAGES; i++)
      {
      /* Restore and display source image. */
      MbufLoad(SourceImageNames[i], MilSourceImage); 
      MdispSelect(MilDisplay, MilSourceImage);

      LocateandMeasureObject(MilSourceImage, &FixturingMarkers, &FixturingData);

      if (i==0)
         {
         /* Allocate the destination image buffer for warping of which the size is the same as the first gum pack. */
         MbufAlloc2d(MilSystem, (MIL_INT)FixturingData.GumSpacingX*GUM_COL_NUM, (MIL_INT)FixturingData.GumSpacingY*GUM_ROW_NUM, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MilDestinationImage);
         /* Allocate the mask image buffer for annotation of which the size is the same as the first gum pack. */
         MbufAlloc2d(MilSystem, (MIL_INT)FixturingData.GumSpacingX*GUM_COL_NUM, (MIL_INT)FixturingData.GumSpacingY*GUM_ROW_NUM, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilMaskImage);
         }

      /* Clear destination buffer and mask image. */
      MbufClear(MilDestinationImage, 0);
      MbufClear(MilMaskImage, 1);

      /* Calibrate the destination image. */
      McalUniform(MilDestinationImage, 0, 0, 1.0, 1.0, 0, M_DEFAULT);

      /* Move the relative coordinates - The center of the first gum is the new origin, the angle of the gum pack is the fixturing angle. */
      McalFixture(MilSourceImage, M_NULL, M_MOVE_RELATIVE, M_POINT_AND_ANGLE, M_NULL, FixturingData.FixturingOffsetX, FixturingData.FixturingOffsetY, FixturingData.FixturingAngle, M_DEFAULT);

      /* Draw the relative coordinate system. */
      MgraColor(M_DEFAULT, M_COLOR_YELLOW);
      McalDraw(M_DEFAULT, M_NULL, MilGraphicList, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);

      MosPrintf(MIL_TEXT("A new gum pack has been located.\n")
         MIL_TEXT("\nPress <Enter> to continue.\n\n"));
      MosGetch();

      /* Clear the graphic list. */
      MgraClear(M_DEFAULT, MilGraphicList);

      /* Set the offset to draw the top-left conner of the first gum at the origin. */
      McalControl(MilDestinationImage, M_CALIBRATION_CHILD_OFFSET_X, -FixturingData.GumWidth/2);
      McalControl(MilDestinationImage, M_CALIBRATION_CHILD_OFFSET_Y, -FixturingData.GumHeight/2);

      /* Warp the image and display the destination image.*/
      McalTransformImage(MilSourceImage, MilDestinationImage, M_NULL, M_BILINEAR + M_OVERSCAN_DISABLE, M_DEFAULT, M_WARP_IMAGE+M_USE_DESTINATION_CALIBRATION);
      MdispSelect(MilDisplay, MilDestinationImage);
      MosPrintf(MIL_TEXT("The gum pack has been warped into a new image.\n\n"));

      /* Set the child buffer of the gum area that will be used for statistics. */
      MbufChild2d(MilDestinationImage, 0, 0, (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE), &MilChildImage);

      /* Set the child buffer of the mask image. */
      MbufChild2d(MilMaskImage, 0, 0, (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE), &MilMaskChildImage);

      /* Allocate two binary mask image buffers for statics. */
      MbufAlloc2d(MilSystem, (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE), 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage1);
      MbufAlloc2d(MilSystem, (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE), 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage2);

      PassNumber =0, FailNumber =0;
      for (MIL_INT RowIndex=0; RowIndex< GUM_ROW_NUM; RowIndex++)
         {
         for (MIL_INT ColIndex=0; ColIndex< GUM_COL_NUM; ColIndex++)
            {
            /* Move the child buffer to the current gum area. */
            MbufChildMove(MilChildImage, (MIL_INT)(FixturingData.GumWidth*((1-GUM_WIDTH_PERCENTAGE)/2)+ColIndex*FixturingData.GumSpacingX), 
                              (MIL_INT)(FixturingData.GumHeight*((1-GUM_HEIGHT_PERCENTAGE)/2)+RowIndex*FixturingData.GumSpacingY),
                              (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE),M_DEFAULT);

            /* Move the mask child buffer to the current gum area. */
            MbufChildMove(MilMaskChildImage, (MIL_INT)(FixturingData.GumWidth*((1-GUM_WIDTH_PERCENTAGE)/2)+ColIndex*FixturingData.GumSpacingX), 
               (MIL_INT)(FixturingData.GumHeight*((1-GUM_HEIGHT_PERCENTAGE)/2)+RowIndex*FixturingData.GumSpacingY),
               (MIL_INT)(FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE), (MIL_INT)(FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE),M_DEFAULT);

            /* Remove the top 10% of the histogram data.*/
            HighThreshold = MimBinarize(MilChildImage, M_NULL, M_PERCENTILE_VALUE+M_LESS, 100-PERCENTILE_VALUE, M_NULL);
            MimBinarize(MilChildImage, MilBinImage1, M_PERCENTILE_VALUE+M_LESS, 100-PERCENTILE_VALUE, M_NULL);

            /* Remove the bottom 10% of the histogram data.*/
            LowThreshold = MimBinarize(MilChildImage, M_NULL, M_PERCENTILE_VALUE+M_GREATER, PERCENTILE_VALUE, M_NULL);
            MimBinarize(MilChildImage, MilBinImage2, M_PERCENTILE_VALUE+M_GREATER, PERCENTILE_VALUE, M_NULL);

            /* Combine the mask. */
            MimArith(MilBinImage1, MilBinImage2, MilBinImage1, M_AND);

            /* Calculate the standard deviation with the mask. */
            MimControl(MilStatContext, M_CONDITION, M_IN_RANGE);
            MimControl(MilStatContext, M_COND_LOW, LowThreshold);
            MimControl(MilStatContext, M_COND_HIGH, HighThreshold);
            MimStatCalculate(MilStatContext, MilChildImage, MilStatResult, M_DEFAULT);
            MimGetResult(MilStatResult, M_STAT_STANDARD_DEVIATION, &StdDiv);
            if(StdDiv>MIN_STANDRAD_DEVIATION)
               {
               MgraColor(M_DEFAULT, M_COLOR_RED);
               FailNumber++;
               }
            else
               {
               MgraColor(M_DEFAULT, M_COLOR_GREEN);
               PassNumber++;
               }

            /* Draw a rectangle around the gum area. */
            MgraRectAngle(M_DEFAULT, MilGraphicList, ColIndex*FixturingData.GumSpacingX, RowIndex*FixturingData.GumSpacingY, 
                           FixturingData.GumWidth*GUM_WIDTH_PERCENTAGE, FixturingData.GumHeight*GUM_HEIGHT_PERCENTAGE, 0, M_CENTER_AND_DIMENSION);

            /* Copy the mask into the mask image. */
            MbufCopy(MilBinImage1, MilMaskChildImage);
         }
      }

      /* Enable the display of overlay annotations. */
      MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);

      /* Inquire the overlay buffer associated with the display. */
      MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

      /* Clear the overlay to transparent. */
      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      /* Draw the mask image in overlay. */
      MbufClearCond(MilOverlayImage, 255, 255, 0, MilMaskImage, M_EQUAL, 0);

      if (i ==0)
         {
         MosPrintf(MIL_TEXT("Displayed in yellow, the saturated pixels (bright and dark pixels) are\n")
            MIL_TEXT("masked out from further statistical calculations. The standard deviation\n")
            MIL_TEXT("of each gum area is used to infer the presence of any defect.\n\n"));
         }

      MosPrintf(MIL_TEXT("The number of the gums that passed: %d"), PassNumber);
      MosPrintf(MIL_TEXT("\nThe number of the gums that failed: %d\n"), FailNumber);

      if(i < NUM_IMAGES-1)
         MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
      else
         MosPrintf(MIL_TEXT("\nPress <Enter> to finish.\n\n"));
      MosGetch();

      MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

      MgraClear(M_DEFAULT, MilGraphicList);
      MbufFree(MilBinImage2);
      MbufFree(MilBinImage1);
      MbufFree(MilMaskChildImage);
      MbufFree(MilChildImage);
      }

   /* Free resources. */
   MbufFree(MilMaskImage);
   MbufFree(MilDestinationImage);
   MimFree(MilStatResult);
   MimFree(MilStatContext);
   MbufFree(MilSourceImage);
   MmeasFree(FixturingMarkers.FirstColumnStripeMarker);
   MmeasFree(FixturingMarkers.RowStripeMarker);
   MmeasFree(FixturingMarkers.HorizontalBoundaryStripeMarker);
   MmeasFree(FixturingMarkers.VerticalLeftBoundaryMarker);

   if(MilGraphicList)
      MgraFree(MilGraphicList);

   MdispFree(MilDisplay);    
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
   }


/* The edge measurement box definition to measure the left edge of the gum pack. */
#define MEAS_BOX_HEIGHT                   50
#define LEFT_EDGE_MEAS_BOX_WIDTH          400
#define LEFT_EDGE_MEAS_BOX_X              250

/* The stripe measurement box definition to measure the top and bottom edges. */
#define TOP_BOTTOM_MEAS_BOX_WIDTH         1000
#define TOP_BOTTOM_MEAS_BOX_ANGLE         270

/* The stripe measurement box definition to measure the gums in the top row.  */
#define TOP_ROW_MEAS_BOX_WIDTH            1500
#define TOP_ROW_MEAS_BOX_HEIGHT           120
#define TOP_ROW_MEAS_BOX_ANGLE_DELTA      20
#define TOP_ROW_MEAS_BOX_ANGLE_ACCURACY   0.1

/* The stripe width score parameters */
#define STRIPE_WIDTH_LOW                  100
#define STRIPE_WIDTH_HIGH                 200
#define STRIPE_HEIGHT_LOW                 200
#define STRIPE_HEIGHT_HIGH                300

/* The stripe measurement box definition to measure the two gums in the first column. */
#define FIRST_COLUMN_MEAS_BOX_WIDTH       1000

/* the max association distance of the the stripe to measure the two gums in the first column. */
#define MAX_ASSOCIATION_DISTANCE          10

void LocateandMeasureObject(MIL_ID MilSourceImage, SFixturingMarkers* FixturingMarkers, SFixturingData* FixturingData)
   {
   MIL_INT SizeX = MbufInquire(MilSourceImage, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquire(MilSourceImage, M_SIZE_Y, M_NULL);
   MIL_INT i;

   /* Specify the left gum pack edge characteristics. */
   MmeasSetMarker(FixturingMarkers->VerticalLeftBoundaryMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(FixturingMarkers->VerticalLeftBoundaryMarker, M_FILTER_TYPE, M_SHEN, M_NULL);

   /* Set score function to find the First edge and ignore the strength score. */
   MmeasSetScore(FixturingMarkers->VerticalLeftBoundaryMarker, M_DISTANCE_FROM_BOX_ORIGIN_SCORE, 0, 0, 0, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MmeasSetScore(FixturingMarkers->VerticalLeftBoundaryMarker, M_STRENGTH_SCORE, 0, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Specify the search region size and position. */
   MmeasSetMarker(FixturingMarkers->VerticalLeftBoundaryMarker, M_BOX_CENTER, LEFT_EDGE_MEAS_BOX_X, SizeY/2.0);
   MmeasSetMarker(FixturingMarkers->VerticalLeftBoundaryMarker, M_BOX_SIZE, LEFT_EDGE_MEAS_BOX_WIDTH, MEAS_BOX_HEIGHT);

   /* Measure the left edge of the gum pack. */
   MmeasFindMarker(M_DEFAULT, MilSourceImage, FixturingMarkers->VerticalLeftBoundaryMarker, M_DEFAULT);
   MIL_DOUBLE VerticalLeftBoundaryAngle;
   MmeasGetResultSingle(FixturingMarkers->VerticalLeftBoundaryMarker, M_ANGLE, &VerticalLeftBoundaryAngle, M_NULL, 0);

   /*-------------------------------------------------------------------------------------------------------------------*/
   /* Specify the stripe characteristics of the top and bottom edge of the gum pack. */
   MmeasSetMarker(FixturingMarkers->HorizontalBoundaryStripeMarker, M_POLARITY, M_POSITIVE, M_NEGATIVE);
   MmeasSetMarker(FixturingMarkers->HorizontalBoundaryStripeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);

   /* Set score function to find the widest stripe. */
   MmeasSetScore(FixturingMarkers->HorizontalBoundaryStripeMarker, M_STRIPE_WIDTH_SCORE, 0, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_MAX_POSSIBLE_VALUE, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Specify the search region size and position. */
   MmeasSetMarker(FixturingMarkers->HorizontalBoundaryStripeMarker, M_BOX_CENTER, SizeX/2.0, SizeY/2.0);
   MmeasSetMarker(FixturingMarkers->HorizontalBoundaryStripeMarker, M_BOX_SIZE, TOP_BOTTOM_MEAS_BOX_WIDTH, MEAS_BOX_HEIGHT);
   MmeasSetMarker(FixturingMarkers->HorizontalBoundaryStripeMarker, M_BOX_ANGLE, TOP_BOTTOM_MEAS_BOX_ANGLE, M_NULL);

   /* Measure the top and bottom edge of th gum pack. */
   MmeasFindMarker(M_DEFAULT, MilSourceImage, FixturingMarkers->HorizontalBoundaryStripeMarker, M_DEFAULT);
   MIL_DOUBLE HorizontalBoundaryStripeY, HorizontalBoundaryStripeWidth;
   MmeasGetResultSingle(FixturingMarkers->HorizontalBoundaryStripeMarker, M_POSITION, M_NULL, &HorizontalBoundaryStripeY, 0);
   MmeasGetResultSingle(FixturingMarkers->HorizontalBoundaryStripeMarker, M_STRIPE_WIDTH, &HorizontalBoundaryStripeWidth, M_NULL, 0);

   /*-------------------------------------------------------------------------------------------------------------------*/
   /* Specify the multi-stripe Marker of top row characteristics. */
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_POLARITY, M_NEGATIVE, M_POSITIVE);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);

   /* Specify the search region size, position, angle range etc. */
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_CENTER, SizeX/2.0, HorizontalBoundaryStripeY-HorizontalBoundaryStripeWidth/4);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_SIZE, TOP_ROW_MEAS_BOX_WIDTH, TOP_ROW_MEAS_BOX_HEIGHT);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE, VerticalLeftBoundaryAngle-90, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_NUMBER, GUM_COL_NUM, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE_MODE, M_ENABLE, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE_DELTA_POS, TOP_ROW_MEAS_BOX_ANGLE_DELTA, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE_DELTA_NEG, TOP_ROW_MEAS_BOX_ANGLE_DELTA, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE_ACCURACY, TOP_ROW_MEAS_BOX_ANGLE_ACCURACY, M_NULL);
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_MAX_ASSOCIATION_DISTANCE, MAX_ASSOCIATION_DISTANCE, M_NULL);

   /* Set the stripe width range. */
   MmeasSetScore(FixturingMarkers->RowStripeMarker, M_STRIPE_WIDTH_SCORE, STRIPE_WIDTH_LOW, STRIPE_WIDTH_LOW, STRIPE_WIDTH_HIGH, STRIPE_WIDTH_HIGH, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Measure the top-row gums. */
   MmeasFindMarker(M_DEFAULT, MilSourceImage, FixturingMarkers->RowStripeMarker, M_DEFAULT);
   MIL_DOUBLE TopRowFirstStripeY, TopRowStripeWidthArray[GUM_COL_NUM], TopRowStripeSpacingArray[GUM_COL_NUM-1];
   MmeasGetResultSingle(FixturingMarkers->RowStripeMarker, M_POSITION, &(FixturingData->FixturingOffsetX), &TopRowFirstStripeY, 0);
   MmeasGetResult(FixturingMarkers->RowStripeMarker, M_BOX_ANGLE_FOUND, &(FixturingData->FixturingAngle), M_NULL);
   MmeasGetResult(FixturingMarkers->RowStripeMarker, M_STRIPE_WIDTH, TopRowStripeWidthArray, M_NULL);
   MmeasGetResult(FixturingMarkers->RowStripeMarker, M_SPACING, TopRowStripeSpacingArray, M_NULL);
   FixturingData->GumSpacingX = 0;
   for(i =0; i<GUM_COL_NUM-1; i++)
      FixturingData->GumSpacingX += TopRowStripeSpacingArray[i];
   FixturingData->GumSpacingX /=GUM_COL_NUM-1;
   
   FixturingData->GumWidth = 0;
   for(i =0; i<GUM_COL_NUM; i++)
      FixturingData->GumWidth += TopRowStripeWidthArray[i];
   FixturingData->GumWidth /= GUM_COL_NUM;

   /* Specify the search region size and position for the bottom row. */
   MmeasSetMarker(FixturingMarkers->RowStripeMarker, M_BOX_CENTER, SizeX/2.0, HorizontalBoundaryStripeY+HorizontalBoundaryStripeWidth/4);

   /* Measure the first gum in the bottom row. */
   MmeasFindMarker(M_DEFAULT, MilSourceImage, FixturingMarkers->RowStripeMarker, M_DEFAULT);
   MIL_DOUBLE BottomRowFirstStripeX,  BottomRowFirstStripeY;
   MmeasGetResultSingle(FixturingMarkers->RowStripeMarker, M_POSITION, &BottomRowFirstStripeX, &BottomRowFirstStripeY, 0);

   /*-------------------------------------------------------------------------------------------------------------------*/
   /* Specify the multi-stripe Marker for the first column. */
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_POLARITY, M_NEGATIVE, M_POSITIVE);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);

   /* Set the score function to find the widest stripe. */
   MmeasSetScore(FixturingMarkers->FirstColumnStripeMarker, M_STRIPE_WIDTH_SCORE, STRIPE_HEIGHT_LOW, STRIPE_HEIGHT_LOW, STRIPE_HEIGHT_HIGH, STRIPE_HEIGHT_HIGH, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   /* Specify the search region size and position. */
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_BOX_CENTER, (FixturingData->FixturingOffsetX+BottomRowFirstStripeX)/2, (TopRowFirstStripeY+BottomRowFirstStripeY)/2);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_BOX_SIZE, FIRST_COLUMN_MEAS_BOX_WIDTH, MEAS_BOX_HEIGHT);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_BOX_ANGLE, FixturingData->FixturingAngle-90, M_NULL);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_NUMBER, GUM_ROW_NUM, M_NULL);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_MAX_ASSOCIATION_DISTANCE, MAX_ASSOCIATION_DISTANCE, M_NULL);
   MmeasSetMarker(FixturingMarkers->FirstColumnStripeMarker, M_SEARCH_REGION_CLIPPING, M_ENABLE, M_NULL);

   /* Measure the two gums in the fist column. */
   MmeasFindMarker(M_DEFAULT, MilSourceImage, FixturingMarkers->FirstColumnStripeMarker, M_DEFAULT);
   MmeasGetResultSingle(FixturingMarkers->FirstColumnStripeMarker, M_POSITION, M_NULL, &(FixturingData->FixturingOffsetY), 0);
   MmeasGetResultSingle(FixturingMarkers->FirstColumnStripeMarker, M_STRIPE_WIDTH, &(FixturingData->GumHeight), M_NULL, 0);
   MmeasGetResultSingle(FixturingMarkers->FirstColumnStripeMarker, M_SPACING, &(FixturingData->GumSpacingY), M_NULL, 0);
   }
