//*************************************************************************************
//
// File name: ClassDetectionCompleteTrain.cpp
//
// Synopsis:  This program uses the classification module to train
//            a context that can detect knots in wood.
//
// Note:      GPU training can be enabled with a MIL update for 64-bit.
//            This can dramatically increase the training speed.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <windows.h>
#include <algorithm>

//==============================================================================
// Example description.
//==============================================================================
void PrintHeader()
   {
   MosPrintf(
      MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("ClassDetectionCompleteTrain\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This example trains an object detection classifier to detect knots of different\n")
      MIL_TEXT("sizes in wood.\n")
      MIL_TEXT("The first step imports the dataset.\n")
      MIL_TEXT("The second step trains a context and displays the train evolution.\n")
      MIL_TEXT("The final step performs predictions on a test dataset using the trained object\n")
      MIL_TEXT("detection classifier as a final check of its performance.\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("graphic, classification.\n\n"));
   }

//==============================================================================
// Constants.
//==============================================================================
#define EXAMPLE_IMAGE_ROOT_PATH        M_IMAGE_PATH MIL_TEXT("Classification/PlywoodTrain/")
#define EXAMPLE_DATASET_ROOT_PATH      EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("Dataset")
#define EXAMPLE_TEST_DATASET_ROOT_PATH EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("TestDataset")
#define EXAMPLE_PRETRAINED_PATH        EXAMPLE_IMAGE_ROOT_PATH MIL_TEXT("PlywoodODNet.mclass")
#define EXAMPLE_TRAIN_DESTINATION_PATH MIL_TEXT("Train/")

static const MIL_INT NUMBER_OF_CLASSES = 2;

//==============================================================================
// Classes.
//==============================================================================
class CTrainEvolutionDashboard
   {
   public:
      CTrainEvolutionDashboard(
         MIL_ID MilSystem,
         MIL_ID TrainCtx,
         MIL_INT TrainImageSizeX,
         MIL_INT TrainImageSizeY,
         MIL_INT TrainEngineUsed,
         const MIL_STRING& TrainEngineDescription);

      ~CTrainEvolutionDashboard();

      void AddEpochData(MIL_DOUBLE Loss, MIL_INT CurEpoch, MIL_DOUBLE EpochBenchMean);

      void AddMiniBatchData(
         MIL_DOUBLE Loss,
         MIL_INT MinibatchIdx,
         MIL_INT EpochIdx,
         MIL_INT NbBatchPerEpoch);

      MIL_ID GetDashboardBufId() const { return m_DashboardBufId; }

   private:
      void UpdateTrainLoss(MIL_DOUBLE Loss);

      void UpdateDevLoss(MIL_DOUBLE Loss);

      void UpdateTrainLossGraph(
         MIL_DOUBLE Loss,
         MIL_INT MiniBatchIdx,
         MIL_INT EpochIdx,
         MIL_INT NbBatchPerEpoch);

      void UpdateDevLossGraph(MIL_DOUBLE Loss, MIL_INT EpochIdx);

      void UpdateProgression(MIL_INT MinibatchIdx, MIL_INT EpochIdx, MIL_INT NbBatchPerEpoch);

      void DrawSectionSeparators();

      void DrawBufferFrame(MIL_ID BufId, MIL_INT FrameThickness);

      void InitializeLossGraph();

      void WriteGeneralTrainInfo(
         MIL_INT MinibatchSize,
         MIL_INT TrainImageSizeX,
         MIL_INT TrainImageSizeY,
         MIL_DOUBLE LearningRate,
         MIL_INT TrainEngineUsed,
         const MIL_STRING& TrainEngineDescription);

      MIL_UNIQUE_BUF_ID m_DashboardBufId {M_NULL};
      MIL_UNIQUE_GRA_ID m_TheGraContext {M_NULL};

      MIL_UNIQUE_BUF_ID m_LossInfoBufId {M_NULL};
      MIL_UNIQUE_BUF_ID m_LossGraphBufId {M_NULL};
      MIL_UNIQUE_BUF_ID m_ProgressionInfoBufId {M_NULL};

      MIL_INT m_MaxEpoch {0};
      MIL_INT m_DashboardWidth {0};
      MIL_INT m_LastTrainMinibatchPosX {0};
      MIL_INT m_LastTrainMinibatchPosY {0};
      MIL_INT m_LastDevEpochLossPosX {0};
      MIL_INT m_LastDevEpochLossPosY {0};

      MIL_INT m_YPositionForTrainLossText {0};
      MIL_INT m_YPositionForDevLossText {0};

      MIL_DOUBLE m_EpochBenchMean {-1.0};

      // Constants useful for the graph.
      const MIL_INT GRAPH_SIZE_X {600};
      const MIL_INT GRAPH_SIZE_Y {400};
      const MIL_INT GRAPH_TOP_MARGIN {30};
      const MIL_INT MARGIN {50};
      const MIL_INT EPOCH_AND_MINIBATCH_REGION_HEIGHT {190};
      const MIL_INT PROGRESSION_INFO_REGION_HEIGHT {100};

      const MIL_INT LOSS_EXPONENT_MAX {0};
      const MIL_INT LOSS_EXPONENT_MIN {-5};

      const MIL_DOUBLE COLOR_GENERAL_INFO {M_RGB888(0, 176, 255)};
      const MIL_DOUBLE COLOR_DEV_SET_INFO {M_COLOR_MAGENTA};
      const MIL_DOUBLE COLOR_TRAIN_SET_INFO {M_COLOR_GREEN};
      const MIL_DOUBLE COLOR_PROGRESS_BAR {M_COLOR_DARK_GREEN};
   };

class CDatasetViewer
   {
   public:
      CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth);

   private:
      void PrintControls();

      MIL_ID m_MilSystem {M_NULL};
      MIL_ID m_Dataset {M_NULL};

      bool m_DisplayGroundTruth {false};

      const MIL_INT Y_MARGIN {15};
      const MIL_INT TEXT_HEIGHT {20};
      const MIL_INT TEXT_MARGIN {20};
   };

//==============================================================================
// Structs.
//==============================================================================
struct SHookDatasetsPrepared
   {
   MIL_ID m_DashboardId {M_NULL};
   MIL_ID m_MilDisplay {M_NULL};
   };

struct SHookEpochData
   {
   CTrainEvolutionDashboard* m_pTheDashboard {nullptr};
   };

struct SHookMiniBatchData
   {
   CTrainEvolutionDashboard* m_pTheDashboard {nullptr};
   };

//==============================================================================
// Functions.
//==============================================================================
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem);

