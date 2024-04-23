//***************************************************************************************
// 
// File name: M3dgra.cpp  
//
// Synopsis: This program contains an example of how to use 3d graphics in MIL.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************

#include <mil.h>
#include <math.h>

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("M3dgra\n\n"));
   
   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates the usage of 3D graphics in MIL.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: application, system, buffer, 3D display, 3D graphics, 3D Geometry, 3D Image Processing.\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING POINT_CLOUD_FILE = M_IMAGE_PATH MIL_TEXT("M3dgra/MaskOrganized.mbufc");
static const MIL_STRING GLASSES_FILE = M_IMAGE_PATH MIL_TEXT("M3dgra/Glasses.png");
static const MIL_STRING LOGO_FILE = M_IMAGE_PATH MIL_TEXT("imaginglogo.mim");

static const MIL_INT FADE_DELAY_MSEC      = 750;
static const MIL_INT CONTROL_DELAY_MSEC   = 2000;
static const MIL_INT CONTROL_GRANULARITY  = 20;

//****************************************************************************
// Function Declaration.
//****************************************************************************
bool                 CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_STRING           GetGraphicTypeString(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel);
void                 PrintGraphicListFlat(MIL_ID MilGraphicList3d);
void                 PrintGraphicListTree(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel = M_ROOT_NODE, const MIL_STRING& Prefix = MIL_TEXT("-"));
void                 FadeIn(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel, MIL_INT Duration);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   // Allocate the MIL application.
   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
 
   // Check for required example files.
   if(!CheckForRequiredMILFile(POINT_CLOUD_FILE))
      {
      return -1;
      }

   MIL_UNIQUE_SYS_ID    MilSystem ;       // System identifier.
   MIL_UNIQUE_3DDISP_ID MilDisplay3d;     // 3D Mil Display.
   MIL_UNIQUE_BUF_ID    MilContainerId;   // Point cloud to display.
   MIL_UNIQUE_BUF_ID    MilTextureId;     // Polygon texture.

   // Allocate MIL objects. 
   MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MilDisplay3d = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // Make sure we meet the minimum requirements for the 3d display.
   if(!MilDisplay3d)
      {
      MosPrintf(MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to end.\n"));
      MosGetch();
      return 0;
      }

   // Show the display.
   MIL_ID MilGraphicList3d = (MIL_ID) M3ddispInquire(MilDisplay3d, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3ddispSetView(MilDisplay3d, M_VIEW_ORIENTATION, -2, -1.1, -1, M_DEFAULT);
   M3ddispSetView(MilDisplay3d, M_UP_VECTOR, 0, 0, 1, M_DEFAULT);
   M3ddispSelect(MilDisplay3d, M_NULL, M_OPEN, M_DEFAULT);


   // Draw an axis and a grid.
   MosPrintf(MIL_TEXT("The 3d display can show many point clouds at the same time.\n")
             MIL_TEXT("It can also show no point cloud and only the contents of a 3D graphics list.\n")
             MIL_TEXT("Here, it shows an axis and a grid.\n"));

   MIL_DOUBLE AxisLength = 200;
   MIL_INT64 AxisLabel = M3dgraAxis(MilGraphicList3d, M_ROOT_NODE, M_DEFAULT, AxisLength, M_NULL, M_DEFAULT, M_DEFAULT);

   MIL_UNIQUE_3DGEO_ID Matrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, AxisLength * 0.4, AxisLength * 0.4, 0, M_DEFAULT, M_DEFAULT);
   MIL_INT64 GridLabel = M3dgraGrid(MilGraphicList3d, AxisLabel, M_SIZE_AND_SPACING, Matrix, AxisLength * 0.8, AxisLength * 0.8, 16, 16, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, GridLabel, M_FILL_COLOR, M_COLOR_WHITE);
   M3dgraControl(MilGraphicList3d, GridLabel, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(MilGraphicList3d, GridLabel, M_OPACITY, 30);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Restore and display the point cloud.
   MilContainerId = MbufRestore(POINT_CLOUD_FILE, MilSystem, M_UNIQUE_ID);
   MIL_INT64 ContainerLabel = M3dgraAdd(MilGraphicList3d, AxisLabel, MilContainerId, M_DEFAULT);

   // Set various color modes.
   MosPrintf(MIL_TEXT("A point cloud has been added to the display.\n")
             MIL_TEXT("By default, point clouds are colored using the reflectance or intensity component.\n")
             MIL_TEXT("However, you can use any band(s) of any component for the color, and optionally apply a LUT.\n")
             MIL_TEXT("Press <Enter> to view the coloring options.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Range:                     The XYZ values are rescaled to RGB.\n"));
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   MosGetch();

   MosPrintf(MIL_TEXT("Range 3rd band with a LUT: Highlights elevation differences (Z).\n"));
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT_BAND, 2);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   MosGetch();

   MosPrintf(MIL_TEXT("Normals:                   Highlights details.\n"));
   M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainerId, MilContainerId, M_DEFAULT);
   M3ddispControl(MilDisplay3d, M_UPDATE, M_DISABLE);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_USE_LUT, M_FALSE);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT_BAND, M_ALL_BANDS);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT, M_COMPONENT_NORMALS_MIL);
   M3ddispControl(MilDisplay3d, M_UPDATE, M_ENABLE);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   MosGetch();

   MosPrintf(MIL_TEXT("Solid color:               Differentiates between multiple point clouds.\n"));
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR, M_COLOR_BLUE);
   M3dgraControl(MilGraphicList3d, ContainerLabel, M_COLOR_COMPONENT, M_NULL);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   MosGetch();

   // Add a mesh.
   MosPrintf(MIL_TEXT("Solid color (with a mesh): Similar to solid color while still showing details.\n"));
   M3dimMesh(M_MESH_CONTEXT_ORGANIZED, MilContainerId, MilContainerId, M_DEFAULT);
   MosPrintf(MIL_TEXT("<Enter to continue>.\n\n"));
   MosGetch();

   // Restore the matrox logo and draw it on a polygon.
   MosPrintf(MIL_TEXT("2D images can be displayed in the 3D graphics list via textured polygons.\n"));

   M3dgraRemove(MilGraphicList3d, ContainerLabel, M_DEFAULT);
   MilTextureId = MbufRestore(LOGO_FILE, MilSystem, M_UNIQUE_ID);
   MIL_DOUBLE PolygonCenterX = 100;
   MIL_DOUBLE PolygonCenterY = 100;
   MIL_INT PolygonHalfSizeX = MbufInquire(MilTextureId, M_SIZE_X, M_NULL) / 5;
   MIL_INT PolygonHalfSizeY = MbufInquire(MilTextureId, M_SIZE_Y, M_NULL) / 5;
   MIL_DOUBLE PolygonX[] = { PolygonCenterX - PolygonHalfSizeY, PolygonCenterX + PolygonHalfSizeY, PolygonCenterX + PolygonHalfSizeY, PolygonCenterX - PolygonHalfSizeY };
   MIL_DOUBLE PolygonY[] = { PolygonCenterY - PolygonHalfSizeX, PolygonCenterY - PolygonHalfSizeX, PolygonCenterY + PolygonHalfSizeX, PolygonCenterY + PolygonHalfSizeX };
   MIL_DOUBLE PolygonZ[] = { 30, 30, 30, 30 };
   MIL_INT64 PolygonLabel = M3dgraPolygon(MilGraphicList3d, AxisLabel, M_DEFAULT, 4, PolygonX, PolygonY, PolygonZ, M_NULL, M_NULL, MilTextureId, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, PolygonLabel, M_SHADING, M_NONE);
  
   // Draw other graphics.
   MosPrintf(MIL_TEXT("Press <Enter> to show other graphic primitives.\n\n"));
   MosGetch();
   M3dgraRemove(MilGraphicList3d, PolygonLabel, M_DEFAULT);
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, 100, 100, 0, M_DEFAULT, M_DEFAULT);
   MIL_INT64 NodeLabel = M3dgraNode(MilGraphicList3d, AxisLabel, Matrix, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_OPACITY, 0);

   // Plane.
   MIL_INT64 PlaneLabel = M3dgraPlane(MilGraphicList3d, NodeLabel, M_POINT_AND_NORMAL, 0, 0, 10, 0, 0, 1, M_DEFAULT, M_DEFAULT, M_DEFAULT, 70, M_DEFAULT);
   FadeIn(MilGraphicList3d, PlaneLabel, FADE_DELAY_MSEC);

   // Box.
   MIL_INT64 BoxLabel = M3dgraBox(MilGraphicList3d, NodeLabel, M_CENTER_AND_DIMENSION, 0, 0, 40, 40, 40, 60, M_DEFAULT, M_DEFAULT);
   FadeIn(MilGraphicList3d, BoxLabel, FADE_DELAY_MSEC);

   // Cylinder.
   MIL_INT64 CylinderLabel = M3dgraCylinder(MilGraphicList3d, NodeLabel, M_TWO_POINTS, 0, 0, 70, 0, 0, 120, 20, M_DEFAULT, M_DEFAULT);
   FadeIn(MilGraphicList3d, CylinderLabel, FADE_DELAY_MSEC);

   // Sphere.
   MIL_INT64 SphereLabel = M3dgraSphere(MilGraphicList3d, NodeLabel, 0, 0, 140, 20, M_DEFAULT);
   FadeIn(MilGraphicList3d, SphereLabel, FADE_DELAY_MSEC);

   // Line.
   MIL_INT64 LineLabel = M3dgraLine(MilGraphicList3d, NodeLabel, M_TWO_POINTS, M_DEFAULT, 0, -20, 110, 0, -40, 70, M_DEFAULT, M_DEFAULT);
   FadeIn(MilGraphicList3d, LineLabel, FADE_DELAY_MSEC);

   // Arc.
   MIL_INT64 ArcLabel = M3dgraArc(MilGraphicList3d, NodeLabel, M_THREE_POINTS, M_DEFAULT, 0, 20, 110, 0, 40, 130, 0, 40, 150, M_DEFAULT, M_DEFAULT);
   FadeIn(MilGraphicList3d, ArcLabel, FADE_DELAY_MSEC);

   // Dots.
   MIL_DOUBLE DotsX[] = {  18,  18 };
   MIL_DOUBLE DotsY[] = { -10,  10 };
   MIL_DOUBLE DotsZ[] = { 145, 145 };
   MIL_INT64 DotsLabel = M3dgraDots(MilGraphicList3d, NodeLabel, 2, DotsX, DotsY, DotsZ, M_NULL, M_NULL, M_NULL, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, DotsLabel, M_THICKNESS, 3);
   FadeIn(MilGraphicList3d, DotsLabel, FADE_DELAY_MSEC);

   // Polygon.
   MilTextureId = MbufImport(GLASSES_FILE, M_PNG, M_RESTORE, MilSystem, M_UNIQUE_ID);
   MIL_DOUBLE GlassesX[] = {  20,  20,  20,  20 };
   MIL_DOUBLE GlassesY[] = { -18, -18,  18,  18 };
   MIL_DOUBLE GlassesZ[] = { 153, 141.5, 141.5, 153 };
   MIL_INT64 GlassesLabel = M3dgraPolygon(MilGraphicList3d, NodeLabel, M_DEFAULT, 4, GlassesX, GlassesY, GlassesZ, M_NULL, M_NULL, MilTextureId, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, GlassesLabel, M_KEYING_COLOR, M_COLOR_WHITE);
   FadeIn(MilGraphicList3d, GlassesLabel, FADE_DELAY_MSEC);

   // Text.
   M3dgeoMatrixSetWithAxes(Matrix, M_XY_AXES, 0, 0, 165, 0, 1, 0, 0, 0, 1, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_FONT_SIZE, 15);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_TEXT_ALIGN_VERTICAL, M_BOTTOM);
   MIL_INT64 TextLabel = M3dgraText(MilGraphicList3d, NodeLabel, MIL_TEXT("Welcome to MIL!"), Matrix, M_DEFAULT, M_DEFAULT);
   FadeIn(MilGraphicList3d, TextLabel, FADE_DELAY_MSEC);
   MosPrintf(MIL_TEXT("\n"));

   M3dgraControl(MilGraphicList3d, M_DEFAULT_SETTINGS, M_OPACITY, 100);

   // Print the contents of the 3D graphics list.
   MosPrintf(MIL_TEXT("The contents of the graphics list can be inquired either in a flat list or recursively.\n")
             MIL_TEXT("Press <Enter> to view the graphics in a flat list with their absolute position.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Graphic type          Position X      Position Y      Position Z\n")
             MIL_TEXT("-----------------------------------------------------------------\n"));
   PrintGraphicListFlat(MilGraphicList3d);

   MosPrintf(MIL_TEXT("\nPress <Enter> to view the graphics in a tree and their position relative to their parent.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Graphic type          Position X      Position Y      Position Z\n")
             MIL_TEXT("-----------------------------------------------------------------\n"));
   PrintGraphicListTree(MilGraphicList3d);

   // Perform various controls,
   MosPrintf(MIL_TEXT("\nThe tree structure makes controlling groups of graphics easy.\n")
             MIL_TEXT("Here, the character's graphics are controlled all at once via their parent node.\n")
             MIL_TEXT("Press <Enter> see various controls.\n\n"));
   MosGetch();

   // Color.
   MosPrintf(MIL_TEXT("Color:      Doesn't affect textured polygons.\n"));
   MIL_UNIQUE_BUF_ID ColorLut = MbufAllocColor(MilSystem, 3, CONTROL_GRANULARITY, 1, M_UNSIGNED + 8, M_LUT, M_UNIQUE_ID);
   MgenLutFunction(ColorLut, M_COLORMAP_HUE, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      MIL_UINT8 Color[3];
      MbufGet1d(ColorLut, i % CONTROL_GRANULARITY, 1, Color);
      M3dgraControl(MilGraphicList3d, NodeLabel, M_COLOR + M_RECURSIVE, M_RGB888(Color[0], Color[1], Color[2]));
      MosSleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
      }
   M3dgraControl(MilGraphicList3d, NodeLabel, M_COLOR + M_RECURSIVE, M_COLOR_WHITE);   
   MosGetch();

   // Opacity.
   MosPrintf(MIL_TEXT("Opacity:    Graphics can be from fully opaque to fully transparent.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, NodeLabel, M_OPACITY + M_RECURSIVE, 50 + 50 * sin(3.1415 * 2 * i / CONTROL_GRANULARITY));
      MosSleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
      }
   M3dgraControl(MilGraphicList3d, NodeLabel, M_OPACITY + M_RECURSIVE, 100);   
   MosGetch();

   // Resolution.
   MosPrintf(MIL_TEXT("Resolution: Controls how fine the mesh is for cylinders, spheres and arcs.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, NodeLabel, M_GRAPHIC_RESOLUTION + M_RECURSIVE, 3 + i % CONTROL_GRANULARITY);
      MosSleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
      }
   M3dgraControl(MilGraphicList3d, NodeLabel, M_GRAPHIC_RESOLUTION + M_RECURSIVE, 16);   
   MosGetch();

   // Shading.
   MosPrintf(MIL_TEXT("Shading:    Graphics can choose between flat, Gouraud, Phong or no shading at all.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   const MIL_INT SHADINGS[] = { M_NONE, M_FLAT, M_GOURAUD, M_PHONG };
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, NodeLabel, M_SHADING + M_RECURSIVE, SHADINGS[i % 4]);
      MosSleep(CONTROL_DELAY_MSEC / 4);
      }   
   MosGetch();
   M3dgraControl(MilGraphicList3d, NodeLabel, M_SHADING + M_RECURSIVE, M_GOURAUD);

   // Thickness.
   MosPrintf(MIL_TEXT("Thickness:  Controls how thick lines and points look.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, NodeLabel, M_THICKNESS + M_RECURSIVE, 1 + i % CONTROL_GRANULARITY);
      MosSleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
      }
   M3dgraControl(MilGraphicList3d, NodeLabel, M_THICKNESS + M_RECURSIVE, 1);   
   MosGetch();

   // Movement.
   MosPrintf(MIL_TEXT("Movement:   Graphics can be moved with rigid transformations.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, -100, -100, 0, M_DEFAULT, M_DEFAULT);
   M3dgeoMatrixSetTransform(Matrix, M_ROTATION_Z, 90.0 / CONTROL_GRANULARITY, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, 100, 100, 0, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   
   while(!MosKbhit())
      {
      M3dgraCopy(Matrix, M_DEFAULT, MilGraphicList3d, NodeLabel, M_TRANSFORMATION_MATRIX + M_COMPOSE_WITH_CURRENT, M_DEFAULT);
      MosSleep(CONTROL_DELAY_MSEC / CONTROL_GRANULARITY);
      }
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, 100, 100, 0, M_DEFAULT, M_DEFAULT);
   M3dgraCopy(Matrix, M_DEFAULT, MilGraphicList3d, NodeLabel, M_TRANSFORMATION_MATRIX, M_DEFAULT);
   MosGetch();

   // Copy.
   MosPrintf(MIL_TEXT("Copy:       Graphics can be copied across the same or different graphics lists.\n"));
   MIL_INT64 CopyLabel = M3dgraCopy(MilGraphicList3d, NodeLabel, MilGraphicList3d, AxisLabel, M_GRAPHIC + M_RECURSIVE, M_DEFAULT);
   M3dgeoMatrixSetTransform(Matrix, M_TRANSLATION, 100, 250, 0, M_DEFAULT, M_DEFAULT);
   M3dgraCopy(Matrix, M_DEFAULT, MilGraphicList3d, CopyLabel, M_TRANSFORMATION_MATRIX, M_DEFAULT);
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   MosGetch();

   // Visibility.
   MosPrintf(MIL_TEXT("Visibility: Unneeded graphics can be hidden without deleting them.\n"));
   MosPrintf(MIL_TEXT("<Enter to continue>.\r"));
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, NodeLabel, M_VISIBLE + M_RECURSIVE, (i + 1) % 4 < 2);
      M3dgraControl(MilGraphicList3d, CopyLabel, M_VISIBLE + M_RECURSIVE, i % 4 < 2);
      MosSleep(CONTROL_DELAY_MSEC / 4);
      }
   M3dgraControl(MilGraphicList3d, NodeLabel, M_VISIBLE + M_RECURSIVE, M_TRUE);
   M3dgraControl(MilGraphicList3d, CopyLabel, M_VISIBLE + M_RECURSIVE, M_FALSE);
   MosGetch();

   // Inquire and draw the bounding box.
   MosPrintf(MIL_TEXT("                    \n")
             MIL_TEXT("It may be useful to know the bounding box of the 3D graphics list.\n"));

   MIL_UNIQUE_3DGEO_ID MilBoxGeometry = M3dgeoAlloc(MilSystem, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   M3dgraCopy(MilGraphicList3d, M_LIST, MilBoxGeometry, M_DEFAULT, M_BOUNDING_BOX, M_DEFAULT);
   MIL_INT64 BoundingBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBoxGeometry, MilGraphicList3d, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_OPACITY, 30);
   M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_THICKNESS, 3);
   M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_FILL_COLOR, M_COLOR_WHITE);
   M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_COLOR, M_COLOR_BLACK);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Change the appearance of the bounding box.
   MosPrintf(MIL_TEXT("Graphics can be displayed as either points, wireframe, or solid surfaces.\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));   

   MIL_INT Appearances[] = { M_POINTS, M_WIREFRAME, M_SOLID_WITH_WIREFRAME, M_SOLID };
   for(MIL_INT i = 0; !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_APPEARANCE, Appearances[i % 4]);
      MosSleep(CONTROL_DELAY_MSEC / 4);
      }
   M3dgraControl(MilGraphicList3d, BoundingBoxLabel, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);
   MosGetch();

   // Clip a plane using the bounding box.
   MosPrintf(MIL_TEXT("The bounding box is used to clip infinite geometries like planes, lines and cylinders.\n")
             MIL_TEXT("Press <Enter> to show plane clipping.\n\n"));
   MosGetch();

   MIL_INT64 InfinitePlaneLabel = M3dgraPlane(MilGraphicList3d, M_ROOT_NODE, M_POINT_AND_NORMAL, 100, 100, 30, 0.5, 0.4, 3, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_INFINITE, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, InfinitePlaneLabel, M_COLOR, M_COLOR_BLUE);
   M3dgraControl(MilGraphicList3d, InfinitePlaneLabel, M_OPACITY, 60);

   // Change the clipping box and clip a line.
   MosPrintf(MIL_TEXT("The clipping box can also be set manually; it does not need to be the same as the bounding box.\n")
             MIL_TEXT("Press <Enter> to set a different clipping box.\n\n"));
   MosGetch();

   // Remove plane to focus on the clipped line.
   M3dgraRemove(MilGraphicList3d, InfinitePlaneLabel, M_DEFAULT);

   M3dgeoBox(MilBoxGeometry, M_CENTER_AND_DIMENSION, 0, 0, 0, 350, 350, 350, M_DEFAULT);
   M3dgraCopy(MilBoxGeometry, M_DEFAULT, MilGraphicList3d, M_LIST, M_CLIPPING_BOX, M_DEFAULT);
   M3dgraRemove(MilGraphicList3d, BoundingBoxLabel, M_DEFAULT);
   MIL_INT64 ClippingBoxLabel = M3dgeoDraw3d(M_DEFAULT, MilBoxGeometry, MilGraphicList3d, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, ClippingBoxLabel, M_OPACITY, 30);
   M3dgraControl(MilGraphicList3d, ClippingBoxLabel, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);
   M3dgraControl(MilGraphicList3d, ClippingBoxLabel, M_FILL_COLOR, M_COLOR_WHITE);
   M3dgraControl(MilGraphicList3d, ClippingBoxLabel, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(MilGraphicList3d, ClippingBoxLabel, M_THICKNESS, 3);

   MosPrintf(MIL_TEXT("Showing a clipped infinite line in cyan.\n\n"));

   MIL_INT64 InfiniteLineLabel = M3dgraLine(MilGraphicList3d, M_ROOT_NODE, M_POINT_AND_VECTOR, M_DEFAULT, 140, 50, 0, 0, 5, 7, M_INFINITE, M_DEFAULT);
   M3dgraControl(MilGraphicList3d, InfiniteLineLabel, M_COLOR, M_COLOR_CYAN);
   M3dgraControl(MilGraphicList3d, InfiniteLineLabel, M_THICKNESS, 5);

   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Check for required files to run the example.
//****************************************************************************
bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The file needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//****************************************************************************
MIL_STRING GetGraphicTypeString(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel)
   {
   MIL_STRING Info;
   if(GraphicLabel == M_ROOT_NODE)
      {
      Info = MIL_TEXT("Root        ");
      }
   else
      {
      MIL_INT Type;
      M3dgraInquire(MilGraphicList3d, GraphicLabel, M_GRAPHIC_TYPE, &Type);
      switch(Type)
         {
         case M_GRAPHIC_TYPE_ARC:         Info = MIL_TEXT("Arc         "); break;
         case M_GRAPHIC_TYPE_AXIS:        Info = MIL_TEXT("Axis        "); break;
         case M_GRAPHIC_TYPE_BOX:         Info = MIL_TEXT("Box         "); break;
         case M_GRAPHIC_TYPE_CYLINDER:    Info = MIL_TEXT("Cylinder    "); break;
         case M_GRAPHIC_TYPE_DOTS:        Info = MIL_TEXT("Dots        "); break;
         case M_GRAPHIC_TYPE_GRID:        Info = MIL_TEXT("Grid        "); break;
         case M_GRAPHIC_TYPE_LINE:        Info = MIL_TEXT("Line        "); break;
         case M_GRAPHIC_TYPE_NODE:        Info = MIL_TEXT("Node        "); break;
         case M_GRAPHIC_TYPE_PLANE:       Info = MIL_TEXT("Plane       "); break;
         case M_GRAPHIC_TYPE_POINT_CLOUD: Info = MIL_TEXT("Point cloud "); break;
         case M_GRAPHIC_TYPE_POLYGON:     Info = MIL_TEXT("Polygon     "); break;
         case M_GRAPHIC_TYPE_SPHERE:      Info = MIL_TEXT("Sphere      "); break;
         case M_GRAPHIC_TYPE_TEXT:        Info = MIL_TEXT("Text        "); break;
         default:                         Info = MIL_TEXT("Unknown     "); break;
         }
      }

   return Info;
   }

//****************************************************************************
void PrintGraphicListFlat(MIL_ID MilGraphicList3d)
   {
   std::vector<MIL_INT64> GraphicLabels;
   M3dgraInquire(MilGraphicList3d, M_ROOT_NODE, M_CHILDREN + M_RECURSIVE, GraphicLabels);
   GraphicLabels.push_back(M_ROOT_NODE);

   for(MIL_UINT i = 0; i < GraphicLabels.size(); i++)
      {
      MIL_STRING GraphicInfo = GetGraphicTypeString(MilGraphicList3d, GraphicLabels[i]);

      MIL_DOUBLE PosX, PosY, PosZ;
      M3dgraInquire(MilGraphicList3d, GraphicLabels[i], M_POSITION_X + M_RELATIVE_TO_ROOT, &PosX);
      M3dgraInquire(MilGraphicList3d, GraphicLabels[i], M_POSITION_Y + M_RELATIVE_TO_ROOT, &PosY);
      M3dgraInquire(MilGraphicList3d, GraphicLabels[i], M_POSITION_Z + M_RELATIVE_TO_ROOT, &PosZ);

      MosPrintf(MIL_TEXT("-%s\t\t%.2f\t\t%.2f\t\t%.2f\n"), GraphicInfo.c_str(), PosX, PosY, PosZ);
      }
   }

//****************************************************************************
void PrintGraphicListTree(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel, const MIL_STRING& Prefix)
   {
   MIL_STRING GraphicInfo = GetGraphicTypeString(MilGraphicList3d, GraphicLabel);
   MosPrintf(MIL_TEXT("%s%s"), Prefix.c_str(), GraphicInfo.c_str());

   MIL_INT Padding = 24 - (Prefix.length() + GraphicInfo.length());
   for(MIL_INT i = 0; i < Padding; i++)
      { MosPrintf(MIL_TEXT(" ")); }

   MIL_DOUBLE PosX, PosY, PosZ;
   M3dgraInquire(MilGraphicList3d, GraphicLabel, M_POSITION_X, &PosX);
   M3dgraInquire(MilGraphicList3d, GraphicLabel, M_POSITION_Y, &PosY);
   M3dgraInquire(MilGraphicList3d, GraphicLabel, M_POSITION_Z, &PosZ);

   MosPrintf(MIL_TEXT("%.2f\t\t%.2f\t\t%.2f\n"), PosX, PosY, PosZ);

   std::vector<MIL_INT64> ChildrenLabels;
   M3dgraInquire(MilGraphicList3d, GraphicLabel, M_CHILDREN, ChildrenLabels);
   for(MIL_UINT i = 0; i < ChildrenLabels.size(); i++)
      {
      MIL_STRING ChildrenPrefix;
      for(MIL_UINT j = 0; j < Prefix.size(); j++)
         {
         ChildrenPrefix += Prefix[j] == MIL_TEXT('|') ? MIL_TEXT('|') : MIL_TEXT(' ');
         }
      if(i + 1 < ChildrenLabels.size())
         {
         ChildrenPrefix += MIL_TEXT("|-");
         }
      else
         {
         ChildrenPrefix += MIL_TEXT("'-");
         }
      PrintGraphicListTree(MilGraphicList3d, ChildrenLabels[i], ChildrenPrefix);
      }
   }

//****************************************************************************
void FadeIn(MIL_ID MilGraphicList3d, MIL_INT64 GraphicLabel, MIL_INT Duration)
   {
   MIL_STRING GraphicTypeStr = GetGraphicTypeString(MilGraphicList3d, GraphicLabel);
   GraphicTypeStr.erase(GraphicTypeStr.find_last_not_of(MIL_TEXT(" \n\t")) + 1);
   MosPrintf(MIL_TEXT("Adding %s.\n"), GraphicTypeStr.c_str());

   for(MIL_INT i = 0; (i < CONTROL_GRANULARITY) && !MosKbhit(); i++)
      {
      M3dgraControl(MilGraphicList3d, GraphicLabel, M_OPACITY + M_RECURSIVE, 100.0 * i / CONTROL_GRANULARITY);
      MosSleep(Duration / CONTROL_GRANULARITY);
      }

   M3dgraControl(MilGraphicList3d, GraphicLabel, M_OPACITY + M_RECURSIVE, 100.0);
   }
