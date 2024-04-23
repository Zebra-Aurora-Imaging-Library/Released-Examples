//*************************************************************************************
//
// File name: ClassCNNCompleteTrain.cpp
//
// Synopsis:  This program uses the classification module to train
//            a context able to classify 3 different types of fabrics.
//
// Note:      GPU training can be enabled via a MIL update for 64-bit.
//            This can dramatically increase the training speed.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <windows.h>
#include <mil.h>
#include <string>
#include <algorithm>
#include <random>
#include <numeric>
#include <map>
#include <math.h>

static const MIL_INT NUMBER_OF_CLASSES         = 3;
static const MIL_INT NB_AUGMENTATION_PER_IMAGE = 2;

// ===========================================================================
// Example description.
// ===========================================================================
void PrintHeader()
   {
   MosPrintf(
      MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("ClassCNNCompleteTrain\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This example trains a CNN model to classify the %d fabrics shown.\n")
      MIL_TEXT("The first step prepares the single dataset needed for the training.\n")
      MIL_TEXT("The second step trains a context and displays the train evolution.\n")
      MIL_TEXT("The final step performs predictions on test data using the trained\n")
      MIL_TEXT("CNN model as a final check of the expected model performance.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("graphic, classification.\n\n"), NUMBER_OF_CLASSES);
   }

// Path definitions.
#define EXAMPLE_IMAGE_ROOT_PATH           M_IMAGE_PATH MIL_TEXT("Classification/Fabrics/")
#define EXAMPLE_ORIGINAL_DATA_PATH        M_IMAGE_PATH MIL_TEXT("Classification/Fabrics/OriginalData/")
#define EXAMPLE_DATA_PATH                 MIL_TEXT("Dataset")
#define EXAMPLE_PREPARED_DATA_PATH        MIL_TEXT("PreparedData/")
#define EXAMPLE_PREPARED_DATA_PATH_TEST   MIL_TEXT("PreparedData/TestSet")

MIL_STRING FABRICS_CLASS_NAME[NUMBER_OF_CLASSES] = {MIL_TEXT("Fabric1"),
                                                    MIL_TEXT("Fabric2"),
                                                    MIL_TEXT("Fabric3")};

// Nb images per classes.
MIL_INT FABRICS_CLASS_NB_IMAGES[NUMBER_OF_CLASSES] = {200, 200, 200};

// Icon image for each class.
MIL_STRING FABRICS_CLASS_ICON[NUMBER_OF_CLASSES] = {EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Fabric1_Icon.mim"),
                                                    EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Fabric2_Icon.mim"),
                                                    EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Fabric3_Icon.mim")};

MIL_INT IsTrainingSupportedOnPlatform(MIL_ID MilSystem);
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem);

MIL_UNIQUE_BUF_ID CreateImageOfAllClasses(
   MIL_ID            MilSystem,
   const MIL_STRING* FabricClassIcon,
   MIL_INT           NumberOfClasses);

MIL_STRING GetExampleCurrentDirectory();

void SetupTheDatasets(
   MIL_ID            MilSystem,
   const MIL_STRING* FabricsClassName,
   const MIL_STRING* FabricsClassIcon,
   MIL_INT           NumberOfClasses,
   const MIL_STRING& OriginalDataPath,
   const MIL_STRING& ExampleDataPath,
   MIL_ID            WorkingDataset,
   MIL_ID            TestDataset);

void AddClassDescription(
   MIL_ID            MilSystem,
   MIL_ID            Dataset,
   const MIL_STRING* FabricsClassName,
   const MIL_STRING* FabricsClassIcon,
   MIL_INT           NumberOfClasses);
void AddClassToDataset(
   MIL_INT           ClassIndex,
   const MIL_STRING& DataToTrainPath,
   const MIL_STRING& FabricName,
   MIL_ID            Dataset);
void GetSizes(
   MIL_ID   MilSysId,
   MIL_ID   Dataset,
   MIL_INT* SizeX,
   MIL_INT* SizeY);

void ListFilesInFolder(const MIL_STRING& FolderName, std::vector<MIL_STRING>& FilesInFolder);
void PrepareExampleDataFolder(
   const MIL_STRING& ExampleDataPath,
   const MIL_STRING* FabricsClassName,
   MIL_INT           NumberOfClasses);

MIL_UNIQUE_CLASS_ID TrainTheModel(
   MIL_ID MilSystem,
   MIL_ID WorkingDataset,
   MIL_ID MilDisplay,
   MIL_ID TestPrepareDataCtx);

void CreateFolder(const MIL_STRING& FolderPath);
void SetupTrainDataPreparationContext(MIL_ID TrainPrepareDataCtx, MIL_ID WorkingDataset, bool* pIsDevDataset);
void SetupTestDataPreparationContext(MIL_ID TrainPrepareDataCtx, MIL_ID TestPrepareDataCtx);

MIL_STRING ConvertPrepareDataStatusToStr(MIL_INT Status);

void ExportTrainAndDevDatasets(MIL_ID MilSystem, MIL_ID TrainRes);

void PredictUsingTrainedContext(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID TrainedCtx,
   MIL_ID TestDataset,
   MIL_ID TestPrepareDataCtx);

void PrepareTestDataset(
   MIL_ID            TrainedCtx,
   MIL_ID            TestPrepareDataCtx,
   MIL_ID            TestDataset,
   MIL_ID            PreparedTestDataset,
   const MIL_STRING& PrepareDataPath);

const std::vector<MIL_INT> CreateShuffledIndex(MIL_INT NbEntries, unsigned int Seed);

//............................................................................
class CTrainEvolutionDashboard
   {
   public:
      CTrainEvolutionDashboard(
         MIL_ID      MilSystem,
         MIL_INT     MaxEpoch,
         MIL_INT     MinibatchSize,
         MIL_DOUBLE  LearningRate,
         MIL_INT     TrainImageSizeX,
         MIL_INT     TrainImageSizeY,
         MIL_INT     TrainDatasetSize,
         MIL_INT     DevDatasetSize,
         MIL_INT     TrainEngineUsed,
         MIL_STRING& TrainEngineDescription);

      ~CTrainEvolutionDashboard();

      void AddEpochData(
         MIL_DOUBLE TrainErrorRate,
         MIL_DOUBLE DevErrorRate,
         MIL_INT    CurEpoch,
         bool       TheEpochIsTheBestUpToNow,
         MIL_DOUBLE EpochBenchMean);
      void AddMiniBatchData(
         MIL_DOUBLE LossError,
         MIL_INT    MinibatchIdx,
         MIL_INT    EpochIdx,
         MIL_INT    NbBatchPerEpoch);
      void AddDatasetsPreparedData(MIL_INT TrainDatasetSize, MIL_INT DevDatasetSize);

      MIL_ID GetDashboardBufId() const { return m_DashboardBufId; }

   protected:
      void UpdateEpochInfo(
         MIL_DOUBLE TrainErrorRate,
         MIL_DOUBLE DevErrorRate,
         MIL_INT    CurEpoch,
         bool       TheEpochIsTheBestUpToNow);

      void UpdateLoss(MIL_DOUBLE LossError);

      void UpdateEpochGraph(
         MIL_DOUBLE TrainErrorRate,
         MIL_DOUBLE DevErrorRate,
         MIL_INT    CurEpoch);
      void UpdateLossGraph(
         MIL_DOUBLE LossError,
         MIL_INT    MiniBatchIdx,
         MIL_INT    EpochIdx,
         MIL_INT    NbBatchPerEpoch);

      void UpdateProgression(
         MIL_INT MinibatchIdx,
         MIL_INT EpochIdx,
         MIL_INT NbBatchPerEpoch);

      void UpdateDatasetsSize(MIL_INT TrainDatasetSize, MIL_INT DevDatasetSize);

      void DrawSectionSeparators();

      void DrawBufferFrame(MIL_ID BufId, MIL_INT FrameThickness);

      void InitializeEpochGraph();

      void InitializeLossGraph();

      void WriteGeneralTrainInfo(
         MIL_INT     MinibatchSize,
         MIL_INT     TrainImageSizeX,
         MIL_INT     TrainImageSizeY,
         MIL_INT     TrainDatasetSize,
         MIL_INT     DevDatasetSize,
         MIL_DOUBLE  LearningRate,
         MIL_INT     TrainEngineUsed,
         MIL_STRING& TrainEngineDescription);

      MIL_UNIQUE_BUF_ID m_DashboardBufId;
      MIL_UNIQUE_GRA_ID m_TheGraContext;

      MIL_UNIQUE_BUF_ID m_EpochInfoBufId;
      MIL_UNIQUE_BUF_ID m_EpochGraphBufId;
      MIL_UNIQUE_BUF_ID m_LossInfoBufId;
      MIL_UNIQUE_BUF_ID m_LossGraphBufId;
      MIL_UNIQUE_BUF_ID m_ProgressionInfoBufId;

      MIL_INT m_MaxEpoch;
      MIL_INT m_DashboardWidth;
      MIL_INT m_LastTrainPosX;
      MIL_INT m_LastTrainPosY;
      MIL_INT m_LastDevPosX;
      MIL_INT m_LastDevPosY;
      MIL_INT m_LastTrainMinibatchPosX;
      MIL_INT m_LastTrainMinibatchPosY;

      MIL_INT m_YPositionForLossText;

      MIL_DOUBLE m_EpochBenchMean;

      // Constants useful for the graph.
      MIL_INT GRAPH_SIZE_X;
      MIL_INT GRAPH_SIZE_Y;
      MIL_INT GRAPH_TOP_MARGIN;
      MIL_INT MARGIN;
      MIL_INT EPOCH_AND_MINIBATCH_REGION_HEIGHT;
      MIL_INT PROGRESSION_INFO_REGION_HEIGHT;

      MIL_INT LOSS_EXPONENT_MAX;
      MIL_INT LOSS_EXPONENT_MIN;

      MIL_DOUBLE COLOR_GENERAL_INFO;
      MIL_DOUBLE COLOR_DEV_SET_INFO;
      MIL_DOUBLE COLOR_TRAIN_SET_INFO;
      MIL_DOUBLE COLOR_PROGRESS_BAR;
   };

//............................................................................
struct HookEpochData
   {
   CTrainEvolutionDashboard* TheDashboard;
   };

//............................................................................
struct HookMiniBatchData
   {
   CTrainEvolutionDashboard* TheDashboard;
   };

//............................................................................
struct HookDatasetsPreparedData
   {
   CTrainEvolutionDashboard* TheDashboard;
   MIL_ID m_MilSystem;
   MIL_ID m_MilDisplay;
   };

//............................................................................
MIL_INT MFTYPE HookFuncEpoch(
   MIL_INT HookType,
   MIL_ID  EventId,
   void*   UserData);
MIL_INT MFTYPE HookFuncMiniBatch(
   MIL_INT HookType,
   MIL_ID  EventId,
   void*   UserData);
MIL_INT MFTYPE HookFuncDatasetsPrepared(
   MIL_INT HookType,
   MIL_ID  EventId,
   void*   UserData);
MIL_INT MFTYPE HookFuncPrpDataTrainAndDevSetEntryPost(
   MIL_INT HookType,
   MIL_ID  EventId,
   void*   UserData);
MIL_INT MFTYPE HookFuncPrpDataTestSetEntryPost(
   MIL_INT HookType,
   MIL_ID  EventId,
   void* UserData);

//............................................................................
class CPredictResultDisplay
   {
   public:
      CPredictResultDisplay(
         MIL_ID MilSystem,
         MIL_ID MilDisplay,
         MIL_ID TestDataset);

      ~CPredictResultDisplay();

      void Update(
         MIL_ID     ImageToPredict,
         MIL_INT    BestIndex,
         MIL_DOUBLE BestScore);

   protected:
      MIL_ID  m_MilSystem;
      MIL_ID  m_MilDisplay;
      MIL_INT m_MaxTrainImageSize;

      MIL_UNIQUE_BUF_ID m_MilDispImage;
      MIL_UNIQUE_BUF_ID m_MilDispChild;
      MIL_ID            m_MilOverlay;

      MIL_UNIQUE_GRA_ID m_GraContext;

      const MIL_DOUBLE COLOR_PREDICT_INFO;
      const MIL_INT    MARGIN;
   };

//............................................................................
class CDatasetViewer
   {
   public:
      CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth);

   private:
      void PrintControls();

   private:
      MIL_ID  m_MilSystem;
      MIL_ID  m_Dataset;

      const MIL_INT Y_MARGIN;
      const MIL_INT TEXT_MARGIN;
   };

// ****************************************************************************
//    Main.
// ****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   if(IsTrainingSupportedOnPlatform(MilSystem) != M_TRUE)
      {
      MosPrintf(MIL_TEXT("Press <enter> to end.\n"));
      MosGetch();
      return -1;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Display a representative image of all classes.
   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   MIL_UNIQUE_CLASS_ID WorkingDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID TestDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);

   MclassControl(WorkingDataset, M_CONTEXT, M_ROOT_PATH, GetExampleCurrentDirectory());
   MclassControl(TestDataset, M_CONTEXT, M_ROOT_PATH, GetExampleCurrentDirectory());

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("LOADING AND CONVERTING THE DATASETS...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));
   SetupTheDatasets(
      MilSystem,
      FABRICS_CLASS_NAME,
      FABRICS_CLASS_ICON,
      NUMBER_OF_CLASSES,
      EXAMPLE_ORIGINAL_DATA_PATH,
      EXAMPLE_DATA_PATH,
      WorkingDataset,
      TestDataset);

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("TRAINING... THIS WILL TAKE SOME TIME...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   // We will need to save the internal prepare data context (to apply it to the test set).
   MIL_UNIQUE_CLASS_ID TestPrepareDataCtx = MclassAlloc(MilSystem, M_PREPARE_IMAGES_CNN, M_DEFAULT, M_UNIQUE_ID);

   MIL_UNIQUE_CLASS_ID TrainedCtx = TrainTheModel(MilSystem, WorkingDataset, MilDisplay, TestPrepareDataCtx);

   if(TrainedCtx)
      {
      MosPrintf(MIL_TEXT("\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("PREDICTING USING THE TRAINED CONTEXT...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n"));

      PredictUsingTrainedContext(MilSystem, MilDisplay, TrainedCtx, TestDataset, TestPrepareDataCtx);
      }
   else
      {
      MosPrintf(MIL_TEXT("\nTraining has not completed properly !!!!!!!!!!!!!!\n"));
      MosPrintf(MIL_TEXT("Press <enter> to end...\n"));
      MosGetch();
      }

   return 0;
   }

//............................................................................
MIL_INT IsTrainingSupportedOnPlatform(MIL_ID MilSystem)
   {
   // Validate that the MilSystem is allocated on a 64-bit platform.
   MIL_ID MilSystemOwnerApp = M_NULL;
   MsysInquire(MilSystem, M_OWNER_APPLICATION, &MilSystemOwnerApp);

   MIL_INT SystemPlatformBitness = 0;
   MappInquire(MilSystemOwnerApp, M_PLATFORM_BITNESS, &SystemPlatformBitness);

   MIL_INT SystemOsType = M_NULL;
   MappInquire(MilSystemOwnerApp, M_PLATFORM_OS_TYPE, &SystemOsType);

   // Verify if the platform is supported for training.
   bool SupportedTrainingPlaform = ((SystemPlatformBitness == 64) && (SystemOsType == M_OS_WINDOWS));
   if(!SupportedTrainingPlaform)
      {
      MosPrintf(MIL_TEXT("\n***** MclassTrain() is available only for Windows 64-bit platforms. *****\n"));
      return M_FALSE;
      }

   // If no train engine is installed on the MIL system then the train example cannot run.
   if(CnnTrainEngineDLLInstalled(MilSystem) != M_TRUE)
      {
      MosPrintf(MIL_TEXT("\n***** No train engine installed, MclassTrain() cannot run! *****\n"));
      return M_FALSE;
      }

   return M_TRUE;
   }

//............................................................................
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem)
   {
   MIL_INT IsInstalled = M_FALSE;

   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_CNN, M_DEFAULT, M_UNIQUE_ID);
   MclassInquire(TrainCtx, M_DEFAULT, M_TRAIN_ENGINE_IS_INSTALLED + M_TYPE_MIL_INT, &IsInstalled);

   return IsInstalled;
   }

//............................................................................
MIL_UNIQUE_BUF_ID CreateImageOfAllClasses(
   MIL_ID            MilSystem,
   const MIL_STRING* FabricClassIcon,
   MIL_INT           NumberOfClasses)
   {
   MIL_INT MaxSizeY = MIL_INT_MIN;
   MIL_INT SumSizeX = 0;
   std::vector<MIL_UNIQUE_BUF_ID> IconsToDisplay;
   for(MIL_INT i = 0; i < NumberOfClasses; i++)
      {
      IconsToDisplay.push_back(MbufRestore(FabricClassIcon[i], MilSystem, M_UNIQUE_ID));
      MIL_INT SizeX = MbufInquire(IconsToDisplay.back(), M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(IconsToDisplay.back(), M_SIZE_Y, M_NULL);

      MaxSizeY = std::max<MIL_INT>(SizeY, MaxSizeY);
      SumSizeX = SumSizeX + SizeX;
      }

   MIL_UNIQUE_BUF_ID AllClassesImage = MbufAllocColor(MilSystem, 3, SumSizeX, MaxSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(AllClassesImage, 0.0);

   MIL_UNIQUE_GRA_ID GraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);
   MgraColor(GraContext, M_COLOR_BLUE);

   MIL_INT CurXOffset = 0;
   for(const auto& IconImage : IconsToDisplay)
      {
      MIL_INT SizeX = MbufInquire(IconImage, M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(IconImage, M_SIZE_Y, M_NULL);

      MbufCopyColor2d(IconImage, AllClassesImage, M_ALL_BANDS, 0, 0, M_ALL_BANDS, CurXOffset, 0, SizeX, SizeY);
      MgraRect(GraContext, AllClassesImage, CurXOffset, 0, CurXOffset + SizeX - 1, SizeY - 1);
      CurXOffset += SizeX;
      }

   return AllClassesImage;
   }

//............................................................................
MIL_STRING GetExampleCurrentDirectory()
   {
   DWORD CurDirStrSize = GetCurrentDirectory(0, NULL) + 1;

   std::vector<MIL_TEXT_CHAR> vCurDir(CurDirStrSize, 0);
   GetCurrentDirectory(CurDirStrSize, (LPTSTR)&vCurDir[0]);

   MIL_STRING sRet = &vCurDir[0];
   return sRet;
   }

//............................................................................
void SetupTheDatasets(
   MIL_ID            MilSystem,
   const MIL_STRING* FabricsClassName,
   const MIL_STRING* FabricsClassIcon,
   MIL_INT           NumberOfClasses,
   const MIL_STRING& OriginalDataPath,
   const MIL_STRING& ExampleDataPath,
   MIL_ID            WorkingDataset,
   MIL_ID            TestDataset)
   {
   // Create the ExampleDataPath folder if it does not already exist. If it does exist, remove
   // the Images folder inside to ensure repeatability.
   PrepareExampleDataFolder(ExampleDataPath, FabricsClassName, NumberOfClasses);

   MIL_UNIQUE_CLASS_ID FullDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   AddClassDescription(MilSystem, FullDataset, FabricsClassName, FabricsClassIcon, NumberOfClasses);
   for(MIL_INT ClassIdx = 0; ClassIdx < NumberOfClasses; ClassIdx++)
      {
      AddClassToDataset(ClassIdx, OriginalDataPath, FabricsClassName[ClassIdx], FullDataset);
      }

   // Copy the dataset to the ExampleDataPath folder and update the paths of the entries
   // to the new location so we do not modify the original data (at OriginalDataPath).
   MclassControl(FullDataset, M_CONTEXT, M_CONSOLIDATE_ENTRIES_INTO_FOLDER, ExampleDataPath);

   const MIL_STRING ConsolidatedDatasetPath = GetExampleCurrentDirectory() + MIL_TEXT("\\") + ExampleDataPath;

   MosPrintf(MIL_TEXT("The dataset was loaded and converted to Matrox format.\n"));
   MosPrintf(MIL_TEXT("The dataset has been consolidated in Matrox format and images \ncan be found here: %s \n\n"), ConsolidatedDatasetPath.c_str());

   MosPrintf(MIL_TEXT("Press <v> to view the converted dataset.\nPress <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if(KeyVal == 'v' || KeyVal == 'V')
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE CONVERTED DATASET...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(MilSystem, FullDataset, true);
      }

   MosPrintf(MIL_TEXT("\nSplitting the dataset to working and test datasets...\n"));
   // The dataset will be split Test=10%, Working will be automatically split.
   const MIL_DOUBLE PERCENTAGE_IN_TEST_DATASET = 10.0;

   // Create the test dataset. The train and dev datasets will be automatically handled by the training.
   MclassSplitDataset(M_SPLIT_CONTEXT_FIXED_SEED, FullDataset, WorkingDataset, TestDataset,
                      100.0 - PERCENTAGE_IN_TEST_DATASET, M_NULL, M_DEFAULT);

   // Save the datasets. Uncomment if required...
   // MclassSave(MIL_TEXT("TestDataset.mclassd"), TestDataset, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nA test dataset was created using %.lf%% of the original images.\n"), PERCENTAGE_IN_TEST_DATASET);
   MosPrintf(MIL_TEXT("Press <enter> to continue...\n"));
   MosGetch();
   }

//............................................................................
void PrepareExampleDataFolder(
   const MIL_STRING& ExampleDataPath,
   const MIL_STRING* FabricsClassName,
   MIL_INT           NumberOfClasses)
   {
   MIL_INT ExampleDataPathExists = M_NO;
   MappFileOperation(M_DEFAULT, ExampleDataPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &ExampleDataPathExists);
   if(ExampleDataPathExists == M_YES)
      {
      MappFileOperation(M_DEFAULT, ExampleDataPath, M_NULL, M_NULL, M_FILE_DELETE_DIR, M_RECURSIVE, M_NULL);
      }
   CreateFolder(ExampleDataPath);
   }

//............................................................................
void ListFilesInFolder(const MIL_STRING& FolderName, std::vector<MIL_STRING>& FilesInFolder)
   {
   MIL_STRING FileToSearch = FolderName;
   FileToSearch += MIL_TEXT("*.*");

   MIL_INT NumberOfFiles;
   MappFileOperation(M_DEFAULT, FileToSearch, M_NULL, M_NULL, M_FILE_NAME_FIND_COUNT, M_DEFAULT, &NumberOfFiles);
   FilesInFolder.resize(NumberOfFiles);

   for(MIL_INT i = 0; i < NumberOfFiles; i++)
      {
      MIL_STRING Filename;
      MappFileOperation(M_DEFAULT, FileToSearch, M_NULL, M_NULL, M_FILE_NAME_FIND, i, Filename);
      FilesInFolder[i] = FolderName + Filename;
      }
   }

//............................................................................
void AddClassDescription(
   MIL_ID            MilSystem,
   MIL_ID            Dataset,
   const MIL_STRING* FabricsClassName,
   const MIL_STRING* FabricsClassIcon,
   MIL_INT           NumberOfClasses)
   {
   for(MIL_INT i = 0; i < NumberOfClasses; i++)
      {
      MclassControl(Dataset, M_DEFAULT, M_CLASS_ADD, FabricsClassName[i]);
      MIL_UNIQUE_BUF_ID IconImageId = MbufRestore(FabricsClassIcon[i], MilSystem, M_UNIQUE_ID);
      MclassControl(Dataset, M_CLASS_INDEX(i), M_CLASS_ICON_ID, IconImageId);
      }
   }

//............................................................................
void AddClassToDataset(
   MIL_INT           ClassIndex,
   const MIL_STRING& DataToTrainPath,
   const MIL_STRING& FabricName,
   MIL_ID            Dataset)
   {
   MIL_INT NbEntries;
   MclassInquire(Dataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);

   MIL_STRING FolderName = DataToTrainPath + FabricName + MIL_TEXT("/");

   std::vector<MIL_STRING> FilesInFolder;
   ListFilesInFolder(FolderName, FilesInFolder);

   MIL_INT CurImageIndex = 0;
   for(const auto& File : FilesInFolder)
      {
      MclassControl(Dataset, M_DEFAULT, M_ENTRY_ADD, M_DEFAULT);
      MclassControlEntry(Dataset, NbEntries + CurImageIndex, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH, ClassIndex, M_NULL, M_DEFAULT);
      MclassControlEntry(Dataset, NbEntries + CurImageIndex, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, M_DEFAULT, File, M_DEFAULT);
      CurImageIndex++;
      }
   }

//............................................................................
void GetSizes(
   MIL_ID   MilSysId,
   MIL_ID   Dataset,
   MIL_INT* SizeX,
   MIL_INT* SizeY)
   {
   MIL_STRING EntryImagePathAbs;
   MclassInquireEntry(Dataset, 0, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH_ABS, EntryImagePathAbs);
   MbufDiskInquire(EntryImagePathAbs, M_SIZE_X, SizeX);
   MbufDiskInquire(EntryImagePathAbs, M_SIZE_Y, SizeY);
   }

//............................................................................
MIL_UNIQUE_CLASS_ID TrainTheModel(
   MIL_ID MilSystem,
   MIL_ID WorkingDataset,
   MIL_ID MilDisplay,
   MIL_ID TestPrepareDataCtx)
   {
   // Initialize to 0, the display will be updated with the HookDatasetsPreparedData.
   const MIL_INT TrainDatasetNbImages = 0;
   const MIL_INT DevDatasetNbImages = 0;

   // Allocate a context and a result for the training.
   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_CNN, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID TrainRes = MclassAllocResult(MilSystem, M_TRAIN_CNN_RESULT, M_DEFAULT, M_UNIQUE_ID);

   // Set the path for the data preparation.
   CreateFolder(EXAMPLE_PREPARED_DATA_PATH);
   MclassControl(TrainCtx, M_CONTEXT, M_TRAIN_DESTINATION_FOLDER, EXAMPLE_PREPARED_DATA_PATH);

   // Use the proper parameters for the training context.
   const MIL_INT    MAX_NUMBER_OF_EPOCH = 10;
   const MIL_INT    MINI_BATCH_SIZE = 64;
   const MIL_DOUBLE LEARNING_RATE = 0.001;
   MclassControl(TrainCtx, M_DEFAULT, M_MAX_EPOCH, MAX_NUMBER_OF_EPOCH);
   MclassControl(TrainCtx, M_DEFAULT, M_MINI_BATCH_SIZE, MINI_BATCH_SIZE);
   MclassControl(TrainCtx, M_DEFAULT, M_INITIAL_LEARNING_RATE, LEARNING_RATE);

   // Get the internal prepare data context from the train context.
   MIL_ID TrainPrepareDataCtx;
   MclassInquire(TrainCtx, M_CONTEXT, M_PREPARE_DATA_CONTEXT_ID + M_TYPE_MIL_ID, &TrainPrepareDataCtx);

   bool IsDevDataset = false;
   // Adjust parameters for the data augmentation.
   SetupTrainDataPreparationContext(TrainPrepareDataCtx, WorkingDataset, &IsDevDataset);

   // Save and set up the internal prepare data context for later use.
   SetupTestDataPreparationContext(TrainPrepareDataCtx, TestPrepareDataCtx);

   MclassPreprocess(TrainCtx, M_DEFAULT);

   MIL_INT TrainEngineUsed;
   MclassInquire(TrainCtx, M_CONTEXT, M_TRAIN_ENGINE_USED + M_TYPE_MIL_INT, &TrainEngineUsed);

   if(TrainEngineUsed == M_GPU)
      {
      MIL_INT GpuTrainEngineStatus;
      MclassInquire(TrainCtx, M_CONTEXT, M_GPU_TRAIN_ENGINE_LOAD_STATUS + M_TYPE_MIL_INT, &GpuTrainEngineStatus);
      if(GpuTrainEngineStatus == M_JIT_COMPILATION_REQUIRED)
         {
         MosPrintf(MIL_TEXT("\nWarning :: The training might not be optimal for the current system.\n"));
         MosPrintf(MIL_TEXT("Use the CNN Train Engine Test under Classification in MILConfig for more information.\n"));
         MosPrintf(MIL_TEXT("It may take some time before displaying the first results...\n"));
         }
      }
   else if(TrainEngineUsed == M_CPU)
      {
      MosPrintf(MIL_TEXT("\nWarning :: The training is being done on the CPU.\n"));
      MosPrintf(MIL_TEXT("If a training on GPU was expected, use the CNN Train Engine Test under Classification in MILConfig for more information.\n"));
      }

   MIL_STRING TrainEngineDescription;
   MclassInquire(TrainCtx, M_CONTEXT, M_TRAIN_ENGINE_USED_DESCRIPTION, TrainEngineDescription);

   MIL_INT ImageSizeX = 0;
   MIL_INT ImageSizeY = 0;
   GetSizes(MilSystem, WorkingDataset, &ImageSizeX, &ImageSizeY);

   // Initialize the object responsible for displaying the train evolution.
   CTrainEvolutionDashboard TheTrainEvolutionDashboard(
      MilSystem,
      MAX_NUMBER_OF_EPOCH,
      MINI_BATCH_SIZE,
      LEARNING_RATE,
      ImageSizeX,
      ImageSizeY,
      TrainDatasetNbImages,
      DevDatasetNbImages,
      TrainEngineUsed,
      TrainEngineDescription);

   // Initialize the hook associated to the epoch trained event.
   HookEpochData TheHookEpochData;
   TheHookEpochData.TheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_EPOCH_TRAINED, HookFuncEpoch, &TheHookEpochData);

   // Initialize the hook associated to the mini batch trained event.
   HookMiniBatchData TheHookMiniBatchData;
   TheHookMiniBatchData.TheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_MINI_BATCH_TRAINED, HookFuncMiniBatch, &TheHookMiniBatchData);

   // Initialize the hook associated to the datasets prepared event.
   HookDatasetsPreparedData TheHookDatasetsPreparedData;
   TheHookDatasetsPreparedData.TheDashboard = &TheTrainEvolutionDashboard;
   TheHookDatasetsPreparedData.m_MilSystem  = MilSystem;
   TheHookDatasetsPreparedData.m_MilDisplay = MilDisplay;
   MclassHookFunction(TrainCtx, M_DATASETS_PREPARED, HookFuncDatasetsPrepared, &TheHookDatasetsPreparedData);

   MosPrintf(MIL_TEXT("Preparing the train and dev datasets...\n"));

   // Start the training process.
   MclassTrain(TrainCtx, M_NULL, WorkingDataset, M_NULL, TrainRes, M_DEFAULT);

   MIL_UNIQUE_CLASS_ID TrainedCtx;

   // Check the training status to ensure the training has completed properly.
   MIL_INT Status = -1;
   MclassGetResult(TrainRes, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);
   if(Status == M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("\nTraining was successful.\n"));

      // Check if at some point there were missing train images.
      MIL_INT NbErrorImage = -1;
      MclassGetResult(TrainRes, M_DEFAULT, M_TRAIN_DATASET_ERROR_ENTRIES + M_NB_ELEMENTS + M_TYPE_MIL_INT, &NbErrorImage);
      if(NbErrorImage != 0)
         {
         MosPrintf(MIL_TEXT("Warning :: few images (%d) were missing at some part of the training.\n"), NbErrorImage);
         }

      TrainedCtx = MclassAlloc(MilSystem, M_CLASSIFIER_CNN_PREDEFINED, M_DEFAULT, M_UNIQUE_ID);
      MclassCopyResult(TrainRes, M_DEFAULT, TrainedCtx, M_DEFAULT, M_TRAINED_CLASSIFIER, M_DEFAULT);

      // Export the prepared train and dev dataset. Uncomment if required...
      // ExportTrainAndDevDatasets(MilSystem, TrainRes);

      MosPrintf(MIL_TEXT("A training report was saved: \"TrainReport.csv\".\n"));
      MclassExport(MIL_TEXT("TrainReport.csv"), M_FORMAT_TXT, TrainRes, M_DEFAULT, M_TRAIN_REPORT, M_DEFAULT);

      MIL_DOUBLE TrainErrorRate = 0;
      MclassGetResult(TrainRes, M_DEFAULT, M_TRAIN_DATASET_ERROR_RATE, &TrainErrorRate);
      MIL_DOUBLE DevErrorRate = 0;
      MclassGetResult(TrainRes, M_DEFAULT, M_DEV_DATASET_ERROR_RATE, &DevErrorRate);

      MIL_INT LastUpdatedEpochIndex;
      MclassGetResult(TrainRes, M_DEFAULT, M_LAST_EPOCH_UPDATED_PARAMETERS + M_TYPE_MIL_INT, &LastUpdatedEpochIndex);

      MosPrintf(MIL_TEXT("\nThe best epoch was epoch %d with an error rate on the dev dataset of %.8lf.\n"), LastUpdatedEpochIndex, DevErrorRate);
      MosPrintf(MIL_TEXT("The associated train error rate is %.8lf.\n"), TrainErrorRate);

      MosPrintf(MIL_TEXT("Press <enter> to continue...\n"));
      MosGetch();
      }

   return TrainedCtx;
   }

//............................................................................
void CreateFolder(const MIL_STRING& FolderPath)
   {
   MIL_INT FolderExists = M_NO;
   MappFileOperation(M_DEFAULT, FolderPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FolderExists);
   if(FolderExists == M_NO)
      {
      MappFileOperation(M_DEFAULT, FolderPath, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
      }
   }

//............................................................................
void SetupTrainDataPreparationContext(MIL_ID TrainPrepareDataCtx, MIL_ID WorkingDataset, bool* pIsDevDataset)
   {
   MIL_ID AugmentContext;
   MclassInquire(TrainPrepareDataCtx, M_CONTEXT, M_AUGMENT_CONTEXT_ID + M_TYPE_MIL_ID, &AugmentContext);

   const MIL_INT NumberOfEntries = MclassInquire(WorkingDataset, M_CONTEXT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, M_NULL);

   // On average, we do two augmentations per image + the original images.
   MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_AUGMENT_NUMBER_FACTOR, NB_AUGMENTATION_PER_IMAGE);

   // Ensure repeatability with a fixed seed.
   MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_SEED_MODE, M_USER_DEFINED);
   MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_SEED_VALUE, 42);

   // Translation augmentation and presets in the prepare data context.
   // MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_PRESET_TRANSLATION, M_ENABLE);
   MimControl(AugmentContext, M_AUG_TRANSLATION_X_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_TRANSLATION_X_OP_MAX, 2);
   MimControl(AugmentContext, M_AUG_TRANSLATION_Y_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_TRANSLATION_Y_OP_MAX, 2);

   // Scale augmentation and presets in the prepare data context.
   // MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_PRESET_SCALE, M_ENABLE);
   MimControl(AugmentContext, M_AUG_SCALE_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_SCALE_OP_FACTOR_MIN, 0.97);
   MimControl(AugmentContext, M_AUG_SCALE_OP_FACTOR_MAX, 1.03);

   // Rotation augmentation and presets in the prepare data context.
   // MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_PRESET_ROTATION, M_ENABLE);
   MimControl(AugmentContext, M_AUG_ROTATION_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_ROTATION_OP_ANGLE_DELTA, 5.0);

   // Overwrite prepared images between calls.
   MclassControl(TrainPrepareDataCtx, M_CONTEXT, M_DESTINATION_FOLDER_MODE, M_OVERWRITE);

   // Hook to show augmentations' progress.
   MclassHookFunction(TrainPrepareDataCtx, M_PREPARE_ENTRY_POST, HookFuncPrpDataTrainAndDevSetEntryPost, pIsDevDataset);
   }

