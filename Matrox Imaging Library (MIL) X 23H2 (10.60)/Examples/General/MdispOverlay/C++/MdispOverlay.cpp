/*************************************************************************************/
/*
 * File name: MDispOverlay.cpp
 *
 * Synopsis:  This program shows how to display an image while creating
 *            text and graphics annotations on top of it using MIL
 *            graphic functions and windows GDI drawing under Windows
 *            or cairo drawing under Linux.
 *            If the target system supports grabbing, the annotations
 *            are done on top of a continuous grab.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#if M_MIL_USE_WINDOWS
#include <windows.h>
#else
#include <stdlib.h>
#include <cairo.h>
#endif

/* Target image. */
#define IMAGE_FILE     M_IMAGE_PATH MIL_TEXT("Board.mim")

/* Title for the display window. */
#define WINDOW_TITLE   MIL_TEXT("Custom Title")

/* Overlay drawing function prototype. */
void OverlayDraw(MIL_ID Display);

int MosMain(void)
   {
   MIL_ID MilApplication,            /* Application identifier. */
          MilSystem,                 /* System identifier.      */
          MilDisplay,                /* Display identifier.     */
          MilDigitizer=0,            /* Digitizer identifier.   */
          MilImage;                  /* Image identifier.       */

   /* Allocate defaults */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, M_NULL, M_NULL);

   /* If the system have a digitizer, use it. */
   if (MsysInquire(MilSystem, M_DIGITIZER_NUM, M_NULL))
      {
      MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);
      MbufAllocColor(MilSystem,
                     MdigInquire(MilDigitizer,M_SIZE_BAND,M_NULL),
                     MdigInquire(MilDigitizer,M_SIZE_X,M_NULL),
                     MdigInquire(MilDigitizer,M_SIZE_Y,M_NULL),
                     8+M_UNSIGNED,
                     M_IMAGE+M_DISP+M_PROC+M_GRAB,
                     &MilImage);
      MbufClear(MilImage, 0);
      }
   else
      {
      /* Restore a static image. */
      MbufRestore(IMAGE_FILE, MilSystem, &MilImage);
      }

   /* Change display window title. */
   MdispControl(MilDisplay, M_TITLE, WINDOW_TITLE);

   /* Display the image buffer. */
   MdispSelect(MilDisplay, MilImage);

   /* Draw text and graphics annotations in the display overlay. */
   OverlayDraw(MilDisplay);

   /* If the system supports it, grab continuously in the displayed image. */
   if (MilDigitizer)
      MdigGrabContinuous(MilDigitizer, MilImage);

   /* Pause to show the image. */
   MosPrintf(MIL_TEXT("\nOVERLAY ANNOTATIONS:\n"));
   MosPrintf(MIL_TEXT("--------------------\n\n"));
   MosPrintf(MIL_TEXT("Displaying an image with overlay annotations.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Stop the continuous grab and free digitizer if needed. */
   if (MilDigitizer)
      {
      MdigHalt(MilDigitizer);
      MdigFree(MilDigitizer);

      /* Pause to show the result. */
      MosPrintf(MIL_TEXT("Displaying the last grabbed image.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   /* Free image. */
   MbufFree(MilImage);

   /* Free default allocations. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
   }


/* --------------------------------------------------------------- */
/* Name:      OverlayDraw
 * Synopsis:  This function draws annotations in the display overlay.
 */
void OverlayDraw(MIL_ID MilDisplay)
   {
   MIL_ID         MilOverlayImage;
   MIL_INT        ImageWidth, ImageHeight;
   int            Count;
   MIL_TEXT_CHAR chText[80];

   /* Prepare overlay buffer. */
   /***************************/

   /* Enable the display of overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);

   /* Inquire the overlay buffer associated with the display. */
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);

   /* Clear the overlay to transparent. */
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

   /* Disable the overlay display update to accelerate annotations. */
   MdispControl(MilDisplay, M_OVERLAY_SHOW, M_DISABLE);

   /* Inquire overlay size. */
   ImageWidth  = MbufInquire(MilOverlayImage, M_SIZE_X, M_NULL);
   ImageHeight = MbufInquire(MilOverlayImage, M_SIZE_Y, M_NULL);

   /* Draw MIL overlay annotations. */
   /*********************************/

   /* Set the graphic text background to transparent. */
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   /* Print a white string in the overlay image buffer. */
   MgraColor(M_DEFAULT, M_COLOR_WHITE);
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5,    
                                             MIL_TEXT(" -------------------- "));
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5+25, 
                                             MIL_TEXT(" - MIL Overlay Text - "));
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth/9, ImageHeight/5+50, 
                                             MIL_TEXT(" -------------------- "));

   /* Print a green string in the overlay image buffer. */
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5,    
                                             MIL_TEXT(" ---------------------"));
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5+25, 
                                             MIL_TEXT(" - MIL Overlay Text - "));
   MgraText(M_DEFAULT, MilOverlayImage, ImageWidth*11/18, ImageHeight/5+50, 
                                             MIL_TEXT(" ---------------------"));

   /* Re-enable the overlay display after all annotations are done. */
   MdispControl(MilDisplay, M_OVERLAY_SHOW, M_ENABLE);


   /* Draw GDI/Cairo color overlay annotation */
   /*************************************/

   /* Disable error printing for the inquire might not be supported */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