void GetImageSizes(
   MIL_ID MilSystem,
   MIL_ID Dataset,
   MIL_INT* pImgSizeX,
   MIL_INT* pImgSizeY);

MIL_INT GetNumberOfGTs(MIL_ID Dataset, MIL_INT EntryIndex);

MIL_INT IsTrainingSupportedOnPlatform(MIL_ID MilSystem);

void LoadDatasets(MIL_ID MilSystem, MIL_ID Dataset, MIL_ID TestDataset, bool SkipTrain);

void PredictUsingTrainedContext(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID TrainedCtx,
   MIL_ID TestDataset);

void PrintStatusMessage(MIL_INT Status);

MIL_STRING ConvertPrepareDataStatusToStr(MIL_INT Status);

void SetAugmentationControls(MIL_ID TrainCtx, bool* pIsDevDataset);

void SetTrainControls(MIL_ID TrainCtx);

MIL_UNIQUE_CLASS_ID TrainTheModel(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID Dataset);

//==============================================================================
// Hook functions.
//==============================================================================
MIL_INT MFTYPE HookDatasetsPreparedFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData);

MIL_INT MFTYPE HookEpochFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData);

MIL_INT MFTYPE HookMiniBatchFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData);

MIL_INT MFTYPE HookNumPreparedEntriesFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData);

