﻿/***************************************************************************************/
/*
* File name: DepthFromFocus.cpp
*
* Synopsis:  This program demonstrates how to obtain an index map from
*            multiple images taken at different focus distances using 
*            a liquid lens.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>

/* Example functions prototypes. */
void OfflineDepthFromFocusIndexMapAndConfidenceMap(MIL_ID             MilSystem,
                                                   MIL_ID             MilDisplay,
                                                   MIL_INT            NbImages,
                                                   MIL_INT            SizeX,
                                                   MIL_INT            SizeY,
                                                   MIL_INT            Type,
                                                   MIL_INT64          Attribute,
                                                   MIL_CONST_TEXT_PTR ImageDirectory);

void OnlineDepthFromFocusIndexMapAndIntensityMap  (MIL_ID             MilSystem,
                                                   MIL_ID             MilDisplay,
                                                   MIL_INT            NbImages,
                                                   MIL_INT            SizeX,
                                                   MIL_INT            SizeY,
                                                   MIL_INT            Type,
                                                   MIL_INT64          Attribute,
                                                   MIL_CONST_TEXT_PTR ImageDirectory);

void OnlineDepthFromFocusIndexMapAndConfidenceMap (MIL_ID             MilSystem,
                                                   MIL_ID             MilDisplay,
                                                   MIL_INT            NbImages,
                                                   MIL_INT            SizeX,
                                                   MIL_INT            SizeY,
                                                   MIL_INT            Type,
                                                   MIL_INT64          Attribute,
                                                   MIL_CONST_TEXT_PTR ImageDirectory);
/* Utility functions. */
void RemapDisplayRangeTo8Bits(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilScrImage, MIL_ID MilDisplayedImage);

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);

/* Source images directories. */
#define IMAGES_DIR_SOURCE_HEAT_SINK M_IMAGE_PATH MIL_TEXT("DepthFromFocus/HeatSinkFocusStackingImages")
#define IMAGES_DIR_SOURCE_IRIS_CASE M_IMAGE_PATH MIL_TEXT("DepthFromFocus/IrisCaseFocusStackingImages")
#define IMAGES_DIR_SOURCE_BOTTLE    M_IMAGE_PATH MIL_TEXT("DepthFromFocus/BottleFocusStackingImages")

/* Constants for Offline depth from focus computations of the index map and the confidence map. */
const MIL_INT    NB_IMG_HEAT_SINK        = 141;
const MIL_INT    SIZE_X_IMG_HEAT_SINK    = 672;
const MIL_INT    SIZE_Y_IMG_HEAT_SINK    = 512;
const MIL_INT    TYPE_IMG_HEAT_SINK      = 8 + M_UNSIGNED;
const MIL_INT64  ATTRIBUTE_IMG_HEAT_SINK = M_IMAGE + M_DISP + M_GRAB + M_PROC;

/* Constants for Online depth from focus computation of the index map. */
const MIL_INT    NB_IMG_IRIS_CASE        = 61;
const MIL_INT    SIZE_X_IMG_IRIS_CASE    = 672;
const MIL_INT    SIZE_Y_IMG_IRIS_CASE    = 512;
const MIL_INT    TYPE_IMG_IRIS_CASE      = 8 + M_UNSIGNED;
const MIL_INT64  ATTRIBUTE_IMG_IRIS_CASE = M_IMAGE + M_DISP + M_GRAB + M_PROC;

/* Constants for Online depth from focus computations of the index map and the confidence map. */
const MIL_INT    NB_IMG_BOTTLE        = 101;
const MIL_INT    SIZE_X_IMG_BOTTLE    = 512;
const MIL_INT    SIZE_Y_IMG_BOTTLE    = 672;
const MIL_INT    TYPE_IMG_BOTTLE      = 8 + M_UNSIGNED;
const MIL_INT64  ATTRIBUTE_IMG_BOTTLE = M_IMAGE + M_DISP + M_GRAB + M_PROC;

