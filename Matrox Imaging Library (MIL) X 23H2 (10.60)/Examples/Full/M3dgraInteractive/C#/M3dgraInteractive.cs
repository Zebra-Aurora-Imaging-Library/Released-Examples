//----------------------------------------------------------------------------
// 
// File name: M3dgraInteractive.cs
//
// Synopsis: This program contains an example on how to interactively edit a
//           3D box geometry.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//----------------------------------------------------------------------------

using System;
using System.Threading;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using Matrox.MatroxImagingLibrary;

namespace M3dgraInteractive
    {
    class Program
        {
        // Constants.
        private static readonly string PT_CLD_FILE = MIL.M_IMAGE_PATH + "M3dgra/MaskOrganized.mbufc";

        //----------------------------------------------------------------------------
        public class SPickStruct
            {
            public MIL_INT BoxLabel;
            public MIL_ID  Box;
            public MIL_ID  Gralist;
            public MIL_ID  OriginalContainer;
            public MIL_ID  CroppedContainer;
            };

        //----------------------------------------------------------------------------*
        // Main.
        //----------------------------------------------------------------------------*
        static void Main(string[] args)
            {
            Console.Write("[EXAMPLE NAME]\n");
            Console.Write("M3dgraInteractive\n\n");

            Console.Write("[SYNOPSIS]\n");
            Console.Write("This example demonstrates how to interactively edit a 3D box geometry.\n\n");

            Console.Write("[MODULES USED]\n");
            Console.Write("Modules used: application, system, buffer, 3D display, 3D graphics.\n");

            Console.Write("\n");

            MIL_ID MilApplication = MIL.M_NULL;     // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;          // System identifier.

            // Allocate defaults.
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL);

            // Check for required example files.
            if (!CheckForRequiredMILFile(PT_CLD_FILE))
                {
                return;
                }

            // Allocate the display.
            MIL_ID Mil3dDisplay = Alloc3dDisplayId(MilApplication, MilSystem);
            MIL_ID Mil3dGraList = (MIL_ID) MIL.M3ddispInquire(Mil3dDisplay, MIL.M_3D_GRAPHIC_LIST_ID, MIL.M_NULL);

            // Restore the point cloud from a file.
            MIL_ID OriginalContainer = MIL.MbufImport(PT_CLD_FILE, MIL.M_DEFAULT, MIL.M_RESTORE, MilSystem, MIL.M_NULL);

            // Create a cropped copy of the point cloud and add it to the graphic list.
            MIL_ID CroppedContainer = MIL.MbufAllocContainer(MilSystem, MIL.M_PROC + MIL.M_DISP, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dgraAdd(Mil3dGraList, MIL.M_ROOT_NODE, CroppedContainer, MIL.M_DEFAULT);

            // Create an editable box in the graphic list.
            // Initialize the size of the box to a fraction of the original point cloud's size.
            MIL_ID BoundingBox = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.M3dimStat(MIL.M_STAT_CONTEXT_BOUNDING_BOX, OriginalContainer, BoundingBox, MIL.M_DEFAULT);

            MIL.M3dgeoBox(BoundingBox, MIL.M_CENTER_AND_DIMENSION, MIL.M_UNCHANGED, MIL.M_UNCHANGED, MIL.M_UNCHANGED,
                          MIL.M3dgeoInquire(BoundingBox, MIL.M_SIZE_X, MIL.M_NULL) * 0.5,
                          MIL.M3dgeoInquire(BoundingBox, MIL.M_SIZE_Y, MIL.M_NULL) * 0.5,
                          MIL.M_UNCHANGED, MIL.M_DEFAULT);

            MIL_INT BoxLabel = (MIL_INT) MIL.M3dgeoDraw3d(MIL.M_DEFAULT, BoundingBox, Mil3dGraList, MIL.M_ROOT_NODE, MIL.M_DEFAULT);
            MIL.M3dgraControl(Mil3dGraList, BoxLabel, MIL.M_EDITABLE, MIL.M_ENABLE);

            MIL_ID CroppingBox = MIL.M3dgeoAlloc(MilSystem, MIL.M_GEOMETRY, MIL.M_DEFAULT, MIL.M_NULL);

            // Create a hook to crop the container when the box is modified in the graphics list.
            SPickStruct PickStruct = new SPickStruct();
            PickStruct.Box = CroppingBox;
            PickStruct.BoxLabel = BoxLabel;
            PickStruct.Gralist = Mil3dGraList;
            PickStruct.OriginalContainer = OriginalContainer;
            PickStruct.CroppedContainer = CroppedContainer;

            GCHandle hUserData = GCHandle.Alloc(PickStruct);
            MIL_3DGRA_HOOK_FUNCTION_PTR HandlerFunctionPtr = new MIL_3DGRA_HOOK_FUNCTION_PTR(BoxModifiedHandler);

            MIL.M3dgraHookFunction(Mil3dGraList, MIL.M_EDITABLE_GRAPHIC_MODIFIED, HandlerFunctionPtr, GCHandle.ToIntPtr(hUserData));

            // Crop a first time before starting the interactivity.
            RetrieveBoxAndCrop(ref PickStruct);

            // Open the 3d display.
            MIL.M3ddispSelect(Mil3dDisplay, MIL.M_NULL, MIL.M_OPEN, MIL.M_DEFAULT);

            Console.Write("A 3D point cloud is restored from a ply file and displayed.\n");
            Console.Write("The box is editable.\n");
            Console.Write("Only the points inside the interactive box are shown.\n");
            Console.Write("\n");
            Console.Write("- Use side box handles to resize.\n");
            Console.Write("- Use axis arrow tips to translate.\n");
            Console.Write("- Use axis arcs to rotate.\n");
            Console.Write("\n");
            Console.Write("Press <Enter> to end.\n");
            Console.ReadKey();

            MIL.M3dgeoFree(CroppingBox);
            MIL.M3dgeoFree(BoundingBox);
            MIL.MbufFree(CroppedContainer);
            MIL.MbufFree(OriginalContainer);
            MIL.M3ddispFree(Mil3dDisplay);
            MIL.MappFreeDefault(MilApplication, MilSystem, MIL.M_NULL, MIL.M_NULL, MIL.M_NULL);
            }

        static public MIL_INT BoxModifiedHandler(MIL_INT HookType, MIL_ID EventId, IntPtr UserDataPtr)
            {
            GCHandle hUserData = GCHandle.FromIntPtr(UserDataPtr);
            SPickStruct PickStruct = hUserData.Target as SPickStruct;

            RetrieveBoxAndCrop(ref PickStruct);

            return 0;
            }

        static public void RetrieveBoxAndCrop(ref SPickStruct PickStruct)
            {
            // Retrieve the edited box from the graphics list.
            MIL.M3dgraCopy(PickStruct.Gralist, PickStruct.BoxLabel, PickStruct.Box, MIL.M_DEFAULT, MIL.M_GEOMETRY, MIL.M_DEFAULT);

            // Crop the point cloud using the retrieved box.
            MIL.M3dimCrop(PickStruct.OriginalContainer, PickStruct.CroppedContainer, PickStruct.Box, MIL.M_NULL, MIL.M_SAME, MIL.M_DEFAULT);
            }

        // Check for required files to run the example.
        static bool CheckForRequiredMILFile(string FileName)
          {
          MIL_INT FilePresent = MIL.M_NO;

          MIL.MappFileOperation(MIL.M_DEFAULT, FileName, MIL.M_NULL, MIL.M_NULL, MIL.M_FILE_EXISTS, MIL.M_DEFAULT, ref FilePresent);
          if(FilePresent == MIL.M_NO)
             {
             Console.Write("The footage needed to run this example is missing. You need \n");
             Console.Write("to obtain and apply a separate specific update to have it.\n\n");

             Console.Write("Press <Enter> to end.\n\n");
             Console.ReadKey();
             }

          return (FilePresent == MIL.M_YES);
          }

       //----------------------------------------------------------------------------*
       // Allocates a 3D display and returns its MIL identifier.
       //----------------------------------------------------------------------------*
       static MIL_ID Alloc3dDisplayId(MIL_ID MilApplication, MIL_ID MilSystem)
          {
          MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE);
          MIL_ID MilDisplay3D = MIL.M3ddispAlloc(MilSystem, MIL.M_DEFAULT, "M_DEFAULT", MIL.M_DEFAULT, MIL.M_NULL);
          MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE);

          if (MilDisplay3D == MIL.M_NULL)
             {
             Console.Write("\n");
             Console.Write("The current system does not support the 3D display.\n");
             Console.Write("Press any key to exit.\n");
             Console.ReadKey();
             MIL.MsysFree(MilSystem);
             MIL.MappFree(MilApplication);
             Environment.Exit(0);
             }

          return MilDisplay3D;
          }
       }
    }
