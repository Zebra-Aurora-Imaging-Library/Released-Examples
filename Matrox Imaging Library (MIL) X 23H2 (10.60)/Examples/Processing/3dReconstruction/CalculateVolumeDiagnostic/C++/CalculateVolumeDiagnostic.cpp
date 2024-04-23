//***************************************************************************************/
//
// File name: CalculateVolumeDiagnostic.cpp
//
// Synopsis:  This example shows how to use the 3D Metrology module to calculate a volume,
//            and then diagnose the result.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <map>
#include "ExampleUtil.h"
#include "VolumeSourceInfo.h"
#include "ZoomDisplay.h"
#include "VolumeDisplay3dSelectionProcess.h"

// Source file specification.
const MIL_STRING EXAMPLE_PATH = MIL_STRING(M_IMAGE_PATH) + MIL_TEXT("CalculateVolumeDiagnostic/");
static const MIL_INT NB_SOURCE_DATA = 4;
static const SSourceDataInfo EXAMPLE_SOURCES[NB_SOURCE_DATA] =
   {
         {eSourceFile, MIL_TEXT("Depth-Depth"), MIL_TEXT(".mim"), MIL_TEXT(".mim"),    M_TOP_TILTED},
         {eSourceFile, MIL_TEXT("Depth-Plane"), MIL_TEXT(".mim"), MIL_TEXT(".m3dgeo"), M_TOP_TILTED},
         {eXYPlane,    MIL_TEXT("Mesh-Plane"),  MIL_TEXT(".ply"), MIL_TEXT(""),        M_TOP_TILTED},
         {eNone   ,    MIL_TEXT("Mesh"),        MIL_TEXT(".ply"), MIL_TEXT(""),        M_TOP_TILTED}
   };

// Utility structs.
struct SVolumeOutputType
   {
   MIL_INT Value;
   MIL_CONST_TEXT_PTR Name;
   };

// Utility maps.
static const std::map<MIL_TEXT_CHAR, SVolumeOutputType> VOLUME_OUTPUT_TYPE_MAP =
   {
         {MIL_TEXT('T'), {M_TOTAL,        MIL_TEXT("Total")}},
         {MIL_TEXT('U'), {M_UNDER,        MIL_TEXT("Under")}},
         {MIL_TEXT('A'), {M_ABOVE,        MIL_TEXT("Above")}},
         {MIL_TEXT('D'), {M_DIFFERENCE,   MIL_TEXT("Difference")}},
   };

// Functions declarations.
MIL_STRING CreateVolumeOutputKeyChoices();
MIL_INT ModifyDisplay(MIL_INT ExampleCase, SVolumeOutputType& VolumeOutputType);
MIL_UNIQUE_BUF_ID CreateSelectedSource(MIL_ID MilSource, bool IsSourceContainer);
MIL_UNIQUE_BUF_ID CreateSelectedReference(MIL_ID MilSelectedSource,
                                          MIL_ID MilVolumeResult,
                                          ReferenceType Reference,
                                          bool IsSourceContainer);
void SetupStatusDisplay(MIL_ID MilVolumeResult,
                        MIL_UNIQUE_BUF_ID& MilStatusImage,
                        MIL_UNIQUE_BUF_ID& MilIndexImage,
                        CVolume3dDisplaySelectionProcessing& SelectionProcessing,
                        CZoomDisplay& StatusDisplay,
                        MIL_INT SurfaceLabel,
                        bool IsSourceContainer);

// Constants.
static const MIL_TEXT_CHAR ESC_KEY = 27;

static const MIL_INT DISP_SX           = 320;
static const MIL_INT DISP_SY           = 240;
static const MIL_INT DISP_IMAGE_SX_MAX = 2 * DISP_SX;
static const MIL_INT DISP_IMAGE_SY_MAX = 2 * DISP_SY;
static const MIL_INT DISP_SPACING      = 30;
static const MIL_INT REF_DISP_Y        = DISP_SY + DISP_SPACING;
static const MIL_INT STATUS_DISP_Y     = 2 * REF_DISP_Y;
static const MIL_INT DISP3D_SIZE       = 3 * DISP_SY + 2 * DISP_SPACING;

static const MIL_DOUBLE CONTAINER_IMAGE_PIXEL_SIZE = 0.01;