/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DepthFromFocus\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program demonstrates how to combine\n")
             MIL_TEXT("multiple images taken at different focus\n")
             MIL_TEXT("distances to obtain a resulting ordered map\n")
             MIL_TEXT("of indexes. Each index corresponds to the\n")
             MIL_TEXT("best focus distance at each pixel.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: Application, System, Display,\n")
             MIL_TEXT("Buffer, Image Processing, Registration.\n"));
   }

int MosMain()
   {
   MIL_ID MilApplication, /* Application identifier. */
          MilSystem,      /* System identifier.      */
          MilDisplay;     /* Display identifier.     */

   PrintHeader();

   /* Allocate application, system and display. */
   MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication);

   /* Check for required file. */
   if (!CheckForRequiredMILFile(IMAGES_DIR_SOURCE_HEAT_SINK MIL_TEXT("/Img_heatsink_000.mim")))
      {
      MappFree(MilApplication);
      return -1;
      }

   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\n\nFirst example: offline operation on a textured\n")
               MIL_TEXT("surface\n")
               MIL_TEXT("----------------------------------------------\n")
               MIL_TEXT("All the images are first collected. The depth\n")
               MIL_TEXT("from focus index map image is then calculated.\n\n")
               MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   OfflineDepthFromFocusIndexMapAndConfidenceMap(MilSystem,
                                                 MilDisplay,
                                                 NB_IMG_HEAT_SINK,
                                                 SIZE_X_IMG_HEAT_SINK,
                                                 SIZE_Y_IMG_HEAT_SINK,
                                                 TYPE_IMG_HEAT_SINK,
                                                 ATTRIBUTE_IMG_HEAT_SINK,
                                                 IMAGES_DIR_SOURCE_HEAT_SINK);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nSecond example: online operation on a textured\n")
             MIL_TEXT("object\n")
             MIL_TEXT("----------------------------------------------\n")
             MIL_TEXT("The images are sequentially acquired and added\n")
             MIL_TEXT("to the computation of the index map.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Note that the online operation requires less temporary memory. */
   OnlineDepthFromFocusIndexMapAndIntensityMap(MilSystem,
                                               MilDisplay,
                                               NB_IMG_IRIS_CASE,
                                               SIZE_X_IMG_IRIS_CASE,
                                               SIZE_Y_IMG_IRIS_CASE,
                                               TYPE_IMG_IRIS_CASE,
                                               ATTRIBUTE_IMG_IRIS_CASE,
                                               IMAGES_DIR_SOURCE_IRIS_CASE);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nThird example: using the confidence map result\n")
             MIL_TEXT("on a textureless object\n")
             MIL_TEXT("----------------------------------------------\n")
             MIL_TEXT("The images are sequentially acquired and added\n")
             MIL_TEXT("to the computation of the index map.\n\n")
             MIL_TEXT("A pattern is cast on the smooth surface of the\n")
             MIL_TEXT("object using a high power structured light.\n\n")
             MIL_TEXT("To filter out irrelevant areas in the index\n")
             MIL_TEXT("map, a confidence map is calculated.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Note that the online operation requires less temporary memory. */
   OnlineDepthFromFocusIndexMapAndConfidenceMap(MilSystem,
                                                MilDisplay,
                                                NB_IMG_BOTTLE,
                                                SIZE_X_IMG_BOTTLE,
                                                SIZE_Y_IMG_BOTTLE,
                                                TYPE_IMG_BOTTLE,
                                                ATTRIBUTE_IMG_BOTTLE,
                                                IMAGES_DIR_SOURCE_BOTTLE);

   /* Free application, system and display. */
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

/********************************************************************************/
/*  Offline depth from focus without result.                                    */
/********************************************************************************/

/* User's displaying function hook data structure. */
typedef struct
   {
   MIL_ID Display;
   } HookDisplayStruct;

/* User's displaying function called every time a grab buffer is ready. */
MIL_INT MFTYPE DisplayingFunction(MIL_INT HookType,
                                  MIL_ID  HookId,
                                  void   *UserDisplayPtr)
   {
   HookDisplayStruct* UserStruct = (HookDisplayStruct*)UserDisplayPtr;

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MIL_ID ModifiedBufferId;
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   /* Display the image to be loaded. */
   MdispSelect(UserStruct->Display, ModifiedBufferId);
   MosSleep(50);

   return 0;
   }

void OfflineDepthFromFocusIndexMapAndConfidenceMap(MIL_ID             MilSystem,
                                                   MIL_ID             MilDisplay,
                                                   MIL_INT            NbImages,
                                                   MIL_INT            SizeX,
                                                   MIL_INT            SizeY,
                                                   MIL_INT            Type,
                                                   MIL_INT64          Attribute,
                                                   MIL_CONST_TEXT_PTR ImageDirectory)
   {
   MIL_ID            IndexMap,          /* Id of the index map image.                                  */
                     DisplayedIndexMap, /* Id of the remapped index map for a better display contrast. */
                     RegContext,        /* Id of the registration context.                             */
                     DigId;             /* Id of the digitizer used to read the images.                */                 
   MIL_ID*           ImagesArray;       /* Ids of the images processed.                                */
   HookDisplayStruct UserHookData;      /* User's displaying function data structure.                  */

   /* Allocating the depth from focus registration object. */
   RegContext = MregAlloc(MilSystem, M_DEPTH_FROM_FOCUS, M_DEFAULT, M_NULL);

   /* Setting the registration context parameters. */
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_MODE, M_AVERAGE);
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_SIZE, 5);

   /* Allocating the sequence of images. */
   ImagesArray = new MIL_ID[NbImages];
   for(MIL_INT NumImg = 0; NumImg < NbImages; NumImg++)
      {
      MbufAlloc2d(MilSystem, SizeX, SizeY, Type, Attribute, &ImagesArray[NumImg]);
      }

   /* Allocate the index map buffers. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &IndexMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &DisplayedIndexMap);

   /* Initialize the user's processing function data structure. */
   UserHookData.Display = MilDisplay;

   /* Loading the sequence of images. */
   DigId = MdigAlloc(MilSystem, M_DEFAULT, ImageDirectory, M_EMULATED, M_NULL);
   MosPrintf(MIL_TEXT("A stack of images is acquired using a liquid lens.\n"));
   MosPrintf(MIL_TEXT("Load in progress...\n\n"));
   MdigProcess(DigId, ImagesArray, NbImages, M_SEQUENCE + M_COUNT(NbImages), M_DEFAULT, DisplayingFunction, &UserHookData);
   MosPrintf(MIL_TEXT("A stack of %d images has been loaded.\n"), NbImages);
   MdigFree(DigId);
   DigId = M_NULL;

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Compute and display the index map image. */
   MosPrintf(MIL_TEXT("Calculation in progress...\n\n"));
   MregCalculate(RegContext, ImagesArray, IndexMap, NbImages, M_COMPUTE);

   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, IndexMap, DisplayedIndexMap);
   MosPrintf(MIL_TEXT("The index map result is displayed.\n"));
   MosPrintf(MIL_TEXT("Each gray value corresponds to the index of an\n"));
   MosPrintf(MIL_TEXT("image among the acquired stack.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MimFilterMajority(M_DEFAULT, IndexMap, IndexMap, M_5X5_RECT, M_DEFAULT);
   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, IndexMap, DisplayedIndexMap);
   MosPrintf(MIL_TEXT("A majority filter is applied on\n"));
   MosPrintf(MIL_TEXT("the index map to remove noise\n"));
   MosPrintf(MIL_TEXT("while preserving valid index values.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(IndexMap);
   MbufFree(DisplayedIndexMap);
   for(MIL_INT NumImg = 0; NumImg < NbImages; NumImg++)
      {
      MbufFree(ImagesArray[NumImg]);
      }
   delete[] ImagesArray;

   /* Free registration context. */
   MregFree(RegContext);
   }

/*******************************************************************************************/
/*  Online depth from focus with specified context and result.                             */
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
   MosSleep(30);

   /* Accumulate the current buffer in the registration result. */
   MregCalculate(UserStruct->RegContext, &ModifiedBufferId, UserStruct->RegResult, 1, M_ACCUMULATE_AND_COMPUTE);

   return 0;
   }

void OnlineDepthFromFocusIndexMapAndIntensityMap(MIL_ID             MilSystem,
                                                 MIL_ID             MilDisplay,
                                                 MIL_INT            NbImages,
                                                 MIL_INT            SizeX,
                                                 MIL_INT            SizeY,
                                                 MIL_INT            Type,
                                                 MIL_INT64          Attribute,
                                                 MIL_CONST_TEXT_PTR ImageDirectory)
   {
   MIL_ID         IndexMap,          /* Id of the index map image.                                  */
                  DisplayedIndexMap, /* Id of the remapped index map for a better display contrast. */
                  IntensityMap,      /* Id of the intensity map image.                              */
                  RegContext,        /* Id of the registration context.                             */
                  RegResult,         /* Id of the registration result.                              */
                  DigId;             /* Id of the digitizer used to read the images.                */
   MIL_ID*        ImagesArray;       /* Ids of the images processed.                                */
   HookDataStruct UserHookData;      /* User's processing function data structure.                  */


   /* Allocating the depth from focus registration objects. */
   RegContext = MregAlloc      (MilSystem, M_DEPTH_FROM_FOCUS, M_DEFAULT, M_NULL);
   RegResult  = MregAllocResult(MilSystem, M_DEPTH_FROM_FOCUS_RESULT    , M_NULL);

   /* Allocating the digitizer. */
   DigId = MdigAlloc(MilSystem, M_DEFAULT, ImageDirectory, M_EMULATED, M_NULL);

   /* Setting the registration context parameters. */
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_MODE, M_AVERAGE);
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_SIZE, 11);
   MregControl(RegContext, M_DEFAULT, M_INTENSITY_MAP      , M_ENABLE);

   /* Allocating the sequence of images. */
   const MIL_INT ImageCount = 1;
   ImagesArray = new MIL_ID[ImageCount];
   for(MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
      {
      MbufAlloc2d(MilSystem, SizeX, SizeY, Type, Attribute, &ImagesArray[NumImg]);
      }

   /* Allocating the other buffer. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &IndexMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &DisplayedIndexMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, Type          , Attribute, &IntensityMap);

   /* Initialize the user's processing function data structure. */
   UserHookData.RegContext = RegContext;
   UserHookData.RegResult  = RegResult;
   UserHookData.Display    = MilDisplay;

   /* Reading the current image. */
   MosPrintf(MIL_TEXT("The images are processed when acquired.\n"));
   MosPrintf(MIL_TEXT("Load and processing in progress...\n\n"));
   MdigProcess(DigId, ImagesArray, ImageCount, M_SEQUENCE + M_COUNT(NbImages), M_DEFAULT, ProcessingFunction, &UserHookData);
   MosPrintf(MIL_TEXT("A stack of %d images has been processed.\n"), NbImages);
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Get the index map image. */
   MregDraw(M_DEFAULT, RegResult, IndexMap, M_DRAW_DEPTH_INDEX_MAP, M_DEFAULT, M_DEFAULT);

   /* Display the index map image. */
   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, IndexMap, DisplayedIndexMap);
   MosPrintf(MIL_TEXT("The resulting index map image is displayed.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();


   /* Get the intensity map image. */
   MregDraw(M_DEFAULT, RegResult, IntensityMap, M_DRAW_DEPTH_INTENSITY_MAP, M_DEFAULT, M_DEFAULT);

   /* Display the intensity map image. */
   MdispSelect(MilDisplay, IntensityMap);
   MosPrintf(MIL_TEXT("An extended depth of field image, reconstructed\n"));
   MosPrintf(MIL_TEXT("using the index map result, is displayed.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(IndexMap);
   MbufFree(DisplayedIndexMap);
   MbufFree(IntensityMap);
   for(MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
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


/*************************************************************************************************/
/*  Online depth from focus with specified context and result and the use of the confidence map. */
/*************************************************************************************************/
void OnlineDepthFromFocusIndexMapAndConfidenceMap(MIL_ID             MilSystem,
                                                  MIL_ID             MilDisplay,
                                                  MIL_INT            NbImages,
                                                  MIL_INT            SizeX,
                                                  MIL_INT            SizeY,
                                                  MIL_INT            Type,
                                                  MIL_INT64          Attribute,
                                                  MIL_CONST_TEXT_PTR ImageDirectory)
   {
   MIL_ID         IndexMap,               /* Id of the index map image.                                       */
                  DisplayedIndexMap,      /* Id of the remapped index map for a better display contrast.      */
                  ConfidenceMap,          /* Id of the confidence map image.                                  */
                  DisplayedConfidenceMap, /* Id of the remapped confidence map for a better display contrast. */
                  RelevantResult,         /* Id of the relevant result image.                                 */
                  RegContext,             /* Id of the registration context.                                  */
                  RegResult,              /* Id of the registration result.                                   */
                  DigId;                  /* Id of the digitizer used to read the images.                     */
   MIL_ID*        ImagesArray;            /* Ids of the images processed.                                     */
   HookDataStruct UserHookData;           /* User's processing function data structure.                       */


   /* Allocating the depth from focus registration objects. */
   RegContext = MregAlloc(MilSystem, M_DEPTH_FROM_FOCUS, M_DEFAULT, M_NULL);
   RegResult  = MregAllocResult(MilSystem, M_DEPTH_FROM_FOCUS_RESULT, M_NULL);

   /* Allocating the digitizer. */
   DigId = MdigAlloc(MilSystem, M_DEFAULT, ImageDirectory, M_EMULATED, M_NULL);

   /* Setting the registration context parameters. */
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_MODE, M_AVERAGE);
   MregControl(RegContext, M_DEFAULT, M_REGULARIZATION_SIZE, 7);
   MregControl(RegContext, M_DEFAULT, M_CONFIDENCE_MAP     , M_ENABLE);

   /* Allocating the sequence of images. */
   const MIL_INT ImageCount = 1;
   ImagesArray = new MIL_ID[ImageCount];
   for(MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
      {
      MbufAlloc2d(MilSystem, SizeX, SizeY, Type, Attribute, &ImagesArray[NumImg]);
      }

   /* Allocating the other buffer. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &IndexMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8 + M_UNSIGNED, Attribute, &DisplayedIndexMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT  , Attribute, &ConfidenceMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT  , Attribute, &DisplayedConfidenceMap);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 32 + M_FLOAT  , Attribute, &RelevantResult);

   /* Initialize the user's processing function data structure. */
   UserHookData.RegContext = RegContext;
   UserHookData.RegResult = RegResult;
   UserHookData.Display = MilDisplay;

   /* Reading the current image. */
   /* A pattern has been casted on the smooth surface of the object using a 
      high power structured lighting such as EFFI-Lase by ®Effilux.*/
   MosPrintf(MIL_TEXT("The images are processed when acquired.\n"));
   MosPrintf(MIL_TEXT("Load and processing in progress...\n\n"));
   MdigProcess(DigId, ImagesArray, ImageCount, M_SEQUENCE + M_COUNT(NbImages), M_DEFAULT, ProcessingFunction, &UserHookData);
   MosPrintf(MIL_TEXT("A stack of %d images has been processed.\n"), NbImages);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();

   /* Get the index map image. */
   MregDraw(M_DEFAULT, RegResult, IndexMap, M_DRAW_DEPTH_INDEX_MAP, M_DEFAULT, M_DEFAULT);

   /* Display the index map image. */
   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, IndexMap, DisplayedIndexMap);
   MosPrintf(MIL_TEXT("The resulting index map image is displayed.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Get the confidence map image. */
   MregDraw(M_DEFAULT, RegResult, ConfidenceMap, M_DRAW_DEPTH_CONFIDENCE_MAP, M_DEFAULT, M_DEFAULT);

   /* Display the confidence map image. */
   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, ConfidenceMap, DisplayedConfidenceMap);
   MosPrintf(MIL_TEXT("The resulting confidence map image is\n"));
   MosPrintf(MIL_TEXT("displayed.\n"));  
   MosPrintf(MIL_TEXT("Darker values correspond to lower confidence\n"));
   MosPrintf(MIL_TEXT("areas while brighter values correspond to\n"));
   MosPrintf(MIL_TEXT("highter confidence areas.\n"));
   MosPrintf(MIL_TEXT("Higher confidence areas indicate meaningful\n")); 
   MosPrintf(MIL_TEXT("index map areas.\n"));
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   /* Threshold the confidence map to keep the relevant result only. */
   MimBinarize(ConfidenceMap, RelevantResult, M_FIXED + M_GREATER, 2.6, M_NULL);

   /* Correct the index map result. */ 
   MimArith(IndexMap, RelevantResult, IndexMap, M_MULT);

   /* Display the corrected index map. */
   RemapDisplayRangeTo8Bits(MilSystem, MilDisplay, IndexMap, DisplayedIndexMap);
   MosPrintf(MIL_TEXT("Low confidence areas are masked and the\n"));
   MosPrintf(MIL_TEXT("resulting index map image is displayed.\n"));

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   /* Free buffers. */
   MbufFree(IndexMap);
   MbufFree(DisplayedIndexMap);
   MbufFree(ConfidenceMap);
   MbufFree(DisplayedConfidenceMap);
   MbufFree(RelevantResult);
   for(MIL_INT NumImg = 0; NumImg < ImageCount; NumImg++)
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


void RemapDisplayRangeTo8Bits(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilScrImage, MIL_ID MilDisplayedImage)
   {
   // Allocate a statistics context and result to compute source's min and max values.
   MIL_ID MilStatContext = MimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilStatContext, M_STAT_MIN, M_ENABLE);
   MimControl(MilStatContext, M_STAT_MAX, M_ENABLE);

   MIL_ID MilStatResult  = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);

   // Allocate a ramp LUT that will map the dynamic range.
   MIL_ID MilDynRangeLut = MbufAlloc1d(MilSystem, 256, 8 + M_UNSIGNED, M_LUT, M_NULL);

   // Compute source's minimum and maximum values.
   MimStatCalculate(MilStatContext, MilScrImage, MilStatResult, M_DEFAULT);

   // Obtain the source's minimum value.
   MIL_INT StatMin;
   MimGetResult(MilStatResult, M_STAT_MIN + M_TYPE_MIL_INT, &StatMin);

   // Obtain the source's maximum value.
   MIL_INT StatMax;
   MimGetResult(MilStatResult, M_STAT_MAX + M_TYPE_MIL_INT, &StatMax);

   // Define a ramp LUT mapping.
   MbufClear(MilDynRangeLut, 0.0);
   MgenLutRamp(MilDynRangeLut, StatMin, 1.0, StatMax, 255.0);

   // Perform the LUT mapping.
   MimLutMap(MilScrImage, MilDisplayedImage, MilDynRangeLut);

   // Free allocations.
   MbufFree(MilDynRangeLut);
   MimFree(MilStatResult);
   MimFree(MilStatContext);

   // Display the remapped image.
   MdispSelect(MilDisplay, MilDisplayedImage);
   }

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
