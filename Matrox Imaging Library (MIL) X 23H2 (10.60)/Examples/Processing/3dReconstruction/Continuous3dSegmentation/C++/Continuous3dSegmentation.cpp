﻿//***************************************************************************************/
//
// File name: Continuous3dSegmentation.cpp
//
// Synopsis:  This program continuously scans a conveyor and counts the passing objects.
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <algorithm>

// Source file specification.
static const MIL_STRING PT_CLD_FOLDER = M_IMAGE_PATH MIL_TEXT("Continuous3dSegmentation");

// Conveyor animation.
static const MIL_DOUBLE CONVEYOR_LENGTH = 750;        // Conveyor display length (in mm).
static const MIL_DOUBLE FRAME_RATE = 10;              // Speed of the conveyor (in frames/s).

// Segmentation thresholds.
static const MIL_INT32  MIN_NB_POINTS = 10000;        // Min number of points for a rock to be counted.
static const MIL_INT32  KERNEL_SIZE = 3;              // Size of the square kernel used to find neighbors.
static const MIL_DOUBLE DISTANCE_THRESHOLD = 2.0;     // Max distance between 2 points for them to be blobbed together (in mm).
static const MIL_INT    MAX_BLOB_LINES = 2000;        // Max number of lines that a rock can be.

// Function declarations.
void                    CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID    Alloc3dDisplayId(MIL_ID MilSystem);
void                    StitchContainers(MIL_ID TopContainer, MIL_ID BottomContainer, MIL_ID StitchedContainer);

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Continuous3dSegmentation\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example performs continuous 3d segmentation to\n")
             MIL_TEXT("count rocks on a conveyor. \n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Blob Analysis, 3D Image Processing, 3D Metrology,\n")
             MIL_TEXT("3d Geometry, 3D Display, 3D Graphics, Buffer, and Digitizer.\n\n"));
   }

