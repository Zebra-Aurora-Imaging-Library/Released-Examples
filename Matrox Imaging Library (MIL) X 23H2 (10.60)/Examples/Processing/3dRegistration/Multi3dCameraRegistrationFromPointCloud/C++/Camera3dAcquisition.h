//***************************************************************************************
//
// File name: Camera3dAcquisition.h
//
// Synopsis:  Utility header that handles the data acquisition of the example. It contains
//            functions to allocate the digitizer from real cameras or disk. In the case of
//            disk source, it can either use a predefined example path or some user selected
//            data.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************

#pragma once

enum SyncConfiguration
   {
   NoSync = 0,
   Synch
   };

enum class FrameTriggerMode
   {
   None,
   Software,
   Trigger,
   Unknown
   };

static const std::vector<MIL_CONST_TEXT_PTR> FrameTriggerChoices =
   {
   MIL_TEXT("None"),
   MIL_TEXT("Trigger"),
   };

enum class LineTriggerMode
   {
   None,
   Continuous,
   Trigger,
   Encoder,
   Unknown
   };

static const std::vector<MIL_CONST_TEXT_PTR> LineTriggerChoices =
   {
   MIL_TEXT("None"),
   MIL_TEXT("Continous"),
   MIL_TEXT("Trigger"),
   MIL_TEXT("Encoder")
   };

//****************************************************************************
// Functions declaration.
//****************************************************************************
bool VerifyDiskPointClouds(MIL_ID MilSystem, const std::vector<MIL_STRING>& UserContainerNames);


//****************************************************************************
// Digitizer configuration functions.
//****************************************************************************
MIL_STRING DigGetString(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_BOOL FeaturePresent;
   MdigInquireFeature(Dig, M_FEATURE_PRESENT, FeatureName, M_TYPE_BOOLEAN, &FeaturePresent);

   if(FeaturePresent)
      {
      MIL_STRING FeatureValue;
      MdigInquireFeature(Dig, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, FeatureValue);
      return FeatureValue;
      }

   return MIL_TEXT("FeatureNotPresent");
   }

MIL_DOUBLE DigGetDouble(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName)
   {
   MIL_BOOL FeaturePresent;
   MdigInquireFeature(Dig, M_FEATURE_PRESENT, FeatureName, M_TYPE_BOOLEAN, &FeaturePresent);

   if(FeaturePresent)
      {
      MIL_DOUBLE FeatureValue;
      MdigInquireFeature(Dig, M_FEATURE_VALUE, FeatureName, M_TYPE_DOUBLE, &FeatureValue);
      return FeatureValue;
      }
   return MIL_DOUBLE_MAX;
   }

void DigSetString(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName, MIL_CONST_TEXT_PTR FeatureValue)
   {
   MosPrintf(MIL_TEXT("Digitizer: %s set to %s\n"), FeatureName, FeatureValue);
   MdigControlFeature(Dig, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, FeatureValue);
   }

void DigSetDouble(MIL_ID Dig, MIL_CONST_TEXT_PTR FeatureName, MIL_DOUBLE FeatureValue)
   {
   MosPrintf(MIL_TEXT("Digitizer: %s set to %f\n"), FeatureName, FeatureValue);
   MdigControlFeature(Dig, M_FEATURE_VALUE, FeatureName, M_TYPE_DOUBLE, &FeatureValue);
   }

void DigSetTransformParam(MIL_ID Dig, MIL_CONST_TEXT_PTR ParamName, MIL_DOUBLE ParamValue)
   {
   DigSetString(Dig, MIL_TEXT("Scan3dCoordinateTransformSelector"), ParamName);
   DigSetDouble(Dig, MIL_TEXT("Scan3dTransformValue"), ParamValue);
   }

LineTriggerMode DigGetLineTriggerMode(MIL_ID Dig)
   {
   MIL_STRING TriggerModeName = DigGetString(Dig, MIL_TEXT("Scan3dTriggerSourceMode"));

   if(TriggerModeName == MIL_TEXT("FeatureNotPresent"))
      {
      MosPrintf(MIL_TEXT("Unable to certify camera 3d %s Line trigger mode.\n"));
      MIL_INT Choice = AskMakeChoice(MIL_TEXT("Please select your 3d camera line trigger mode"), LineTriggerChoices);
      TriggerModeName = LineTriggerChoices[Choice];
      }

   if(TriggerModeName == MIL_TEXT("Continuous"))
      return LineTriggerMode::Continuous;
   if(TriggerModeName == MIL_TEXT("Trigger"))
      return LineTriggerMode::Trigger;
   if(TriggerModeName == MIL_TEXT("Encoder"))
      return LineTriggerMode::Encoder;
   if(TriggerModeName == MIL_TEXT("None"))
      return LineTriggerMode::None;

   return LineTriggerMode::Unknown;
   }

