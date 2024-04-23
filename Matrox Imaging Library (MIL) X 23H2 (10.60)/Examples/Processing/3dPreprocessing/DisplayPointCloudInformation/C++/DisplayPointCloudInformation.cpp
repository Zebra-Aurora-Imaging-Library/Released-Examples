//***************************************************************************************/
//
// File name: DisplayPointCloudInformation.cpp
//
// Synopsis:  This example restores a point cloud from a file, then displays
//            information about the container buffer and statistics of the data.
//            The file can be the default example or a file supplied either
//            interactively or as a command argument.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <math.h>
#include "Vec3.h"

// Source file specification.
static const MIL_STRING PT_CLD_FILE = M_IMAGE_PATH MIL_TEXT("3dModelHeightDefect/3dObject.mbufc");

// Constants and structures.
static const MIL_INT DISP3D_SIZE_X           = 600;
static const MIL_INT DISP3D_SIZE_Y           = 450;
static const MIL_INT NORMALS_WINDOW_OFFSET_Y = DISP3D_SIZE_Y + 30;
static const MIL_INT ANNOTATION_THICKNESS    = 3;

struct Properties3d
   {
   MIL_INT     m_3dDistanceUnit;
   MIL_INT     m_3dCoordinateSystemType;
   MIL_INT     m_3dRepresentation;
   MIL_DOUBLE  m_3dScaleX;
   MIL_DOUBLE  m_3dScaleY;
   MIL_DOUBLE  m_3dScaleZ;
   MIL_DOUBLE  m_3dOffsetX;
   MIL_DOUBLE  m_3dOffsetY;
   MIL_DOUBLE  m_3dOffsetZ;
   MIL_DOUBLE  m_3dShearX;
   MIL_DOUBLE  m_3dShearZ;
   MIL_INT     m_3dInvalidDataFlag;
   MIL_DOUBLE  m_3dInvalidDataValue;
   MIL_DOUBLE  m_3dDisparityFocal;
   MIL_DOUBLE  m_3dDisparityBaseline;
   MIL_DOUBLE  m_3dDisparityPointU;
   MIL_DOUBLE  m_3dDisparityPointV;
   };

// Function declarations.
MIL_UNIQUE_BUF_ID ObtainPointCloud(MIL_STRING& PointCloudFile);
MIL_UNIQUE_BUF_ID RestorePointCloud(MIL_CONST_TEXT_PTR PointCloudFilename);

void ScanComponents(MIL_ID MilContainer);
void PrintComponent(MIL_ID MilComponent, MIL_UINT CompIdx, const MIL_STRING& CompName);
Properties3d Fetch3dProperties(MIL_ID MilComponent);
void Print3dProperties(const Properties3d& Props);
void PrintProcessableState(MIL_ID MilContainer);

void DrawBoundingBox(MIL_ID MilContainer, MIL_ID MilGraphicsList);
void DrawPCA(MIL_ID MilContainer, MIL_ID MilGraphicsList);
void DrawNormals(MIL_ID MilContainer, MIL_ID Mil3dDisplay);
void CalculateMoments(MIL_ID MilContainer);
void CalculateSurfaceVariation(MIL_ID MilContainer);
void CalculateNearestNeighbors(MIL_ID MilContainer);

