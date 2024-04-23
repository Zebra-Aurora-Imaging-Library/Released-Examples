
/***************************************************************************************/
/*
* File name: ExtendedDepthOfField.cpp
*
* Synopsis:  This program demonstrates how to combine (fuse) multiple
*            images taken at different focus distances using a liquid
*            lens to give a resulting in-focus image with an extended
*            depth of field.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Example functions prototypes. */
void OfflineExtendedDepthOfField(MIL_ID             MilSystem,
                                 MIL_ID             MilDisplay,
                                 MIL_INT            NbImages,
                                 MIL_INT            SizeBand,
                                 MIL_INT            SizeX,
                                 MIL_INT            SizeY,
                                 MIL_INT            Type,
                                 MIL_INT64          Attribute,
                                 MIL_CONST_TEXT_PTR ImageDirectory);

void OnlineExtendedDepthOfField(MIL_ID             MilSystem,
                                MIL_ID             MilDisplay,
                                MIL_INT            NbImages,
                                MIL_INT            SizeBand,
                                MIL_INT            SizeX,
                                MIL_INT            SizeY,
                                MIL_INT            Type,
                                MIL_INT64          Attribute,
                                MIL_CONST_TEXT_PTR ImageDirectory);

/* Source images directories. */
#define IMAGES_DIR_SOURCE_BOARD   M_IMAGE_PATH MIL_TEXT("ExtendedDepthOfField/BoardFocusStackingImages")
#define IMAGES_DIR_SOURCE_BOTTLES M_IMAGE_PATH MIL_TEXT("ExtendedDepthOfField/BottlesFocusStackingImages")

/* Local variables definitions for Offline fusion. */
const MIL_INT    NB_IMG_BOTTLES                     = 30;
const MIL_INT    SIZE_BAND_IMG_BOTTLES              = 1;
const MIL_INT    SIZE_X_IMG_BOTTLES                 = 672;
const MIL_INT    SIZE_Y_IMG_BOTTLES                 = 512;
const MIL_INT    TYPE_IMG_BOTTLES                   = 8 + M_UNSIGNED;
const MIL_INT64  ATTRIBUTE_IMG_BOTTLES              = M_IMAGE + M_DISP + M_GRAB + M_PROC;

/* Local variables definitions for Online fusion. */
const MIL_INT    NB_IMG_BOARD                       = 25;
const MIL_INT    SIZE_BAND_IMG_BOARD                = 1;
const MIL_INT    SIZE_X_IMG_BOARD                   = 672;
const MIL_INT    SIZE_Y_IMG_BOARD                   = 512;
const MIL_INT    TYPE_IMG_BOARD                     = 8 + M_UNSIGNED;
const MIL_INT64  ATTRIBUTE_IMG_BOARD                = M_IMAGE + M_DISP + M_GRAB + M_PROC;
const MIL_INT    MODE                               = M_RECONSTRUCTION;
const MIL_DOUBLE TRANSLATION_TOTELANCE              = 1;
const MIL_DOUBLE MAXIMUM_CIRCLE_OF_CONFUSION_RADIUS = 8;

/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("ExtendedDepthOfField\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This program demonstrates how to combine (fuse) multiple\n")
      MIL_TEXT("images taken at different focus distances using a liquid\n")
      MIL_TEXT("lens to give a resulting in-focus image with an extended\n")
      MIL_TEXT("depth of field.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("image processing, registration.\n\n"));

   }

