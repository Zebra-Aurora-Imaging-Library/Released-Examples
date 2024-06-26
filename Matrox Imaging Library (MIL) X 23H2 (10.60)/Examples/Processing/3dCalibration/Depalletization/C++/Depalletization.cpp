﻿//////////////////////////////////////////////////////////////////////////////////
//
// File name: Depalletization.cpp 
//
// Synopsis:  This program calibrates a camera to find stacked blocks from a top-down view.
//            The blocks are located using a 2D rectangle finder in the world plane
//            and a robot arm removes the top layer of blocks. Succeeding layers are correctly
//            detected at the next expected Z-position before removal.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
#include <mil.h>
#include "RobotArmAnimation.h"

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING GRID_AT_ANGLE_FILE = M_IMAGE_PATH MIL_TEXT("Depalletization/CalGridAtAngle.jpg");
static const MIL_STRING GRID_FROM_TOP_FILE = M_IMAGE_PATH MIL_TEXT("Depalletization/CalGridFromTop.jpg");

static std::vector<MIL_STRING> OBJECT_FILES =
   {
      M_IMAGE_PATH MIL_TEXT("Depalletization/Layer01.jpg"),
      M_IMAGE_PATH MIL_TEXT("Depalletization/Layer02.jpg"),
      M_IMAGE_PATH MIL_TEXT("Depalletization/Layer03.jpg"),
      M_IMAGE_PATH MIL_TEXT("Depalletization/Layer04.jpg"),
      M_IMAGE_PATH MIL_TEXT("Depalletization/Layer05.jpg")
   };

// Calibration.
static const MIL_INT    GRID_ROW_NB         = 8;
static const MIL_INT    GRID_COLUMN_NB      = 11;
static const MIL_DOUBLE GRID_ROW_SPACING    = 10.0; // in mm
static const MIL_DOUBLE GRID_COLUMN_SPACING = 10.0; // in mm

// Objects.
static const MIL_DOUBLE BLOCK_THICKNESS = 11.7; // in mm
static const MIL_DOUBLE BLOCK_WIDTH     = 60.0; // in mm
static const MIL_DOUBLE BLOCK_HEIGHT    = 20.0; // in mm

// Display.
static const MIL_INT    DISPLAY_2D_SIZE_X = 600;
static const MIL_INT    DISPLAY_2D_SIZE_Y = 600;
static const MIL_INT    DISPLAY_3D_SIZE_X = 600;
static const MIL_INT    DISPLAY_3D_SIZE_Y = 600;
static const MIL_DOUBLE GRAPHIC_FONT_SIZE = 5;

// Robot arm animation colors.
static const MIL_INT64 ARM_SECTION_COLOR = M_COLOR_YELLOW;
static const MIL_INT64 ARM_JOINT_COLOR   = M_COLOR_GRAY;

// Robot dimensions in mm.
static const MIL_DOUBLE ARM_RADIUS   = 10.0;              
static const MIL_DOUBLE ARM_LENGTH_A = 80.0;
static const MIL_DOUBLE ARM_LENGTH_B = 100.0;
static const MIL_DOUBLE ARM_LENGTH_C = 30.0;

// Robot base position in mm.
static const MIL_DOUBLE ARM_BASE_POS_X = -ARM_RADIUS * 3;            
static const MIL_DOUBLE ARM_BASE_POS_Y = 0;
static const MIL_DOUBLE ARM_BASE_POS_Z = -BLOCK_HEIGHT;

// Position where blocks are dropped in mm.
static const MIL_DOUBLE ARM_REST_POS_X = ARM_BASE_POS_X;             
static const MIL_DOUBLE ARM_REST_POS_Y = 75;
static const MIL_DOUBLE ARM_REST_POS_Z = -BLOCK_THICKNESS;

// Height above the grabbed object to prevent collisions.
static const MIL_DOUBLE ARM_SAFETY_HEIGHT = 40;

// Speed of the grabber (in mm/s).
static const MIL_DOUBLE ARM_ANIMATION_SPEED = 200;                   

//****************************************************************************
// Utility structures.
//****************************************************************************
struct SBlockAnnotations
   {
   MIL_INT64 All;                   // Node that holds all block annotations as children.
   std::vector<MIL_INT64> Blocks;   // Annotations for each specific block.
   };