FrameTriggerMode DigGetFrameTriggerMode(MIL_ID Dig)
   {
   // Set the trigger selector to frame start.
   DigSetString(Dig, MIL_TEXT("TriggerSelector"), MIL_TEXT("FrameStart"));

   // Check if the trigger is active.
   if(DigGetString(Dig, MIL_TEXT("TriggerMode")) == MIL_TEXT("Off"))
      return FrameTriggerMode::None;

   // Get the trigger source.
   auto FrameTriggerSource = DigGetString(Dig, MIL_TEXT("TriggerSource"));
   if(FrameTriggerSource == MIL_TEXT("Software"))
      return FrameTriggerMode::Software;
   else
      return FrameTriggerMode::Trigger;

   return FrameTriggerMode::Unknown;
   }

//****************************************************************************
// Digitizer allocation.
//****************************************************************************
enum Camera3dDataSource
   {
   Example = 0,
   User,
   Cameras
   };

// Choices of data sources.
static const std::vector<MIL_CONST_TEXT_PTR> DataSourceChoices =
   {
   MIL_TEXT("Default sample data restored from disk"),
   MIL_TEXT("User data restored from disk"),
   MIL_TEXT("Data acquired from connected 3D cameras")
   };


// DeviceScanTypes handled by the example.
static const std::vector<MIL_CONST_TEXT_PTR> PossibleDeviceScanTypes =
   {
   MIL_TEXT("Linescan3D"),
   MIL_TEXT("Areascan3D")
   };

//****************************************************************************
// Gets the camera data source.
//****************************************************************************
Camera3dDataSource GetCameraDataSource()
   {
   // Ask the user how to allocate the camera.
   Camera3dDataSource DataSource = (Camera3dDataSource)AskMakeChoice(MIL_TEXT("Please select the 3d data source"), DataSourceChoices);
   return DataSource;
   }

//****************************************************************************
// Allocates the simulated digitizers.
//****************************************************************************
std::vector<MIL_UNIQUE_DIG_ID> AllocateDisk3dCameras(MIL_ID MilSystem, const std::vector<MIL_STRING>& SimDigsPath)
   {
   std::vector<MIL_UNIQUE_DIG_ID> All3dCameras;

   // Allocate the digitizers if the files are correct.
   if(VerifyDiskPointClouds(MilSystem, SimDigsPath))
      {
      All3dCameras.resize(SimDigsPath.size());
      for(MIL_INT d = 0; d < (MIL_INT)SimDigsPath.size(); d++)
         All3dCameras[d] = MdigAlloc(MilSystem, M_DEFAULT, SimDigsPath[d], M_DEFAULT, M_UNIQUE_ID);
      }

   return All3dCameras;
   }

//****************************************************************************
// Allocates the simulated digitizers. One for each image or one for each folder.
//****************************************************************************
std::vector<MIL_UNIQUE_DIG_ID> AllocateExample3dCameras(MIL_ID MilSystem, bool SingleSimulatedDig)
   {
   if(SingleSimulatedDig)
      return AllocateDisk3dCameras(MilSystem, {EXAMPLE_IMAGE_PATH});
   else
      return AllocateDisk3dCameras(MilSystem, EXAMPLE_IMAGES);
   }

//****************************************************************************
// Gets the path to the user 3d data folder.
//****************************************************************************
MIL_STRING GetUser3dDataPath()
   {
   MIL_STRING User3dDataPath;
   MappInquire(M_DEFAULT, M_MIL_DIRECTORY_EXAMPLES, User3dDataPath);
   User3dDataPath += USER_DATA_PATH;
   return User3dDataPath;
   }

//****************************************************************************
// Creates the user data folder if necessary.
//****************************************************************************
MIL_STRING CreateUser3dDataFolder()
   {
   auto UserDataPath = GetUser3dDataPath();

   MIL_INT FileFound = M_NO;
   MappFileOperation(M_DEFAULT, UserDataPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileFound);
   if(FileFound != M_YES)
      MappFileOperation(M_DEFAULT, UserDataPath, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);

   return UserDataPath;
   }