static const MIL_INT NB_ELEMENT_DISP_PERFORMANCE_WARNING = 4096;

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("CalculateVolumeDiagnostic\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example shows how to use the 3D Metrology module to calculate a volume,\n")
             MIL_TEXT("and then diagnose the result.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Display, 3D Geometry, 3D Graphics, 3D Image Processing,\n")
             MIL_TEXT("3D Metrology, Buffer, Display, Graphics\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to start.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the diagnostic displays.
   auto MilSourceDisplay = AllocImageDisplay(M_DEFAULT_HOST, 0, 0, DISP_SX, DISP_SY, MIL_TEXT("Source"));
   auto MilRefDisplay = AllocImageDisplay(M_DEFAULT_HOST, 0, REF_DISP_Y, DISP_SX, DISP_SY, MIL_TEXT("Reference"));
   CZoomDisplay MilStatusDisplay(M_DEFAULT_HOST, 0, STATUS_DISP_Y, DISP_SX, DISP_SY, MIL_TEXT("Status"));

   auto MilDiagDisplay = Alloc3dDisplayId(M_DEFAULT_HOST, DISP_SX, 0, DISP3D_SIZE, DISP3D_SIZE,
                                          MIL_TEXT("DiagnosticDisplay"));

   // Get the diagnostic display graphics list.
   auto MilDiagGraList = M3ddispInquire(MilDiagDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   
   // Declare the data used by the example.
   MIL_UNIQUE_BUF_ID MilStatusImage;
   MIL_UNIQUE_BUF_ID MilIndexImage;
   MIL_UNIQUE_BUF_ID MilSelectedReference;
   MIL_UNIQUE_BUF_ID MilSelectedSource;
   std::unique_ptr<SMilSource> pMilExampleData;

   // Allocate the 3D metrology context and result.
   auto MilVolumeContext = M3dmetAlloc(M_DEFAULT_HOST, M_VOLUME_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dmetControl(MilVolumeContext, M_SAVE_VOLUME_INFO, M_TRUE);
   auto MilVolumeResult = M3dmetAllocResult(M_DEFAULT_HOST, M_CALCULATE_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the processing to be done when the selection changes in the status image.
   CVolume3dDisplaySelectionProcessing SelectionProcessing(M_DEFAULT_HOST, MilVolumeResult, MilDiagDisplay);

   PrintHeader();     
   
   // The default is difference of depth-plane.
   bool SourceChanged = true;
   MIL_INT SourceIndex = 1;
   auto VolumeOutputMode = VOLUME_OUTPUT_TYPE_MAP.at(MIL_TEXT('D'));
   MIL_STRING VolumeOutputKeyChoices = CreateVolumeOutputKeyChoices();

   // Use fast transparency sort mode.
   M3ddispControl(MilDiagDisplay, M_TRANSPARENCY_SORT_MODE, M_FAST);

   do
      {
      const auto& SourceData = EXAMPLE_SOURCES[SourceIndex];

      // Restore the source and reference data of the example.
      pMilExampleData = SourceData.MakeMilSource(M_DEFAULT_HOST, EXAMPLE_PATH);
      MIL_ID MilSource = pMilExampleData->GetSource();
      MIL_ID MilReference = pMilExampleData->GetReference();
      bool IsSourceContainer = MobjInquire(MilSource, M_OBJECT_TYPE, M_NULL) == M_CONTAINER;

      // Deselect the display.
      MilStatusDisplay.Deselect();

      // Calculate the volume.
      if(SourceChanged)
         M3dmetVolumeEx(MilVolumeContext, MilSource, MilReference, MilVolumeResult, M_DEFAULT);
      
      // Set the volume output type.
      M3dmetControl(MilVolumeResult, M_VOLUME_OUTPUT_MODE, VolumeOutputMode.Value);

      if(SourceChanged)
         {
         // Set up the source display.
         MilSelectedSource = CreateSelectedSource(MilSource, IsSourceContainer);
         SelectImageOnDisplay(MilSourceDisplay, MilSelectedSource);

         // Set up the reference display.
         MilSelectedReference = CreateSelectedReference(MilSelectedSource, MilVolumeResult,
                                                        SourceData.Reference, IsSourceContainer);
         SelectImageOnDisplay(MilRefDisplay, MilSelectedReference);
         }

      // Draw the diagnostic annotations.
      M3ddispControl(MilDiagDisplay, M_UPDATE, M_DISABLE);

      M3dgraRemove(MilDiagGraList, M_ALL, M_DEFAULT);

      // Draw the surface. Only draw the source surface for a mesh with a reference plane.
      MIL_UNIQUE_3DMET_ID Mil3dmetDrawContext = M3dmetAlloc(M_DEFAULT_HOST, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
      M3dmetControlDraw(Mil3dmetDrawContext, M_ALL, M_ACTIVE, M_ENABLE);
      M3dmetControlDraw(Mil3dmetDrawContext, M_DRAW_VOLUME_ELEMENTS, M_ACTIVE, M_DISABLE);
      MIL_INT SurfaceAppearance = SourceData.IsReferencePlane() ? M_SURFACE_SOURCE : M_SURFACE;
      M3dmetControlDraw(Mil3dmetDrawContext, M_ALL, M_VOLUME_ELEMENT_APPEARANCE, SurfaceAppearance);
      M3dmetControlDraw(Mil3dmetDrawContext, M_ALL, M_OPACITY, UNSELECTED_SURFACE_OPACITY);
      auto SurfaceLabel = M3dmetDraw3d(Mil3dmetDrawContext, MilVolumeResult, MilDiagGraList, M_ROOT_NODE, M_DEFAULT);      

      MIL_INT NbElements = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_VOLUME_NB_ELEMENTS, M_NULL);
      bool DrawTransparentVolumeElements = NbElements < NB_ELEMENT_DISP_PERFORMANCE_WARNING;
      if(DrawTransparentVolumeElements)
         {
         // Draw the volume but with a very low opacity.
         M3dmetControlDraw(Mil3dmetDrawContext, M_ALL, M_VOLUME_ELEMENT_APPEARANCE, M_VOLUME);
         M3dmetControlDraw(Mil3dmetDrawContext, M_ALL, M_OPACITY, 1);
         M3dmetDraw3d(Mil3dmetDrawContext, MilVolumeResult, MilDiagGraList, M_ROOT_NODE, M_DEFAULT);
         }      

      // Draw the reference plane.
      MIL_INT64 ReferenceLabel = 0;
      if(SourceData.IsReference3dGeo())
         ReferenceLabel = M3dgeoDraw3d(M_DEFAULT, MilReference, MilDiagGraList, M_DEFAULT, M_DEFAULT);
      else if(SourceData.Reference == eXYPlane)
         ReferenceLabel = M3dgraPlane(MilDiagGraList, M_DEFAULT, M_COEFFICIENTS, 0, 0, 1, 0,
                                      M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
      if(ReferenceLabel)
         {
         M3dgraControl(MilDiagGraList, ReferenceLabel, M_COLOR, M_COLOR_BLUE);
         M3dgraControl(MilDiagGraList, ReferenceLabel, M_OPACITY, 40);
         }

      // Open the display and set the view.
      M3ddispSelect(MilDiagDisplay, M_NULL, M_OPEN, M_DEFAULT);
      M3ddispSetView(MilDiagDisplay, M_AUTO, SourceData.DefaultView, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      M3ddispControl(MilDiagDisplay, M_UPDATE, M_ENABLE);

      // Set up the status display.
      SetupStatusDisplay(MilVolumeResult, MilStatusImage, MilIndexImage,
                         SelectionProcessing, MilStatusDisplay,
                         SurfaceLabel, IsSourceContainer);

      // Print the diagnostic display controls.
      MosPrintf(MIL_TEXT("DIAGNOSTIC DISPLAY CONTROLS\n"));
      MosPrintf(MIL_TEXT("---------------------------------\n"));
      MosPrintf(MIL_TEXT("Choose example source      (1-%d)\n"), std::size(EXAMPLE_SOURCES));
      for(MIL_INT c = 0; c < NB_SOURCE_DATA; c++)
         MosPrintf(MIL_TEXT("   (%d) %s\n"), c+1, EXAMPLE_SOURCES[c].SourceName.c_str());
      MosPrintf(MIL_TEXT("Choose volume output mode  (%s)  \n"), VolumeOutputKeyChoices.c_str());
      for(const auto& volumeOutput : VOLUME_OUTPUT_TYPE_MAP)
         MosPrintf(MIL_TEXT("   (%c) %s\n"), volumeOutput.first, volumeOutput.second.Name);
      MosPrintf(MIL_TEXT("Exit                       (esc)\n\n"));

      MosPrintf(MIL_TEXT("Currently displaying...\n"));
      MosPrintf(MIL_TEXT("Example source       = %s\n"), SourceData.SourceName.c_str());                
      MosPrintf(MIL_TEXT("Volume output mode   = %s\n"), VolumeOutputMode.Name);
      MIL_DOUBLE Volume = M3dmetGetResult(MilVolumeResult, M_VOLUME, M_NULL);
      MosPrintf(MIL_TEXT("Volume               = %.3f\n"), Volume);
      MosPrintf(MIL_TEXT("Nb elements          = %d\n"), NbElements);
      MIL_INT NbPosElements = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_VOLUME_NB_POSITIVE_ELEMENTS, M_NULL);
      MosPrintf(MIL_TEXT("Nb positive elements = %d\n"), NbPosElements);
      MIL_INT NbNegElements = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_VOLUME_NB_NEGATIVE_ELEMENTS, M_NULL);
      MosPrintf(MIL_TEXT("Nb negative elements = %d\n"), NbNegElements);
      MIL_INT NbUnusedElements = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_VOLUME_NB_UNUSED_ELEMENTS, M_NULL);
      MosPrintf(MIL_TEXT("Nb unused elements   = %d\n\n"), NbUnusedElements);

      if(!DrawTransparentVolumeElements)
         {
         MosPrintf(MIL_TEXT("Transparent volume elements were not drawn.\n"));
         MosPrintf(MIL_TEXT("When M_VOLUME_ELEMENT_APPEARANCE is set to M_VOLUME, a powerful\n"));
         MosPrintf(MIL_TEXT("GPU is required to draw a large number of transparent volume elements.\n"));
         MosPrintf(MIL_TEXT("To draw transparent volume elements, modify the\n"));
         MosPrintf(MIL_TEXT("NB_ELEMENT_DISP_PERFORMANCE_WARNING example setting.\n\n"));
         }

      MosPrintf(MIL_TEXT("Hover over the status window to see a corresponding volume element\n"));
      MosPrintf(MIL_TEXT("in the diagnostic display.\n"));
      if(IsSourceContainer && SourceData.Reference != eNone)
         MosPrintf(MIL_TEXT("Darker colors indicate an element that was spliced by the reference.\n"));

      MosPrintf(MIL_TEXT("   (Green)  Positive\n"));
      MosPrintf(MIL_TEXT("   (Red)    Negative\n"));
      MosPrintf(MIL_TEXT("   (Yellow) Positive and Negative\n"));
      MosPrintf(MIL_TEXT("   (Black)  Unused\n\n"));
      
      MIL_INT NewSourceIndex = ModifyDisplay(SourceIndex, VolumeOutputMode);
      SourceChanged = SourceIndex != NewSourceIndex;
      SourceIndex = NewSourceIndex;

      }while(SourceIndex >= 0);
  
   return 0;
   }

//****************************************************************************
// Creates the source image that will be selected on the source display.
//****************************************************************************
MIL_UNIQUE_BUF_ID CreateSelectedSource(MIL_ID MilSource, bool IsSourceContainer)
   {
   MIL_UNIQUE_BUF_ID MilSelectedSource;

   // Create the selected source image. If a container, create a depth map image.  
   if(IsSourceContainer)
      MilSelectedSource = GenerateDepthMap(MilSource,
                                           CONTAINER_IMAGE_PIXEL_SIZE,
                                           DISP_IMAGE_SX_MAX,
                                           DISP_IMAGE_SY_MAX);
   else
      MilSelectedSource = MbufClone(MilSource, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                                    M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
   return MilSelectedSource;
   }

//****************************************************************************
// Creates the reference image that will be selected on the reference display.
//****************************************************************************
MIL_UNIQUE_BUF_ID CreateSelectedReference(MIL_ID MilSelectedSource,
                                         MIL_ID MilVolumeResult,
                                         ReferenceType Reference,
                                         bool IsSourceContainer)
   {
   MIL_UNIQUE_BUF_ID MilSelectedReference;
   if(Reference == eNone)
      return MilSelectedReference;
      
   if(IsSourceContainer)
      {
      MilSelectedReference = MbufClone(MilSelectedSource, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                                       M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);
      auto MilRefContainer = MbufAllocContainer(M_DEFAULT_HOST, M_PROC, M_DEFAULT, M_UNIQUE_ID);
      M3dmetCopyResult(MilVolumeResult, MilRefContainer, M_VOLUME_REFERENCE_CONTAINER, M_DEFAULT);
      M3dimProject(MilRefContainer, MilSelectedReference, M_NULL, M_MESH_BASED, M_MAX_Z, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      MilSelectedReference = MbufClone(MilSelectedSource, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                                       M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
      M3dmetCopyResult(MilVolumeResult, MilSelectedReference, M_VOLUME_REFERENCE_DEPTH_MAP, M_DEFAULT);
      }

   return MilSelectedReference;   
   }

//****************************************************************************
// Sets up the status display.
//****************************************************************************
void SetupStatusDisplay(MIL_ID MilVolumeResult,
                        MIL_UNIQUE_BUF_ID& MilStatusImage,
                        MIL_UNIQUE_BUF_ID& MilIndexImage,
                        CVolume3dDisplaySelectionProcessing& SelectionProcessing,
                        CZoomDisplay& StatusDisplay,
                        MIL_INT SurfaceLabel,
                        bool IsSourceContainer)
   {
   // Create the status and index images.
   MIL_INT SizeX = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_RESULT_ELEMENT_IMAGE_SIZE_X, M_NULL);
   MIL_INT SizeY = (MIL_INT)M3dmetGetResult(MilVolumeResult, M_RESULT_ELEMENT_IMAGE_SIZE_Y, M_NULL);
   MilStatusImage = MbufAlloc2d(M_DEFAULT_HOST, SizeX, SizeY, 8 + M_UNSIGNED,
                                M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dmetCopyResult(MilVolumeResult, MilStatusImage, M_VOLUME_ELEMENT_STATUS_IMAGE, M_DEFAULT);
   MilIndexImage = MbufAlloc2d(M_DEFAULT_HOST, SizeX, SizeY, 32 + M_UNSIGNED,
                               M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   M3dmetCopyResult(MilVolumeResult, MilIndexImage, M_VOLUME_ELEMENT_INDEX_IMAGE, M_DEFAULT);

   // For source containers, make the status and index images square for easier displayability.
   MIL_STRING StatusDisplayTitle = MIL_TEXT("Status");
   if(IsSourceContainer)
      {
      MIL_INT SquareSizeX = (MIL_INT)sqrt(SizeX) + 1;
      MIL_INT SquareSizeY = SizeX / SquareSizeX + 1;

      MilStatusImage = Create2dImageFrom1d<MIL_UINT8>(MilStatusImage, SquareSizeX, SquareSizeY);
      MilIndexImage = Create2dImageFrom1d<MIL_UINT32>(MilIndexImage, SquareSizeX, SquareSizeY);

      StatusDisplayTitle += MIL_TEXT(" (Size = ") + M_TO_STRING(SizeX) + MIL_TEXT("x") +
                            M_TO_STRING(SizeY) + MIL_TEXT(")");
      
      }
   SelectionProcessing.InitSelection(MilIndexImage, IsSourceContainer? SurfaceLabel: 0);
   StatusDisplay.Select(MilStatusImage, &SelectionProcessing);
   MdispControl(StatusDisplay, M_TITLE, StatusDisplayTitle);

   // Put a LUT on the status display.
   auto MilDisplayLut = MbufAllocColor(M_DEFAULT_HOST, 3, 256, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
   MbufPutColor2d(MilDisplayLut, M_PACKED + M_RGB24, M_ALL_BANDS, 0, 0, 8, 1, STATUS_LUT);
   MdispLut(StatusDisplay, MilDisplayLut);
   }

//****************************************************************************
// Modifies the drawings in the 3D display according to the key pressed.
//****************************************************************************
MIL_INT ModifyDisplay(MIL_INT ExampleCase, SVolumeOutputType& VolumeOutputType)
   {
   while(1)
      {
      auto Key = (MIL_TEXT_CHAR)std::toupper((int)MosGetch());

      if(Key == ESC_KEY)
         return -1;

      if(Key >= MIL_TEXT('1') && Key < MIL_TEXT('1') + (MIL_TEXT_CHAR)std::size(EXAMPLE_SOURCES))
         {
         MIL_INT NewExampleCase = Key - MIL_TEXT('1');
         return NewExampleCase;
         }
      else if(VOLUME_OUTPUT_TYPE_MAP.count(Key))
         {
         VolumeOutputType = VOLUME_OUTPUT_TYPE_MAP.at(Key);
         return ExampleCase;
         }
      }
   }

//****************************************************************************
// Create the displayed string that contains all the volume mode key choices.
//****************************************************************************
MIL_STRING CreateVolumeOutputKeyChoices()
   {
   MIL_STRING VolumeOutputKeyChoices;
   MIL_STRING Separator = MIL_TEXT("");
   for(const auto& volumeOutput : VOLUME_OUTPUT_TYPE_MAP)
      {
      VolumeOutputKeyChoices += Separator + volumeOutput.first;
      Separator = MIL_TEXT(", ");
      }
   return VolumeOutputKeyChoices;
   }


