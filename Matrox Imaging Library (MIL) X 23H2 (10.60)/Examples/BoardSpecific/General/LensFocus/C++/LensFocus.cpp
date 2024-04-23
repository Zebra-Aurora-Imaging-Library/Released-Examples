/*****************************************************************************/
/* 
 * File name: LensFocus.cpp
 *
 * Synopsis:  This program shows how to adjust the focus on the Matrox Iris GTR or Matrox Iris GTX
 *            using a liquid lens.
 *            On Matrox Iris GTR and Matrox Iris GTX, the lens movement is done using MdigControl()
 *            with M_FOCUS.
 *
 *     Note : Under MIL-Lite, the MdigFocus() function is not supported.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
{
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("LensFocus\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program shows how to adjust the \n")\
             MIL_TEXT("focus on the Matrox Iris GTR or Matrox Iris GTX using a liquid lens. \n\n")\
             MIL_TEXT("On Matrox Iris GTR and Matrox Iris GTX, the lens movement is done using\n")\
             MIL_TEXT("MdigControl() with M_FOCUS.\n\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, display, buffer, digitizer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n"));
   MosGetch();
}

/* Autofocus search properties. */
#define FOCUS_MAX_POSITION_VARIATION   M_DEFAULT
#define FOCUS_MODE                     M_SMART_SCAN
#define FOCUS_SENSITIVITY              1

#define PG_UP_PG_DN_FIRST_CHAR         224
#define PG_UP                          73
#define PG_DN                          81
#define INCREMENT                      50

/* User Data structure definition. */
typedef struct 
{  
   MIL_ID      Digitizer;
   MIL_ID      FocusImage;
   MIL_ID      Display;
   long        Iteration;
}  DigHookUserData;

/* Autofocus callback function responsible for moving the lens. */
MIL_INT MFTYPE MoveLensHookFunction(MIL_INT HookType,
                                    MIL_INT Position,
                                    void*   UserDataHookPtr);