//****************************************************************************
// Checks the DeviceUserIds of the cameras. They must all be set and unique.
//****************************************************************************
bool CheckCamerasDeviceUserIds(const std::vector<MIL_ID>& MilDigitizers)
   {
   std::vector<MIL_STRING> AllDeviceUserId;
   for(MIL_INT c = 0; c < (MIL_INT)MilDigitizers.size(); c++)
      {
      MIL_STRING DeviceUserID;
      MdigInquireFeature(MilDigitizers[c], M_FEATURE_VALUE, MIL_TEXT("DeviceUserID"), M_TYPE_STRING, DeviceUserID);

      // All devices must be named.
      if(DeviceUserID == MIL_TEXT(""))
         return false;

      // There must not be any duplicated names.
      if(std::find(AllDeviceUserId.begin(), AllDeviceUserId.end(), DeviceUserID) != AllDeviceUserId.end())
         return false;
      AllDeviceUserId.push_back(DeviceUserID);
      }

   return true;
   }

//****************************************************************************
// Gets the path of the matrix file.
//****************************************************************************
MIL_STRING GetDigitizerMatrixName(const MIL_STRING& UserDataPath,
                                  MIL_INT DigIndex,
                                  const std::vector<MIL_ID>& MilDigitizers,
                                  Camera3dDataSource DataSource)
   {
   MIL_STRING_STREAM MatrixNameStream;
   if(DataSource == Camera3dDataSource::Cameras)
      {
      MIL_STRING DeviceUserID;
      MdigInquireFeature(MilDigitizers[DigIndex], M_FEATURE_VALUE, MIL_TEXT("DeviceUserID"), M_TYPE_STRING, DeviceUserID);
      MatrixNameStream << UserDataPath << DeviceUserID << MIL_TEXT(".m3dgeo");
      }
   else
      MatrixNameStream << UserDataPath << DigIndex << MIL_TEXT(".m3dgeo");
   return MatrixNameStream.str();
   }

//****************************************************************************
// Restores the previous alignment matrices from the user data folder if available.
//****************************************************************************
std::vector<MIL_UNIQUE_3DGEO_ID> RestorePreviousAlignmentMatrices(MIL_ID MilSystem,
                                                                  const std::vector<MIL_ID>& MilDigitizers,
                                                                  Camera3dDataSource DataSource)
   {
   // Create the folder if it doesn't exist.
   auto UserDataPath = CreateUser3dDataFolder();

   // Get the names of the previous matrices already in the folder.
   std::vector<MIL_UNIQUE_3DGEO_ID> PreviousAlignmentMatrices(MilDigitizers.size());
   MIL_INT FileFound = M_NO;
   for(MIL_INT c = 0; c < (MIL_INT)MilDigitizers.size(); c++)
      {
      auto MatrixName = GetDigitizerMatrixName(UserDataPath, c, MilDigitizers, DataSource);
      MappFileOperation(M_DEFAULT, MatrixName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileFound);
      if(FileFound == M_YES)
         PreviousAlignmentMatrices[c] = M3dgeoRestore(MatrixName, MilSystem, M_DEFAULT, M_UNIQUE_ID);
      else
         return std::vector<MIL_UNIQUE_3DGEO_ID>();
      }

   return PreviousAlignmentMatrices;
   }
//****************************************************************************
// Saves the alignment matrices in the user data folder.
//****************************************************************************
void SaveAlignmentMatrices(std::vector<MIL_ID> MilAlignmentMatrices,
                           const std::vector<MIL_ID>& MilDigitizers,
                           Camera3dDataSource DataSource)
   {
   // Create the folder if it doesn't exist.
   auto UserDataPath = CreateUser3dDataFolder();

   // Save all the matrices.
   for(MIL_INT c = 0; c < (MIL_INT)MilAlignmentMatrices.size(); c++)
      {
      auto MatrixName = GetDigitizerMatrixName(UserDataPath, c, MilDigitizers, DataSource);
      M3dgeoSave(MatrixName, MilAlignmentMatrices[c], M_DEFAULT);
      }

   // Print a message.
   MosPrintf(MIL_TEXT("The alignment matrices were saved under \n   %s\n\n"), UserDataPath.c_str());
   }

