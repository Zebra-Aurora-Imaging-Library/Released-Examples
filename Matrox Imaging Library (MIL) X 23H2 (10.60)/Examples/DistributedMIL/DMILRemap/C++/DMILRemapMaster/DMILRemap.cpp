/*****************************************************************************************/
/*
 * File name: DMILRemap.cpp
 *
 * Synopsis:  This example shows how to use the MIL Function Development module to 
 *            create a custom asynchronous MIL function that does a series of MIL 
 *            commands on a target system in a single call from the host. The function 
 *            shows how to avoid having the Host to wait for remote calculation result 
 *            and also show how to reduce the overhead of sending multiple commands by
 *            grouping them in a meta MIL function. 
 *
 *            The example creates a Master MIL function that register all the parameters
 *            to MIL and calls the Slave function on the target system. The Slave function 
 *            retrieves all the parameters, finds the Max and Min of the source buffer and 
 *            remaps it to have its full range (min at 0x0 and the max at 0xFF).
 *
 *            The slave function can be found in the DistributedMilRemapSlave project.
 *             
 *            Note: For simplicity, the images are assumed to be 8-bit unsigned.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

/* MIL Header. */
#include <mil.h>

/* Master MIL functions declarations */
void MFTYPE CustomRemap(MIL_ID SrcImage, MIL_ID DstImage, MIL_UINT Option);


/* Standard I/0 */

/* Target image file name */
#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("Wafer.mim")

#define SLAVE_SYSTEM_DESCRIPTOR   M_SYSTEM_DEFAULT


/* Slave dll path and name */
#define SLAVE_DLL_NAME MIL_TEXT("dmilremapslave")

#if M_MIL_USE_WINDOWS
#define SLAVE_DLL_TARGET_NAME M_USER_DLL_DIR SLAVE_DLL_NAME MIL_TEXT(".dll")
#elif M_MIL_USE_LINUX
#define SLAVE_DLL_TARGET_NAME M_USER_DLL_DIR MIL_TEXT("lib") SLAVE_DLL_NAME MIL_TEXT(".so")
#endif

//The display format
#define DISPLAY_FORMAT MIL_TEXT("M_DEFAULT")

static bool SetupDMILExample(MIL_ID MilSystem);

/* Main to test the function. */
/* -------------------------- */