/*****************************************************************************/
/*  Main application function.                                               */
int MosMain(void)
{
   MIL_ID  MilApplication,                  /* Application identifier.       */
           MilSystem,                       /* System identifier.            */
           MilDisplay,                      /* Display identifier.           */
           MilDigitizer,                    /* Digitizer identifier.         */
           MilImage;                        /* Grab buffer.                  */
   MIL_INT FocusPos;                        /* Best focus position           */
   MIL_INT LicenseModules;                  /* List of available MIL modules.*/
   MIL_INT BoardType;
   MIL_INT FocusPersistence;
   MIL_INT FocusPersistentValue;
   MIL_INT MaxValue;
   MIL_INT MinValue;
   MIL_INT c = 0;

   PrintHeader();

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, &MilDigitizer, &MilImage);
   MbufClear(MilImage, 0);

   /* Select image on the display. */ 
   MdispSelect(MilDisplay, MilImage);

   /* Grab the first image. */
   MdigGrab(MilDigitizer, MilImage);

   MdigInquire(MilDigitizer, M_FOCUS + M_MAX_VALUE, &MaxValue);
   MdigInquire(MilDigitizer, M_FOCUS + M_MIN_VALUE, &MinValue);
   
   /* Inquire the board type. */
   MsysInquire(MilSystem, M_BOARD_TYPE, &BoardType);
   if (BoardType != M_IRIS_GTR && BoardType != M_IRIS_GTX)
      {
      MosPrintf(MIL_TEXT("This example can only execute on Matrox Iris GTR or Matrox Iris GTX.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      goto end;
      }

   /* Inquire the actual focus position. */
   MdigInquire(MilDigitizer, M_FOCUS, &FocusPos);
   if (FocusPos == M_INVALID)
      {
      MosPrintf(MIL_TEXT("Cannot communicate with liquid lens.  At power off, verify connection.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      goto end;
      }

   /* Inquire focus persistence and focus persistent value */
   /* If M_FOCUS_PERSISTENCE is enabled, then MdigAlloc() will initialize lens at */
   /* the position given by M_FOCUS_PERSISTENT_VALUE. */
   MdigInquire(MilDigitizer, M_FOCUS_PERSISTENCE, &FocusPersistence);
   MdigInquire(MilDigitizer, M_FOCUS_PERSISTENT_VALUE, &FocusPersistentValue);
   MappInquire(MilApplication, M_LICENSE_MODULES, &LicenseModules);

   /* If we are not licensed for MdigFocus(), use manual adjustment. */
   if (!(LicenseModules & M_LICENSE_IM))
      {
      MdigGrabContinuous(MilDigitizer, MilImage);

      MosPrintf(MIL_TEXT("\nMANUAL FOCUS:\n"));
      MosPrintf(MIL_TEXT("-------------\n\n"));
      if (FocusPersistence == M_ENABLE)
         {
         MosPrintf(MIL_TEXT("Focus persistence is enabled.\n"));
         MosPrintf(MIL_TEXT("It's position is set to %d.\n"), FocusPersistentValue);
         }
      MosPrintf(MIL_TEXT("Press '+/-' to do fine focus adjustment.\n"));
      MosPrintf(MIL_TEXT("Press 'PgUp/PgDn' to do coarse focus adjustment.\n"));
      MosPrintf(MIL_TEXT("Press 'q' to quit.\n\n"));
      do
         {
         c = MosGetch();
         if (c == PG_UP_PG_DN_FIRST_CHAR)
            c = MosGetch();

         /* The M_FOCUS digitizer control type is used to change the lens position.
            We use the M_WAIT attribute to be sure that the lens position is stable 
            after the control.
         */
         switch (c)
            {
               case '+':
                  FocusPos = (FocusPos < MaxValue)? FocusPos + 1: FocusPos;
                  MdigControl(MilDigitizer, M_FOCUS + M_WAIT, FocusPos);
                  break;
               case '-':
                  FocusPos = (FocusPos > MinValue)? FocusPos - 1: FocusPos;
                  MdigControl(MilDigitizer, M_FOCUS + M_WAIT, FocusPos);
                  break;
               case PG_UP:
                  FocusPos = ((FocusPos + INCREMENT) < MaxValue) ? FocusPos + INCREMENT : MaxValue;
                  MdigControl(MilDigitizer, M_FOCUS + M_WAIT, FocusPos);
                  break;
               case PG_DN:
                  FocusPos = ((FocusPos - INCREMENT) > MinValue) ? FocusPos - INCREMENT: MinValue;
                  MdigControl(MilDigitizer, M_FOCUS + M_WAIT, FocusPos);
                  break;
               case 'q':
                  break;
               default:
                  c = 0;
                  break;
            }
         if (c != 0)
            MosPrintf(MIL_TEXT("\rFocus position: %4d."), FocusPos);
         } while (c != 'q');
      MosPrintf(MIL_TEXT("\n"));

      /* Stop continuous grab. */
      MdigHalt(MilDigitizer);
      }
   else
      {
      DigHookUserData UserData; /* User data passed to the hook  */

      /* Initialize user data needed within the hook function. */
      UserData.Digitizer   = MilDigitizer;
      UserData.FocusImage  = MilImage;
      UserData.Iteration   = 0L;
      UserData.Display     = MilDisplay;

      /* Pause to show the original image. */
      MosPrintf(MIL_TEXT("\nAUTOFOCUS:\n"));
      MosPrintf(MIL_TEXT("----------\n\n"));
      if (FocusPersistence == M_ENABLE)
         {
         MosPrintf(MIL_TEXT("Focus persistence is enabled. \n"));
         MosPrintf(MIL_TEXT("An image was grabbed with lens set at postion %d.\n"), FocusPersistentValue);
         }
      MosPrintf(MIL_TEXT("Automatic focusing operation will be done on this image.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));  
      MosGetch();
      MosPrintf(MIL_TEXT("Autofocusing...\n\n"));  
  
      /* Perform Autofocus. 
         With MdigControl() and M_FOCUS, we will change lens position.
      */ 

      MdigFocus(MilDigitizer,
                MilImage,
                M_DEFAULT,
                MoveLensHookFunction,
                &UserData,
                MinValue,
                (MaxValue - MinValue) / 2, /* Start in the middle of the lens position range. */
                MaxValue,
                FOCUS_MAX_POSITION_VARIATION,
                FOCUS_MODE + FOCUS_SENSITIVITY,
                &FocusPos); 

      /* Grab a new image at optimal focus. */
      MdigGrab(MilDigitizer, MilImage);

      /* Print the best focus position and number of iterations. */
      MosPrintf(MIL_TEXT("The best focus position is %d.\n"), (int)FocusPos);
      MosPrintf(MIL_TEXT("The best focus position found in %d iterations.\n\n"), 
                                                            (int)UserData.Iteration);
      }

   MosPrintf(MIL_TEXT("Press 's' to enable persistent focus and keep the position.\n"));
   MosPrintf(MIL_TEXT("Press 'd' to disable persistence.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));  
   c = MosGetch();
   if (c == 's')
      {
      MdigControl(MilDigitizer, M_FOCUS_PERSISTENCE, M_ENABLE);
      MdigControl(MilDigitizer, M_FOCUS_PERSISTENT_VALUE, FocusPos);
      }
   else if(c == 'd')
      {
      MdigControl(MilDigitizer, M_FOCUS_PERSISTENCE, M_DISABLE);
      }

end:
  /* Free all allocations. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);

   return 0;
}

/********************************************************************************/
/* Autofocus hook function responsible to move the lens.                        */

MIL_INT MFTYPE MoveLensHookFunction(MIL_INT HookType,
                                    MIL_INT Position,
                                    void*   UserDataHookPtr)
   {
   DigHookUserData *UserData = (DigHookUserData *)UserDataHookPtr;

   /* Here, the lens position must be changed according to the Position parameter.
      We use the M_WAIT attribute to be sure that the lens position is stable 
      after the control.
   */
   if(HookType == M_CHANGE || HookType == M_ON_FOCUS)
      {
      MdigControl(UserData->Digitizer, M_FOCUS + M_WAIT, Position);
      UserData->Iteration++;
      }

   return 0;
   }