//............................................................................
void SetupTestDataPreparationContext(MIL_ID TrainPrepareDataCtx, MIL_ID TestPrepareDataCtx)
   {
   // Copy train data preparation context to the test data preparation context.
   std::vector<MIL_UINT8> Mem;
   MclassStream(Mem, M_NULL, M_SAVE, M_MEMORY, M_DEFAULT, M_DEFAULT, &TrainPrepareDataCtx, M_NULL);
   MclassStream(Mem, M_NULL, M_LOAD, M_MEMORY, M_DEFAULT, M_DEFAULT, &TestPrepareDataCtx, M_NULL);

   // Disable augmentations for the test set.
   MclassControl(TestPrepareDataCtx, M_CONTEXT, M_AUGMENT_NUMBER_MODE, M_DISABLE);

   // Overwrite prepared images between calls.
   MclassControl(TestPrepareDataCtx, M_CONTEXT, M_DESTINATION_FOLDER_MODE, M_OVERWRITE);

   // When passing a classifier to MclassPrepareData, M_AUTO as M_SIZE_MODE.
   MclassControl(TestPrepareDataCtx, M_CONTEXT, M_SIZE_MODE, M_AUTO);

   // Hook to show the preparation's progress.
   MclassHookFunction(TestPrepareDataCtx, M_PREPARE_ENTRY_POST, HookFuncPrpDataTestSetEntryPost, nullptr);
   }

