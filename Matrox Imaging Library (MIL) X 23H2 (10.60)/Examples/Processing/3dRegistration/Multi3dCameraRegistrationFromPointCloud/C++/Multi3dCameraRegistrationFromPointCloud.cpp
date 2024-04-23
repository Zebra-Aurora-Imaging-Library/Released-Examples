//****************************************************************************
// 
// File name: Multi3dCameraRegistrationFromPointCloud.cpp
// 
// Synopsis:  This example demonstrates how to rigidly align data from multiple 3D cameras.
//            If real cameras are used, the program discovers them on the network;
//            otherwise, 3D data is loaded from disk using a location that is predefined by
//            the user.
//
//            Once the 3D data are acquired, the best rigid transformation is found using either:
//            -A model-less method, which is a rigid alignment based on overlapping data;
//            -A model based method, which is a rigid alignment of the data with the reference model
//            of the scanned object.
//            
//            Then, if real cameras are used, the application sets the SFNC transformation
//            parameters(Rx, Ry, Rz, Tx, Ty, Tz). A final acquisition can be done to validate
//            that the alignment of the multiple 3D cameras data works correctly.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//****************************************************************************
#include <mil.h>

// Example images paths.
#define EXAMPLE_IMAGE_PATH M_IMAGE_PATH MIL_TEXT("Multi3dCameraRegistrationFromPointCloud/")
static std::vector<MIL_STRING> EXAMPLE_IMAGES =
   {
   EXAMPLE_IMAGE_PATH MIL_TEXT("Left3dProfiler.mbufc"),
   EXAMPLE_IMAGE_PATH MIL_TEXT("Right3dProfiler.mbufc")
   };

#define ALIGN_MODEL_NAME MIL_TEXT("AlignModel.mbufc");
static MIL_CONST_TEXT_PTR ALIGN_MODEL_FILE = EXAMPLE_IMAGE_PATH ALIGN_MODEL_NAME;

static MIL_CONST_TEXT_PTR USER_DATA_PATH = MIL_TEXT("Processing/3dRegistration/Multi3dCameraRegistrationFromPointCloud/User3dData/");

// Utility functions used by the example.
#include "ExampleUtil.h"