//****************************************************************************
// Function declarations.
//****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);

MIL_UNIQUE_CAL_ID ComputeCalibration(MIL_ID Image);
MIL_ID MoveCalibrationCamera(MIL_ID Image);

MIL_UNIQUE_MOD_ID CreateBlocksFinder(MIL_ID System);
MIL_UNIQUE_MOD_ID FindBlocks(MIL_ID ModContext, MIL_ID Image);

void DrawCalibration2d(MIL_ID Image, MIL_ID Display);
MIL_INT64 DrawCalibration3d(MIL_ID Image, MIL_ID Display);
void DrawBlocks2d(MIL_ID Image, MIL_ID ModResult, MIL_ID Display);
SBlockAnnotations DrawBlocks3d(MIL_ID Image, MIL_ID ModResult, MIL_ID Display);
void MoveBlocks(MIL_ID System, MIL_ID GraList3d, MIL_ID DropPosition, const SBlockAnnotations& BlockAnnotations,
                CRobotArmAnimation& RobotArm, MIL_INT64& PreviousPickedBlock);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Depalletization\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This program calibrates a camera to find stacked blocks from a top-down view.\n")
             MIL_TEXT("The blocks are located using a 2D rectangle finder in the world plane and\n")
             MIL_TEXT("a robot arm removes the top layer of blocks. Succeeding layers are correctly\n")
             MIL_TEXT("detected at the next expected Z - position before removal.\n\n"));


   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, Buffer, Calibration, Model Finder,\n")
             MIL_TEXT("Image Processing, Display, Graphics, 3D Display, 3D Graphics, 3D Geometry,\n")
             MIL_TEXT("3D Metrology, and 3D Image Processing.\n\n"));
   }