//*****************************************************************************
// Class that counts rocks on a conveyor.
//*****************************************************************************
class CRockCounter
   {
   public:
      CRockCounter(MIL_ID Digitizer, MIL_ID FrameDisplay, MIL_ID ConveyorDisplay);

      static MIL_INT MFTYPE DigProcessFunc(MIL_INT HookType, MIL_ID EventId, void* UserDataPtr);

   private:
      void       InitFromFirstFrame(MIL_ID Container, MIL_ID FrameDisplay, MIL_ID ConveyorDisplay);
      MIL_DOUBLE StitchAndSegment(MIL_ID Container);
      void       UpdateDisplay(MIL_DOUBLE TranslationY);

      // Display related non-owned objects.
      MIL_ID               m_ConveyorDisplay;      // Conveyor animation display.
      MIL_ID               m_ConveyorGraphicList;  // Conveyor animation graphic list.
      MIL_INT64            m_SlidingNode;          // The node used to move all moving graphics along the conveyor.

      // Containers.
      MIL_UNIQUE_BUF_ID    m_DisplayContainer;     // The point cloud currently being displayed.
      MIL_UNIQUE_BUF_ID    m_CurrentContainer;     // The point cloud containing the current frame.
      MIL_UNIQUE_BUF_ID    m_PreviousContainer;    // The point cloud containing the previous unprocessed blobs.
      MIL_UNIQUE_BUF_ID    m_StitchedContainer;    // The current point cloud stitched with the previous one.

      // Segmentation objects.
      MIL_UNIQUE_3DBLOB_ID m_SegmentationContext;  // Context for M3dblobSegment.
      MIL_UNIQUE_3DBLOB_ID m_CalculateContext;     // Context for M3dblobCalculate.
      MIL_UNIQUE_3DBLOB_ID m_Draw3dContext;        // Context for M3dblobDraw3d.
      MIL_UNIQUE_3DBLOB_ID m_AllBlobs;             // Result containing all blobs that were found by the segmentation.
      MIL_UNIQUE_3DBLOB_ID m_ProcessedBlobs;       // Result containing the blobs that were counted.
      MIL_UNIQUE_3DBLOB_ID m_UnprocessedBlobs;     // Result containing the blobs that will be processed in the next frame.

      // Temporary objects.
      MIL_UNIQUE_3DGEO_ID  m_BoundingBox;          // Each frame's bounding box.
      MIL_UNIQUE_3DGEO_ID  m_CroppingPlane;        // Plane used to quickly remove the background.
      MIL_UNIQUE_3DGEO_ID  m_TranslationMat;       // Matrix used to translate graphics along the conveyor.

      MIL_DOUBLE           m_PrevMinY = 0;         // Used to align the current frame with the previous one.
      MIL_INT              m_NbFrames = 0;         // Total number of frames.
      MIL_INT              m_NbBlobs  = 0;         // Total number of rocks.
   };

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Check for required example files.
   CheckForRequiredMILFile(PT_CLD_FOLDER);

   auto MilSystem = MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   auto MilDigitizer = MdigAlloc(MilSystem, M_DEFAULT, PT_CLD_FOLDER + MIL_TEXT("@") + M_TO_STRING(FRAME_RATE), M_DEFAULT, M_UNIQUE_ID);
   
   // Allocate the displays.
   auto FrameDisplay = Alloc3dDisplayId(MilSystem);         // Displays the current frame.
   auto ConveyorDisplay = Alloc3dDisplayId(MilSystem);      // Displays the full conveyor.

   // Set up the displays.
   M3ddispControl(FrameDisplay, M_TITLE, MIL_TEXT("Current Frame"));
   M3ddispControl(ConveyorDisplay, M_TITLE, MIL_TEXT("Conveyor"));

   M3ddispControl(FrameDisplay, M_SIZE_X, 300);
   M3ddispControl(FrameDisplay, M_SIZE_Y, 300);
   M3ddispControl(FrameDisplay, M_WINDOW_INITIAL_POSITION_X, 800);

   // Set up segmentation objects.
   CRockCounter RockCounter(MilDigitizer, FrameDisplay, ConveyorDisplay);

   MosPrintf(MIL_TEXT("This example continuously loads snapshots of rocks on a conveyor.\n"));
   MosPrintf(MIL_TEXT("At each snapshot:\n"));
   MosPrintf(MIL_TEXT("   -The background is removed.\n"));
   MosPrintf(MIL_TEXT("   -The rocks are located by 3d segmentation.\n"));
   MosPrintf(MIL_TEXT("   -Fully visible rocks are added to the total count and shown on the display.\n"));
   MosPrintf(MIL_TEXT("   -Rocks which touch the edge of the frame are not counted. Instead, they\n"));
   MosPrintf(MIL_TEXT("    are aligned and stitched with the next snapshot, and the process repeats.\n\n"));
   MosPrintf(MIL_TEXT("The rocks are much larger than the snapshot, so it can take 2-3 frames\n"));
   MosPrintf(MIL_TEXT("before a rock gets counted. Quicker and smaller snapshots improve latency,\n"));
   MosPrintf(MIL_TEXT("but require more processing since more segmentation per rock is performed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start.\n"));
   MosGetch();
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
   
   // Dispatch the processing function.
   MdigProcess(MilDigitizer, M_NULL, M_DEFAULT, M_START, M_DEFAULT, &CRockCounter::DigProcessFunc, &RockCounter);

   MosGetch();

   // Stop the processing thread.
   MdigProcess(MilDigitizer, M_NULL, M_DEFAULT, M_STOP, M_DEFAULT, &CRockCounter::DigProcessFunc, &RockCounter);

   return 0;
   }



   
