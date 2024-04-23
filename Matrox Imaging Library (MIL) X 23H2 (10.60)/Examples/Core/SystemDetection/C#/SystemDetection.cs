﻿//*****************************************************************************
//
// File name: SystemDetection.cs
//
// Synopsis:  This program shows how to use the MappInquire(M_INSTALLED_... inquires to detect
//            installed systems types and display their names using SystemDetection.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*****************************************************************************
using System;
using System.Runtime.InteropServices;
using System.Text;

using Matrox.MatroxImagingLibrary;

namespace SystemDetection
   {
   class Program
      {
      static readonly MIL_INT IMAGE_SIZEX = 640;
      static readonly MIL_INT IMAGE_SIZEY = 480;

      static void Main(string[] args)
         {
         MIL_ID MilApplication = MIL.M_NULL;
         MIL_INT NbAvailableSystems = MIL.M_NULL;

         MIL.MappAlloc(MIL.M_NULL, MIL.M_DEFAULT, ref MilApplication);
         MIL.MappInquire(MilApplication, MIL.M_INSTALLED_SYSTEM_COUNT, ref NbAvailableSystems);

         MIL_ID [] MilSystem  = new MIL_ID[NbAvailableSystems];
         MIL_ID [] MilDisplay = new MIL_ID[NbAvailableSystems];
         MIL_ID [] MilBuffer  = new MIL_ID[NbAvailableSystems];

         StringBuilder CurrentBoardName = new StringBuilder();
         StringBuilder CurrentDisplayableBoardName = new StringBuilder();
         MIL_INT CurrentSystemType = 0;
         MIL_INT DeviceCount = 0;
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);

         Console.Write(NbAvailableSystems + " system(s) found\n\n\n");

         for(MIL_INT i = 0; i < NbAvailableSystems; i++)
            {
            MIL.MappInquire(MilApplication, MIL.M_INSTALLED_SYSTEM_PRINT_NAME + i, CurrentDisplayableBoardName);
            MIL.MappInquire(MilApplication, MIL.M_INSTALLED_SYSTEM_DESCRIPTOR + i, CurrentBoardName);
            MIL.MappInquire(MilApplication, MIL.M_INSTALLED_SYSTEM_TYPE + i, ref CurrentSystemType);
            MIL.MappInquire(MilApplication, MIL.M_INSTALLED_SYSTEM_DEVICE_COUNT + i, ref DeviceCount);

            Console.Write("\nSystem Print Name: " + CurrentDisplayableBoardName);
            Console.Write("\nSystem Descriptor: " + CurrentBoardName);
            Console.Write("\nSystem Type      : " + CurrentSystemType);
            Console.Write("\nSystem Count     : ");

            if(DeviceCount == MIL.M_UNKNOWN)
               {
               Console.Write("unknown\n");
               }
            else
               {
               Console.Write(DeviceCount + "\n");
               }

            if(DeviceCount == 0)
               {
               continue;
               }

            MIL.MsysAlloc(MilApplication, CurrentBoardName.ToString(), MIL.M_DEFAULT, (long)MIL.M_DEFAULT, ref MilSystem[i]);
            if(MilSystem[i] != MIL.M_NULL)
               {
               MIL.MbufAlloc2d(MilSystem[i], IMAGE_SIZEX, IMAGE_SIZEY, 8 + MIL.M_UNSIGNED, MIL.M_IMAGE + MIL.M_DISP, ref MilBuffer[i]);
               MIL.MbufClear(MilBuffer[i], MIL.M_COLOR_BLACK);
               MIL.MdispAlloc(MilSystem[i], MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, ref MilDisplay[i]);
               MIL.MdispControl(MilDisplay[i], MIL.M_TITLE, CurrentDisplayableBoardName.ToString());
               MIL.MdispSelect(MilDisplay[i], MilBuffer[i]);

               String WriteMessage = "Allocated " + CurrentDisplayableBoardName;
               if(DeviceCount == MIL.M_UNKNOWN)
                  {
                  Console.WriteLine(WriteMessage + ".");
                  }

               MIL.MgraText(MIL.M_DEFAULT, MilBuffer[i], IMAGE_SIZEX / 5, IMAGE_SIZEY / 3, WriteMessage);
               }
            else
               {
               if(DeviceCount == MIL.M_UNKNOWN)
                  {
                  Console.WriteLine("No " + CurrentBoardName + " are present in the system.");
                  }
               else
                  {
                  Console.WriteLine(CurrentBoardName + " failed to initialize.");
                  }
               }
            }
         Console.WriteLine("\nPress <Enter> to quit the application");
         Console.ReadKey();

         for(MIL_INT SysCount = 0; SysCount < NbAvailableSystems; SysCount++)
            {
            if(MilBuffer[SysCount] != MIL.M_NULL)
               {
               MIL.MbufFree(MilBuffer[SysCount]);
               }
            if(MilDisplay[SysCount] != MIL.M_NULL)
               {
               MIL.MdispFree(MilDisplay[SysCount]);
               }
            if (MilSystem[SysCount] != MIL.M_NULL)
               {
               MIL.MsysFree(MilSystem[SysCount]);
               }
            }
         MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);
         MIL.MappFree(MilApplication);
         }
      }
   }