//****************************************************************************
// Verifies if the point cloud is valid.
//****************************************************************************
bool VerifyPointCloud(MIL_ID MilPointCloud, MIL_INT& DistanceUnits, bool UpdateDistanceUnits)
   {
   // Verify if there is a range.
   if(!MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_COMPONENT_ID, M_NULL))
      {
      MosPrintf(MIL_TEXT("\nThe container doesn't have a range component.\n")
                MIL_TEXT("This data cannot be used by the example!\n\n"));
      return false;
      }

   // Verify the distance units.
   if(UpdateDistanceUnits)
      MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_3D_DISTANCE_UNIT, &DistanceUnits);

   else if(DistanceUnits != MbufInquireContainer(MilPointCloud, M_COMPONENT_RANGE, M_3D_DISTANCE_UNIT, M_NULL))
      {
      MosPrintf(MIL_TEXT("\nThe point clouds have different distance units.\n")
                MIL_TEXT("This data cannot be used by the example!\n\n"));
      return false;
      }

   return true;
   }

//****************************************************************************
// Verifies if all the point clouds on the disk are valid.
//****************************************************************************
bool VerifyDiskPointClouds(MIL_ID MilSystem, const std::vector<MIL_STRING>& UserContainerNames)
   { 
   // Check if all the point clouds range component have the same units.
   MIL_INT DistanceUnits;
   for(MIL_INT p = 0; p < (MIL_INT)UserContainerNames.size(); p++)
      {
      // Restore the container.
      auto MilPointCloud = MbufRestore(UserContainerNames[p], MilSystem, M_UNIQUE_ID);

      // Verify the point cloud.
      if(!VerifyPointCloud(MilPointCloud, DistanceUnits, p == 0))
         return false;
      }
   return true;
   }

