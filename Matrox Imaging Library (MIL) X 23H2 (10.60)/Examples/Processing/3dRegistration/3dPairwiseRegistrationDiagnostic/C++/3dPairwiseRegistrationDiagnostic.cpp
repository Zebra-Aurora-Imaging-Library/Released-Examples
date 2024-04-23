﻿//***************************************************************************************/
//
// File name: 3dPairwiseRegistrationDiagnostic.cpp
//
// Synopsis:  This program is both an example and a tool for diagnosing the pairwise 3D
//            registration process. It can draw 3D registration results using example data
//            or using an .m3dreg result file that is supplied interactively or as a
//            command argument.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include <map>
#include <cctype>

// Source file specification.
static const MIL_INT NB_POINT_CLOUDS = 2;
static const MIL_STRING REG_POINT_CLOUD_FILES[]
   {
   M_IMAGE_PATH MIL_TEXT("3dModelHeightDefect/3dModel.ply"),
   M_IMAGE_PATH MIL_TEXT("3dModelHeightDefect/3dObject.mbufc"),
   };

// Function declarations.
void CheckForRequiredMILFile(const MIL_STRING& FileName);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
bool AskYesNo(MIL_CONST_TEXT_PTR QuestionString);

MIL_UNIQUE_3DREG_ID ObtainRegistrationResult(MIL_STRING& ResultFile,
                                             std::vector<MIL_UNIQUE_BUF_ID>& MilPointClouds,
                                             std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays);
MIL_UNIQUE_3DREG_ID RestoreRegistrationResult(MIL_CONST_TEXT_PTR ResultFilename);
MIL_UNIQUE_3DREG_ID GenerateRegistrationResult(std::vector<MIL_UNIQUE_BUF_ID>& MilPointClouds,
                                               std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays);
void                ShowRegistrationSources(const std::vector<MIL_ID>& MilPointClouds,
                                            std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays);
MIL_UNIQUE_3DREG_ID CalculateRegistrationResult(const std::vector<MIL_ID>& MilPointClouds);

void PrintRegistrationStatus(MIL_ID Mil3dregResult);

void ToggleDrawControl(MIL_ID MilDrawContext, MIL_INT Draw, MIL_INT Control);
MIL_CONST_TEXT_PTR DrawInquireString(MIL_ID MilDrawContext, MIL_INT Draw, MIL_INT Control);

// Constants.
static const MIL_INT       SOURCE_DISPLAY_SIZE = 285;
static const MIL_INT       SOURCE_DISPLAY_SPACING = 30;
static const MIL_TEXT_CHAR ESC_KEY = 27;

// Registration context control constants.
static const MIL_DOUBLE GRID_SIZE = 1;
static const MIL_DOUBLE OVERLAP = 100; // %
static const MIL_INT    MAX_ITERATIONS = 100;
static const MIL_DOUBLE RMS_ERROR_RELATIVE_THRESHOLD = 0.5;  // %

// Utility structure that holds the diagnostic display settings.
struct SDisplaySettings
   {
   bool ModifyDisplay(MIL_ID MilDrawContext, MIL_INT MaxElement);

   MIL_STRING IterationStr() const { return IsLastIteration ? MIL_STRING(MIL_TEXT("Last")) : M_TO_STRING(Iteration); }
   MIL_STRING ElementStr()   const { return IsAllElement ? MIL_TEXT("All") : M_TO_STRING(Element); }
   MIL_STRING RankStr()      const { return IsAllRank ? MIL_TEXT("All") : M_TO_STRING(Rank); }

   MIL_INT ElementParam()   const { return IsAllElement ? M_ALL : Element; }
   MIL_INT RankParam()      const { return IsAllRank ? M_ALL : Rank; }
   MIL_INT IterationParam() const { return IsLastIteration ? M_LAST_ITERATION : Iteration; }
   
   MIL_INT Element      = 1;
   bool IsAllElement    = false;
   MIL_INT Iteration    = 0;
   bool IsLastIteration = false;
   MIL_INT Rank         = 0;
   bool IsAllRank       = false;
   };

