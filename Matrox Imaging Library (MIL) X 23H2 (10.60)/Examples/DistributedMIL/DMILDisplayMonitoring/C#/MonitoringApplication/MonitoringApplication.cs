//*****************************************************************************
//
// File name: MonitoringApplication.cs
//
// Synopsis:  This program monitors an application (see PublishingApplication.cpp example)
//            It shows how to copy the content of a published remote display image and its
//            overlay in a local display image and a local overlay, to be viewed locally.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using Matrox.MatroxImagingLibrary;

namespace MonitoringApplication
{
   class Program
   {
      // Title for the display window.
      private const string WINDOW_TITLE = "Monitoring Application";

      // Define the publishing system name.
#if (CONNECT_TO_RTOS)
// We want to connect to the RTOS version of PublishingApplication
private const string PUBLISH_SYSTEM_NAME = "dmilshmrt://localhost";
#else
// If we are connecting to the local host, use dmilshm.
// If the PublishingApplication is on another computer,
// use dmiltcp instead.
private const string PUBLISH_SYSTEM_NAME = "dmilshm://localhost";
#endif

      static void Main(string[] args)
      {
         // Local resources.
         MIL_ID MilApplication = MIL.M_NULL,
                MilSystem = MIL.M_NULL,
                LocalDisplay = MIL.M_NULL,
                LocalDisplayImage = MIL.M_NULL,
                LocalOverlayImage = MIL.M_NULL;

         // Remote Resources.
         MIL_ID RemoteDisplayImage = MIL.M_NULL,
                RemoteOverlayImage = MIL.M_NULL,
                MilRemoteApplication = MIL.M_NULL;

         // Allocate defaults.
         MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref LocalDisplay, MIL.M_NULL, MIL.M_NULL);

         // Initiate the monitoring.
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
         MIL.MappOpenConnection(PUBLISH_SYSTEM_NAME, MIL.M_DEFAULT, MIL.M_DEFAULT, ref MilRemoteApplication);
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

         if (MilRemoteApplication == 0)
         {
            Console.WriteLine("Cannot open a connection to the remote application.");
            Console.WriteLine("Please make sure the publishing example is running.");
            Console.WriteLine("Press <Enter> to end.");
            Console.ReadLine();
         }
         else
         {

            // Retrieve published objects.
            MIL.MappInquireConnection(MilRemoteApplication, MIL.M_DMIL_PUBLISHED_NAME,
               "DisplayImage", MIL.M_DEFAULT, ref RemoteDisplayImage);
            MIL.MappInquireConnection(MilRemoteApplication, MIL.M_DMIL_PUBLISHED_NAME,
               "OverlayImage", MIL.M_DEFAULT, ref RemoteOverlayImage);

            // If a remote image is published, we want to display it locally.
            if (MIL.M_NULL != RemoteDisplayImage)
            {
               // Allocate a local display image in which remote image will be copied.
               MIL.MbufAllocColor(MilSystem,
                              MIL.MbufInquire(RemoteDisplayImage, MIL.M_SIZE_BAND, MIL.M_NULL),
                              MIL.MbufInquire(RemoteDisplayImage, MIL.M_SIZE_X, MIL.M_NULL),
                              MIL.MbufInquire(RemoteDisplayImage, MIL.M_SIZE_Y, MIL.M_NULL),
                              MIL.MbufInquire(RemoteDisplayImage, MIL.M_TYPE, MIL.M_NULL),
                              MIL.M_IMAGE + MIL.M_DISP,
                              ref LocalDisplayImage);

               // Copy remote image content.
               MIL.MbufCopy(RemoteDisplayImage, LocalDisplayImage);

               // Select this local image on a local display.
               MIL.MdispSelect(LocalDisplay, LocalDisplayImage);
               MIL.MdispControl(LocalDisplay, MIL.M_TITLE, WINDOW_TITLE);

               // Hook to remote image modifications.
               MIL.MbufLink(RemoteDisplayImage, LocalDisplayImage, MIL.M_LINK, MIL.M_DEFAULT);

               // If an overlay has been published, show it.
               if (MIL.M_NULL != RemoteOverlayImage)
               {
                  // Enable the overlay on the local display and inquire its ID.
                  MIL.MdispControl(LocalDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
                  LocalOverlayImage = (MIL_ID)MIL.MdispInquire(LocalDisplay, MIL.M_OVERLAY_ID, MIL.M_NULL);

                  // Copy remote overlay contain in local overlay.
                  MIL.MbufCopy(RemoteOverlayImage, LocalOverlayImage);

                  // Hook to the modifications of the remote overlay (will be copied in local overlay).
                  MIL.MbufLink(RemoteOverlayImage, LocalOverlayImage, MIL.M_LINK, MIL.M_DEFAULT);
               }

               Console.WriteLine("DMIL MONITORING (Monitoring):");
               Console.WriteLine("-------------------------------------------");
               Console.WriteLine("The image displayed is the image published by another");
               Console.WriteLine("application for DMIL monitoring.\n");
               Console.WriteLine("Press <Enter> to end.");
               Console.ReadLine();

               // Make sure to unlink.
               MIL.MbufLink(RemoteDisplayImage, LocalDisplayImage, MIL.M_UNLINK, MIL.M_DEFAULT);

               if (MIL.M_NULL != RemoteOverlayImage)
                  MIL.MbufLink(RemoteOverlayImage, LocalOverlayImage, MIL.M_UNLINK, MIL.M_DEFAULT);
            }
            else
            {
               Console.WriteLine("No image to display.");
               Console.WriteLine("Press <Enter> to continue.\n");
               Console.ReadLine();
            }

            MIL.MappCloseConnection(MilRemoteApplication);
         }
         MIL.MappFreeDefault(MilApplication, MilSystem, LocalDisplay, MIL.M_NULL, LocalDisplayImage);
      }
   }
}
