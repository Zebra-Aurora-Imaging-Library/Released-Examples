//***************************************************************************************
// 
// File name: BinaryBasedCircleDetection.cpp  
//
// Synopsis:  This example demontrates a method to find circles using grayscale
//            correlation.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("BinaryBasedCircleDetection\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to find circles that have a radius\n")
             MIL_TEXT("less than 255 pixels using a distance transform and grayscale\n"));
   MosPrintf(MIL_TEXT("correlation.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, graphic,\n")
             MIL_TEXT("image processing, pattern matching.\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

///*****************************************************************************
// Constants 
//*****************************************************************************
// Source image files specification. 
#define SOURCE_IMAGE_FILE M_IMAGE_PATH \
MIL_TEXT("BinaryBasedCircleDetection/BottleCaps.mim")

//Example defines
#define STRING_SIZE              128
#define IMAGE_THRESHOLD_VALUE    39L 
#define MIN_CIRCLE_RADIUS        2L
#define SYNTHETIC_CIRCLE_RADIUS  60L
#define CONE_RADIUS              20L

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID            MilApplication,        // Application identifier.
                     MilImage,              // Image identifier.
                     MilDisplay,            // Display identifier.
                     MilGraphics,           // Graphics identifier.
                     MilGraphicList,        // Graphics list identifier.
                     MilSystem;             // System identifier.     

   MIL_ID            MilBinarizedImage,     // Identifier for binary image.                     
                     MilDistanceImage8bit,  // Identifier for distance image.
                     MilDistanceImage16bit, // Identifier for distance image.
                     MilConeContext,        // Pattern matching context of cone model.
                     MilCircleImage,        // Image with synthetic circle to define the 
                                            // model
                     MilCircleDistanceImage8bit,   // Image 
                     MilCircleDistanceImage16bit,  // Image 
                     MilPatternResult;         // Results of pattern matching search

   MIL_INT           ImageSizeX,            // Size x of MilImage
                     ImageSizeY;            // Size y of MilImage

   // Allocate MIL objects. 
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED,
              &MilDisplay);

   // Print Header. 
   PrintHeader();

   // Restore source image into an automatically allocated image buffer. 
   MbufRestore(SOURCE_IMAGE_FILE, MilSystem, &MilImage);

   //Set the size x and y of the image
   ImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   ImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Allocate the binary image 
   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               8,
               M_IMAGE+M_PROC+M_DISP,
               &MilBinarizedImage);

   // Allocate the distance image
   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               8,
               M_IMAGE+M_PROC+M_DISP,
               &MilDistanceImage8bit);

   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               16,
               M_IMAGE+M_PROC+M_DISP,
               &MilDistanceImage16bit);

   // Display the image buffer. 
   MdispSelect(MilDisplay, MilImage);

   // Allocate a graphic context
   MgraAlloc(MilSystem, &MilGraphics);

   // Set the graphics mode to transparent.
   MgraControl(MilGraphics, M_BACKGROUND_MODE, M_TRANSPARENT);

   // Allocate a graphic list associated to the display
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   // Associate the graphics list to the display
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   MosPrintf(MIL_TEXT("The bottle caps in the displayed image will be found using the \n")
             MIL_TEXT("following steps:\n\n"));
   MosPrintf(MIL_TEXT("1. Binarization.\n"));
   MosPrintf(MIL_TEXT("2. Distance transform.\n"));
   MosPrintf(MIL_TEXT("3. Grayscale pattern matching using the distance transform \n")
             MIL_TEXT("   of a synthetic disk as a model.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Binarize the image
   MimBinarize(MilImage, MilBinarizedImage, M_FIXED+M_GREATER_OR_EQUAL,
                                       IMAGE_THRESHOLD_VALUE, M_NULL);

   //Eliminate small blobs
   MimOpen(MilBinarizedImage, MilBinarizedImage, MIN_CIRCLE_RADIUS, M_BINARY);
   MimClose(MilBinarizedImage, MilBinarizedImage, MIN_CIRCLE_RADIUS, M_BINARY);

   // Display the image buffer. 
   MdispSelect(MilDisplay, MilBinarizedImage);

   MosPrintf(MIL_TEXT("The binarized image is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Apply a distance transform
   //Note: even though the result is put into a 16-bit buffer, the maximum distance should
   //not exceed 255.
   MimDistance(MilBinarizedImage, MilDistanceImage16bit, M_CHAMFER_3_4);

   //Copy to an 8 bit buffer
   MbufCopy(MilDistanceImage16bit, MilDistanceImage8bit);

   // Display the image buffer. 
   MdispSelect(MilDisplay, MilDistanceImage16bit);

   MosPrintf(MIL_TEXT("The distance transform is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //Allocate the synthetic circle image
   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               8,
               M_IMAGE+M_PROC+M_DISP,
               &MilCircleImage);

   //Allocate the circle distance image
   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               8,
               M_IMAGE+M_PROC+M_DISP,
               &MilCircleDistanceImage8bit);

   MbufAlloc2d(MilSystem,
               ImageSizeX,
               ImageSizeY,
               16,
               M_IMAGE+M_PROC+M_DISP,
               &MilCircleDistanceImage16bit);

   //Clear the circle image
   MbufClear(MilCircleImage, 0);

   // Set the text color to green
   MgraColor(MilGraphics, M_COLOR_WHITE);

   MIL_INT CircleCenterX = (ImageSizeX-1)/2;
   MIL_INT CircleCenterY = (ImageSizeY-1)/2;

   //Draw the circle
   MgraArcFill(MilGraphics, MilCircleImage, CircleCenterX, CircleCenterY,
         SYNTHETIC_CIRCLE_RADIUS, SYNTHETIC_CIRCLE_RADIUS, 0, 360);

   //Apply a distance transform to the synthetic circle
   MimDistance(MilCircleImage, MilCircleDistanceImage16bit, M_CHAMFER_3_4);

   //Copy the result to an 8 bit buffer
   MbufCopy(MilCircleDistanceImage16bit, MilCircleDistanceImage8bit);

   //Allocate the model from the distance image
   MIL_INT ModelOffsetX = CircleCenterX - CONE_RADIUS;
   MIL_INT ModelOffsetY = CircleCenterY - CONE_RADIUS;
   MIL_INT ModelSizeX = CONE_RADIUS*2;
   MIL_INT ModelSizeY = CONE_RADIUS*2;

   MpatAlloc(MilSystem, M_NORMALIZED, M_DEFAULT, &MilConeContext);
   MpatDefine(MilConeContext, M_REGULAR_MODEL, MilCircleDistanceImage8bit,
              ModelOffsetX, ModelOffsetY,
              ModelSizeX,   ModelSizeY, M_DEFAULT);

   //Set the number of occurrences to find to ALL
   MpatControl(MilConeContext, 0, M_NUMBER, M_ALL);

   // Preprocess the model.
   MpatPreprocess(MilConeContext, M_DEFAULT, MilDistanceImage8bit);

   //Allocate pattern matching result object
   MpatAllocResult(MilSystem, M_DEFAULT, &MilPatternResult);

   //Find the occurrences
   MpatFind(MilConeContext, MilDistanceImage8bit, MilPatternResult);

   //Get the number of occurrences
   MIL_INT NumOccurences;
   MpatGetResult(MilPatternResult, M_GENERAL, M_NUMBER+M_TYPE_MIL_INT, &NumOccurences);

   if (NumOccurences > 0)
      {
      // Set the text color to green
      MgraColor(MilGraphics, M_COLOR_GREEN);

      //Draw a box arround the occurrences
      MpatDraw(MilGraphics, MilPatternResult, MilGraphicList, M_DRAW_BOX+M_DRAW_POSITION,
           M_DEFAULT, M_DEFAULT);

      MIL_DOUBLE* PositionX = new MIL_DOUBLE[NumOccurences];
      MIL_DOUBLE* PositionY = new MIL_DOUBLE[NumOccurences];

      //Get the position of each bottle cap
      MpatGetResult(MilPatternResult, M_ALL, M_POSITION_X, PositionX);
      MpatGetResult(MilPatternResult, M_ALL, M_POSITION_Y, PositionY);

      MIL_TEXT_CHAR DistanceString[STRING_SIZE];

      //Get the distance at the found location of each bottle cap and display it
      for (MIL_INT i = 0; i<NumOccurences; i++)
         {
         char Distance;
         MbufGet2d(MilDistanceImage8bit, (MIL_INT)(PositionX[i]),
              (MIL_INT)(PositionY[i]), 1, 1, &Distance);

         MosSprintf(DistanceString, STRING_SIZE, MIL_TEXT("d=%d"), (MIL_INT)Distance);
         MgraText(MilGraphics, MilGraphicList, PositionX[i], PositionY[i]+20,
                  DistanceString);
         }

      MosPrintf(MIL_TEXT("A pattern matching model has been defined using the ")
                MIL_TEXT("distance\ntransform of a synthetic disk, and is used to ")
                MIL_TEXT("detect the caps.\n\n"));
      MosPrintf(MIL_TEXT("%d bottle caps have been found.  The locations and ")
                MIL_TEXT("the\ncorresponding distances are shown.\n\n"),
                NumOccurences);
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      //Display the positions and radii
      MgraClear(M_DEFAULT, MilGraphicList);
      MgraColor(MilGraphics, M_COLOR_RED);

      MdispSelect(MilDisplay, MilImage);

      const MIL_DOUBLE StartAngle = 330;
      const MIL_DOUBLE EndAngle = 30;

      for (MIL_INT i=0; i<NumOccurences; i++)
         {
         char Distance;
         MbufGet2d(MilDistanceImage8bit, (MIL_INT)(PositionX[i]),
               (MIL_INT)(PositionY[i]), 1, 1, &Distance);

         MgraLine(MilGraphics, MilGraphicList, PositionX[i], PositionY[i],
               PositionX[i]+Distance, PositionY[i]);
         MgraArc(MilGraphics, MilGraphicList, PositionX[i], PositionY[i], Distance,
               Distance, StartAngle, EndAngle);
         }

      delete [] PositionX;
      delete [] PositionY;

      MosPrintf(MIL_TEXT("The position and approximate radius of each bottle cap is displayed\n")
                MIL_TEXT("in the original image.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();

      }
   else
      {
      MosPrintf(MIL_TEXT("Could not find the bottle caps!\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   // Free MIL objects
   MbufFree(MilCircleImage);
   MbufFree(MilCircleDistanceImage8bit);
   MbufFree(MilCircleDistanceImage16bit);
   MbufFree(MilBinarizedImage);
   MbufFree(MilDistanceImage8bit);
   MbufFree(MilDistanceImage16bit);
   MbufFree(MilImage);
   MpatFree(MilConeContext);
   MpatFree(MilPatternResult);
   MgraFree(MilGraphics);
   MgraFree(MilGraphicList);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   return 0;
   }