int MosMain()
   {
   MIL_ID MilApplication,    /* Application identifier. */
          MilSystem,         /* System identifier.      */
          MilDisplay;        /* Display identifier.     */

   /* Allocate application, system and display. */
   MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   PrintHeader();

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nFirst method (offline operation) :\n")
             MIL_TEXT("The extended depth of field image is calculated in a single \n")
             MIL_TEXT("step using all the provided images.\n\n")
             MIL_TEXT("Press <Enter> to load the sequence of images.\n"));
   MosGetch();

   OfflineExtendedDepthOfField(MilSystem,
                               MilDisplay,
                               NB_IMG_BOTTLES,
                               SIZE_BAND_IMG_BOTTLES,
                               SIZE_X_IMG_BOTTLES,
                               SIZE_Y_IMG_BOTTLES,
                               TYPE_IMG_BOTTLES,
                               ATTRIBUTE_IMG_BOTTLES,
                               IMAGES_DIR_SOURCE_BOTTLES);
   
   /* Print a message. */
   MosPrintf(MIL_TEXT("\nSecond method (online operation) :\n")
             MIL_TEXT("The images are sequentially provided and pre-fused into a\n")
             MIL_TEXT("result buffer. Then the extended depth of field image is generated.\n")
             MIL_TEXT("Press <Enter> to load the images and to pre-fuse them sequentially.\n\n"));
   MosGetch();

   /* Note that the online operation requires less temporary memory. */
   OnlineExtendedDepthOfField(MilSystem,
                              MilDisplay,
                              NB_IMG_BOARD,
                              SIZE_BAND_IMG_BOARD,
                              SIZE_X_IMG_BOARD,
                              SIZE_Y_IMG_BOARD,
                              TYPE_IMG_BOARD,
                              ATTRIBUTE_IMG_BOTTLES,
                              IMAGES_DIR_SOURCE_BOARD);

   /* Free application, system and display. */
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

/********************************************************************************/
/*  Offline extended depth of field without context or result.                  */
/********************************************************************************/
void OfflineExtendedDepthOfField(MIL_ID             MilSystem,
                                 MIL_ID             MilDisplay,
                                 MIL_INT            NbImages,
                                 MIL_INT            SizeBand,
                                 MIL_INT            SizeX,
                                 MIL_INT            SizeY,
                                 MIL_INT            Type,
                                 MIL_INT64          Attribute,
                                 MIL_CONST_TEXT_PTR ImageDirectory)
   {
   MIL_ID  FusionImage;     /* Id of the fusion image.                      */
   MIL_ID  DigId;           /* Id of the digitizer used to read the images. */
   MIL_ID* ImagesArray;     /* Ids of the images processed.                 */

   /* Allocating the sequence of images. */
   ImagesArray = new MIL_ID[NbImages];
   for (MIL_INT NumImg = 0; NumImg < NbImages; NumImg++)
      {
      MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, Attribute, &ImagesArray[NumImg]);
      }

   /* Allocate the fusion buffer. */
   MbufClone(ImagesArray[0], M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &FusionImage);

   /* Loading the sequence of images. */
   DigId = MdigAlloc(MilSystem, M_DEFAULT, ImageDirectory, M_EMULATED, M_NULL);
   MdigProcess(DigId, ImagesArray, NbImages, M_SEQUENCE, M_DEFAULT, M_NULL, M_NULL);
   MdigFree(DigId);
   DigId = M_NULL;

   for (MIL_INT NumImg = 0; NumImg < NbImages; NumImg++)
      {
      /* Display the image loaded. */
      MdispSelect(MilDisplay, ImagesArray[NumImg]);
      MosSleep(80);
      }
   MosPrintf(MIL_TEXT("\nPress <Enter> to process the sequence.\n\n"));
   MosGetch();

   /* Compute and display the image fusion. */
   MosPrintf(MIL_TEXT("Calculation in progress..."));
   MregCalculate(M_DEFAULT_EXTENDED_DEPTH_OF_FIELD_CONTEXT, ImagesArray, FusionImage, NbImages, M_COMPUTE);
   MdispSelect(MilDisplay, FusionImage);
   MosPrintf(MIL_TEXT("\n\nImage fusion result."));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(FusionImage);
   for (MIL_INT NumImg = 0; NumImg < NbImages; NumImg++)
      {
      MbufFree(ImagesArray[NumImg]);
      }
   delete[] ImagesArray;
   }


/*******************************************************************************************/
/*  Online extended depth of field with specified context and result.                      */
/*******************************************************************************************/

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID RegContext;
   MIL_ID RegResult;
   MIL_ID Display;
   } HookDataStruct;


