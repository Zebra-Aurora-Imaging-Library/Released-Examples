//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: PolarizationImaging.cpp
// 
// Synopsis:  This program demonstrates how to use MIL to process  images
//            captured from a polarization sensor.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#include <vector>

#define IMAGE_DIR M_IMAGE_PATH MIL_TEXT("PolarizationImaging")

const MIL_INT NB_TILES_TO_DISP = 4;  
const MIL_INT NB_EXAMPLES = 4; 
const MIL_CONST_TEXT_PTR ImageNameArray[NB_EXAMPLES] = {MIL_TEXT("CarbonFiber.mim"),
                                                        MIL_TEXT("CellphoneCase.mim"),
                                                        MIL_TEXT("PlasticPiece.mim"),
                                                        MIL_TEXT("BlackCone.mim")};

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("PolarizationImaging\n\n")
             
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to process images captured\n")
             MIL_TEXT("from a polarization sensor to enhance features, detect a\n")
             MIL_TEXT("material's internal stress, and detect defects.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display, image processing, system.\n"));
      }

// Functions to process the polarized images.
void ExtractPolarizedChannels    (  MIL_ID               MilSysId,
                                    MIL_ID               OriginalImgId,
                                    std::vector<MIL_ID>  PolarizedImgVect);
                                        
void ComputeStokesParams         (  MIL_ID               MilSysId, 
                                    std::vector<MIL_ID>  pPolarizedImgVect,
                                    std::vector<MIL_ID>  pStokesImgVect);

void ComputeDescriptors          (  MIL_ID               MilSysId, 
                                    MIL_ID               OriginalImgId,
                                    std::vector<MIL_ID>  StokesImgVect,
                                    std::vector<MIL_ID>  DescriptorImgVect);

// Functions to display original and processed images.
void ShowOriginalImage           (  MIL_ID                MilSystemId, 
                                    MIL_ID                MilDisplayId, 
                                    MIL_ID                OriginalImgId, 
                                    MIL_ID                DisplayImgId);

void ShowPolarizedChannels       (  MIL_ID                MilSystemId, 
                                    MIL_ID                MilDisplayId, 
                                    std::vector<MIL_ID>   PolarizedImgVect, 
                                    MIL_ID                DisplayImgId,
                                    MIL_ID                MilGraphicListId);

void ShowStokesParameters        (  MIL_ID                MilSystemId, 
                                    MIL_ID                MilDisplayId, 
                                    std::vector<MIL_ID>   StokesImgVect, 
                                    MIL_ID                DisplayImgId,
                                    MIL_ID                MilGraphicListId);

void ShowDescriptors             (  MIL_ID                MilSystemId,
                                    MIL_ID                MilDisplayId,
                                    std::vector<MIL_ID>   DescriptorImgVect,
                                    MIL_ID                DisplayImgId,
                                    MIL_ID                MilGraphicListId);

// Function to do the pseudo color mapping on the direction of the polarization sensor.
void PseudoColorAoLP(MIL_ID  MilSysId, MIL_ID SourceImgId, MIL_ID DisplayImgId);

