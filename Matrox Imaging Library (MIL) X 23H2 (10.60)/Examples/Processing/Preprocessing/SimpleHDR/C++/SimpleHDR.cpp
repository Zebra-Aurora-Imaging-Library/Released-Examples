﻿//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: SimpleHDR.cpp
// 
// Synopsis:  This example shows how to perform a simple HDR operation.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////
#include <mil.h>
#include <math.h>

using namespace std;

#define SHORT_EXPOSURE_IMAGE M_IMAGE_PATH MIL_TEXT("SimpleHDR/ShortExposure.tif")
#define LONG_EXPOSURE_IMAGE  M_IMAGE_PATH MIL_TEXT("SimpleHDR/LongExposure.tif")

// Util constants.
static const MIL_DOUBLE ToneMappingCoefficient = 0.4;
static const MIL_DOUBLE ToneMappingLowThreshold  = 0.4;
static const MIL_DOUBLE ToneMappingHighThreshold = 99.0;
static const MIL_DOUBLE FusionCoverage = 0.1;
static const MIL_DOUBLE FusionLowThreshold = 0.5;
static const MIL_DOUBLE FusionHighThreshold = 99.4;

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("SimpleHDR\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to use the registration module to combine two images\n")
             MIL_TEXT("taken with different exposures to obtain a High Dynamic Range (HDR) result\nimage.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display,       \n")
             MIL_TEXT("image processing, system.                      \n\n"));
   }

// Main.
int MosMain(void)
{
   MIL_ID MilApplication,       // Application identifier.
          MilSystem,            // System identifier.
          ShortExposureBuf,     // Image identifier.
          LongExposureBuf,      // Image identifier.
          HdrBuf,               // Image identifier.
          MilDispShort,         // Display identifier.
          MilDispLong,          // Display identifier.
          MilDispHdr;           // Display identifier.

   PrintHeader();

   // Allocate application, system and display.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   // Restore source buffers.
   MbufRestore(LONG_EXPOSURE_IMAGE, MilSystem, &LongExposureBuf);
   MbufRestore(SHORT_EXPOSURE_IMAGE, MilSystem, &ShortExposureBuf);

   std::vector<MIL_ID> ImagesArray;
   ImagesArray.reserve(2);
   ImagesArray.push_back(LongExposureBuf);
   ImagesArray.push_back(ShortExposureBuf);

   // Get the size and type of the images.
   MIL_INT ImageSizeX = MbufInquire(ImagesArray[0], M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(ImagesArray[0], M_SIZE_Y, M_NULL);
   MIL_INT Type = MbufInquire(ImagesArray[0], M_TYPE, M_NULL);

   // Allocate Hdr buffer.
   MbufAlloc2d(MilSystem, ImageSizeX, ImageSizeY, Type, M_IMAGE + M_PROC + M_DISP, &HdrBuf);

   // Allocate displays.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDispShort);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDispLong);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDispHdr);

   MdispControl(MilDispLong, M_WINDOW_INITIAL_POSITION_X, ImageSizeX);
   MdispControl(MilDispHdr,  M_WINDOW_INITIAL_POSITION_X, ImageSizeX * 2);
   
   MdispControl(MilDispShort, M_TITLE, MIL_TEXT("Short exposure"));
   MdispControl(MilDispLong,  M_TITLE, MIL_TEXT("Long exposure"));
   MdispControl(MilDispHdr,   M_TITLE, MIL_TEXT("HDR result"));
   MdispControl(MilDispHdr,   M_VIEW_MODE, M_AUTO_SCALE);
   MdispControl(MilDispShort, M_VIEW_MODE, M_AUTO_SCALE);
   MdispControl(MilDispLong,  M_VIEW_MODE, M_AUTO_SCALE);

   MdispSelect(MilDispShort, ShortExposureBuf);
   MdispSelect(MilDispLong,  LongExposureBuf);

   /////////////////////////////////////////////////////////////////////////////////////////////
   // Perform HDR.
   /////////////////////////////////////////////////////////////////////////////////////////////

   // Allocate a registration context for a high dynamic range registration operation.
   MIL_ID RegContext = MregAlloc(MilSystem, M_HIGH_DYNAMIC_RANGE, M_DEFAULT, M_NULL);

   // Set control values of M_HIGH_DYNAMIC_RANGE registration context.
   MregControl(RegContext, M_DEFAULT, M_TONE_MAPPING_MODE, M_PERCENTILE_VALUE + M_IN_RANGE);
   MregControl(RegContext, M_DEFAULT, M_TONE_MAPPING_COEFFICIENT, ToneMappingCoefficient);
   MregControl(RegContext, M_DEFAULT, M_TONE_MAPPING_LOW_THRESHOLD, ToneMappingLowThreshold);
   MregControl(RegContext, M_DEFAULT, M_TONE_MAPPING_HIGH_THRESHOLD, ToneMappingHighThreshold);

   MregControl(RegContext, M_DEFAULT, M_FUSION_COVERAGE, FusionCoverage);
   MregControl(RegContext, M_DEFAULT, M_FUSION_LOW_THRESHOLD, FusionLowThreshold);
   MregControl(RegContext, M_DEFAULT, M_FUSION_HIGH_THRESHOLD, FusionHighThreshold);
   MregControl(RegContext, M_DEFAULT, M_FUSION_MODE, M_PERCENTILE_VALUE + M_IN_RANGE);

   // Perform the high dynamic range registration operation on the input images.
   MregCalculate(RegContext, ImagesArray, HdrBuf, ImagesArray.size(), M_COMPUTE);

   // Display the HDR result. 
   MdispSelect(MilDispHdr, HdrBuf);

   MosPrintf(MIL_TEXT("The short and the long exposure images are displayed along\n")
             MIL_TEXT("with the resulting HDR image.\n")
             MIL_TEXT("The short exposure image contains details in bright areas. \n")
             MIL_TEXT("The long exposure image contains details in dark areas.\n\n"));
  
   // Wait for a key press.
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();
   
   // Free allocations.
   MregFree(RegContext);
   
   MbufFree(ShortExposureBuf);
   MbufFree(LongExposureBuf);
   MbufFree(HdrBuf);
   
   MdispFree(MilDispHdr);
   MdispFree(MilDispShort);
   MdispFree(MilDispLong);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
}
