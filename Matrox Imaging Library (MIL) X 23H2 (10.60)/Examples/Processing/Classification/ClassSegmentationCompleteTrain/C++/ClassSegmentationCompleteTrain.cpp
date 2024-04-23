//*************************************************************************************
//
// File name: ClassSegmentationCompleteTrain.cpp
//
// Synopsis:  This program uses the classification module to train
//            a context able to segment steel defects.
//
// Note:      GPU training can be enabled via a MIL update for 64-bit.
//            This can dramatically increase the training speed.
//
// This example and data was inspired from the work of:
// Kechen Song and Yunhui Yan, "Micro surface defect detection method 
// for silicon steel strip based on saliency convex active contour model".
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <algorithm>
#include <windows.h>


// ===========================================================================
// Example description.
// ===========================================================================
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("ClassSegmentationCompleteTrain\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example trains a segmentation model to segment defects in steel.\n")
             MIL_TEXT("The first step of the example converts an existing dataset to Matrox format.\n")
             MIL_TEXT("The second step trains a context and displays the train evolution.\n")
             MIL_TEXT("The final step performs predictions on a test data using the trained\n")
             MIL_TEXT("coarse segmentation model as a final check of the expected model performance.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, classification.\n\n"));
   }

#define EXAMPLE_IMAGE_ROOT_PATH              M_IMAGE_PATH MIL_TEXT("Classification/SurfaceSteel/")
#define EXAMPLE_PRETRAINED_PATH              EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("SurfaceSteelSegNet.mclass")
#define EXAMPLE_DATASET_PATH                 EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Dataset/")
#define EXAMPLE_DATASET_IMAGES_PATH          EXAMPLE_DATASET_PATH MIL_TEXT("Images/")
#define EXAMPLE_DATASET_LABELS_PATH          EXAMPLE_DATASET_PATH MIL_TEXT("Labels/")
#define EXAMPLE_DATASET_IMAGE_SEARCH_PATTERN EXAMPLE_DATASET_IMAGES_PATH MIL_TEXT("*.bmp")
#define EXAMPLE_DATASET_LABEL_SEARCH_PATTERN EXAMPLE_DATASET_LABELS_PATH MIL_TEXT("*.mim")
#define EXAMPLE_REGION_MASKS_PATH            MIL_TEXT("Masks/")
#define EXAMPLE_TRAIN_DESTINATION_PATH       MIL_TEXT("Train/")
#define EXAMPLE_SEGMENTATION_FOLDER_PATH     MIL_TEXT("Segmentations/")

static const MIL_INT NUMBER_OF_CLASSES = 3;

MIL_STRING SURFACE_STEEL_CLASS_NAMES[NUMBER_OF_CLASSES] = { MIL_TEXT("NoDefect"),
                                                            MIL_TEXT("Spot"),
                                                            MIL_TEXT("Pit") };