bool AskYesNo(MIL_CONST_TEXT_PTR QuestionString);
MIL_CONST_TEXT_PTR UnitString(MIL_INT Unit);
MIL_CONST_TEXT_PTR CSString(MIL_INT CS);
MIL_CONST_TEXT_PTR RepresentationString(MIL_INT Representation);
MIL_CONST_TEXT_PTR BoolString(MIL_INT Bool);
MIL_CONST_TEXT_PTR BufTypeString(MIL_INT64 ObjType);
MIL_CONST_TEXT_PTR ElemTypeString(MIL_INT64 ElemType);
bool AddComponentNormalsIfMissing(MIL_ID MilContainer);
void CheckForRequiredMILFile(const MIL_STRING& FileName);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DisplayPointCloudInformation\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example restores a point cloud from a file, then displays\n")
             MIL_TEXT("information about the container buffer and statistics of the data.\n")
             MIL_TEXT("The file can be the default example or a file supplied either\n")
             MIL_TEXT("interactively or as a command argument.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("3D Display, 3D Geometry, 3D Graphics, 3D Image Processing and Buffer.\n\n"));
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(int argc, MIL_TEXT_CHAR* argv[])
   {
   // Allocate the MIL application and system.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a 3D display for displaying the point cloud's bounding box.
   auto MilDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   M3ddispControl(MilDisplay, M_SIZE_X, DISP3D_SIZE_X);
   M3ddispControl(MilDisplay, M_SIZE_Y, DISP3D_SIZE_Y);
   
   // Allocate a 3D display for displaying the principal component analysis (PCA).
   auto MilPCADisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   M3ddispControl(MilPCADisplay, M_SIZE_X, DISP3D_SIZE_X);
   M3ddispControl(MilPCADisplay, M_SIZE_Y, DISP3D_SIZE_Y);
   M3ddispControl(MilPCADisplay, M_WINDOW_INITIAL_POSITION_X, DISP3D_SIZE_X);

   // Allocate a 3D display for displaying the normals.
   auto MilNormalsDisplay = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   M3ddispControl(MilNormalsDisplay, M_SIZE_X, DISP3D_SIZE_X);
   M3ddispControl(MilNormalsDisplay, M_SIZE_Y, DISP3D_SIZE_Y);
   M3ddispControl(MilNormalsDisplay, M_WINDOW_INITIAL_POSITION_Y, NORMALS_WINDOW_OFFSET_Y);

   MIL_STRING PointCloudFile = MIL_TEXT("");
   if(argc >= 2)
      PointCloudFile = argv[1];
   else
      PrintHeader();

   // Allocate and restore a point cloud.
   auto MilPointCloud = ObtainPointCloud(PointCloudFile);

   // Print the 3D processable status of the point cloud.
   PrintProcessableState(MilPointCloud);

   // Scan the components of the point cloud and print information about them.
   ScanComponents(MilPointCloud);

   // Calculate the bounding box and display it.
   DrawBoundingBox(MilPointCloud, MilDisplay);

   // Calculate the PCA and display it.
   DrawPCA(MilPointCloud, MilPCADisplay);

   // Calculate the normals and display them.
   DrawNormals(MilPointCloud, MilNormalsDisplay);

   // Calculate the first order moments and print them.
   CalculateMoments(MilPointCloud);

   // Calculate the surface variation statistics and print them.
   CalculateSurfaceVariation(MilPointCloud);

   // Calculate the nearest neighbor statistics and print them.
   CalculateNearestNeighbors(MilPointCloud);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   return 0;
   }

//****************************************************************************
// Finds all the components of a 3D container, and prints information about them.
//****************************************************************************
void ScanComponents(MIL_ID MilContainer)
   {
   MosPrintf(MIL_TEXT("Components:\n"));

   std::vector<MIL_ID> ComponentIds;
   MbufInquireContainer(MilContainer, M_CONTAINER, M_COMPONENT_LIST, ComponentIds);
   for(MIL_UINT ComponentIndex = 0; ComponentIndex < ComponentIds.size(); ++ComponentIndex)
      {
      MIL_STRING ComponentName;
      MbufInquire(ComponentIds[ComponentIndex], M_COMPONENT_TYPE_NAME, ComponentName);
      PrintComponent(ComponentIds[ComponentIndex], ComponentIndex, ComponentName);

      MIL_INT64 ComponentType;
      MbufInquire(ComponentIds[ComponentIndex], M_COMPONENT_TYPE, &ComponentType);
      if(ComponentType == M_COMPONENT_RANGE || ComponentType == M_COMPONENT_DISPARITY)
         {
         const auto Props = Fetch3dProperties(ComponentIds[ComponentIndex]);
         Print3dProperties(Props);
         }
      }
   MosPrintf(MIL_TEXT("\n"));
   }

//****************************************************************************
// Prints the 3D processable status of a container.
//****************************************************************************
void PrintProcessableState(MIL_ID MilContainer)
   {
   const auto IsProc = MbufInquireContainer(MilContainer, M_CONTAINER, M_3D_PROCESSABLE, nullptr);
   const auto IsProcMesh = MbufInquireContainer(MilContainer, M_CONTAINER, M_3D_PROCESSABLE_MESHED, nullptr);
   const auto IsDisp = MbufInquireContainer(MilContainer, M_CONTAINER, M_3D_DISPLAYABLE, nullptr);
   const auto IsConv = MbufInquireContainer(MilContainer, M_CONTAINER, M_3D_CONVERTIBLE, nullptr);

   MosPrintf(MIL_TEXT("General inquires:\n"));

   MosPrintf(MIL_TEXT("   M_3D_PROCESSABLE       : "));
   switch(IsProc)
      {
      case M_PROCESSABLE:     MosPrintf(MIL_TEXT("M_PROCESSABLE    \n")); break;
      case M_NOT_PROCESSABLE: MosPrintf(MIL_TEXT("M_NOT_PROCESSABLE\n")); break;
      default:                MosPrintf(MIL_TEXT("<unknown value>\n"));   break;
      }

   MosPrintf(MIL_TEXT("   M_3D_PROCESSABLE_MESHED: "));
   switch(IsProcMesh)
      {
      case M_TRUE:  MosPrintf(MIL_TEXT("M_TRUE \n"));         break;
      case M_FALSE: MosPrintf(MIL_TEXT("M_FALSE\n"));         break;
      default:      MosPrintf(MIL_TEXT("<unknown value>\n")); break;
      }

   MosPrintf(MIL_TEXT("   M_3D_DISPLAYABLE       : "));
   switch(IsDisp)
      {
      case M_DISPLAYABLE:                 MosPrintf(MIL_TEXT("M_DISPLAYABLE                \n")); break;
      case M_DISPLAYABLE_WITH_CONVERSION: MosPrintf(MIL_TEXT("M_DISPLAYABLE_WITH_CONVERSION\n")); break;
      case M_NOT_DISPLAYABLE:             MosPrintf(MIL_TEXT("M_NOT_DISPLAYABLE            \n")); break;
      default:                            MosPrintf(MIL_TEXT("<unknown value>\n"));               break;
      }

   MosPrintf(MIL_TEXT("   M_3D_CONVERTIBLE       : "));
   switch(IsConv)
      {
      case M_CONVERTIBLE:                   MosPrintf(MIL_TEXT("M_CONVERTIBLE                  \n")); break;
      case M_CONVERTIBLE_WITH_COMPENSATION: MosPrintf(MIL_TEXT("M_CONVERTIBLE_WITH_COMPENSATION\n")); break;
      case M_NOT_CONVERTIBLE:               MosPrintf(MIL_TEXT("M_NOT_CONVERTIBLE              \n")); break;
      default:                              MosPrintf(MIL_TEXT("<unknown value>\n"));                 break;
      }

   MosPrintf(MIL_TEXT("\n"));
   }

//****************************************************************************
// Prints the properties of a 3D component.
//****************************************************************************
void Print3dProperties(const Properties3d& Props)
   {
   MosPrintf(MIL_TEXT("       M_3D_DISTANCE_UNIT              : %s\n"), UnitString(Props.m_3dDistanceUnit));
   MosPrintf(MIL_TEXT("       M_3D_COORDINATE_SYSTEM_TYPE     : %s\n"), CSString(Props.m_3dCoordinateSystemType));
   MosPrintf(MIL_TEXT("       M_3D_REPRESENTATION             : %s\n"), RepresentationString(Props.m_3dRepresentation));
   MosPrintf(MIL_TEXT("       M_3D_SCALE_X                    : %f\n"), Props.m_3dScaleX);
   MosPrintf(MIL_TEXT("       M_3D_SCALE_Y                    : %f\n"), Props.m_3dScaleY);
   MosPrintf(MIL_TEXT("       M_3D_SCALE_Z                    : %f\n"), Props.m_3dScaleZ);
   MosPrintf(MIL_TEXT("       M_3D_OFFSET_X                   : %f\n"), Props.m_3dOffsetX);
   MosPrintf(MIL_TEXT("       M_3D_OFFSET_Y                   : %f\n"), Props.m_3dOffsetY);
   MosPrintf(MIL_TEXT("       M_3D_OFFSET_Z                   : %f\n"), Props.m_3dOffsetZ);
   MosPrintf(MIL_TEXT("       M_3D_SHEAR_X                    : %f\n"), Props.m_3dShearX);
   MosPrintf(MIL_TEXT("       M_3D_SHEAR_Z                    : %f\n"), Props.m_3dShearZ);
   MosPrintf(MIL_TEXT("       M_3D_INVALID_DATA_FLAG          : %s\n"), BoolString(Props.m_3dInvalidDataFlag));
   MosPrintf(MIL_TEXT("       M_3D_INVALID_DATA_VALUE         : %f\n"), Props.m_3dInvalidDataValue);
   MosPrintf(MIL_TEXT("       M_3D_DISPARITY_FOCAL_LENGTH     : %f\n"), Props.m_3dDisparityFocal);
   MosPrintf(MIL_TEXT("       M_3D_DISPARITY_BASELINE         : %f\n"), Props.m_3dDisparityBaseline);
   MosPrintf(MIL_TEXT("       M_3D_DISPARITY_PRINCIPAL_POINT_X: %f\n"), Props.m_3dDisparityPointU);
   MosPrintf(MIL_TEXT("       M_3D_DISPARITY_PRINCIPAL_POINT_Y: %f\n"), Props.m_3dDisparityPointV);
   }

//****************************************************************************
// Prints information about a 3D component.
//****************************************************************************
void PrintComponent(MIL_ID MilComponent, MIL_UINT CompIdx, const MIL_STRING& CompName)
   {
   MIL_INT64 ObjType;
   MobjInquire(MilComponent, M_OBJECT_TYPE, &ObjType);
   if(ObjType == M_CONTAINER)
      {
      MosPrintf(MIL_TEXT("  %2d: CONTAINER '%s'\n"), (int)CompIdx, CompName.c_str());
      }
   else
      {
      MIL_INT ElemType = MbufInquire(MilComponent, M_TYPE, nullptr);
      MIL_INT SizeX = MbufInquire(MilComponent, M_SIZE_X, nullptr);
      MIL_INT SizeY = MbufInquire(MilComponent, M_SIZE_Y, nullptr);
      MIL_INT NumBands = MbufInquire(MilComponent, M_SIZE_BAND, nullptr);
      MosPrintf(MIL_TEXT("  %2d: %s %s x%d %dx%d '%s'\n"), (int)CompIdx,
                                                           BufTypeString(ObjType),
                                                           ElemTypeString(ElemType),
                                                           (int)NumBands,
                                                           (int)SizeX,
                                                           (int)SizeY,
                                                           CompName.c_str());
      }
   }

//****************************************************************************
// Inquires the properties of a 3D component and stores them in a structure.
//****************************************************************************
Properties3d Fetch3dProperties(MIL_ID MilComponent)
   {
   Properties3d Props;
   MIL_INT64 ComponentType;
   MbufInquire(MilComponent, M_COMPONENT_TYPE, &ComponentType);

   MbufInquire(MilComponent, M_3D_DISTANCE_UNIT, &Props.m_3dDistanceUnit);
   MbufInquire(MilComponent, M_3D_COORDINATE_SYSTEM_TYPE, &Props.m_3dCoordinateSystemType);
   MbufInquire(MilComponent, M_3D_REPRESENTATION, &Props.m_3dRepresentation);
   MbufInquire(MilComponent, M_3D_SCALE_X, &Props.m_3dScaleX);
   MbufInquire(MilComponent, M_3D_SCALE_Y, &Props.m_3dScaleY);
   MbufInquire(MilComponent, M_3D_SCALE_Z, &Props.m_3dScaleZ);
   MbufInquire(MilComponent, M_3D_OFFSET_X, &Props.m_3dOffsetX);
   MbufInquire(MilComponent, M_3D_OFFSET_Y, &Props.m_3dOffsetY);
   MbufInquire(MilComponent, M_3D_OFFSET_Z, &Props.m_3dOffsetZ);
   MbufInquire(MilComponent, M_3D_SHEAR_X, &Props.m_3dShearX);
   MbufInquire(MilComponent, M_3D_SHEAR_Z, &Props.m_3dShearZ);
   MbufInquire(MilComponent, M_3D_INVALID_DATA_FLAG, &Props.m_3dInvalidDataFlag);
   MbufInquire(MilComponent, M_3D_INVALID_DATA_VALUE, &Props.m_3dInvalidDataValue);
   if(ComponentType == M_COMPONENT_DISPARITY)
      {
      MbufInquire(MilComponent, M_3D_DISPARITY_FOCAL_LENGTH, &Props.m_3dDisparityFocal);
      MbufInquire(MilComponent, M_3D_DISPARITY_BASELINE, &Props.m_3dDisparityBaseline);
      MbufInquire(MilComponent, M_3D_DISPARITY_PRINCIPAL_POINT_X, &Props.m_3dDisparityPointU);
      MbufInquire(MilComponent, M_3D_DISPARITY_PRINCIPAL_POINT_Y, &Props.m_3dDisparityPointV);
      }
   else
      {
      Props.m_3dDisparityFocal = 1.0;
      Props.m_3dDisparityBaseline = 1.0;
      Props.m_3dDisparityPointU = 0.0;
      Props.m_3dDisparityPointV = 0.0;
      }
   return Props;
   }

//****************************************************************************
// Obtains a point cloud either from file or by calculating with example data.
//****************************************************************************
MIL_UNIQUE_BUF_ID ObtainPointCloud(MIL_STRING& PointCloudFile)
   {
   MIL_UNIQUE_BUF_ID MilPointCloud;
   do
      {
      if(PointCloudFile != MIL_TEXT(""))
         MilPointCloud = RestorePointCloud(PointCloudFile.c_str());
      else if(AskYesNo(MIL_TEXT("Do you want to load a user point cloud")))
         {
         MosPrintf(MIL_TEXT("Please select a .mbufc or .ply point cloud file.\n\n"));
         MilPointCloud = RestorePointCloud(M_NULL);
         }
      else
         {
         MosPrintf(MIL_TEXT("The example will run using a point cloud from example source data.\n\n"));
         CheckForRequiredMILFile(PT_CLD_FILE);
         MilPointCloud = RestorePointCloud(PT_CLD_FILE.c_str());
         }
      } while(!MilPointCloud);

      // Convert the source point cloud to a 3D processable format.
      MIL_ID MilSystem;
      MobjInquire(MilPointCloud, M_OWNER_SYSTEM, &MilSystem);

      auto MilProcessableContainer = MbufAllocContainer(MilSystem, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
      MbufConvert3d(MilPointCloud, MilProcessableContainer, M_NULL, M_REMOVE_NON_FINITE, M_COMPENSATE);

      return MilProcessableContainer;
   }

//****************************************************************************
// Restores the registration result from file.
//****************************************************************************
MIL_UNIQUE_BUF_ID RestorePointCloud(MIL_CONST_TEXT_PTR PointCloudFilename)
   {
   // Restore the 3dreg result.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto MilPointCloud = MbufImport(PointCloudFilename, M_DEFAULT, M_RESTORE, M_DEFAULT_HOST, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // The restored file must be a convertible container.
   if(MilPointCloud &&
      (MobjInquire(MilPointCloud, M_OBJECT_TYPE, M_NULL) != M_CONTAINER ||
       MbufInquireContainer(MilPointCloud, M_CONTAINER, M_3D_CONVERTIBLE, M_NULL) == M_NOT_CONVERTIBLE))
      {
      MilPointCloud.reset();
      }

   // Verify that the result is valid.
   if(!MilPointCloud)
      MosPrintf(MIL_TEXT("No valid .mbufc file restored.\n\n"));

   return MilPointCloud;
   }

//*****************************************************************************
// Draws the bounding box of a 3D point cloud, and returns the max dimension.
//*****************************************************************************
void DrawBoundingBox(MIL_ID MilContainer, MIL_ID Mil3dDisplay)
   {
   M3ddispControl(Mil3dDisplay, M_UPDATE, M_DISABLE);

   auto MilGraphicsList = M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Display the point cloud and specify to use the range component when coloring the points
   // of the point cloud if the reflectance is not available.
   const auto PtCldLabel = M3ddispSelect(Mil3dDisplay, MilContainer, M_DEFAULT, M_DEFAULT);

   auto MilReflectanceComponent = MbufInquireContainer(MilContainer, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);

   if(MilReflectanceComponent)
      {
      M3dgraControl(MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT, M_COMPONENT_REFLECTANCE);
      M3ddispControl(Mil3dDisplay, M_TITLE, MIL_TEXT("Reflectance, Origin, Bounding box"));
      }
   else
      {
      M3dgraControl(MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
      M3dgraControl(MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT_BAND, 2);
      M3ddispControl(Mil3dDisplay, M_TITLE, MIL_TEXT("Range, Origin, Bounding box"));
      M3dgraControl(MilGraphicsList, PtCldLabel, M_COLOR_USE_LUT, M_TRUE);
      }   

   // Set all the annotations thickness.
   M3dgraControl(MilGraphicsList, M_DEFAULT_SETTINGS, M_THICKNESS, ANNOTATION_THICKNESS);

   // Allocate a statistics 3D image processing result buffer.
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Calculate the bounding box.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, MilContainer, MilStatResult, M_DEFAULT);

   // Copy the resulting bounding box to a 3d geometry object and display it.
   auto BBox = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dimCopyResult(MilStatResult, BBox, M_BOUNDING_BOX, M_DEFAULT);
   const auto BoxLabel = M3dgeoDraw3d(M_DEFAULT, BBox, MilGraphicsList, M_DEFAULT, M_DEFAULT);

   M3dgraControl(MilGraphicsList, BoxLabel, M_COLOR, M_COLOR_YELLOW);
   M3dgraControl(MilGraphicsList, BoxLabel, M_APPEARANCE, M_WIREFRAME);

   // Inquire and display the dimensions of the bounding box and display them.
   const MIL_DOUBLE Dims[3] = {M3dgeoInquire(BBox, M_SIZE_X, nullptr),
                               M3dgeoInquire(BBox, M_SIZE_Y, nullptr),
                               M3dgeoInquire(BBox, M_SIZE_Z, nullptr)};
   MosPrintf(MIL_TEXT("Bounding box: %.3g x %.3g x %.3g\n\n"), Dims[0], Dims[1], Dims[2]);

   // Display the axis of the bounding box.
   const auto BoxMaxDim = *std::max_element(Dims, Dims + 3);
   const auto AxisDim = 0.1 * BoxMaxDim;
   M3dgraAxis(MilGraphicsList, M_DEFAULT, M_DEFAULT, AxisDim, nullptr, M_DEFAULT, M_DEFAULT);

   M3ddispControl(Mil3dDisplay, M_UPDATE, M_ENABLE);
   }

//*****************************************************************************
// Draws the PCA of a 3D point cloud.
//*****************************************************************************
void DrawPCA(MIL_ID MilContainer, MIL_ID Mil3dDisplay)
   {
   M3ddispControl(Mil3dDisplay, M_UPDATE, M_DISABLE);

   const MIL_INT PCA_AXIS_COLORS[] = {M_COLOR_MAGENTA, M_COLOR_YELLOW, M_COLOR_CYAN};

   auto MilGraphicsList = M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraControl(MilGraphicsList, M_DEFAULT_SETTINGS, M_THICKNESS, ANNOTATION_THICKNESS);

   // Display the point cloud and specify to use the range component
   // when coloring the points of the point cloud.
   const auto PtCldLabel = M3ddispSelect(Mil3dDisplay, MilContainer, M_DEFAULT, M_DEFAULT);
   M3dgraControl (MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3dgraControl (MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT_BAND, 2);
   M3ddispControl(Mil3dDisplay, M_TITLE, MIL_TEXT("Range, PCA"));

   M3dgraControl(MilGraphicsList, PtCldLabel, M_OPACITY, 10);

   // Set all the annotations thickness.
   M3dgraControl(MilGraphicsList, M_DEFAULT_SETTINGS, M_THICKNESS, ANNOTATION_THICKNESS);

   // Allocate a statistics 3D image processing result buffer.
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Calculate the PCA.
   M3dimStat(M_STAT_CONTEXT_PCA, MilContainer, MilStatResult, M_DEFAULT);

   // Get the PCA results and eigenvalues.
   std::vector<MIL_UNIQUE_3DGEO_ID> PCAAxis(3);
   std::vector<MIL_DOUBLE> PCAEigenvalues(3);
   for(MIL_UINT i = 0; i < PCAAxis.size(); i++)
      {
      PCAAxis[i] = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
      }
   M3dimCopyResult(MilStatResult, PCAAxis[0], M_PRINCIPAL_AXIS_1, M_DEFAULT);
   M3dimCopyResult(MilStatResult, PCAAxis[1], M_PRINCIPAL_AXIS_2, M_DEFAULT);
   M3dimCopyResult(MilStatResult, PCAAxis[2], M_PRINCIPAL_AXIS_3, M_DEFAULT);

   Vec3 Centroid;
   Centroid.x = M3dimGetResult(MilStatResult, M_CENTROID_X, M_NULL);
   Centroid.y = M3dimGetResult(MilStatResult, M_CENTROID_Y, M_NULL);
   Centroid.z = M3dimGetResult(MilStatResult, M_CENTROID_Z, M_NULL);

   M3dimGetResult(MilStatResult, M_EIGENVALUE_1, &PCAEigenvalues[0]);
   M3dimGetResult(MilStatResult, M_EIGENVALUE_2, &PCAEigenvalues[1]);
   M3dimGetResult(MilStatResult, M_EIGENVALUE_3, &PCAEigenvalues[2]);

   // Allocate a node for the PCA results and draw the line geometries into the 3D graphics list.
   auto NodeLabel = M3dgraNode(MilGraphicsList, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   for(MIL_UINT i = 0; i < PCAAxis.size(); i++)
      {
      // Calculate the axis line start and end.
      Vec3 Axis;
      Axis.x = M3dgeoInquire(PCAAxis[i], M_AXIS_X, M_NULL);
      Axis.y = M3dgeoInquire(PCAAxis[i], M_AXIS_Y, M_NULL);
      Axis.z = M3dgeoInquire(PCAAxis[i], M_AXIS_Z, M_NULL);

      MIL_DOUBLE LineLength = 2.5 * sqrt(PCAEigenvalues[i]);
      Axis = LineLength * Axis;

      Vec3 StartAxis = Centroid + Axis;
      Vec3 EndAxis = Centroid - Axis;

      auto AxisGraphic = M3dgraLine(MilGraphicsList, NodeLabel, M_TWO_POINTS, M_DEFAULT,
                                    StartAxis.x, StartAxis.y, StartAxis.z,
                                    EndAxis.x, EndAxis.y, EndAxis.z,
                                    M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilGraphicsList, AxisGraphic, M_COLOR, PCA_AXIS_COLORS[i]);
      }

   M3ddispControl(Mil3dDisplay, M_UPDATE, M_ENABLE);
   }

//*****************************************************************************
// Draws the normals information of a 3D point cloud.
//*****************************************************************************
void DrawNormals(MIL_ID MilContainer, MIL_ID Mil3dDisplay)
   {
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);

   M3ddispControl(Mil3dDisplay, M_UPDATE, M_DISABLE);

   // Add normals to the pointcloud if they are missing.
   auto WereNormalsAdded = AddComponentNormalsIfMissing(MilContainer);
   if(WereNormalsAdded)
      {
      M3ddispControl(Mil3dDisplay, M_TITLE, MIL_TEXT("Normals (Calculated), Normals average orientation"));
      }
   else
      {
      M3ddispControl(Mil3dDisplay, M_TITLE, MIL_TEXT("Normals, Normals average orientation"));
      }

   auto MilGraphicsList = M3ddispInquire(Mil3dDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Display the point cloud and specify to use the normals component
   // when coloring the points of the point cloud.
   const auto PtCldLabel = M3ddispSelect(Mil3dDisplay, MilContainer, M_DEFAULT, M_DEFAULT);

   // Set all the annotations thickness.
   M3dgraControl(MilGraphicsList, M_DEFAULT_SETTINGS, M_THICKNESS, ANNOTATION_THICKNESS);

   // Color the point cloud.
   M3dgraControl(MilGraphicsList, PtCldLabel, M_COLOR_COMPONENT, M_COMPONENT_NORMALS_MIL);

   // Calculate the centroid of the point cloud.
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);
   M3dimStat(M_STAT_CONTEXT_PCA, MilContainer, MilStatResult, M_DEFAULT);
   Vec3 Centroid;
   Centroid.x = M3dimGetResult(MilStatResult, M_CENTROID_X, M_NULL);
   Centroid.y = M3dimGetResult(MilStatResult, M_CENTROID_Y, M_NULL);
   Centroid.z = M3dimGetResult(MilStatResult, M_CENTROID_Z, M_NULL);

   // Calculate the PCA of the normals.
   M3dimStat(M_STAT_CONTEXT_PCA_NORMALS, MilContainer, MilStatResult, M_DEFAULT);

   // Get the average of the normals and normalize the vector.
   Vec3 AverageDirection;
   AverageDirection.x = M3dimGetResult(MilStatResult, M_CENTROID_X, M_NULL);
   AverageDirection.y = M3dimGetResult(MilStatResult, M_CENTROID_Y, M_NULL);
   AverageDirection.z = M3dimGetResult(MilStatResult, M_CENTROID_Z, M_NULL);
   AverageDirection   = Normalize(AverageDirection);

   MosPrintf(MIL_TEXT("Average normal direction (Centroid of normals):\n"));
   MosPrintf(MIL_TEXT("X component: %.5f\n")  , AverageDirection.x);
   MosPrintf(MIL_TEXT("Y component: %.5f\n")  , AverageDirection.y);
   MosPrintf(MIL_TEXT("Z component: %.5f\n\n"), AverageDirection.z);

   // Get the average orientation.
   Vec3 AverageOrientation;
   std::vector<MIL_DOUBLE> PCAEigenvalues(3);
   AverageOrientation.x = M3dimGetResult(MilStatResult, M_PRINCIPAL_AXIS_1_X, M_NULL);
   AverageOrientation.y = M3dimGetResult(MilStatResult, M_PRINCIPAL_AXIS_1_Y, M_NULL);
   AverageOrientation.z = M3dimGetResult(MilStatResult, M_PRINCIPAL_AXIS_1_Z, M_NULL);

   MosPrintf(MIL_TEXT("Average normal orientation (PCA of normals):\n"));
   MosPrintf(MIL_TEXT("X component: %.5f\n"), AverageOrientation.x);
   MosPrintf(MIL_TEXT("Y component: %.5f\n"), AverageOrientation.y);
   MosPrintf(MIL_TEXT("Z component: %.5f\n\n"), AverageOrientation.z);

   M3dimGetResult(MilStatResult, M_EIGENVALUE_1, &PCAEigenvalues[0]);
   M3dimGetResult(MilStatResult, M_EIGENVALUE_2, &PCAEigenvalues[1]);
   M3dimGetResult(MilStatResult, M_EIGENVALUE_3, &PCAEigenvalues[2]);

   // Draw the average orientation of the vector into the 3D graphics list.
   M3dgraLine(MilGraphicsList,
              M_DEFAULT,
              M_POINT_AND_VECTOR,
              M_DEFAULT,
              Centroid.x,
              Centroid.y,
              Centroid.z,
              AverageOrientation.x,
              AverageOrientation.y,
              AverageOrientation.z,
              M_INFINITE,
              M_DEFAULT);

   M3ddispControl(Mil3dDisplay, M_UPDATE, M_ENABLE);
   }

//*****************************************************************************
// Calculates and displays the moments of a 3D point cloud.
//*****************************************************************************
void CalculateMoments(MIL_ID MilContainer)
   {
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);

   auto MilStatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilStatResult  = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3dimControl(MilStatContext, M_MOMENTS, M_ENABLE);
   M3dimStat(MilStatContext, MilContainer, MilStatResult, M_DEFAULT);

   MosPrintf(MIL_TEXT("Central X moment : %.5f\n"),   M3dimGetResult(MilStatResult, M_MOMENT_XYZ(1, 0, 0), M_NULL));
   MosPrintf(MIL_TEXT("Central Y moment : %.5f\n"),   M3dimGetResult(MilStatResult, M_MOMENT_XYZ(0, 1, 0), M_NULL));
   MosPrintf(MIL_TEXT("Central Z moment : %.5f\n\n"), M3dimGetResult(MilStatResult, M_MOMENT_XYZ(0, 0, 1), M_NULL));
   }

//*****************************************************************************
// Calculates and displays the surface variation stats of a 3D point cloud.
//*****************************************************************************
void CalculateSurfaceVariation(MIL_ID MilContainer)
   {
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);

   auto MilStatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilStatResult  = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3dimControl(MilStatContext, M_DISTANCE_TO_NEAREST_NEIGHBOR, M_ENABLE);
   M3dimControl(MilStatContext, M_SURFACE_VARIATION,            M_ENABLE);
   M3dimStat(MilStatContext, MilContainer, MilStatResult, M_DEFAULT);

   MosPrintf(MIL_TEXT("Surface variation min               : %.5f\n"),
             M3dimGetResult(MilStatResult, M_SURFACE_VARIATION_MIN    , M_NULL));
   MosPrintf(MIL_TEXT("Surface variation max               : %.5f\n"),
             M3dimGetResult(MilStatResult, M_SURFACE_VARIATION_MAX    , M_NULL));
   MosPrintf(MIL_TEXT("Surface variation average           : %.5f\n"),
             M3dimGetResult(MilStatResult, M_SURFACE_VARIATION_AVERAGE, M_NULL));
   MosPrintf(MIL_TEXT("Surface variation standard deviation: %.5f\n\n"),
             M3dimGetResult(MilStatResult, M_SURFACE_VARIATION_STDEV  , M_NULL));
   }

//*****************************************************************************
// Calculates and displays the nearest neighbor statistics of a 3D point cloud.
//*****************************************************************************
void CalculateNearestNeighbors(MIL_ID MilContainer)
   {
   MIL_ID MilSystem;
   MobjInquire(MilContainer, M_OWNER_SYSTEM, &MilSystem);

   auto MilStatContext = M3dimAlloc(MilSystem, M_STATISTICS_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   auto MilStatResult = M3dimAllocResult(MilSystem, M_STATISTICS_RESULT, M_DEFAULT, M_UNIQUE_ID);

   M3dimControl(MilStatContext, M_DISTANCE_TO_NEAREST_NEIGHBOR, M_ENABLE);
   M3dimStat(MilStatContext, MilContainer, MilStatResult, M_DEFAULT);

   MosPrintf(MIL_TEXT("Nearest neighbor min                : %.5f\n")  ,
             M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_MIN    , M_NULL));
   MosPrintf(MIL_TEXT("Nearest neighbor max                : %.5f\n")  ,
             M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_MAX    , M_NULL));
   MosPrintf(MIL_TEXT("Nearest neighbor average            : %.5f\n")  ,
             M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_AVERAGE, M_NULL));
   MosPrintf(MIL_TEXT("Nearest neighbor standard deviation : %.5f\n\n"),
             M3dimGetResult(MilStatResult, M_DISTANCE_TO_NEAREST_NEIGHBOR_STDEV  , M_NULL));
   }

