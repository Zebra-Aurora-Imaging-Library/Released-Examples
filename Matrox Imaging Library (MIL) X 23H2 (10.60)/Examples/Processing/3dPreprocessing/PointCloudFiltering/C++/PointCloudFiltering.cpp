//***************************************************************************************/
// 
// File name: PointCloudFiltering.cpp  
//
// Synopsis:  This example demonstrates how to apply a smoothing filter to a point cloud.
//            This operation reduces noise and eliminates outlier points that can arise
//            during point cloud acquisition.
//
//            The point cloud was captured using an Intel RealSense Camera and
//            is loaded from a PLY file. The filter is then applied and the result is displayed.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/
#include <mil.h>

//--------------------------------------------------------------------------
// Example description.
//--------------------------------------------------------------------------
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("PointCloudFiltering\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to apply a smoothing filter to a point cloud.\n"));
   MosPrintf(MIL_TEXT("This operation reduces noise and eliminates outlier points that can arise\n"));
   MosPrintf(MIL_TEXT("during point cloud acquisition.\n\n"));

   MosPrintf(MIL_TEXT("The point cloud was captured using an Intel Realsense Camera and\n"));
   MosPrintf(MIL_TEXT("is loaded from a PLY file. The filter is then applied and the result is displayed.\n\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Display, 3D Image Processing, Buffer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

// Enumerators definitions.
enum { eOriginal = 0, eFiltered1 = 1, eFiltered2 = 2 };

MIL_CONST_TEXT_PTR ToString(MIL_INT Filter)
   {
   switch(Filter)
      {
      case M_SMOOTH_BILATERAL: return MIL_TEXT("Bilateral");
      case M_SMOOTH_MLS:       return MIL_TEXT("MLS");
      default:                 return MIL_TEXT("Unknown filter mode");
      }
   }

//----------------------------------------------------------------------------
// Struct containing all necessary information for the settings of a filter.
// In MIL Help, see M3dimControl's parameter associations section for possible
// values that can be specified. 
//----------------------------------------------------------------------------
struct SFilterOptions
   {
   MIL_INT      Mode = M_DEFAULT;
   MIL_BOOL     NormalsMode = M_FALSE;
   MIL_INT      NumNeighbors = M_DEFAULT;
   MIL_DOUBLE   DistWeight = M_DEFAULT;
   MIL_DOUBLE   NormalsFactor = M_DEFAULT;
   };

//----------------------------------------------------------------------------
// Struct containing all necessary information related to a filter :
//    - A filter context
//    - The filter's options
//----------------------------------------------------------------------------
struct SFilter3dim
   {
   MIL_UNIQUE_3DIM_ID Context;
   SFilterOptions Options;
   };

// Functions declarations.
MIL_UNIQUE_3DIM_ID BuildFilter(MIL_ID SysId, const SFilterOptions& Options);
void ApplyFilter(MIL_ID SysId, MIL_ID MilPointCloud, MIL_ID DstContainer, const SFilter3dim& Filter);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool CheckForRequiredMILFile(const MIL_STRING& FileName);

// Input data file.
static const MIL_STRING BOX_NOISY_POINT_CLOUD = M_IMAGE_PATH MIL_TEXT("PointCloudFiltering/box.ply");

static const MIL_INT    DISP_SIZE_X = 480;
static const MIL_INT    DISP_SIZE_Y = 520;

int MosMain(void)
   {
   // Print example information in console.
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   if(!CheckForRequiredMILFile(BOX_NOISY_POINT_CLOUD))
      { return 0; }

   MIL_UNIQUE_SYS_ID    MilSystem;        // System identifier.
   MIL_UNIQUE_3DDISP_ID MilDisplay3d[3];  // 3D Mil displays.
   MIL_UNIQUE_BUF_ID    MilPointCloud;    // Point cloud container.

   // Allocate MIL objects. 
   MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MilDisplay3d[eOriginal] = Alloc3dDisplayId(MilSystem);

   // Import 3D model from PLY file.
   MilPointCloud = MbufRestore(BOX_NOISY_POINT_CLOUD, MilSystem, M_UNIQUE_ID);

   // Control 3d display settings.
   M3ddispControl(MilDisplay3d[eOriginal], M_SIZE_X, DISP_SIZE_X);
   M3ddispControl(MilDisplay3d[eOriginal], M_SIZE_Y, DISP_SIZE_Y);
   M3ddispControl(MilDisplay3d[eOriginal], M_TITLE, MIL_TEXT("Input : Noisy point cloud"));
   M3ddispSetView(MilDisplay3d[eOriginal], M_AUTO, M_TOP_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   // Display the point cloud.
   M3ddispSelect(MilDisplay3d[eOriginal], MilPointCloud, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilDisplay3d[eOriginal], M_ZOOM, 1.4, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("A source point cloud with noisy data is loaded from a PLY file and displayed.\n"));
   MosPrintf(MIL_TEXT("The point cloud is displayed as a mesh for better visibility.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //--------------------------------------------------------------------------
   SFilter3dim Filter1;
   Filter1.Options.Mode          = M_SMOOTH_MLS;
   Filter1.Options.NormalsMode   = M_FALSE;
   Filter1.Options.NumNeighbors  = 100;
   Filter1.Options.DistWeight    = 1.0f;
   Filter1.Options.NormalsFactor = 1.0f;
   Filter1.Context = BuildFilter(MilSystem, Filter1.Options);

   MosPrintf(MIL_TEXT("A filter with the following parameters has been created:\n"));
   MosPrintf(MIL_TEXT("\tFilter mode           : %s\n"),   ToString(Filter1.Options.Mode));
   MosPrintf(MIL_TEXT("\tUse source normals    : %s\n"),
      (Filter1.Options.NormalsMode) ? MIL_TEXT("True") : MIL_TEXT("False"));
   MosPrintf(MIL_TEXT("\tNumber of neighbors   : %d\n"),   Filter1.Options.NumNeighbors);
   MosPrintf(MIL_TEXT("\tDistance weight       : %lf\n"),  Filter1.Options.DistWeight);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate and set a new display to show the filter's result.
   MilDisplay3d[eFiltered1] = Alloc3dDisplayId(MilSystem);

   M3ddispControl(MilDisplay3d[eFiltered1], M_SIZE_X, DISP_SIZE_X);
   M3ddispControl(MilDisplay3d[eFiltered1], M_SIZE_Y, DISP_SIZE_Y);
   M3ddispControl(MilDisplay3d[eFiltered1], M_WINDOW_INITIAL_POSITION_X, (MIL_INT)(1 * 1.04 * DISP_SIZE_X));
   M3ddispControl(MilDisplay3d[eFiltered1], M_TITLE, MIL_TEXT("Output : MLS Filter Mode"));
   M3ddispSetView(MilDisplay3d[eFiltered1], M_AUTO, M_TOP_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MIL_UNIQUE_BUF_ID MilPointCloudFiltered1 = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   ApplyFilter(MilSystem, MilPointCloud, MilPointCloudFiltered1, Filter1);
   M3ddispSelect(MilDisplay3d[eFiltered1], MilPointCloudFiltered1, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilDisplay3d[eFiltered1], M_ZOOM, 1.4, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The filter is applied to the source point cloud and displayed in a new window.\n"));            
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   //--------------------------------------------------------------------------
   SFilter3dim Filter2;
   Filter2.Options.Mode          = M_SMOOTH_BILATERAL;
   Filter2.Options.NormalsMode   = M_FALSE;
   Filter2.Options.NumNeighbors  = 100;
   Filter2.Options.DistWeight    = 1.0f;
   Filter2.Options.NormalsFactor = 0.08f;
   Filter2.Context = BuildFilter(MilSystem, Filter2.Options);

   MosPrintf(MIL_TEXT("A second filter with the following parameters has been created:\n"));
   MosPrintf(MIL_TEXT("\tFilter mode           : %s\n"), ToString(Filter2.Options.Mode));
   MosPrintf(MIL_TEXT("\tUse source normals    : %s\n"),
      (Filter1.Options.NormalsMode) ? MIL_TEXT("True") : MIL_TEXT("False"));
   MosPrintf(MIL_TEXT("\tNumber of neighbors   : %d\n"), Filter2.Options.NumNeighbors);
   MosPrintf(MIL_TEXT("\tDistance weight       : %lf\n"), Filter2.Options.DistWeight);
   MosPrintf(MIL_TEXT("\tNormal weight         : %lf\n"), Filter2.Options.NormalsFactor);
   MosPrintf(MIL_TEXT("This filter better preserves the edges.\n"), Filter2.Options.DistWeight);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MilDisplay3d[eFiltered2] = Alloc3dDisplayId(MilSystem);

   M3ddispControl(MilDisplay3d[eFiltered2], M_SIZE_X, DISP_SIZE_X);
   M3ddispControl(MilDisplay3d[eFiltered2], M_SIZE_Y, DISP_SIZE_Y);
   M3ddispControl(MilDisplay3d[eFiltered2], M_WINDOW_INITIAL_POSITION_X, (MIL_INT)(2 * 1.04 * DISP_SIZE_X));
   M3ddispControl(MilDisplay3d[eFiltered2], M_TITLE, MIL_TEXT("Output : Bilateral Filter Mode"));
   M3ddispSetView(MilDisplay3d[eFiltered2], M_AUTO, M_TOP_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MIL_UNIQUE_BUF_ID MilPointCloudFiltered2 = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   ApplyFilter(MilSystem, MilPointCloud, MilPointCloudFiltered2, Filter2);
   M3ddispSelect(MilDisplay3d[eFiltered2], MilPointCloudFiltered2, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(MilDisplay3d[eFiltered2], M_ZOOM, 1.4, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("The second filter is applied to the source point cloud and displayed\n"));
   MosPrintf(MIL_TEXT("in a new window.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//--------------------------------------------------------------------------
// Create a filter and return its MIL identifier.
//--------------------------------------------------------------------------
MIL_UNIQUE_3DIM_ID BuildFilter(MIL_ID SysId, const SFilterOptions& Options)
   {
   MIL_UNIQUE_3DIM_ID Filter = M3dimAlloc(SysId, M_FILTER_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   MIL_ID Normal;
   M3dimInquire(Filter, M_NORMALS_CONTEXT_ID, &Normal);

   M3dimControl(Normal, M_DIRECTION_MODE, M_AWAY_FROM_POSITION);
   M3dimControl(Normal, M_DIRECTION_REFERENCE_Z, 0.0);
   M3dimControl(Normal, M_MAXIMUM_NUMBER_NEIGHBORS, Options.NumNeighbors);
   M3dimControl(Filter, M_FILTER_MODE, Options.Mode);
   M3dimControl(Filter, M_WEIGHT_MODE, M_RELATIVE);
   M3dimControl(Filter, M_DISTANCE_WEIGHT, Options.DistWeight);
   M3dimControl(Filter, M_NORMALS_WEIGHT_FACTOR, Options.NormalsFactor);
   M3dimControl(Filter, M_USE_SOURCE_NORMALS, Options.NormalsMode);

   return Filter;
   }

//--------------------------------------------------------------------------
// Apply a filter to a point cloud.
//--------------------------------------------------------------------------
void ApplyFilter(MIL_ID SysId, MIL_ID MilPointCloud, MIL_ID DstContainer, const SFilter3dim& Filter)
   {
   // To use the input's normals and the input point cloud as the destination point cloud, a separate copy
   // of the source's normals must be kept.
   MIL_UNIQUE_BUF_ID IdealNormals;
   if(Filter.Options.NormalsMode == M_TRUE)
      {
      MIL_ID Normals = MbufInquireContainer(MilPointCloud, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);
      IdealNormals = MbufClone(Normals, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
      }

   M3dimFilter(Filter.Context, MilPointCloud, DstContainer, M_DEFAULT);
   // If using ideal normals, copy the source's normals to the destination's normals.
   if(Filter.Options.NormalsMode == M_TRUE)
      {
      MIL_ID Normals = MbufInquireContainer(DstContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);
      MbufCopy(IdealNormals, Normals);
      }
   }

//--------------------------------------------------------------------------
// Create a 3D display and return its MIL identifier.
//--------------------------------------------------------------------------
MIL_UNIQUE_3DDISP_ID  Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID  MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }

   return MilDisplay3D;
   }

//--------------------------------------------------------------------------
// Check for required files to run the example.    
//--------------------------------------------------------------------------
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
