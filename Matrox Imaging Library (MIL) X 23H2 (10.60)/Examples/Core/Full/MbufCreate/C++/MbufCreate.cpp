//*******************************************************************************/
/*
* File name: MbufCreate.cpp
*
* Synopsis:  This program shows how to use MbufCreate function to
*            create a MIL buffer on user data or on the data of another 
*            MIL buffer.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>

/* Target image size. */
#define IMAGE_SIZE_X 512
#define IMAGE_SIZE_Y 512

/* MIL buffer creation example for a monochrome buffer. */
void MonochromeBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* MIL buffer creation example for a color packed buffer. */
void ColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* MIL buffer creation example for a color planar buffer. */
void ColorPlanarBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* MIL buffer creation example for monochrome buffer on packed buffer. */
void MonochromeOnColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* Utility functions. */
MIL_UINT Mandelbrot(MIL_INT PosX, MIL_INT PosY,
                    MIL_DOUBLE RefX, MIL_DOUBLE RefY, MIL_DOUBLE Dim);
MIL_UINT8 GetColorFromIndex(MIL_INT Band, MIL_INT Index, MIL_INT MaxIndex);

int MosMain(void)
   {
   MIL_ID  MilApplication, /* Application identifier.   */
           MilSystem,      /* System identifier.        */
           MilDisplay;     /* Display identifier.       */

   MosPrintf(MIL_TEXT("\nMIL BUFFER CREATION:\n"));
   MosPrintf(MIL_TEXT("--------------------\n\n"));
   MosPrintf(MIL_TEXT("This example shows how to use the MbufCreate functions\n"));
   MosPrintf(MIL_TEXT("to create a MIL buffer from the memory at a specified location\n"));
   MosPrintf(MIL_TEXT("by pointing to the address of user data or the identifier of an\n"));
   MosPrintf(MIL_TEXT("already existing MIL buffer.\n\n"));

   /* Allocate default objects. */
   MappAlloc(M_DEFAULT, &MilApplication);
   MilSystem = M_DEFAULT_HOST;
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* MIL buffer creation example for a monochrome buffer. */
   MonochromeBufCreateExample(MilSystem, MilDisplay);

   /* MIL buffer creation example for a color packed buffer. */
   ColorPackedBufCreateExample(MilSystem, MilDisplay);

   /* MIL buffer creation example for a color planar buffer. */
   ColorPlanarBufCreateExample(MilSystem, MilDisplay);

   /* MIL buffer creation examples for a monochrome buffer on packed buffer. */
   MonochromeOnColorPackedBufCreateExample(MilSystem, MilDisplay);

   /* Free allocated objects. */
   MdispFree(MilDisplay);
   MappFree(MilApplication);

   return 0;
   }

/* MIL buffer creation example for a monochrome buffer. */
/* -----------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF1 -0.500
#define Y_REF1 +0.002
#define DIM1   +3.200

void MonochromeBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_UINT8* UserImagePtr;         /* User image pointer.      */
   MIL_ID     MilImage;             /* Image buffer identifier. */
   MIL_INT    i, x, y;              /* Buffer access variables. */
   MIL_UINT   Value;                /* Value to write.          */

   MosPrintf(MIL_TEXT("- A monochrome MIL buffer was created by pointing to \n")
             MIL_TEXT("  the address of user data. The buffer was modified directly\n")
             MIL_TEXT("  using the user data pointer.\n\n"));

   /* Allocate a monochrome user array. */
   UserImagePtr = new MIL_UINT8[IMAGE_SIZE_X * IMAGE_SIZE_Y];

   /* Create a MIL monochrome image buffer on the user array. */
   MbufCreate2d(MilSystem, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP, M_HOST_ADDRESS + M_PITCH, M_DEFAULT,
      UserImagePtr, &MilImage);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* Initialize the user array's value index. */
   i=0;

   /* For each row. */
   for (y = 0; y < IMAGE_SIZE_Y; y++)
      {
      /* For each column. */
      for (x = 0; x < IMAGE_SIZE_X; x++)
         {
         /* Calculate the pixel value. */
         Value = Mandelbrot(x, y, X_REF1, Y_REF1, DIM1);

         /* Write the pixel using its pointer. */
         UserImagePtr[i] = MIL_UINT8(Value);

         /* Increment the user array's value index. */
         i++;
         }
      }

   /* Signal MIL that the buffer data has been updated. */
   MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

   /* Unlock buffer. */
   MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

   /* Select to display. */
   MdispSelect(MilDisplay, MilImage);

   /* Print a message and wait for a key. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
   };

/* MIL buffer creation example for a color packed buffer. */
/* -------------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF2 -1.1355
#define Y_REF2 -0.2510
#define DIM2   +0.1500

/* Utility to pack B,G,R values into 32 bits integer. */
MIL_UINT32 PackToBGR32(MIL_UINT8 b, MIL_UINT8 g, MIL_UINT8 r) 
   {
   return ((MIL_UINT32)b | (MIL_UINT32)(g << 8) | (MIL_UINT32)(r << 16));
   };

void ColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_UINT32* UserImagePtr;        /* User image pointer.      */
   MIL_ID      MilImage;            /* Image buffer identifier. */
   MIL_INT     i, x, y;             /* Buffer access variables. */
   MIL_UINT    Value;               /* Equation Value.          */
   MIL_UINT32  Value_BGR32;         /* Color value to write.    */

   MosPrintf(MIL_TEXT("- A 32-bit color packed MIL buffer was created by pointing to\n")
             MIL_TEXT("  the address of user data. The buffer was modified directly.\n")
             MIL_TEXT("  using the user data pointer.\n\n"));

   /* Allocate a packed color user array. */
   UserImagePtr = new MIL_UINT32[IMAGE_SIZE_X * IMAGE_SIZE_Y];
   
   /* Create a MIL packed color image buffer on the user array. */
   MbufCreateColor(MilSystem, 3, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP + M_BGR32 + M_PACKED,
      M_HOST_ADDRESS + M_PITCH, M_DEFAULT, (void**)(&UserImagePtr), &MilImage);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* Initialize the user array's value index. */
   i=0;

   /* For each row. */
   for (y = 0; y < IMAGE_SIZE_Y; y++)
      {
      /* For each column. */
      for (x = 0; x < IMAGE_SIZE_X; x++)
         {
         /* Calculate the pixel value. */
         Value = Mandelbrot(x, y, X_REF2, Y_REF2, DIM2);

         Value_BGR32 = PackToBGR32(
            GetColorFromIndex(M_BLUE, Value, 255),
            GetColorFromIndex(M_GREEN, Value, 255),
            GetColorFromIndex(M_RED, Value, 255));

         /* Write the color pixel using its pointer. */
         UserImagePtr[i] = Value_BGR32;

         /* Increment the user array's value index. */
         i++;
         }
      }

   /* Signal MIL the buffer data has been updated. */
   MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

   /* Unlock buffer. */
   MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

   /* Select to display. */
   MdispSelect(MilDisplay, MilImage);

   /* Print a message and wait for a key. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
   };

/* MIL buffer creation example for a color planar buffer. */
/* -------------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF3 -0.7453
#define Y_REF3 +0.1127
#define DIM3   +0.0060

void ColorPlanarBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_UINT8*  UserBandPtr[3];          /* User image band pointer. */
   MIL_ID      MilImage;                /* Image buffer identifier. */
   MIL_INT     x, y, i, b;              /* Buffer access variables. */
   MIL_UINT    Value;                   /* Value write.             */
   MIL_UINT    ColorBand[3] =    { M_RED, M_GREEN, M_BLUE };

   MosPrintf(MIL_TEXT("- A 24-bit color planar MIL buffer was created by pointing to\n")
             MIL_TEXT("  the addresses of 3 user data arrays. The buffers were modified\n")
             MIL_TEXT("  directly using the user data pointers.\n\n"));

   /* Allocate three user arrays representing the 3 bands of a color image. */
   for(b = 0; b < 3; b++)
      UserBandPtr[b] = new MIL_UINT8[IMAGE_SIZE_X*IMAGE_SIZE_Y];

   /* Create a MIL planar color image buffer on the user arrays. */
   MbufCreateColor(MilSystem, 3, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP + M_PLANAR + M_RGB24,
      M_HOST_ADDRESS + M_PITCH, M_DEFAULT, (void**)UserBandPtr, &MilImage);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* For each color band. */
   for (b = 0; b < 3; b++)
      {
      /* Initialize the user array's value index. */
      i=0;

      /* For each row. */
      for (y = 0; y < IMAGE_SIZE_Y; y++)
         {
         /* For each column. */
         for (x = 0; x < IMAGE_SIZE_X; x++)
            {
            /* Calculate the pixel value. */
            Value = Mandelbrot(x, y, X_REF3, Y_REF3, DIM3);

            /* Write the color pixel using its pointer. */
            UserBandPtr[b][i] = GetColorFromIndex(ColorBand[b], Value, 255);

            /* Increment the user array's value index. */
            i++;
            }
         }
      }

   /* Signal MIL the buffer data has been updated. */
   MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

   /* Unlock buffer. */
   MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

   /* Select to display. */
   MdispSelect(MilDisplay, MilImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
   };