//==============================================================================
// Main.
//==============================================================================
int MosMain()
   {
   PrintHeader();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("\nTo skip the training and proceed directly to prediction press <s>.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   char KeyVal = (char)MosGetch();
   bool SkipTrain = false;
   switch(KeyVal)
      {
      case 'S':
      case 's':
         SkipTrain = true;
         MosPrintf(MIL_TEXT("Skipping the training.\n"));
         break;
      }

   if(!SkipTrain && IsTrainingSupportedOnPlatform(MilSystem) != M_TRUE)
      {
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      return -1;
      }

   MosPrintf(MIL_TEXT("\n***************************************************************\n"));
   MosPrintf(MIL_TEXT("IMPORTING THE DATASETS..."));
   MosPrintf(MIL_TEXT("\n***************************************************************\n"));

   MIL_UNIQUE_CLASS_ID Dataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID TestDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   LoadDatasets(MilSystem, Dataset, TestDataset, SkipTrain);

   MIL_UNIQUE_CLASS_ID TrainedCtx;
   if(SkipTrain)
      {
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));
      MosPrintf(MIL_TEXT("RESTORING A PRETRAINED CONTEXT..."));
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));

      TrainedCtx = MclassRestore(EXAMPLE_PRETRAINED_PATH, MilSystem, M_DEFAULT, M_UNIQUE_ID);
      MosPrintf(MIL_TEXT("Successfully restored the trained context.\n\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));
      MosPrintf(MIL_TEXT("TRAINING... THIS WILL TAKE SOME TIME..."));
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));

      TrainedCtx = TrainTheModel(MilSystem, MilDisplay, Dataset);
      }

   if(TrainedCtx)
      {
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));
      MosPrintf(MIL_TEXT("PREDICTING USING THE TRAINED CONTEXT..."));
      MosPrintf(MIL_TEXT("\n***************************************************************\n"));

      PredictUsingTrainedContext(MilSystem, MilDisplay, TrainedCtx, TestDataset);
      }
   else
      {
      MosPrintf(MIL_TEXT("\nTraining has not completed properly!\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      return -1;
      }

   return 0;
   }

//==============================================================================
CTrainEvolutionDashboard::CTrainEvolutionDashboard(
   MIL_ID MilSystem,
   MIL_ID TrainCtx,
   MIL_INT TrainImageSizeX,
   MIL_INT TrainImageSizeY,
   MIL_INT TrainEngineUsed,
   const MIL_STRING& TrainEngineDescription)
   {
   MclassInquire(TrainCtx, M_DEFAULT, M_MAX_EPOCH + M_TYPE_MIL_INT, &m_MaxEpoch);

   MIL_DOUBLE InitLearningRate {0.0};
   MclassInquire(TrainCtx, M_DEFAULT, M_INITIAL_LEARNING_RATE + M_TYPE_MIL_DOUBLE, &InitLearningRate);
   MIL_INT MiniBatchSize {0};
   MclassInquire(TrainCtx, M_DEFAULT, M_MINI_BATCH_SIZE + M_TYPE_MIL_INT, &MiniBatchSize);

   const MIL_INT GraphBoxWidth = GRAPH_SIZE_X + 2 * MARGIN;
   const MIL_INT GraphBoxHeight = GRAPH_SIZE_Y + GRAPH_TOP_MARGIN + MARGIN;

   m_DashboardWidth = GraphBoxWidth;
   const MIL_INT DashboardHeight = GraphBoxHeight + EPOCH_AND_MINIBATCH_REGION_HEIGHT + PROGRESSION_INFO_REGION_HEIGHT;

   m_DashboardBufId = MbufAllocColor(MilSystem, 3, m_DashboardWidth, DashboardHeight, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MbufClear(m_DashboardBufId, M_COLOR_BLACK);

   m_TheGraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);

   const MIL_INT GraphYPosition = EPOCH_AND_MINIBATCH_REGION_HEIGHT;
   const MIL_INT ProgressionInfoYPosition = GraphYPosition + GraphBoxHeight;

   m_LossInfoBufId = MbufChild2d(m_DashboardBufId, 0, 0, GraphBoxWidth, EPOCH_AND_MINIBATCH_REGION_HEIGHT, M_UNIQUE_ID);
   m_LossGraphBufId = MbufChild2d(m_DashboardBufId, 0, GraphYPosition, GraphBoxWidth, GraphBoxHeight, M_UNIQUE_ID);
   m_ProgressionInfoBufId = MbufChild2d(m_DashboardBufId, 0, ProgressionInfoYPosition, m_DashboardWidth, PROGRESSION_INFO_REGION_HEIGHT, M_UNIQUE_ID);

   DrawSectionSeparators();

   InitializeLossGraph();

   WriteGeneralTrainInfo(MiniBatchSize, TrainImageSizeX, TrainImageSizeY, InitLearningRate, TrainEngineUsed, TrainEngineDescription);
   }

//==============================================================================
CTrainEvolutionDashboard::~CTrainEvolutionDashboard()
   {
   m_TheGraContext = M_NULL;
   m_LossInfoBufId = M_NULL;
   m_LossGraphBufId = M_NULL;
   m_ProgressionInfoBufId = M_NULL;
   m_DashboardBufId = M_NULL;
   }

//==============================================================================
void CTrainEvolutionDashboard::AddEpochData(MIL_DOUBLE Loss, MIL_INT CurEpoch, MIL_DOUBLE EpochBenchMean)
   {
   m_EpochBenchMean = EpochBenchMean;
   UpdateDevLoss(Loss);
   UpdateDevLossGraph(Loss, CurEpoch);
   }

//==============================================================================
void CTrainEvolutionDashboard::AddMiniBatchData(
   MIL_DOUBLE Loss,
   MIL_INT MinibatchIdx,
   MIL_INT EpochIdx,
   MIL_INT NbBatchPerEpoch)
   {
   UpdateTrainLoss(Loss);
   UpdateTrainLossGraph(Loss, MinibatchIdx, EpochIdx, NbBatchPerEpoch);
   UpdateProgression(MinibatchIdx, EpochIdx, NbBatchPerEpoch);
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
void CTrainEvolutionDashboard::UpdateTrainLossGraph(
   MIL_DOUBLE Loss,
   MIL_INT MiniBatchIdx,
   MIL_INT EpochIdx,
   MIL_INT NbBatchPerEpoch)
   {
   const MIL_INT NbMiniBatch = m_MaxEpoch * NbBatchPerEpoch;
   const MIL_INT CurMiniBatch = EpochIdx * NbBatchPerEpoch + MiniBatchIdx;

   const MIL_DOUBLE XRatio = static_cast<MIL_DOUBLE>(CurMiniBatch) / static_cast<MIL_DOUBLE>(NbMiniBatch);

   const MIL_INT CurTrainMBPosX = MARGIN + static_cast<MIL_INT>(XRatio * static_cast<MIL_DOUBLE>(GRAPH_SIZE_X));

   const MIL_DOUBLE MaxVal = std::pow(10.0, LOSS_EXPONENT_MAX);
   const MIL_INT NbTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;

   // Saturate to the highest value of the graph.
   Loss = std::min<MIL_DOUBLE>(Loss, MaxVal);
   const MIL_DOUBLE Log10RemapPos = std::max<MIL_DOUBLE>(log10(Loss) + (-LOSS_EXPONENT_MIN), 0.0);
   const MIL_DOUBLE YRatio = Log10RemapPos / static_cast<MIL_DOUBLE>(NbTick);

   const MIL_INT CurTrainMBPosY = GRAPH_TOP_MARGIN + static_cast<MIL_INT>((1.0 - YRatio) * static_cast<MIL_DOUBLE>(GRAPH_SIZE_Y));

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
void CTrainEvolutionDashboard::UpdateDevLossGraph(MIL_DOUBLE Loss, MIL_INT EpochIdx)
   {
   const MIL_DOUBLE XRatio = static_cast<MIL_DOUBLE>(EpochIdx + 1) / static_cast<MIL_DOUBLE>(m_MaxEpoch);

   const MIL_INT CurTrainMBPosX = MARGIN + static_cast<MIL_INT>(XRatio * static_cast<MIL_DOUBLE>(GRAPH_SIZE_X));

   const MIL_DOUBLE MaxVal = std::pow(10.0, LOSS_EXPONENT_MAX);
   const MIL_INT NbTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;

   // Saturate to the highest value of the graph.
   Loss = std::min<MIL_DOUBLE>(Loss, MaxVal);
   const MIL_DOUBLE Log10RemapPos = std::max<MIL_DOUBLE>(log10(Loss) + (-LOSS_EXPONENT_MIN), 0.0);
   const MIL_DOUBLE YRatio = Log10RemapPos / static_cast<MIL_DOUBLE>(NbTick);

   const MIL_INT CurTrainMBPosY = GRAPH_TOP_MARGIN + static_cast<MIL_INT>((1.0 - YRatio) * static_cast<MIL_DOUBLE>(GRAPH_SIZE_Y));

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
      const MIL_DOUBLE MinibatchBenchMean = m_EpochBenchMean / static_cast<MIL_DOUBLE>(NbBatchPerEpoch);
      const MIL_DOUBLE RemainingTime = MinibatchBenchMean * static_cast<MIL_DOUBLE>(NbMinibatchRemaining);
      MIL_TEXT_CHAR RemainingTimeText[512];
      MosSprintf(RemainingTimeText, 512, MIL_TEXT("Estimated remaining time: %8.0lf seconds"), RemainingTime);

      if(NbMinibatchDone == NbMinibatch)
         {
         MgraText(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin, MIL_TEXT("Training completed!                         "));
         }
      else
         {
         MgraText(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin, RemainingTimeText);
         }
      }

   // Update the progression bar.
   const MIL_INT ProgressionBarWidth = m_DashboardWidth - 2 * MARGIN;
   const MIL_INT ProgressionBarHeight = 30;
   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   MgraRectFill(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin + TextHeight, MARGIN + ProgressionBarWidth, YMargin + TextHeight + ProgressionBarHeight);

   const MIL_DOUBLE PercentageComplete = static_cast<MIL_DOUBLE>(NbMinibatchDone) / static_cast<MIL_DOUBLE>(NbMinibatch);
   const MIL_INT PercentageCompleteWidth = static_cast<MIL_INT>(PercentageComplete * ProgressionBarWidth);
   MgraColor(m_TheGraContext, COLOR_PROGRESS_BAR);
   MgraRectFill(m_TheGraContext, m_ProgressionInfoBufId, MARGIN, YMargin + TextHeight, MARGIN + PercentageCompleteWidth, YMargin + TextHeight + ProgressionBarHeight);
   }

//==============================================================================
void CTrainEvolutionDashboard::DrawSectionSeparators()
   {
   // Draw a frame for the whole dashboard.
   DrawBufferFrame(m_DashboardBufId, 4);
   // Draw a frame for each section.
   DrawBufferFrame(m_LossInfoBufId, 2);
   DrawBufferFrame(m_LossGraphBufId, 2);
   DrawBufferFrame(m_ProgressionInfoBufId, 2);
   }

//==============================================================================
void CTrainEvolutionDashboard::DrawBufferFrame(MIL_ID BufId, MIL_INT FrameThickness)
   {
   const MIL_ID SizeX = MbufInquire(BufId, M_SIZE_X, M_NULL);
   const MIL_ID SizeY = MbufInquire(BufId, M_SIZE_Y, M_NULL);

   MgraColor(m_TheGraContext, COLOR_GENERAL_INFO);
   MgraRectFill(m_TheGraContext, BufId, 0, 0, SizeX - 1, FrameThickness - 1);
   MgraRectFill(m_TheGraContext, BufId, SizeX - FrameThickness, 0, SizeX - 1, SizeY - 1);
   MgraRectFill(m_TheGraContext, BufId, 0, SizeY - FrameThickness, SizeX - 1, SizeY - 1);
   MgraRectFill(m_TheGraContext, BufId, 0, 0, FrameThickness - 1, SizeY - 1);
   }

//==============================================================================
void CTrainEvolutionDashboard::InitializeLossGraph()
   {
   // Draw axis.
   MgraColor(m_TheGraContext, M_COLOR_WHITE);
   MgraRect(m_TheGraContext, m_LossGraphBufId, MARGIN, GRAPH_TOP_MARGIN, MARGIN + GRAPH_SIZE_X, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);

   MgraControl(m_TheGraContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);

   const MIL_INT NbLossValueTick = LOSS_EXPONENT_MAX - LOSS_EXPONENT_MIN;
   const MIL_DOUBLE TickRatio = 1.0 / static_cast<MIL_DOUBLE>(NbLossValueTick);

   MIL_DOUBLE TickNum = 0.0;
   for(MIL_INT i = LOSS_EXPONENT_MAX; i >= LOSS_EXPONENT_MIN; i--)
      {
      MIL_TEXT_CHAR CurTickText[128];
      MosSprintf(CurTickText, 128, MIL_TEXT("1e%d"), i);

      const MIL_INT TickYPos = static_cast<MIL_INT>(TickNum * TickRatio * GRAPH_SIZE_Y);
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
      const MIL_DOUBLE Percentage = static_cast<MIL_DOUBLE>(CurTick) / static_cast<MIL_DOUBLE>(m_MaxEpoch);
      const MIL_INT XOffset = static_cast<MIL_INT>(Percentage * GRAPH_SIZE_X);
      MgraText(m_TheGraContext, m_LossGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y + 5, M_TO_STRING(CurTick - 1));
      MgraLine(m_TheGraContext, m_LossGraphBufId, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y - 5, MARGIN + XOffset, GRAPH_TOP_MARGIN + GRAPH_SIZE_Y);
      }
   }

//==============================================================================
void CTrainEvolutionDashboard::WriteGeneralTrainInfo(
   MIL_INT MinibatchSize,
   MIL_INT TrainImageSizeX,
   MIL_INT TrainImageSizeY,
   MIL_DOUBLE LearningRate,
   MIL_INT TrainEngineUsed,
   const MIL_STRING& TrainEngineDescription)
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
      {
      MosSprintf(TheString, 512, MIL_TEXT("Training is being performed on the CPU"));
      }
   else
      {
      MosSprintf(TheString, 512, MIL_TEXT("Training is being performed on the GPU"));
      }

   MgraText(m_TheGraContext, m_LossInfoBufId, TextMargin, TextYPos, TheString);
   TextYPos += TextHeight;

   MosSprintf(TheString, 512, MIL_TEXT("Engine: %s"), TrainEngineDescription.c_str());
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

   // The loss will be drawn under later on, so we retain its position.
   m_YPositionForTrainLossText = TextYPos;
   TextYPos += TextHeight;
   m_YPositionForDevLossText = TextYPos;
   }

//==============================================================================
CDatasetViewer::CDatasetViewer(MIL_ID MilSystem, MIL_ID Dataset, bool DisplayGroundTruth)
   : m_MilSystem(MilSystem)
   , m_Dataset(Dataset)
   , m_DisplayGroundTruth(DisplayGroundTruth)
   {
   PrintControls();

   MIL_UNIQUE_DISP_ID MilDisplay = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);

   MIL_INT ImageSizeX = 0;
   MIL_INT ImageSizeY = 0;
   GetImageSizes(m_MilSystem, m_Dataset, &ImageSizeX, &ImageSizeY);

   const MIL_INT IconSize = ImageSizeY / NUMBER_OF_CLASSES;
   MIL_UNIQUE_BUF_ID DispImage = MbufAllocColor(MilSystem, 3, ImageSizeX + IconSize, ImageSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID DispChild = MbufChild2d(DispImage, 0, 0, ImageSizeX, ImageSizeY, M_UNIQUE_ID);

   MdispSelect(MilDisplay, DispImage);
   MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
   MIL_UNIQUE_BUF_ID OverlayChild = MbufChild2d(MilOverlay, 0, 0, ImageSizeX, ImageSizeY, M_UNIQUE_ID);

   MbufClear(DispImage, M_COLOR_BLACK);

   // For bounding boxes.
   MIL_UNIQUE_GRA_ID GraList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, GraList);

   // Set annotation color.
   MgraColor(M_DEFAULT, M_COLOR_RED);

   // Set up the display.
   for(MIL_INT Iter = 0; Iter < NUMBER_OF_CLASSES; Iter++)
      {
      // Allocate a child buffer per product category.
      MIL_UNIQUE_BUF_ID MilChildSample = MbufChild2d(DispImage, ImageSizeX, Iter * IconSize, IconSize, IconSize, M_UNIQUE_ID);
      MIL_UNIQUE_BUF_ID MilOverlayChildSample = MbufChild2d(MilOverlay, ImageSizeX, Iter * IconSize, IconSize, IconSize, M_UNIQUE_ID);
      MbufClear(MilChildSample, M_COLOR_BLACK);
      MbufClear(MilOverlayChildSample, M_COLOR_BLACK);

      // Load the sample image.
      MIL_ID ClassIconId = MclassInquire(m_Dataset, M_CLASS_INDEX(Iter), M_CLASS_ICON_ID + M_TYPE_MIL_ID, M_NULL);

      // Retrieve the class description.
      MIL_STRING Text;
      MclassInquire(m_Dataset, M_CLASS_INDEX(Iter), M_CLASS_NAME, Text);

      if(ClassIconId != M_NULL)
         {
         // Retrieve the color associated to the class.
         MIL_DOUBLE ClassColor;
         MclassInquire(m_Dataset, M_CLASS_INDEX(Iter), M_CLASS_DRAW_COLOR, &ClassColor);

         // Draw the class name using the color associated to the class.
         MgraColor(M_DEFAULT, ClassColor);
         MgraText(M_DEFAULT, MilChildSample, 10, 10, Text);
         MgraText(M_DEFAULT, MilOverlayChildSample, 10, 10, Text);

         const MIL_INT ClassImageExampleSizeX = MbufInquire(ClassIconId, M_SIZE_X, M_NULL);
         const MIL_INT ClassImageExampleSizeY = MbufInquire(ClassIconId, M_SIZE_Y, M_NULL);

         if((ClassImageExampleSizeX >= IconSize) || (ClassImageExampleSizeY >= IconSize))
            {
            MimResize(ClassIconId, MilChildSample, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
            MimResize(ClassIconId, MilOverlayChildSample, M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE);
            }
         else
            {
            const MIL_INT OffsetX = (IconSize - ClassImageExampleSizeX) / 2;
            const MIL_INT OffsetY = (IconSize - ClassImageExampleSizeY) / 2;
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

      MdispControl(MilDisplay, M_OVERLAY_OPACITY, 0.0);

      MgraClear(M_DEFAULT, GraList);
      MbufClear(OverlayChild, 0.0);

      // Draw the desired overlay
      if(m_DisplayGroundTruth)
         {
         const MIL_INT NumGTs = GetNumberOfGTs(Dataset, EntryIndex);
         MclassDrawEntry(M_DEFAULT, Dataset, GraList, M_DESCRIPTOR_TYPE_BOX + M_PSEUDO_COLOR, EntryIndex, M_DEFAULT_KEY, M_DETECTION, M_DEFAULT, M_NULL, M_DEFAULT);
         MosSprintf(OverlayText, 512, MIL_TEXT("Ground truth overlay, there are %d GTs"), NumGTs);
         }
      else
         {
         MIL_INT PredictInfo {M_FALSE};
         MclassGetResultEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_DETECTION, M_DEFAULT, M_PREDICT_INFO + M_TYPE_MIL_INT, &PredictInfo);
         if(PredictInfo == M_TRUE)
            {
            MIL_INT NumInstances {0};
            MclassGetResultEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_DETECTION, M_DEFAULT, M_NUMBER_OF_INSTANCES + M_TYPE_MIL_INT, &NumInstances);

            MclassDrawEntry(GraContext, Dataset, GraList, M_DRAW_BOX + M_DRAW_BOX_NAME + M_DRAW_BOX_SCORE, EntryIndex, M_DEFAULT_KEY, M_DETECTION, M_DEFAULT, M_NULL, M_DEFAULT);
            MosSprintf(OverlayText, 512, MIL_TEXT("%d instance(s) found"), NumInstances);
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
            case 'E':
            case 'e':
               Exit = true;
               break;
            case 'T':
            case 't':
               m_DisplayGroundTruth = !m_DisplayGroundTruth;
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
   MosPrintf(MIL_TEXT("t: Toggle between the GT overlay and the prediction overlay\n"));
   MosPrintf(MIL_TEXT("e: exit\n\n"));

   MosPrintf(MIL_TEXT("The possible colors in the overlay are:\n"));
   MosPrintf(MIL_TEXT("Green: Small knot\n"));
   MosPrintf(MIL_TEXT("Red: Large knot\n"));

   MosPrintf(MIL_TEXT("Select a dataset viewer control:\n"));
   }

//==============================================================================
MIL_INT CnnTrainEngineDLLInstalled(MIL_ID MilSystem)
   {
   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_DET, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT IsInstalled {M_FALSE};
   MclassInquire(TrainCtx, M_DEFAULT, M_TRAIN_ENGINE_IS_INSTALLED + M_TYPE_MIL_INT, &IsInstalled);

   return IsInstalled;
   }

//==============================================================================
void GetImageSizes(
   MIL_ID MilSystem,
   MIL_ID Dataset,
   MIL_INT* pImgSizeX,
   MIL_INT* pImgSizeY)
   {
   MIL_STRING EntryImgPathAbs {};
   MclassInquireEntry(Dataset, 0, M_DEFAULT_KEY, M_DEFAULT, M_ENTRY_IMAGE_PATH_ABS, EntryImgPathAbs);

   MbufDiskInquire(EntryImgPathAbs, M_SIZE_X, pImgSizeX);
   MbufDiskInquire(EntryImgPathAbs, M_SIZE_Y, pImgSizeY);
   }

//==============================================================================
MIL_INT GetNumberOfGTs(MIL_ID Dataset, MIL_INT EntryIndex)
   {
   MIL_INT NumGTs {0};

   const MIL_INT NumRegions = MclassInquireEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_DEFAULT, M_NUMBER_OF_REGIONS, M_NULL);
   // Skip region 0, we want the bounding boxes.
   for(MIL_INT RegionIndex {1}; RegionIndex < NumRegions; RegionIndex++)
      {
      NumGTs += MclassInquireEntry(Dataset, EntryIndex, M_DEFAULT_KEY, M_REGION_INDEX(RegionIndex), M_NUMBER_OF_DESCRIPTOR_TYPE_BOX, M_NULL);
      }
   return NumGTs;
   }

//==============================================================================
MIL_INT IsTrainingSupportedOnPlatform(MIL_ID MilSystem)
   {
   MIL_ID MilSysOwnerApp {M_NULL};
   MsysInquire(MilSystem, M_OWNER_APPLICATION, &MilSysOwnerApp);

   MIL_INT SysPlatformBitness {M_NULL};
   MappInquire(MilSysOwnerApp, M_PLATFORM_BITNESS, &SysPlatformBitness);

   MIL_INT SysOsType {M_NULL};
   MappInquire(MilSysOwnerApp, M_PLATFORM_OS_TYPE, &SysOsType);

   const bool SupportedTrainingPlatform = ((SysPlatformBitness == 64) && (SysOsType == M_OS_WINDOWS));

   if(!SupportedTrainingPlatform)
      {
      MosPrintf(MIL_TEXT("\n***** MclassTrain() is available only for Windows 64-bit platforms. *****\n"));
      return M_FALSE;
      }

   if(!CnnTrainEngineDLLInstalled(MilSystem))
      {
      MosPrintf(MIL_TEXT("\n***** MclassTrain() cannot run; no train engine is installed. *****\n"));
      return M_FALSE;
      }

   return M_TRUE;
   }

//==============================================================================
void LoadDatasets(MIL_ID MilSystem, MIL_ID Dataset, MIL_ID TestDataset, bool SkipTrain)
   {
   MclassImport(EXAMPLE_DATASET_ROOT_PATH, M_IMAGE_DATASET_FOLDER, Dataset, M_DEFAULT, M_COMPLETE, M_DEFAULT);
   MclassImport(EXAMPLE_TEST_DATASET_ROOT_PATH, M_IMAGE_DATASET_FOLDER, TestDataset, M_DEFAULT, M_COMPLETE, M_DEFAULT);

   MosPrintf(MIL_TEXT("The datasets were successfully imported.\n\n"));

   if(!SkipTrain)
      {
      MosPrintf(MIL_TEXT("Press <v> to view the imported training dataset.\n"));
      }
   MosPrintf(MIL_TEXT("Press <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if((KeyVal == 'v' || KeyVal == 'V') && !SkipTrain)
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE IMPORTED TRAINING DATASET...\n"));
      MosPrintf(MIL_TEXT("*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(MilSystem, Dataset, true);
      }
   }

//==============================================================================
void PredictUsingTrainedContext(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID TrainedCtx,
   MIL_ID TestDataset)
   {
   MclassPreprocess(TrainedCtx, M_DEFAULT);

   MclassPredict(TrainedCtx, TestDataset, TestDataset, M_DEFAULT);

   MIL_INT NumEntries {0};
   MclassInquire(TestDataset, M_DEFAULT, M_NUMBER_OF_ENTRIES + M_TYPE_MIL_INT, &NumEntries);

   MosPrintf(MIL_TEXT("\nPredictions will be performed on the test dataset as a final check\nof the trained object detection classifier.\n"));
   MosPrintf(MIL_TEXT("The test dataset contains %d images.\n"), NumEntries);
   MosPrintf(MIL_TEXT("The prediction results will be shown for the all %d images.\n\n"), NumEntries);

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("VIEWING THE PREDICTED TEST DATASET...\n"));
   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   CDatasetViewer(MilSystem, TestDataset, false);

   MosPrintf(MIL_TEXT("Press <Enter> to end...\n"));
   MosGetch();
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
void SetAugmentationControls(MIL_ID TrainCtx, bool* pIsDevDataset)
   {
   MIL_ID PrepareDataCtx {M_NULL};
   MclassInquire(TrainCtx, M_DEFAULT, M_PREPARE_DATA_CONTEXT_ID + M_TYPE_MIL_ID, &PrepareDataCtx);

   // Reproducibility.
   MclassControl(PrepareDataCtx, M_DEFAULT, M_SEED_MODE, M_USER_DEFINED);
   MclassControl(PrepareDataCtx, M_DEFAULT, M_SEED_VALUE, 16);

   // Number of augmentations.
   MclassControl(PrepareDataCtx, M_DEFAULT, M_AUGMENT_NUMBER_MODE, M_FACTOR);
   MclassControl(PrepareDataCtx, M_DEFAULT, M_AUGMENT_NUMBER_FACTOR, 9.0);
   MclassControl(PrepareDataCtx, M_DEFAULT, M_AUGMENT_BALANCING, 0.0);

   // Presets.
   MclassControl(PrepareDataCtx, M_DEFAULT, M_PRESET_TRANSLATION, M_ENABLE);
   MclassControl(PrepareDataCtx, M_DEFAULT, M_PRESET_ROTATION, M_ENABLE);
   MclassControl(PrepareDataCtx, M_DEFAULT, M_PRESET_FLIP, M_ENABLE);

   MIL_ID AugmentContext {M_NULL};
   MclassInquire(PrepareDataCtx, M_DEFAULT, M_AUGMENT_CONTEXT_ID + M_TYPE_MIL_ID, &AugmentContext);

   // Chosen probability to achieve on average 1.75 of the following augmentations 
   MIL_INT Probability = 35;

   MimControl(AugmentContext, M_AUG_HUE_OFFSET_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_HUE_OFFSET_OP + M_PROBABILITY, Probability);
   MimControl(AugmentContext, M_AUG_HUE_OFFSET_OP_MAX, 360);
   MimControl(AugmentContext, M_AUG_HUE_OFFSET_OP_MIN, 0);

   MimControl(AugmentContext, M_AUG_LIGHTING_DIRECTIONAL_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_LIGHTING_DIRECTIONAL_OP + M_PROBABILITY, Probability);
   MimControl(AugmentContext, M_AUG_LIGHTING_DIRECTIONAL_OP_INTENSITY_MAX, 1.2);
   MimControl(AugmentContext, M_AUG_LIGHTING_DIRECTIONAL_OP_INTENSITY_MIN, 0.8);

   MimControl(AugmentContext, M_AUG_INTENSITY_ADD_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_INTENSITY_ADD_OP + M_PROBABILITY, Probability);
   MimControl(AugmentContext, M_AUG_INTENSITY_ADD_OP_DELTA, 32);
   MimControl(AugmentContext, M_AUG_INTENSITY_ADD_OP_MODE, M_LUMINANCE);
   MimControl(AugmentContext, M_AUG_INTENSITY_ADD_OP_VALUE, 0);

   MimControl(AugmentContext, M_AUG_SATURATION_GAIN_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_SATURATION_GAIN_OP + M_PROBABILITY, Probability);
   MimControl(AugmentContext, M_AUG_SATURATION_GAIN_OP_MAX, 1.5);
   MimControl(AugmentContext, M_AUG_SATURATION_GAIN_OP_MIN, 0.75);

   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP, M_ENABLE);
   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP + M_PROBABILITY, Probability);
   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP_DISTRIBUTION, M_UNIFORM);
   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP_INTENSITY_MIN, 0);
   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP_STDDEV, 0.1);
   MimControl(AugmentContext, M_AUG_NOISE_MULTIPLICATIVE_OP_STDDEV_DELTA, 0.1);

   // Hook to show augmentations' progress.
   MclassHookFunction(PrepareDataCtx, M_PREPARE_ENTRY_POST, HookNumPreparedEntriesFunc, pIsDevDataset);
   }

//==============================================================================
void SetTrainControls(MIL_ID TrainCtx)
   {
   // Delete and create training directory.
   MIL_INT TrainFolderExists {M_FALSE};
   MappFileOperation(M_DEFAULT, EXAMPLE_TRAIN_DESTINATION_PATH, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &TrainFolderExists);
   if(TrainFolderExists == M_TRUE)
      {
      MappFileOperation(M_DEFAULT, EXAMPLE_TRAIN_DESTINATION_PATH, M_NULL, M_NULL, M_FILE_DELETE_DIR, M_RECURSIVE, M_NULL);
      }
   MappFileOperation(M_DEFAULT, EXAMPLE_TRAIN_DESTINATION_PATH, M_NULL, M_NULL, M_FILE_MAKE_DIR, M_DEFAULT, M_NULL);

   MclassControl(TrainCtx, M_DEFAULT, M_TRAIN_DESTINATION_FOLDER, EXAMPLE_TRAIN_DESTINATION_PATH);

   MclassControl(TrainCtx, M_DEFAULT, M_MAX_EPOCH, 20);
   MclassControl(TrainCtx, M_DEFAULT, M_MINI_BATCH_SIZE, 4);
   MclassControl(TrainCtx, M_DEFAULT, M_INITIAL_LEARNING_RATE, 0.001);
   MclassControl(TrainCtx, M_DEFAULT, M_LEARNING_RATE_DECAY, 0.1);

   MclassControl(TrainCtx, M_DEFAULT, M_SPLIT_SEED_MODE, M_FIXED);
   MclassControl(TrainCtx, M_DEFAULT, M_SPLIT_PERCENTAGE, 80.0);
   }

//==============================================================================
MIL_UNIQUE_CLASS_ID TrainTheModel(
   MIL_ID MilSystem,
   MIL_ID MilDisplay,
   MIL_ID Dataset)
   {
   MIL_UNIQUE_CLASS_ID TrainCtx = MclassAlloc(MilSystem, M_TRAIN_DET, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_CLASS_ID TrainRslt = MclassAllocResult(MilSystem, M_TRAIN_DET_RESULT, M_DEFAULT, M_UNIQUE_ID);

   bool IsDevDataset = false;
   SetAugmentationControls(TrainCtx, &IsDevDataset);

   SetTrainControls(TrainCtx);

   MclassPreprocess(TrainCtx, M_DEFAULT);

   MIL_INT TrainEngineUsed {M_NULL};
   MclassInquire(TrainCtx, M_DEFAULT, M_TRAIN_ENGINE_USED + M_TYPE_MIL_INT, &TrainEngineUsed);

   if(TrainEngineUsed == M_GPU)
      {
      MIL_INT GpuTrainEngineStatus {M_NULL};
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

   MIL_STRING TrainEngineDescription {};
   MclassInquire(TrainCtx, M_DEFAULT, M_TRAIN_ENGINE_USED_DESCRIPTION, TrainEngineDescription);

   MIL_INT ImgSizeX {0};
   MIL_INT ImgSizeY {0};
   GetImageSizes(MilSystem, Dataset, &ImgSizeX, &ImgSizeY);

   CTrainEvolutionDashboard TheTrainEvolutionDashboard(
      MilSystem,
      TrainCtx,
      ImgSizeX,
      ImgSizeY,
      TrainEngineUsed,
      TrainEngineDescription);

   SHookEpochData EpochData {};
   EpochData.m_pTheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_EPOCH_TRAINED, HookEpochFunc, &EpochData);

   SHookMiniBatchData MiniBatchData {};
   MiniBatchData.m_pTheDashboard = &TheTrainEvolutionDashboard;
   MclassHookFunction(TrainCtx, M_MINI_BATCH_TRAINED, HookMiniBatchFunc, &MiniBatchData);

   SHookDatasetsPrepared DatasetsPreparedData {};
   DatasetsPreparedData.m_DashboardId = TheTrainEvolutionDashboard.GetDashboardBufId();
   DatasetsPreparedData.m_MilDisplay = MilDisplay;
   MclassHookFunction(TrainCtx, M_DATASETS_PREPARED, HookDatasetsPreparedFunc, &DatasetsPreparedData);

   MosPrintf(MIL_TEXT("Augmenting the datasets before training.\n"));
   MclassTrain(TrainCtx, M_NULL, Dataset, M_NULL, TrainRslt, M_DEFAULT);

   MIL_UNIQUE_CLASS_ID TrainedCtx {};

   MIL_INT Status {-1};
   MclassGetResult(TrainRslt, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);

   if(Status == M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("\nTraining completed successfully!\n"));

      TrainedCtx = MclassAlloc(MilSystem, M_CLASSIFIER_DET_PREDEFINED, M_DEFAULT, M_UNIQUE_ID);
      MclassCopyResult(TrainRslt, M_DEFAULT, TrainedCtx, M_DEFAULT, M_TRAINED_CLASSIFIER, M_DEFAULT);

      const MIL_STRING SaveCtxName = MIL_TEXT("PlywoodODNet.mclass");
      MclassSave(SaveCtxName, TrainedCtx, M_DEFAULT);
      MosPrintf(MIL_TEXT("\nThe trained context was saved: \"%s\".\n"), SaveCtxName.c_str());

      MosPrintf(MIL_TEXT("\nA training report was saved: \"TrainReport.csv\".\n"));
      MclassExport(MIL_TEXT("TrainReport.csv"), M_FORMAT_TXT, TrainRslt, M_DEFAULT, M_TRAIN_REPORT, M_DEFAULT);

      std::vector<MIL_DOUBLE> DevLosses {};
      MclassGetResult(TrainRslt, M_DEFAULT, M_DEV_DATASET_EPOCH_LOSS, DevLosses);

      MIL_INT LastUpdatedEpochIndex {0};
      MclassGetResult(TrainRslt, M_DEFAULT, M_LAST_EPOCH_UPDATED_PARAMETERS + M_TYPE_MIL_INT, &LastUpdatedEpochIndex);

      MosPrintf(MIL_TEXT("\nThe best epoch is considered to be the epoch with the lowest dev loss.\n"));
      MosPrintf(MIL_TEXT("\nThe best epoch was epoch %d with loss on the dev dataset of %.8lf.\n"), LastUpdatedEpochIndex, DevLosses[LastUpdatedEpochIndex]);

      MosPrintf(MIL_TEXT("\nPress <Enter> to continue...\n"));
      MosGetch();
      }
   else if(Status == M_STOPPED_BY_REQUEST)
      {
      MosPrintf(MIL_TEXT("\nThe training was stopped so we have restored a pre-trained context to predict with.\n"));
      TrainedCtx = MclassRestore(EXAMPLE_PRETRAINED_PATH, MilSystem, M_DEFAULT, M_UNIQUE_ID);

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
MIL_INT MFTYPE HookDatasetsPreparedFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData)
   {
   SHookDatasetsPrepared* pHookData = reinterpret_cast<SHookDatasetsPrepared*>(pUserData);

   MIL_ID TrainRslt {M_NULL};
   MclassGetHookInfo(EventId, M_RESULT_ID + M_TYPE_MIL_ID, &TrainRslt);

   MIL_ID MilSystem {M_NULL};
   MclassInquire(TrainRslt, M_DEFAULT, M_OWNER_SYSTEM + M_TYPE_MIL_ID, &MilSystem);

   MIL_UNIQUE_CLASS_ID PrpTrainDataset = MclassAlloc(MilSystem, M_DATASET_IMAGES, M_DEFAULT, M_UNIQUE_ID);
   MclassCopyResult(TrainRslt, M_DEFAULT, PrpTrainDataset, M_DEFAULT, M_PREPARED_TRAIN_DATASET, M_DEFAULT);

   MosPrintf(MIL_TEXT("Press <v> to view the augmented train dataset.\nPress <Enter> to continue...\n"));

   char KeyVal = (char)MosGetch();
   if(KeyVal == 'v' || KeyVal == 'V')
      {
      MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
      MosPrintf(MIL_TEXT("VIEWING THE AUGMENTED TRAIN DATASET..."));
      MosPrintf(MIL_TEXT("\n*******************************************************\n\n"));
      CDatasetViewer DatasetViewer(MilSystem, PrpTrainDataset, true);
      }

   MosPrintf(MIL_TEXT("\nThe training has started.\n"));
   MosPrintf(MIL_TEXT("It can be paused at any time by pressing 'p'.\n"));
   MosPrintf(MIL_TEXT("It can then be stopped or continued.\n"));

   MosPrintf(MIL_TEXT("\nDuring training, you can observe the evolution of the losses\n"));
   MosPrintf(MIL_TEXT("of the train and dev datasets together.\n"));
   MosPrintf(MIL_TEXT("The best epoch is determined by the epoch with the smallest dev loss.\n"));

   MdispSelect(pHookData->m_MilDisplay, pHookData->m_DashboardId);

   return M_NULL;
   }

//==============================================================================
MIL_INT MFTYPE HookEpochFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData)
   {
   SHookEpochData* pHookData = reinterpret_cast<SHookEpochData*>(pUserData);

   MIL_DOUBLE CurBench = 0.0;
   MIL_DOUBLE CurBenchMean = -1.0;

   MIL_INT CurEpochIndex = 0;
   MclassGetHookInfo(EventId, M_EPOCH_INDEX + M_TYPE_MIL_INT, &CurEpochIndex);

   MappTimer(M_DEFAULT, M_TIMER_READ, &CurBench);
   const MIL_DOUBLE EpochBenchMean = CurBench / (CurEpochIndex + 1);

   MIL_DOUBLE DevLoss = 0;
   MclassGetHookInfo(EventId, M_DEV_DATASET_LOSS, &DevLoss);

   pHookData->m_pTheDashboard->AddEpochData(DevLoss, CurEpochIndex, EpochBenchMean);

   return M_NULL;
   }

//==============================================================================
MIL_INT MFTYPE HookMiniBatchFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData)
   {
   SHookMiniBatchData* pHookData = reinterpret_cast<SHookMiniBatchData*>(pUserData);

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

   pHookData->m_pTheDashboard->AddMiniBatchData(Loss, MiniBatchIdx, EpochIdx, NbMiniBatchPerEpoch);

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
MIL_INT MFTYPE HookNumPreparedEntriesFunc(MIL_INT /*HookType*/, MIL_ID EventId, void* pUserData)
   {
   bool* pIsDevDataset = reinterpret_cast<bool*>(pUserData);

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
