/*****************************************************************************/
/* 
 * File name: MGraInteractive.cpp
 *
 * Synopsis:  This program uses the capabilities of MIL's interactive
 *            graphics, along with the Blob analysis module, to count 
 *            objects within a user-defined region.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>

/* Data structure to handle parameters for the hook function. */
struct STestParameters
   {
   MIL_ID   MilDisplay,
            MilGraphicsList,
            MilGraphicsContext,
            MilBinImage,
            MilBlobContext,
            MilBlobResult;
   MIL_INT  RegionLabel;
   };

/* Example function declarations. */ 
void           InteractivityExample(MIL_ID MilSystem, MIL_ID MilDisplay);

MIL_INT MFTYPE HookHandler(MIL_INT HookType, MIL_ID EventId, void* UserData);

void           CountObjects(MIL_ID MilDisplay, MIL_ID MilGraphicsList,
                            MIL_ID MilGraphicsContext, MIL_ID MilBinImage,
                            MIL_ID MilBlobContext, MIL_ID MilBlobResult);

/*****************************************************************************
 Main.
 *****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,     /* Application identifier. */
          MilSystem,          /* System Identifier.      */
          MilDisplay;         /* Display identifier.     */

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, 
      M_NULL, M_NULL);

   /* Print example name. */ 
   MosPrintf(MIL_TEXT("\nINTERACTIVE REGIONS AND SUBPIXEL ANNOTATIONS:\n"));
   MosPrintf(MIL_TEXT("---------------------------------------------\n\n"));
   MosPrintf(MIL_TEXT("This program determines the number of blobs in a region\n"));
   MosPrintf(MIL_TEXT("defined interactively by the user. The extracted blob's\n"));
   MosPrintf(MIL_TEXT("features are drawn with subpixel accuracy in a zoomable\n"));
   MosPrintf(MIL_TEXT("display.\n\n"));

   /* Run Interactivity Example. */
   InteractivityExample(MilSystem, MilDisplay);
 
   /* Free defaults. */    
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);
   
   return 0;
   }

/*****************************************************************************/
/* Interactivity example. */

/* Source MIL image file specification. */ 
#define IMAGE_FILE M_IMAGE_PATH MIL_TEXT("Seals.mim")
/* Threshold for image binarization. */
#define IMAGE_THRESHOLD_VALUE 110L 
/* Initial region parameters. */
#define RECTANGLE_POSITION_X 160
#define RECTANGLE_POSITION_Y 310
#define RECTANGLE_WIDTH      200
#define RECTANGLE_HEIGHT     175
#define RECTANGLE_ANGLE      0
/* Interactivity parameters. */
#define SELECTION_RADIUS     10
/* Value of enter key in Ascii table */
#define ASCII_ENTER 13

void InteractivityExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID  MilImage,                  /* Image buffer identifier.        */
           MilGraphicsList,           /* Graphics list identifier.       */
           MilGraphicsContext,        /* Graphics context identifier.    */
           MilBinImage,               /* Binary image buffer identifier. */
           MilBlobContext,        	  /* Context identifier.             */
           MilBlobResult;             /* Blob result buffer identifier.  */

   MIL_INT SizeX,                     /* Size X of the source buffer.    */
           SizeY,                     /* Size Y of the source buffer.    */
           RegionLabel;               /* Label value of the region.      */

   STestParameters DataStructure;     /* Hook function data structure.   */
   
   /* Restore the source image. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);

   /* Display the buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Allocate a graphics list to hold the subpixel annotations. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicsList);

   /* Increase the selection radius for easier interactivity. */
   MgraControlList(MilGraphicsList, M_LIST, M_DEFAULT, M_SELECTION_RADIUS,
      SELECTION_RADIUS);

   /* Associate the graphics list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicsList);

   /* Allocate a graphics context for the draw operations. */
   MgraAlloc(MilSystem, &MilGraphicsContext);

   /* Enable the interactive mode. */
   MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);

   /* Enable the use of action keys */
   MgraControlList(MilGraphicsList, M_LIST, M_DEFAULT, M_ACTION_KEYS, M_ENABLE);

   /* Add a selectable rectangular region.*/
   MgraRectAngle(MilGraphicsContext, MilGraphicsList, RECTANGLE_POSITION_X, 
      RECTANGLE_POSITION_Y, RECTANGLE_WIDTH, RECTANGLE_HEIGHT, RECTANGLE_ANGLE,
      M_CENTER_AND_DIMENSION);
   
   /* Retrieve the label of the rectangle graphic. */
   MgraInquireList(MilGraphicsList, M_LIST, M_DEFAULT, M_LAST_LABEL, &RegionLabel);

   /* Modify the line thickness of the rectangle. */
   MgraControlList(MilGraphicsList, M_GRAPHIC_LABEL(RegionLabel), M_DEFAULT, M_LINE_THICKNESS, 5.0);

   /* Disable the selectable mode for the next annotations to the graphics list. */
   MgraControl(MilGraphicsContext, M_SELECTABLE, M_DISABLE);

   /* Allocate a binary image buffer for fast processing. */
   MbufInquire(MilImage, M_SIZE_X, &SizeX);
   MbufInquire(MilImage, M_SIZE_Y, &SizeY);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);
 
   /* Binarize the source image. */
   MimBinarize(MilImage, MilBinImage, M_FIXED+M_LESS, IMAGE_THRESHOLD_VALUE, M_NULL);
   
   /* Allocate a blob context and a blob result. */ 
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobResult);

   /* Select the blob features to calculate (Center Of Gravity and Box). */
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
   MblobControl(MilBlobContext, M_BOX, M_ENABLE);

   /* Programmatically initialize the selected state of the rectangle region. */
   MgraControlList(MilGraphicsList, M_GRAPHIC_LABEL(RegionLabel), M_DEFAULT, 
      M_GRAPHIC_SELECTED, M_TRUE);

   /* Perform and display a first count of the number of objects */
   /* within the initial region.                                 */
   CountObjects(MilDisplay, MilGraphicsList, MilGraphicsContext, MilBinImage,
      MilBlobContext, MilBlobResult);

   /* Initialize the hook data structure, then associate the hook function to */ 
   /* the "M_GRAPHIC_MODIFIED" event. The hook function will be called        */
   /* with any region interaction by the user.                                */
   DataStructure.MilDisplay         = MilDisplay;
   DataStructure.MilGraphicsList    = MilGraphicsList;
   DataStructure.MilGraphicsContext = MilGraphicsContext;
   DataStructure.MilBinImage        = MilBinImage;
   DataStructure.RegionLabel        = RegionLabel;
   DataStructure.MilBlobContext     = MilBlobContext;
   DataStructure.MilBlobResult      = MilBlobResult;
      
   MgraHookFunction(MilGraphicsList, M_GRAPHIC_MODIFIED, &HookHandler, &DataStructure);

   MosPrintf(MIL_TEXT("You can try using your mouse or your keyboard to interactively\n"));
   MosPrintf(MIL_TEXT("modify the displayed region, such as moving, resizing, or\n"));
   MosPrintf(MIL_TEXT("rotating the region. If you do so, the results and annotations\n"));
   MosPrintf(MIL_TEXT("will be immediately updated.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to exit.\n"));
   MIL_INT PressedKey = 0;
   while(PressedKey != ASCII_ENTER)
      PressedKey = MosGetch();

   MgraHookFunction(MilGraphicsList, M_GRAPHIC_MODIFIED+M_UNHOOK,
      &HookHandler, &DataStructure);

   /* Free all allocations. */
   MblobFree(MilBlobResult); 
   MblobFree(MilBlobContext); 
   MbufFree(MilBinImage);
   MgraFree(MilGraphicsContext);
   MgraFree(MilGraphicsList);
   MbufFree(MilImage);
   }