// Icon image for each class.
MIL_STRING SURFACE_STEEL_CLASS_ICONS[NUMBER_OF_CLASSES] = { EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("NoDefect.png"),
                                                            EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Spot.png"),
                                                            EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Pit.png") };

//............................................................................
class CTrainEvolutionDashboard
   {
   public:
      CTrainEvolutionDashboard(MIL_ID MilSystem, MIL_ID TrainCtx,
                               MIL_INT TrainImageSizeX, MIL_INT TrainImageSizeY,
                               MIL_INT TrainEngineUsed, MIL_STRING& TrainEngineDescription);
      ~CTrainEvolutionDashboard();
      void AddEpochData(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_DOUBLE DevLoss,
                        MIL_INT CurEpoch, bool TheEpochIsTheBestUpToNow,
                        MIL_DOUBLE EpochBenchMean);

      void AddMiniBatchData(MIL_DOUBLE Loss, MIL_INT MinibatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch);

      MIL_ID GetDashboardBufId()
         {
         return m_DashboardBufId;
         }

   protected:
      void UpdateEpochInfo(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_INT CurEpoch, bool TheEpochIsTheBestUpToNow);
      void UpdateTrainLoss(MIL_DOUBLE Loss);
      void UpdateDevLoss(MIL_DOUBLE Loss);
      void UpdateEpochGraph(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_DOUBLE DevLoss, MIL_INT CurEpoch);
      void UpdateTrainLossGraph(MIL_DOUBLE Loss, MIL_INT MiniBatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch);
      void UpdateDevLossGraph(MIL_DOUBLE DevLoss, MIL_INT EpochIdx);
      void UpdateProgression(MIL_INT MinibatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch);

      void DrawSectionSeparators();
      void DrawBufferFrame(MIL_ID BufId, MIL_INT FrameThickness);
      void InitializeEpochGraph();
      void InitializeLossGraph();
      void WriteGeneralTrainInfo(MIL_INT     MinibatchSize,
                                 MIL_INT     TrainImageSizeX,
                                 MIL_INT     TrainImageSizeY,
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
      MIL_INT m_LastDevEpochLossPosX;
      MIL_INT m_LastDevEpochLossPosY;

      MIL_INT m_YPositionForTrainLossText;
      MIL_INT m_YPositionForDevLossText;

      MIL_DOUBLE m_EpochBenchMean;

      //Constants useful for the graph.
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
class CDatasetViewer
   {
   public:
      CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth);

   private:
      void PrintControls();

   private:
      MIL_ID  m_MilSystem;
      MIL_ID  m_Dataset;

      MIL_DOUBLE m_Opacity;
      bool m_DisplayGroundTruth;
      bool m_DisplayContour;

      const MIL_INT Y_MARGIN;
      const MIL_INT TEXT_HEIGHT;
      const MIL_INT TEXT_MARGIN;
      const MIL_DOUBLE OPACITY_INCREMENT;
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
struct HookDatasetsPrepared
   {
   bool m_SkipTrain;
   MIL_ID m_DashboardId;
   MIL_ID m_MilDisplay;
   };

MIL_INT MFTYPE HookFuncEpoch(MIL_INT HookType, MIL_ID EventId, void* UserData);
MIL_INT MFTYPE HookFuncMiniBatch(MIL_INT HookType, MIL_ID EventId, void* UserData);
MIL_INT MFTYPE HookFuncDatasetsPrepared(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData);
void CreateDirectory(MIL_STRING DirectoryPath);
MIL_STRING GetExampleCurrentDirectory();

void GetSizes(MIL_ID MilSysId, MIL_ID Dataset, MIL_INT* SizeX, MIL_INT* SizeY);
MIL_INT IsTrainingSupportedOnPlatform(MIL_ID MilSystem);
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem);
MIL_UNIQUE_CLASS_ID TrainTheModel(MIL_ID MilSystem, MIL_ID Dataset, MIL_ID DevDataset, MIL_ID MilDisplay);
void PredictUsingTrainedContext(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID TrainedCtx, MIL_ID TestDataset);
bool LoadAndConvertDatasets(MIL_ID Dataset, MIL_ID MilSystem);
MIL_STRING ConvertPrepareDataStatusToStr(MIL_INT Status);

// ****************************************************************************
//    Main.
// ****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   if(IsTrainingSupportedOnPlatform(MilSystem) != M_TRUE)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      return -1;
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("CONVERTING THE DATASET...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   // The dataset will automatically be split into the train and development datasets during the call to MclassTrain.
   MIL_UNIQUE_CLASS_ID Dataset, DevDataset;
   Dataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   DevDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);

   bool DatasetLoaded = LoadAndConvertDatasets(Dataset, MilSystem);

   if(!DatasetLoaded)
      {
      MosPrintf(MIL_TEXT("\nDataset not loaded properly !!!!!!!!!!!!!!\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end...\n"));
      MosGetch();
      return 0;
      }

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("TRAINING... THIS WILL TAKE SOME TIME...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   MIL_UNIQUE_CLASS_ID TrainedCtx = TrainTheModel(MilSystem, Dataset, DevDataset, MilDisplay);

   if(TrainedCtx)
      {
      MosPrintf(MIL_TEXT("\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("PREDICTING USING THE TRAINED CONTEXT...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n"));

      PredictUsingTrainedContext(MilSystem, MilDisplay, TrainedCtx, DevDataset);
      }
   else
      {
      MosPrintf(MIL_TEXT("\nTraining has not completed properly !!!!!!!!!!!!!!\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end...\n"));
      MosGetch();
      }

   return 0;
   }

//==============================================================================
void PredictUsingTrainedContext(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID TrainedCtx, MIL_ID TestDataset)
   {
   MclassPreprocess(TrainedCtx, M_DEFAULT);

   // Create a Directory for the segmentation results of TestDataset.
   CreateDirectory(EXAMPLE_SEGMENTATION_FOLDER_PATH);
   MclassControl(TestDataset, M_DEFAULT, M_SEGMENTATION_FOLDER, GetExampleCurrentDirectory() + EXAMPLE_SEGMENTATION_FOLDER_PATH);

   // Create a predict context from the train result and classify with it.
   MclassPredict(TrainedCtx, TestDataset, TestDataset, M_DEFAULT);

   MIL_INT NbEntries = 0;
   MclassInquire(TestDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);

   MosPrintf(MIL_TEXT("\nPredictions will be performed on the test dataset as a final check\nof the trained segmentation model.\n"));
   MosPrintf(MIL_TEXT("The test dataset contains %d images.\n"), NbEntries);
   MosPrintf(MIL_TEXT("The prediction results will be shown for the all %d images.\n"), NbEntries);

   MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("VIEWING THE PREDICTED TEST DATASET...\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n\n"));
   CDatasetViewer DatasetViewer(MilSystem, TestDataset, false);

   const MIL_STRING SaveCtxName = MIL_TEXT("SurfaceSteelSegNet.mclass");
   MclassSave(SaveCtxName, TrainedCtx, M_DEFAULT);
   MosPrintf(MIL_TEXT("\nThe trained context was saved: \"%s\".\n"), SaveCtxName.c_str());

   MosPrintf(MIL_TEXT("Press <Enter> to continue...\n"));
   MosGetch();
   }

//==============================================================================
MIL_INT MFTYPE HookFuncEpoch(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   auto HookData = (HookEpochData *)UserData;

   MIL_DOUBLE CurBench = 0.0;
   MIL_DOUBLE CurBenchMean = -1.0;

   MIL_INT CurEpochIndex = 0;
   MclassGetHookInfo(EventId, M_EPOCH_INDEX + M_TYPE_MIL_INT, &CurEpochIndex);

   MappTimer(M_DEFAULT, M_TIMER_READ, &CurBench);
   MIL_DOUBLE EpochBenchMean = CurBench / (CurEpochIndex + 1);

   MIL_DOUBLE TrainIOUMean = 0;
   MclassGetHookInfo(EventId, M_TRAIN_DATASET_IOU_MEAN, &TrainIOUMean);
   MIL_DOUBLE DevIOUMean = 0;
   MclassGetHookInfo(EventId, M_DEV_DATASET_IOU_MEAN, &DevIOUMean);
   MIL_DOUBLE DevLoss = 0;
   MclassGetHookInfo(EventId, M_DEV_DATASET_LOSS, &DevLoss);

   MIL_INT AreTrainedCNNParametersUpdated = M_FALSE;
   MclassGetHookInfo(EventId,
                     M_TRAINED_PARAMETERS_UPDATED + M_TYPE_MIL_INT,
                     &AreTrainedCNNParametersUpdated);
   // By default trained parameters are updated when the dev loss
   // is the best up to now.
   bool TheEpochIsTheBestUpToNow = (AreTrainedCNNParametersUpdated == M_TRUE);

   HookData->TheDashboard->AddEpochData(
      TrainIOUMean, DevIOUMean, DevLoss,
      CurEpochIndex, TheEpochIsTheBestUpToNow, EpochBenchMean);

   return M_NULL;
   }

//==============================================================================
MIL_INT MFTYPE HookFuncDatasetsPrepared(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   const auto* HookData = (HookDatasetsPrepared *)UserData;

   MIL_ID TrainResult = M_NULL;
   MclassGetHookInfo(EventId, M_RESULT_ID + M_TYPE_MIL_ID, &TrainResult);

   MIL_ID MilSystem = M_NULL;
   MclassInquire(TrainResult, M_DEFAULT, M_OWNER_SYSTEM + M_TYPE_MIL_ID, &MilSystem);

   MIL_UNIQUE_CLASS_ID PreparedTrainDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MclassCopyResult(TrainResult, M_DEFAULT, PreparedTrainDataset, M_DEFAULT, M_PREPARED_TRAIN_DATASET, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <v> to view the augmented train dataset.\nPress <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if(KeyVal == 'v' || KeyVal == 'V')
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE AUGMENTED TRAIN DATASET...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(MilSystem, PreparedTrainDataset, true);
      }

   // Stop the training if we want to skip it.
   if(HookData->m_SkipTrain)
      {
      MclassControl(TrainResult, M_DEFAULT, M_STOP_TRAIN, M_DEFAULT);
      return M_NULL;
      }

   MosPrintf(MIL_TEXT("\nThe training has started.\n"));
   MosPrintf(MIL_TEXT("It can be paused at any time by pressing 'p'.\n"));
   MosPrintf(MIL_TEXT("It can then be skipped or resumed.\n"));

   MosPrintf(MIL_TEXT("\nDuring training, you can observe the displayed mean IOU of the train\n"));
   MosPrintf(MIL_TEXT("and dev datasets together with the evolution of the losses.\n"));
   MosPrintf(MIL_TEXT("The best epoch is determined by the epoch with the smallest dev loss.\n"));

   MdispSelect(HookData->m_MilDisplay, HookData->m_DashboardId);

   return M_NULL;
   }

//==============================================================================
MIL_INT MFTYPE HookFuncMiniBatch(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   auto HookData = (HookMiniBatchData *)UserData;

   MIL_DOUBLE Loss = 0;
   MclassGetHookInfo(EventId, M_MINI_BATCH_LOSS, &Loss);

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

   HookData->TheDashboard->AddMiniBatchData(Loss, MiniBatchIdx, EpochIdx, NbMiniBatchPerEpoch);

   if(MosKbhit() != 0)
      {
      char KeyVal = (char)MosGetch();
      if(KeyVal == 'p' || KeyVal == 'P')
         {
         MosPrintf(MIL_TEXT("\nPress 's' to stop the training or any other key to continue.\n"));
         while(MosKbhit() == 0)
            {
            KeyVal = (char)MosGetch();
            if(KeyVal == 's' || KeyVal == 'S')
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

   return(M_NULL);
   }

//==============================================================================
void CreateDirectory(MIL_STRING DirectoryPath)
   {
   // If the directory exists from a previous run of the example, remove it to start again.
   MIL_INT DirectoryExists = M_NO;
   MappFileOperation(M_DEFAULT, DirectoryPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &DirectoryExists);
   if(DirectoryExists == M_YES)
      {
      MappFileOperation(M_DEFAULT, DirectoryPath, M_NULL, M_NULL, M_FILE_DELETE_DIR, M_RECURSIVE, M_NULL);
      }
   MappFileOperation(M_DEFAULT, DirectoryPath, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);
   }

//==============================================================================
MIL_STRING GetExampleCurrentDirectory()
   {
   DWORD CurDirStrSize = GetCurrentDirectory(0, NULL) + 1;

   std::vector<MIL_TEXT_CHAR> vCurDir(CurDirStrSize, 0);
   GetCurrentDirectory(CurDirStrSize, (LPTSTR)&vCurDir[0]);

   MIL_STRING sRet = &vCurDir[0];
   return sRet + MIL_TEXT("/");
   }

//==============================================================================
bool LoadAndConvertDatasets(MIL_ID Dataset, MIL_ID MilSystem)
   {
   // This is a temporary dataset that we will then split into the train and test datasets.
   MIL_UNIQUE_CLASS_ID DatasetToExport = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);

   // If the masks directory exists from a previous run of the example, remove it to start again.
   CreateDirectory(EXAMPLE_REGION_MASKS_PATH);

   MclassControl(DatasetToExport, M_DEFAULT, M_REGION_MASKS_FOLDER, GetExampleCurrentDirectory() + EXAMPLE_REGION_MASKS_PATH);

   MIL_INT NumberOfImages;
   MappFileOperation(M_DEFAULT, EXAMPLE_DATASET_IMAGE_SEARCH_PATTERN, M_NULL, M_NULL, M_FILE_NAME_FIND_COUNT, M_DEFAULT, &NumberOfImages);

   MIL_INT NumberOfLabels;
   MappFileOperation(M_DEFAULT, EXAMPLE_DATASET_LABEL_SEARCH_PATTERN, M_NULL, M_NULL, M_FILE_NAME_FIND_COUNT, M_DEFAULT, &NumberOfLabels);

   // We expect the number of images to equal the number of labels.
   if(NumberOfImages != NumberOfLabels)
      {
      return false;
      }

   for(MIL_INT ImageIdx = 0; ImageIdx < NumberOfImages; ImageIdx++)
      {
      MIL_STRING ImageName;
      MappFileOperation(M_DEFAULT, EXAMPLE_DATASET_IMAGE_SEARCH_PATTERN, M_NULL, M_NULL, M_FILE_NAME_FIND, ImageIdx, ImageName);
      MIL_STRING ImageLabel;
      MappFileOperation(M_DEFAULT, EXAMPLE_DATASET_LABEL_SEARCH_PATTERN, M_NULL, M_NULL, M_FILE_NAME_FIND, ImageIdx, ImageLabel);

      MclassControl(DatasetToExport, M_DEFAULT, M_ENTRY_ADD, M_DEFAULT);
      MclassControlEntry(DatasetToExport, ImageIdx, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH, M_DEFAULT, EXAMPLE_DATASET_IMAGES_PATH + ImageName, M_DEFAULT);

      MIL_UNIQUE_BUF_ID RestoredBuffer = MbufRestore(EXAMPLE_DATASET_LABELS_PATH + ImageLabel, MilSystem, M_UNIQUE_ID);

      // When passing a ground truth image to MclassEntryAddRegion it will automatically convert the image to a collection of binary masks.
      // These binary masks are stored to the location specified by M_REGION_MASKS_FOLDER.
      MclassEntryAddRegion(DatasetToExport, ImageIdx, M_DEFAULT_KEY, M_GROUND_TRUTH_IMAGE, RestoredBuffer, M_NULL, M_DEFAULT, M_DEFAULT);
      }

   for(MIL_INT i = 0; i < NUMBER_OF_CLASSES; ++i)
      {
      MIL_UNIQUE_BUF_ID ClassIcon = MbufRestore(SURFACE_STEEL_CLASS_ICONS[i], MilSystem, M_UNIQUE_ID);
      MclassControl(DatasetToExport, M_CLASS_INDEX(i), M_CLASS_ICON_ID, ClassIcon);
      MclassControl(DatasetToExport, M_CLASS_INDEX(i), M_CLASS_NAME, SURFACE_STEEL_CLASS_NAMES[i]);
      }

   // This export/import is done solely for demonstration purposes to show the functionality to users. 
   MIL_STRING DatasetInMatroxFormatFolder = MIL_TEXT("DatasetInMatroxFormat");
   CreateDirectory(DatasetInMatroxFormatFolder);
   MclassExport(DatasetInMatroxFormatFolder, M_IMAGE_DATASET_FOLDER, DatasetToExport, M_DEFAULT, M_COMPLETE, M_DEFAULT);
   MclassImport(DatasetInMatroxFormatFolder, M_IMAGE_DATASET_FOLDER, Dataset, M_DEFAULT, M_COMPLETE, M_DEFAULT);

   MclassControl(Dataset, M_DEFAULT, M_MAKE_FILE_PATHS_RELATIVE, M_DEFAULT);

   MIL_STRING RootPath = MIL_TEXT("");
   MclassInquire(Dataset, M_DEFAULT, M_ROOT_PATH, RootPath);

   MosPrintf(MIL_TEXT("The dataset has been converted to Matrox format.\n"));
   MosPrintf(MIL_TEXT("The dataset has been exported in Matrox format\nand can be found here: %s \n\n"), RootPath.c_str());

   MosPrintf(MIL_TEXT("Press <v> to view the converted dataset.\nPress <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if(KeyVal == 'v' || KeyVal == 'V')
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE CONVERTED DATASET...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(MilSystem, Dataset, true);
      }

   return true;
   }

//==============================================================================
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

//==============================================================================
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem)
   {
   MIL_INT IsInstalled = M_FALSE;

   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_SEG, M_DEFAULT, M_UNIQUE_ID);
   MclassInquire(TrainCtx, M_DEFAULT, M_TRAIN_ENGINE_IS_INSTALLED + M_TYPE_MIL_INT, &IsInstalled);

   return IsInstalled;
   }

//==============================================================================
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

//==============================================================================
CTrainEvolutionDashboard::CTrainEvolutionDashboard(MIL_ID MilSystem, MIL_ID TrainCtx,
                                                   MIL_INT TrainImageSizeX, MIL_INT TrainImageSizeY,
                                                   MIL_INT TrainEngineUsed, MIL_STRING& TrainEngineDescription):
   m_DashboardBufId(M_NULL),
   m_TheGraContext(M_NULL),
   m_EpochInfoBufId(M_NULL),
   m_EpochGraphBufId(M_NULL),
   m_LossInfoBufId(M_NULL),
   m_LossGraphBufId(M_NULL),
   m_ProgressionInfoBufId(M_NULL),
   m_DashboardWidth(0),
   m_LastTrainPosX(0),
   m_LastTrainPosY(0),
   m_LastDevPosX(0),
   m_LastDevPosY(0),
   m_LastTrainMinibatchPosX(0),
   m_LastTrainMinibatchPosY(0),
   m_YPositionForTrainLossText(0),
   m_YPositionForDevLossText(0),
   m_EpochBenchMean(-1.0),
   GRAPH_SIZE_X(400),
   GRAPH_SIZE_Y(400),
   GRAPH_TOP_MARGIN(30),
   MARGIN(50),
   EPOCH_AND_MINIBATCH_REGION_HEIGHT(190),
   PROGRESSION_INFO_REGION_HEIGHT(100),
   LOSS_EXPONENT_MAX(0),
   LOSS_EXPONENT_MIN(-5),
   COLOR_GENERAL_INFO(M_RGB888(0, 176, 255)),
   COLOR_DEV_SET_INFO(M_COLOR_MAGENTA),
   COLOR_TRAIN_SET_INFO(M_COLOR_GREEN),
   COLOR_PROGRESS_BAR(M_COLOR_DARK_GREEN)
   {
   //Get values from the training context.
   MclassInquire(TrainCtx, M_DEFAULT, M_MAX_EPOCH + M_TYPE_MIL_INT, &m_MaxEpoch);
   MIL_DOUBLE LearningRate = 0.0;
   MclassInquire(TrainCtx, M_DEFAULT, M_INITIAL_LEARNING_RATE, &LearningRate);
   MIL_INT MinibatchSize = 0;
   MclassInquire(TrainCtx, M_DEFAULT, M_MINI_BATCH_SIZE + M_TYPE_MIL_INT, &MinibatchSize);

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

   WriteGeneralTrainInfo(MinibatchSize, TrainImageSizeX, TrainImageSizeY, LearningRate, TrainEngineUsed, TrainEngineDescription);
   }

//==============================================================================
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

//==============================================================================
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


//==============================================================================
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

//==============================================================================
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

//==============================================================================
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

//==============================================================================
void CTrainEvolutionDashboard::WriteGeneralTrainInfo(MIL_INT     MinibatchSize,
                                                     MIL_INT     TrainImageSizeX,
                                                     MIL_INT     TrainImageSizeY,
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

   MosSprintf(TheString, 512, MIL_TEXT("Training engine: %s"), TrainEngineDescription.c_str());
   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Train image size: %dx%d"), TrainImageSizeX, TrainImageSizeY);
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
   m_YPositionForTrainLossText = TextYPos;
   TextYPos += TextHeight;
   m_YPositionForDevLossText = TextYPos;
   }

//==============================================================================
void CTrainEvolutionDashboard::AddEpochData(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_DOUBLE DevLoss,
                                            MIL_INT CurEpoch, bool TheEpochIsTheBestUpToNow,
                                            MIL_DOUBLE EpochBenchMean)
   {
   m_EpochBenchMean = EpochBenchMean;
   UpdateDevLoss(DevLoss);
   UpdateEpochInfo(TrainIOUMean, DevIOUMean, CurEpoch, TheEpochIsTheBestUpToNow);
   UpdateEpochGraph(TrainIOUMean, DevIOUMean, DevLoss, CurEpoch);
   UpdateDevLossGraph(DevLoss, CurEpoch);
   }

//==============================================================================
void CTrainEvolutionDashboard::AddMiniBatchData(MIL_DOUBLE TrainLoss, MIL_INT MinibatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch)
   {
   UpdateTrainLoss(TrainLoss);
   UpdateTrainLossGraph(TrainLoss, MinibatchIdx, EpochIdx, NbBatchPerEpoch);
   UpdateProgression(MinibatchIdx, EpochIdx, NbBatchPerEpoch);
   }

//==============================================================================
void CTrainEvolutionDashboard::UpdateEpochInfo(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_INT CurEpoch, bool TheEpochIsTheBestUpToNow)
   {
   const MIL_INT YMargin = 15;
   const MIL_INT TextHeight = 20;
   const MIL_INT TextMargin = MARGIN - 10;

   MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
   MIL_TEXT_CHAR DevIOUMeanText[512];
   MosSprintf(DevIOUMeanText, 512, MIL_TEXT("Current Dev IOU Mean: %7.4lf %%"), DevIOUMean);
   MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin, DevIOUMeanText);

   MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
   MIL_TEXT_CHAR TrainIOUMeanText[512];
   MosSprintf(TrainIOUMeanText, 512, MIL_TEXT("Current Train IOU Mean: %7.4lf %%"), TrainIOUMean);
   MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + TextHeight, TrainIOUMeanText);

   if(TheEpochIsTheBestUpToNow)
      {
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MIL_TEXT_CHAR BestDevIOUMeanText[512];
      MosSprintf(BestDevIOUMeanText, 512, MIL_TEXT("Dev IOU Mean for the best epoch: %7.4lf %%   (Epoch %d)"), DevIOUMean, CurEpoch);
      MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + 2 * TextHeight, BestDevIOUMeanText);

      MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
      MIL_TEXT_CHAR BestTrainIOUMeanText[512];
      MosSprintf(BestTrainIOUMeanText, 512, MIL_TEXT("Train IOU Mean for the best epoch: %7.4lf %%"), TrainIOUMean);
      MgraText(m_TheGraContext, m_EpochInfoBufId, TextMargin, YMargin + 3 * TextHeight, BestTrainIOUMeanText);
      }
   }

//==============================================================================
void CTrainEvolutionDashboard::UpdateTrainLoss(MIL_DOUBLE Loss)
   {
   const MIL_INT TextMargin = MARGIN - 10;

   MgraColor(m_TheGraContext, COLOR_TRAIN_SET_INFO);
   MIL_TEXT_CHAR LossText[512];
   MosSprintf(LossText, 512, MIL_TEXT("Current train loss value: %11.7lf"), Loss);

   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, m_YPositionForTrainLossText, LossText);
   }

