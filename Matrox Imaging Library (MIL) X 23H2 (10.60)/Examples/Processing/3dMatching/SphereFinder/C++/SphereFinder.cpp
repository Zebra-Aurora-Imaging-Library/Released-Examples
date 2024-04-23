//***************************************************************************************
// 
// File name: 3dmodmodelfinder.cpp  
//
// Synopsis: This example uses sphere finder to define sphere models and search for spheres
//           in 3D point clouds. A simple sphere finder example is presented first (multiple
//           occurrences in a simple scene), followed by a more complex example (multiple 
//           occurrences in a complex scene with advanced search conditions).
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//****************************************************************************
#include <mil.h>

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_STRING FILENAMES[2] = {M_IMAGE_PATH MIL_TEXT("SphereFinder/Spheres.mbufc"),
                                        M_IMAGE_PATH MIL_TEXT("SphereFinder/ClementineBox.ply")};

//****************************************************************************
// Function Declaration.
//****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
void SimpleSphereRangeFinder(MIL_ID MilSystem, MIL_ID MilDisplay);
void ComplexSphereNominalFinder(MIL_ID MilSystem, MIL_ID MilDisplay);
bool CheckForRequiredMILFile(MIL_STRING FileName);
void AddComponentNormalsIfMissing(MIL_ID MilContainer);
//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Sphere Finder\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to use sphere finder to define\n"));
   MosPrintf(MIL_TEXT("sphere models and search for spheres in 3D point clouds.\n")
             MIL_TEXT("A simple sphere finder example is presented first (multiple\n")
             MIL_TEXT("occurrences in a simple scene), followed by a more complex\n")
             MIL_TEXT("example (multiple occurrences in a complex scene with advanced \n")
             MIL_TEXT("search conditions).\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Application, System, 3D Model Finder, \n")
             MIL_TEXT("3D Image Processing, 3D Display, and 3D Graphics. \n\n"));
   }
//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   // Print Header. 
   PrintHeader();

   MIL_UNIQUE_APP_ID    MilApplication;
   MIL_UNIQUE_SYS_ID    MilSystem;        // System identifier.
   MIL_UNIQUE_3DDISP_ID MilDisplay;      // 3D Mil Display.
   MIL_UNIQUE_BUF_ID    MilContainer;

   // Allocate MIL objects.
   MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MilSystem      = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Check for the required example files.
   if(!CheckForRequiredMILFile(FILENAMES[0]))
      {
      return -1;
      }

   MilDisplay = Alloc3dDisplayId(MilSystem);

   // Run Simple sphere finder example
   SimpleSphereRangeFinder(MilSystem, MilDisplay);

   // Run complex sphere finder example
   ComplexSphereNominalFinder(MilSystem, MilDisplay);

   }