//............................................................................
MIL_STRING ConvertPrepareDataStatusToStr(MIL_INT Status)
   {
   switch(Status)
      {
      case M_COMPLETE:
         return MIL_TEXT("M_COMPLETE");
      case M_INVALID_AUG_OP_FOR_1_BAND_BUFFER:
         return MIL_TEXT("M_INVALID_AUG_OP_FOR_1_BAND_BUFFER");
      case M_INVALID_AUG_OP_FOR_1_BIT_BUFFER:
         return MIL_TEXT("M_INVALID_AUG_OP_FOR_1_BIT_BUFFER");
      case M_SOURCE_TOO_SMALL_FOR_DERICHE_OP:
         return MIL_TEXT("M_SOURCE_TOO_SMALL_FOR_DERICHE_OP");
      case M_FLOAT_IMAGE_NOT_NORMALIZED:
         return MIL_TEXT("M_FLOAT_IMAGE_NOT_NORMALIZED");
      case M_FAILED_TO_SAVE_IMAGE:
         return MIL_TEXT("M_FAILED_TO_SAVE_IMAGE");
      case M_IMAGE_FILE_NOT_FOUND:
         return MIL_TEXT("M_IMAGE_FILE_NOT_FOUND");
      case M_INVALID_BUFFER_SIGN_FOR_AUG:
         return MIL_TEXT("M_INVALID_BUFFER_SIGN_FOR_AUG");
      case M_INVALID_CENTER:
         return MIL_TEXT("M_INVALID_CENTER");
      case M_MASK_FILE_NOT_FOUND:
         return MIL_TEXT("M_MASK_FILE_NOT_FOUND");
      case M_RESIZED_IMAGE_TOO_SMALL:
         return MIL_TEXT("M_RESIZED_IMAGE_TOO_SMALL");
      default:
      case M_INTERNAL_ERROR:
         return MIL_TEXT("M_INTERNAL_ERROR");
      }
   }