/* MIL buffer creation example for monochrome buffer on packed buffer. */
/* --------------------------------------------------------------------*/

/* Target MIL image file specifications. */
#define COLOR_IMAGE_FILE            M_IMAGE_PATH MIL_TEXT("BaboonRGB.mim")

/* Source image information. */
#define SOURCE_SIZE_X          256
#define SOURCE_SIZE_Y          256
#define KEPT_BITS              0x80

/* Maximum number of events. */
#define MAX_NB_EVENTS         (SOURCE_SIZE_X * SOURCE_SIZE_Y)

/* Display zoom. */
#define DISPLAY_ZOOM           3.0

/* Color value to replace. */
#define SOURCE_RED      128
#define SOURCE_GREEN    0
#define SOURCE_BLUE     0
#define DEST_RED        0
#define DEST_GREEN      255
#define DEST_BLUE       0

/* BGR32 mask. */
#define BGR2_MASK       0X00FFFFFF

void MonochromeOnColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilGraList;              /* Graphic list identifier.                    */
   MIL_ID      MilGraCtx;               /* Graphic context.                            */
   MIL_ID      MilImage;                /* Image buffer identifier.                    */
   MIL_ID      MilDestImage;            /* Destination image buffer identifier.        */
   MIL_ID      MilDispImage;            /* Display image buffer identifier.            */
   MIL_ID      MilMonoImage;            /* Created monochrome image buffer identifier. */
   MIL_UINT    MonoColorValue;          /* Monochrome color value to replace.          */
   MIL_ID      MilEventResult;          /* Locate event result buffer.                 */
   MIL_INT     NbEvents;                /* Number of events located in the image.      */
   MIL_INT*    EventXPtr;               /* Pointer to array of x coordinates.          */
   MIL_INT*    EventYPtr;               /* Pointer to array of y coordinates.          */

   MosPrintf(MIL_TEXT("- A 32-bit monochrome MIL buffer was created by pointing to\n")
             MIL_TEXT("  the identifier of a MIL packed color buffer. This was done to\n")
             MIL_TEXT("  use the newly created buffer with a function that requires \n")
             MIL_TEXT("  monochrome image buffers. In this example, the positions of\n")
             MIL_TEXT("  dark red pixels found using MimLocateEvent() are displayed in\n")
             MIL_TEXT("  green.\n\n"));

   /* Allocate a graphic context. */
   MgraAlloc(MilSystem, &MilGraCtx);

   /* Allocate a graphic list and associate it to the display. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   /* Allocate the display image and the child buffers of the source and destination. */
   MbufAllocColor(MilSystem, 3, SOURCE_SIZE_X * 2, SOURCE_SIZE_Y, 8 + M_UNSIGNED,
                  M_BGR32 + M_PACKED + M_IMAGE + M_PROC + M_DISP, &MilDispImage);
   MbufChild2d(MilDispImage, 0, 0, SOURCE_SIZE_X, SOURCE_SIZE_Y, &MilImage);
   MbufChild2d(MilDispImage, SOURCE_SIZE_X, 0, SOURCE_SIZE_X, SOURCE_SIZE_Y, &MilDestImage);

   /* Allocate the event result buffer. */ 
   MimAllocResult(MilSystem, MAX_NB_EVENTS, M_EVENT_LIST, &MilEventResult);
   EventXPtr = new MIL_INT[MAX_NB_EVENTS];
   EventYPtr = new MIL_INT[MAX_NB_EVENTS];

   /* Restore the color image and preprocess it to reduce the number of colors. */
   MbufLoad(COLOR_IMAGE_FILE, MilImage);
   MimArith(MilImage, KEPT_BITS, MilImage, M_AND_CONST);
   MbufCopy(MilImage, MilDestImage);

   /* Create a monochrome buffer on the memory of the color image. */
   MbufCreateColor(MilSystem, 1, M_DEFAULT, M_DEFAULT, 32 + M_UNSIGNED, M_IMAGE + M_PROC,
                   M_MIL_ID + M_PITCH, M_DEFAULT, (void**)&MilImage, &MilMonoImage);

   /* Locate the coordinates of pixels of a certain BGR packed color value with MimLocateEvent. */
   MonoColorValue = PackToBGR32(SOURCE_BLUE, SOURCE_GREEN, SOURCE_RED);

   /* The last "don't care" byte in the BGR32 buffer must be masked so the event is
      detected correctly. */
   MimArith(MilMonoImage, BGR2_MASK, MilMonoImage, M_AND_CONST);

   MimLocateEvent(MilMonoImage,MilEventResult, M_EQUAL, (MIL_DOUBLE)MonoColorValue, M_NULL);
   MimGetResult(MilEventResult, M_NB_EVENT, &NbEvents);
   MimGetResult(MilEventResult, M_POSITION_X + M_TYPE_MIL_INT, EventXPtr);
   MimGetResult(MilEventResult, M_POSITION_Y + M_TYPE_MIL_INT, EventYPtr);

   /* Mark the located pixels over the destination image. */
   MgraColor(MilGraCtx, M_RGB888(DEST_RED, DEST_GREEN, DEST_BLUE));
   MgraControl(MilGraCtx, M_DRAW_OFFSET_X, -SOURCE_SIZE_X);
   MgraDots(MilGraCtx, MilGraList, NbEvents, EventXPtr, EventYPtr, M_DEFAULT);

   /* Select to display. */
   MdispZoom(MilDisplay, DISPLAY_ZOOM, DISPLAY_ZOOM);
   MdispSelect(MilDisplay, MilDispImage);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Remove the zoom. */
   MdispZoom(MilDisplay, 1.0, 1.0);

   /* Free event position arrays. */
   delete [] EventYPtr;
   delete [] EventXPtr;

   /* Free allocation. */
   MbufFree(MilMonoImage);
   MimFree(MilEventResult);
   MbufFree(MilDestImage);
   MbufFree(MilImage);
   MbufFree(MilDispImage);
   MgraFree(MilGraList);
   MgraFree(MilGraCtx);
   };

