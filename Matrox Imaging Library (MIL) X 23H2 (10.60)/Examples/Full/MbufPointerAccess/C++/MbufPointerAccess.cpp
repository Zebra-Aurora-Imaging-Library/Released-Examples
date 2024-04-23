//*******************************************************************************/
/*
* File name: MbufPointerAccess.cpp
*
* Synopsis:  This program shows how to use the pointer of a
*            MIL buffer in order to directly access its data.
*
* *  Note: This program does not support Distributed MIL (DMIL).
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>  

/* Target image size. */
#define IMAGE_SIZE_X 512
#define IMAGE_SIZE_Y 512

/* Pointer access example for a monochrome buffer. */
void MonochromeBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* Pointer access example for a color packed buffer. */
void ColorPackedBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* Pointer access example for a color planar buffer. */
void ColorPlanarBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay);

/* Utility functions */
MIL_UINT Mandelbrot(MIL_INT PosX, MIL_INT PosY,
                    MIL_DOUBLE RefX, MIL_DOUBLE RefY, MIL_DOUBLE Dim);
MIL_UINT8 GetColorFromIndex(MIL_INT Band, MIL_INT Index, MIL_INT MaxIndex);

int MosMain(void)
   {
   MIL_ID  MilApplication, /* Application identifier.   */
           MilSystem,      /* System identifier.        */
           MilDisplay;     /* Display identifier.       */

   MosPrintf(MIL_TEXT("\nMIL buffer pointer access example.\n"));
   MosPrintf(MIL_TEXT("----------------------------------\n\n"));

   /* Allocate default objects. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   if (MsysInquire(MilSystem, M_LOCATION, M_NULL) == M_LOCAL)
      {

      /* Pointer access example for a monochrome buffer */
      MonochromeBufferPointerAccessExample(MilSystem, MilDisplay);

      /* Pointer access example for a color packed buffer. */
      ColorPackedBufferPointerAccessExample(MilSystem, MilDisplay);

      /* Pointer access example for a color planar buffer. */
      ColorPlanarBufferPointerAccessExample(MilSystem, MilDisplay);
      }
   else
      {
      /* Print that the example don't run remotely. */
      MosPrintf(MIL_TEXT("This example doesn't run with Distributed MIL.\n"));
      /* Wait for a key to terminate. */
      MosPrintf(MIL_TEXT("Press a key to terminate.\n\n"));
      MosGetch();
      }
   /* Free allocated objects. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }

/* Pointer access example for a monochrome buffer. */
/* ------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF1 -0.500
#define Y_REF1 +0.002
#define DIM1   +3.200

void MonochromeBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID     MilImage;             /* Image buffer identifier. */
   MIL_UINT8* MilImagePtr = M_NULL; /* Image pointer.           */
   MIL_INT    MilImagePitch = 0;    /* Image pitch.             */
   MIL_INT    x, y;                 /* Buffer access variables. */
   MIL_UINT   Value;                /* Value to write.          */

   MosPrintf(MIL_TEXT("- The data of a 8bits monochrome MIL buffer is modified\n"));
   MosPrintf(MIL_TEXT("  using its pointer to directly access the memory.\n\n"));

   /* Allocate a monochrome buffer. */
   MbufAlloc2d(MilSystem, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP, &MilImage);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* Retrieving buffer data pointer and pitch information. */
   MbufInquire(MilImage, M_HOST_ADDRESS, &MilImagePtr);
   MbufInquire(MilImage, M_PITCH, &MilImagePitch);

   /* Direct Access to the buffer's data. */
   if (MilImagePtr != M_NULL)
      {
      /* For each row. */
      for (y = 0; y < IMAGE_SIZE_Y; y++)
         {
         /* For each column. */
         for (x = 0; x < IMAGE_SIZE_X; x++)
            {
            /* Calculate the pixel value. */
            Value = Mandelbrot(x, y, X_REF1, Y_REF1, DIM1);

            /* Write the pixel using its pointer. */
            MilImagePtr[x] = MIL_UINT8(Value);
            }

         /* Move pointer to the next line taking into account the image's pitch. */
         MilImagePtr += MilImagePitch;
         }

      /* Signals MIL that the buffer data has been updated. */
      MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

      /* Unlock buffer. */
      MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

      /* Select to display. */
      MdispSelect(MilDisplay, MilImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("The source buffer has no accessible memory\n"));
      MosPrintf(MIL_TEXT("address on this specific system. Try changing\n")); 
      MosPrintf(MIL_TEXT("the system in the MIL Config utility.\n\n"));
      }

   /* Print a message and wait for a key. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
   };

/* Pointer access example for a color packed buffer. */
/* --------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF2 -1.1355
#define Y_REF2 -0.2510
#define DIM2   +0.1500

/* Utility to pack B,G,R values into 32 bits integer. */
MIL_UINT32 PackToBGR32(MIL_UINT8 b, MIL_UINT8 g, MIL_UINT8 r) 
   {
   return ((MIL_UINT32)b | (MIL_UINT32)(g << 8) | (MIL_UINT32)(r << 16));
   };

void ColorPackedBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage;            /* Image buffer identifier. */
   MIL_UINT32* MilImagePtr=M_NULL;  /* Image pointer.           */
   MIL_INT     MilImagePitch = 0;   /* Image Pitch.             */
   MIL_INT     x, y, NbBand = 3;    /* Buffer access variables. */
   MIL_UINT    Value;               /* Equation Value.          */
   MIL_UINT32  Value_BGR32;         /* Color Value to write.    */

   MosPrintf(MIL_TEXT("- The data of a 32bits color packed MIL buffer is modified\n"));
   MosPrintf(MIL_TEXT("  using its pointer to directly access the memory.\n\n"));

   /* Allocate a color buffer. */
   MbufAllocColor(MilSystem, NbBand, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP + M_BGR32 + M_PACKED, &MilImage);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* Retrieving buffer pointer and pitch information. */
   MbufInquire(MilImage, M_HOST_ADDRESS, &MilImagePtr);
   MbufInquire(MilImage, M_PITCH, &MilImagePitch);

   /* Custom modification of the buffer's data. */
   if (MilImagePtr != M_NULL)
      {
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
               GetColorFromIndex(M_RED, Value, 255)
               );

            /* Write the color pixel using its pointer. */
            MilImagePtr[x] = Value_BGR32;
            }

         /* Move pointer to the next line taking into account the image's pitch. */
         MilImagePtr += MilImagePitch;
         }

      /* Signals MIL the buffer data has been updated. */
      MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

      /* Unlock buffer. */
      MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

      /* Select to display. */
      MdispSelect(MilDisplay, MilImage);

      }
   else
      {
      MosPrintf(MIL_TEXT("The source buffer has no accessible memory\n"));
      MosPrintf(MIL_TEXT("address on this specific system. Try changing\n")); 
      MosPrintf(MIL_TEXT("the system in the MIL Config utility.\n\n"));
      }

   /* Print a message and wait for a key. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
   };

/* Pointer access example for a color planar buffer. */
/* ------------------------------------------------*/

/* Pixel value calculation parameters. */
#define X_REF3 -0.7453
#define Y_REF3 +0.1127
#define DIM3   +0.0060

void ColorPlanarBufferPointerAccessExample(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID      MilImage,             /* Image buffer identifier. */
               MilImageBand;         /* Image band identifier.   */
   MIL_UINT8*  MilImageBandPtr= M_NULL;      /* Image band pointer.      */
   MIL_INT     MilImagePitch = 0;    /* Image Pitch.             */
   MIL_INT     x, y, i, NbBand = 3;  /* Buffer access variables. */
   MIL_UINT    Value;                /* Value write.             */

   MIL_UINT    ColorBand[3] =    { M_RED, M_GREEN, M_BLUE    };

   MosPrintf(MIL_TEXT("- The data of a 24bits color planar MIL buffer is modified using\n"));
   MosPrintf(MIL_TEXT("  each color band pointer's to directly access the memory.\n\n"));

   /* Allocate a color buffer. */
   MbufAllocColor(MilSystem, NbBand, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + M_UNSIGNED,
      M_IMAGE + M_PROC + M_DISP + M_PLANAR, &MilImage);

   /* Retrieving buffer pitch information. */
   MbufInquire(MilImage, M_PITCH, &MilImagePitch);

   /* Lock buffer for direct access. */
   MbufControl(MilImage, M_LOCK, M_DEFAULT);

   /* Verifying the buffer has a host address. */
   MbufChildColor(MilImage, M_RED, &MilImageBand);
   MbufInquire(MilImageBand, M_HOST_ADDRESS, &MilImageBandPtr);
   MbufFree(MilImageBand);

   if (MilImageBandPtr != M_NULL)
      {
      /* For each color band. */
      for (i = 0; i < NbBand; i++)
         {
         /* Retrieving buffer color band pointer. */
         MbufChildColor(MilImage, ColorBand[i], &MilImageBand);
         MbufInquire(MilImageBand, M_HOST_ADDRESS, &MilImageBandPtr);

         /* For each row. */
         for (y = 0; y < IMAGE_SIZE_Y; y++)
            {
            /* For each column. */
            for (x = 0; x < IMAGE_SIZE_X; x++)
               {
               /* Calculate the pixel value. */
               Value = Mandelbrot(x, y, X_REF3, Y_REF3, DIM3);

               /* Write the color pixel using its pointer. */
               MilImageBandPtr[x] = GetColorFromIndex(ColorBand[i], Value, 255);
               }

            /* Move pointer to the next line taking into account the image's pitch. */
            MilImageBandPtr += MilImagePitch;
            }

         /* Release the child band identifier. */
         MbufFree(MilImageBand);
         }

      /* Signals MIL the buffer data has been updated. */
      MbufControl(MilImage, M_MODIFIED, M_DEFAULT);

      /* Unlock buffer. */
      MbufControl(MilImage, M_UNLOCK, M_DEFAULT);

      /* Select to display. */
      MdispSelect(MilDisplay, MilImage);
      }
   else
      {
      MosPrintf(MIL_TEXT("The source buffer has no accessible memory\n"));
      MosPrintf(MIL_TEXT("address on this specific system. Try changing\n")); 
      MosPrintf(MIL_TEXT("the system in the MIL Config utility.\n\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Free allocation. */
   MbufFree(MilImage);
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
