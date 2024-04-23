//*****************************************************************************
//
// File name: PublishingApplication.cs
//
// Synopsis:  This program shows how to allow the monitoring of a MIL application
//            and how to publish a display image with its overlay. 
//            The published MIL objects can then be accessed by an external MIL application.
//            (see MonitoringApplication.cpp example).
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using Matrox.MatroxImagingLibrary;

namespace PublishingApplication
{
   class Program
   {
      // Target image specifications.
      private const string IMAGE_FILE = MIL.M_IMAGE_PATH + "BaboonMono.mim";

      // Title for the display window.
      private const string WINDOW_TITLE = "Publishing Application";

      static void Main(string[] args)
      {
         MIL_ID MilApplication = MIL.M_NULL,
                  MilSystem = MIL.M_NULL,
                  MilDisplay = MIL.M_NULL,
                  MilSrcImage = MIL.M_NULL,
                  MilDisplayImage = MIL.M_NULL;
         MIL_INT LicenseModules = 0;
         double TempColor = 0;

         // Allocate defaults.
         // We do not rely on the DMIL Cluster Manager being setup and force a non-default 
         // Cluster Node number using M_CLUSTER_NODE() to be sure the example works even without
         // the Cluster Manager server.
         MIL.MappAllocDefault(MIL.M_DEFAULT+MIL.M_CLUSTER_NODE(15), ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

         // Check if the system is local.
         if (MIL.MsysInquire(MilSystem, MIL.M_LOCATION, MIL.M_NULL) != MIL.M_LOCAL)
         {
            Console.WriteLine("This example requires the default system to be a local system.");
            Console.WriteLine("Please select a local system as the default.");
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
            Console.ReadKey();
            Environment.Exit(-1);
         }

         /* Inquire MIL licenses. */
         MIL.MappInquire(MIL.M_DEFAULT, MIL.M_LICENSE_MODULES, ref LicenseModules);

         // Restore the source image.
         MIL.MbufRestore(IMAGE_FILE, MilSystem, ref MilSrcImage);

         // Retrieve the image sizes of the restored image
         MIL_INT ImageSizeX = MIL.MbufInquire(MilSrcImage, MIL.M_SIZE_X, MIL.M_NULL);
         MIL_INT ImageSizeY = MIL.MbufInquire(MilSrcImage, MIL.M_SIZE_Y, MIL.M_NULL);

         // Allocate a display image and show it.
         MIL.MbufAllocColor(MilSystem, 3, ImageSizeX, ImageSizeY, 8, MIL.M_IMAGE + MIL.M_DISP + MIL.M_PROC, ref MilDisplayImage);
         MIL.MdispSelect(MilDisplay, MilDisplayImage);
         MIL.MdispControl(MilDisplay, MIL.M_TITLE, WINDOW_TITLE);

         //************* Prepare the overlay **********************
         MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
         MIL_ID MilOverlayImage = 0;
         MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlayImage);

         // Inquire overlay size.
         MIL_INT ImageWidth = MIL.MbufInquire(MilOverlayImage, MIL.M_SIZE_X, MIL.M_NULL);
         MIL_INT ImageHeight = MIL.MbufInquire(MilOverlayImage, MIL.M_SIZE_Y, MIL.M_NULL);

         // Setup MIL overlay annotations. 
         MIL.MgraControl(MIL.M_DEFAULT, MIL.M_BACKGROUND_MODE, MIL.M_TRANSPARENT);
         MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);

         //************** Allow Monitoring ***************************
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_DMIL_CONNECTION, MIL.M_DMIL_CONTROL);

         MIL.MobjControl(MilDisplayImage, MIL.M_OBJECT_NAME, "DisplayImage");
         MIL.MobjControl(MilDisplayImage, MIL.M_DMIL_PUBLISH, MIL.M_READ_ONLY);

         MIL.MobjControl(MilOverlayImage, MIL.M_OBJECT_NAME, "OverlayImage");
         MIL.MobjControl(MilOverlayImage, MIL.M_DMIL_PUBLISH, MIL.M_READ_ONLY);
   
         //************** Processsing *********************************
         MIL_INT Angle = 0;
         double CenterX = (double)(ImageSizeX)/2;
         double CenterY = (double)(ImageSizeX)/2;
         string Text = string.Empty;

         Console.WriteLine("DMIL MONITORING (Publishing):");
         Console.WriteLine("-------------------------------------------");
         Console.WriteLine("The image displayed is published using DMIL in order to");
         Console.WriteLine("make it available for external monitoring.\n");
         Console.WriteLine("You can now run the monitoring example.\n");
         Console.WriteLine("Press <Enter> to end.");

         while (!Console.KeyAvailable)
         {
            Angle = (Angle + 5) % 360;

            // In order to optimize display updates and network transfers,
            // modifications hooks and display updates are disabled.
            MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_DISABLE);
            MIL.MbufControl(MilDisplayImage, MIL.M_MODIFICATION_HOOK, MIL.M_DISABLE);
            MIL.MbufControl(MilOverlayImage, MIL.M_MODIFICATION_HOOK, MIL.M_DISABLE);

            // Rotate the image.
#if (!M_MIL_LITE)
            if ((LicenseModules & MIL.M_LICENSE_IM) != 0)
            {
               MIL.MimRotate(MilSrcImage, MilDisplayImage, (double)Angle, CenterX, CenterX,
                         CenterX, CenterY, MIL.M_NEAREST_NEIGHBOR);
            }
            else
#endif
            {
               MIL.MbufCopy(MilSrcImage, MilDisplayImage);
               MIL.MgraInquire(MIL.M_DEFAULT, MIL.M_COLOR, ref TempColor);
               MIL.MgraColor(MIL.M_DEFAULT, 0x80);
               MIL.MgraArcFill(MIL.M_DEFAULT, MilDisplayImage, CenterX, CenterY, (double)Angle,
                           (double)Angle, 0, 360);
               MIL.MgraColor(MIL.M_DEFAULT, TempColor);
            }

            // Modify the overlay.
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY_CLEAR, MIL.M_DEFAULT);
            Text = string.Format(" - MIL Overlay Text {0}- ", (int)Angle);
            MIL.MgraText(MIL.M_DEFAULT, MilOverlayImage, CenterX, CenterX, Text);
            MIL.MdispControl(MilDisplay, MIL.M_UPDATE, MIL.M_ENABLE);
            MIL.MbufControl(MilDisplayImage, MIL.M_MODIFICATION_HOOK, MIL.M_ENABLE);
            MIL.MbufControl(MilOverlayImage, MIL.M_MODIFICATION_HOOK, MIL.M_ENABLE);
            MIL.MbufControl(MilDisplayImage, MIL.M_MODIFIED, MIL.M_DEFAULT);
            MIL.MbufControl(MilOverlayImage, MIL.M_MODIFIED, MIL.M_DEFAULT);
         }

         MIL.MbufFree(MilSrcImage);
         MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MilDisplayImage);
      }
   }
}