/* Mandelbrot fractal utility functions. */
MIL_INT    MinVal(MIL_INT a, MIL_INT b)    { return (a > b) ? b : a;    }
MIL_DOUBLE Remap(MIL_DOUBLE pos, MIL_DOUBLE size, MIL_DOUBLE min, MIL_DOUBLE max)
   {
   return ( (((max-min) / size) * pos) + min );
   }

MIL_UINT Mandelbrot(MIL_INT PosX, MIL_INT PosY,
                    MIL_DOUBLE RefX, MIL_DOUBLE RefY, MIL_DOUBLE Dim)
   {
   const MIL_UINT maxIter = 256;
   MIL_DOUBLE xMin = RefX - (0.5 * Dim);
   MIL_DOUBLE xMax = RefX + (0.5 * Dim);
   MIL_DOUBLE yMin = RefY - (0.5 * Dim);
   MIL_DOUBLE yMax = RefY + (0.5 * Dim);
   MIL_DOUBLE x0 = Remap((MIL_DOUBLE)PosX, (MIL_DOUBLE)IMAGE_SIZE_X, xMin, xMax);
   MIL_DOUBLE y0 = Remap((MIL_DOUBLE)PosY, (MIL_DOUBLE)IMAGE_SIZE_Y, yMin, yMax);
   MIL_DOUBLE x = 0.0;
   MIL_DOUBLE y = 0.0;
   MIL_UINT Iter = 0;

   while (((x*x + y*y) < 4) && (Iter < maxIter))
      {
      MIL_DOUBLE Temp = x*x - y*y + x0;
      y = 2 * x*y + y0;
      x = Temp;
      Iter++;
      }

   return MinVal(255, Iter);
   }

/* Calculate color from index. */
MIL_UINT8 GetColorFromIndex(MIL_INT Band, MIL_INT Index, MIL_INT MaxIndex)
   {
   MIL_UINT8* Segments = M_NULL;
   MIL_UINT8  SegmentsR[] =    {   0,   0,   0, 255, 255, 128 };
   MIL_UINT8  SegmentsG[] =    {   0,   0, 255, 255,   0,   0 };
   MIL_UINT8  SegmentsB[] =    { 128, 255, 255,   0,   0,   0 };

   switch (Band)
      {
      case M_RED:
         Segments = SegmentsR;
         break;
      case M_GREEN:
         Segments = SegmentsG;
         break;
      case M_BLUE:
         Segments = SegmentsB;
         break;
      }

   MIL_DOUBLE RemapedIndex = Index * MaxIndex / 256.0;
   MIL_UINT8  SegmentIndex = MIL_UINT8(RemapedIndex * 5.0 / 256.0);
   MIL_DOUBLE Slope = (Segments[SegmentIndex + 1] - Segments[SegmentIndex]) / (256.0 / 5.0);
   MIL_DOUBLE Offset = (Segments[SegmentIndex] - (Slope * SegmentIndex * 256.0 / 5.0));
   MIL_UINT8  Value = MIL_UINT8(Slope * RemapedIndex + Offset + 0.5);

   return Value;
   }