//*****************************************************************************
// Stitches two containers on top of each other to preserve organization (as opposed to M3dimMerge).  
//*****************************************************************************
void StitchContainers(MIL_ID TopContainer, MIL_ID BottomContainer, MIL_ID StitchedContainer)
   {
   // Get the dimensions.
   MIL_INT SizeX = MbufInquireContainer(TopContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT TopSizeY = MbufInquireContainer(TopContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_INT BottomSizeY = MbufInquireContainer(BottomContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   MIL_INT StitchedSizeY = TopSizeY + BottomSizeY;

   // Prevent extremely long buffers. This is usually a sign of bad cropping.
   if(StitchedSizeY > MAX_BLOB_LINES)
      StitchedSizeY = MAX_BLOB_LINES;

   // Get the components from both containers.
   std::vector<MIL_INT64> TopComponentsTypes, BottomComponentsTypes;
   MbufInquireContainer(TopContainer, M_CONTAINER, M_COMPONENT_TYPE_LIST, TopComponentsTypes);
   MbufInquireContainer(BottomContainer, M_CONTAINER, M_COMPONENT_TYPE_LIST, BottomComponentsTypes);

   // Make a list of the components that are part of both containers.
   std::vector<MIL_INT64> CommonComponentTypes;
   std::sort(TopComponentsTypes.begin(), TopComponentsTypes.end());
   std::sort(BottomComponentsTypes.begin(), BottomComponentsTypes.end());
   std::set_intersection(TopComponentsTypes.begin(), TopComponentsTypes.end(),
                         BottomComponentsTypes.begin(), BottomComponentsTypes.end(),
                         std::back_inserter(CommonComponentTypes));

   // Free all existing components.
   MbufFreeComponent(StitchedContainer, M_COMPONENT_ALL, M_DEFAULT);

   // Stitch the components together.
   for(const auto& ComponentType : CommonComponentTypes)
      {
      MIL_ID TopComponent = MbufInquireContainer(TopContainer, ComponentType, M_COMPONENT_ID, M_NULL);
      MIL_ID BottomComponent = MbufInquireContainer(BottomContainer, ComponentType, M_COMPONENT_ID, M_NULL);
      MIL_INT SizeBand = MbufInquireContainer(BottomContainer, ComponentType, M_SIZE_BAND, M_NULL);
      MIL_INT Type = MbufInquireContainer(BottomContainer, ComponentType, M_TYPE, M_NULL);
      MIL_ID StitchedComponent = MbufAllocComponent(StitchedContainer, SizeBand, SizeX, TopSizeY + BottomSizeY, Type, M_IMAGE + M_PROC, ComponentType, M_NULL);

      MbufCopyClip(TopComponent, StitchedComponent, 0, 0);
      MbufCopyClip(BottomComponent, StitchedComponent, 0, TopSizeY);
      }
   }

//*****************************************************************************
// Constructor.  
//*****************************************************************************
CRockCounter::CRockCounter(MIL_ID Digitizer, MIL_ID FrameDisplay, MIL_ID ConveyorDisplay)
   {
   // Save the non-owned objects.
   MIL_ID System = MobjInquire(Digitizer, M_OWNER_SYSTEM, M_NULL);
   m_ConveyorDisplay = ConveyorDisplay;
   m_ConveyorGraphicList = (MIL_ID)M3ddispInquire(ConveyorDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   // Allocate the required containers.
   m_DisplayContainer  = MbufAllocContainer(System, M_PROC + M_DISP, M_DEFAULT, M_UNIQUE_ID);
   m_CurrentContainer  = MbufAllocContainer(System, M_PROC, M_DEFAULT, M_UNIQUE_ID);         
   m_PreviousContainer = MbufAllocContainer(System, M_PROC, M_DEFAULT, M_UNIQUE_ID);         
   m_StitchedContainer = MbufAllocContainer(System, M_PROC, M_DEFAULT, M_UNIQUE_ID);         

   // Allocate the segmentation objects.
   m_SegmentationContext = M3dblobAlloc(System, M_SEGMENTATION_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   m_CalculateContext = M3dblobAlloc(System, M_CALCULATE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   m_Draw3dContext = M3dblobAlloc(System, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   m_AllBlobs = M3dblobAllocResult(System, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);        
   m_ProcessedBlobs = M3dblobAllocResult(System, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);  
   m_UnprocessedBlobs = M3dblobAllocResult(System, M_SEGMENTATION_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Set up the segmentation objects.
   M3dblobControl(m_SegmentationContext, M_DEFAULT, M_NEIGHBOR_SEARCH_MODE, M_ORGANIZED);
   M3dblobControl(m_SegmentationContext, M_DEFAULT, M_NEIGHBORHOOD_ORGANIZED_SIZE, KERNEL_SIZE);
   M3dblobControl(m_SegmentationContext, M_DEFAULT, M_MAX_DISTANCE, DISTANCE_THRESHOLD);

   M3dblobControl(m_CalculateContext, M_DEFAULT, M_SEMI_ORIENTED_BOX, M_ENABLE);

   M3dblobControlDraw(m_Draw3dContext, M_DRAW_BLOBS, M_ACTIVE, M_ENABLE);
   M3dblobControlDraw(m_Draw3dContext, M_DRAW_BLOBS, M_THICKNESS, 3);
   M3dblobControlDraw(m_Draw3dContext, M_DRAW_SEMI_ORIENTED_BOX, M_ACTIVE, M_ENABLE);
   M3dblobControlDraw(m_Draw3dContext, M_DRAW_SEMI_ORIENTED_BOX, M_COLOR, M_COLOR_YELLOW);

   // Allocate the other objects.
   m_BoundingBox = M3dgeoAlloc(System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   m_CroppingPlane = M3dgeoAlloc(System, M_GEOMETRY, M_DEFAULT, M_UNIQUE_ID);
   m_TranslationMat = M3dgeoAlloc(System, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);

   // Grab once to initialize the objects before the processing loop.
   auto GrabContainer = MbufAllocContainer(System, M_GRAB, M_DEFAULT, M_UNIQUE_ID);
   MdigGrab(Digitizer, GrabContainer);
   InitFromFirstFrame(GrabContainer, FrameDisplay, ConveyorDisplay);
   }

//*****************************************************************************
// Static function to fit in MdigProcess's API.  
//*****************************************************************************
MIL_INT MFTYPE CRockCounter::DigProcessFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* UserDataPtr)
   {
   // Get the current frame.
   MIL_ID GrabContainer;
   MdigGetHookInfo(EventId, M_MODIFIED_BUFFER + M_BUFFER_ID, &GrabContainer);

   // Call the processing function.
   auto* RockCounter = static_cast<CRockCounter*>(UserDataPtr);
   MIL_DOUBLE TranslationY = RockCounter->StitchAndSegment(GrabContainer);
   RockCounter->UpdateDisplay(TranslationY);
   return 0;
   }

//*****************************************************************************
// Initializes the graphics and cropping plane from the first grab.  
//*****************************************************************************
void CRockCounter::InitFromFirstFrame(MIL_ID GrabContainer, MIL_ID FrameDisplay, MIL_ID ConveyorDisplay)
   {
   // Convert to a processable format.
   MbufConvert3d(GrabContainer, m_PreviousContainer, M_NULL, M_DEFAULT, M_DEFAULT);

   // Identify the background to quickly crop it out during processing.
   M3dmetFit(M_DEFAULT, m_PreviousContainer, M_PLANE, m_CroppingPlane, DISTANCE_THRESHOLD, M_DEFAULT);

   // Slide the plane up a bit so it crops more.
   M3dimTranslate(m_CroppingPlane, m_CroppingPlane, 0, 0, DISTANCE_THRESHOLD, M_DEFAULT);

   // Compute the bounding box to know where to draw the conveyor.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, m_PreviousContainer, m_BoundingBox, M_DEFAULT);

   // Draw the frame's position.
   MIL_INT64 FrameLabel = M3dgeoDraw3d(M_DEFAULT, m_BoundingBox, m_ConveyorGraphicList, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(m_ConveyorGraphicList, FrameLabel, M_APPEARANCE, M_WIREFRAME);
   M3dgraControl(m_ConveyorGraphicList, FrameLabel, M_THICKNESS, 3);
   M3dgraControl(m_ConveyorGraphicList, FrameLabel, M_COLOR, M_COLOR_WHITE);

   // Draw the conveyor.
   MIL_DOUBLE FrameLength = M3dgeoInquire(m_BoundingBox, M_SIZE_Y, M_NULL);
   M3dgeoBox(m_BoundingBox, M_CENTER_AND_DIMENSION,
             M_UNCHANGED, M_UNCHANGED, M3dgeoInquire(m_CroppingPlane, M_CLOSEST_TO_ORIGIN_Z, M_NULL) - DISTANCE_THRESHOLD * 4,
             M_UNCHANGED, CONVEYOR_LENGTH + FrameLength * 3, DISTANCE_THRESHOLD,
             M_DEFAULT);
   M3dimTranslate(m_BoundingBox, m_BoundingBox, 0, CONVEYOR_LENGTH / 2 + FrameLength, 0, M_DEFAULT);

   MIL_INT64 ConveyorLabel = M3dgeoDraw3d(M_DEFAULT, m_BoundingBox, m_ConveyorGraphicList, M_ROOT_NODE, M_DEFAULT);
   M3dgraControl(m_ConveyorGraphicList, ConveyorLabel, M_APPEARANCE, M_SOLID_WITH_WIREFRAME);
   M3dgraControl(m_ConveyorGraphicList, ConveyorLabel, M_THICKNESS, 3);
   M3dgraControl(m_ConveyorGraphicList, ConveyorLabel, M_COLOR, M_COLOR_GRAY);
   M3dgraControl(m_ConveyorGraphicList, ConveyorLabel, M_FILL_COLOR, M_COLOR_BLACK);

   // Create the node used to move annotations along the conveyor.
   m_SlidingNode = M3dgraNode(m_ConveyorGraphicList, M_ROOT_NODE, M_DEFAULT, M_DEFAULT);

   // Open the displays.
   M3ddispSelect(ConveyorDisplay, M_NULL, M_OPEN, M_DEFAULT);
   M3ddispSelect(FrameDisplay, m_DisplayContainer, M_SELECT, M_DEFAULT);

   // Empty PreviousContainer by making all points invalid.
   MIL_ID PreviousConf = MbufInquireContainer(m_PreviousContainer, M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
   MbufClear(PreviousConf, 0);
   }

//*****************************************************************************
// Performs one processing iteration at every frame:  
// -Stitch the current frame with the previous unprocessed blobs.
// -Do 3d segmentation.
// -Select blobs that are far enough down the conveyor, add them to the total count.
// -Save those that aren't far enough. They will be stitched in the next iteration.
// -Returns the translation between the current and last frame because it is used for display purposes.
//*****************************************************************************
MIL_DOUBLE CRockCounter::StitchAndSegment(MIL_ID GrabContainer)
   {
   // Convert to a processable format.
   MbufConvert3d(GrabContainer, m_DisplayContainer, M_NULL, M_DEFAULT, M_DEFAULT);

   // Get the container's size.
   M3dimStat(M_STAT_CONTEXT_BOUNDING_BOX, m_DisplayContainer, m_BoundingBox, M_DEFAULT);
   MIL_DOUBLE MinY = M3dgeoInquire(m_BoundingBox, M_UNROTATED_MIN_Y, M_NULL);
   MIL_DOUBLE MaxY = M3dgeoInquire(m_BoundingBox, M_UNROTATED_MAX_Y, M_NULL);

   // Align the previous container with the current one.
   MIL_DOUBLE TranslationY = MaxY - m_PrevMinY;
   M3dimTranslate(m_PreviousContainer, m_PreviousContainer, 0, TranslationY, 0, M_DEFAULT);
   m_PrevMinY = MinY;

   // Crop the background.
   M3dimCrop(m_DisplayContainer, m_CurrentContainer, m_CroppingPlane, M_NULL, M_SAME, M_DEFAULT);

   // Stitch the two containers together.
   StitchContainers(m_CurrentContainer, m_PreviousContainer, m_StitchedContainer);

   // Segment the stitched container.
   M3dblobSegment(m_SegmentationContext, m_StitchedContainer, m_AllBlobs, M_DEFAULT);

   // Discard the blobs that are too close to the top of the image since they are not fully visible yet.
   M3dblobSelect(m_AllBlobs, m_UnprocessedBlobs, M_PIXEL_MIN_Y, M_LESS, (KERNEL_SIZE - 1) / 2, M_NULL, M_DEFAULT);

   // Extract the discarded blobs into a container. They will be stitched with the next frame.
   M3dblobExtract(m_StitchedContainer, m_UnprocessedBlobs, M_ALL, m_PreviousContainer, M_SHRINK_VERTICALLY, M_DEFAULT);

   // Select the fully visible blobs, excluding very small ones.
   M3dblobCombine(m_AllBlobs, m_UnprocessedBlobs, m_ProcessedBlobs, M_SUB, M_DEFAULT);
   M3dblobSelect(m_ProcessedBlobs, m_ProcessedBlobs, M_NUMBER_OF_POINTS, M_GREATER_OR_EQUAL, MIN_NB_POINTS, M_NULL, M_DEFAULT);

   // Calculate the selected blobs' semi-oriented bounding box.
   M3dblobCalculate(m_CalculateContext, m_StitchedContainer, m_ProcessedBlobs, M_ALL, M_DEFAULT);

   // Add the blobs to the total count.
   m_NbFrames++;
   m_NbBlobs += (MIL_INT)M3dblobGetResult(m_ProcessedBlobs, M_GENERAL, M_NUMBER, M_NULL);
   MosPrintf(MIL_TEXT("\rFrames processed: %i\tNumber of rocks: %i"), m_NbFrames, m_NbBlobs);

   return TranslationY;
   }

//*****************************************************************************
// Updates the display at every frame:
// -Draw the current container and blobs.
// -Move all graphics along the conveyor.
// -Delete graphics that are too far down the conveyor.
//*****************************************************************************
void CRockCounter::UpdateDisplay(MIL_DOUBLE TranslationY)
   {
   // Disable updates because a lot of graphics are going to be changed.
   M3ddispControl(m_ConveyorDisplay, M_UPDATE, M_DISABLE);

   // Put the current translation in a matrix to move the graphical annotations.
   M3dgeoMatrixSetTransform(m_TranslationMat, M_TRANSLATION, 0, TranslationY, 0, M_DEFAULT, M_DEFAULT);

   // Get the labels of all graphics on the conveyor.
   std::vector<MIL_INT64> Children;
   M3dgraInquire(m_ConveyorGraphicList, m_SlidingNode, M_CHILDREN, Children);
   for(const auto& Child : Children)
      {
      // Move the graphic forward on the conveyor.
      M3dgraCopy(m_TranslationMat, M_DEFAULT, m_ConveyorGraphicList, Child, M_TRANSFORMATION_MATRIX + M_COMPOSE_WITH_CURRENT, M_DEFAULT);

      // Inquire the graphic's new position. If it is too far, remove it.
      MIL_DOUBLE PositionY;
      M3dgraInquire(m_ConveyorGraphicList, Child, M_POSITION_Y, &PositionY);
      if(PositionY > CONVEYOR_LENGTH)
         M3dgraRemove(m_ConveyorGraphicList, Child, M_DEFAULT);
      }

   // Draw the container. Make a copy that is owned by the graphic so there is no concern about keeping track of the container.
   M3dgraAdd(m_ConveyorGraphicList, m_SlidingNode, m_DisplayContainer, M_NO_LINK);

   // Draw the counted blobs. Add a color offset so the colors don't repeat between consecutive draws.
   M3dblobDraw3d(m_Draw3dContext, m_StitchedContainer, m_ProcessedBlobs, M_ALL, m_ConveyorGraphicList, m_SlidingNode, M_DEFAULT);
   MIL_INT ColorOffset = (MIL_INT)M3dblobInquireDraw(m_Draw3dContext, M_GLOBAL_DRAW_SETTINGS, M_PSEUDO_COLOR_OFFSET, M_NULL);
   ColorOffset += (MIL_INT)M3dblobGetResult(m_ProcessedBlobs, M_GENERAL, M_MAX_LABEL_VALUE, M_NULL);
   M3dblobControlDraw(m_Draw3dContext, M_GLOBAL_DRAW_SETTINGS, M_PSEUDO_COLOR_OFFSET, ColorOffset);

   // Re-enable updates.
   M3ddispControl(m_ConveyorDisplay, M_UPDATE, M_ENABLE);
   }

//****************************************************************************
// Check for required files to run the example.    
//****************************************************************************
void CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      exit(0);
      }
   }

//*****************************************************************************
// Allocates a 3D display and return its MIL identifier.  
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MIL_UNIQUE_3DDISP_ID MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
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