//==============================================================================
void CTrainEvolutionDashboard::UpdateDevLoss(MIL_DOUBLE Loss)
   {
   const MIL_INT TextMargin = MARGIN - 10;

   MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
   MIL_TEXT_CHAR LossText[512];
   MosSprintf(LossText, 512, MIL_TEXT("Current dev loss value: %11.7lf"), Loss);

   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, m_YPositionForDevLossText, LossText);
   }

//==============================================================================
void CTrainEvolutionDashboard::UpdateEpochGraph(MIL_DOUBLE TrainIOUMean, MIL_DOUBLE DevIOUMean, MIL_DOUBLE DevLoss, MIL_INT CurEpoch)
   {
   MIL_INT EpochIndex = CurEpoch + 1;
   MIL_INT CurTrainPosX = MARGIN + (MIL_INT)((MIL_DOUBLE)(EpochIndex) / (MIL_DOUBLE)(m_MaxEpoch)*(MIL_DOUBLE)GRAPH_SIZE_X);
   MIL_INT CurTrainPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - TrainIOUMean * 0.01));

   MIL_INT CurDevPosX = CurTrainPosX;
   MIL_INT CurDevPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - DevIOUMean * 0.01));

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

//==============================================================================
void CTrainEvolutionDashboard::UpdateTrainLossGraph(MIL_DOUBLE Loss, MIL_INT MiniBatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch)
   {
   MIL_INT NBMiniBatch = m_MaxEpoch * NbBatchPerEpoch;
   MIL_INT CurMiniBatch = EpochIdx * NbBatchPerEpoch + MiniBatchIdx;

   MIL_DOUBLE XRatio = ((MIL_DOUBLE)CurMiniBatch / (MIL_DOUBLE)(NBMiniBatch));

   MIL_INT CurTrainMBPosX = MARGIN + (MIL_INT)(XRatio * (MIL_DOUBLE)GRAPH_SIZE_X);

   const MIL_DOUBLE MaxVal = std::pow(10.0, LOSS_EXPONENT_MAX);
   const MIL_INT    NbTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;

   // Saturate to the highest value of the graph.
   Loss = std::min<MIL_DOUBLE>(Loss, MaxVal);
   MIL_DOUBLE Log10RemapPos = std::max<MIL_DOUBLE>(log10(Loss) + (-LOSS_EXPONENT_MIN), 0.0);
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

//==============================================================================
void CTrainEvolutionDashboard::UpdateDevLossGraph(MIL_DOUBLE DevLoss, MIL_INT EpochIdx)
   {
   MIL_DOUBLE XRatio = ((MIL_DOUBLE)(EpochIdx + 1)/ (MIL_DOUBLE)(m_MaxEpoch));

   MIL_INT CurTrainMBPosX = MARGIN + (MIL_INT)(XRatio * (MIL_DOUBLE)GRAPH_SIZE_X);

   const MIL_DOUBLE MaxVal = std::pow(10.0, LOSS_EXPONENT_MAX);
   const MIL_INT    NbTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;

   // Saturate to the highest value of the graph.
   DevLoss = std::min<MIL_DOUBLE>(DevLoss, MaxVal);
   MIL_DOUBLE Log10RemapPos = std::max<MIL_DOUBLE>(log10(DevLoss) + (-LOSS_EXPONENT_MIN), 0.0);
   MIL_DOUBLE YRatio = Log10RemapPos / (MIL_DOUBLE)NbTick;

   MIL_INT CurTrainMBPosY = GRAPH_TOP_MARGIN + (MIL_INT)((MIL_DOUBLE)GRAPH_SIZE_Y*(1.0 - YRatio));

   if(EpochIdx == 0)
      {
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MgraDot(m_TheGraContext, m_LossGraphBufId, CurTrainMBPosX, CurTrainMBPosY);
      }
   else
      {
      MgraColor(m_TheGraContext, COLOR_DEV_SET_INFO);
      MgraLine(m_TheGraContext, m_LossGraphBufId, m_LastDevEpochLossPosX, m_LastDevEpochLossPosY, CurTrainMBPosX, CurTrainMBPosY);
      }

   m_LastDevEpochLossPosX = CurTrainMBPosX;
   m_LastDevEpochLossPosY = CurTrainMBPosY;
   }

//==============================================================================
void CTrainEvolutionDashboard::UpdateProgression(MIL_INT MinibatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch)
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

//==============================================================================
void GetSizes(MIL_ID MilSysId, MIL_ID Dataset, MIL_INT* SizeX, MIL_INT* SizeY)
   {
   MIL_STRING EntryImagePathAbs;
   MclassInquireEntry(Dataset, 0, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH_ABS, EntryImagePathAbs);
   MbufDiskInquire(EntryImagePathAbs, M_SIZE_X, SizeX);
   MbufDiskInquire(EntryImagePathAbs, M_SIZE_Y, SizeY);
   }

//==============================================================================
MIL_INT MFTYPE HookFuncNbPreparedEntries(MIL_INT /*HookType*/, MIL_ID EventId, void* UserData)
   {
   bool* IsDevset = static_cast<bool*>(UserData);

   MIL_ID SrcDatasetId = M_NULL;
   MclassGetHookInfo(EventId, M_SRC_DATASET_ID + M_TYPE_MIL_ID, &SrcDatasetId);

   MIL_INT NbEntries = 0;
   MclassInquire(SrcDatasetId, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NbEntries);

   MIL_INT NbPreparedEntries = 0;
   MclassGetHookInfo(EventId, M_NUMBER_OF_PREPARED_SRC_ENTRIES + M_TYPE_MIL_INT, &NbPreparedEntries);

   if(NbPreparedEntries == 1)
      {
      *IsDevset ? MosPrintf(MIL_TEXT("Preparing the dev dataset...\n")) :
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

   MosPrintf(MIL_TEXT("Entry %d of %d completed with status: %s.%c"), NbPreparedEntries, NbEntries, StatusStr.c_str(), EndOfLine);

   if(NbPreparedEntries == NbEntries)
      {
      EndOfLine == '\r' ? MosPrintf(MIL_TEXT("\n\n")) : MosPrintf(MIL_TEXT("\n"));
      *IsDevset = true;
      }

   return M_NULL;
   }

//==============================================================================
void SetAugmentationControls(MIL_ID TrainCtx, bool* IsDevset)
   {
   MIL_ID DataPreparationCtx = MclassInquire(TrainCtx, M_DEFAULT, M_PREPARE_DATA_CONTEXT_ID + M_TYPE_MIL_ID, M_NULL);

   // Set seed for reproducibility.
   MclassControl(DataPreparationCtx, M_DEFAULT, M_SEED_MODE, M_USER_DEFINED);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_SEED_VALUE, 25);

   // Set some basic augmentation controls.
   MclassControl(DataPreparationCtx, M_DEFAULT, M_AUGMENT_NUMBER_MODE, M_FACTOR);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_AUGMENT_NUMBER_FACTOR, 10.0);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_AUGMENT_BALANCING, 0.0);

   // Enable some presets.
   MclassControl(DataPreparationCtx, M_DEFAULT, M_PRESET_ROTATION, M_ENABLE);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_PRESET_TRANSLATION, M_ENABLE);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_PRESET_CROP, M_ENABLE);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_PRESET_SCALE, M_ENABLE);
   MclassControl(DataPreparationCtx, M_DEFAULT, M_PRESET_GAMMA, M_ENABLE);

   // Hook function to show progress of augmentation.
   MclassHookFunction(DataPreparationCtx, M_PREPARE_ENTRY_POST, HookFuncNbPreparedEntries, IsDevset);
   }

