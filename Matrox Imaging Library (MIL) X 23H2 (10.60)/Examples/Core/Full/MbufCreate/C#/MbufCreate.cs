﻿/*******************************************************************************/
/*
* File name: MIL.MbufCreate.cs
*
* Synopsis:  This program shows how to use MbufCreate function to
*            create a MIL buffer on user data or on the data of another 
*            MIL buffer.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*/

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Matrox.MatroxImagingLibrary;

namespace MBufCreate
   {
   class Program
      {
      /* Target image size. */
      private const int IMAGE_SIZE_X = 512;
      private const int IMAGE_SIZE_Y = 512;

      static void Main(string[] args)
         {
         MIL_ID MilApplication = MIL.M_NULL;  /*  Application identifier. */
         MIL_ID MilSystem = MIL.M_NULL;       /*  System identifier.      */
         MIL_ID MilDisplay = MIL.M_NULL;      /*  Display identifier.     */
         
         Console.Write("\nMIL BUFFER CREATION:\n");
         Console.Write("--------------------\n\n");
         Console.Write("This example shows how to use the MbufCreate functions\n");
         Console.Write("to create a MIL buffer from the memory at a specified location\n");
         Console.Write("by pointing to the address of user data or the identifier of an\n");
         Console.Write("already existing MIL buffer.\n\n");

         /* Allocate default objects. */
         MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
         MilSystem = MIL.M_DEFAULT_HOST;
         MIL.MdispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_WINDOWED, ref MilDisplay);

         /* MIL buffer creation example for a monochrome buffer */
         MonochromeBufCreateExample(MilSystem, MilDisplay);

         /* MIL buffer creation example for a color packed buffer. */
         ColorPackedBufCreateExample(MilSystem, MilDisplay);

         /* MIL buffer creation example for a color planar buffer. */
         ColorPlanarBufCreateExample(MilSystem, MilDisplay);

         /* MIL buffer creation examples for a monochrome buffer on packed buffer. */
         MonochromeOnColorPackedBufCreateExample(MilSystem, MilDisplay);

         /* Free allocated objects. */
         MIL.MdispFree(MilDisplay);
         MIL.MappFree(MilApplication);
         }

      /* MIL buffer creation example for a monochrome buffer. */
      /* -----------------------------------------------------*/

      /* Pixel value calculation parameters. */
      private const double X_REF1 = -0.500;
      private const double Y_REF1 = +0.002;
      private const double DIM1   = +3.200;

      static void MonochromeBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
         {
         MIL_ID   MilImage = MIL.M_NULL;      /* Image buffer identifier. */
         byte[]   UserImageData = null;       /* User image data.         */
         GCHandle UserDataHandle;             /* Handle to the user data. */
         MIL_INT  i, x, y;                    /* Buffer access variables. */
         MIL_UINT Value;                      /* Value to write.          */

         Console.Write("- A monochrome MIL buffer was created by pointing to \n");
         Console.Write("  the address of user data. The buffer was modified directly\n");
         Console.Write("  using the user data pointer.\n\n");
         
         /* Allocate a monochrome user array. */
         UserImageData = new byte[IMAGE_SIZE_X * IMAGE_SIZE_Y];
         UserDataHandle = GCHandle.Alloc(UserImageData, GCHandleType.Pinned);

         /* Create a MIL monochrome image buffer on the user array */
         MIL.MbufCreate2d(MilSystem, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
            MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP, MIL.M_HOST_ADDRESS + MIL.M_PITCH,
            MIL.M_DEFAULT, (ulong)UserDataHandle.AddrOfPinnedObject(), ref MilImage);

         /* Lock buffer for direct access. */
         MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

         /* Initialize the user array's value index. */
         i = 0;

         /* For each row. */
         for (y = 0; y < IMAGE_SIZE_Y; y++)
            {
            /* For each column. */
            for (x = 0; x < IMAGE_SIZE_X; x++)
               {
               /* Calculate the pixel value. */
               Value = Mandelbrot(x, y, X_REF1, Y_REF1, DIM1);

               /* Write the pixel using its pointer. */
               UserImageData[i] = (byte)(Value);

               /* Increment the user array's value index. */
               i++;
               }
            }

         /* Signal MIL that the buffer data has been updated. */
         MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);

         /* Unlock buffer. */
         MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);

         /* Select to display. */
         MIL.MdispSelect(MilDisplay, MilImage);

         /* Print a message and wait for a key. */
         Console.Write("Press <Enter> to continue.\n\n");
         Console.ReadKey();

         /* Free allocations. */
         MIL.MbufFree(MilImage);
         UserDataHandle.Free();
         }

      /* MIL buffer creation example for a color packed buffer. */
      /* -------------------------------------------------------*/

      /* Pixel value calculation parameters. */
      private const double  X_REF2 = -1.1355;
      private const double  Y_REF2 = -0.2510;
      private const double  DIM2   = +0.1500;

      /* Utility to pack B,G,R values into 32 bits integer. */
      static uint PackToBGR32(Byte b, Byte g, Byte r)
         {
         return ((uint)b | (uint)(g << 8) | (uint)(r << 16));
         }

      static void ColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
         {
         MIL_ID   MilImage = MIL.M_NULL;      /* Image buffer identifier.  */
         uint[]   UserImageData = null;       /* User image data.          */
         GCHandle UserDataHandle;             /* Handle to the user data.  */
         IntPtr   MilImageAddr = IntPtr.Zero; /* Pointer to the user data. */
         MIL_INT  x, y, i;                    /* Buffer access variables.  */
         uint     Value;                      /* Value to write.           */
         uint     Value_BGR32;                /* Value to write.           */

         Console.Write("- A 32-bit color packed MIL buffer was created by pointing to\n");
         Console.Write("  the address of user data. The buffer was modified directly.\n");
         Console.Write("  using the user data pointer.\n\n");

         /* Allocate a packed color user array. */
         UserImageData = new uint[IMAGE_SIZE_X * IMAGE_SIZE_Y];
         UserDataHandle = GCHandle.Alloc(UserImageData, GCHandleType.Pinned);
         MilImageAddr = UserDataHandle.AddrOfPinnedObject();

         /* Create a MIL packed color image buffer on the user array. */
         MIL.MbufCreateColor(MilSystem, 3, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
            MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP + MIL.M_BGR32 + MIL.M_PACKED,
            MIL.M_HOST_ADDRESS + MIL.M_PITCH, MIL.M_DEFAULT,
            ref MilImageAddr, ref MilImage);

         /* Lock buffer for direct access. */
         MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

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
                  (byte)GetColorFromIndex(MIL.M_BLUE, (MIL_INT)Value, 255),
                  (byte)GetColorFromIndex(MIL.M_GREEN, (MIL_INT)Value, 255),
                  (byte)GetColorFromIndex(MIL.M_RED, (MIL_INT)Value, 255));

               /* Write the pixel using its pointer. */
               UserImageData[i] = Value_BGR32;
                  
               /* Increment the user array's value index. */
               i++;
               }
            }

         /* Signal MIL that the buffer data has been updated. */
         MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);

         /* Unlock buffer. */
         MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);

         /* Select to display. */
         MIL.MdispSelect(MilDisplay, MilImage);

         /* Print a message and wait for a key. */
         Console.Write("Press <Enter> to continue.\n\n");
         Console.ReadKey();

         /* Free allocations. */
         MIL.MbufFree(MilImage);
         UserDataHandle.Free();
         }

      /* MIL buffer creation example for a color planar buffer. */
      /* -------------------------------------------------------*/

      /* Pixel value calculation parameters. */
      private const double  X_REF3 = -0.7453;
      private const double  Y_REF3 = +0.1127;
      private const double  DIM3 = +0.0060;

      static void ColorPlanarBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
         {
         MIL_ID     MilImage = MIL.M_NULL;            /* Image buffer identifier.  */
         byte[][]   UserImageData = new byte[3][];    /* User image data.          */
         GCHandle[] UserDataHandle = new GCHandle[3]; /* Handle to the user data.  */
         IntPtr[]   UserBandPtr = new IntPtr[3];      /* User image band pointers. */
         MIL_INT    x, y, i, b;                       /* Buffer access variables.  */
         MIL_UINT   Value;                            /* Value to write.           */
         MIL_INT[]  ColorBand = new MIL_INT[] { MIL.M_RED, MIL.M_GREEN, MIL.M_BLUE };

         Console.Write("- A 24-bit color planar MIL buffer was created by pointing to\n");
         Console.Write("  the addresses of 3 user data arrays. The buffers were modified\n");
         Console.Write("  directly using the user data pointers.\n\n");

         /* Allocate three user arrays representing the 3 bands of a color image. */
         for (b = 0; b < 3; b++)
            {
            UserImageData[b]  = new byte[IMAGE_SIZE_X * IMAGE_SIZE_Y];
            UserDataHandle[b] = GCHandle.Alloc(UserImageData[b], GCHandleType.Pinned);
            UserBandPtr[b]    = UserDataHandle[b].AddrOfPinnedObject();
            }

         /* Create a MIL planar color image buffer on the user arrays. */
         MIL.MbufCreateColor(MilSystem, 3, IMAGE_SIZE_X, IMAGE_SIZE_Y, 8 + MIL.M_UNSIGNED,
            MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP + MIL.M_PLANAR + MIL.M_RGB24,
            MIL.M_HOST_ADDRESS + MIL.M_PITCH, MIL.M_DEFAULT, UserBandPtr, ref MilImage);

         /* Lock buffer for direct access. */
         MIL.MbufControl(MilImage, MIL.M_LOCK, MIL.M_DEFAULT);

         /* For each color band. */
         for (b = 0; b < 3; b++)
            {
            /* Initialize the user array's value index. */
            i = 0;

            /* For each row. */
            for (y = 0; y < IMAGE_SIZE_Y; y++)
               {
               /* For each column. */
               for (x = 0; x < IMAGE_SIZE_X; x++)
                  {
                  /* Calculate the pixel value. */
                  Value = Mandelbrot(x, y, X_REF3, Y_REF3, DIM3);

                  /* Write the pixel using its pointer. */
                  UserImageData[b][i] = (byte)GetColorFromIndex(ColorBand[b], (MIL_INT)Value, 255);

                  /* Increment the user array's value index. */
                  i++;
                  }
               }
            }

         /* Signal MIL that the buffer data has been updated. */
         MIL.MbufControl(MilImage, MIL.M_MODIFIED, MIL.M_DEFAULT);
    
         /* Unlock buffer. */
         MIL.MbufControl(MilImage, MIL.M_UNLOCK, MIL.M_DEFAULT);
    
         /* Select to display. */
         MIL.MdispSelect(MilDisplay, MilImage);

         /* Print a message and wait for a key. */
         Console.Write("Press <Enter> to continue.\n\n");
         Console.ReadKey();

         /* Free allocations. */
         MIL.MbufFree(MilImage);
         for (b = 0; b < 3; b++)
            UserDataHandle[b].Free();
         }

      /* MIL buffer creation example for monochrome buffer on packed buffer. */
      /* --------------------------------------------------------------------*/

      /* Target MIL image file specifications. */
      private static readonly string COLOR_IMAGE_FILE          = 
         MIL.M_IMAGE_PATH + "BaboonRGB.mim";

      /* Source image information. */
      private const int SOURCE_SIZE_X = 256;
      private const int SOURCE_SIZE_Y = 256;
      private const byte KEPT_BITS = 0x80;

      /* Maximum number of events. */
      private const int MAX_NB_EVENTS = SOURCE_SIZE_X * SOURCE_SIZE_Y;

      /* Display zoom. */
      private const double DISPLAY_ZOOM = 3.0;

      /* Color value to replace. */
      private const byte SOURCE_RED   = 128;
      private const byte SOURCE_GREEN = 0;
      private const byte SOURCE_BLUE  = 0;
      private const byte DEST_RED     = 0;
      private const byte DEST_GREEN   = 255;
      private const byte DEST_BLUE    = 0;

      /* BGR32 mask. */
      private const int  BGR2_MASK    = 0X00FFFFFF;


        static void MonochromeOnColorPackedBufCreateExample(MIL_ID MilSystem, MIL_ID MilDisplay)
         {
         MIL_ID    MilGraList = MIL.M_NULL;     /* Graphic list identifier.              */
         MIL_ID    MilImage = MIL.M_NULL;       /* Image buffer identifier.              */
         MIL_ID    MilDestImage = MIL.M_NULL;   /* Destination image buffer identifier.  */
         MIL_ID    MilDispImage = MIL.M_NULL;   /* Display image buffer identifier.      */
         MIL_ID    MilMonoImage = MIL.M_NULL;   /* Created image buffer identifier.      */
         MIL_UINT  MonoColorValue;              /* Monochrome color value to replace.    */
         MIL_ID    MilEventResult = MIL.M_NULL; /* Locate event result buffer.           */
         MIL_INT   NbEvents = 0;                /* Number of event located in the image. */
         MIL_INT[] EventX = null;               /* Array of x coordinates.               */
         MIL_INT[] EventY = null;               /* Array of y coordinates.               */

         Console.Write("- A 32-bit monochrome MIL buffer was created by pointing to\n");
         Console.Write("  the identifier of a MIL packed color buffer. This was done to\n");
         Console.Write("  use the newly created buffer with a function that requires \n");
         Console.Write("  monochrome image buffers. In this example, the positions of\n");
         Console.Write("  dark red pixels found using MimLocateEvent() are displayed in\n");
         Console.Write("  green.\n\n");

         /* Allocate a graphic list and associate it to the display. */
         MIL.MgraAllocList(MilSystem, MIL.M_DEFAULT, ref MilGraList);
         MIL.MdispControl(MilDisplay, MIL.M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);
         
         /* Allocate the display image and the child buffers of the source and destination. */
         MIL.MbufAllocColor(MilSystem, 3, SOURCE_SIZE_X * 2, SOURCE_SIZE_Y, 8 + MIL.M_UNSIGNED,
            MIL.M_BGR32 + MIL.M_PACKED + MIL.M_IMAGE + MIL.M_PROC + MIL.M_DISP,
            ref MilDispImage);
         MIL.MbufChild2d(MilDispImage, 0, 0, SOURCE_SIZE_X, SOURCE_SIZE_Y, ref MilImage);
         MIL.MbufChild2d(MilDispImage, SOURCE_SIZE_X, 0, SOURCE_SIZE_X, SOURCE_SIZE_Y,
            ref MilDestImage);

         /* Allocate the event result buffer. */ 
         MIL.MimAllocResult(MilSystem, MAX_NB_EVENTS, MIL.M_EVENT_LIST,  ref MilEventResult);
         EventX = new MIL_INT[MAX_NB_EVENTS];
         EventY = new MIL_INT[MAX_NB_EVENTS];

         /* Restore the color image preprocess it to reduce the number of colors. */
         MIL.MbufLoad(COLOR_IMAGE_FILE, MilImage);
         MIL.MimArith(MilImage, KEPT_BITS, MilImage, MIL.M_AND_CONST);
         MIL.MbufCopy(MilImage, MilDestImage);

         /* Create a monochrome buffer on the memory of the color image. */
         MIL.MbufCreateColor(MilSystem, 1, MIL.M_DEFAULT, MIL.M_DEFAULT, 32 + MIL.M_UNSIGNED,
            MIL.M_IMAGE + MIL.M_PROC, MIL.M_MIL_ID + MIL.M_PITCH, MIL.M_DEFAULT,
            ref MilImage, ref MilMonoImage);

         /* Locate the coordinates of pixels of a certain BGR packed color value with */
         /* MimLocateEvent. */
         MonoColorValue = PackToBGR32(SOURCE_BLUE, SOURCE_GREEN, SOURCE_RED);

         /* The last "don't care" byte in the BGR32 buffer must be masked so the event is
            detected correctly. */
         MIL.MimArith(MilMonoImage, BGR2_MASK, MilMonoImage, MIL.M_AND_CONST);

         MIL.MimLocateEvent(MilMonoImage,MilEventResult, MIL.M_EQUAL, MonoColorValue, MIL.M_NULL);
         MIL.MimGetResult(MilEventResult, MIL.M_NB_EVENT, ref NbEvents);
   
         MIL.MimGetResult(MilEventResult, MIL.M_POSITION_X + MIL.M_TYPE_MIL_INT, EventX);
         MIL.MimGetResult(MilEventResult, MIL.M_POSITION_Y + MIL.M_TYPE_MIL_INT, EventY);

         /* Mark the located pixels in the destination image. */
         MIL.MgraColor(MIL.M_DEFAULT, MIL.M_RGB888(DEST_RED, DEST_GREEN, DEST_BLUE));
         MIL.MgraControl(MIL.M_DEFAULT, MIL.M_DRAW_OFFSET_X, -SOURCE_SIZE_X);
         MIL.MgraDots(MIL.M_DEFAULT, MilGraList, NbEvents, EventX, EventY, MIL.M_DEFAULT);

         /* Select to display. */
         MIL.MdispZoom(MilDisplay, DISPLAY_ZOOM, DISPLAY_ZOOM);
         MIL.MdispSelect(MilDisplay, MilDispImage);

         Console.Write("Press <Enter> to end.\n\n");
         Console.ReadKey();

         /* Remove the zoom. */
         MIL.MdispZoom(MilDisplay, 1.0, 1.0);

         int Val = 0;
         MIL.MgraInquireList(MilGraList, MIL.M_GRAPHIC_INDEX(0), MIL.M_DEFAULT, MIL.M_POSITION_X + MIL.M_TYPE_MIL_INT32, ref Val);

         /* Free allocation. */
         MIL.MbufFree(MilMonoImage);
         MIL.MimFree(MilEventResult);
         MIL.MbufFree(MilDestImage);
         MIL.MbufFree(MilImage);
         MIL.MbufFree(MilDispImage);
         MIL.MgraFree(MilGraList);
         }

      /* Mandelbrot fractal utility functions. */
      static double Remap(double pos, double size, double min, double max)
         {
         return ((((max) - (min)) / (size)) * (pos) + (min));
         }

      static uint Mandelbrot(MIL_INT PosX, MIL_INT PosY, double RefX,
         double RefY, double Dim)
         {
         const int maxIter = 256;
         double xMin = RefX - 0.5 * Dim;
         double xMax = RefX + 0.5 * Dim;
         double yMin = RefY - 0.5 * Dim;
         double yMax = RefY + 0.5 * Dim;
         double x0 = Remap((double)PosX, (double)IMAGE_SIZE_X, xMin, xMax);
         double y0 = Remap((double)PosY, (double)IMAGE_SIZE_Y, yMin, yMax);
         double x = 0.0;
         double y = 0.0;
         uint   Iter = 0;

         while ((x * x + y * y < 4) && (Iter < maxIter))
            {
            double Temp = x * x - y * y + x0;
            y = 2 * x * y + y0;
            x = Temp;
            Iter++;
            }

         if (Iter > 255)
            return 255;
         else
            return Iter;
         }

      /* Calculate color from index. */
      static MIL_INT GetColorFromIndex(MIL_INT Band, MIL_INT Index, MIL_INT MaxIndex)
         {
         MIL_INT[] Segments  = { };
         MIL_INT[] SegmentsR = { 0, 0, 0, 255, 255, 128 };
         MIL_INT[] SegmentsG = { 0, 0, 255, 255, 0, 0 };
         MIL_INT[] SegmentsB = { 128, 255, 255, 0, 0, 0 };

         switch ((int)Band)
            {
            case MIL.M_RED:
               Segments = SegmentsR;
               break;
            case MIL.M_GREEN:
               Segments = SegmentsG;
               break;
            case MIL.M_BLUE:
               Segments = SegmentsB;
               break;
            }

         double  RemapedIndex = Index * MaxIndex / 256.0;
         MIL_INT SegmentIndex = (MIL_INT)(RemapedIndex * 5.0 / 256.0);
         double  Slope = (Segments[SegmentIndex + 1] - Segments[SegmentIndex]) / (256.0 / 5.0);
         double  Offset = (Segments[SegmentIndex] - Slope * SegmentIndex * 256.0 / 5.0);
         MIL_INT Value = (MIL_INT)(Slope * RemapedIndex + Offset + 0.5);

         return Value;
         }
      }
   }