// Alignment displays of the default sample images.
const SSegment EXAMPLE_ALIGN_DISPLAY_INIT[] =
   {
      {825, 264, 823, 483},
      {239, 252, 238, 468},
      {281, 177, 280, 398}
   };

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("Multi3dCameraRegistrationFromPointCloud\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example demonstrates how to rigidly align data from multiple 3D cameras.\n"));
   MosPrintf(MIL_TEXT("If real cameras are used, the program discovers them on the network;\n"));
   MosPrintf(MIL_TEXT("otherwise, 3D data is loaded from disk using a location that is predefined by\n"));
   MosPrintf(MIL_TEXT("the user.\n\n"));

   MosPrintf(MIL_TEXT("Once the 3D data are acquired, the rigid transformation is found using either:\n"));
   MosPrintf(MIL_TEXT("-A model-less method, which is a rigid alignment based on overlapping data;\n"));
   MosPrintf(MIL_TEXT("-A model based method, which is a rigid alignment of the data with the\n"));
   MosPrintf(MIL_TEXT(" reference model of the scanned object.\n\n"));

   MosPrintf(MIL_TEXT("Then, if real cameras are used, the application sets the SFNC transformation\n"));
   MosPrintf(MIL_TEXT("parameters(Rx, Ry, Rz, Tx, Ty, Tz). A final acquisition can be done to validate\n"));
   MosPrintf(MIL_TEXT("that the alignment of the multiple 3D cameras data works correctly.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: 3D Registration, 3D Geometry, 3D Metrology,\n")
             MIL_TEXT("3D Image Processing, 3D Display, Buffer, Calibration, Digitizer,\n")
             MIL_TEXT("Display, Graphics, Image Processing\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//****************************************************************************
// Functions.
//****************************************************************************
int Terminate(MIL_CONST_TEXT_PTR Message);
bool Reset3dCamerasCoordinateSystems(const std::vector<MIL_ID>& MilDigitizers);
bool Setup3dCamerasCoordinateSystems(const std::vector<MIL_ID>& MilDigitizers, const std::vector<MIL_ID>& MilAlignmentMatrices);

//****************************************************************************
// Constants.
//****************************************************************************
// Example methods.
enum ExampleMode
   {
   AlignOverlap = 0,
   AlignModel,
   AlignedSource,
   AlignedUsingMatrix
   };
static std::vector<MIL_CONST_TEXT_PTR> const AlignmentMethodsNames = {MIL_TEXT("Model-less alignment computation (based on overlapping data)"),
                                                                      MIL_TEXT("Model-based alignment computation (against a reference model)")};

static const MIL_DOUBLE MIN_NORMAL_ANGLE = 0;
//****************************************************************************
// Main.                                                          
//****************************************************************************
int MosMain()
   {
   // Allocate the application.
   auto MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);

   // Print the header of the application.
   PrintHeader();

   // Get the source of the data.
   auto DataSource = GetCameraDataSource();
   
   // Allocate the system.
   MIL_UNIQUE_SYS_ID MilSystem;
   if(DataSource == Camera3dDataSource::Cameras)
      {
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
      MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_GIGE_VISION, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
      if(!MilSystem)
         {
         MosPrintf(MIL_TEXT("\n")
                   MIL_TEXT("Unable to allocate M_SYSTEM_GIGE_VISION system.\n")
                   MIL_TEXT("Please make sure that it is correctly installed.\n")
                   MIL_TEXT("Press <Enter> to end.\n"));
         MosGetch();
         exit(0);
         }
      }
   else
      MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   // Allocate a 3d display.
   auto MilComplete3dDisp = Allocate3dDisplay(MilSystem);

   if(DataSource == Camera3dDataSource::Example)
      {
      for(const auto& imageFile : EXAMPLE_IMAGES)
         {
         if(!CheckForRequiredMILFile(imageFile))
            return 0;
         }
      if(!CheckForRequiredMILFile(ALIGN_MODEL_FILE))
         return 0;
      }

   // Allocate the 3d cameras from the chosen data source.
   auto Cameras3d = Allocate3dCameras(MilSystem, DataSource);

   // Terminate if there isn't enough cameras.
   if(Cameras3d.size() < 2)
      {
      if(DataSource == Camera3dDataSource::Cameras)
         return Terminate(MIL_TEXT("At least 2 3D cameras must be used!\nConnect more 3D cameras and restart the application."));
      else
         return Terminate(MIL_TEXT("At least 2 3D containers must be used!\nAcquire more 3D data and restart the application."));
      }

   // Get the digitizers' ids in a separate array.
   std::vector<MIL_ID> MilDigitizers(Cameras3d.begin(), Cameras3d.end());

   // Check the names of the 3d cameras.
   if(DataSource == Camera3dDataSource::Cameras)
      {
      if(!CheckCamerasDeviceUserIds(MilDigitizers))
         return Terminate(MIL_TEXT("The DeviceUserId of the 3D cameras must be set and unique!\n")
                          MIL_TEXT("Set the DeviceUserId of the 3D cameras and restart the application."));
      }

   // Get the synchronization configuration of the 3d cameras. Change them if required.
   auto SyncConfig = GetSyncConfig(MilDigitizers);

   MosPrintf(MIL_TEXT("----------------------------------------\n\n"));

   // Declare the alignment matrices vector.
   std::vector<MIL_UNIQUE_3DGEO_ID> AlignmentMatrices = RestorePreviousAlignmentMatrices(MilSystem, MilDigitizers, DataSource);

   // Ask which alignment method to use.
   auto ExampleModeChoices = AlignmentMethodsNames;
   if(DataSource == Camera3dDataSource::Cameras)
      {
      ExampleModeChoices.push_back(MIL_TEXT("Acquisition test with aligned source"));
      if(AlignmentMatrices.size() > 0)
         ExampleModeChoices.push_back(MIL_TEXT("Acquisition test with matrices from previous alignment"));
      }

   ExampleMode Mode = (ExampleMode)AskMakeChoice(MIL_TEXT("Please choose the example mode"), ExampleModeChoices);
   bool AskForFinalAcquisition = Mode == ExampleMode::AlignModel || Mode == ExampleMode::AlignOverlap;

   // If a model is required.
   MIL_UNIQUE_BUF_ID MilAlignModel;
   MIL_UNIQUE_3DDISP_ID MilModel3dDisp;
   if (Mode == ExampleMode::AlignModel)
      {
      // Allocate the Model 3d display.
      MilModel3dDisp = Allocate3dDisplay(MilSystem);
      if(!MilModel3dDisp)
         return Terminate(MIL_TEXT("The current system does not support the 3D display."));

      if(DataSource == Camera3dDataSource::Example)
         {
         MilAlignModel = MbufRestore(ALIGN_MODEL_FILE, MilSystem, M_UNIQUE_ID);
         }
      else
         {
         auto UserDataPath = CreateUser3dDataFolder();
         auto UserModelFilePath = UserDataPath + ALIGN_MODEL_NAME;
         MIL_INT FileFound;
         MappFileOperation(M_DEFAULT, UserModelFilePath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileFound);

         if(FileFound)
            {
            MosPrintf(MIL_TEXT("Alignment model container file found under \n   %s\n\n"), UserDataPath.c_str());
            MosPrintf(MIL_TEXT("To use a new file, remove the current file from the directory\n"));
            MosPrintf(MIL_TEXT("and restart the application.\n"));

            MilAlignModel = MbufRestore(UserModelFilePath, MilSystem, M_UNIQUE_ID);
            ConvertPointCloud(MilAlignModel, MilAlignModel);
            }
         else
            {
            // Try to load a model.
            MosPrintf(MIL_TEXT("Please select the container file that contains the alignment model.\n"));
            MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
            MilAlignModel = MbufRestore(M_INTERACTIVE, MilSystem, M_UNIQUE_ID);
            MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

            // Terminate if the model could not be loaded.
            if(MilAlignModel)
               {
               ConvertPointCloud(MilAlignModel, MilAlignModel);
               MbufSave(UserModelFilePath, MilAlignModel);
               }
            else
               return Terminate(MIL_TEXT("No reference model loaded.\nGenerate an alignment model and restart the application."));
            }
         }
      }

   MosPrintf(MIL_TEXT("========================================\n\n"));

   // Reset the coordinate systems of the cameras when computing a new alignment.
   if ((Mode == ExampleMode::AlignOverlap || Mode == ExampleMode::AlignModel) && DataSource == Camera3dDataSource::Cameras)
      {
      Reset3dCamerasCoordinateSystems(MilDigitizers);
      MosPrintf(MIL_TEXT("\nAlignment required.\n"));
      MosPrintf(MIL_TEXT("The 3D cameras' coordinate systems have been reset to the anchor position.\n\n"));
      }

   if (Mode == ExampleMode::AlignOverlap || Mode == ExampleMode::AlignModel)
      {
      // Clear the previous alignment matrices.
      AlignmentMatrices.clear();
      AlignmentMatrices.resize(Cameras3d.size());

      // Acquire the whole alignment object.
      MosPrintf(MIL_TEXT("Position the alignment object: it must be visible to all 3D cameras.\n"));
      auto ToAlignPointClouds = GrabPointClouds(MilDigitizers, MIN_NORMAL_ANGLE);
      auto MilToAlignPointClouds = std::vector<MIL_ID>(ToAlignPointClouds.begin(), ToAlignPointClouds.end());

      // Crop the point clouds.
      if(DataSource != Camera3dDataSource::Example)
         InteractivePointCloudsCropping(MilToAlignPointClouds);

      // Register the point clouds.
      auto MilRegResult = RegisterPointClouds(MilAlignModel, MilToAlignPointClouds, DataSource);

      if(!MilRegResult)
         return Terminate(MIL_TEXT("The registration of all 3D data was not successfully completed."));

      //  Choose the merge location.
      MIL_INT MergeLocation = M_REGISTRATION_GLOBAL;

      MIL_STRING_STREAM DefaultChoice;
      DefaultChoice << MIL_TEXT("Registration Global (");
      if(Mode == ExampleMode::AlignOverlap)
         DefaultChoice << Cameras3d[0];
      else
         DefaultChoice << MIL_TEXT("Model");
      DefaultChoice << MIL_TEXT(")");

      if(DataSource != Camera3dDataSource::Example)
         {
         MIL_INT Choice = AskMakeChoice(MIL_TEXT("Please choose the merge location"), Cameras3d, DefaultChoice.str().c_str());
         if(Choice >= 0)
            MergeLocation = Choice;
         M3dregControl(MilRegResult, M_GENERAL, M_MERGE_LOCATION, MergeLocation);
         }
      else
         MosPrintf(MIL_TEXT("The merge location is %s.\n"), DefaultChoice.str().c_str());

      // Display the transformed grabbed point clouds.
      auto MilAligned3dDisp = Allocate3dDisplay(MilSystem);

      // Get the transformation matrix for all 3d cameras.
      for (MIL_INT c = 0; c < (MIL_INT)Cameras3d.size(); c++)
         {
         AlignmentMatrices[c] = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3dregCopyResult(MilRegResult, c, MergeLocation, AlignmentMatrices[c], M_REGISTRATION_MATRIX, M_DEFAULT);
         }
      std::vector<MIL_ID> MilAlignmentMatrices(AlignmentMatrices.begin(), AlignmentMatrices.end());

      // Merge all the point clouds.
      if (MilAlignModel)
         MilToAlignPointClouds.push_back(MilAlignModel);
      auto MilMergedPointClouds = MergePointClouds(MilRegResult, MilToAlignPointClouds);

      // Display the registration result.
      M3ddispSelect(MilAligned3dDisp, MilMergedPointClouds, M_SELECT, M_DEFAULT);
      MosPrintf(MIL_TEXT("The alignment of the 3D data is displayed.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      if (DataSource == Camera3dDataSource::Cameras)
         {
         // Ask if we want to apply the alignment.
         if(AskYesNo(MIL_TEXT("Do you want to apply the alignment to the 3d cameras?")))
            {
            // Setup the 3d cameras so they are aligned.
            if(Setup3dCamerasCoordinateSystems(MilDigitizers, MilAlignmentMatrices))
               {
               MosPrintf(MIL_TEXT("\nThe 3d cameras reference coordinate systems have been updated.\n"));
               MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
               MosGetch();
               }
            else
               return Terminate(MIL_TEXT("\nUnable to correctly apply the alignment to the 3d cameras."));

            Mode = ExampleMode::AlignedSource;
            }
         else
            {
            Mode = ExampleMode::AlignedUsingMatrix;
            }
         }

      // Save the alignment matrices in the User Data folder.
      SaveAlignmentMatrices(MilAlignmentMatrices, MilDigitizers, DataSource);
      }

   // If synchronization and alignment of the 3d cameras is achieved, ask whether to verify the
   // programming of the SFNC coefficients or the application of the alignment matrix.
   if(DataSource == Camera3dDataSource::Cameras)
      {
      if(!AskForFinalAcquisition || AskYesNo(MIL_TEXT("Do you want to acquire a new scan to verify the alignment")))
         {
         if(SyncConfig == SyncConfiguration::Synch || AskYesNo(MIL_TEXT("No acquisition synchronisation detected.\nDo you want to acquire a new scan anyways?")))
            {
            // Acquire an object to show that the alignment worked.
            MosPrintf(MIL_TEXT("Verification of the alignment.\n\n"));
            MosPrintf(MIL_TEXT("Place an object that will be visible to all 3d cameras.\n"));
            auto AlignedPointClouds = GrabPointClouds(MilDigitizers, MIN_NORMAL_ANGLE);
            auto MilAlignedPointClouds = std::vector<MIL_ID>(AlignedPointClouds.begin(), AlignedPointClouds.end());

            // Align using the matrices if the alignment is not to be done at the 3d camera source.
            if(Mode == ExampleMode::AlignedUsingMatrix)
               {
               for(MIL_INT c = 0; c < (MIL_INT)Cameras3d.size(); c++)
                  M3dimMatrixTransform(AlignedPointClouds[c], AlignedPointClouds[c], AlignmentMatrices[c], M_DEFAULT);
               MosPrintf(MIL_TEXT("The alignment matrices were used to align the point clouds.\n"));
               }

            // Show the alignment point clouds.
            auto MilMergedPointClouds = MergePointClouds(M_NULL, MilAlignedPointClouds);
            M3ddispSelect(MilComplete3dDisp, MilMergedPointClouds, M_SELECT, M_DEFAULT);
            MosPrintf(MIL_TEXT("The aligned point clouds are displayed.\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
            MosGetch();
            }
         }
      }
   }


//****************************************************************************
// Resets the camera coordinate systems.
//****************************************************************************
bool Reset3dCamerasCoordinateSystems(const std::vector<MIL_ID>& MilDigitizers)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(auto& d : MilDigitizers)
      DigSetString(d, MIL_TEXT("Scan3dCoordinateSystemReference"), MIL_TEXT("Anchor"));
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
   if(MappGetError(M_DEFAULT, M_GLOBAL + M_SYNCHRONOUS, 0) != M_NULL_ERROR)
      MosPrintf(MIL_TEXT("\nUnable to correctly reset the 3d cameras coordinate system!\n"));
   return true;
   }

//****************************************************************************
// Sets the coordinate systems.
//****************************************************************************
bool Setup3dCamerasCoordinateSystems(const std::vector<MIL_ID>& MilDigitizers, const std::vector<MIL_ID>& MilAlignmentMatrices)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for(MIL_INT d = 0; d < (MIL_INT)MilDigitizers.size(); d++)
      {
      if (MilAlignmentMatrices[d])
         {
         // Extract rotation and translation in SFNC format.
         MIL_DOUBLE Rx, Ry, Rz, Tx, Ty, Tz;
         M3dgeoMatrixGetTransform(MilAlignmentMatrices[d], M_ROTATION_ZYX, &Rz, &Ry, &Rx, nullptr, M_DEFAULT);
         M3dgeoMatrixGetTransform(MilAlignmentMatrices[d], M_TRANSLATION, &Tx, &Ty, &Tz, nullptr, M_DEFAULT);

         // Set the Transformed parameters on the camera.
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("RotationX"), Rx);
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("RotationY"), Ry);
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("RotationZ"), Rz);
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("TranslationX"), Tx);
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("TranslationY"), Ty);
         DigSetTransformParam(MilDigitizers[d], MIL_TEXT("TranslationZ"), Tz);

         // Activate the Transformed coordinate system.
         DigSetString(MilDigitizers[d], MIL_TEXT("Scan3dOutputMode"), MIL_TEXT("CalibratedABC_Grid"));
         DigSetString(MilDigitizers[d], MIL_TEXT("Scan3dCoordinateSystemReference"), MIL_TEXT("Transformed"));
         }
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   return MappGetError(M_DEFAULT, M_GLOBAL + M_SYNCHRONOUS, 0) == M_NULL_ERROR;
   }

//****************************************************************************
// Terminates the application printing an exit message.
//****************************************************************************
int Terminate(MIL_CONST_TEXT_PTR Message)
   {
   MosPrintf(MIL_TEXT("%s\nPress <Enter> to end.\n"), Message);
   MosGetch();
   return 0;
   }