//////////////////////////////////////////////////////////////////////////////////////////
int MosMain()
   {
   PrintHeader();

   // Allocate MIL objects.
   MIL_ID MilApplicationId = MappAlloc (M_DEFAULT, M_NULL);
   MIL_ID MilSystemId      = MsysAlloc (M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplayId     = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispZoom(MilDisplayId, 0.5, 0.5);

      /* Allocate a graphic list to hold the sub pixel annotations to draw. */
   MIL_ID MilGraphicList   = MgraAllocList(MilSystemId, M_DEFAULT, M_NULL);
   /* Associate the graphic list to the display for annotations. */
   MdispControl(MilDisplayId, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   std::vector<MIL_ID> PolarizedImgVect(NB_TILES_TO_DISP, M_NULL);
   std::vector<MIL_ID> StokesImgVect(NB_TILES_TO_DISP, M_NULL);
   std::vector<MIL_ID> DescriptorImgVect(NB_TILES_TO_DISP, M_NULL);
   MIL_ID OriginalImgId, DisplayImgId;

   MIL_INT ImgSizeX, ImgSizeY, i;
   MIL_TEXT_CHAR ImgName[256];
   
   MIL_CONST_TEXT_PTR DescPtr[4] = {MIL_TEXT("In this example, the angle of linear polarization is used to distinguish\nstructures made with a carbon fiber material.         \n"),
                                    MIL_TEXT("In this example, polarization imaging results emphasize the presence of\ninternal stress in parts made with a transparent material.\n"),
                                    MIL_TEXT("In this example, the degree of linear polarization is used to detect defects\nin parts made with a dark plastic material.          \n"),
                                    MIL_TEXT("In this example, polarization imaging is used to increase the contrast\nbetween parts made with a dark plastic material.           \n")};

   // Iterate through each example.
   for (MIL_INT ExampleIdx = 0; ExampleIdx < NB_EXAMPLES; ExampleIdx++)
      {
      MosPrintf(MIL_TEXT("\n-----------------------------------------------------------------------\n"));
      MosPrintf(DescPtr[ExampleIdx]);
      
      // Restore source image.
      MosSprintf(ImgName, 256, MIL_TEXT("%s/%s"), IMAGE_DIR, ImageNameArray[ExampleIdx]);
      MbufRestore(ImgName, MilSystemId, &OriginalImgId);

      // Inquire source image size
      ImgSizeX = MbufInquire(OriginalImgId, M_SIZE_X, M_NULL);
      ImgSizeY = MbufInquire(OriginalImgId, M_SIZE_Y, M_NULL);

      // Allocate a display image that will be divided into 2x2 tiles.
      MbufAllocColor(MilSystemId, 3, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &DisplayImgId);

      // Allocate 4 polarized images.
      for (i = 0; i < NB_TILES_TO_DISP; ++i)
         MbufAlloc2d(MilSystemId, ImgSizeX / 2, ImgSizeY / 2, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &PolarizedImgVect[i]);
      
      // Allocate 3 Stokes images.
      for (i = 0; i < NB_TILES_TO_DISP -1; ++i)
         MbufAlloc2d(MilSystemId, ImgSizeX / 2, ImgSizeY / 2, 32 + M_FLOAT, M_IMAGE + M_PROC, &StokesImgVect[i]);

      // Allocate 3 descriptor images.
      for (i = 0; i < NB_TILES_TO_DISP -1; ++i)
         MbufAlloc2d(MilSystemId, ImgSizeX / 2, ImgSizeY / 2, 32 + M_FLOAT, M_IMAGE + M_PROC, &DescriptorImgVect[i]);
      
      // Display source image.
      ShowOriginalImage(MilSystemId, MilDisplayId, OriginalImgId, DisplayImgId);

      // Extract and display 2x2 polarized channels.
      ExtractPolarizedChannels(MilSystemId, OriginalImgId, PolarizedImgVect);
      ShowPolarizedChannels(MilSystemId, MilDisplayId, PolarizedImgVect, DisplayImgId, MilGraphicList);

      // Compute and display 4 Stokes images.
      ComputeStokesParams(MilSystemId, PolarizedImgVect, StokesImgVect);
      ShowStokesParameters(MilSystemId, MilDisplayId, StokesImgVect, DisplayImgId, MilGraphicList);

      // Compute and display the minimum intensities, the degree of and the angle of the polarization.
      ComputeDescriptors(MilSystemId, OriginalImgId, StokesImgVect, DescriptorImgVect);
      ShowDescriptors(MilSystemId, MilDisplayId, DescriptorImgVect, DisplayImgId, MilGraphicList);

      // Free allocations
      for (i = 0; i < NB_TILES_TO_DISP -1; ++i)
         {
         MbufFree(DescriptorImgVect[i]);
         MbufFree(StokesImgVect[i]);
         }
      
      for (i = 0; i < NB_TILES_TO_DISP; ++i)
         {
         MbufFree(PolarizedImgVect[i]);
         }

      MbufFree(DisplayImgId);
      MbufFree(OriginalImgId);
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   MgraFree(MilGraphicList);
   MdispFree(MilDisplayId);
   MsysFree (MilSystemId);
   MappFree (MilApplicationId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Extract 4 polarized images.
void ExtractPolarizedChannels(MIL_ID MilSysId, MIL_ID OriginalImgId, std::vector<MIL_ID> PolarizedImgVect)
   {
   MIL_INT ImgSizeX = MbufInquire(OriginalImgId, M_SIZE_X, M_NULL);
   MIL_INT ImgSizeY = MbufInquire(OriginalImgId, M_SIZE_Y, M_NULL);

   // Generate the warp matrix for a scaling operation by a factor of 0.5.
   MIL_ID WarpMatrixId = MbufAllocColor(M_DEFAULT_HOST, 1, 3, 3, 32 + M_FLOAT, M_ARRAY, M_NULL);
   MgenWarpParameter(M_NULL, WarpMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_SCALE, 0.5, 0.5);

   // Extract the top-left pixels of every 2x2 group.
   MIL_ID SrcChildId = MbufChild2d(OriginalImgId, 0, 0, ImgSizeX, ImgSizeY, M_NULL);
   MimWarp(SrcChildId, PolarizedImgVect[0], WarpMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_NEAREST_NEIGHBOR);

   // Extract the top-right pixels of every 2x2 group.
   MbufChildMove(SrcChildId, 1, 0, ImgSizeX - 1, ImgSizeY, M_DEFAULT);
   MimWarp(SrcChildId, PolarizedImgVect[1], WarpMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_NEAREST_NEIGHBOR);

   // Extract the bottom-left pixels of every 2x2 group.
   MbufChildMove(SrcChildId, 0, 1, ImgSizeX, ImgSizeY - 1, M_DEFAULT);
   MimWarp(SrcChildId, PolarizedImgVect[2], WarpMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_NEAREST_NEIGHBOR);

   // Extract the bottom-right pixels of every 2x2 group.
   MbufChildMove(SrcChildId, 1, 1, ImgSizeX - 1, ImgSizeY - 1, M_DEFAULT);
   MimWarp(SrcChildId, PolarizedImgVect[3], WarpMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_NEAREST_NEIGHBOR);

   MbufFree(WarpMatrixId);
   MbufFree(SrcChildId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Computer 3 stock parameters.
void ComputeStokesParams(MIL_ID MilSysId, std::vector<MIL_ID> PolarizedImgVect, std::vector<MIL_ID> StokesImgVect)
   {
   MIL_INT TileSizeX = MbufInquire((PolarizedImgVect)[0], M_SIZE_X, M_NULL);
   MIL_INT TileSizeY = MbufInquire((PolarizedImgVect)[0], M_SIZE_Y, M_NULL);

   // S0 = I0 +I90 = LayerVect[3] + LayerVect[0];
   MimArith(PolarizedImgVect[3], PolarizedImgVect[0], StokesImgVect[0], M_ADD);

   // S1 = I0 -I90 = LayerVect[3] - LayerVect[0];
   MimArith(PolarizedImgVect[3], PolarizedImgVect[0], StokesImgVect[1], M_SUB);
   
   // S2 = I45 -I135 = LayerVect[1] - LayerVect[2];
   MimArith(PolarizedImgVect[1], PolarizedImgVect[2], StokesImgVect[2], M_SUB);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Compute minimum intensities of 4 channels, degree and angle of polarization.
void ComputeDescriptors(MIL_ID MilSysId, MIL_ID OriginalImgId, std::vector<MIL_ID> StokesImgVect, std::vector<MIL_ID> DescriptorImgVect)
   {
   MIL_INT TileSizeX = MbufInquire(StokesImgVect[0], M_SIZE_X, M_NULL);
   MIL_INT TileSizeY = MbufInquire(StokesImgVect[0], M_SIZE_Y, M_NULL);

   // DescriptorImgVect[0]: the minimum of the every 2x2 polarized pixels. 
   MimResize(OriginalImgId, DescriptorImgVect[0], 0.5, 0.5, M_MIN); 

   // DescriptorImgVect[1]: DOLP = SQRT(S1*S1 +S2*S2)/S0 
   // DescriptorImgVect[2]: AOLP = 0.5*atan(S2/S1)
   MimTransform(StokesImgVect[1], StokesImgVect[2], DescriptorImgVect[1], DescriptorImgVect[2], M_POLAR, M_FORWARD);
   MimArith(DescriptorImgVect[1], StokesImgVect[0], DescriptorImgVect[1], M_DIV);
   MimArith(DescriptorImgVect[2], 0.5, DescriptorImgVect[2], M_MULT_CONST);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Display original image.
void ShowOriginalImage(MIL_ID MilSystemId, MIL_ID MilDisplayId, MIL_ID OriginalImgId, MIL_ID DisplayImgId)
   {
   MosPrintf(MIL_TEXT("\n(1) Capture a polarized image using a polarization sensor.\n"));

   MdispControl(MilDisplayId, M_TITLE, MIL_TEXT("Original Image"));
   MbufCopy(OriginalImgId, DisplayImgId);
   MdispSelect(MilDisplayId, DisplayImgId);

   MosPrintf(MIL_TEXT("Press any key to continue...\n"));
   MosGetch();
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Display 4 polarized images.
void ShowPolarizedChannels(MIL_ID MilSystemId, MIL_ID MilDisplayId, std::vector<MIL_ID> PolarizedImgVect, MIL_ID DisplayImgId, MIL_ID MilGraphicListId)
   {
   MosPrintf(MIL_TEXT("\n(2) De-mosaic the 4 polarization orientations.\n"));
   
   // Inquire the 2x2 tile image size.
   MIL_INT TileSizeX = MbufInquire(DisplayImgId, M_SIZE_X, M_NULL) / 2;
   MIL_INT TileSizeY = MbufInquire(DisplayImgId, M_SIZE_Y, M_NULL) / 2;
     
   // Copy 4 polarized images into 4 display tiles.
   MbufCopyClip(PolarizedImgVect[0], DisplayImgId, 0, 0);
   MbufCopyClip(PolarizedImgVect[1], DisplayImgId, TileSizeX, 0);
   MbufCopyClip(PolarizedImgVect[2], DisplayImgId, 0, TileSizeY);
   MbufCopyClip(PolarizedImgVect[3], DisplayImgId, TileSizeX, TileSizeY);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   
   MdispControl(MilDisplayId, M_TITLE, MIL_TEXT("Polarization channels."));
   MgraText(M_DEFAULT, MilGraphicListId, 0, 0, MIL_TEXT("I90 - Pixels polarized at 90 degrees"));
   MgraText(M_DEFAULT, MilGraphicListId, TileSizeX, 0, MIL_TEXT("I45 - Pixels polarized at 45 degrees"));
   MgraText(M_DEFAULT, MilGraphicListId, 0, TileSizeY, MIL_TEXT("I135 - Pixels polarized at 135 degrees"));
   MgraText(M_DEFAULT, MilGraphicListId, TileSizeX, TileSizeY, MIL_TEXT("I0 - Pixels polarized at 0 degree"));

   MosPrintf(MIL_TEXT("Press any key to continue...\n"));
   MosGetch();
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Display 3 Stokes parameters.
void ShowStokesParameters(MIL_ID MilSystemId, MIL_ID MilDisplayId, std::vector<MIL_ID> StokesImgVect, MIL_ID DisplayImgId, MIL_ID MilGraphicListId)
{
   MosPrintf(MIL_TEXT("\n(3) Compute the Stokes polarization state parameters.\n"));

   // Inquire the 2x2 tile image size.
   MIL_INT TileSizeX = MbufInquire(DisplayImgId, M_SIZE_X, M_NULL) / 2;
   MIL_INT TileSizeY = MbufInquire(DisplayImgId, M_SIZE_Y, M_NULL) / 2;

   MbufClear(DisplayImgId, 0L);
   MgraClear(M_DEFAULT, MilGraphicListId);

   // Copy 4 polarized images into 4 display tiles.
   MIL_ID DstChildId = MbufChild2d(DisplayImgId, 0, 0, TileSizeX, TileSizeY, M_NULL);
   MbufControl(StokesImgVect[0], M_MIN, 0);
   MbufControl(StokesImgVect[0], M_MAX, 510);
   MimRemap(M_DEFAULT, StokesImgVect[0], DstChildId, M_FIT_SRC_RANGE);

   MbufChildMove(DstChildId, TileSizeX, 0, TileSizeX, TileSizeY, M_NULL);
   MbufControl(StokesImgVect[1], M_MIN, -255);
   MbufControl(StokesImgVect[1], M_MAX, 255);
   MimRemap(M_DEFAULT, StokesImgVect[1], DstChildId, M_FIT_SRC_RANGE);

   MbufChildMove(DstChildId, 0, TileSizeY, TileSizeX, TileSizeY, M_NULL);
   MbufControl(StokesImgVect[2], M_MIN, -255);
   MbufControl(StokesImgVect[2], M_MAX, 255);
   MimRemap(M_DEFAULT, StokesImgVect[2], DstChildId, M_FIT_SRC_RANGE);

   MbufFree(DstChildId);

   MdispControl(MilDisplayId, M_TITLE, MIL_TEXT("Stokes parameters"));
   // S0 : adding the intensities of the vertically and horizontally polarized pixels.
   MgraText(M_DEFAULT,MilGraphicListId, 0, 0, MIL_TEXT("S0 = I0 + I90"));
   // S1: the difference between the horizontal and vertical components.
   MgraText(M_DEFAULT, MilGraphicListId, TileSizeX, 0, MIL_TEXT("S1 = I0 - I90"));
   // S2: the 45° component. Positive values are 45° linearly polarized.Negative values are - 45° linearly polarized.
   MgraText(M_DEFAULT, MilGraphicListId, 0, TileSizeY, MIL_TEXT("S2 = I45 - I135"));

   MosPrintf(MIL_TEXT("Press any key to continue...\n"));
   MosGetch();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Display minimum intensities of 4 channels, degree and angle of polarization.
void ShowDescriptors(MIL_ID MilSystemId, MIL_ID MilDisplayId, std::vector<MIL_ID> DescriptorImgVect, MIL_ID DisplayImgId, MIL_ID MilGraphicListId)
  {
   MosPrintf(MIL_TEXT("\n(4) Compute results for the derived minimum intensity, degree of Linear\n")
             MIL_TEXT("Polarization, and angle of Linear Polarization.\n"));

   // Inquire the 2x2 tile image size.
   MIL_INT TileSizeX = MbufInquire(DisplayImgId, M_SIZE_X, M_NULL) / 2;
   MIL_INT TileSizeY = MbufInquire(DisplayImgId, M_SIZE_Y, M_NULL) / 2;

   MIL_ID DstChildId = MbufChild2d(DisplayImgId, 0, 0, TileSizeX, TileSizeY, M_NULL);

   MdispControl(MilDisplayId, M_TITLE, MIL_TEXT("Result Images"));
   MgraClear(M_DEFAULT, MilGraphicListId);

   // Top left: S0-adding the intensities of the vertically and horizontally polarized pixels.
   // (No update on the display.)
   MgraText(M_DEFAULT, MilGraphicListId, 0, 0, MIL_TEXT("Intensity."));

   // Top right: display the minimum of every 2x2 polarized pixels. 
   MbufCopyClip(DescriptorImgVect[0], DisplayImgId, TileSizeX, 0);

   // BottomLeft: degree of polarization DOLP = SQRT(S1*S1 +S2*S2)/S0 
   MbufChildMove(DstChildId, 0, TileSizeY, TileSizeX, TileSizeY, M_NULL);
   MbufControl(DescriptorImgVect[1], M_MIN, 0.0);
   MbufControl(DescriptorImgVect[1], M_MAX, 1.0);
   MimRemap(M_DEFAULT, DescriptorImgVect[1], DstChildId, M_FIT_SRC_RANGE);

   // BottomRight: angle of polarization AoLP = 0.5*(S2/S1)
   MbufChildMove(DstChildId, TileSizeX, TileSizeY, TileSizeX, TileSizeY, M_NULL);
   MbufControl(DescriptorImgVect[2], M_MIN, 0.0);
   MbufControl(DescriptorImgVect[2], M_MAX, 180.0);
   // Pseudo color mapping on AoLP for display.
   PseudoColorAoLP(MilSystemId, DescriptorImgVect[2], DstChildId);

   MgraText(M_DEFAULT, MilGraphicListId, TileSizeX, 0, MIL_TEXT("Minimum Intensity."));
   MgraText(M_DEFAULT, MilGraphicListId, 0, TileSizeY, MIL_TEXT("Degree of Linear Polarization."));
   MgraText(M_DEFAULT, MilGraphicListId, TileSizeX, TileSizeY, MIL_TEXT("Angle of Linear Polarization."));

   MbufFree(DstChildId);

   MosPrintf(MIL_TEXT("Press any key to continue...\n"));
   MosGetch();
   MgraClear(M_DEFAULT, MilGraphicListId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Pseudo color mapping on the angle of polarization for display.
void PseudoColorAoLP(MIL_ID  MilSysId, MIL_ID SourceImgId, MIL_ID DisplayImgId)
   {
   MIL_INT TileSizeX = MbufInquire(DisplayImgId, M_SIZE_X, M_NULL);
   MIL_INT TileSizeY = MbufInquire(DisplayImgId, M_SIZE_Y, M_NULL);

   // Remap the source image to the proper range.
   MIL_ID MonoSrcRemapped = MbufAlloc2d(MilSysId, TileSizeX, TileSizeY, 8+M_UNSIGNED, M_IMAGE + M_PROC, M_NULL);
   MimRemap(M_DEFAULT, SourceImgId, MonoSrcRemapped, M_FIT_SRC_RANGE);

   // LUT through which to map source input values.
   MIL_ID MilLut;                
   // Allocate a color LUT buffer for color mapping.
   MbufAllocColor(MilSysId, 3, 256, 1, 8 + M_UNSIGNED, M_LUT, &MilLut);

   // Fill the LUT buffer with a HUE color-map.
   MgenLutFunction(MilLut, M_COLORMAP_HUE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MimLutMap(MonoSrcRemapped, DisplayImgId, MilLut);

   MbufFree(MilLut);
   MbufFree(MonoSrcRemapped);
   }  
