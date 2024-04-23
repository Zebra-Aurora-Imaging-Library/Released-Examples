//***************************************************************************************
//
// File name: ExampleUtil.h
//
// Synopsis:  Utility header that provides functions and classes to simplify the example code.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************

#pragma once

// Constants
static const MIL_DOUBLE DIV_PI_180 = 0.017453292519943295769236907684886;
static const MIL_DOUBLE DIV_180_PI = 57.295779513082320866997945294156;

//****************************************************************************
// Structure representing a BGR32 color.
//****************************************************************************
struct SBGR32Color
   {
   MIL_UINT8 B;
   MIL_UINT8 G;
   MIL_UINT8 R;
   MIL_UINT8 A;
   };

//****************************************************************************
// Depth map generation from a point cloud.
//****************************************************************************
MIL_UNIQUE_BUF_ID GenerateDepthMap(MIL_ID MilPointCloudContainer, MIL_DOUBLE PixelSize = 0)
   {
   auto MilPointCloudContainerClone = MbufClone(MilPointCloudContainer, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Create the mesh component of the depth map.
   auto MilRangeComponent = MbufInquireContainer(MilPointCloudContainer, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL);
   if(MbufInquire(MilRangeComponent, M_3D_REPRESENTATION, M_NULL) == M_CALIBRATED_XYZ_UNORGANIZED)
      {
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilPointCloudContainer, MilPointCloudContainerClone, M_DEFAULT);
      M3dimMesh(M_MESH_CONTEXT_SMOOTHED, MilPointCloudContainerClone, MilPointCloudContainerClone, M_DEFAULT);
      }
   else
      M3dimMesh(M_MESH_CONTEXT_ORGANIZED, MilPointCloudContainer, MilPointCloudContainerClone, M_DEFAULT);

   // Calculate the size required for the depth map.
   auto MilMapSizeContext = M3dimAlloc(M_DEFAULT_HOST, M_CALCULATE_MAP_SIZE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilMapSizeContext, M_PIXEL_SIZE_X, PixelSize);
   M3dimControl(MilMapSizeContext, M_PIXEL_SIZE_Y, PixelSize);
   M3dimControl(MilMapSizeContext, M_PIXEL_ASPECT_RATIO, M_DEFAULT);
   MIL_INT DepthMapSizeX, DepthMapSizeY;
   M3dimCalculateMapSize(MilMapSizeContext, MilPointCloudContainerClone, M_NULL, M_DEFAULT, &DepthMapSizeX, &DepthMapSizeY);

   // Allocate and calibrate the depth map.
   auto MilDepthmap = MbufAlloc2d(M_DEFAULT_HOST, DepthMapSizeX, DepthMapSizeY, M_UNSIGNED + 16, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dimCalibrateDepthMap(MilPointCloudContainerClone, MilDepthmap, M_NULL, M_NULL, M_DEFAULT, M_POSITIVE, M_DEFAULT);

   // Project the point cloud on the depth map.
   M3dimProject(MilPointCloudContainerClone, MilDepthmap, M_NULL, M_MESH_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);

   return MilDepthmap;
   }

//****************************************************************************
// Gets a certain number of distinct colors.
//****************************************************************************
std::vector<SBGR32Color> GetDistinctColors(MIL_INT NbColors)
   {
   auto MilPointCloudColors = MbufAllocColor(M_DEFAULT_HOST, 3, NbColors, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
   MgenLutFunction(MilPointCloudColors, M_COLORMAP_DISTINCT_256, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   std::vector<SBGR32Color> Colors(NbColors);
   MbufGetColor(MilPointCloudColors, M_PACKED + M_BGR32, M_ALL_BANDS, (MIL_UINT32*)(&Colors[0]));
   return Colors;
   }

//****************************************************************************
// Color the container.
//****************************************************************************
void ColorCloud(MIL_ID MilPointCloud, MIL_INT Col)
   {
   MIL_INT SizeX = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   
   auto MilRefelectance = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);
   if (MilRefelectance)
      MbufFreeComponent(MilPointCloud, M_COMPONENT_REFLECTANCE, M_DEFAULT);

   auto MilReflectance = MbufAllocComponent(MilPointCloud, 3, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PLANAR, M_COMPONENT_REFLECTANCE, M_NULL);
   MbufClear(MilReflectance, static_cast<MIL_DOUBLE>(Col));
   }

//****************************************************************************
// Checks for required files to run the example.
//****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Allocate3dDisplay(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      exit(0);
      }

   return MilDisplay3D;
   }

//*****************************************************************************
// Converts the container. Exit if it is not possible.
//*****************************************************************************
void ConvertPointCloud(MIL_ID MilSrcPointCloud, MIL_ID MilDstPointCloud)
   {
   if(MbufInquireContainer(MilSrcPointCloud, M_CONTAINER, M_3D_CONVERTIBLE, M_NULL) == M_NOT_CONVERTIBLE)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("Unable to convert the point cloud to a processable format.\n")
                MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      exit(0);
      }
   else
      MbufConvert3d(MilSrcPointCloud, MilDstPointCloud, M_NULL, M_DEFAULT, M_DEFAULT);
   }

