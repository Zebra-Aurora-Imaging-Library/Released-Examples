/************************************************************************************/
/*
 * File name: InteractiveBayer.cpp
 *
 * Synopsis:  This program shows how to perform Bayer-to-Color conversion.
 *
 * This example requires a camera that provides a raw bayer image.
 * Make sure to modify the settings in feature browser to set the bayer
 * pattern in the Pixel Format setting.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include "mil.h"

int MosMain(void)
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\nInteractiveBayer\n\n"));

   MIL_ID   MilApplication,
            MilSystem,
            MilDigitizer,
            MilDisplay,
            MilWBCoefficients,
            MilImageDisp,
            MilImageGrab;

   /* User array for white balance coefficients. */
   float WBCoefficients[3];

   /* Specify the Bayer pattern of your camera. */
   MIL_INT ConversionType;

   /* Specify the Bayer conversion is done by the digitizer. */
   MIL_INT BayerConversion;

   /* Buffer characteristics. */
   MIL_INT XSize;
   MIL_INT YSize;

   /* Allocate an application. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay, &MilDigitizer, M_NULL);

   /* Inquire about the Bayer properties. */ 
   MIL_INT Error=M_NULL_ERROR;

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   
   /* Enable the bayer conversion */
   MdigControl(MilDigitizer, M_BAYER_CONVERSION, M_ENABLE);

   /* Get the bayer pattern selected by the user */
   MdigInquire(MilDigitizer, M_BAYER_PATTERN, &ConversionType);
   Error = MappGetError(M_GLOBAL + M_SYNCHRONOUS, 0);

   /* Disable the bayer conversion so we get the raw image. */
   MdigControl(MilDigitizer, M_BAYER_CONVERSION, M_DISABLE);

   /* Check to see if the bayer conversion is really disabled */
   MdigInquire(MilDigitizer, M_BAYER_CONVERSION, &BayerConversion);

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if ((Error != M_NULL_ERROR) || (ConversionType==M_NULL) || (BayerConversion==M_ENABLE))
      {
      /* If there is no Bayer pattern used by the camera,    */
      /* or if the Bayer conversion is done by the digitizer */
      /* then release the allocated objects and quit.        */
      MosPrintf(MIL_TEXT("This example requires a camera that provides a raw bayer image.\n"));
      MosPrintf(MIL_TEXT("Make sure to modify the settings in feature browser to set the\n"));
      MosPrintf(MIL_TEXT("bayer pattern in the Pixel Format setting.\n\n"));
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, M_NULL);
      MosPrintf(MIL_TEXT("Press <ENTER> to end\n"));
      MosGetchar();
      return 0;
      }

   XSize = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   YSize = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

   /* Allocate a display buffer. */
   MbufAllocColor(MilSystem, 3, XSize, YSize, 8L+M_UNSIGNED, M_PROC+M_IMAGE+M_DISP,&MilImageDisp);

   /* Allocate a grab buffer. */
   MbufAllocColor(MilSystem, 1, XSize, YSize, 8L+M_UNSIGNED, M_IMAGE+M_DISP+M_GRAB+M_PROC, &MilImageGrab);

   /* Allocate an array for the white balance coefficients. */
   MbufAlloc1d(MilSystem, 3, 32+M_FLOAT, M_ARRAY, &MilWBCoefficients);

   /* Display the image. */
   MbufClear(MilImageDisp, M_RGB888(0, 0, 0));
   MdispSelect(MilDisplay, MilImageDisp);
   
   /* Ask the user for a white image for white balance. */
   MosPrintf(MIL_TEXT("Place a white reference in front of the\n"));
   MosPrintf(MIL_TEXT("camera and press <ENTER> when ready.\n"));

   do
      {
      /* Grab a white Bayer image. */
      MdigGrab(MilDigitizer, MilImageGrab);

      /* Convert the white Bayer image to color without white balance. */
      MbufBayer(MilImageGrab, MilImageDisp, M_DEFAULT, ConversionType);
      }
   while (!MosKbhit());

   /* Determine the white balance coefficients. */
   MbufBayer(MilImageGrab, MilImageDisp, MilWBCoefficients,
      ConversionType+M_WHITE_BALANCE_CALCULATE);
   
   /* Print the computed coefficients. */
   MbufGet(MilWBCoefficients, (void *) &WBCoefficients[0]);
   MosPrintf(MIL_TEXT("\nWhite balance correction coefficients : %f, %f, %f\n\n"),
      WBCoefficients[0], WBCoefficients[1], WBCoefficients[2]);
   
   /* Grab a new Bayer image with white balance correction. */
   MosPrintf(MIL_TEXT("Press <ENTER> to grab white balanced images\n"));
   MosGetchar();
   
   do
      {
      /* Grab a Bayer image. */
      MdigGrab(MilDigitizer, MilImageGrab);

      /* Convert the Bayer image to color. */
      MbufBayer(MilImageGrab, MilImageDisp, MilWBCoefficients, ConversionType);
      }
   while (!MosKbhit());

   MosPrintf(MIL_TEXT("Press <ENTER> to end\n"));
   MosGetchar();

   /* Terminate and free allocated resources. */
   MbufFree(MilImageGrab);
   MbufFree(MilImageDisp);
   MbufFree(MilWBCoefficients);
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, M_NULL);
   
   return 0;
   }