//*****************************************************************************
// Sphere Finder defining a range model
//*****************************************************************************
void SimpleSphereRangeFinder(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   MosPrintf(MIL_TEXT("\nUsing sphere finder in a simple scene:\n"));
   MosPrintf(MIL_TEXT("------------------------------------------\n\n"));

   // Inquire 3D graphics list
   MIL_ID MilGraphicsList = (MIL_ID)M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);

   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a file and displayed.\n\n"));

   // Restore the point cloud.
   auto MilContainer = MbufRestore(FILENAMES[0], MilSystem, M_UNIQUE_ID);

   // Display the point cloud.
   M3ddispSetView(MilDisplay, M_AUTO, M_TOP_TILTED, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   MIL_INT64 Label = M3ddispSelect(MilDisplay, MilContainer, M_ADD, M_DEFAULT);
   M3dgraControl(MilGraphicsList, Label, M_COLOR_USE_LUT, M_TRUE);
   M3dgraControl(MilGraphicsList, Label, M_COLOR_COMPONENT_BAND, 2);
   M3dgraControl(MilGraphicsList, Label, M_COLOR_COMPONENT, M_COMPONENT_RANGE);
   M3ddispSelect(MilDisplay, M_NULL, M_OPEN, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("The point cloud is subsampled to have faster processing.\n"));
   MosPrintf(MIL_TEXT("The subsampling is done while preserving enough points \n")
             MIL_TEXT("for the smallest occurrence.\n\n"));

   auto MilSubsampleContext = M3dimAlloc(MilSystem, M_SUBSAMPLE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
   M3dimControl(MilSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_DECIMATE);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_X, 4);
   M3dimControl(MilSubsampleContext, M_STEP_SIZE_Y, 4);
   M3dimSample(MilSubsampleContext, MilContainer, MilContainer, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a Sphere Finder context.
   auto MilContext = M3dmodAlloc(MilSystem, M_FIND_SPHERE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a Sphere Finder result.
   auto MilResult = M3dmodAllocResult(MilSystem, M_FIND_SPHERE_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Define the sphere model.
   MIL_DOUBLE MinRadius = 2;
   MIL_DOUBLE MaxRadius = 20;
   M3dmodDefine(MilContext, M_ADD, M_SPHERE_RANGE, MinRadius, MaxRadius,
                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT); 

   MosPrintf(MIL_TEXT("A sphere finder is defined with a radii range of (%.1f, %.1f).\n"
                      "False sphere are unlikely in this scene, therefore the certainty is \n"
                      "reduced to speed up the search.\n\n"),
             MinRadius, MaxRadius);

   // Find all ocurrences
   M3dmodControl(MilContext, 0, M_NUMBER, M_ALL);
   M3dmodControl(MilContext, 0, M_CERTAINTY, 80);

   // Preprocess the context.
   M3dmodPreprocess(MilContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));

   // The sphere finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud.
   AddComponentNormalsIfMissing(MilContainer);

   MosPrintf(MIL_TEXT("3D sphere finder is running..\n"));

   // Reset the timer.
   MIL_DOUBLE ComputationTime = 0.0;
   MappTimer(M_TIMER_RESET, M_NULL);

   // Find the model.
   M3dmodFind(MilContext, MilContainer, MilResult, M_DEFAULT);
   MappTimer(M_TIMER_READ, &ComputationTime);

   MIL_INT NumResults = 0;

   MIL_INT Status;
   M3dmodGetResult(MilResult, M_DEFAULT, M_STATUS, &Status);

   switch(Status)
      {
      case M_NOT_INITIALIZED:
         MosPrintf(MIL_TEXT("Sphere finding failed: the result is not initialized.\n\n"));
         break;
      case M_NOT_ENOUGH_MEMORY:
         MosPrintf(MIL_TEXT("Sphere finding failed: not enough memory.\n\n"));
         break;
      case M_NOT_ENOUGH_VALID_DATA:
         MosPrintf(MIL_TEXT("Sphere finding failed: not enough valid points in the point cloud.\n\n"));
         break;
      case M_MISSING_COMPONENT_NORMALS_MIL:
         MosPrintf(MIL_TEXT("Sphere finding failed: M_COMPONENT_NORMALS_MIL is not found in the point cloud.\n\n"));
         break;
      case M_COMPLETE: { M3dmodGetResult(MilResult, M_DEFAULT, M_NUMBER, &NumResults);
                         MosPrintf(MIL_TEXT("Found %i occurrences in %.2f s.\n\n"), NumResults, ComputationTime);
                        }break;
      default:  break;
      }
  
 
   // If a model is found with score above the acceptance.
   if(NumResults > 0)
      { 

      MosPrintf(MIL_TEXT("Index        Error        Score       Radius \n "));
      MosPrintf(MIL_TEXT("---------------------------------------------\n"));

      for(MIL_INT i = 0; i < NumResults; ++i)
         {
         // Get results
         MIL_DOUBLE Error  = M3dmodGetResult(MilResult, i, M_RMS_ERROR, M_NULL);
         MIL_DOUBLE Score  = M3dmodGetResult(MilResult, i, M_SCORE, M_NULL);
         MIL_DOUBLE Radius = M3dmodGetResult(MilResult, i, M_RADIUS, M_NULL);

         // Print the results for each sphere found.
         MosPrintf(MIL_TEXT("  %02i          %.2f        %6.2f       %5.2f       \n"),
                   i, Error, Score, Radius);
         }
   
      // Draw all occurrences by the default draw3d context.
      M3dmodDraw3d(M_DEFAULT, MilResult, M_ALL, MilGraphicsList, M_DEFAULT, M_DEFAULT);

      }
   MosPrintf(MIL_TEXT("\nPress <Enter> for the next example.\n\n"));
   MosGetch();
   }
//*********************************************************************************
// Sphere Model finder defining a nominal model and tolerance
//*********************************************************************************
void ComplexSphereNominalFinder(MIL_ID MilSystem, MIL_ID MilDisplay)
   {
   // Remove previous example
   MIL_ID MilGraphicsList = (MIL_ID)M3ddispInquire(MilDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3dgraRemove(MilGraphicsList, M_ALL, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nUsing sphere finder in a complex scene:\n"));
   MosPrintf(MIL_TEXT("------------------------------------------\n\n"));

   // Restore the point cloud.
   auto MilContainer = MbufRestore(FILENAMES[1], MilSystem, M_UNIQUE_ID);

   // Display the point cloud.
   M3ddispSetView(MilDisplay, M_AZIM_ELEV_ROLL, 102.55, 75, 12.5, M_DEFAULT);
   MIL_INT64 Label = M3ddispSelect(MilDisplay, MilContainer, M_SELECT, M_DEFAULT);
   M3ddispSetView(MilDisplay, M_AUTO, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3dgraControl(MilGraphicsList, Label, M_APPEARANCE, M_POINTS);

   MosPrintf(MIL_TEXT("A 3D point cloud is restored from a file and displayed.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a sphere finder context.
   auto MilContext = M3dmodAlloc(MilSystem, M_FIND_SPHERE_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a Sphere Finder result.
   auto MilResult  = M3dmodAllocResult(MilSystem, M_FIND_SPHERE_RESULT, M_DEFAULT, M_UNIQUE_ID);
 
   // Define the sphere model.
   MIL_DOUBLE NominalRadius   = 25;
   MIL_DOUBLE Tolerance = 1;
   M3dmodDefine(MilContext, M_ADD, M_SPHERE, NominalRadius, Tolerance,
                M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("A sphere finder is defined with a nominal radius of %.1f +/- %.0f.\n\n"),
             NominalRadius, Tolerance);

   MIL_DOUBLE MaxCoverage = 40;
   MIL_DOUBLE NormalDistance = 15;
   M3dmodControl(MilContext, 0, M_NUMBER    , M_ALL);
   M3dmodControl(MilContext, 0, M_COVERAGE_MAX, MaxCoverage);
   M3dmodControl(MilContext, 0, M_CERTAINTY, 80);
   M3dmodControl(MilContext, M_DEFAULT, M_FIT_NORMALS_DISTANCE, NormalDistance);

   MosPrintf(MIL_TEXT("Key Controls \n"));
   MosPrintf(MIL_TEXT("-------------------------\n"));
   MosPrintf(MIL_TEXT(" M_COVERAGE_MAX        : %.0f%%\n")  , MaxCoverage);
   MosPrintf(MIL_TEXT(" M_FIT_NORMALS_DISTANCE: %.0f\xF8 \n\n"), NormalDistance);

   MosPrintf(MIL_TEXT("In this point cloud, the fruits are not ideal spheres with low coverage.\n")
             MIL_TEXT("The M_COVERAGE_MAX is set to %.0f. This point cloud is noisy, and the\n")
             MIL_TEXT("created normals are not precise, so M_FIT_NORMALS_DISTANCE is set to")
             MIL_TEXT(" %.0f.\n\n"), MaxCoverage, NormalDistance);

   // Preprocess the context.
   M3dmodPreprocess(MilContext, M_DEFAULT);

   MosPrintf(MIL_TEXT("M_COMPONENT_NORMALS_MIL is added to the point cloud if not present.\n\n"));

   // The sphere finder requires the existence of M_COMPONENT_NORMALS_MIL in the point cloud.
   AddComponentNormalsIfMissing(MilContainer);

   MosPrintf(MIL_TEXT("3D sphere finder is running..\n"));

   // Reset the timer.
   MIL_DOUBLE ComputationTime = 0.0;
   MappTimer(M_TIMER_RESET, M_NULL);

   // Find the model.
   M3dmodFind(MilContext, MilContainer, MilResult, M_DEFAULT);

   // Read the find time.
   MappTimer(M_TIMER_READ, &ComputationTime);

   MIL_INT NumResults = 0;
   M3dmodGetResult(MilResult, M_DEFAULT, M_NUMBER, &NumResults);
   MosPrintf(MIL_TEXT("Found %i occurrences in %.2f s.\n\n"), NumResults, ComputationTime);

   // If a model is found with score abvoe the acceptance.
   if(NumResults > 0)
      {

      MosPrintf(MIL_TEXT("Index        Error        Score       Radius \n "));
      MosPrintf(MIL_TEXT("---------------------------------------------\n"));

      for(MIL_INT i = 0; i < NumResults; ++i)
         {
         // Get results
         MIL_DOUBLE Error  = M3dmodGetResult(MilResult, i, M_RMS_ERROR, M_NULL);
         MIL_DOUBLE Score  = M3dmodGetResult(MilResult, i, M_SCORE, M_NULL);
         MIL_DOUBLE Radius = M3dmodGetResult(MilResult, i, M_RADIUS, M_NULL);

         // Print the results for each sphere found.
         MosPrintf(MIL_TEXT("  %02i          %.2f        %6.2f      %5.2f       \n"),
                   i, Error, Score, Radius);

         }

      // Draw all occurrences' reserved points.
      auto MilDrawContext = M3dmodAlloc(MilSystem, M_DRAW_3D_GEOMETRIC_CONTEXT, M_DEFAULT, M_UNIQUE_ID);
      M3dmodControlDraw(MilDrawContext, M_DRAW_MODEL, M_ACTIVE, M_DISABLE);
      M3dmodControlDraw(MilDrawContext, M_DRAW_BOX, M_COLOR, M_COLOR_CYAN);
      M3dmodControlDraw(MilDrawContext, M_DRAW_RESERVED_POINTS, M_ACTIVE, M_ENABLE);
      M3dmodControlDraw(MilDrawContext, M_DRAW_RESERVED_POINTS, M_THICKNESS, 3);
      M3dmodDraw3d(MilDrawContext, MilResult, M_ALL, MilGraphicsList, M_DEFAULT, M_DEFAULT);
      }
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
//*****************************************************************************
void AddComponentNormalsIfMissing(MIL_ID MilContainer)
   {
   MIL_ID MilNormals =
      MbufInquireContainer(MilContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);

   if(MilNormals != M_NULL)
      return;
   MIL_INT SizeX = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   if(SizeX < 50 || SizeY < 50)
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, M_DEFAULT);
   else
      M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, M_DEFAULT);
   }
//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto MilDisplay3D =
      M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();
      }
   return MilDisplay3D;
   }
//*******************************************************************************
// Checks the required files exist.
//*****************************************************************************
bool CheckForRequiredMILFile(MIL_STRING FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);

   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }
