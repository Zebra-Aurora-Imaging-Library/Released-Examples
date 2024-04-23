/*****************************************************************************************/
/*
 * File name: DMILAddConstantMain.cpp
 *
 * Synopsis:  This file contains the main program to test and time the different 
 *            versions of a custom add constant MIL function MIL created with 
 *            the MIL Function Development module.
 *
 *            The slave functions can be found in the DistributedMILAddConstantSlave project.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

/* Standard headers. */
#include <mil.h>

/* Target image file name */
#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("Board.mim")

/* Timing loop iterations. */
#define NB_LOOP 100

/* Defines for the different versions of the target function to run on the remote computer. */
#define USE_C           0 /* Target C function C.                 */
#define USE_MIL         1 /* Target function MIL.                 */
#define NB_VERSIONS     2 /* Number of different versions to call. */
static MIL_CONST_TEXT_PTR VersionName[] = {MIL_TEXT("C"), MIL_TEXT("MIL")};

#define SLAVE_SYSTEM_DESCRIPTOR   M_SYSTEM_DEFAULT


/* Slave dll path and name */
#define SLAVE_DLL_NAME MIL_TEXT("dmiladdconstantslave")

#if M_MIL_USE_WINDOWS
#define SLAVE_DLL_TARGET_NAME M_USER_DLL_DIR SLAVE_DLL_NAME MIL_TEXT(".dll")
#elif M_MIL_USE_LINUX
#define SLAVE_DLL_TARGET_NAME M_USER_DLL_DIR MIL_TEXT("lib") SLAVE_DLL_NAME MIL_TEXT(".so")
#endif

//The display format
#define DISPLAY_FORMAT MIL_TEXT("M_DEFAULT")

/* Master functions prototypes: */
/* Custom C function (See DTKAddConstantC.c) */
void MFTYPE AddConstantC(MIL_ID SrcImage, MIL_ID DstImage, MIL_UINT Constant);
static bool SetupDMILExample(MIL_ID MilSystem);


/* Main to test the AddConstant functions. */
/* --------------------------------------- */

int MosMain(void)
   {
   MIL_ID MilApplication,           /* Application Identifier.  */
          MilSystem,                /* System Identifier.       */
          MilDisplay,               /* Display Identifier.      */
          MilImageDisp,             /* Image buffer Identifier. */ 
          MilImageSrc,              /* Image buffer Identifier. */ 
          MilImageDst;              /* Image buffer Identifier. */ 

   int    Version, n;            /* Loop variables.          */
   MIL_DOUBLE Time;                  /* Processing time.         */

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

   /* Restore the source image into one display buffer and 2 processing buffers. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilImageDisp);
   MbufRestore(IMAGE_FILE, MilSystem, &MilImageSrc);
   MbufRestore(IMAGE_FILE, MilSystem, &MilImageDst);

   /* Display the source image. */
   MdispSelect(MilDisplay, MilImageDisp);
   
   /* Pause */
   MosPrintf(MIL_TEXT("\nMIL DTK:\n"));
   MosPrintf(MIL_TEXT("--------\n\n"));
   MosPrintf(MIL_TEXT("This example tests and times a custom asynchronous MIL function\n"));
   MosPrintf(MIL_TEXT("that adds a constant to an image and compare it's speed with the\n"));
   MosPrintf(MIL_TEXT("equivalent MimArith() MIL function.\n"));
   MosPrintf(MIL_TEXT("Press a key to continue.\n\n"));
   MosGetch();

   /* Process the image using the C version of the custom MIL function. */
   AddConstantC(MilImageSrc, MilImageDisp, 0x40);
   
   /* Print comment */
   MosPrintf(MIL_TEXT("A constant was added to the image using a user-made MIL function.\n\n"));

   /* Call and time all the versions of the add constant function.
      Do it in loop for more precision.
   */
   for (Version = 0; Version < NB_VERSIONS; Version++)
      {
      for (n= 0; n < NB_LOOP+1; n++)
         {
         /* Don't time the first iteration (avoid DLL load time, ...). */
         if (n == 1)
            MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);

         /* Call the proper version */
         switch(Version)
            {
            case USE_C:
                 AddConstantC(MilImageSrc, MilImageDst, 0x40);
                 break;

            case USE_MIL:
                 MimArith(MilImageSrc, 0x40, MilImageDst, M_ADD_CONST+M_SATURATION);  
                 break;
            }
         }

      /* Read and print the time.*/
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &Time);
      MosPrintf(MIL_TEXT("Add constant time (%s version): %.3f ms.\n"), VersionName[Version], 
         Time*1000/NB_LOOP);
      }

   /* Pause */
   MosPrintf(MIL_TEXT("Press a key to terminate.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(MilImageSrc);
   MbufFree(MilImageDst);
   MbufFree(MilImageDisp);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
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