//****************************************************************************
// Asks a question with a yes/no answer.
//****************************************************************************
bool AskYesNo(MIL_CONST_TEXT_PTR Question)
   {
   MosPrintf(MIL_TEXT("%s (y/n)?\n\n"), Question);
   while (1)
      {
      switch (MosGetch())
         {
         case 'Y':
         case 'y':
            return true;
         case 'N':
         case 'n':
            return false;
         default:
            break;
         }
      }
   }

//****************************************************************************
// Verifies if we have a simulated digitizer.
//****************************************************************************
bool IsRealDig(MIL_ID MilDigitizer)
   {
   return MdigInquire(MilDigitizer, M_SOURCE_NUMBER_OF_FRAMES, M_NULL) == M_INFINITE;
   }

//****************************************************************************
// MIL_UNIQUE_DIG_ID text streaming.
//****************************************************************************
MIL_STRING_STREAM& operator<<(MIL_STRING_STREAM& os, const MIL_UNIQUE_DIG_ID& MilDigitizer)
   {   
   if(IsRealDig(MilDigitizer))
      {
      MIL_STRING DeviceUserID = MIL_TEXT("");
      MIL_STRING DeviceScanType = MIL_TEXT("");
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceUserID"), M_TYPE_STRING, DeviceUserID);
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceScanType"), M_TYPE_STRING, DeviceScanType);
      os << DeviceUserID << MIL_TEXT(" ") << DeviceScanType << MIL_TEXT(" ");
      }
   os << MIL_TEXT("(MIL_ID = ") << MilDigitizer.get() << MIL_TEXT(")");

   return os;
   }

//****************************************************************************
// Asks a question with a a list of choices.
//****************************************************************************
template <class PrintableChoice>
MIL_INT AskMakeChoice(MIL_CONST_TEXT_PTR ChoiceQuestion, const std::vector<PrintableChoice>& Choices, MIL_CONST_TEXT_PTR DefaultChoice = M_NULL)
   {
   MIL_INT Choice;
   MIL_INT MinChoice = DefaultChoice != M_NULL ? -1 : 0;
   do
      {
      MosPrintf(MIL_TEXT("%s\n"), ChoiceQuestion);

      // Print the default choice.
      if(DefaultChoice != M_NULL)
         MosPrintf(MIL_TEXT("0. %s\n"),  DefaultChoice);

      // Print the choices.
      for(MIL_INT c = 0; c < (MIL_INT)Choices.size(); c++)
         {
         MIL_STRING_STREAM ChoiceStream;
         ChoiceStream << c + 1 << ". ";
         ChoiceStream << Choices[c];
         MosPrintf(MIL_TEXT("%s\n"), ChoiceStream.str().c_str());
         }

      MosPrintf(MIL_TEXT("\n"));
      Choice = MosGetch();
      Choice -= '1';
      }
      while(Choice < MinChoice || Choice >= (MIL_INT)Choices.size());

      // Print the choice.
      MIL_STRING_STREAM ChoiceStream;
      ChoiceStream << Choice + 1 << ". ";
      if(Choice == -1)
         ChoiceStream << DefaultChoice;
      else
         ChoiceStream << Choices[Choice];
      MosPrintf(MIL_TEXT("%s\n"), ChoiceStream.str().c_str());

      return Choice;
   }

