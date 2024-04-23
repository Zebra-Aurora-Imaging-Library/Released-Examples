//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name:  MimBoundingBox.cpp
// 
// Synopsis:   This program demonstrates how to use the function MimBoundingBox to compute
//             the corners of the smallest rectangular region including all the foreground 
//             pixels of an image. This region is thereafter alternately used and ignored 
//             according to the region support capability of the subsequent operations 
//             performed on the image.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#define IMAGE_FILE                     M_IMAGE_PATH MIL_TEXT("Preprocessing/Cookie.mim")

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
            MIL_TEXT("MimBoundingBox\n\n")
            MIL_TEXT("[SYNOPSIS]\n")
            MIL_TEXT("This program demonstrates how to use the function MimBoundingBox to compute\n")
            MIL_TEXT("the corners of the smallest rectangular region including all the foreground\n")
            MIL_TEXT("pixels of a depth map. This region is thereafter alternately used and ignored\n")
            MIL_TEXT("according to the region support capability of the subsequent operations\n")
            MIL_TEXT("performed on the image.\n\n")
            MIL_TEXT("[MODULES USED]\n")
            MIL_TEXT("Modules used: application, buffer, display, image processing, system.\n\n"));
   }

int MosMain()
   {
   PrintHeader();

   MIL_ID      MilApplication,      // Application identifier.
               MilSystem,           // System identifier.
               MilDisplay,          // Display identifier.
               MilGraphicListDisp,  // Graphic list associated to the display.
               MilGraphicListRegion,// Graphic list used to set the region.
               MilImage;            // Image buffer identifier.

   MIL_DOUBLE  AnnotationColor = M_COLOR_GREEN,
               BackGroundValue = 0;
   
   // Allocate objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Allocate a graphic list used to set a region.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicListRegion);

   // Allocate a graphic list to hold the subpixel annotations to draw.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicListDisp);
   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicListDisp);
   MgraColor(M_DEFAULT, AnnotationColor);

   // Restore and display the original image.
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   // Pause to show the original image.
   MosPrintf(MIL_TEXT("The original image is displayed.\n\n")
   MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Return the bounding box that contains all the pixels of the cookie.
   MIL_INT  TopLeftX, TopLeftY, BottomLeftX, BottomLeftY;
   MimBoundingBox(MilImage, M_GREATER, BackGroundValue, M_NULL, M_BOTH_CORNERS, 
      &TopLeftX, &TopLeftY, &BottomLeftX, &BottomLeftY, M_DEFAULT);

   // Display the minimum bounding box.
   MgraRect(M_DEFAULT, MilGraphicListDisp, TopLeftX, TopLeftY, BottomLeftX, BottomLeftY);
   
   // Fill the minimum bounidng box to set the region.
   MgraRectFill(M_DEFAULT, MilGraphicListRegion, TopLeftX, TopLeftY, BottomLeftX, BottomLeftY);
   MbufSetRegion(MilImage, MilGraphicListRegion, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The minimum bounding box that contains all the pixels of the object\n")
      MIL_TEXT("is found. It is used to set a region of interest.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   
   // Allocate the statistic context and result buffer. 
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   // Enable the statistics for calculation. 
   MimControl(MilStatContext, M_STAT_MAX, M_ENABLE);
   MimControl(MilStatContext, M_STAT_MEAN, M_ENABLE);
   MimStatCalculate(MilStatContext, MilImage, MilStatResult, M_DEFAULT);

   MIL_DOUBLE StatMeanVal, StatMaxVal;
   for (MIL_INT i = 0; i < 2; i++)
      {
	   // Calculate and get the statistics in the region.
      MimStatCalculate(MilStatContext, MilImage, MilStatResult, M_DEFAULT); 
      MimGetResult(MilStatResult, M_STAT_MEAN, &StatMeanVal);
	   MimGetResult(MilStatResult, M_STAT_MAX, &StatMaxVal);

      if (i == 0)
         {
         MosPrintf(MIL_TEXT("The region is used by default.\n\n"));
         MosPrintf(MIL_TEXT("Statistics are calculated in the region.\n\n"));
         }
      else
         {
         MosPrintf(MIL_TEXT("Statistics are re-calculated in the region.\n\n"));
         }

	   MosPrintf(MIL_TEXT("The mean pixel value is %.2f.\n"), StatMeanVal);
      MosPrintf(MIL_TEXT("The maximum pixel value is %.2f.\n\n"), StatMaxVal);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      
      if (i == 0)
         {
		   // Ignore the region in order to perform a morphological operation that does not support regions.
         MbufControl(MilImage, M_REGION_USE, M_IGNORE);
         MimOpen(MilImage, MilImage, 5, M_GRAYSCALE);
         MosPrintf(MIL_TEXT("The region is ignored in order to perform a 5-iteration\n")
            MIL_TEXT("open morphological operation.\n\n"));
         MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();

		   // Re-use the region to do some statstics inside the region.
         MbufControl(MilImage, M_REGION_USE, M_USE);
         MosPrintf(MIL_TEXT("The region is re-used.\n\n"));
         }
      }
   
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   // Free allocations.
   MimFree(MilStatResult);
   MimFree(MilStatContext);
   MgraFree(MilGraphicListRegion);
   MgraFree(MilGraphicListDisp);
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   }