//............................................................................
void ExportTrainAndDevDatasets(MIL_ID MilSystem, MIL_ID TrainRes)
   {
   const MIL_STRING ExportTrainDatasetPath = MIL_TEXT("TrainDataset");
   const MIL_STRING ExportDevDatasetPath = MIL_TEXT("DevDataset");
   CreateFolder(ExportTrainDatasetPath);
   CreateFolder(ExportDevDatasetPath);

   MIL_UNIQUE_CLASS_ID TrainDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID DevDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MclassCopyResult(TrainRes, M_DEFAULT, TrainDataset, M_DEFAULT, M_PREPARED_TRAIN_DATASET, M_DEFAULT);
   MclassCopyResult(TrainRes, M_DEFAULT, DevDataset, M_DEFAULT, M_PREPARED_DEV_DATASET, M_DEFAULT);

   MclassExport(ExportTrainDatasetPath, M_IMAGE_DATASET_FOLDER, TrainDataset, M_DEFAULT, M_COMPLETE, M_DEFAULT);
   MclassExport(ExportDevDatasetPath, M_IMAGE_DATASET_FOLDER, DevDataset, M_DEFAULT, M_COMPLETE, M_DEFAULT);
   }

//............................................................................
void PredictUsingTrainedContext(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID TrainedCtx,
   MIL_ID TestDataset,
   MIL_ID TestPrepareDataCtx)
   {
   // Test dataset with images cropped to the correct size.
   MIL_UNIQUE_CLASS_ID PreparedTestDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);

   // Create the TestSet folder.
   CreateFolder(EXAMPLE_PREPARED_DATA_PATH_TEST);

   // Prepare the test dataset.
   PrepareTestDataset(TrainedCtx, TestPrepareDataCtx, TestDataset, PreparedTestDataset, EXAMPLE_PREPARED_DATA_PATH_TEST);

   CPredictResultDisplay ThePredictResultDisplay(MilSystem, MilDisplay, PreparedTestDataset);

   MclassPreprocess(TrainedCtx, M_DEFAULT);

   // Create a predict context from the train result and classify with it.
   MIL_UNIQUE_CLASS_ID PredictedDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);

   MclassPredict(TrainedCtx, PreparedTestDataset, PredictedDataset, M_DEFAULT);

   MIL_INT    NbEntries = 0;
   MIL_INT    NbEntriesPredicted = 0;
   MIL_DOUBLE PredAvg = 0;
   MclassInquire(PredictedDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);
   MclassInquire(PredictedDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES_PREDICTED + M_TYPE_MIL_INT, &NbEntriesPredicted);
   MclassInquire(PredictedDataset, M_DEFAULT, M_PREDICTED_SCORE_AVERAGE, &PredAvg);

   // Shuffle the index of the test dataset to ensure classification is shown for all classes.
   const unsigned int ShuffledIndexSeed = 49;
   const std::vector<MIL_INT>& ShuffeldIndex = CreateShuffledIndex(NbEntries, ShuffledIndexSeed);

   const MIL_INT NbPredictionToShow = std::min<MIL_INT>(10, NbEntriesPredicted);

   MosPrintf(MIL_TEXT("Predictions will be performed on the test dataset as a final check\nof the trained CNN model.\n"));
   MosPrintf(MIL_TEXT("The test dataset contains %d images.\n"), NbEntries);
   MosPrintf(MIL_TEXT("The prediction results will be shown for the first %d images.\n"), NbPredictionToShow);

   MIL_INT NbGoodPredictions = 0;
   for(MIL_INT i = 0; i < NbEntries; i++)
      {
      // Check that entry was predicted.
      MIL_INT EntryPredicted = 0;
      MclassGetResultEntry(PredictedDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_CLASSIFICATION, M_DEFAULT, M_BEST_CLASS_INDEX + M_NB_ELEMENTS + M_TYPE_MIL_INT, &EntryPredicted);

      if(EntryPredicted == 1)
         {
         MIL_INT GroundTruthIndex;
         MclassInquireEntry(PreparedTestDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH + M_TYPE_MIL_INT, &GroundTruthIndex);

         MIL_INT PredIndex = 0;
         MclassGetResultEntry(PredictedDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_CLASSIFICATION, M_DEFAULT, M_BEST_CLASS_INDEX + M_TYPE_MIL_INT, &PredIndex);
         std::vector<MIL_DOUBLE> PredScores;
         MclassGetResultEntry(PredictedDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_CLASSIFICATION, M_DEFAULT, M_CLASS_SCORES, PredScores);

         if(PredIndex == GroundTruthIndex)
            NbGoodPredictions++;

         if(i < NbPredictionToShow)
            {
            MIL_STRING FilePath;
            MclassInquireEntry(PreparedTestDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, FilePath);

            MIL_UNIQUE_BUF_ID ImageToPredict = MbufRestore(FilePath, MilSystem, M_UNIQUE_ID);

            ThePredictResultDisplay.Update(ImageToPredict, PredIndex, PredScores[PredIndex]);
            MosPrintf(MIL_TEXT("The predicted index is %d and the predicted score is %.2lf%s (Ground truth=%d)\n"), PredIndex, PredScores[PredIndex], MIL_TEXT("%"), GroundTruthIndex);

            MosPrintf(MIL_TEXT("Press <enter> to continue...\n"));
            MosGetch();
            }
         }
      else
         {
         MIL_STRING FilePath;
         MclassInquireEntry(PreparedTestDataset, ShuffeldIndex[i], M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, FilePath);
         MosPrintf(MIL_TEXT("The image \"%s\" failed to be predicted.\n"), FilePath.c_str());
         }
      }

   const MIL_STRING SaveCtxName = MIL_TEXT("FabricsNet_Gray.mclass");
   MclassSave(SaveCtxName, TrainedCtx, M_DEFAULT);

   MosPrintf(MIL_TEXT("The accuracy on the test dataset using the trained context is %.2lf%s.\n"), ((MIL_DOUBLE)NbGoodPredictions / (MIL_DOUBLE)NbEntriesPredicted)*100.0, MIL_TEXT("%"));
   MosPrintf(MIL_TEXT("The average predicted score on the test dataset using the trained\ncontext is %.2lf%s.\n"), PredAvg, MIL_TEXT("%"));
   MosPrintf(MIL_TEXT("The trained context was saved: \"%s\".\n"), SaveCtxName.c_str());
   MosPrintf(MIL_TEXT("Press <enter> to end...\n"));

   MosGetch();
   }