//==============================================================================
void SetTrainControls(MIL_ID TrainCtx, MIL_ID Dataset)
   {
   CreateDirectory(EXAMPLE_TRAIN_DESTINATION_PATH);
   MclassControl(TrainCtx, M_DEFAULT, M_TRAIN_DESTINATION_FOLDER, EXAMPLE_TRAIN_DESTINATION_PATH);

   // Set parameters for the training context.
   MclassControl(TrainCtx, M_DEFAULT, M_MAX_EPOCH, 50);
   MclassControl(TrainCtx, M_DEFAULT, M_MINI_BATCH_SIZE, 8);
   MclassControl(TrainCtx, M_DEFAULT, M_INITIAL_LEARNING_RATE, 0.0025);
   MclassControl(TrainCtx, M_DEFAULT, M_LEARNING_RATE_DECAY, 0.05);

   // Set seed for reproducible results.
   MclassControl(TrainCtx, M_DEFAULT, M_SPLIT_SEED_MODE, M_FIXED);

   // Since we are performing a single dataset train, the dataset will be split into train/dev by the following percentage.
   MclassControl(TrainCtx, M_DEFAULT, M_SPLIT_PERCENTAGE, 80.0);

   // To get the best results, dataset specific weights should be used.
   MclassControl(TrainCtx, M_DEFAULT, M_CLASS_WEIGHT_STRENGTH, 50);
   }

