﻿/***************************************************************************************/
/* 
* File name: Morphology.cpp  
*
* Synopsis:  This program contains examples of morphological operations
*            used in different situations.
*            See the PrintHeader() function below for detailed description.
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
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Morphology\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program performs various grayscale\n")
             MIL_TEXT("and binary morphological operations.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//**********************************
// TOP-HAT FILTERING declarations
//**********************************

static MIL_CONST_TEXT_PTR TopHatFilteringFilename = M_IMAGE_PATH MIL_TEXT("/Preprocessing/Dust.tif");

void TopHatFiltering(MIL_CONST_TEXT_PTR SrcFilename, 
                     MIL_ID MilSystem, 
                     MIL_ID MilDisplay);


//********************************
// OBJECT SKELETON declarations
//********************************

static MIL_CONST_TEXT_PTR ObjectSkeletonFilename = M_IMAGE_PATH MIL_TEXT("/Preprocessing/Circuit.tif");

void ObjectSkeleton(MIL_CONST_TEXT_PTR SrcFilename, 
                    MIL_ID MilSystem, 
                    MIL_ID MilDisplay);

//********************************
// OBJECT CONNECTMAP declarations
//********************************

static MIL_CONST_TEXT_PTR ObjectConnectMapFilename = M_IMAGE_PATH MIL_TEXT("/Preprocessing/CircuitPins.tif");

void ObjectConnectMap(MIL_CONST_TEXT_PTR SrcFilename, 
                      MIL_ID MilSystem, 
                      MIL_ID MilDisplay);

//************************************
// OBJECT SEGMENTATION declarations
//************************************

static MIL_CONST_TEXT_PTR ObjectSegmentationFilename = M_IMAGE_PATH MIL_TEXT("/Preprocessing/Connector.tif");

void ObjectSegmentation(MIL_CONST_TEXT_PTR SrcFilename, 
                        MIL_ID MilSystem, 
                        MIL_ID MilDisplay);

//*********************************************
// MORPHOLOGICAL RECONSTRUCTION declarations
//*********************************************

static MIL_CONST_TEXT_PTR MorphologicalReconstructionFilename = M_IMAGE_PATH MIL_TEXT("/Preprocessing/Retina.tif");

void MorphologicalReconstruction(MIL_CONST_TEXT_PTR SrcFilename, 
                                 MIL_ID MilSystem, 
                                 MIL_ID MilDisplay);

void MorphoReconstruction(MIL_ID  MilSystem,
                          MIL_ID  SrcImage, 
                          MIL_ID  Seed, 
                          MIL_ID  DstImage, 
                          MIL_INT MaxIter);

//**********************************
// OBJECT PERIMETER declarations
//**********************************

static MIL_CONST_TEXT_PTR ObjectPerimeterFilename = M_IMAGE_PATH MIL_TEXT("/Cell.mbufi");

void ObjectPerimeter(MIL_CONST_TEXT_PTR SrcFilename,
                     MIL_ID MilSystem,
                     MIL_ID MilDisplay);

//**************************
// Utility sub-functions.
//**************************
void AllocDisplayImage(MIL_ID MilSystem, 
                       MIL_ID SrcImage, 
                       MIL_ID MilDisplay, 
                       MIL_ID& DispProcImage, 
                       MIL_ID& MilOverlay);

void AllocGenPseudoColorLUT(MIL_ID  MilSystem, 
                            MIL_ID  MilDisplay,
                            MIL_INT StartIndex, 
                            MIL_INT EndIndex, 
                            MIL_ID& PseudoColorLut);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL); // Application identifier.
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL); // System identifier.
   MIL_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL); // Display identifier.

   // Print Header.
   PrintHeader();

   //*********************
   // TOP-HAT FILTERING
   //*********************
   TopHatFiltering(TopHatFilteringFilename, MilSystem, MilDisplay);

   /*******************/
   /* OBJECT SKELETON */
   /*******************/
   ObjectSkeleton(ObjectSkeletonFilename, MilSystem, MilDisplay);

   /**********************/
   /* OBJECT CONNECT MAP */
   /**********************/
   ObjectConnectMap(ObjectConnectMapFilename, MilSystem, MilDisplay);

   //***********************
   // OBJECT SEGMENTATION
   //***********************
   ObjectSegmentation(ObjectSegmentationFilename, MilSystem, MilDisplay);

   //***********************
   // OBJECT PERIMETER
   //***********************
   ObjectPerimeter(ObjectPerimeterFilename, MilSystem, MilDisplay);

   //*****************************
   // MORPHOLOGICAL RECONSTRUCTION 
   //*****************************
   MorphologicalReconstruction(MorphologicalReconstructionFilename, MilSystem, MilDisplay);

   // Free allocated objects.
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//************************
// TOP-HAT FILTERING
//************************
void TopHatFiltering(MIL_CONST_TEXT_PTR SrcFilename, 
                     MIL_ID MilSystem, 
                     MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[TOP_HAT FILTERING]\n\n")

             MIL_TEXT("In this example a top-hat filtering operation with\n")
             MIL_TEXT("a dedicated structuring element is used to enhance\n")
             MIL_TEXT("defects in a scene with non-uniform illumination.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a vertical structuring element to minimize geometric aberration.
   MIL_ID MilStructElement = MbufAlloc2d(MilSystem, 1, 8, 32, M_STRUCT_ELEMENT, M_NULL);
   MbufClear(MilStructElement, 0);

   // Apply the top-hat filetring.
   MimMorphic(MilSrcImage, MilDispProcImage, MilStructElement, M_TOP_HAT, 1, M_GRAYSCALE);

   MosPrintf(MIL_TEXT("The result of the top-hat filtering is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Segment the image.
   MimBinarize(MilDispProcImage, MilDispProcImage, M_PERCENTILE_VALUE+M_GREATER, 95, M_NULL);

   MosPrintf(MIL_TEXT("The 5%% brightest pixels are thresholded and displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Remove small binary noise.
   MimOpen(MilDispProcImage, MilDispProcImage, 1, M_BINARY);

   MosPrintf(MIL_TEXT("A morphological opening is applied to remove small noise.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects
   MbufFree(MilSrcImage);
   MbufFree(MilStructElement);
   MbufFree(MilDispProcImage);
   }

//*******************
// OBJECT SKELETON
//*******************
void ObjectSkeleton(MIL_CONST_TEXT_PTR SrcFilename, 
                    MIL_ID MilSystem, 
                    MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[OBJECT SKELETON]\n\n")

             MIL_TEXT("In this example a binary thinning operation is used to extract\n")
             MIL_TEXT("the paths in a network. The result of a distance transformation\n")
             MIL_TEXT("is combined with the path in order to determine the thickness of\n")
             MIL_TEXT("the network.\n")
             MIL_TEXT("A pseudo color display is used to enhance the final result.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   // Allocate the intermediate destination buffers.
   MIL_ID MilDistanceImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilSkeletonImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);

   // Segment the source image.
   MimBinarize(MilSrcImage, MilSkeletonImage, M_FIXED+M_LESS, 25, M_NULL);
   
   // Close small holes.
   MimClose(MilSkeletonImage, MilSkeletonImage, 1, M_BINARY);

   // Display the segmentation result.
   MbufCopy(MilSkeletonImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("The source image is thresholded and displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Compute the distance transform of the object.
   MimDistance(MilSkeletonImage, MilDistanceImage, M_CHAMFER_3_4); 

   // Perform the binary thinning to get the object skeleton.
   MimThin(MilSkeletonImage, MilSkeletonImage, M_TO_SKELETON, M_BINARY3); 

   // Display the thinning result.
   MbufCopy(MilSkeletonImage, MilOverlayImage);
   MosPrintf(MIL_TEXT("A binary thinning is applied and the result skeleton is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Combine the skeleton with the distance image.
   MimArith(MilSkeletonImage, MilDistanceImage, MilDistanceImage, M_AND);
   
   // Allocate and generate the pseudo color LUT */
   MIL_ID MilStatResult = MimAllocResult(MilSystem, M_DEFAULT, M_STATISTICS_RESULT, M_NULL);
   MimStatCalculate(M_STAT_CONTEXT_MAX, MilDistanceImage, MilStatResult, M_DEFAULT);

   MIL_INT MaxValue;
   MimGetResult(MilStatResult, M_STAT_MAX+M_TYPE_MIL_INT, &MaxValue);

   MIL_ID MilPseudoColorLut;
   AllocGenPseudoColorLUT(MilSystem, MilDisplay, 1, MaxValue-5, MilPseudoColorLut);
   
   // Display the thinning result in pseudo color.
   MimLutMap(MilDistanceImage, MilOverlayImage, MilPseudoColorLut);
   MosPrintf(MIL_TEXT("The thickness of the object is retrieved by combining the object's skeleton\n"));
   MosPrintf(MIL_TEXT("with a distance transform result of the object. The maximum distance value\n"));
   MosPrintf(MIL_TEXT("is %d pixels. A LUT mapping is used to display the skeleton in pseudo colors\n"), (int)MaxValue);
   MosPrintf(MIL_TEXT("based on the object's thickness: blue = thin sections to red = thick sections.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDistanceImage);
   MbufFree(MilSkeletonImage);
   MbufFree(MilPseudoColorLut);
   MbufFree(MilDispProcImage);
   MimFree(MilStatResult);
   }


//*******************
// OBJECT CONNECT MAP
//*******************
void ObjectConnectMap(MIL_CONST_TEXT_PTR SrcFilename, 
                      MIL_ID MilSystem, 
                      MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[OBJECT CONNECT MAP]\n\n")

             MIL_TEXT("In this example a binary thinning is combined with a connect map\n")
             MIL_TEXT("operation in order to identify the breaks in and the bridges\n")
             MIL_TEXT("between parallel circuit lines.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   // Allocate the destination buffer.
   MIL_ID MilProcImage     = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L + M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilBinaryImage   = MbufAlloc2d(MilSystem, SizeX, SizeY, 1L + M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilSkeletonImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 1L + M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);

   // Noise reduction
   MIL_ID MilLinearFilterIIRContext = MimAlloc(MilSystem, M_LINEAR_FILTER_IIR_CONTEXT, M_DEFAULT, M_NULL);
   MimControl(MilLinearFilterIIRContext, M_FILTER_SMOOTHNESS, 50);
   MimConvolve(MilSrcImage, MilProcImage, MilLinearFilterIIRContext);
   MimFree(MilLinearFilterIIRContext);

   // Evaluate the threshold value to segment the image
   const MIL_INT ChildOffsetX =  30; 
   const MIL_INT ChildOffsetY =  50;
   const MIL_INT ChildSizeX   =  80; 
   const MIL_INT ChildSizeY   = 300;

   MIL_ID MilProcChild = MbufChild2d(MilProcImage, ChildOffsetX, ChildOffsetY, ChildSizeX, ChildSizeY, M_NULL);

   MIL_INT ThresholdValue = MimBinarize(MilProcChild, M_NULL, M_BIMODAL+M_GREATER, M_NULL, M_NULL);

   // Segment the source image.
   MimBinarize(MilProcImage, MilBinaryImage, M_FIXED+M_GREATER, (MIL_DOUBLE)ThresholdValue, M_NULL);

   // Remove small blobs
   MimOpen(MilBinaryImage, MilBinaryImage, 1, M_BINARY);

   // Perform the binary thinning to get the object skeleton.
   MimThin(MilBinaryImage, MilSkeletonImage, M_TO_SKELETON, M_BINARY3); 

   // Display the segmentation result.
   MbufClearCond(MilOverlayImage, 255, 255, 255, MilSkeletonImage, M_NOT_EQUAL, 0);
   MosPrintf(MIL_TEXT("The skeleton of the segmented source image is displayed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate then clear the connectivity Lut.
   MIL_ID LutBufId = MbufAlloc1d(MilSystem, 512, 8L+M_UNSIGNED, M_LUT, M_NULL);
   MbufClear(LutBufId, 0);

   // Add connectivity codes for isolated points.
   const MIL_UINT8 IsolatedPointCode = 1;
   MbufPut1d(LutBufId, 256, 1, &IsolatedPointCode);

   // Add connectivity codes for end points.
   const MIL_UINT8 EndPointCode = 2;
   MbufPut1d(LutBufId, 257, 1, &EndPointCode);
   MbufPut1d(LutBufId, 258, 1, &EndPointCode);
   MbufPut1d(LutBufId, 260, 1, &EndPointCode);
   MbufPut1d(LutBufId, 264, 1, &EndPointCode);
   MbufPut1d(LutBufId, 272, 1, &EndPointCode);
   MbufPut1d(LutBufId, 288, 1, &EndPointCode);
   MbufPut1d(LutBufId, 320, 1, &EndPointCode);
   MbufPut1d(LutBufId, 384, 1, &EndPointCode);
   MbufPut1d(LutBufId, 259, 1, &EndPointCode);
   MbufPut1d(LutBufId, 262, 1, &EndPointCode);
   MbufPut1d(LutBufId, 268, 1, &EndPointCode);
   MbufPut1d(LutBufId, 280, 1, &EndPointCode);
   MbufPut1d(LutBufId, 304, 1, &EndPointCode);
   MbufPut1d(LutBufId, 352, 1, &EndPointCode);
   MbufPut1d(LutBufId, 448, 1, &EndPointCode);
   MbufPut1d(LutBufId, 385, 1, &EndPointCode);

   // Add connectivity codes for triple points.
   const MIL_UINT8 TriplePointCode = 3;
   MbufPut1d(LutBufId, 277, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 340, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 337, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 325, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 298, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 424, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 418, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 394, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 404, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 293, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 338, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 329, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 297, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 330, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 402, 1, &TriplePointCode);
   MbufPut1d(LutBufId, 420, 1, &TriplePointCode);

   // Add connectivity codes for cross points.
   const MIL_UINT8 CrossPointCode = 4;
   MbufPut1d(LutBufId, 341, 1, &CrossPointCode);
   MbufPut1d(LutBufId, 426, 1, &CrossPointCode);
      
   // Determine the map of connections.
   MimConnectMap(MilSkeletonImage, MilProcImage, LutBufId);

   // Display the segmentation result.
   MimDilate(MilProcImage, MilProcImage, 2, M_GRAYSCALE); // Thicken the map to improve the visualization
   MbufClearCond(MilOverlayImage,   0,   0, 255, MilProcImage, M_EQUAL, IsolatedPointCode);
   MbufClearCond(MilOverlayImage, 255,   0, 255, MilProcImage, M_EQUAL, EndPointCode     );
   MbufClearCond(MilOverlayImage, 255,   0,   0, MilProcImage, M_EQUAL, TriplePointCode  );
   MbufClearCond(MilOverlayImage, 255, 255,   0, MilProcImage, M_EQUAL, CrossPointCode   );
   
   MosPrintf(MIL_TEXT("The result of the connectivity analysis is displayed:\n"));
   MosPrintf(MIL_TEXT("   - blue   : isolated points\n"));
   MosPrintf(MIL_TEXT("   - red    : triple points\n"));
   MosPrintf(MIL_TEXT("   - yellow : cross points\n"));
   MosPrintf(MIL_TEXT("   - magenta: end points\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilProcChild);
   MbufFree(MilProcImage);
   MbufFree(MilBinaryImage);
   MbufFree(MilSkeletonImage);
   MbufFree(MilDispProcImage);   
   MbufFree(LutBufId);
   }

//***********************
// OBJECT SEGMENTATION
//***********************
void ObjectSegmentation(MIL_CONST_TEXT_PTR SrcFilename, 
                        MIL_ID MilSystem, 
                        MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[OBJECT SEGMENTATION]\n\n")

             MIL_TEXT("In this example, combinations of binary morphological operations\n")
             MIL_TEXT("are used to segment the object into its principal components.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate destination buffers.
   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);
   MIL_ID MilDstImage1 = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilDstImage2 = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);

   // Binarize the source image.
   MimBinarize(MilSrcImage, MilSrcImage, M_BIMODAL+M_GREATER, M_NULL, M_NULL);

   // Segment the horizontal components using a horizontal structuring element.
   MIL_ID MilStructElement = MbufAlloc2d(MilSystem, 10, 1, 32, M_STRUCT_ELEMENT, M_NULL);
   MbufControl(MilStructElement, M_OVERSCAN, M_MIRROR);
   MbufClear(MilStructElement, 1);
   MimMorphic(MilSrcImage,  MilDstImage1,  MilStructElement, M_OPEN, 1, M_BINARY);
   MbufFree(MilStructElement);

   MbufCopy(MilDstImage1, MilOverlayImage);
   MosPrintf(MIL_TEXT("A horizontal opening is used to remove the vertical structures.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Segment the vertical components using a vertical structuring element.
   MilStructElement = MbufAlloc2d(MilSystem, 1, 40, 32, M_STRUCT_ELEMENT, M_NULL);
   MbufControl(MilStructElement, M_OVERSCAN, M_MIRROR);
   MbufClear(MilStructElement, 1);
   MimMorphic(MilSrcImage,  MilDstImage2,  MilStructElement, M_OPEN, 1, M_BINARY);
   MimDilate(MilDstImage2, MilDstImage2, 1, M_BINARY);
   MbufFree(MilStructElement);

   MbufCopy(MilDstImage2, MilOverlayImage);
   MosPrintf(MIL_TEXT("A vertical opening is used to isolate the vertical structures.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Combine results.
   MimShift(MilDstImage1, MilDstImage1, -2);
   MimShift(MilDstImage2, MilDstImage2, -1);
   MimArith(MilDstImage1, MilDstImage2, MilDstImage1, M_OR);
   MbufCopy(MilDstImage1,MilDstImage2);
   MbufCopyCond(MilSrcImage, MilDstImage1, MilDstImage2, M_EQUAL, 0);

   // Display the segmentation result using pseudo colors.
   MIL_ID MilPseudoColorLut;
   AllocGenPseudoColorLUT(MilSystem, MilDisplay, 255>>2, 255, MilPseudoColorLut);
   MimLutMap(MilDstImage1, MilOverlayImage, MilPseudoColorLut);
   MosPrintf(MIL_TEXT("Results are combined and displayed using pseudo colors.\n"));
   MosPrintf(MIL_TEXT("The twisted pin's sections appear with emphasis in red.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   MbufFree(MilDstImage1);
   MbufFree(MilDstImage2);
   MbufFree(MilPseudoColorLut);
   }

//**********************************
// OBJECT PERIMETER
//**********************************

void ObjectPerimeter(MIL_CONST_TEXT_PTR SrcFilename,
                     MIL_ID MilSystem,
                     MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[OBJECT PERIMETER]\n\n")

      MIL_TEXT("In this example, the exoskeletons of the perimeters of\n")
      MIL_TEXT("dark objects are extracted and displayed.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   /* Pause to show the original image. */
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   MIL_ID BinImage,        /* Binary image buffer identifier.        */
          DilBinImage;     /* Dilated binary image buffer identifier.*/

   const MIL_DOUBLE IMAGE_THRESHOLD_VALUE  = 200;
   const MIL_INT SMALL_PARTICLE_RADIUS  = 2L;

   /* Allocate 2 binary image buffers for fast processing. */
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &BinImage);
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1 + M_UNSIGNED, M_IMAGE + M_PROC, &DilBinImage);

   /* Binarize the image. */
   MimBinarize(MilSrcImage, BinImage, M_FIXED + M_LESS_OR_EQUAL, IMAGE_THRESHOLD_VALUE, M_NULL);

   /* Remove small particles. */
   MimOpen(BinImage, BinImage, SMALL_PARTICLE_RADIUS, M_BINARY);

   /* Dilate image (adds one pixel around all objects). */
   MimDilate(BinImage, DilBinImage, 1L, M_BINARY);

   /* XOR the dilated image with the original image. */
   MimArith(BinImage, DilBinImage, BinImage, M_XOR);

   /* Display the resulting image. */
   MbufClear(MilDispProcImage, 0);
   MbufClearCond(MilOverlayImage, 255, 0, 255, BinImage, M_EQUAL, 1);

   /* Pause to show the resulting image. */
   MosPrintf(MIL_TEXT("Exoskeletons of the object's perimeters are displaed.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Release the allocated objects. */
   MbufFree(MilSrcImage);
   MbufFree(MilDispProcImage);
   MbufFree(DilBinImage);
   MbufFree(BinImage);
   }

//********************************
// MORPHOLOGICAL RECONSTRUCTION
//********************************
void MorphologicalReconstruction(MIL_CONST_TEXT_PTR SrcFilename, 
                                 MIL_ID MilSystem, 
                                 MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("[MORPHOLOGICAL RECONSTRUCTION]\n\n")

             MIL_TEXT("In this example, a combination of morphological operations and image\n")
             MIL_TEXT("arithmetics is used to perform a morphological reconstruction operation.\n\n"));

   // Restore the image.
   MIL_ID MilSrcImage = MbufRestore(SrcFilename, MilSystem, M_NULL);

   // Allocate a display image.
   MIL_ID MilDispProcImage,         // Display and destination buffer.
          MilOverlayImage;          // Overlay buffer.
   AllocDisplayImage(MilSystem, MilSrcImage, MilDisplay, MilDispProcImage, MilOverlayImage);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate the seed and the destination buffers.
   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);
   MIL_ID MilSeedImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilDstImage  = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);

   // Generate a seed buffer for the reconstruction.
   MimDilate(MilSrcImage, MilSeedImage, 5, M_GRAYSCALE);
   
   MbufCopy(MilSeedImage, MilDispProcImage);

   MosPrintf(MIL_TEXT("The reconstruction's seed image is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Perform the reconstruction.
   MorphoReconstruction(MilSystem, MilSrcImage, MilSeedImage, MilDstImage, 100);

   // Display the result.
   MbufCopy(MilDstImage, MilDispProcImage);

   MosPrintf(MIL_TEXT("The reconstruction is displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   // Release the allocated objects.
   MbufFree(MilSrcImage);
   MbufFree(MilSeedImage);
   MbufFree(MilDstImage);
   MbufFree(MilDispProcImage);
   }

///////////////////////////////////////////////
// Morphological reconstruction: core algorithm
///////////////////////////////////////////////
//
// Successive dilations of a seed image until 
// contours fits the source image. 
//
// ... : Seed image
// *** : Source image
//
//                     ***
//                    *   *
//        **         *     *
//      **  *       *       *
//     *      *     *  .    *
//    *   ..  *   *  .   .  *
//   *  ..  .. * *  .     . *
// *  ..      .....       . *
// ..                     ...............
//
// ...: Seed image
// ***: Reconstruction
//
//                   *******
//      ******      *       *
//     *      *     *  .    *
//    *   ..  *   *  .   .  *
//   *  ..  .. * *  .     . *
// *  ..      .....       . *
// ..                     ...............
//
//////////////////////////////////////////////
void MorphoReconstruction(MIL_ID  MilSystem,
                          MIL_ID  MilSrcImage, 
                          MIL_ID  MilSeedImage, 
                          MIL_ID  MilDstImage, 
                          MIL_INT MaxIter)
   {
   // Allocate a Mim result object.
   MIL_ID MilCountResult = MimAllocResult(MilSystem, 1, M_COUNT_LIST, M_NULL);

   // Retrieve source image properties.
   MIL_INT SizeX, SizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SizeY);

   // Allocate temporary buffers.
   MIL_ID MilCondImage    = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MIL_ID MilPrevDstImage = MbufAlloc2d(MilSystem, SizeX, SizeY, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   
   // Init conditions.
   MbufCopy(MilSeedImage, MilDstImage);
   MbufCopy(MilSeedImage, MilPrevDstImage);

   // Perform the morpological reconstruction.
   MimArith(MilDstImage, MilSrcImage, MilCondImage, M_SUB+M_SATURATION);
   MbufCopyCond(MilSrcImage, MilPrevDstImage, MilCondImage, M_EQUAL, 0);
   MimErode(MilPrevDstImage, MilDstImage, 1, M_GRAYSCALE);

   MIL_INT CountDiff=1;
   for(MIL_INT ii=0; ii<MaxIter && (CountDiff>0); ii++)
      {
      MimArith(MilDstImage, MilSrcImage, MilCondImage, M_SUB+M_SATURATION);
      MbufCopyCond(MilSrcImage, MilDstImage, MilCondImage, M_EQUAL, 0);
      MimCountDifference(MilDstImage, MilPrevDstImage, MilCountResult);
      MIL_INT CountDiff;
      MimGetResult(MilCountResult, M_VALUE+M_TYPE_MIL_INT, &CountDiff);
      if(CountDiff>0)
         {
         MbufCopy(MilDstImage, MilPrevDstImage);
         MimErode(MilPrevDstImage, MilDstImage, 1, M_GRAYSCALE);
         }
      }
   MbufFree(MilCondImage);
   MbufFree(MilPrevDstImage);
   MimFree(MilCountResult);
   };


/***************************/
void AllocDisplayImage(MIL_ID MilSystem, 
                       MIL_ID MilSrcImage, 
                       MIL_ID MilDisplay, 
                       MIL_ID& MilDispProcImage, 
                       MIL_ID& MilOverlayImage)
   {
   // Retrieve the source image size.
   MIL_INT SrcSizeX, SrcSizeY;
   MbufInquire(MilSrcImage, M_SIZE_X, &SrcSizeX);
   MbufInquire(MilSrcImage, M_SIZE_Y, &SrcSizeY);

   // Allocate the display image.
   MbufAlloc2d(MilSystem, 
               SrcSizeX,
               SrcSizeY,
               8L+M_UNSIGNED,
               M_IMAGE+M_PROC+M_DISP, 
               &MilDispProcImage);

   MbufCopy(MilSrcImage, MilDispProcImage);

   // Display the image buffer.
   MdispSelect(MilDisplay, MilDispProcImage);

   /* Prepare for overlay annotations. */
   MdispControl(MilDisplay, M_OVERLAY, M_ENABLE);
   MdispInquire(MilDisplay, M_OVERLAY_ID, &MilOverlayImage);
   MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
   }

/***************************/
void AllocGenPseudoColorLUT(MIL_ID  MilSystem,
                            MIL_ID  MilDisplay,
                            MIL_INT StartIndex, 
                            MIL_INT EndIndex, 
                            MIL_ID& MilPseudoColorLut)
   {
   // Calculate the interpolation param for H.
   MIL_DOUBLE Slope  = 160.0/(MIL_DOUBLE)(StartIndex-EndIndex);
   MIL_DOUBLE Offset = -Slope*(MIL_DOUBLE)EndIndex;

   // Generate the H LUT value.
   MIL_UINT8 HLut[256];
   for(MIL_INT ii=0; ii<256; ii++)
      {
      if(ii<StartIndex)
         HLut[ii]=160;
      else if(ii>EndIndex)
         HLut[ii]=0;
      else
         HLut[ii]=static_cast<MIL_UINT8>(Slope*ii+Offset+0.5);
      }

   // Convert the HSL values to RGB.
   MIL_ID MilTmpBuffer = MbufAllocColor(MilSystem, 3, 256, 1, 8L+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
   MbufClear(MilTmpBuffer, M_RGB888(0, 230, 120));
   MbufPutColor(MilTmpBuffer, M_SINGLE_BAND, 0, HLut);
   MimConvert(MilTmpBuffer, MilTmpBuffer, M_HSL_TO_RGB);

   // Map the '0' index to the keying color for overlay transparency.
   MIL_INT KeyingColor;
   MdispInquire(MilDisplay, M_TRANSPARENT_COLOR, &KeyingColor);
   MIL_ID MilTmpChild = MbufChild2d(MilTmpBuffer, 0, 0, 1, 1, M_NULL);
   MbufClear(MilTmpChild, (double) KeyingColor);

   // Copy values to the LUT buffer.
   MilPseudoColorLut = MbufAllocColor(MilSystem, 3, 256, 1, 8L+M_UNSIGNED, M_LUT, M_NULL);
   MbufCopy(MilTmpBuffer, MilPseudoColorLut);

   // Release temporary object.
   MbufFree(MilTmpChild);
   MbufFree(MilTmpBuffer);
   };