// Utility maps.
static const std::map<MIL_INT64, MIL_INT64> TOGGLE_MAP =
   {
      {M_ENABLE , M_DISABLE},
      {M_DISABLE, M_ENABLE},
      {M_TRUE   , M_FALSE},
      {M_FALSE  , M_TRUE}
   };

static const std::map<MIL_INT64, MIL_CONST_TEXT_PTR> INQUIRE_STRING_MAP =
   {
      {M_ENABLE , MIL_TEXT("enable")},
      {M_DISABLE, MIL_TEXT("disable")},
      {M_TRUE   , MIL_TEXT("true")},
      {M_FALSE  , MIL_TEXT("false")}
   };

static const std::map<MIL_INT, MIL_CONST_TEXT_PTR> REG_STATUS_STRINGS =
   {
      {M_REGISTRATION_GLOBAL                 , MIL_TEXT("Registration global")},
      {M_NOT_INITIALIZED                     , MIL_TEXT("Not initialized")},
      {M_NOT_ENOUGH_POINT_PAIRS              , MIL_TEXT("Not enough pairs")},
      {M_MAX_ITERATIONS_REACHED              , MIL_TEXT("Max iterations reached")},
      {M_RMS_ERROR_THRESHOLD_REACHED         , MIL_TEXT("RMS error threshold reached")},
      {M_RMS_ERROR_RELATIVE_THRESHOLD_REACHED, MIL_TEXT("RMS error relative threshold reached")},
   };

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("3dPairwiseRegistrationDiagnostic\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program is both an example and a tool for diagnosing the pairwise 3D\n")
             MIL_TEXT("registration process. It can draw 3D registration results using example data\n")
             MIL_TEXT("or using an .m3dreg result file that is supplied interactively or as a\n")
             MIL_TEXT("command argument.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Display, 3D Geometry, 3D Graphics, 3D Image Processing\n")
             MIL_TEXT("and Buffer.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(int argc, MIL_TEXT_CHAR* argv[])
   {
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Allocate the diagnostic display.
   std::vector<MIL_UNIQUE_3DDISP_ID> MilDisplays;
   MilDisplays.push_back(Alloc3dDisplayId(M_DEFAULT_HOST));
   MIL_ID MilDiagDisplay = MilDisplays[0];
   M3ddispControl(MilDiagDisplay, M_TITLE, MIL_TEXT("Diagnostic Display"));

   MIL_STRING ResultFile = MIL_TEXT("");
   if(argc >= 2)
      ResultFile = argv[1];
   else
      PrintHeader();

   // Restore the registration result.
   std::vector<MIL_UNIQUE_BUF_ID> MilPointClouds(NB_POINT_CLOUDS);
   MIL_UNIQUE_3DREG_ID Mil3dregResult = ObtainRegistrationResult(ResultFile, MilPointClouds, MilDisplays);

   // Print the status of the registration elements.
   PrintRegistrationStatus(Mil3dregResult);

   // Initialize the diagnostic display.
   MIL_ID MilDiagGraList = (MIL_ID)M3ddispInquire(MilDiagDisplay, M_3D_GRAPHIC_LIST_ID, M_NULL);
   M3ddispSetView(MilDiagDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);
   M3ddispSelect(MilDiagDisplay, M_NULL, M_OPEN, M_DEFAULT);

   // Allocate the draw 3D context.
   auto MilDrawContext = M3dregAlloc(M_DEFAULT_HOST, M_DRAW_3D_CONTEXT, M_DEFAULT, M_UNIQUE_ID);

   // Set up some drawing parameters.
   M3dregControlDraw(MilDrawContext, M_DRAW_OVERLAP_POINTS, M_THICKNESS, 3);
   M3dregControlDraw(MilDrawContext, M_DRAW_EXCLUDED_POINTS, M_THICKNESS, 3);
   M3dregControlDraw(MilDrawContext, M_DRAW_PAIRS, M_ACTIVE, M_ENABLE);

   SDisplaySettings Settings;
   MIL_INT NbElement;
   M3dregGetResult(Mil3dregResult, M_GENERAL, M_NUMBER_OF_REGISTRATION_ELEMENTS, &NbElement);
   do
      {
      // Print the menu.
      MosPrintf(MIL_TEXT("DIAGNOSTIC DISPLAY CONTROLS\n"));
      MosPrintf(MIL_TEXT("---------------------------------\n"));
      MosPrintf(MIL_TEXT("Toggle Overlap Points   (o)    \n"));
      MosPrintf(MIL_TEXT("Toggle Overlap Lut      (l)    \n"));
      MosPrintf(MIL_TEXT("Toggle Excluded Points  (e)    \n"));
      MosPrintf(MIL_TEXT("Toggle Pairs Lines      (p)    \n"));
      MosPrintf(MIL_TEXT("Change Iteration       (+/-)   \n"));
      MosPrintf(MIL_TEXT("First Iteration         (f)    \n"));
      MosPrintf(MIL_TEXT("Last Iteration          (g)    \n"));
      MosPrintf(MIL_TEXT("Change Pair Rank       (b/n)   \n"));
      MosPrintf(MIL_TEXT("All Pairs Rank          (m)    \n"));
      MosPrintf(MIL_TEXT("Change Reg Element     (z/x)   \n"));
      MosPrintf(MIL_TEXT("All Reg Elements        (c)    \n"));
      MosPrintf(MIL_TEXT("Exit                   (esc)   \n\n"));

      MosPrintf(MIL_TEXT("Currently displaying...\n"));
      MosPrintf(MIL_TEXT("Overlap      = %s\n"),
                DrawInquireString(MilDrawContext, M_DRAW_OVERLAP_POINTS , M_ACTIVE));
      MosPrintf(MIL_TEXT("Overlap lut  = %s\n"),
                DrawInquireString(MilDrawContext, M_DRAW_OVERLAP_POINTS , M_COLOR_USE_LUT));
      MosPrintf(MIL_TEXT("Excluded     = %s\n"),
                DrawInquireString(MilDrawContext, M_DRAW_EXCLUDED_POINTS, M_ACTIVE));
      MosPrintf(MIL_TEXT("Pairs        = %s\n"),
                DrawInquireString(MilDrawContext, M_DRAW_PAIRS          , M_ACTIVE ));
      MosPrintf(MIL_TEXT("Iteration    = %s\n"), Settings.IterationStr().c_str());
      MosPrintf(MIL_TEXT("Rank         = %s\n"), Settings.RankStr().c_str());
      MosPrintf(MIL_TEXT("Element      = %s\n\n"), Settings.ElementStr().c_str());

      // Render the diagnostic display.
      M3ddispControl(MilDiagDisplay, M_UPDATE, M_DISABLE);
      M3dgraRemove(MilDiagGraList, M_ALL, M_DEFAULT);
      auto DrawNode = M3dregDraw3d(MilDrawContext, Mil3dregResult,
                                   Settings.ElementParam(), Settings.IterationParam(), Settings.RankParam(),
                                   MilDiagGraList, M_DEFAULT, M_DEFAULT);
      M3dgraControl(MilDiagGraList, DrawNode, M_APPEARANCE + M_RECURSIVE, M_POINTS);
      M3ddispControl(MilDiagDisplay, M_UPDATE, M_ENABLE);

      } while(Settings.ModifyDisplay(MilDrawContext, NbElement-1));

   return 0;
   }

//****************************************************************************
// Obtains a 3D registration result, either from file or from calculating
// example data.
//****************************************************************************
MIL_UNIQUE_3DREG_ID ObtainRegistrationResult(MIL_STRING& RegistrationResultFile,
                                             std::vector<MIL_UNIQUE_BUF_ID>& MilPointClouds,
                                             std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays)
   {
   MIL_UNIQUE_3DREG_ID Mil3dregResult;
   do
      {
      if(RegistrationResultFile != MIL_TEXT(""))
         Mil3dregResult = RestoreRegistrationResult(RegistrationResultFile.c_str());
      else if(AskYesNo(MIL_TEXT("Do you want to load a user 3dreg result")))
         {
         MosPrintf(MIL_TEXT("Please select an .m3dreg result file.\n")
                   MIL_TEXT("The result must have been calculated with M_SAVE_PAIRS_INFO set to M_TRUE.\n\n"));
         Mil3dregResult = RestoreRegistrationResult(M_NULL);
         }
      else
         Mil3dregResult = GenerateRegistrationResult(MilPointClouds, MilDisplays);
      } while(!Mil3dregResult);

   return Mil3dregResult;
   }

//****************************************************************************
// Restores the registration result from file.
//****************************************************************************
MIL_UNIQUE_3DREG_ID RestoreRegistrationResult(MIL_CONST_TEXT_PTR ResultFilename)
   {
   // Restore the 3dreg result.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto Mil3dregResult = M3dregRestore(ResultFilename, M_DEFAULT_HOST, M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // The restored object must be a 3dreg result.
   if(Mil3dregResult &&
      MobjInquire(Mil3dregResult, M_OBJECT_TYPE, M_NULL) != M_3DREG_PAIRWISE_REGISTRATION_RESULT)
      {
      Mil3dregResult.reset();
      }

   if(!Mil3dregResult)
      MosPrintf(MIL_TEXT("No valid .m3dreg result file restored.\n\n"));
   else if(M3dregGetResult(Mil3dregResult, M_GENERAL, M_SAVE_PAIRS_INFO, M_NULL) == M_FALSE)
      {
      MosPrintf(MIL_TEXT("The selected .m3dreg result file doesn't contain the pairs information.\n")
                MIL_TEXT("Please regenerate the result with M_SAVE_PAIRS_INFO set to M_TRUE.\n\n"));

      Mil3dregResult.reset();
      }

   return Mil3dregResult;
   }

//****************************************************************************
// Generates the registration result from the example source data.
//****************************************************************************
MIL_UNIQUE_3DREG_ID GenerateRegistrationResult(std::vector<MIL_UNIQUE_BUF_ID>& MilPointClouds,
                                               std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays)
   {
   MosPrintf(MIL_TEXT("The example will run using a 3dreg result calculated from example source data.\n\n"));

   // Restore the point cloud containers from files.
   for(MIL_INT i = 0; i < NB_POINT_CLOUDS; i++)
      {
      CheckForRequiredMILFile(REG_POINT_CLOUD_FILES[i]);
      MilPointClouds[i] = MbufImport(REG_POINT_CLOUD_FILES[i], M_DEFAULT, M_RESTORE, M_DEFAULT_HOST, M_UNIQUE_ID);
      }
   std::vector<MIL_ID> MilPointCloudsId(MilPointClouds.begin(), MilPointClouds.end());

   // Show the source data.
   ShowRegistrationSources(MilPointCloudsId, MilDisplays);

   // Move the diagnostic display.
   M3ddispControl(MilDisplays[0], M_WINDOW_INITIAL_POSITION_X, SOURCE_DISPLAY_SIZE);

   // Generate the result.
   return CalculateRegistrationResult(MilPointCloudsId);
   }

//****************************************************************************
// Modifies the drawings in the 3D display according to the key pressed.
//****************************************************************************
bool SDisplaySettings::ModifyDisplay(MIL_ID MilDrawContext, MIL_INT MaxElement)
   {
   while(1)
      {
      auto Key = (MIL_TEXT_CHAR)std::toupper((int)MosGetch());

      switch(Key)
         {
         case MIL_TEXT('O'):
            ToggleDrawControl(MilDrawContext, M_DRAW_OVERLAP_POINTS, M_ACTIVE);
            return true;
         case MIL_TEXT('E'):
            ToggleDrawControl(MilDrawContext, M_DRAW_EXCLUDED_POINTS, M_ACTIVE);
            return true;
         case MIL_TEXT('L'):
            ToggleDrawControl(MilDrawContext, M_DRAW_OVERLAP_POINTS, M_COLOR_USE_LUT);
            return true;
         case MIL_TEXT('P'):
            ToggleDrawControl(MilDrawContext, M_DRAW_PAIRS, M_ACTIVE);
            return true;
         case MIL_TEXT('+'):
            Iteration++;
            IsLastIteration = false;
            return true;
         case MIL_TEXT('-'):
            if(Iteration > 0)
               Iteration--;
            IsLastIteration = false;
            return true;
         case MIL_TEXT('F'):
            Iteration = 0;
            IsLastIteration = false;
            return true;
         case MIL_TEXT('G'):
            IsLastIteration = true;
            return true;
         case MIL_TEXT('N'):
            if(!IsAllRank)
               Rank++;
            return true;
         case MIL_TEXT('B'):
            if(Rank > 0 && !IsAllRank)
               Rank--;
            return true;
         case MIL_TEXT('M'):
            IsAllRank = !IsAllRank;
            return true;
         case MIL_TEXT('Z'):
            if(Element > 0)
               Element--;
            return true;
         case MIL_TEXT('X'):
            if(Element < MaxElement)
               Element++;
            return true;
         case MIL_TEXT('C'):
            IsAllElement = !IsAllElement;
            return true;
         case ESC_KEY:
            return false;
         default:
            break;
         }
      }
   }

//****************************************************************************
// Toggles a control value.
//****************************************************************************
void ToggleDrawControl(MIL_ID MilDrawContext, MIL_INT Draw, MIL_INT Control)
   {   
   auto Value = M3dregInquireDraw(MilDrawContext, Draw, Control, M_NULL);
   M3dregControlDraw(MilDrawContext, Draw, Control, TOGGLE_MAP.at(Value));
   }

//****************************************************************************
// Inquires the string mapped to a draw 3D control type setting.
//****************************************************************************
MIL_CONST_TEXT_PTR DrawInquireString(MIL_ID MilDrawContext, MIL_INT Draw, MIL_INT Control)
   {   
   auto Value = M3dregInquireDraw(MilDrawContext, Draw, Control, M_NULL);
   return INQUIRE_STRING_MAP.at(Value);
   }

//****************************************************************************
// Checks for required files to run the example.
//****************************************************************************
void CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
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
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
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
   return MilDisplay3D;
   }

