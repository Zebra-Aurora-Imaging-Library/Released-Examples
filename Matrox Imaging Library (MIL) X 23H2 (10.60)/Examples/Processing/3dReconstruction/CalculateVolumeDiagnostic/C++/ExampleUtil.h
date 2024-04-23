//***************************************************************************************/
//
// File name: ExampleUtil.h
//
// Synopsis:  Utility header that contains helper functions and variables for the
//            CalculateVolumeDiagnostic example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#ifndef EXAMPLE_UTIL_H
#define EXAMPLE_UTIL_H

#include <cctype>

// Utility LUTS.
static std::vector<MIL_UINT8> STATUS_LUT = {  0,   0,   0, // Unused
                                              0, 255,   0, // Positive
                                            255,   0,   0, // Negative
                                            255, 255,   0, // Positive and Negative
                                              0,   0, 192, // Spliced Unused 
                                              0, 192,   0, // Spliced Positive
                                            192,   0,   0, // Spliced Negative
                                            192, 192,   0};// Spliced Positive and Negative

//*****************************************************************************
// Allocates a display of a given window size at a given position.
//*****************************************************************************
inline MIL_UNIQUE_DISP_ID AllocImageDisplay(MIL_ID MilSystem,
                                            MIL_INT OffsetX,
                                            MIL_INT OffsetY,
                                            MIL_INT SizeX,
                                            MIL_INT SizeY,
                                            MIL_CONST_TEXT_PTR DisplayName)
   {
   auto MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilDisplay, M_TITLE, DisplayName);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, OffsetX);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, OffsetY);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_X, SizeX);
   MdispControl(MilDisplay, M_WINDOW_INITIAL_SIZE_Y, SizeY);
   MdispControl(MilDisplay, M_SCALE_DISPLAY, M_ENABLE);
   return MilDisplay;
   }

//****************************************************************************
// Checks for required files to run the example.
//****************************************************************************
inline void CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to exit.\n\n"));
      MosGetch();
      exit(0);
      }
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.  
//*****************************************************************************
inline MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem,
                                             MIL_INT OffsetX,
                                             MIL_INT OffsetY,
                                             MIL_INT SizeX,
                                             MIL_INT SizeY,
                                             MIL_CONST_TEXT_PTR DisplayName)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"),
                                                    M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press <Enter> to exit.\n"));
      MosGetch();
      exit(0);
      }

   M3ddispControl(MilDisplay3D, M_WINDOW_INITIAL_POSITION_X, OffsetX);
   M3ddispControl(MilDisplay3D, M_WINDOW_INITIAL_POSITION_Y, OffsetY);
   M3ddispControl(MilDisplay3D, M_SIZE_X, SizeX);
   M3ddispControl(MilDisplay3D, M_SIZE_Y, SizeY);
   M3ddispControl(MilDisplay3D, M_TITLE, DisplayName);

   return MilDisplay3D;
   }