//............................................................................
void PrepareTestDataset(
   MIL_ID            TrainedCtx,
   MIL_ID            TestPrepareDataCtx,
   MIL_ID            TestDataset,
   MIL_ID            PreparedTestDataset,
   const MIL_STRING& PrepareDataPath)
   {
   // Set the destination for the data preparation of the test set.
   MclassControl(TestPrepareDataCtx, M_CONTEXT, M_PREPARED_DATA_FOLDER, EXAMPLE_PREPARED_DATA_PATH_TEST);

   MclassPreprocess(TestPrepareDataCtx, M_DEFAULT);
   MclassPrepareData(TestPrepareDataCtx, TestDataset, PreparedTestDataset, TrainedCtx, M_DEFAULT);
   }

//............................................................................
const std::vector<MIL_INT> CreateShuffledIndex(MIL_INT NbEntries, unsigned int Seed)
   {
   std::vector<MIL_INT> IndexVector(NbEntries);
   std::iota(IndexVector.begin(), IndexVector.end(), 0);
   std::mt19937 gen(Seed);
   std::shuffle(IndexVector.begin(), IndexVector.end(), gen);
   return IndexVector;
   }

//............................................................................
MIL_INT MFTYPE HookFuncEpoch(
   MIL_INT /*HookType*/,
   MIL_ID  EventId,
   void*   UserData)
   {
   auto HookData = (HookEpochData *)UserData;

   MIL_DOUBLE CurBench = 0.0;
   MIL_DOUBLE CurBenchMean = -1.0;

   MIL_INT CurEpochIndex = 0;
   MclassGetHookInfo(EventId, M_EPOCH_INDEX + M_TYPE_MIL_INT, &CurEpochIndex);

   MappTimer(M_DEFAULT, M_TIMER_READ, &CurBench);
   MIL_DOUBLE EpochBenchMean = CurBench / (CurEpochIndex + 1);

   MIL_DOUBLE TrainErrorRate = 0;
   MclassGetHookInfo(EventId, M_TRAIN_DATASET_ERROR_RATE, &TrainErrorRate);
   MIL_DOUBLE DevErrorRate = 0;
   MclassGetHookInfo(EventId, M_DEV_DATASET_ERROR_RATE, &DevErrorRate);

   MIL_INT AreTrainedCNNParametersUpdated = M_FALSE;
   MclassGetHookInfo(EventId,
                     M_TRAINED_PARAMETERS_UPDATED + M_TYPE_MIL_INT,
                     &AreTrainedCNNParametersUpdated);
   // By default trained parameters are updated when the dev error rate
   // is the best up to now.
   bool TheEpochIsTheBestUpToNow = (AreTrainedCNNParametersUpdated == M_TRUE);

   HookData->TheDashboard->AddEpochData(
      TrainErrorRate, DevErrorRate,
      CurEpochIndex, TheEpochIsTheBestUpToNow, EpochBenchMean);

   return M_NULL;
   }

//............................................................................
MIL_INT MFTYPE HookFuncMiniBatch(
   MIL_INT /*HookType*/,
   MIL_ID  EventId,
   void*   UserData)
   {
   auto HookData = (HookMiniBatchData *)UserData;

   MIL_DOUBLE LossError = 0;
   MclassGetHookInfo(EventId, M_MINI_BATCH_LOSS, &LossError);

   MIL_INT MiniBatchIdx = 0;
   MclassGetHookInfo(EventId, M_MINI_BATCH_INDEX + M_TYPE_MIL_INT, &MiniBatchIdx);

   MIL_INT EpochIdx = 0;
   MclassGetHookInfo(EventId, M_EPOCH_INDEX + M_TYPE_MIL_INT, &EpochIdx);

   MIL_INT NbMiniBatchPerEpoch = 0;
   MclassGetHookInfo(EventId, M_MINI_BATCH_PER_EPOCH + M_TYPE_MIL_INT, &NbMiniBatchPerEpoch);

   if(EpochIdx == 0 && MiniBatchIdx == 0)
      {
      MappTimer(M_DEFAULT, M_TIMER_RESET, M_NULL);
      }

   HookData->TheDashboard->AddMiniBatchData(LossError, MiniBatchIdx, EpochIdx, NbMiniBatchPerEpoch);

   if(MosKbhit() != 0)
      {
      char KeyVal = (char)MosGetch();
      if(KeyVal == 'p')
         {
         MosPrintf(MIL_TEXT("\nPress 's' to stop the training or any other key to continue.\n"));
         while(MosKbhit() == 0)
            {
            KeyVal = (char)MosGetch();
            if(KeyVal == 's')
               {
               MIL_ID HookInfoTrainResId = M_NULL;
               MclassGetHookInfo(EventId, M_RESULT_ID + M_TYPE_MIL_ID, &HookInfoTrainResId);
               MclassControl(HookInfoTrainResId, M_DEFAULT, M_STOP_TRAIN, M_DEFAULT);
               MosPrintf(MIL_TEXT("The training has been stopped.\n"));
               break;
               }
            else
               {
               MosPrintf(MIL_TEXT("The training will continue.\n"));
               break;
               }
            }
         }
      }

   return M_NULL;
   }

