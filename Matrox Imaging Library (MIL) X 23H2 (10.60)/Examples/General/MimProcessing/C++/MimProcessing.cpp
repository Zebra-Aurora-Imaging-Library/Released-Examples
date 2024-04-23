/*****************************************************************************/
/* 
 * File name: MImProcessing.cpp
 *
 * Synopsis:  This program show the usage of image processing. Under MIL lite, 
 *            it binarizes two images to isolate specific zones.
 *            Under MIL full, it also uses different image processing primitives 
 *            to determine the number of cell nuclei that are larger than a 
 *            certain size and show them in pseudo-color.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Target MIL image file specifications. */
#define IMAGE_FILE                     M_IMAGE_PATH MIL_TEXT("Cell.mbufi")
#define IMAGE_CUP                      M_IMAGE_PATH MIL_TEXT("PlasticCup.mim")
#define IMAGE_SMALL_PARTICLE_RADIUS    1

void ExtractParticlesExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay);
void ExtractForegroundExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay);

int MosMain(void)
{
   MIL_ID   MilApplication,      /* Application identifier.  */
            MilSystem,           /* System identifier.       */
            MilDisplay;          /* Display identifier.      */

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem,
                    &MilDisplay, M_NULL, M_NULL);

   /* Show header */
   MosPrintf(MIL_TEXT("\nIMAGE PROCESSING:\n"));
   MosPrintf(MIL_TEXT("-----------------\n\n"));
   MosPrintf(MIL_TEXT("This program shows two image processing examples.\n"));

   /* Example about extracting particles in an image */
   ExtractParticlesExample(MilApplication, MilSystem, MilDisplay);

   /* Example about isolating objects from the background in an image */
   ExtractForegroundExample(MilApplication, MilSystem, MilDisplay);

   /* Free all allocations. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, M_NULL, M_NULL);

   return 0;
}

void ExtractParticlesExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
{
   MIL_ID   MilImage,             /* Image buffer identifier. */
            MilExtremeResult = 0; /* Extreme result buffer identifier. */

#if (!M_MIL_LITE)
   MIL_ID   MilRemoteApplication = 0; /* Remote application identifier.*/
#endif

   MIL_INT  MaxLabelNumber = 0;   /* Highest label value. */
   MIL_INT  LicenseModules = 0;   /* List of available MIL modules */

   /* Restore source image and display it. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("\n1) Particles extraction:\n"));
   MosPrintf(MIL_TEXT("-----------------\n\n"));
   MosPrintf(MIL_TEXT("This first example extracts the dark particles in an image.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Binarize the image with an automatically calculated threshold so that
      particles are represented in white and the background removed.*/
   MimBinarize(MilImage, MilImage, M_BIMODAL + M_LESS_OR_EQUAL, M_NULL, M_NULL);

   /* Print a message for the extracted particles. */
   MosPrintf(MIL_TEXT("These particles were extracted from the original image.\n"));

#if (!M_MIL_LITE)
   /* If MIL IM module is available, count and label the larger particles. */
   MsysInquire(MilSystem, M_OWNER_APPLICATION, &MilRemoteApplication);
   MappInquire(MilRemoteApplication, M_LICENSE_MODULES, &LicenseModules);
   if(LicenseModules & M_LICENSE_IM)
   {
      /* Pause to show the extracted particles. */
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      /* Close small holes. */
      MimClose(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, M_BINARY);

      /* Remove small particles. */
      MimOpen(MilImage, MilImage, IMAGE_SMALL_PARTICLE_RADIUS, M_BINARY);

      /* Label the image. */
      MimLabel(MilImage, MilImage, M_DEFAULT);

      /*The largest label value corresponds to the number of particles in the image.*/
      MimAllocResult(MilSystem, 1L, M_EXTREME_LIST, &MilExtremeResult);
      MimFindExtreme(MilImage, MilExtremeResult, M_MAX_VALUE);
      MimGetResult(MilExtremeResult, M_VALUE, &MaxLabelNumber);
      MimFree(MilExtremeResult);

      /* Multiply the labeling result to augment the gray level of the particles. */
      MimArith(MilImage, (MIL_INT)(256L / (MIL_DOUBLE)MaxLabelNumber), MilImage,
               M_MULT_CONST);

      /* Display the resulting particles in pseudo-color. */
      MdispLut(MilDisplay, M_PSEUDO);

      /* Print results. */
      MosPrintf(MIL_TEXT("There were %d large particles in the original image.\n"),
                (int)MaxLabelNumber);
   }
#endif

   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Reset MilDisplay to M_DEFAULT. */
   MdispLut(MilDisplay, M_DEFAULT);

   /* Free all allocations. */
   MbufFree(MilImage);
}

void ExtractForegroundExample(MIL_ID MilApplication, MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MIL_ID   MilImage,             /* Image buffer identifier. */
            MilExtremeResult = 0; /* Extreme result buffer identifier. */
   MIL_INT  MaxLabelNumber = 0;   /* Highest label value. */
   MIL_INT  LicenseModules = 0;   /* List of available MIL modules */

   /* Restore source image and display it. */
   MbufRestore(IMAGE_CUP, MilSystem, &MilImage);
   MdispSelect(MilDisplay, MilImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("\n2) Background removal:\n"));
   MosPrintf(MIL_TEXT("-----------------\n\n"));
   MosPrintf(MIL_TEXT("This second example separates a cup on a table from the background using MimBinarize() with an M_DOMINANT mode.\n"));
   MosPrintf(MIL_TEXT("In this case, the dominant mode (black background) is separated from the rest. Note, using an M_BIMODAL mode\n"));
   MosPrintf(MIL_TEXT("would give another result because the background and the cup would be considered as the same mode.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   /* Binarize the image with an automatically calculated threshold so that
      cup and table are represented in white and the background removed.*/
   MimBinarize(MilImage, MilImage, M_DOMINANT + M_LESS_OR_EQUAL, M_NULL, M_NULL);

   /* Print a message for the extracted cup and table. */
   MosPrintf(MIL_TEXT("The cup and the table were separated from the background with M_DOMINANT mode of MimBinarize.\n"));
      
   /* Pause to show the result. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   /* Free all allocations. */
   MbufFree(MilImage);
   }