//****************************************************************************
// Region display
//****************************************************************************
const MIL_INT MAX_REGION_DISPLAY_SIZE_X = 640;
class CRegionDisplay
   {
   public:

      CRegionDisplay(MIL_ID MilPointCloud)
         {
         // Generate the depth maps of the point clouds.
         m_MilDepthMap = GenerateDepthMap(MilPointCloud);
         MIL_INT SizeX = MbufInquire(m_MilDepthMap, M_SIZE_X, M_NULL);
         MIL_INT SizeY = MbufInquire(m_MilDepthMap, M_SIZE_Y, M_NULL);

         // Allocate the axis displays.
         m_MilDisplay = MdispAlloc(M_DEFAULT_HOST, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
         m_MilGraList = MgraAllocList(M_DEFAULT_HOST, M_DEFAULT, M_UNIQUE_ID);
         MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraList);
         MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
         MdispControl(m_MilDisplay, M_REGION_OUTSIDE_COLOR, M_COLOR_GRAY);
         MdispControl(m_MilDisplay, M_REGION_OUTSIDE_SHOW, M_GRAPHIC_LIST_OPACITY);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_OPACITY, 75);

         // Set the window initial size x.
         MIL_DOUBLE WindowZoom = 1.0;
         if(SizeX > MAX_REGION_DISPLAY_SIZE_X)
            {
            WindowZoom = (MIL_DOUBLE)MAX_REGION_DISPLAY_SIZE_X / SizeX;
            MdispZoom(m_MilDisplay, WindowZoom, WindowZoom);
            }

         // Select the depth map.
         MdispSelect(m_MilDisplay, m_MilDepthMap);

         // Zoom out so that we see the valid rectangle.
         MdispZoom(m_MilDisplay, 0.9 * WindowZoom, 0.9 * WindowZoom);

         // Draw the rectangle.
         MgraColor(M_DEFAULT, M_COLOR_BLACK);
         MgraRect(M_DEFAULT, m_MilGraList, 0.0, 0.0, SizeX - 1, SizeY - 1);

         // Set the color map LUT.
         auto MilColorMapLut = MbufAllocColor(M_DEFAULT_HOST, 3, 65535, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
         MgenLutFunction(MilColorMapLut, M_COLORMAP_TURBO + M_LAST_GRAY, M_DEFAULT, M_RGB888(128, 128, 128), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         MdispLut(m_MilDisplay, MilColorMapLut);

         MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
         }

      void StartInteractivity()
         {
         MgraHookFunction(m_MilGraList, M_GRAPHIC_MODIFIED, UpdateRegionHook, this);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_GRAPHIC_SELECTED, M_TRUE);
         }

      void StopInteractivity()
         {
         MgraHookFunction(m_MilGraList, M_GRAPHIC_MODIFIED + M_UNHOOK, UpdateRegionHook, this);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_DISABLE);
         }

      MIL_UNIQUE_3DGEO_ID GetValidBox() const
         {
         MIL_DOUBLE CX[2];
         MIL_DOUBLE CY[2];
         MIL_DOUBLE CZ[2] = {0.0, 65534.0};
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_CORNER_TOP_LEFT_X, &CX[0]);
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_CORNER_TOP_LEFT_Y, &CY[0]);
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_CORNER_BOTTOM_RIGHT_X, &CX[1]);
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_CORNER_BOTTOM_RIGHT_Y, &CY[1]);
         McalTransformCoordinate3dList(m_MilDepthMap, M_PIXEL_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                                       2, CX, CY, CZ, CX, CY, CZ, M_DEPTH_MAP);

         auto MilValidBox = M3dgeoAlloc(M_DEFAULT_HOST, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
         M3dgeoBox(MilValidBox, M_BOTH_CORNERS, CX[0], CY[0], CZ[0], CX[1], CY[1], CZ[1], M_DEFAULT);
         return MilValidBox;
         }
      
      bool UpdateRegion()
         {
         // Copy the region in the displayed depth map.
         auto MilRegionList = MgraAllocList(M_DEFAULT_HOST, M_DEFAULT, M_UNIQUE_ID);
         MgraCopy(m_MilGraList, MilRegionList, M_COPY, M_DEFAULT, M_ALL, M_NULL, M_NULL, M_DEFAULT);
         MbufSetRegion(m_MilDepthMap, MilRegionList, M_DEFAULT, M_RASTERIZE + M_FILL_REGION, M_DEFAULT);

         return true;
         }

   private:
      static MIL_INT MFTYPE UpdateRegionHook(MIL_INT, MIL_ID, void *pUserData)
         {
         ((CRegionDisplay*)pUserData)->UpdateRegion();
         return 0;
         }

      MIL_UNIQUE_BUF_ID m_MilDepthMap;
      MIL_UNIQUE_DISP_ID m_MilDisplay;
      MIL_UNIQUE_GRA_ID m_MilGraList;
   };

//****************************************************************************
// Interactively crops the point clouds.
//****************************************************************************
void InteractivePointCloudsCropping(const std::vector<MIL_ID>& MilPointClouds)
   {
   MosPrintf(MIL_TEXT("Action required:\n"));
   MosPrintf(MIL_TEXT("Use the interactive display to select regions\n"));
   MosPrintf(MIL_TEXT("that contain the alignment object.\n\n"));

   for(MIL_INT p = 0; p < (MIL_INT)MilPointClouds.size(); p++)
      {
      CRegionDisplay RegionDisplay(MilPointClouds[p]);

      RegionDisplay.StartInteractivity();
      MosPrintf(MIL_TEXT("Select point cloud %i region.\n"), p);
      MosPrintf(MIL_TEXT("Press <Enter> when done.\n\n"));
      MosGetch();
      RegionDisplay.StopInteractivity();

      auto MilValidBox = RegionDisplay.GetValidBox();
      M3dimCrop(MilPointClouds[p], MilPointClouds[p], MilValidBox, M_NULL, M_SAME, M_DEFAULT);
      }
   }

#include "Camera3dAcquisition.h"
#include "InteractiveAlignment.h"
#include "PointCloudsRegistration.h"
