﻿/**********************************************************************************************/
/*
* File name: DMILBasic.cpp
*
* Synopsys: Basic processing example using Distributed MIL.
*
*           A local image file is processed by a remote DMIL system and the result
*           is displayed locally.
*
*           Warning: The default processing system in this example is a Host system
*                    located on the same PC that runs the example
*                    (see: "dmiltcp://localhost/M_SYSTEM_HOST"). This can be changed
*                    to explicitly target a remote PC with a DMIL installation.
*
*                    Alternatively, the MILConfig utility can be used to specify the
*                    default target DMIL system to use for all MIL examples.
*                    To do so, open MILConfig and access the "Controlling" subfolder
*                    of the "Distributed MIL" folder to detect the DMIL systems available
*                    on the remote PC, and to add them as registered DMIL systems, which
*                    makes them available to be set as the default. Then, using the
*                    "Default Values" subpage of the "General" folder in MILConfig, you can
*                    set your chosen system as the default System Type. Finally, set
*                    EXPLICIT_DMIL_SYSTEM_DESCRIPTOR in the code below to M_NO to use
*                    the default system specified in MILConfig.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*
*/

/* MIL Header. */
#include <mil.h>

/* Sets whether the DMIL target system is explicitly specified or
if the default DMIL system specified in MILConfig should be used.
*/
#define EXPLICIT_DMIL_SYSTEM_DESCRIPTOR M_YES

/* Target DMIL system specification.
Format is: "DMILProtocol://TargetPCName/TargetSystemType".
*/
#if (EXPLICIT_DMIL_SYSTEM_DESCRIPTOR)
#define SLAVE_SYSTEM_DESCRIPTOR   MIL_TEXT("dmiltcp://localhost/M_SYSTEM_HOST")
#else
#define SLAVE_SYSTEM_DESCRIPTOR   MIL_TEXT("M_SYSTEM_DEFAULT")
#endif

/* Specify the image file to process (on the local PC by default).
A remote image file can be specified using the "remote:///" prefix
(eg: "remote:///C:\TargetDirectory\BaboonRGB.mim").
*/
#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("BaboonRGB.mim")

/* Display format to be used. */
#define DISPLAY_FORMAT MIL_TEXT("M_DEFAULT")

/* Main */
/* ---- */

int MosMain(void)
{
   MIL_ID MilApplication,       /* Application Identifier.  */
      MilRemoteSystem,          /* System Identifier.       */
      MilRemoteImage,           /* Image buffer Identifier. */
      MilDisplay;               /* Display Identifier.      */

   /* Allocate a MIL application and DMIL system. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);

   /* Allocate the remote DMIL system. */
   MsysAlloc(M_DEFAULT, SLAVE_SYSTEM_DESCRIPTOR, M_DEFAULT, M_DEFAULT, &MilRemoteSystem);

   /* Verify that a remote DMIL system is actually targeted.
   Since DMIL code is transparent and portable, it can also run locally
   on the PC without using DMIL.
   */
   if (MsysInquire(MilRemoteSystem, M_LOCATION, M_NULL) != M_REMOTE)
   {
      MosPrintf(MIL_TEXT("WARNING: Your target system is not a Distributed MIL system.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue anyway.\n\n"));
      MosGetch();
   }

   /* Restore the source image into a remote image buffer automatically allocated
   on the remote target system.
   */
   MbufRestore(IMAGE_FILE, MilRemoteSystem, &MilRemoteImage);

   /* Allocate a display and display the image. By default, the display will be visible
   on the local PC. To have the image displayed on the remote PC, use
   M_DEFAULT+M_REMOTE_DISPLAY as the InitFlag parameter value.
   */
   MdispAlloc(MilRemoteSystem, M_DEFAULT, DISPLAY_FORMAT, M_DEFAULT, &MilDisplay);
   MdispSelect(MilDisplay, MilRemoteImage);

   /* Pause for user input.*/
   MosPrintf(MIL_TEXT("\nDMIL basic processing:\n"));
   MosPrintf(MIL_TEXT("----------------------\n\n"));
   MosPrintf(MIL_TEXT("This example processes a buffer using a remote system.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   /* Process the image with the remote target system.
   Since all the Image buffers provided to the function are allocated on the
   same remote DMIL system, the processing command will automatically be sent
   to that system for execution. Note that the DMIL commands that do not return a
   a value are asynchronous and return control to the calling thread immediately.
   This means that the Master function is then free to do other tasks while the
   Slave function is processing the command.
   */
   MimHistogramEqualize(MilRemoteImage, MilRemoteImage, M_UNIFORM, M_NULL, 0, 255);

   /* Explicitly force the Master's calling thread to wait until the end of the DMIL
   function execution on the Slave, and the update of the Display, before exiting.
   */
   MthrWait(M_DEFAULT, M_THREAD_WAIT, M_NULL);

   /* Pause for user input.*/
   MosPrintf(MIL_TEXT("Contrast enhancement was performed using the remote DMIL system.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(MilRemoteImage);
   MdispFree(MilDisplay);
   MsysFree(MilRemoteSystem);
   MappFree(MilApplication);

   return 0;
}