//==============================================================================
void PrintStatusMessage(MIL_INT Status)
   {
   switch(Status)
      {
      case M_INTERNAL_ERROR:
         MosPrintf(MIL_TEXT("An unexpected internal error has occurred!\n"));
         break;
      case M_NON_FINITE_VALUE_DETECTED:
         MosPrintf(MIL_TEXT("Training terminated because a non-finite value was detected!\n"));
         break;
      case M_NOT_ENOUGH_GPU_MEMORY:
         MosPrintf(MIL_TEXT("Ran out of GPU memory, try reducing the batch size!\n"));
         break;
      case M_NOT_ENOUGH_MEMORY:
         MosPrintf(MIL_TEXT("Ran out of memory, try reducing the batch size!\n"));
         break;
      case M_TIMEOUT_REACHED:
         MosPrintf(MIL_TEXT("Timeout reached, try increasing the timeout!\n"));
         break;
      default:
         MosPrintf(MIL_TEXT("Unexpected status code received!\n"));
         break;
      }
   }

//==============================================================================
MIL_UNIQUE_CLASS_ID TrainTheModel(MIL_ID MilSystem, MIL_ID Dataset, MIL_ID DevDataset, MIL_ID MilDisplay)
   {
   // Allocate a context and a result for the training.
   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_SEG, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID TrainRes = MclassAllocResult(MilSystem, M_TRAIN_SEG_RESULT, M_DEFAULT, M_UNIQUE_ID);

   bool IsDevset = false;
   SetAugmentationControls(TrainCtx, &IsDevset);

   SetTrainControls(TrainCtx, Dataset);

   MclassPreprocess(TrainCtx, M_DEFAULT);

   MIL_INT TrainEngineUsed;
   MclassInquire(TrainCtx, M_CONTEXT, M_TRAIN_ENGINE_USED + M_TYPE_MIL_INT, &TrainEngineUsed);

   HookDatasetsPrepared TheHookDatasetsPrepared;
   TheHookDatasetsPrepared.m_SkipTrain = false;
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
      MosPrintf(MIL_TEXT("\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("WARNING: TRAINING ON CPU CAN TAKE OVER AN HOUR!...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n"));
      MosPrintf(MIL_TEXT("If a training on GPU was expected, use the CNN Train Engine Test under Classification in MILConfig for more information.\n"));
      MosPrintf(MIL_TEXT("\nPress <s> to skip the training and restore a pre-trained context, this is the recommended option.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to continue...\n"));

      char KeyVal = (char)MosGetch();
      if(KeyVal == 's' || KeyVal == 'S')
         {
         TheHookDatasetsPrepared.m_SkipTrain = true;
         }
      }

   MIL_STRING TrainEngineDescription;
   MclassInquire(TrainCtx, M_CONTEXT, M_TRAIN_ENGINE_USED_DESCRIPTION, TrainEngineDescription);

   MIL_INT ImageSizeX = 0;
   MIL_INT ImageSizeY = 0;
   GetSizes(MilSystem, Dataset, &ImageSizeX, &ImageSizeY);

   // Initialize the object responsible to display the train evolution.
   CTrainEvolutionDashboard TheTrainEvolutionDashboard(MilSystem, TrainCtx,
                                                       ImageSizeX, ImageSizeY,
                                                       TrainEngineUsed, TrainEngineDescription);

   // Initialize the hook associated to the epoch trained event.
   HookEpochData TheHookEpochData;
   TheHookEpochData.TheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_EPOCH_TRAINED, HookFuncEpoch, &TheHookEpochData);

   // Initialize the hook associated to the mini batch trained event.
   HookMiniBatchData TheHookMiniBatchData;
   TheHookMiniBatchData.TheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_MINI_BATCH_TRAINED, HookFuncMiniBatch, &TheHookMiniBatchData);

   TheHookDatasetsPrepared.m_DashboardId = TheTrainEvolutionDashboard.GetDashboardBufId();
   TheHookDatasetsPrepared.m_MilDisplay = MilDisplay;
   MclassHookFunction(TrainCtx, M_DATASETS_PREPARED, HookFuncDatasetsPrepared, &TheHookDatasetsPrepared);

   MosPrintf(MIL_TEXT("Augmenting the datasets before training.\n"));
   // Start the training process.
   MclassTrain(TrainCtx, M_NULL, Dataset, M_NULL, TrainRes, M_DEFAULT);

   MIL_UNIQUE_CLASS_ID TrainedCtx;

   // Check the training status to ensure the training has completed properly.
   MIL_INT Status = -1;
   MclassGetResult(TrainRes, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);
   if(Status == M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("\nTraining was successful.\n"));

      TrainedCtx = MclassAlloc(MilSystem, M_CLASSIFIER_SEG_PREDEFINED, M_DEFAULT, M_UNIQUE_ID);
      MclassCopyResult(TrainRes, M_DEFAULT, TrainedCtx, M_DEFAULT, M_TRAINED_CLASSIFIER, M_DEFAULT);

      // Copy the dev dataset to perform prediction on it later.
      MclassCopyResult(TrainRes, M_DEFAULT, DevDataset, M_DEFAULT, M_PREPARED_DEV_DATASET, M_DEFAULT);

      MosPrintf(MIL_TEXT("A training report was saved: \"TrainReport.csv\".\n"));
      MclassExport(MIL_TEXT("TrainReport.csv"), M_FORMAT_TXT, TrainRes, M_DEFAULT, M_TRAIN_REPORT, M_DEFAULT);

      std::vector<MIL_DOUBLE> TrainIOUMean;
      MclassGetResult(TrainRes, M_DEFAULT, M_TRAIN_DATASET_EPOCH_IOU_MEAN, TrainIOUMean);
      std::vector<MIL_DOUBLE>  DevIOUMean;
      MclassGetResult(TrainRes, M_DEFAULT, M_DEV_DATASET_EPOCH_IOU_MEAN, DevIOUMean);

      MIL_INT LastUpdatedEpochIndex;
      MclassGetResult(TrainRes, M_DEFAULT, M_LAST_EPOCH_UPDATED_PARAMETERS + M_TYPE_MIL_INT, &LastUpdatedEpochIndex);

      MosPrintf(MIL_TEXT("\nThe best epoch is considered to be the epoch with the highest dev mean IOU.\n"));
      MosPrintf(MIL_TEXT("\nThe best epoch was epoch %d with mean IOU on the dev dataset of %.8lf.\n"), LastUpdatedEpochIndex, DevIOUMean[LastUpdatedEpochIndex]);
      MosPrintf(MIL_TEXT("The associated train mean IOU is %.8lf.\n"), TrainIOUMean[LastUpdatedEpochIndex]);

      MosPrintf(MIL_TEXT("Press <Enter> to continue...\n"));
      MosGetch();
      }
   else if(Status == M_STOPPED_BY_REQUEST)
      {
      MosPrintf(MIL_TEXT("\nThe training was stopped or skipped so we have restored a pre-trained context to predict with.\n"));
      TrainedCtx = MclassRestore(EXAMPLE_PRETRAINED_PATH, MilSystem, M_DEFAULT, M_UNIQUE_ID);

      MclassCopyResult(TrainRes, M_DEFAULT, DevDataset, M_DEFAULT, M_PREPARED_DEV_DATASET, M_DEFAULT);

      MosPrintf(MIL_TEXT("\nPress <Enter> to continue...\n"));
      MosGetch();
      }
   else
      {
      PrintStatusMessage(Status);
      }

   return TrainedCtx;
   }