//*****************************************************************************
// Prompts user for yes/no.
//*****************************************************************************
bool AskYesNo(MIL_CONST_TEXT_PTR QuestionString)
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

//*****************************************************************************
// Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
//*****************************************************************************
bool AddComponentNormalsIfMissing(MIL_ID MilContainer)
   {
   MIL_ID MilNormals =
      MbufInquireContainer(MilContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);

   if(MilNormals != M_NULL)
      return false;
   MIL_INT SizeX = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   if(SizeX < 50 || SizeY < 50)
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, M_DEFAULT);
   else
      M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, M_DEFAULT);
   return true;
   }

//****************************************************************************
// Checks for required files to run the example.
//****************************************************************************
void CheckForRequiredMILFile(const MIL_STRING& FileName)
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
// Simple functions for converting MIL constants to strings.
//*****************************************************************************
MIL_CONST_TEXT_PTR UnitString(MIL_INT Unit)
   {
   switch(Unit)
      {
      case M_MILLIMETERS: return MIL_TEXT("M_MILLIMETERS");
      case M_INCHES:      return MIL_TEXT("M_INCHES");
      case M_UNKNOWN:     return MIL_TEXT("M_UNKNOWN");
      default:            return MIL_TEXT("<unknown value>");
      }
   }

MIL_CONST_TEXT_PTR CSString(MIL_INT CS)
   {
   switch(CS)
      {
      case M_CARTESIAN:   return MIL_TEXT("M_CARTESIAN");
      case M_SPHERICAL:   return MIL_TEXT("M_SPHERICAL");
      case M_CYLINDRICAL: return MIL_TEXT("M_CYLINDRICAL");
      default:            return MIL_TEXT("<unknown value>");
      }
   }