MIL_INT MFTYPE HookHandler(MIL_INT HookType, MIL_ID EventId, void* UserData)
   {
   STestParameters* pDataStructure = (STestParameters*)(UserData);

   /* Check that the modified graphic is the rectangular region. */
   MIL_INT ModifiedGraphicLabel;
   MgraGetHookInfo(EventId, M_GRAPHIC_LABEL_VALUE, &ModifiedGraphicLabel);

   if (ModifiedGraphicLabel == pDataStructure->RegionLabel)
      {
      /* Count objects and draw the corresponding annotations. */
      CountObjects(pDataStructure->MilDisplay,
                   pDataStructure->MilGraphicsList, 
                   pDataStructure->MilGraphicsContext,
                   pDataStructure->MilBinImage, 
                   pDataStructure->MilBlobContext, 
                   pDataStructure->MilBlobResult);
      }

   return M_NULL;
   }


#define MAX_TEXT_SIZE 100

void CountObjects(MIL_ID MilDisplay, MIL_ID MilGraphicsList, MIL_ID MilGraphicsContext,
                  MIL_ID MilBinImage, MIL_ID MilBlobContext, MIL_ID MilBlobResult)
   {
   MIL_INT NumberOfBlobs,
           NumberOfPrimitives,
           Index;

   MIL_TEXT_CHAR TextLabel[MAX_TEXT_SIZE];

   /* Disable the display update for better performance. */
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
      
   /* Remove all elements from the graphics list, except the rectangle */
   /* region primitive at index 0.                                     */ 
   MgraInquireList(MilGraphicsList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS,
      &NumberOfPrimitives);
   for(Index = NumberOfPrimitives-1; Index > 0; Index--)
      {
      MgraControlList(MilGraphicsList, M_GRAPHIC_INDEX(Index), M_DEFAULT, M_DELETE,
         M_DEFAULT);
      }

   /* Set the input region. The blob analysis will be done */
   /* from the (filled) interactive rectangle.             */
   MbufSetRegion(MilBinImage, MilGraphicsList, M_DEFAULT,
      M_RASTERIZE + M_FILL_REGION + M_USE_LINE_THICKNESS_1, M_DEFAULT);
      
   /* Calculate the blobs and their features. */ 
   MblobCalculate(MilBlobContext, MilBinImage, M_NULL, MilBlobResult);

   /* Get the total number of blobs. */ 
   MblobGetResult(MilBlobResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NumberOfBlobs);

   /* Set the input units to display unit for the count annotations. */ 
   MgraControl(MilGraphicsContext, M_INPUT_UNITS, M_DISPLAY);
   MosSprintf(TextLabel, MAX_TEXT_SIZE, MIL_TEXT(" Number of blobs found: %2i "),
      (int) NumberOfBlobs);

   MgraColor(MilGraphicsContext, M_COLOR_WHITE);
   MgraText(MilGraphicsContext, MilGraphicsList, 10, 10, TextLabel);
      
   /* Restore the input units to pixel units for result annotations. */ 
   MgraControl(MilGraphicsContext, M_INPUT_UNITS, M_PIXEL);

   /* Draw blob center of gravity annotation. */
   MgraControl(MilGraphicsContext, M_LINE_THICKNESS, 3.0);
   MgraColor(MilGraphicsContext, M_COLOR_RED);
   MblobDraw(MilGraphicsContext, MilBlobResult, MilGraphicsList, M_DRAW_CENTER_OF_GRAVITY, 
      M_INCLUDED_BLOBS, M_DEFAULT);

   /* Draw blob bounding box annotations. */
   MgraColor(MilGraphicsContext, M_COLOR_GREEN);
   MgraControl(MilGraphicsContext, M_LINE_THICKNESS, 1.0);
   MblobDraw(MilGraphicsContext, MilBlobResult, MilGraphicsList, M_DRAW_BOX, 
      M_INCLUDED_BLOBS, M_DEFAULT);

   /* Enable the display to update the drawings. */
   MdispControl(MilDisplay, M_UPDATE, M_ENABLE);
   }