//==============================================================================
CDatasetViewer::CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth):
   m_MilSystem(MilSystem),
   m_Dataset(Dataset),
   Y_MARGIN(15),
   TEXT_HEIGHT(20),
   TEXT_MARGIN(20),
   OPACITY_INCREMENT(10.0),
   m_Opacity(50.0),
   m_DisplayGroundTruth(DisplayGroundTruth),
   m_DisplayContour(false)
   {
   PrintControls();

   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   MIL_INT ImageSizeX = 0;
   MIL_INT ImageSizeY = 0;
   GetSizes(m_MilSystem, m_Dataset, &ImageSizeX, &ImageSizeY);

   MIL_INT IconSize = ImageSizeY / NUMBER_OF_CLASSES;
   MIL_UNIQUE_BUF_ID DispImage = MbufAllocColor(MilSystem, 3, ImageSizeX + IconSize, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID DispChild = MbufChild2d(DispImage, 0, 0, ImageSizeX, ImageSizeY, M_UNIQUE_ID);

   MdispSelect(MilDisplay, DispImage);
   MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   MIL_UNIQUE_BUF_ID OverlayChild = MbufChild2d(MilOverlay, 0, 0, ImageSizeX, ImageSizeY, M_UNIQUE_ID);

   MbufClear(DispImage, M_COLOR_BLACK);

   // Set annotation color.
   MgraColor(M_DEFAULT, M_COLOR_RED);

   // Set up the display.
   for(int iter = 0; iter < NUMBER_OF_CLASSES; iter++)
      {
      // Allocate a child buffer per product category.
      MIL_UNIQUE_BUF_ID MilChildSample = MbufChild2d(DispImage, ImageSizeX, iter * IconSize, IconSize, IconSize, M_UNIQUE_ID);
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
         MgraText(M_DEFAULT, MilChildSample, 10, 10, Text);
         MgraText(M_DEFAULT, MilOverlayChildSample, 10, 10, Text);

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
   MIL_TEXT_CHAR OverlayText[512];
   MIL_INT EntryIndex = 0;
   bool Exit = false;
   while(!Exit)
      {
      MdispControl(MilDisplay, M_UPDATE, M_DISABLE);

      MIL_STRING EntryImagePath;
      MclassInquireEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH_ABS, EntryImagePath);
      MbufLoad(EntryImagePath, DispChild);

      MdispControl(MilDisplay, M_OVERLAY_OPACITY, m_Opacity);

      // Clear the buffer to the no region pixel class color if available, otherwise clear to black.
      MIL_INT NoRegionPixelClass = 0;
      MclassInquire(Dataset, M_DEFAULT, M_NO_REGION_PIXEL_CLASS + M_TYPE_MIL_INT, &NoRegionPixelClass);
      if(NoRegionPixelClass == M_NO_CLASS || NoRegionPixelClass == M_DEFAULT)
         {
         MbufClear(OverlayChild, 0.0);
         }
      else
         {
         MIL_DOUBLE NoClassColor = 0;
         MclassInquire(Dataset, M_CLASS_INDEX(NoRegionPixelClass), M_CLASS_DRAW_COLOR, &NoClassColor);
         MbufClear(OverlayChild, NoClassColor);
         }

      // Draw the desired overlay
      if(m_DisplayGroundTruth)
         {
         MclassDrawEntry(M_DEFAULT, Dataset, OverlayChild, M_GROUND_TRUTH_IMAGE + M_PSEUDO_COLOR, EntryIndex, M_DEFAULT_KEY, M_SEGMENTATION, M_DEFAULT, M_NULL, M_DEFAULT);
         MosSprintf(OverlayText, 512, MIL_TEXT("Ground truth overlay"));
         }
      else
         {
         MIL_STRING SegmentationPath;
         MclassInquireEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_DEFAULT, M_SEGMENTATION_PATH_ABS, SegmentationPath);
         if(!SegmentationPath.empty())
            {
            if(m_DisplayContour)
               {
               MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
               MdispControl(MilDisplay, M_OVERLAY_OPACITY, 100.0);
               MclassDrawEntry(GraContext, Dataset, OverlayChild, M_DRAW_BEST_INDEX_CONTOUR_IMAGE + M_PSEUDO_COLOR, EntryIndex, M_DEFAULT_KEY, M_SEGMENTATION, M_DEFAULT, M_NULL, M_DEFAULT);
               MosSprintf(OverlayText, 512, MIL_TEXT("Best index predicted contour image overlay"));
               }
            else
               {
               MclassDrawEntry(GraContext, Dataset, OverlayChild, M_DRAW_BEST_INDEX_IMAGE + M_PSEUDO_COLOR, EntryIndex, M_DEFAULT_KEY, M_SEGMENTATION, M_DEFAULT, M_NULL, M_DEFAULT);
               MosSprintf(OverlayText, 512, MIL_TEXT("Best index predicted overlay"));
               }
            }
         else
            {
            MosSprintf(OverlayText, 512, MIL_TEXT("No prediction to display"));
            }
         }

      MIL_INT TextYPos = Y_MARGIN;

      MosSprintf(IndexText, 512, MIL_TEXT("Entry Index %d / %d"), EntryIndex, NbEntries - 1);
      MgraText(GraContext, DispChild, TEXT_MARGIN, TextYPos, IndexText);
      MgraText(GraContext, OverlayChild, TEXT_MARGIN, TextYPos, IndexText);
      TextYPos += TEXT_HEIGHT;

      MgraText(GraContext, DispChild, TEXT_MARGIN, TextYPos, OverlayText);
      MgraText(GraContext, OverlayChild, TEXT_MARGIN, TextYPos, OverlayText);

      MdispControl(MilDisplay, M_UPDATE, M_ENABLE);

      // Look for user key input
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
            case 'I':
            case 'i':
               if(m_Opacity + OPACITY_INCREMENT <= 100.0)
                  {
                  m_Opacity += OPACITY_INCREMENT;
                  }
               break;
            case 'D':
            case 'd':
               if(m_Opacity - OPACITY_INCREMENT >= 0.0)
                  {
                  m_Opacity -= OPACITY_INCREMENT;
                  }
               break;
            case 'E':
            case 'e':
               Exit = true;
               break;
            case 'T':
            case 't':
               m_DisplayGroundTruth = !m_DisplayGroundTruth;
               break;
            case 'C':
            case 'c':
               m_DisplayContour = !m_DisplayContour;
               break;
            default:
               break;
            }
         }
      }
   }

//==============================================================================
void CDatasetViewer::PrintControls()
   {
   MosPrintf(MIL_TEXT("Here are the dataset viewer controls:\n"));
   MosPrintf(MIL_TEXT("n: Display next image\n"));
   MosPrintf(MIL_TEXT("p: Display previous image\n"));
   MosPrintf(MIL_TEXT("i: Increase the opacity\n"));
   MosPrintf(MIL_TEXT("d: Decrease the opacity\n"));
   MosPrintf(MIL_TEXT("t: Toggle between the GT overlay and the prediction overlay\n"));
   MosPrintf(MIL_TEXT("c: Toggle the prediction overlay between the best index and contour image\n"));
   MosPrintf(MIL_TEXT("e: exit\n\n"));

   MosPrintf(MIL_TEXT("The possible colors in the overlay are as follows:\n"));
   MosPrintf(MIL_TEXT("Green: No defect\n"));
   MosPrintf(MIL_TEXT("Red: Spot defect\n"));
   MosPrintf(MIL_TEXT("Blue: Pit defect\n"));
   MosPrintf(MIL_TEXT("White: Don't care class (introduced by augmentation)\n\n"));

   MosPrintf(MIL_TEXT("Select a dataset viewer control:\n"));
   }