#if M_MIL_USE_WINDOWS
   HDC            hCustomDC;
   HGDIOBJ        hpen, hpenOld;
   /* Create a device context to draw in the overlay buffer with GDI. */
   MbufControl(MilOverlayImage, M_DC_ALLOC, M_DEFAULT);

   /* Inquire the device context. */
   hCustomDC = ((HDC)MbufInquire(MilOverlayImage, M_DC_HANDLE, M_NULL));

   /* Re-enable error printing */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Perform operation if GDI drawing is supported. */
   if (hCustomDC)
      {
      POINT Hor[2];
      POINT Ver[2];
      SIZE  TxtSz;
      RECT  Txt;

      /* Draw a blue cross. */
      hpen=CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
      hpenOld = SelectObject(hCustomDC, hpen);

      Hor[0].x = (MIL_INT32)0;
      Hor[0].y = (MIL_INT32)(ImageHeight/2);
      Hor[1].x = (MIL_INT32)ImageWidth;
      Hor[1].y = (MIL_INT32)(ImageHeight/2);
      Polyline(hCustomDC, Hor,2);

      Ver[0].x = (MIL_INT32)(ImageWidth/2);
      Ver[0].y = (MIL_INT32)0;
      Ver[1].x = (MIL_INT32)(ImageWidth/2);
      Ver[1].y = (MIL_INT32)ImageHeight;
      Polyline(hCustomDC, Ver,2);

      SelectObject(hCustomDC, hpenOld);
      DeleteObject(hpen);

      /* Prepare transparent text annotations. */
      SetBkMode(hCustomDC, TRANSPARENT);
      MosStrcpy(chText, 80, MIL_TEXT("GDI Overlay Text"));
      Count = (int) MosStrlen(chText);
      GetTextExtentPoint(hCustomDC, chText, Count, &TxtSz);


      /* Write red text. */
      Txt.left = (MIL_INT32)(ImageWidth*3/18);
      Txt.top  = (MIL_INT32)(ImageHeight*17/24);
      Txt.right  = (MIL_INT32)(Txt.left + TxtSz.cx);
      Txt.bottom = (MIL_INT32)(Txt.top  + TxtSz.cy);
      SetTextColor(hCustomDC,RGB(255, 0, 0));
      DrawText(hCustomDC, chText, Count, &Txt, DT_RIGHT);

      /* Write yellow text. */
      Txt.left = (MIL_INT32)ImageWidth*12/18;
      Txt.top  = (MIL_INT32)ImageHeight*17/24;
      Txt.right  = (MIL_INT32)(Txt.left + TxtSz.cx);
      Txt.bottom = (MIL_INT32)(Txt.top  + TxtSz.cy);
      SetTextColor(hCustomDC, RGB(255, 255, 0));
      DrawText(hCustomDC, chText, Count, &Txt, DT_RIGHT);

      /* Delete device context. */
      MbufControl(MilOverlayImage, M_DC_FREE, M_DEFAULT);

      /* Signal MIL that the overlay buffer was modified. */
      MbufControl(MilOverlayImage, M_MODIFIED, M_DEFAULT);
      }
#else
   /* Create a cairo surface to draw in the overlay buffer. */
   MbufControl(MilOverlayImage, M_SURFACE_ALLOC, M_COMPENSATION_ENABLE);

   /* Inquire the cairo surface. */
   cairo_surface_t * surface = ((cairo_surface_t *)MbufInquire(MilOverlayImage, M_SURFACE_HANDLE, M_NULL));
   if(surface)
      {
      cairo_t *cr;
      cr = cairo_create (surface);

      cairo_set_source_rgb (cr, 0, 0, 1);
      // Draw a blue cross in the overlay buffer.
      cairo_move_to (cr, 0, ImageHeight/2);
      cairo_line_to (cr, ImageWidth, ImageHeight/2);
      cairo_stroke (cr);
      cairo_move_to (cr, ImageWidth/2, 0);
      cairo_line_to (cr, ImageWidth/2, ImageHeight);
      cairo_stroke (cr);

      // Write Red text in the overlay buffer.
      MosStrcpy(chText, 80, "Cairo Overlay Text ");
      cairo_set_source_rgb (cr, 1, 0, 0);
      cairo_set_font_size(cr, 13);
      cairo_move_to(cr, ImageWidth*3/18, ImageHeight*4/6);
      cairo_show_text(cr, chText);

      // Write Yellow text in the overlay buffer.
      cairo_set_source_rgb (cr, 1, 1, 0);
      cairo_set_font_size(cr, 13);
      cairo_move_to(cr,  ImageWidth*12/18,ImageHeight*4/6);
      cairo_show_text(cr, chText);

      cairo_surface_flush(surface);
      cairo_destroy(cr);

      /* Delete device context. */
      MbufControl(MilOverlayImage, M_SURFACE_FREE, M_DEFAULT);

      /* Signal MIL that the overlay buffer was modified. */
      MbufControl(MilOverlayImage, M_MODIFIED, M_DEFAULT);
      }
#endif
    }