MIL_CONST_TEXT_PTR RepresentationString(MIL_INT Representation)
   {
   switch(Representation)
      {
      case M_UNCALIBRATED_Z:                    return MIL_TEXT("M_UNCALIBRATED_Z");
      case M_CALIBRATED_XYZ:                    return MIL_TEXT("M_CALIBRATED_XYZ");
      case M_CALIBRATED_XYZ_UNORGANIZED:        return MIL_TEXT("M_CALIBRATED_XYZ_UNORGANIZED");
      case M_CALIBRATED_XZ_UNIFORM_Y:           return MIL_TEXT("M_CALIBRATED_XZ_UNIFORM_Y");
      case M_CALIBRATED_XZ_EXTERNAL_Y:          return MIL_TEXT("M_CALIBRATED_XZ_EXTERNAL_Y");
      case M_CALIBRATED_Z:                      return MIL_TEXT("M_CALIBRATED_Z");
      case M_CALIBRATED_Z_EXTERNAL_Y:           return MIL_TEXT("M_CALIBRATED_Z_EXTERNAL_Y");
      case M_CALIBRATED_Z_UNIFORM_XY:           return MIL_TEXT("M_CALIBRATED_Z_UNIFORM_XY");
      case M_CALIBRATED_Z_UNIFORM_X_EXTERNAL_Y: return MIL_TEXT("M_CALIBRATED_Z_UNIFORM_X_EXTERNAL_Y");
      case M_DISPARITY:                         return MIL_TEXT("M_DISPARITY");
      case M_DISPARITY_EXTERNAL_Y:              return MIL_TEXT("M_DISPARITY_EXTERNAL_Y");
      case M_DISPARITY_UNIFORM_Y:               return MIL_TEXT("M_DISPARITY_UNIFORM_Y");
      default:                                  return MIL_TEXT("<unknown value>");
      }
   }

