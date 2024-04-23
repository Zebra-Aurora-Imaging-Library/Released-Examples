﻿/*******************************************************************************/
/*
 * File name: MBlob.cpp
 *
 * Synopsis:  This program loads an image of some nuts, bolts and washers, 
 *            determines the number of each of these, finds and marks
 *            their center of gravity using the Blob analysis module.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */ 
#include <mil.h>  
 
/* Target MIL image file specifications. */  
#define IMAGE_FILE            M_IMAGE_PATH MIL_TEXT("BoltsNutsWashers.mim")
#define IMAGE_THRESHOLD_VALUE 26L 

/* Minimum and maximum area of blobs. */
#define MIN_BLOB_AREA         50L 
#define MAX_BLOB_AREA         50000L

/* Radius of the smallest particles to keep. */
#define MIN_BLOB_RADIUS       3L

/* Minimum hole compactness corresponding to a washer. */
#define MIN_COMPACTNESS       1.5


int MosMain(void)
{ 
   MIL_ID     MilApplication,                 /* Application identifier.            */
              MilSystem,                      /* System identifier.                 */
              MilDisplay,                     /* Display identifier.                */
              MilImage,                       /* Image buffer identifier.           */
              MilGraphicList,                 /* Graphic list identifier.           */
              MilBinImage,                    /* Binary image buffer identifier.    */
              MilBlobResult,                  /* Blob result buffer identifier.     */
              MilBlobContext;                 /* Blob Context identifier.           */
   MIL_INT    TotalBlobs,                     /* Total number of blobs.             */
              BlobsWithHoles,                 /* Number of blobs with holes.        */
              BlobsWithRoughHoles,            /* Number of blobs with rough holes.  */
              n,                              /* Counter.                           */
              SizeX,                          /* Size X of the source buffer.       */
              SizeY;                          /* Size Y of the source buffer.       */  
                                               
   MIL_DOUBLE *CogX,                          /* X coordinate of center of gravity. */
              *CogY;                          /* Y coordinate of center of gravity. */
    
   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* Restore source image into image buffer. */ 
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);

   /* Allocate a graphic list to hold the subpixel annotations to draw. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   /* Associate the graphic list to the display. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   /* Display the buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a binary image buffer for fast processing. */
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

   /* Pause to show the original image. */ 
   MosPrintf(MIL_TEXT("\nBLOB ANALYSIS:\n"));
   MosPrintf(MIL_TEXT("--------------\n\n"));
   MosPrintf(MIL_TEXT("This program determines the number of bolts, nuts and washers\n"));
   MosPrintf(MIL_TEXT("in the image and finds their center of gravity.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
 
   /* Binarize image. */
   MimBinarize(MilImage, MilBinImage, M_FIXED+M_GREATER_OR_EQUAL, 
                                      IMAGE_THRESHOLD_VALUE, M_NULL);

   /* Remove small particles and then remove small holes. */
   MimOpen(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, M_BINARY);
   MimClose(MilBinImage, MilBinImage, MIN_BLOB_RADIUS, M_BINARY);
 
   /* Allocate a context. */ 
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
  
   /* Enable the Center Of Gravity feature calculation. */ 
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
 
   /* Allocate a blob result buffer. */
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult); 
 
   /* Calculate selected features for each blob. */ 
   MblobCalculate(MilBlobContext, MilBinImage, M_NULL, MilBlobResult);
 
   /* Exclude blobs whose area is too small. */ 
   MblobSelect(MilBlobResult, M_EXCLUDE, M_AREA, M_LESS_OR_EQUAL, 
                                                 MIN_BLOB_AREA, M_NULL); 
 
   /* Get the total number of selected blobs. */ 
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &TotalBlobs);
   MosPrintf(MIL_TEXT("There are %d objects "), (int) TotalBlobs); 
  
   /* Read and print the blob's center of gravity. */ 
   if ((CogX = new MIL_DOUBLE[TotalBlobs]) && 
       (CogY = new MIL_DOUBLE[TotalBlobs]))
      { 
      /* Get the results. */
      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_X + M_BINARY, CogX);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_Y + M_BINARY, CogY);
    
      /* Print the center of gravity of each blob. */     
      MosPrintf(MIL_TEXT("and their centers of gravity are:\n")); 
      for(n=0; n < TotalBlobs; n++)
         MosPrintf(MIL_TEXT("Blob #%d: X=%5.1f, Y=%5.1f\n"), (int) n, CogX[n], CogY[n]);

      delete [] CogX; CogX = NULL;
      delete [] CogY; CogY = NULL;
      }
   else
      MosPrintf(MIL_TEXT("\nError: Not enough memory.\n")); 
  
   /* Draw a cross at the center of gravity of each blob. */  
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MblobDraw(M_DEFAULT, MilBlobResult, MilGraphicList, M_DRAW_CENTER_OF_GRAVITY, 
                                                       M_INCLUDED_BLOBS, M_DEFAULT);

   /* Reverse what is considered to be the background so that
    * holes are seen as being blobs. 
    */
   MblobControl(MilBlobContext, M_FOREGROUND_VALUE, M_ZERO);

   /* Add a feature to distinguish between types of holes. Since area
    * has already been added to the context, and the processing 
    * mode has been changed, all blobs will be re-included and the area 
    * of holes will be calculated automatically.
    */
   MblobControl(MilBlobContext, M_COMPACTNESS, M_ENABLE);

   /* Calculate selected features for each blob. */
   MblobCalculate(MilBlobContext, MilBinImage, M_NULL, MilBlobResult);

   /* Exclude small holes and large (the area around objects) holes. */
   MblobSelect(MilBlobResult, M_EXCLUDE, M_AREA, M_OUT_RANGE, 
                                         MIN_BLOB_AREA, MAX_BLOB_AREA);

   /* Get the number of blobs with holes. */
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &BlobsWithHoles);

   /* Exclude blobs whose holes are compact (i.e. nuts). */
   MblobSelect(MilBlobResult, M_EXCLUDE, M_COMPACTNESS, M_LESS_OR_EQUAL, 
                                                        MIN_COMPACTNESS, M_NULL);

   /* Get the number of blobs with holes that are NOT compact. */
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &BlobsWithRoughHoles);

   /* Print results. */
   MosPrintf(MIL_TEXT("\nIdentified objects:\n"));
   MosPrintf(MIL_TEXT("%d bolts\n"), (int) (TotalBlobs-BlobsWithHoles));
   MosPrintf(MIL_TEXT("%d nuts\n"), (int) (BlobsWithHoles - BlobsWithRoughHoles));
   MosPrintf(MIL_TEXT("%d washers\n\n"), (int) (BlobsWithRoughHoles));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n")); 
   MosGetch();

   /* Free all allocations. */
   MgraFree(MilGraphicList);
   MblobFree(MilBlobResult); 
   MblobFree(MilBlobContext);
   MbufFree(MilBinImage);
   MbufFree(MilImage);
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
}