//*****************************************************************************
// Selects the image on the display if it exists. A color LUT is added to the display.
//*****************************************************************************
inline void SelectImageOnDisplay(MIL_ID MilDisplay, MIL_ID MilImage)
   {
   MdispSelect(MilDisplay, MilImage);
   if(MilImage)
      {
      MIL_INT MaxSelectedTargetValue = MbufInquire(MilImage, M_MAX, M_NULL);
      auto MilDisplayLut = MbufAllocColor(M_DEFAULT_HOST, 3, MaxSelectedTargetValue + 1, 1,
                                          8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
      MgenLutFunction(MilDisplayLut, M_COLORMAP_TURBO + M_LAST_GRAY, M_DEFAULT, M_COLOR_GRAY,
                      M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      MdispLut(MilDisplay, MilDisplayLut);
      }
   }

//*****************************************************************************
// Prompts user for yes/no.
//*****************************************************************************
inline bool AskYesNo(MIL_CONST_TEXT_PTR QuestionString)
   {
   MosPrintf(MIL_TEXT("%s (y/n)?\n"), QuestionString);
   while(true)
      {
      switch(MosGetch())
         {
         case MIL_TEXT('y'):
         case MIL_TEXT('Y'):
            MosPrintf(MIL_TEXT("YES\n\n"));
            return true;

         case MIL_TEXT('n'):
         case MIL_TEXT('N'):
            MosPrintf(MIL_TEXT("NO\n\n"));
            return false;
         }
      }
   }

//****************************************************************************
// Clip the two sizes if the reference size is too large. Preserves the
// aspect ratio.
//****************************************************************************
inline void ClipSizesIfRequired(MIL_INT MaxReferenceSize, MIL_INT& ReferenceSize, MIL_INT& OtherSize)
   {
   if(ReferenceSize > MaxReferenceSize)
      {
      OtherSize = (MIL_INT)(OtherSize * (MIL_DOUBLE)MaxReferenceSize / ReferenceSize);
      ReferenceSize = MaxReferenceSize;
      }
   }

//****************************************************************************
// Depth map generation from a point cloud.
//****************************************************************************
inline MIL_UNIQUE_BUF_ID GenerateDepthMap(MIL_ID MilPointCloudContainer,
                                          MIL_DOUBLE PixelSize,
                                          MIL_INT MaxDepthMapSizeX,
                                          MIL_INT MaxDepthMapSizeY)
   {
   auto MilPointCloudContainerClone = MbufClone(MilPointCloudContainer, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                                                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Create the mesh component of the depth map.
   auto MilRangeComponent = MbufInquireContainer(MilPointCloudContainer, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   auto MilMeshComponent = MbufInquireContainer(MilPointCloudContainer, M_COMPONENT_MESH_MIL, M_COMPONENT_ID, M_NULL);
   if(!MilMeshComponent)
      {
      if(MbufInquire(MilRangeComponent, M_3D_REPRESENTATION, M_NULL) == M_CALIBRATED_XYZ_UNORGANIZED)
         {
         M3dimNormals(M_NORMALS_CONTEXT_TREE, MilPointCloudContainer, MilPointCloudContainerClone, M_DEFAULT);
         M3dimMesh(M_MESH_CONTEXT_SMOOTHED, MilPointCloudContainerClone, MilPointCloudContainerClone, M_DEFAULT);
         }
      else
         M3dimMesh(M_MESH_CONTEXT_ORGANIZED, MilPointCloudContainer, MilPointCloudContainerClone, M_DEFAULT);
      }
   else
      MbufCopy(MilPointCloudContainer, MilPointCloudContainerClone);

   // Calculate the size required for the depth map.
   auto MilMapSizeContext = M3dimAlloc(M_DEFAULT_HOST, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilMapSizeContext, M_PIXEL_SIZE_X, PixelSize);
   M3dimControl(MilMapSizeContext, M_PIXEL_ASPECT_RATIO, M_DEFAULT);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MilMapSizeContext, MilPointCloudContainerClone, M_NULL, M_DEFAULT,
                         &DepthMapSizeX, &DepthMapSizeY);

   // Clip the depth map sizes.
   MIL_DOUBLE SizeXRatio = (MIL_DOUBLE)MaxDepthMapSizeX / DepthMapSizeX;
   MIL_DOUBLE SizeYRatio = (MIL_DOUBLE)MaxDepthMapSizeY / DepthMapSizeY;
   if(SizeXRatio < SizeYRatio)
      ClipSizesIfRequired(MaxDepthMapSizeX, DepthMapSizeX, DepthMapSizeY);
   else
      ClipSizesIfRequired(MaxDepthMapSizeY, DepthMapSizeY, DepthMapSizeX);

   // Allocate and calibrate the depth map.
   auto MilDepthmap = MbufAlloc2d(M_DEFAULT_HOST, DepthMapSizeX, DepthMapSizeY,
                                  M_UNSIGNED + 16, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dimCalibrateDepthMap(MilPointCloudContainerClone, MilDepthmap, M_NULL, M_NULL, M_DEFAULT, M_POSITIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(MilPointCloudContainerClone, MilDepthmap, M_NULL, M_MESH_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   return MilDepthmap;
   }

//*****************************************************************************
// Creates a 2D image from a 1D image, putting 0 in extra pixels.
//*****************************************************************************
template <typename T>
inline MIL_UNIQUE_BUF_ID Create2dImageFrom1d(MIL_ID MilImage1d, MIL_INT SizeX, MIL_INT SizeY)
   {
   auto MilImage2d = MbufAlloc2d(M_DEFAULT_HOST, SizeX, SizeY, MilTraits<T>::BufferTypeFlag,
                                 M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   std::vector<T> Data2d(SizeX * SizeY, 0);
   MbufGet(MilImage1d, Data2d.data());
   MbufPut(MilImage2d, Data2d);
   return MilImage2d;
   }

#endif // EXAMPLE_UTIL_H