MIL_CONST_TEXT_PTR BoolString(MIL_INT Bool)
   {
   switch(Bool)
      {
      case M_TRUE:  return MIL_TEXT("M_TRUE");
      case M_FALSE: return MIL_TEXT("M_FALSE");
      default:      return MIL_TEXT("<unknown value>");
      }
   }

MIL_CONST_TEXT_PTR BufTypeString(MIL_INT64 ObjType)
   {
   switch(ObjType)
      {
      case M_IMAGE: return MIL_TEXT("IMAGE");
      case M_ARRAY: return MIL_TEXT("ARRAY");
      default:      return MIL_TEXT("other");
      }
   }

MIL_CONST_TEXT_PTR ElemTypeString(MIL_INT64 ElemType)
   {
   switch(ElemType)
      {
      case M_UNSIGNED + 1:  return MIL_TEXT(" 1U");
      case M_UNSIGNED + 8:  return MIL_TEXT(" 8U");
      case M_UNSIGNED + 16: return MIL_TEXT("16U");
      case M_UNSIGNED + 32: return MIL_TEXT("32U");
      case M_UNSIGNED + 64: return MIL_TEXT("64U");
      case M_SIGNED + 8:    return MIL_TEXT(" 8S");
      case M_SIGNED + 16:   return MIL_TEXT("16S");
      case M_SIGNED + 32:   return MIL_TEXT("32S");
      case M_SIGNED + 64:   return MIL_TEXT("64S");
      case M_FLOAT + 32:    return MIL_TEXT("32F");
      case M_FLOAT + 64:    return MIL_TEXT("64F");
      default:              return MIL_TEXT("<unknown value>");
      }
   }