//****************************************************************************
// Allocates the simulated digitizer based on some user selected images.
//****************************************************************************
std::vector<MIL_UNIQUE_DIG_ID> AllocateUser3dCameras(MIL_ID MilSystem, bool SingleSimulatedDig)
   {
   // Create the folder if it doesn't exist.
   auto UserDataPath = CreateUser3dDataFolder();

   // Get the names of the user images already in the folder.
   std::vector<MIL_STRING> UserContainerNames;
   MIL_UNIQUE_BUF_ID MilUserContainer;
   MIL_INT NbUserDig = 0;
   MIL_INT FileFound = M_NO;
   do
      {
      MIL_STRING_STREAM UserContainerNameStream;
      UserContainerNameStream << UserDataPath << NbUserDig << MIL_TEXT(".mbufc");
      MappFileOperation(M_DEFAULT, UserContainerNameStream.str(), M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FileFound);
      if(FileFound == M_YES)
         {
         NbUserDig++;
         UserContainerNames.push_back(UserContainerNameStream.str());
         }
      } while(FileFound);

   // Transfer new user images if the folder is empty.
   if(NbUserDig)
      {
      MosPrintf(MIL_TEXT("%i container files found under \n   %s\n\n"), NbUserDig, UserDataPath.c_str());
      MosPrintf(MIL_TEXT("To use new files, remove the current files from the directory and\n"));
      MosPrintf(MIL_TEXT("restart the application.\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("For each 3D camera, select the container file (.mbufc) that holds its data.\n"));
      MosPrintf(MIL_TEXT("The files will be copied under\n   %s\n"), UserDataPath.c_str());
      MosPrintf(MIL_TEXT("Press <Cancel> to stop adding scans.\n"));

      MIL_INT DistanceUnits;
      bool UpdateDistanceUnits = true;
      do
         {
         MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
         MilUserContainer = MbufRestore(M_INTERACTIVE, MilSystem, M_UNIQUE_ID);
         MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
         if(MilUserContainer)
            {
            if(VerifyPointCloud(MilUserContainer, DistanceUnits, UpdateDistanceUnits))
               {
               MIL_STRING_STREAM UserContainerNameStream;
               UserContainerNameStream << UserDataPath << NbUserDig << MIL_TEXT(".mbufc");
               MbufSave(UserContainerNameStream.str(), MilUserContainer);
               if(!SingleSimulatedDig)
                  UserContainerNames.push_back(UserContainerNameStream.str());
               NbUserDig++;
               UpdateDistanceUnits = false;
               }
            }
         } while(MilUserContainer);
      }

   if(SingleSimulatedDig)
      UserContainerNames.push_back(UserDataPath);

   // Allocate the digitizers.
   return AllocateDisk3dCameras(MilSystem, UserContainerNames);
   }

//****************************************************************************
// Allocates the real cameras' digitizers.
//****************************************************************************
std::vector<MIL_UNIQUE_DIG_ID> AllocateReal3dCamera(MIL_ID MilSystem)
   {
   MIL_INT NbCameras = MsysInquire(MilSystem, M_NUM_CAMERA_PRESENT, M_NULL);

   MIL_INT NbLineScan3d = 0;
   MIL_INT NbAreaScan3d = 0;
   std::vector<MIL_UNIQUE_DIG_ID> All3dCameras;
   for(MIL_INT p = 0; p < NbCameras; p++)
      {
      // Allocate the digitizer.
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
      auto MilDigitizer = MdigAlloc(MilSystem, M_DEV + p, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

      if(MilDigitizer)
         {
         // Verify if the digitizer is of the right type and add it.
         MIL_STRING DeviceScanType = DigGetString(MilDigitizer, MIL_TEXT("DeviceScanType"));

         if(DeviceScanType == MIL_TEXT("Linescan3D") || DeviceScanType == MIL_TEXT("Areascan3D"))
            {
            All3dCameras.push_back(std::move(MilDigitizer));
            if(DeviceScanType == MIL_TEXT("Linescan3D"))
               NbLineScan3d++;
            else
               NbAreaScan3d++;
            }
         }
      }

   // Cannot combine the type of cameras. Ask to choose one type.
   if(NbLineScan3d > 0 && NbAreaScan3d > 0)
      {
      MosPrintf(MIL_TEXT("%i Linescan3d and %i Areascan3d 3d cameras detected\n\n"), NbLineScan3d, NbAreaScan3d);
      MIL_INT Choice = AskMakeChoice(MIL_TEXT("Please select the type of 3d cameras used"), PossibleDeviceScanTypes);
      std::vector<MIL_UNIQUE_DIG_ID> Kept3dCameras;
      for(auto& c : All3dCameras)
         {
         if(DigGetString(c, MIL_TEXT("DeviceScanType")) == PossibleDeviceScanTypes[Choice])
            Kept3dCameras.push_back(std::move(All3dCameras[c]));
         }
      std::swap(All3dCameras, Kept3dCameras);
      }

   // Print the result of the allocation.
   MosPrintf(MIL_TEXT("%ix 3D cameras detected!\n\n"), All3dCameras.size());

   return All3dCameras;
   }

//****************************************************************************
// Allocates the digitizers depending on the 3d camera data source type.
//****************************************************************************
std::vector<MIL_UNIQUE_DIG_ID> Allocate3dCameras(MIL_ID MilSystem, Camera3dDataSource DataSource, bool SingleSimulatedDig = false)
   {
   switch(DataSource)
      {
      case Cameras:
         return AllocateReal3dCamera(MilSystem);
      case User:
         return AllocateUser3dCameras(MilSystem, SingleSimulatedDig);
      case Example:
      default:
         return AllocateExample3dCameras(MilSystem, SingleSimulatedDig);
      }
   }

//****************************************************************************
// Evaluates the synchronization configuration.
//****************************************************************************
SyncConfiguration EvaluateCurrentConfig(const std::vector<MIL_ID>& MilDigitizers)
   {
   if(IsRealDig(MilDigitizers[0]))
      {
      if(DigGetString(MilDigitizers[0], MIL_TEXT("DeviceScanType")) == MIL_TEXT("Linescan3D"))
         {
         for(auto& d : MilDigitizers)
            {
            // If one of the linescan cameras is not triggered, synchronization is not possible.
            auto TriggerMode = DigGetLineTriggerMode(d);
            if(TriggerMode == LineTriggerMode::Continuous || TriggerMode == LineTriggerMode::Unknown)
               return SyncConfiguration::NoSync;
            }
         }
      }
   return SyncConfiguration::Synch;
   }

//****************************************************************************
// Gets the synchronization configuration of the system.
// If not synchronized, give the user the option to modify settings.
//****************************************************************************
SyncConfiguration GetSyncConfig(const std::vector<MIL_ID>& MilDigitizers)
   {
   // Evaluate the current configuration.
   auto CurSyncConfig = EvaluateCurrentConfig(MilDigitizers);

   while(CurSyncConfig != SyncConfiguration::Synch)
      {
      MosPrintf(MIL_TEXT("Your 3d cameras may not be synchronized.\n"));
      if(AskYesNo(MIL_TEXT("Do you want to modify the triggering parameters of your 3d cameras?")))
         {
         for(auto& d : MilDigitizers)
            MdigControl(d, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
         MosPrintf(MIL_TEXT("Press <Enter> when you have finished your modifications.\n\n"));
         MosGetch();
         for(auto& d : MilDigitizers)
            MdigControl(d, M_GC_FEATURE_BROWSER, M_CLOSE);
         }
      else
         return CurSyncConfig;

      // Evaluate the current configuration.
      CurSyncConfig = EvaluateCurrentConfig(MilDigitizers);
      };

   return CurSyncConfig;
   }

//****************************************************************************
// Acquires the point clouds.
//****************************************************************************
MIL_INT MFTYPE GrabHook(MIL_INT, MIL_ID, void *) {return 0;};
std::vector<MIL_UNIQUE_BUF_ID> GrabPointClouds(const std::vector<MIL_ID>& MilDigitizers, MIL_DOUBLE MinNormalAngle)
   {
   std::vector<MIL_UNIQUE_BUF_ID> MilPointClouds(MilDigitizers.size());

   // Allocate the grab containers.
   for (MIL_INT d = 0; d < (MIL_INT)MilDigitizers.size(); d++)
      {
      auto MilSystem = MdigInquire(MilDigitizers[d], M_OWNER_SYSTEM, M_NULL);
      MilPointClouds[d] = MbufAllocContainer(MilSystem, M_GRAB + M_DISP + M_PROC, M_DEFAULT, M_UNIQUE_ID);
      }

   MosPrintf(MIL_TEXT("Prepare the system to start a new scan.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue and start the motion if necessary.\n\n"));
   MosGetch();

   // Start all the acquisition.
   for (MIL_INT p = 0; p < (MIL_INT)MilDigitizers.size(); p++)
      MdigProcess(MilDigitizers[p], &(MilPointClouds[p]), 1, M_SEQUENCE + M_COUNT(1), M_ASYNCHRONOUS, GrabHook, M_NULL);

   MosPrintf(MIL_TEXT("Acquisition in progress...\n"));

   // Wait for all acquisitions to end.
   for (MIL_INT p = 0; p < (MIL_INT)MilDigitizers.size(); p++)
      MdigProcess(MilDigitizers[p], &(MilPointClouds[p]), 1, M_STOP + M_WAIT, M_DEFAULT, GrabHook, M_NULL);

   // Process the point clouds.
   auto Colors = GetDistinctColors(MilDigitizers.size());
   for (MIL_INT p = 0; p < (MIL_INT)MilDigitizers.size(); p++)
      {
      // Convert the point cloud to a processable format.
      ConvertPointCloud(MilPointClouds[p], MilPointClouds[p]);
      
      // Color the clouds.
      ColorCloud(MilPointClouds[p], M_RGB888(Colors[p].R, Colors[p].G, Colors[p].B));

      // Get the normals and remove points whose normal is close to the horizontal.
      // Given the nature of a 3d camera, it is likely that those points are only visible by one.
      if (MinNormalAngle != 0.0)
         {
         M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilPointClouds[p], MilPointClouds[p], M_DEFAULT);
         auto MilNormals = MbufInquireContainer(MilPointClouds[p], M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);
         auto MilConfidence = MbufInquireContainer(MilPointClouds[p], M_COMPONENT_CONFIDENCE, M_COMPONENT_ID, M_NULL);
         auto MilNormalZ = MbufChildColor(MilNormals, 2, M_UNIQUE_ID);
         MIL_DOUBLE MinNz = asin(MinNormalAngle * DIV_PI_180);
         MIL_INT SizeX = MbufInquire(MilNormalZ, M_SIZE_X, M_NULL);
         MIL_INT SizeY = MbufInquire(MilNormalZ, M_SIZE_Y, M_NULL);
         auto MilValidNormal = MbufAlloc2d(M_DEFAULT_HOST, SizeX, SizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_UNIQUE_ID);
         MimBinarize(MilNormalZ, MilValidNormal, M_OUT_RANGE, -MinNz, MinNz);
         MimArith(MilValidNormal, MilConfidence, MilConfidence, M_AND);

         MosPrintf(MIL_TEXT("Points whose normal angle with regards to the XY plane is\n"));
         MosPrintf(MIL_TEXT("less than %.2f degrees have been removed.\n"), MinNormalAngle);
         }
      }

   MosPrintf(MIL_TEXT("Acquisition done.\n\n"));
   return MilPointClouds;
   }