//****************************************************************************
// Shows the source point clouds used by the 3D registration.
//****************************************************************************
void ShowRegistrationSources(const std::vector<MIL_ID>& MilPointClouds,
                             std::vector<MIL_UNIQUE_3DDISP_ID>& MilDisplays)
   {
   MIL_INT DisplayOffsetY = 0;
   for(MIL_INT p = 0; p < (MIL_INT)MilPointClouds.size(); p++)
      {
      auto MilDisplay = Alloc3dDisplayId(M_DEFAULT_HOST);
      M3ddispControl(MilDisplay, M_SIZE_X, SOURCE_DISPLAY_SIZE);
      M3ddispControl(MilDisplay, M_SIZE_Y, SOURCE_DISPLAY_SIZE);
      M3ddispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, DisplayOffsetY);
      M3ddispSetView(MilDisplay, M_AUTO, M_BOTTOM_VIEW, M_DEFAULT, M_DEFAULT, M_DEFAULT);

      M3ddispControl(MilDisplay, M_UPDATE, M_DISABLE);
      M3ddispSelect(MilDisplay, MilPointClouds[p], M_SELECT, M_DEFAULT);
      M3ddispLut(MilDisplay, M_DEFAULT, M_COLORMAP_TURBO + M_FLIP, M_DEFAULT);
      M3ddispControl(MilDisplay, M_UPDATE, M_ENABLE);

      DisplayOffsetY += SOURCE_DISPLAY_SIZE + SOURCE_DISPLAY_SPACING;

      if(p == 0)
         M3ddispControl(MilDisplay, M_TITLE, MIL_TEXT("Reference"));
      else
         {
         MIL_STRING TargetDisplayName = MIL_STRING(MIL_TEXT("Target")) + M_TO_STRING(p);
         M3ddispControl(MilDisplay, M_TITLE, TargetDisplayName);
         }

      MilDisplays.push_back(std::move(MilDisplay));
      }

   MosPrintf(MIL_TEXT("The source point clouds of the 3D registration are displayed.\n\n"));
   }