//****************************************************************************
// Main.
//****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   // Allocate MIL objects.
   auto Application = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   auto System = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate and set up the displays.
   auto Display3d = Alloc3dDisplayId(System);
   auto GraList3d = M3ddispInquire(Display3d, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3ddispControl(Display3d, M_WINDOW_INITIAL_POSITION_X, DISPLAY_2D_SIZE_X);
   M3ddispControl(Display3d, M_SIZE_X, DISPLAY_3D_SIZE_X);
   M3ddispControl(Display3d, M_SIZE_Y, DISPLAY_3D_SIZE_Y);
   M3ddispControl(Display3d, M_TITLE, MIL_TEXT("3D View"));

   auto Display2d = MdispAlloc(System, M_DEFAULT, MIL_TEXT(""), M_DEFAULT, M_UNIQUE_ID);
   auto GraList2d = MgraAllocList(System, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(Display2d, M_ASSOCIATED_GRAPHIC_LIST_ID, GraList2d);
   MdispControl(Display2d, M_WINDOW_INITIAL_SIZE_X, DISPLAY_2D_SIZE_X);
   MdispControl(Display2d, M_WINDOW_INITIAL_SIZE_Y, DISPLAY_2D_SIZE_Y);
   MdispControl(Display2d, M_TITLE, MIL_TEXT("Camera View"));

   // Create the robot arm graphics.
   MIL_INT64 PreviousPickedBlock = M_INVALID;
   M3dgraControl(GraList3d, M_DEFAULT_SETTINGS, M_COLOR, ARM_JOINT_COLOR);
   M3dgraBox(GraList3d, M_ROOT_NODE, M_BOTH_CORNERS,
             ARM_BASE_POS_X - ARM_RADIUS * 2, ARM_BASE_POS_Y - ARM_RADIUS * 2, 0,
             ARM_BASE_POS_X + ARM_RADIUS * 2, ARM_BASE_POS_Y + ARM_RADIUS * 2, ARM_BASE_POS_Z,
             M_DEFAULT, M_DEFAULT);

   CRobotArmAnimation RobotArm(Display3d,
                               ARM_BASE_POS_X, ARM_BASE_POS_Y, ARM_BASE_POS_Z,
                               ARM_RADIUS,
                               ARM_LENGTH_A, ARM_LENGTH_B,  ARM_LENGTH_C,
                               ARM_ANIMATION_SPEED,
                               ARM_SECTION_COLOR, ARM_JOINT_COLOR,
                               EOrientation::eZDown);

   auto DropPosition = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetWithAxes(DropPosition, M_ZX_AXES,
                           ARM_REST_POS_X, ARM_REST_POS_Y, ARM_REST_POS_Z,
                           0, 0, -1,
                           0, 1, 0, M_DEFAULT);
   RobotArm.MoveInstant(DropPosition);


   // Step 1: Calibrate from an angled camera position.

   MosPrintf(MIL_TEXT("The camera is placed at an angle.\n"));
   MosPrintf(MIL_TEXT("A calibration grid is used to calibrate it.\n"));
   
   // Restore an angled view of the scene and calibrate it using the grid.
   auto ImageAtAngle = MbufRestore(GRID_AT_ANGLE_FILE, System, M_UNIQUE_ID);
   auto CalibrationAtAngle = ComputeCalibration(ImageAtAngle);
   if(CalibrationAtAngle == M_NULL)
      {
      MosPrintf(MIL_TEXT("Could not calibrate the image.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end."));
      MosGetch();
      return -1;
      }

   // Draw the calibration.
   DrawCalibration2d(ImageAtAngle, Display2d);
   MdispSelect(Display2d, ImageAtAngle);
   MdispControl(Display2d, M_SCALE_DISPLAY, M_ONCE);

   auto CalibrationAnnotations = DrawCalibration3d(ImageAtAngle, Display3d);
   auto BlockAnnotations = DrawBlocks3d(ImageAtAngle, M_NULL, Display3d);
   M3ddispSelect(Display3d, M_NULL, M_OPEN, M_DEFAULT);
   M3ddispSetView(Display3d, M_AUTO, M_BOTTOM_TILTED, BlockAnnotations.All, M_DEFAULT, M_DEFAULT);
   M3ddispSetView(Display3d, M_VIEW_ORIENTATION, -1, -1, 1, M_DEFAULT);

   MIL_DOUBLE WorldErrorAvg, WorldErrorMax;
   McalInquire(CalibrationAtAngle, M_AVERAGE_WORLD_ERROR, &WorldErrorAvg);
   McalInquire(CalibrationAtAngle, M_MAXIMUM_WORLD_ERROR, &WorldErrorMax);

   MosPrintf(MIL_TEXT("   - Average world error = %.4f mm\n"), WorldErrorAvg);
   MosPrintf(MIL_TEXT("   - Maximum world error = %.4f mm\n"), WorldErrorMax);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   // Step 2: Calibrate from a top-down camera position.

   MosPrintf(MIL_TEXT("The camera is repositioned to look down on the grid.\n"));
   MosPrintf(MIL_TEXT("The camera updates its calibration based on the new view of the grid.\n"));
   MosPrintf(MIL_TEXT("Note that the initial angled camera position is required before moving to\n"));
   MosPrintf(MIL_TEXT("the overhead position. The angle provides the image perspective needed to\n"));
   MosPrintf(MIL_TEXT("correctly estimate some intrinsic and extrinsic attributes.\n\n"));

   // Restore a top-down view of the scene and calibrate it using the previous angled calibration.
   auto ImageFromTop = MbufRestore(GRID_FROM_TOP_FILE, System, M_UNIQUE_ID);
   McalAssociate(CalibrationAtAngle, ImageFromTop, M_DEFAULT);
   auto CalibrationFromTop = MoveCalibrationCamera(ImageFromTop);
   if(CalibrationFromTop == M_NULL)
      {
      MosPrintf(MIL_TEXT("Could not move the calibration to the new image.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end."));
      MosGetch();
      return -1;
      }

   // Draw the calibration.
   MdispSelect(Display2d, ImageFromTop);
   DrawCalibration2d(ImageFromTop, Display2d);
   M3dgraRemove(GraList3d, CalibrationAnnotations, M_DEFAULT);
   M3dgraRemove(GraList3d, BlockAnnotations.All, M_DEFAULT);
   CalibrationAnnotations = DrawCalibration3d(ImageFromTop, Display3d);
   BlockAnnotations = DrawBlocks3d(ImageFromTop, M_NULL, Display3d);

   McalInquire(CalibrationFromTop, M_AVERAGE_WORLD_ERROR, &WorldErrorAvg);
   McalInquire(CalibrationFromTop, M_MAXIMUM_WORLD_ERROR, &WorldErrorMax);

   MosPrintf(MIL_TEXT("   - Average world error = %.4f mm\n"), WorldErrorAvg);
   MosPrintf(MIL_TEXT("   - Maximum world error = %.4f mm\n"), WorldErrorMax);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();


   // Step 3: Find stacked blocks from the top-down view.

   MosPrintf(MIL_TEXT("Stacked blocks are placed below the camera.\n"));
   MosPrintf(MIL_TEXT("At each layer, the blocks will be located and picked up.\n\n"));

   // Create the blocks model finder.
   auto BlocksModelFinder = CreateBlocksFinder(System);

   for(MIL_INT FileIndex = (MIL_INT)(OBJECT_FILES.size() - 1); FileIndex >= 0; FileIndex--)
      {
      // Restore the image.
      MbufLoad(OBJECT_FILES[FileIndex], ImageFromTop);

      // Offset the relative coordinate system in Z.
      MIL_DOUBLE PosZ = (FileIndex + 1) * -BLOCK_THICKNESS;
      McalAssociate(CalibrationFromTop, ImageFromTop, M_DEFAULT);
      McalSetCoordinateSystem(ImageFromTop, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                              M_TRANSLATION + M_COMPOSE_WITH_CURRENT, M_NULL, 0, 0, PosZ, M_DEFAULT);

      // Find the rectangles in the image.
      auto Rectangles = FindBlocks(BlocksModelFinder, ImageFromTop);

      // Draw the blocks.
      DrawBlocks2d(ImageFromTop, Rectangles, Display2d);
      M3dgraRemove(GraList3d, BlockAnnotations.All, M_DEFAULT);
      BlockAnnotations = DrawBlocks3d(ImageFromTop, Rectangles, Display3d);

      // Print results.
      MIL_INT NbBlocks = 0;
      MmodGetResult(Rectangles, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbBlocks);
      MosPrintf(MIL_TEXT("Found %i blocks in layer %i.\n"), NbBlocks, FileIndex);
      MosPrintf(MIL_TEXT("Press <Enter> to destack. Press <Enter> again to speed up the animation.\n\n"));
      MosGetch();

      // Remove the found blocks from the top layer.
      MoveBlocks(System, GraList3d, DropPosition, BlockAnnotations, RobotArm, PreviousPickedBlock);
      }

   MosPrintf(MIL_TEXT("All layers have been destacked.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Display3d = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!Display3d)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to exit.\n"));
      MosGetch();
      exit(0);
      }
   return Display3d;
   }

//*****************************************************************************
// Computes a 3D camera calibration using a grid.
//*****************************************************************************
MIL_UNIQUE_CAL_ID ComputeCalibration(MIL_ID Image)
   {
   auto System = MobjInquire(Image, M_OWNER_SYSTEM, M_NULL);

   // Calibrate from the grid.
   auto Calibration = McalAlloc(System, M_TSAI_BASED, M_DEFAULT, M_UNIQUE_ID);
   McalGrid(Calibration, Image,
            0, 0, 0,
            GRID_ROW_NB, GRID_COLUMN_NB, GRID_ROW_SPACING, GRID_COLUMN_SPACING,
            M_FULL_CALIBRATION, M_CIRCLE_GRID);

   if(McalInquire(Calibration, M_CALIBRATION_STATUS, M_NULL) != M_CALIBRATED)
      return MIL_UNIQUE_CAL_ID {};

   McalAssociate(Calibration, Image, M_DEFAULT);
   return Calibration;
   }

//*****************************************************************************
// Displaces the camera using the calibration grid.
//*****************************************************************************
MIL_ID MoveCalibrationCamera(MIL_ID Image)
   {
   auto Calibration = McalInquire(Image, M_ASSOCIATED_CALIBRATION, M_NULL);
   McalGrid(Calibration, Image,
            0, 0, 0,
            GRID_ROW_NB, GRID_COLUMN_NB, GRID_ROW_SPACING, GRID_COLUMN_SPACING,
            M_DISPLACE_CAMERA_COORD, M_CIRCLE_GRID);
   return (McalInquire(Calibration, M_CALIBRATION_STATUS, M_NULL) == M_CALIBRATED) ? Calibration : M_NULL;
   }

//*****************************************************************************
// Creates the rectangle model finder context.
//*****************************************************************************
MIL_UNIQUE_MOD_ID CreateBlocksFinder(MIL_ID System)
   {
   auto ModContext = MmodAlloc(System, M_SHAPE_RECTANGLE, M_DEFAULT, M_UNIQUE_ID);
   MmodDefine(ModContext, M_RECTANGLE, M_FOREGROUND_WHITE, BLOCK_WIDTH, BLOCK_HEIGHT, M_DEFAULT, M_DEFAULT);
   MmodControl(ModContext, M_ALL, M_NUMBER, M_ALL);
   MmodPreprocess(ModContext, M_DEFAULT);
   return ModContext;
   }


//*****************************************************************************
// Finds rectangles in the image.
//*****************************************************************************
MIL_UNIQUE_MOD_ID FindBlocks(MIL_ID ModContext, MIL_ID Image)
   {
   auto System = MobjInquire(Image, M_OWNER_SYSTEM, M_NULL);

   // Alloc the model finder result.
   auto ModResult = MmodAllocResult(System, M_SHAPE_RECTANGLE, M_UNIQUE_ID);

   // Find the rectangles in the image.
   MmodFind(ModContext, Image, ModResult);

   return ModResult;
   }

//*****************************************************************************
// Draws the calibration on a 2D display.
//*****************************************************************************
void DrawCalibration2d(MIL_ID Image, MIL_ID Display)
   {
   auto GraList = MdispInquire(Display, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   MgraClear(M_DEFAULT, GraList);

   MgraControl(M_DEFAULT, M_COLOR, M_COLOR_LIGHT_BLUE);
   McalDraw(M_DEFAULT, Image, GraList, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);
   }

//*****************************************************************************
// Draws the calibration on a 3D display.
//*****************************************************************************
MIL_INT64 DrawCalibration3d(MIL_ID Image, MIL_ID Display)
   {
   auto System = MobjInquire(Image, M_OWNER_SYSTEM, M_NULL);
   auto GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);

   auto Draw3dContext = McalAlloc(System, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   McalControl(Draw3dContext, M_DRAW_RELATIVE_XY_PLANE_OPACITY, 0);
   McalControl(Draw3dContext, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_RELATIVE_COORDINATE_SYSTEM_NAME, MIL_TEXT(""));
   McalControl(Draw3dContext, M_DRAW_CAMERA_COORDINATE_SYSTEM_NAME, MIL_TEXT(""));
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_FONT_SIZE, GRAPHIC_FONT_SIZE);

   return McalDraw3d(Draw3dContext, Image, M_DEFAULT, GraList, M_ROOT_NODE, M_NULL, M_DEFAULT);
   }

//*****************************************************************************
// Draws the found blocks on a 2D display.
//*****************************************************************************
void DrawBlocks2d(MIL_ID Image, MIL_ID ModResult, MIL_ID Display)
   {
   auto GraList = MdispInquire(Display, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);
   MgraClear(M_DEFAULT, GraList);

   MgraControl(M_DEFAULT, M_COLOR, M_COLOR_MAGENTA);
   MmodDraw(M_DEFAULT, ModResult, GraList, M_DRAW_POSITION, M_ALL, M_DEFAULT);

   MgraControl(M_DEFAULT, M_COLOR, M_COLOR_RED);
   MmodDraw(M_DEFAULT, ModResult, GraList, M_DRAW_EDGES + M_MODEL, M_ALL, M_DEFAULT);

   MgraControl(M_DEFAULT, M_COLOR, M_COLOR_LIGHT_BLUE);
   McalDraw(M_DEFAULT, Image, GraList, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM, M_DEFAULT, M_DEFAULT);
   }