//............................................................................
MIL_INT MFTYPE HookFuncDatasetsPrepared(
   MIL_INT /*HookType*/,
   MIL_ID  EventId,
   void*   UserData)
   {
   auto HookData = (HookDatasetsPreparedData*)UserData;

   MIL_ID TrainResult;
   MclassGetHookInfo(EventId, M_RESULT_ID + M_TYPE_MIL_ID, &TrainResult);

   MIL_UNIQUE_CLASS_ID TrainPreparedDataset = MclassAlloc(HookData->m_MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MclassCopyResult(TrainResult, M_DEFAULT, TrainPreparedDataset, M_DEFAULT, M_PREPARED_TRAIN_DATASET, M_DEFAULT);
   const MIL_INT TrainDatasetNbImages = MclassInquire(TrainPreparedDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);

   MIL_UNIQUE_CLASS_ID DevPreparedDataset = MclassAlloc(HookData->m_MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MclassCopyResult(TrainResult, M_DEFAULT, DevPreparedDataset, M_DEFAULT, M_PREPARED_DEV_DATASET, M_DEFAULT);
   const MIL_INT DevDatasetNbImages = MclassInquire(DevPreparedDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);

   HookData->TheDashboard->AddDatasetsPreparedData(TrainDatasetNbImages, DevDatasetNbImages);

   MosPrintf(MIL_TEXT("Press <v> to view the augmented train dataset.\nPress <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if(KeyVal == 'v' || KeyVal == 'V')
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE AUGMENTED TRAIN DATASET...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(HookData->m_MilSystem, TrainPreparedDataset, true);
      }

   MosPrintf(MIL_TEXT("\nThe training has started.\n"));
   MosPrintf(MIL_TEXT("It can be paused at any time by pressing 'p'.\n"));
   MosPrintf(MIL_TEXT("It can then be stopped or continued.\n"));

   MosPrintf(MIL_TEXT("\nDuring training, you can observe the displayed error rate of the train\n"));
   MosPrintf(MIL_TEXT("and dev datasets together with the evolution of the loss value...\n"));

   MdispSelect(HookData->m_MilDisplay, HookData->TheDashboard->GetDashboardBufId());

   return M_NULL;
   }

//............................................................................
MIL_INT MFTYPE HookFuncPrpDataTrainAndDevSetEntryPost(
   MIL_INT /*HookType*/,
   MIL_ID  EventId,
   void* UserData)
   {
   bool* pIsDevDataset = reinterpret_cast<bool*>(UserData);

   MIL_ID SrcDataset {M_NULL};
   MclassGetHookInfo(EventId, M_SRC_DATASET_ID + M_TYPE_MIL_ID, &SrcDataset);

   MIL_INT NumPrpEntries {0};
   MclassGetHookInfo(EventId, M_NUMBER_OF_PREPARED_SRC_ENTRIES + M_TYPE_MIL_INT, &NumPrpEntries);

   const MIL_INT NumEntries = MclassInquire(SrcDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);

   if(NumPrpEntries == 1)
      {
      *pIsDevDataset ? MosPrintf(MIL_TEXT("Preparing the dev dataset...\n")) :
         MosPrintf(MIL_TEXT("Augmenting the train dataset...\n"));
      }

   MIL_INT Status {-1};
   MclassGetHookInfo(EventId, M_STATUS + M_TYPE_MIL_INT, &Status);

   const MIL_STRING StatusStr = ConvertPrepareDataStatusToStr(Status);

   MIL_TEXT_CHAR EndOfLine = '\r';
   if(Status != M_COMPLETE)
      {
      EndOfLine = '\n';
      }

   MosPrintf(MIL_TEXT("Entry %d of %d completed with status: %s.%c"), NumPrpEntries, NumEntries, StatusStr.c_str(), EndOfLine);

   if(NumPrpEntries == NumEntries)
      {
      EndOfLine == '\r' ? MosPrintf(MIL_TEXT("\n\n")) : MosPrintf(MIL_TEXT("\n"));
      *pIsDevDataset = true;
      }

   return M_NULL;
   }

//............................................................................
MIL_INT MFTYPE HookFuncPrpDataTestSetEntryPost(
   MIL_INT /*HookType*/,
   MIL_ID  EventId,
   void* /*UserData*/)
   {
   MIL_ID SrcDataset {M_NULL};
   MclassGetHookInfo(EventId, M_SRC_DATASET_ID + M_TYPE_MIL_ID, &SrcDataset);

   MIL_INT NumPrpEntries {0};
   MclassGetHookInfo(EventId, M_NUMBER_OF_PREPARED_SRC_ENTRIES + M_TYPE_MIL_INT, &NumPrpEntries);

   const MIL_INT NumEntries = MclassInquire(SrcDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES, M_NULL);

   if(NumPrpEntries == 1)
      {
      MosPrintf(MIL_TEXT("Preparing the test dataset...\n"));
      }

   MIL_INT Status {-1};
   MclassGetHookInfo(EventId, M_STATUS + M_TYPE_MIL_INT, &Status);

   const MIL_STRING StatusStr = ConvertPrepareDataStatusToStr(Status);

   MIL_TEXT_CHAR EndOfLine = '\r';
   if(Status != M_COMPLETE)
      {
      EndOfLine = '\n';
      }

   MosPrintf(MIL_TEXT("Entry %d of %d completed with status: %s.%c"), NumPrpEntries, NumEntries, StatusStr.c_str(), EndOfLine);

   if(NumPrpEntries == NumEntries)
      {
      EndOfLine == '\r' ? MosPrintf(MIL_TEXT("\n\n")) : MosPrintf(MIL_TEXT("\n"));
      }

   return M_NULL;
   }

//............................................................................
CTrainEvolutionDashboard::CTrainEvolutionDashboard(
   MIL_ID      MilSystem,
   MIL_INT     MaxEpoch,
   MIL_INT     MinibatchSize,
   MIL_DOUBLE  LearningRate,
   MIL_INT     TrainImageSizeX,
   MIL_INT     TrainImageSizeY,
   MIL_INT     TrainDatasetSize,
   MIL_INT     DevDatasetSize,
   MIL_INT     TrainEngineUsed,
   MIL_STRING& TrainEngineDescription)
   : m_DashboardBufId(M_NULL)
   , m_TheGraContext(M_NULL)
   , m_EpochInfoBufId(M_NULL)
   , m_EpochGraphBufId(M_NULL)
   , m_LossInfoBufId(M_NULL)
   , m_LossGraphBufId(M_NULL)
   , m_ProgressionInfoBufId(M_NULL)
   , m_MaxEpoch(MaxEpoch)
   , m_DashboardWidth(0)
   , m_LastTrainPosX(0)
   , m_LastTrainPosY(0)
   , m_LastDevPosX(0)
   , m_LastDevPosY(0)
   , m_LastTrainMinibatchPosX(0)
   , m_LastTrainMinibatchPosY(0)
   , m_YPositionForLossText(0)
   , m_EpochBenchMean(-1.0)
   , GRAPH_SIZE_X(400)
   , GRAPH_SIZE_Y(400)
   , GRAPH_TOP_MARGIN(30)
   , MARGIN(50)
   , EPOCH_AND_MINIBATCH_REGION_HEIGHT(190)
   , PROGRESSION_INFO_REGION_HEIGHT(100)
   , LOSS_EXPONENT_MAX(0)
   , LOSS_EXPONENT_MIN(-5)
   , COLOR_GENERAL_INFO(M_RGB888(0, 176, 255))
   , COLOR_DEV_SET_INFO(M_COLOR_MAGENTA)
   , COLOR_TRAIN_SET_INFO(M_COLOR_GREEN)
   , COLOR_PROGRESS_BAR(M_COLOR_DARK_GREEN)
   {
   // One graph width.
   const MIL_INT GraphBoxWidth = GRAPH_SIZE_X + 2 * MARGIN;
   const MIL_INT GraphBoxHeight = GRAPH_SIZE_Y + MARGIN + GRAPH_TOP_MARGIN;
   // There are 2 graphs side by side.
   m_DashboardWidth = 2 * GraphBoxWidth;

   const MIL_INT DashboardHeight = EPOCH_AND_MINIBATCH_REGION_HEIGHT + GraphBoxHeight + PROGRESSION_INFO_REGION_HEIGHT;

   // Allocate the full dashboard buffer.
   m_DashboardBufId = MbufAllocColor(MilSystem, 3, m_DashboardWidth, DashboardHeight,
                                     8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(m_DashboardBufId, M_COLOR_BLACK);

   m_TheGraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);

   // Allocate child buffers for each different dashboard sections.
   const MIL_INT GraphYPosition = EPOCH_AND_MINIBATCH_REGION_HEIGHT;
   const MIL_INT ProgressionInfoYPosition = GraphYPosition + GraphBoxHeight;

   m_EpochInfoBufId = MbufChild2d(m_DashboardBufId, 0, 0, GraphBoxWidth, EPOCH_AND_MINIBATCH_REGION_HEIGHT, M_UNIQUE_ID);
   m_LossInfoBufId = MbufChild2d(m_DashboardBufId, GraphBoxWidth, 0, GraphBoxWidth, EPOCH_AND_MINIBATCH_REGION_HEIGHT, M_UNIQUE_ID);
   m_EpochGraphBufId = MbufChild2d(m_DashboardBufId, 0, GraphYPosition, GraphBoxWidth, GraphBoxHeight, M_UNIQUE_ID);
   m_LossGraphBufId = MbufChild2d(m_DashboardBufId, GraphBoxWidth, GraphYPosition, GraphBoxWidth, GraphBoxHeight, M_UNIQUE_ID);
   m_ProgressionInfoBufId = MbufChild2d(m_DashboardBufId, 0, ProgressionInfoYPosition, m_DashboardWidth, PROGRESSION_INFO_REGION_HEIGHT, M_UNIQUE_ID);

   // Initialize the different dashboard sections.
   DrawSectionSeparators();

   InitializeEpochGraph();
   InitializeLossGraph();

   WriteGeneralTrainInfo(
      MinibatchSize,
      TrainImageSizeX,
      TrainImageSizeY,
      TrainDatasetSize,
      DevDatasetSize,
      LearningRate,
      TrainEngineUsed,
      TrainEngineDescription);
   }

//............................................................................
CTrainEvolutionDashboard::~CTrainEvolutionDashboard()
   {
   m_TheGraContext = M_NULL;
   m_EpochInfoBufId = M_NULL;
   m_LossInfoBufId = M_NULL;
   m_EpochGraphBufId = M_NULL;
   m_LossGraphBufId = M_NULL;
   m_ProgressionInfoBufId = M_NULL;
   m_DashboardBufId = M_NULL;
   }

//............................................................................
void CTrainEvolutionDashboard::DrawBufferFrame(MIL_ID BufId, MIL_INT FrameThickness)
   {
   MIL_ID SizeX = MbufInquire(BufId, M_SIZE_X, M_NULL);
   MIL_ID SizeY = MbufInquire(BufId, M_SIZE_Y, M_NULL);

   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   MgraRectFill(m_TheGraContext, BufId, 0, 0, SizeX - 1, FrameThickness - 1);
   MgraRectFill(m_TheGraContext, BufId, SizeX - FrameThickness, 0, SizeX - 1, SizeY - 1);
   MgraRectFill(m_TheGraContext, BufId, 0, SizeY - FrameThickness, SizeX - 1, SizeY - 1);
   MgraRectFill(m_TheGraContext, BufId, 0, 0, FrameThickness - 1, SizeY - 1);
   }

//............................................................................
void CTrainEvolutionDashboard::DrawSectionSeparators()
   {
   // Draw a frame for the whole dashboard.
   DrawBufferFrame(m_DashboardBufId, 4);
   // Draw a frame for each section.
   DrawBufferFrame(m_EpochInfoBufId, 2);
   DrawBufferFrame(m_EpochGraphBufId, 2);
   DrawBufferFrame(m_LossInfoBufId, 2);
   DrawBufferFrame(m_LossGraphBufId, 2);
   DrawBufferFrame(m_ProgressionInfoBufId, 2);
   }

//............................................................................
void CTrainEvolutionDashboard::InitializeEpochGraph()
   {
   // Draw axis.
   MgraColor(m_TheGraContext, M_COLOR_WHITE);
   MgraRect(m_TheGraContext, m_EpochGraphBufId, MARGIN, GRAPH_TOP_MARGIN, MARGIN + GRAPH_SIZE_X, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN, MIL_TEXT("100"));
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.25*GRAPH_SIZE_Y)), MIL_TEXT("75"));
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.50*GRAPH_SIZE_Y)), MIL_TEXT("50"));
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.75*GRAPH_SIZE_Y)), MIL_TEXT("25"));
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y, MIL_TEXT("0"));

   MgraLine(m_TheGraContext, m_EpochGraphBufId, MARGIN, GRAPH_TOP_MARGIN + ((MIL_INT)(0.25*GRAPH_SIZE_Y)), MARGIN + 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.25*GRAPH_SIZE_Y)));
   MgraLine(m_TheGraContext, m_EpochGraphBufId, MARGIN, GRAPH_TOP_MARGIN + ((MIL_INT)(0.50*GRAPH_SIZE_Y)), MARGIN + 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.50*GRAPH_SIZE_Y)));
   MgraLine(m_TheGraContext, m_EpochGraphBufId, MARGIN, GRAPH_TOP_MARGIN + ((MIL_INT)(0.75*GRAPH_SIZE_Y)), MARGIN + 5, GRAPH_TOP_MARGIN + ((MIL_INT)(0.75*GRAPH_SIZE_Y)));

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);

   MIL_INT NbTick = std::min<MIL_INT>(m_MaxEpoch, 10);
   const MIL_INT EpochTickValue = m_MaxEpoch / NbTick;

   for(MIL_INT CurTick = 1; CurTick <= m_MaxEpoch; CurTick += EpochTickValue)
      {
      MIL_DOUBLE Percentage = (MIL_DOUBLE)CurTick / (MIL_DOUBLE)m_MaxEpoch;
      MIL_INT XOffset = (MIL_INT)(Percentage * GRAPH_SIZE_X);
      MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 5, M_TO_STRING(CurTick - 1));
      MgraLine(m_TheGraContext, m_EpochGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y - 5, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);
      }
   }

//............................................................................
void CTrainEvolutionDashboard::InitializeLossGraph()
   {
   // Draw axis.
   MgraColor(m_TheGraContext, M_COLOR_WHITE);
   MgraRect(m_TheGraContext, m_LossGraphBufId, MARGIN, GRAPH_TOP_MARGIN, MARGIN + GRAPH_SIZE_X, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);

   const MIL_INT NbLossValueTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;
   const MIL_DOUBLE TickRatio = 1.0 / (MIL_DOUBLE)NbLossValueTick;

   MIL_DOUBLE TickNum = 0.0;
   for(MIL_INT i = LOSS_EXPONENT_MAX; i >= LOSS_EXPONENT_MIN; i--)
      {
      MIL_TEXT_CHAR CurTickText[128];
      MosSprintf(CurTickText, 128, MIL_TEXT("1e%d"), i);

      MIL_INT TickYPos = (MIL_INT)(TickNum*TickRatio*GRAPH_SIZE_Y);
      MgraText(m_TheGraContext, m_LossGraphBufId, MARGIN - 5, GRAPH_TOP_MARGIN + TickYPos, CurTickText);
      if((i != LOSS_EXPONENT_MAX) && (i != LOSS_EXPONENT_MIN))
         {
         MgraLine(m_TheGraContext, m_LossGraphBufId, MARGIN, GRAPH_TOP_MARGIN + TickYPos, MARGIN + 5, GRAPH_TOP_MARGIN + TickYPos);
         }
      TickNum = TickNum + 1.0;
      }

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);

   const MIL_INT NbEpochTick = std::min<MIL_INT>(m_MaxEpoch, 10);
   const MIL_INT EpochTickValue = m_MaxEpoch / NbEpochTick;

   for(MIL_INT CurTick = 1; CurTick <= m_MaxEpoch; CurTick += EpochTickValue)
      {
      MIL_DOUBLE Percentage = (MIL_DOUBLE)CurTick / (MIL_DOUBLE)m_MaxEpoch;
      MIL_INT XOffset = (MIL_INT)(Percentage * GRAPH_SIZE_X);
      MgraText(m_TheGraContext, m_LossGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 5, M_TO_STRING(CurTick - 1));
      MgraLine(m_TheGraContext, m_LossGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y - 5, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);
      }
   }