int MosMain(void)
{
   MIL_ID MilApplication,           /* Application Identifier.  */
          MilSystem,                /* System Identifier.       */
          MilDisplay,               /* Display Identifier.      */
          MilImage;                 /* Image buffer Identifier. */

   
   /* Allocate application, system and display. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, SLAVE_SYSTEM_DESCRIPTOR, M_DEFAULT, M_DEFAULT, &MilSystem);

   /* Validate that the example can be run correctly*/
   if (!SetupDMILExample(MilSystem))
      {
      MsysFree(MilSystem);
      MappFree(MilApplication);
      return -1;
      }

   MdispAlloc(MilSystem, M_DEFAULT, DISPLAY_FORMAT, M_DEFAULT, &MilDisplay);

   /* Restore source image into an automatically allocated image buffer. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);


   /* Display the image. */
   MdispSelect(MilDisplay, MilImage);
   
   /* Pause */
   MosPrintf(MIL_TEXT("\nMIL DTK:\n"));
   MosPrintf(MIL_TEXT("--------\n\n"));
   MosPrintf(MIL_TEXT("Custom asynchronous processing function:\n\n"));
   MosPrintf(MIL_TEXT("This example creates a custom MIL function "));
   MosPrintf(MIL_TEXT("that maximize the contrast.\n"));
   MosPrintf(MIL_TEXT("Press a key to continue.\n\n"));
   MosGetch();

   /* Process the image with the custom MIL function. */
   CustomRemap(MilImage, MilImage, M_DEFAULT);
   
   /* Pause */
   MosPrintf(MIL_TEXT("A smart image remapping was done on the image using "));
   MosPrintf(MIL_TEXT("a user made MIL function.\n"));
   MosPrintf(MIL_TEXT("Press a key to terminate.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(MilImage);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
}



/* Master MIL Function definition. */
/* ------------------------------- */

/* MIL Function specifications */
#define FUNCTION_OPCODE_REMAP   (M_USER_FUNCTION+1)
#define FUNCTION_NB_PARAM       3

/* Slave function name and DLL path */
#define SLAVE_FUNC_NAME      MIL_TEXT("SlaveCustomRemap")

void MFTYPE CustomRemap(MIL_ID SrcImage, MIL_ID DstImage, MIL_UINT Option)
{
   MIL_ID   Func;
   
   /* Allocate a MIL function context that will be used to call a target 
      slave function to do the processing.
   */
   MfuncAlloc(MIL_TEXT("CustomRemap"), 
              FUNCTION_NB_PARAM,
              M_NULL, SLAVE_DLL_NAME, SLAVE_FUNC_NAME,  
              FUNCTION_OPCODE_REMAP, 
              M_ASYNCHRONOUS_FUNCTION, 
              &Func);

   /* Register the parameters. */
   MfuncParamMilId  (Func, 1, SrcImage, M_IMAGE, M_IN  + M_PROC);
   MfuncParamMilId  (Func, 2, DstImage, M_IMAGE, M_OUT + M_PROC);
   MfuncParamMilUint(Func, 3, Option);

   /* Call the target Slave function. */
   MfuncCall(Func);

   /* Free the MIL function context. */
   MfuncFree(Func);
}

bool SetupDMILExample(MIL_ID MilSystem)
   {
   MIL_ID MilSystemOwnerApplication;/* System owner application.*/
   bool CheckExistence = false;
   // Now we check if the system is remote
   if (MsysInquire(MilSystem, M_LOCATION, M_NULL) != M_REMOTE)
      {
      MosPrintf(MIL_TEXT("This example requires the default system to be a remote system.\n"));
      MosPrintf(MIL_TEXT("Please select a remote system as the default.\n"));
      MosPrintf(MIL_TEXT("If no remote systems are registered "));
      MosPrintf(MIL_TEXT("please go to the DistributedMIL->Connections page, "));
      MosPrintf(MIL_TEXT("register a remote system, "));
      MosPrintf(MIL_TEXT("and then select it as the default system.\n"));
      MosGetch();
      return false;
      }

   /* Inquire the system's owner application used to copy the slave dll with
   MappFileOperation. */
   MsysInquire(MilSystem, M_OWNER_APPLICATION, &MilSystemOwnerApplication);

   /* Copy the slave dll to the destination computer if they are compatible */
   if ((MappInquire(M_DEFAULT, M_PLATFORM_BITNESS, M_NULL) ==
      MappInquire(MilSystemOwnerApplication, M_PLATFORM_BITNESS, M_NULL)) &&
      (MappInquire(M_DEFAULT, M_PLATFORM_OS_TYPE, M_NULL) == M_OS_WINDOWS) &&
      (MappInquire(MilSystemOwnerApplication, M_PLATFORM_OS_TYPE, M_NULL) == M_OS_WINDOWS) &&
      (MsysInquire(MilSystem, M_DISTRIBUTED_MIL_TYPE, M_NULL) == M_DMIL_REMOTE))
      {
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
      MappFileOperation(M_DEFAULT, SLAVE_DLL_TARGET_NAME, MilSystemOwnerApplication, M_NULL,
         M_FILE_COPY_MIL_USER_DLL, M_DEFAULT, M_NULL);
      if (0 != MappGetError(M_DEFAULT, M_CURRENT, M_NULL))
         {
         // we have an error during the copy, check the existence
         MosPrintf(MIL_TEXT("There was an error while copying the slave library.\n"));
         MosPrintf(MIL_TEXT("Checking if one is present on the remote system.\n"));
         CheckExistence = true;
         }
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
      }
   else
      CheckExistence = true;

   if (CheckExistence)
      {
      MIL_INT DllExists = M_NO;

      MappFileOperation(MilSystemOwnerApplication, SLAVE_DLL_NAME, M_NULL, M_NULL,
         M_FILE_EXISTS_MIL_USER_DLL, M_DEFAULT, &DllExists);

      if (DllExists != M_YES)
         {
         MosPrintf(MIL_TEXT("The slave library was NOT copied to the remote system.\n"));
         MosPrintf(MIL_TEXT("Make sure it is present for the example to work properly.\n"));
         MosPrintf(MIL_TEXT("See DistributedMILExamples.txt in the DistributedMIL examples "));
         MosPrintf(MIL_TEXT("folder\nfor more information.\n"));
         MosPrintf(MIL_TEXT("Press a key to terminate.\n\n"));
         MosGetch();
         return false;
         }
      }
   return true;
   }