//*****************************************************************************
// Draws the found blocks on a 3D display.
//*****************************************************************************
SBlockAnnotations DrawBlocks3d(MIL_ID Image, MIL_ID ModResult, MIL_ID Display)
   {
   SBlockAnnotations Annotations;
   auto System = MobjInquire(Image, M_OWNER_SYSTEM, M_NULL);
   auto GraList = M3ddispInquire(Display, M_3D_GRAPHIC_LIST_ID, M_NULL);
   Annotations.All = M3dgraNode(GraList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   // Set up a 3D draw calibration context that only draws the image.
   auto Draw3dContext = McalAlloc(System, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   McalControl(Draw3dContext, M_DRAW_CAMERA_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_ABSOLUTE_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_RELATIVE_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_ROBOT_BASE_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_TOOL_COORDINATE_SYSTEM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_FRUSTUM, M_DISABLE);
   McalControl(Draw3dContext, M_DRAW_RELATIVE_XY_PLANE_COLOR_FILL, M_TEXTURE_IMAGE);
   McalControl(Draw3dContext, M_DRAW_RELATIVE_XY_PLANE_COLOR_OUTLINE, M_NO_COLOR);

   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_COLOR, M_COLOR_BLACK);
   M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_FILL_COLOR, M_COLOR_WHITE);

   // Allocate temporary objects.
   auto Box = M3dgeoAlloc(System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   auto BottomTexture = MbufClone(Image, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MbufClear(BottomTexture, 255);
   McalAssociate(Image, BottomTexture, M_DEFAULT);

   MIL_INT NbBlocks = 0;
   if(ModResult)
      MmodGetResult(ModResult, M_GENERAL, M_NUMBER + M_TYPE_MIL_INT, &NbBlocks);

   M3ddispControl(Display, M_UPDATE, M_DISABLE);

   if(NbBlocks > 0)
      {
      // Draw the top blocks.
      Annotations.Blocks.resize(NbBlocks);
      for(MIL_INT OccurenceIndex = 0; OccurenceIndex < NbBlocks; OccurenceIndex++)
         {
         // Get the block's size and location.
         MIL_DOUBLE CenterX, CenterY, CenterZ, SizeX, SizeY, Angle, RelX, RelY;
         MmodGetResult(ModResult, OccurenceIndex, M_CENTER_X, &CenterX);
         MmodGetResult(ModResult, OccurenceIndex, M_CENTER_Y, &CenterY);
         MmodGetResult(ModResult, OccurenceIndex, M_WIDTH, &SizeX);
         MmodGetResult(ModResult, OccurenceIndex, M_HEIGHT, &SizeY);
         MmodGetResult(ModResult, OccurenceIndex, M_ANGLE, &Angle);
         McalGetCoordinateSystem(Image, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM, M_TRANSLATION, M_NULL,
                                 &RelX, &RelY, &CenterZ, M_NULL);
         CenterZ += BLOCK_THICKNESS / 2;

         // Draw a box that represents the block in the 3D display.
         M3dgeoBox(Box, M_CENTER_AND_DIMENSION, CenterX, CenterY, CenterZ, SizeX, SizeY, BLOCK_THICKNESS, M_DEFAULT);
         M3dimRotate(Box, Box, M_ROTATION_XYZ, 180, 0, Angle, M_DEFAULT, M_GEOMETRY_CENTER, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         Annotations.Blocks[OccurenceIndex] = M3dgeoDraw3d(M_DEFAULT, Box, GraList, Annotations.All, M_DEFAULT);

         // Mark the block's location in the image.
         MgraControl(M_DEFAULT, M_COLOR, OccurenceIndex);
         MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
         MgraRectAngle(M_DEFAULT, BottomTexture, CenterX, CenterY, SizeX, SizeY, Angle, M_CENTER_AND_DIMENSION + M_FILLED);

         // Draw the top texture.
         MIL_INT TopTextureOffsetX, TopTextureOffsetY, TopTextureSizeX, TopTextureSizeY;
         MimBoundingBox(BottomTexture, M_EQUAL, (MIL_DOUBLE)OccurenceIndex, M_NULL, M_CORNER_AND_DIMENSION,
                        &TopTextureOffsetX, &TopTextureOffsetY, &TopTextureSizeX, &TopTextureSizeY, M_DEFAULT);
         auto TopTexture = MbufChild2d(Image, TopTextureOffsetX, TopTextureOffsetY,
                                       TopTextureSizeX, TopTextureSizeY, M_UNIQUE_ID);
         MIL_INT64 TopTextureNode = McalDraw3d(Draw3dContext, TopTexture, M_DEFAULT, GraList,
                                               Annotations.Blocks[OccurenceIndex], TopTexture, M_DEFAULT);
         M3dgraCopy(M_IDENTITY_MATRIX, M_DEFAULT, GraList, TopTextureNode,
                    M_TRANSFORMATION_MATRIX + M_RELATIVE_TO_ROOT, M_DEFAULT);
         }

      // Draw a big box below the blocks.
      M3dgraCopy(GraList, Annotations.All, Box, M_DEFAULT, M_BOUNDING_BOX + M_RECURSIVE, M_DEFAULT);
      M3dimTranslate(Box, Box, 0, 0, BLOCK_THICKNESS, M_DEFAULT);
      M3dmetFeatureEx(Box, M_XY_PLANE, M_NULL, Box, M_EXTRUSION_CENTER, M_DEFAULT, M_DEFAULT);
      M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_OPACITY, 50);
      if(M3dgeoInquire(Box, M_CENTER_Z, M_NULL) < 0)
         M3dgeoDraw3d(M_DEFAULT, Box, GraList, Annotations.All, M_DEFAULT);
      M3dgraControl(GraList, M_DEFAULT_SETTINGS, M_OPACITY, 100);
      }

   // Draw the bottom texture by cropping out the top part.
   // Draw it at z = 0 regardless of the current block height.
   MbufCopyCond(Image, BottomTexture, BottomTexture, M_EQUAL, 255);
   McalSetCoordinateSystem(BottomTexture, M_RELATIVE_COORDINATE_SYSTEM, M_ABSOLUTE_COORDINATE_SYSTEM,
                           M_TRANSLATION, M_NULL, 0, 0, 0, M_DEFAULT);
   McalDraw3d(Draw3dContext, BottomTexture, M_DEFAULT, GraList, Annotations.All, BottomTexture, M_DEFAULT);

   M3ddispControl(Display, M_UPDATE, M_ENABLE);

   return Annotations;
   }

//*****************************************************************************
// Makes the robot move the found blocks in the 3D display.
//*****************************************************************************
void MoveBlocks(MIL_ID System, MIL_ID GraList3d, MIL_ID DropPosition, const SBlockAnnotations& BlockAnnotations,
                CRobotArmAnimation& RobotArm, MIL_INT64& PreviousPickedBlock)
   {
   auto PickPosition = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Move the blocks.
   for(const auto& UnpickedBlock : BlockAnnotations.Blocks)
      {
      // Move the robot arm to the block.
      M3dgraCopy(GraList3d, UnpickedBlock, PickPosition, M_DEFAULT, M_TRANSFORMATION_MATRIX, M_DEFAULT);
      M3dgeoMatrixSetTransform(PickPosition, M_TRANSLATION, 0, 0, -BLOCK_THICKNESS / 2, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
      RobotArm.Move(PickPosition, ARM_SAFETY_HEIGHT);
      M3dgeoMatrixSetTransform(PickPosition, M_TRANSLATION, 0, 0, BLOCK_THICKNESS / 2, M_DEFAULT, M_COMPOSE_WITH_CURRENT);

      // Make the block follow the arm around by making it a child in the graphic hierarchy.
      if(PreviousPickedBlock != M_INVALID)
         M3dgraRemove(GraList3d, PreviousPickedBlock, M_DEFAULT);
      auto PickedBlock = M3dgraCopy(GraList3d, UnpickedBlock, GraList3d, RobotArm.m_SectionC,
                                    M_GRAPHIC + M_RECURSIVE, M_DEFAULT);
      M3dgraCopy(PickPosition, M_DEFAULT, GraList3d, PickedBlock, M_TRANSFORMATION_MATRIX + M_RELATIVE_TO_ROOT, M_DEFAULT);
      M3dgraRemove(GraList3d, UnpickedBlock, M_DEFAULT);

      // Move the robot arm back to the drop position.
      RobotArm.Move(DropPosition, ARM_SAFETY_HEIGHT);

      // Detach the block.
      M3dgeoCopy(DropPosition, PickPosition, M_TRANSFORMATION_MATRIX, M_DEFAULT);
      M3dgeoMatrixSetTransform(PickPosition, M_TRANSLATION, 0, 0, BLOCK_THICKNESS / 2, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
      PreviousPickedBlock = M3dgraCopy(GraList3d, PickedBlock, GraList3d, M_ROOT_NODE, M_GRAPHIC + M_RECURSIVE, M_DEFAULT);
      M3dgraCopy(PickPosition, M_DEFAULT, GraList3d, PreviousPickedBlock,
                 M_TRANSFORMATION_MATRIX + M_RELATIVE_TO_ROOT, M_DEFAULT);
      M3dgraRemove(GraList3d, PickedBlock, M_DEFAULT);
      }

   if(MosKbhit())
      MosGetch();
   }