//............................................................................
void CTrainEvolutionDashboard::WriteGeneralTrainInfo(
   MIL_INT     MinibatchSize,
   MIL_INT     TrainImageSizeX,
   MIL_INT     TrainImageSizeY,
   MIL_INT     TrainDatasetSize,
   MIL_INT     DevDatasetSize,
   MIL_DOUBLE  LearningRate,
   MIL_INT     TrainEngineUsed,
   MIL_STRING& TrainEngineDescription)
   {
   MgraControl(m_TheGraContext, M_BACKGROUND_MODE, M_OPAQUE);
   MgraControl(m_TheGraContext, M_BACKCOLOR, M_COLOR_BLACK);

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);

   const MIL_INT YMargin = 15;
   const MIL_INT TextHeight = 20;
   const MIL_INT TextMargin = MARGIN - 10;

   MIL_INT TextYPos = YMargin;

   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);

   MIL_TEXT_CHAR TheString[512];
   if(TrainEngineUsed == M_CPU)
      MosSprintf(TheString, 512, MIL_TEXT("Training is being performed on the CPU"));
   else
      MosSprintf(TheString, 512, MIL_TEXT("Training is being performed on the GPU"));
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Engine: %s"), TrainEngineDescription.c_str());
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Train image size: %dx%d"), TrainImageSizeX, TrainImageSizeY);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Train and Dev dataset size: %d and %d images"), TrainDatasetSize, DevDatasetSize);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Max number of epochs: %d"), m_MaxEpoch);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Minibatch size: %d"), MinibatchSize);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Learning rate: %.2e"), LearningRate);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   // The loss will be drawn under later on, so we retain is position.
   m_YPositionForLossText = TextYPos;
   }

//............................................................................
void CTrainEvolutionDashboard::AddEpochData(
   MIL_DOUBLE TrainErrorRate,
   MIL_DOUBLE DevErrorRate,
   MIL_INT    CurEpoch,
   bool       TheEpochIsTheBestUpToNow,
   MIL_DOUBLE EpochBenchMean)
   {
   m_EpochBenchMean = EpochBenchMean;
   UpdateEpochInfo(TrainErrorRate, DevErrorRate, CurEpoch, TheEpochIsTheBestUpToNow);
   UpdateEpochGraph(TrainErrorRate, DevErrorRate, CurEpoch);
   }

//............................................................................
void CTrainEvolutionDashboard::AddMiniBatchData(
   MIL_DOUBLE LossError,
   MIL_INT    MinibatchIdx,
   MIL_INT    EpochIdx,
   MIL_INT    NbBatchPerEpoch)
   {
   UpdateLoss(LossError);
   UpdateLossGraph(LossError, MinibatchIdx, EpochIdx, NbBatchPerEpoch);
   UpdateProgression(MinibatchIdx, EpochIdx, NbBatchPerEpoch);
   }

//............................................................................
void CTrainEvolutionDashboard::AddDatasetsPreparedData(MIL_INT TrainDatasetSize, MIL_INT DevDatasetSize)
   {
   UpdateDatasetsSize(TrainDatasetSize, DevDatasetSize);
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateEpochInfo(
   MIL_DOUBLE TrainErrorRate,
   MIL_DOUBLE DevErrorRate,
   MIL_INT    CurEpoch,
   bool       TheEpochIsTheBestUpToNow)
   {
   const MIL_INT YMargin = 15;
   const MIL_INT TextHeight = 20;
   const MIL_INT TextMargin = MARGIN - 10;

   MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
   MIL_TEXT_CHAR DevError[512];
   MosSprintf(DevError, 512, MIL_TEXT("Current Dev error rate: %7.4lf %%"), DevErrorRate);
   MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin, DevError);

   MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
   MIL_TEXT_CHAR TrainError[512];
   MosSprintf(TrainError, 512, MIL_TEXT("Current Train error rate: %7.4lf %%"), TrainErrorRate);
   MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + TextHeight, TrainError);

   if(TheEpochIsTheBestUpToNow)
      {
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MIL_TEXT_CHAR BestDevError[512];
      MosSprintf(BestDevError, 512, MIL_TEXT("Best epoch Dev error rate: %7.4lf %%   (Epoch %d)"), DevErrorRate, CurEpoch);
      MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + 2 * TextHeight, BestDevError);

      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MIL_TEXT_CHAR TrainErrorBest[512];
      MosSprintf(TrainErrorBest, 512, MIL_TEXT("Train error rate for the best epoch: %7.4lf %%"), TrainErrorRate);
      MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + 3 * TextHeight, TrainErrorBest);
      }
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateLoss(MIL_DOUBLE LossError)
   {
   const MIL_INT TextMargin = MARGIN - 10;

   MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
   MIL_TEXT_CHAR LossText[512];
   MosSprintf(LossText, 512, MIL_TEXT("Current loss value: %11.7lf"), LossError);

   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, m_YPositionForLossText, LossText);
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateEpochGraph(
   MIL_DOUBLE TrainErrorRate,
   MIL_DOUBLE DevErrorRate,
   MIL_INT    CurEpoch)
   {
   MIL_INT EpochIndex = CurEpoch + 1;
   MIL_INT CurTrainPosX = MARGIN + (MIL_INT)((MIL_DOUBLE)(EpochIndex) / (MIL_DOUBLE)(m_MaxEpoch)*(MIL_DOUBLE)GRAPH_SIZE_X);
   MIL_INT CurTrainPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - TrainErrorRate * 0.01));

   MIL_INT CurDevPosX = CurTrainPosX;
   MIL_INT CurDevPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - DevErrorRate * 0.01));

   if(CurEpoch == 0)
      {
      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MgraArcFill(m_TheGraContext, m_EpochGraphBufId, CurTrainPosX, CurTrainPosY, 2, 2, 0, 360);
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MgraArcFill(m_TheGraContext, m_EpochGraphBufId, CurDevPosX, CurDevPosY, 2, 2, 0, 360);
      }
   else
      {
      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MgraLine(m_TheGraContext, m_EpochGraphBufId, m_LastTrainPosX, m_LastTrainPosY, CurTrainPosX, CurTrainPosY);
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MgraLine(m_TheGraContext, m_EpochGraphBufId, m_LastDevPosX, m_LastDevPosY, CurDevPosX, CurDevPosY);
      }

   m_LastTrainPosX = CurTrainPosX;
   m_LastTrainPosY = CurTrainPosY;
   m_LastDevPosX = CurDevPosX;
   m_LastDevPosY = CurDevPosY;

   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   MIL_TEXT_CHAR EpochText[128];
   MosSprintf(EpochText, 128, MIL_TEXT("Epoch %d completed"), CurEpoch);
   MgraText(m_TheGraContext, m_EpochGraphBufId, MARGIN, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 25, EpochText);
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateLossGraph(
   MIL_DOUBLE LossError,
   MIL_INT    MiniBatchIdx,
   MIL_INT    EpochIdx,
   MIL_INT    NbBatchPerEpoch)
   {
   MIL_INT NBMiniBatch = m_MaxEpoch * NbBatchPerEpoch;
   MIL_INT CurMiniBatch = EpochIdx * NbBatchPerEpoch + MiniBatchIdx;

   MIL_DOUBLE XRatio = ((MIL_DOUBLE)CurMiniBatch / (MIL_DOUBLE)(NBMiniBatch));

   MIL_INT CurTrainMBPosX = MARGIN + (MIL_INT)(XRatio * (MIL_DOUBLE)GRAPH_SIZE_X);

   const MIL_DOUBLE MaxVal = std::pow(10.0, LOSS_EXPONENT_MAX);
   const MIL_INT    NbTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;

   // Saturate to the highest value of the graph.
   LossError = std::min<MIL_DOUBLE>(LossError, MaxVal);
   MIL_DOUBLE Log10RemapPos = std::max<MIL_DOUBLE>(log10(LossError) + (-LOSS_EXPONENT_MIN), 0.0);
   MIL_DOUBLE YRatio = Log10RemapPos / (MIL_DOUBLE)NbTick;

   MIL_INT CurTrainMBPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - YRatio));

   if(EpochIdx == 0 && MiniBatchIdx == 0)
      {
      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MgraDot(m_TheGraContext, m_LossGraphBufId, CurTrainMBPosX, CurTrainMBPosY);
      }
   else
      {
      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MgraLine(m_TheGraContext, m_LossGraphBufId, m_LastTrainMinibatchPosX, m_LastTrainMinibatchPosY, CurTrainMBPosX, CurTrainMBPosY);
      }

   m_LastTrainMinibatchPosX = CurTrainMBPosX;
   m_LastTrainMinibatchPosY = CurTrainMBPosY;

   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   // To clear the previous information.
   MgraText(m_TheGraContext, m_LossGraphBufId, MARGIN, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 25, MIL_TEXT("                                                    "));
   MIL_TEXT_CHAR EpochText[512];
   MosSprintf(EpochText, 512, MIL_TEXT("Epoch %d :: Minibatch %d"), EpochIdx, MiniBatchIdx);
   MgraText(m_TheGraContext, m_LossGraphBufId, MARGIN, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 25, EpochText);
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateProgression(
   MIL_INT MinibatchIdx,
   MIL_INT EpochIdx,
   MIL_INT NbBatchPerEpoch)
   {
   const MIL_INT YMargin = 20;
   const MIL_INT TextHeight = 30;

   const MIL_INT NbMinibatch = m_MaxEpoch * NbBatchPerEpoch;
   const MIL_INT NbMinibatchDone = EpochIdx * NbBatchPerEpoch + MinibatchIdx + 1;
   const MIL_INT NbMinibatchRemaining = NbMinibatch - NbMinibatchDone - 1;

   // Update estimated remaining time.
   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);

   // The first epoch implied data loading and cannot be used to estimate the
   // remaining time accurately.
   if(EpochIdx == 0)
      {
      MgraText(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin, MIL_TEXT("Estimated remaining time: N/A"));
      }
   else
      {
      MIL_DOUBLE MinibatchBenchMean = m_EpochBenchMean / (MIL_DOUBLE)NbBatchPerEpoch;
      MIL_DOUBLE RemainingTime = MinibatchBenchMean * (MIL_DOUBLE)NbMinibatchRemaining;
      MIL_TEXT_CHAR RemainingTimeText[512];
      MosSprintf(RemainingTimeText, 512, MIL_TEXT("Estimated remaining time: %8.0lf seconds"), RemainingTime);

      if(NbMinibatchDone == NbMinibatch)
         MgraText(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin, MIL_TEXT("Training completed!                         "));
      else
         MgraText(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin, RemainingTimeText);
      }

   // Update the progression bar.
   const MIL_INT ProgressionBarWidth = m_DashboardWidth - 2 * MARGIN;
   const MIL_INT ProgressionBarHeight = 30;
   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   MgraRectFill(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin + TextHeight, MARGIN + ProgressionBarWidth, YMargin + TextHeight + ProgressionBarHeight);

   MIL_DOUBLE PercentageComplete = (MIL_DOUBLE)(NbMinibatchDone) / (MIL_DOUBLE)(NbMinibatch);
   MIL_INT PercentageCompleteWidth = (MIL_INT)(PercentageComplete*ProgressionBarWidth);
   MgraColor(m_TheGraContext, COLOR_PROGRESS_BAR);
   MgraRectFill(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin + TextHeight, MARGIN + PercentageCompleteWidth, YMargin + TextHeight + ProgressionBarHeight);
   }

//............................................................................
void CTrainEvolutionDashboard::UpdateDatasetsSize(MIL_INT TrainDatasetSize, MIL_INT DevDatasetSize)
   {
   const MIL_INT DatasetSizeOffset = 5;
   const MIL_INT YMargin = 15;
   const MIL_INT TextHeight = 20;
   const MIL_INT TextMargin = MARGIN - 10;

   MIL_INT TextYPos = DatasetSizeOffset * YMargin;

   MIL_TEXT_CHAR TheString[512];
   MosSprintf(TheString, 512, MIL_TEXT("Train and Dev dataset size: %d and %d images"), TrainDatasetSize, DevDatasetSize);
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   }