/* User's processing function called every time a grab buffer is ready. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, 
                                  MIL_ID  HookId, 
                                  void   *UserDataPtr)
   {
   HookDataStruct* UserStruct = (HookDataStruct*)UserDataPtr;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MIL_ID ModifiedBufferId;
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Display the image to be loaded. */
   MdispSelect(UserStruct->Display, ModifiedBufferId);

   /* Accumulate the current buffer in the registration result. */
   MregCalculate(UserStruct->RegContext, &ModifiedBufferId, UserStruct->RegResult, 1, M_ACCUMULATE);

   return 0;
   }

void OnlineExtendedDepthOfField(MIL_ID             MilSystem,
                                MIL_ID             MilDisplay,
                                MIL_INT            NbImages,
                                MIL_INT            SizeBand,
                                MIL_INT            SizeX,
                                MIL_INT            SizeY,
                                MIL_INT            Type,
                                MIL_INT64          Attribute,
                                MIL_CONST_TEXT_PTR ImageDirectory)
   {
   MIL_ID         FusionImage,     /* Id of the fusion image.                      */
                  RegContext,      /* Id of the registration context.              */
                  RegResult,       /* Id of the registration result.               */
                  DigId;           /* Id of the digitizer used to read the images. */  
   MIL_ID*        ImagesArray;     /* Ids of the images processed.                 */
   HookDataStruct UserHookData;    /* User's processing function data structure.   */


   /* Allocating the fusion registration objects. */
   RegContext = MregAlloc      (MilSystem, M_EXTENDED_DEPTH_OF_FIELD       , M_DEFAULT, M_NULL);
   RegResult  = MregAllocResult(MilSystem, M_EXTENDED_DEPTH_OF_FIELD_RESULT, M_NULL);

   /* Allocating the digitizer. */
   DigId = MdigAlloc(MilSystem, M_DEFAULT, ImageDirectory, M_EMULATED, M_NULL);

   /* Setting the registration context parameters. */
   MregControl(RegContext, M_DEFAULT, M_TRANSLATION_TOLERANCE, TRANSLATION_TOTELANCE);
   MregControl(RegContext, M_DEFAULT, M_CIRCLE_OF_CONFUSION_RADIUS_MAX, MAXIMUM_CIRCLE_OF_CONFUSION_RADIUS);
   MregControl(RegContext, M_DEFAULT, M_MODE, MODE);

   /* Allocating the sequence of images. */
   const MIL_INT ImageCount = 2;
   ImagesArray = new MIL_ID[ImageCount];
   for (MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
      {
      MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, Attribute, &ImagesArray[NumImg]);
      }

   /* Allocating the other buffer. */
   MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, Type, Attribute, &FusionImage);

   /* Initialize the user's processing function data structure. */
   UserHookData.RegContext      = RegContext;
   UserHookData.RegResult       = RegResult;
   UserHookData.Display         = MilDisplay;

   /* Reading the current image. */
   MdigProcess(DigId, ImagesArray, ImageCount, M_SEQUENCE + M_COUNT(NbImages), M_DEFAULT, ProcessingFunction, &UserHookData);
   MosPrintf(MIL_TEXT("\nPress <Enter> to generate the extended depth of field image.\n\n"));
   MosGetch();

   /* Get the fusion image. */
   MregCalculate(RegContext, M_NULL, RegResult, 0, M_ACCUMULATE_AND_COMPUTE);
   MregDraw(M_DEFAULT, RegResult, FusionImage, M_DRAW_EDOF_IMAGE, M_DEFAULT, M_DEFAULT);

   /* Display the image fusion. */
   MdispSelect(MilDisplay, FusionImage);
   MosPrintf(MIL_TEXT("Image fusion result."));

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(FusionImage);
   for (MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
      {
      MbufFree(ImagesArray[NumImg]);
      }
   delete[] ImagesArray;

   /* Free digitizer. */
   MdigFree(DigId);

   /* Free registration objects. */
   MregFree(RegResult);
   MregFree(RegContext);
   }