//****************************************************************************
// Calculates the registration result.
//****************************************************************************
MIL_UNIQUE_3DREG_ID CalculateRegistrationResult(const std::vector<MIL_ID>& MilPointClouds)
   {
   MosPrintf(MIL_TEXT("Calculating the 3dreg registration result...\n\n"));

   auto Mil3dregContext = M3dregAlloc(M_DEFAULT_HOST, M_PAIRWISE_REGISTRATION_CONTEXT,
                                      M_DEFAULT, M_UNIQUE_ID);
   auto Mil3dregResult = M3dregAllocResult(M_DEFAULT_HOST, M_PAIRWISE_REGISTRATION_RESULT,
                                           M_DEFAULT, M_UNIQUE_ID);

   // Subsampling context of the registration.
   MIL_ID MilRegSubsampleContext = M_NULL;
   M3dregInquire(Mil3dregContext, M_DEFAULT, M_SUBSAMPLE_CONTEXT_ID, &MilRegSubsampleContext);

   // Set the subsampling controls that will be used during the registration process.
   M3dimControl(MilRegSubsampleContext, M_SUBSAMPLE_MODE, M_SUBSAMPLE_GRID);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_X, GRID_SIZE);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_Y, GRID_SIZE);
   M3dimControl(MilRegSubsampleContext, M_GRID_SIZE_Z, M_INFINITE);
   M3dimControl(MilRegSubsampleContext, M_ORGANIZATION_TYPE, M_ORGANIZED);

   // Pairwise 3D registration context controls.
   M3dregControl(Mil3dregContext, M_DEFAULT, M_SUBSAMPLE, M_ENABLE);
   M3dregControl(Mil3dregContext, M_DEFAULT, M_PREREGISTRATION_MODE, M_CENTROID);
   M3dregControl(Mil3dregContext, 1, M_OVERLAP, OVERLAP);
   M3dregControl(Mil3dregContext, M_DEFAULT, M_MAX_ITERATIONS, MAX_ITERATIONS);
   M3dregControl(Mil3dregContext, M_DEFAULT,
                 M_RMS_ERROR_RELATIVE_THRESHOLD, RMS_ERROR_RELATIVE_THRESHOLD);
   M3dregControl(Mil3dregContext, M_CONTEXT, M_ERROR_MINIMIZATION_METRIC, M_POINT_TO_POINT);

   M3dregControl(Mil3dregContext, M_CONTEXT, M_PAIRS_CREATION_PER_REFERENCE_POINT_MODE, M_AUTO);
   M3dregControl(Mil3dregContext, M_ALL, M_PAIRS_REJECTION_MODE, M_ROBUST_STANDARD_DEVIATION);
   M3dregControl(Mil3dregContext, M_ALL, M_PAIRS_REJECTION_FACTOR, 4);
   M3dregControl(Mil3dregContext, M_CONTEXT, M_SAVE_PAIRS_INFO, M_TRUE);

   // Perform the 3D registration.
   M3dregCalculate(Mil3dregContext, MilPointClouds, M_DEFAULT, Mil3dregResult, M_DEFAULT);

   return Mil3dregResult;
   }

//****************************************************************************
// Generates the 3D registration result.
//****************************************************************************
void PrintRegistrationStatus(MIL_ID Mil3dregResult)
   {
   MIL_INT NbElement;
   M3dregGetResult(Mil3dregResult, M_GENERAL, M_NUMBER_OF_REGISTRATION_ELEMENTS, &NbElement);

   // Check if the registration was successful.
   for(MIL_INT e = 0; e < (MIL_INT)NbElement; e++)
      {
      MIL_INT RegistrationStatus;
      M3dregGetResult(Mil3dregResult, e, M_STATUS_REGISTRATION_ELEMENT, &RegistrationStatus);      

      MosPrintf(MIL_TEXT("Element %d status : %s\n"), e, REG_STATUS_STRINGS.at(RegistrationStatus));
      }
   MosPrintf(MIL_TEXT("\n"));
   }

//*****************************************************************************
// Prompts user for yes/no
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