//............................................................................
CPredictResultDisplay::CPredictResultDisplay(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID TestDataset)
   : m_MilSystem(MilSystem)
   , m_MilDisplay(MilDisplay)
   , m_MaxTrainImageSize(-1)
   , m_MilDispImage(M_NULL)
   , m_MilDispChild(M_NULL)
   , m_MilOverlay(M_NULL)
   , m_GraContext(M_NULL)
   , COLOR_PREDICT_INFO(M_COLOR_GREEN)
   , MARGIN(100)
   {
   MIL_INT NbClassDef = MclassInquire(TestDataset, M_DEFAULT, M_NUMBER_OF_CLASSES, M_NULL);
   std::vector<MIL_ID> ClassImages(NbClassDef);

   for(MIL_INT i = 0; i < NbClassDef; i++)
      {
      ClassImages[i] = MclassInquire(TestDataset, M_CLASS_INDEX(i), M_CLASS_ICON_ID + M_TYPE_MIL_ID, M_NULL);
      MIL_INT SizeX = MbufInquire(ClassImages[i], M_SIZE_X, M_NULL);
      MIL_INT SizeY = MbufInquire(ClassImages[i], M_SIZE_Y, M_NULL);
      m_MaxTrainImageSize = std::max<MIL_INT>(m_MaxTrainImageSize, SizeX);
      m_MaxTrainImageSize = std::max<MIL_INT>(m_MaxTrainImageSize, SizeY);
      }

   // Allocate a color buffer.
   m_MilDispImage = MbufAllocColor(m_MilSystem, 3, 2 * m_MaxTrainImageSize + MARGIN, NbClassDef*m_MaxTrainImageSize, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(m_MilDispImage, M_COLOR_BLACK);
   m_MilDispChild = MbufChild2d(m_MilDispImage, MARGIN / 2, m_MaxTrainImageSize, m_MaxTrainImageSize, m_MaxTrainImageSize, M_UNIQUE_ID);

   // Set annotation color.
   m_GraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);
   MgraColor(m_GraContext, M_COLOR_RED);

   MIL_INT PosY = 0;
   for(const auto& Image : ClassImages)
      {
      MbufCopyColor2d(Image, m_MilDispImage, M_ALL_BANDS, 0, 0, M_ALL_BANDS, m_MaxTrainImageSize + MARGIN, PosY, m_MaxTrainImageSize, m_MaxTrainImageSize);
      MgraRect(m_GraContext, m_MilDispImage, m_MaxTrainImageSize + MARGIN, PosY, m_MaxTrainImageSize + MARGIN + m_MaxTrainImageSize - 1, PosY + m_MaxTrainImageSize - 1);
      PosY += m_MaxTrainImageSize;
      }

   // Display the window with black color.
   MdispSelect(m_MilDisplay, m_MilDispImage);

   // Prepare for overlay annotations.
   MdispControl(m_MilDisplay, M_OVERLAY, M_ENABLE);
   m_MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   }

//............................................................................
CPredictResultDisplay::~CPredictResultDisplay()
   {
   m_GraContext = M_NULL;
   m_MilDispChild = M_NULL;
   m_MilDispImage = M_NULL;
   }

//............................................................................
void CPredictResultDisplay::Update(
   MIL_ID     ImageToPredict,
   MIL_INT    BestIndex,
   MIL_DOUBLE BestScore)
   {
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
   MdispControl(m_MilDisplay, M_OVERLAY_CLEAR, M_TRANSPARENT_COLOR);
   MbufCopy(ImageToPredict, m_MilDispChild);

   MIL_INT RectOffsetX = m_MaxTrainImageSize + 100;
   MIL_INT RectOffsetY = BestIndex * m_MaxTrainImageSize;

   MgraColor(m_GraContext, COLOR_PREDICT_INFO);
   MgraRect(m_GraContext, m_MilOverlay, RectOffsetX, RectOffsetY, RectOffsetX + m_MaxTrainImageSize - 1, RectOffsetY + m_MaxTrainImageSize - 1);
   MIL_TEXT_CHAR Accuracy_text[256];
   MosSprintf(Accuracy_text, 256, MIL_TEXT("%.2lf%%"), BestScore);
   MgraControl(m_GraContext, M_BACKGROUND_MODE, M_OPAQUE);
   MgraFont(m_GraContext, M_FONT_DEFAULT_SMALL);
   MgraText(m_GraContext, m_MilOverlay, RectOffsetX + 2, RectOffsetY + 2, Accuracy_text);

   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
   }

//............................................................................
CDatasetViewer::CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth)
   : m_MilSystem(MilSystem)
   , m_Dataset(Dataset)
   , Y_MARGIN(106)
   , TEXT_MARGIN(54)
   {
   PrintControls();

   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   MIL_INT ImageSizeX = 0;
   MIL_INT ImageSizeY = 0;
   GetSizes(m_MilSystem, m_Dataset, &ImageSizeX, &ImageSizeY);

   const MIL_INT IconSize = ImageSizeX;
   MIL_UNIQUE_BUF_ID DispImage = MbufAllocColor(MilSystem, 3, (2 * ImageSizeX) + IconSize, 3 * ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID DispChild = MbufChild2d(DispImage, ImageSizeX / 2, ImageSizeY, ImageSizeX + 1, ImageSizeY + 1, M_UNIQUE_ID);

   MdispSelect(MilDisplay, DispImage);
   MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   MIL_UNIQUE_BUF_ID OverlayChild = MbufChild2d(MilOverlay, 0, 0, 2 * ImageSizeX, 3 * ImageSizeY, M_UNIQUE_ID);

   MbufClear(DispImage, M_COLOR_BLACK);

   // Set annotation color.
   MgraColor(M_DEFAULT, M_COLOR_RED);

   // Set up the display.
   for(int iter = 0; iter < NUMBER_OF_CLASSES; iter++)
      {
      // Allocate a child buffer per product category.   
      MIL_UNIQUE_BUF_ID MilChildSample = MbufChild2d(DispImage, (2 * ImageSizeX), iter * IconSize, IconSize, IconSize, M_UNIQUE_ID);
      MIL_UNIQUE_BUF_ID MilOverlayChildSample = MbufChild2d(MilOverlay, ImageSizeX, iter * IconSize, IconSize, IconSize, M_UNIQUE_ID);
      MbufClear(MilChildSample, M_COLOR_BLACK);
      MbufClear(MilOverlayChildSample, M_COLOR_BLACK);

      // Load the sample image.
      MIL_ID ClassIconId = MclassInquire(m_Dataset, M_CLASS_INDEX(iter), M_CLASS_ICON_ID + M_TYPE_MIL_ID, M_NULL);

      // Retrieve the class description.
      MIL_STRING Text;
      MclassInquire(m_Dataset, M_CLASS_INDEX(iter), M_CLASS_NAME, Text);

      if(ClassIconId != M_NULL)
         {
         // Retrieve the color associated to the class.
         MIL_DOUBLE ClassColor;
         MclassInquire(m_Dataset, M_CLASS_INDEX(iter), M_CLASS_DRAW_COLOR, &ClassColor);

         // Draw the class name using the color associated to the class.
         MgraColor(M_DEFAULT, ClassColor);
         MgraText(M_DEFAULT, MilChildSample, 2, 2, Text);
         MgraText(M_DEFAULT, MilOverlayChildSample, 2, 2, Text);

         MIL_INT ClassImageExampleSizeX = MbufInquire(ClassIconId, M_SIZE_X, M_NULL);
         MIL_INT ClassImageExampleSizeY = MbufInquire(ClassIconId, M_SIZE_Y, M_NULL);

         if((ClassImageExampleSizeX >= IconSize) || (ClassImageExampleSizeY >= IconSize))
            {
            MimResize(ClassIconId, MilChildSample, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
            MimResize(ClassIconId, MilOverlayChildSample, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
            }
         else
            {
            MIL_INT OffsetX = (IconSize - ClassImageExampleSizeX) / 2;
            MIL_INT OffsetY = (IconSize - ClassImageExampleSizeY) / 2;
            MbufCopyColor2d(ClassIconId, MilChildSample, M_ALL_BANDS, 0, 0, M_ALL_BANDS, OffsetX, OffsetY, ClassImageExampleSizeX, ClassImageExampleSizeY);
            MbufCopyColor2d(ClassIconId, MilOverlayChildSample, M_ALL_BANDS, 0, 0, M_ALL_BANDS, OffsetX, OffsetY, ClassImageExampleSizeX, ClassImageExampleSizeY);
            }
         }

      // Draw an initial red rectangle around the buffer.
      MgraRect(M_DEFAULT, MilChildSample, 0, 1, IconSize - 1, IconSize - 2);
      MgraRect(M_DEFAULT, MilOverlayChildSample, 0, 1, IconSize - 1, IconSize - 2);
      }

   MIL_UNIQUE_GRA_ID GraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);

   MIL_INT NbEntries = 0;
   MclassInquire(m_Dataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);

   MIL_TEXT_CHAR IndexText[512];
   MIL_INT EntryIndex = 0;
   bool Exit = false;
   while(!Exit)
      {
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      MIL_STRING EntryImagePath;
      MclassInquireEntry(m_Dataset, EntryIndex, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH_ABS, EntryImagePath);
      MbufLoad(EntryImagePath, DispChild);

      MdispControl(MilDisplay, M_OVERLAY_OPACITY, 0.0);

      MIL_INT TextYPos = Y_MARGIN;

      MgraColor(GraContext, M_COLOR_WHITE);
      MosSprintf(IndexText, 512, MIL_TEXT("Entry Index %d / %d"), EntryIndex, NbEntries - 1);
      MgraText(GraContext, DispImage, TEXT_MARGIN, TextYPos, IndexText);
      MgraText(GraContext, OverlayChild, TEXT_MARGIN, TextYPos, IndexText);

      std::vector<MIL_INT> GTIdx;
      MclassInquireEntry(m_Dataset, EntryIndex, M_DEFAULT_KEY, M_REGION_INDEX(0), M_CLASS_INDEX_GROUND_TRUTH, GTIdx);
      MIL_DOUBLE ClassColor;
      MclassInquire(m_Dataset, M_CLASS_INDEX(GTIdx[0]), M_CLASS_DRAW_COLOR, &ClassColor);
      MgraColor(GraContext, ClassColor);
      MgraControl(GraContext, M_LINE_THICKNESS, 3);
      MgraRect(GraContext, DispChild, 0, 0, ImageSizeX, ImageSizeY);

      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

      // Look for user key input.
      if(MosKbhit() != 0)
         {
         char KeyVal = (char)MosGetch();

         switch(KeyVal)
            {
            case 'N':
            case 'n':
               EntryIndex = EntryIndex == (NbEntries - 1) ? (NbEntries - 1) : ++EntryIndex;
               break;
            case 'P':
            case 'p':
               EntryIndex = EntryIndex == 0 ? 0 : --EntryIndex;
               break;
            case 'E':
            case 'e':
               Exit = true;
               break;
            default:
               break;
            }
         }
      }
   }

//............................................................................
void CDatasetViewer::PrintControls()
   {
   MosPrintf(MIL_TEXT("Here are the dataset viewer controls:\n"));
   MosPrintf(MIL_TEXT("n: Display next image\n"));
   MosPrintf(MIL_TEXT("p: Display previous image\n"));
   MosPrintf(MIL_TEXT("e: exit\n\n"));

   MosPrintf(MIL_TEXT("Select a dataset viewer control:\n"));
   }
