//*************************************************************************************
//
// File name: PredictEngineSelection.cpp
//
// Synopsis:  This program goes through all available predict engines on the
//            current machine to compare their performance.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "mil.h"
#include <vector>
#include <fstream>
#include <iterator>
#include <set>
#include <algorithm>

// ===========================================================================
// Example description.
// ===========================================================================
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("PredictEngineSelection\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example will compare the benches of all available predict engines on \nthe machine.\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, buffer, classification.\n\n"));
   }

#define CLASSIFIER_FOLDER M_IMAGE_PATH MIL_TEXT("Classification/PredictEngineSelection/")

MIL_STRING NETWORK = MIL_TEXT("ICNET");
MIL_STRING NETWORK_SIZE = MIL_TEXT("_M");
MIL_INT NUMBER_OF_BANDS = 3;
MIL_INT SIZE_X = 640;
MIL_INT SIZE_Y = 480;
MIL_INT NUMBER_OF_PREDICTIONS = 100;

void EvaluatePredictEngines(MIL_ID MilApplication, MIL_ID Classifier, MIL_ID PredictResult, MIL_ID TestBuffer);
MIL_STRING SelectClassifierAndImageSize();

// ****************************************************************************
//    Main.
// ****************************************************************************
int MosMain()
   {
   PrintHeader();

   MIL_UNIQUE_APP_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_SYS_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_UNIQUE_ID);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MIL_STRING ClassifierPath = SelectClassifierAndImageSize();

   MosPrintf(MIL_TEXT("Beginning predict engine benchmarking...\n"));

   MIL_UNIQUE_CLASS_ID Classifier = MclassRestore(ClassifierPath, MilSystem, M_DEFAULT, M_UNIQUE_ID);

   MIL_INT ResultType {};
   if(NETWORK == MIL_TEXT("CSNET"))
      {
      ResultType = M_PREDICT_SEG_RESULT;
      }
   else if(NETWORK == MIL_TEXT("ICNET"))
      {
      ResultType = M_PREDICT_CNN_RESULT;
      }
   else
      {
      ResultType = M_PREDICT_DET_RESULT;
      }
   MIL_UNIQUE_CLASS_ID PredictResult = MclassAllocResult(MilSystem, ResultType, M_DEFAULT, M_UNIQUE_ID);
   MIL_UNIQUE_BUF_ID TestBuffer = MbufAllocColor(MilSystem, NUMBER_OF_BANDS, SIZE_X, SIZE_Y, 8 + M_UNSIGNED, M_IMAGE + M_PROC, M_UNIQUE_ID);

   EvaluatePredictEngines(MilApplication, Classifier, PredictResult, TestBuffer);

   return 0;
   }

MIL_STRING SelectClassifierAndImageSize()
   {
   MosPrintf(MIL_TEXT("Please select a desired network architecture.\n"));
   MosPrintf(MIL_TEXT("1) M_ICNET_S\n"));
   MosPrintf(MIL_TEXT("2) M_ICNET_M (default)\n"));
   MosPrintf(MIL_TEXT("3) M_ICNET_XL\n"));
   MosPrintf(MIL_TEXT("4) M_CSNET_S\n"));
   MosPrintf(MIL_TEXT("5) M_CSNET_M\n"));
   MosPrintf(MIL_TEXT("6) M_CSNET_XL\n"));
   MosPrintf(MIL_TEXT("7) M_ODNET\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue with the default.\n\n"));

   MIL_INT NbSizesAvailable {};

   char KeyVal = (char)MosGetch();
   switch(KeyVal)
      {
      default:
      case '2':
         NbSizesAvailable = 4;
         NETWORK_SIZE = MIL_TEXT("_M");
         NETWORK = MIL_TEXT("ICNET");
         break;
      case '1':
         NbSizesAvailable = 4;
         NETWORK_SIZE = MIL_TEXT("_S");
         NETWORK = MIL_TEXT("ICNET");
         break;
      case '3':
         NbSizesAvailable = 2;
         NETWORK_SIZE = MIL_TEXT("_XL");
         NETWORK = MIL_TEXT("ICNET");
         break;
      case '4':
         NbSizesAvailable = 6;
         NETWORK_SIZE = MIL_TEXT("_S");
         NETWORK = MIL_TEXT("CSNET");
         break;
      case '5':
         NbSizesAvailable = 6;
         NETWORK_SIZE = MIL_TEXT("_M");
         NETWORK = MIL_TEXT("CSNET");
         break;
      case '6':
         NbSizesAvailable = 6;
         NETWORK_SIZE = MIL_TEXT("_XL");
         NETWORK = MIL_TEXT("CSNET");
         break;
      case '7':
         NbSizesAvailable = 4;
         NETWORK_SIZE = MIL_TEXT("");
         NETWORK = MIL_TEXT("ODNET");
         break;
      }

   MosPrintf(MIL_TEXT("M_%s%s selected.\n\n"), NETWORK.c_str(), NETWORK_SIZE.c_str());

   MosPrintf(MIL_TEXT("Please select a desired target image number of bands.\n"));
   MosPrintf(MIL_TEXT("1) 1 Band\n"));
   MosPrintf(MIL_TEXT("2) 3 Bands (default)\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue with the default.\n\n"));
   KeyVal = (char)MosGetch();
   if(KeyVal == '1')
      {
      NUMBER_OF_BANDS = 1;
      }

   MosPrintf(MIL_TEXT("%d band selected.\n\n"), NUMBER_OF_BANDS);

   MosPrintf(MIL_TEXT("Please select a desired target image size.\n"));
   MosPrintf(MIL_TEXT("1) 128x96 (MMS Small)\n"));
   MosPrintf(MIL_TEXT("2) 640x480 (SD) (default)\n"));
   if(NbSizesAvailable > 2)
      {
      MosPrintf(MIL_TEXT("3) 1920x1080 (HD)\n"));
      }
   if(NbSizesAvailable > 3)
      {
      MosPrintf(MIL_TEXT("4) 3840x2160 (4K)\n"));
      }
   if(NbSizesAvailable > 4)
      {
      MosPrintf(MIL_TEXT("5) 7680x4320 (8K)\n"));
      }
   if(NbSizesAvailable > 5)
      {
      MosPrintf(MIL_TEXT("6) Custom\n"));
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to continue with the default.\n\n"));

   KeyVal = (char)MosGetch();
   if(KeyVal == '1')
      {
      SIZE_X = 128;
      SIZE_Y = 96;
      NUMBER_OF_PREDICTIONS = 1000;
      }
   else if(KeyVal == '3' && NbSizesAvailable > 2)
      {
      SIZE_X = 1920;
      SIZE_Y = 1080;
      NUMBER_OF_PREDICTIONS = 35;
      }
   else if(KeyVal == '4' && NbSizesAvailable > 3)
      {
      SIZE_X = 3840;
      SIZE_Y = 2160;
      NUMBER_OF_PREDICTIONS = 15;
      }
   else if(KeyVal == '5' && NbSizesAvailable > 4)
      {
      SIZE_X = 7680;
      SIZE_Y = 4320;
      NUMBER_OF_PREDICTIONS = 5;
      }
   else if(KeyVal == '6' && NbSizesAvailable > 5)
      {
      MosPrintf(MIL_TEXT("Please enter your desired size x:\n"));
      MOs_scanf_s(MIL_TEXT("%d"), (int*) &SIZE_X);
      MosPrintf(MIL_TEXT("Please enter your desired size y:\n"));
      MOs_scanf_s(MIL_TEXT("%d"), (int*) &SIZE_Y);
      NUMBER_OF_PREDICTIONS = 100;
      }

   MosPrintf(MIL_TEXT("%dx%d image size selected.\n\n"), SIZE_X, SIZE_Y);

   if(NETWORK == MIL_TEXT("CSNET"))
      {
      return CLASSIFIER_FOLDER + NETWORK + NETWORK_SIZE + MIL_TEXT("_") +
         M_TO_STRING(NUMBER_OF_BANDS) + MIL_TEXT("BAND") + MIL_TEXT(".mclass");
      }

   return CLASSIFIER_FOLDER + NETWORK + NETWORK_SIZE + MIL_TEXT("_") +
      M_TO_STRING(NUMBER_OF_BANDS) + MIL_TEXT("x") + M_TO_STRING(SIZE_X) + MIL_TEXT("x") + M_TO_STRING(SIZE_Y) + MIL_TEXT(".mclass");
   }

void GetPredictEngineInfo(MIL_ID Classifier, MIL_INT PredictEngineIndex, MIL_STRING* pProviderString, MIL_STRING* pDescription, MIL_STRING* pPrecisionString)
   {
   MIL_INT Provider = 0;
   MIL_INT Precision = 0;
   MclassInquire(Classifier, M_PREDICT_ENGINE_INDEX(PredictEngineIndex), M_PREDICT_ENGINE_PROVIDER + M_TYPE_MIL_INT, &Provider);
   MclassInquire(Classifier, M_PREDICT_ENGINE_INDEX(PredictEngineIndex), M_PREDICT_ENGINE_DESCRIPTION, *pDescription);
   MclassInquire(Classifier, M_PREDICT_ENGINE_INDEX(PredictEngineIndex), M_PREDICT_ENGINE_PRECISION + M_TYPE_MIL_INT, &Precision);

   switch(Provider)
      {
      default:
      case M_DEFAULT_CPU:
         *pProviderString = MIL_TEXT("DefaultCPU");
         break;
      case M_OPENVINO:
         *pProviderString = MIL_TEXT("OpenVINO");
         break;
      case M_CUDA:
         *pProviderString = MIL_TEXT("CUDA");
         break;
      }

   switch(Precision)
      {
      default:
      case M_FP32:
         *pPrecisionString = MIL_TEXT("FP32");
         break;
      case M_FP16:
         *pPrecisionString = MIL_TEXT("FP16");
         break;
      }
   }

struct SPredictEngine
   {
   SPredictEngine(MIL_INT PredictEngineIndex, MIL_STRING ProviderString, MIL_STRING Description,
                  MIL_STRING PrecisionString, MIL_INT NbCores, MIL_DOUBLE AveragePredictTime):
      m_PredictEngineIndex(PredictEngineIndex), m_ProviderString(ProviderString), m_Description(Description),
      m_PrecisionString(PrecisionString), m_NbCores(NbCores), m_AveragePredictTime(AveragePredictTime)
      {};

   void PrintInfo() const;

   MIL_INT m_PredictEngineIndex {};
   MIL_STRING m_ProviderString {};
   MIL_STRING m_Description {};
   MIL_STRING m_PrecisionString {};
   MIL_INT m_NbCores {};
   MIL_DOUBLE m_AveragePredictTime {};
   };

void SPredictEngine::PrintInfo() const
   {
   MosPrintf(MIL_TEXT("%3d|"), m_PredictEngineIndex);
   MosPrintf(MIL_TEXT("%10.10s|"), m_ProviderString.c_str());
   MosPrintf(MIL_TEXT("%42.42s|"), m_Description.c_str());
   MosPrintf(MIL_TEXT("%4.4s|"), m_PrecisionString.c_str());
   MosPrintf(MIL_TEXT("%5d|"), m_NbCores);
   if(m_AveragePredictTime != -1.0)
      {
      MosPrintf(MIL_TEXT("%9.3f"), m_AveragePredictTime);
      }
   MosPrintf(MIL_TEXT("\n"));
   }

std::set<MIL_INT> GetNbCoreSet()
   {
   std::set<MIL_INT> CoreSet {};

   MthrControlMp(M_DEFAULT, M_CORE_SHARING, M_DEFAULT, M_ENABLE, M_NULL);

   MIL_INT NbCoresAvailable = 0;
   MthrInquireMp(M_DEFAULT, M_CORE_NUM_EFFECTIVE, M_DEFAULT, M_DEFAULT, &NbCoresAvailable);

   CoreSet.insert(NbCoresAvailable);

   MthrControlMp(M_DEFAULT, M_CORE_SHARING, M_DEFAULT, M_DISABLE, M_NULL);

   MthrInquireMp(M_DEFAULT, M_CORE_NUM_EFFECTIVE, M_DEFAULT, M_DEFAULT, &NbCoresAvailable);

   CoreSet.insert(NbCoresAvailable);

   if(NbCoresAvailable > 1)
      {
      CoreSet.insert(NbCoresAvailable - 1);
      }

   int TestCores = 1;
   while(TestCores < NbCoresAvailable)
      {
      CoreSet.insert(TestCores);
      TestCores = TestCores << 1;
      }

   MthrControlMp(M_DEFAULT, M_CORE_SHARING, M_DEFAULT, M_ENABLE, M_NULL);

   return CoreSet;
   }

template<typename Cmp>
bool CompAveragePredictTime(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   return Cmp()(Engine1.m_AveragePredictTime, Engine2.m_AveragePredictTime);
   }

template<typename Cmp>
bool CompIndex(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   if(Engine1.m_PredictEngineIndex != Engine2.m_PredictEngineIndex)
      {
      return Cmp()(Engine1.m_PredictEngineIndex, Engine2.m_PredictEngineIndex);
      }
   return CompAveragePredictTime<std::less<MIL_DOUBLE>>(Engine1, Engine2);
   }

template<typename Cmp>
bool CompProvider(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   if(Engine1.m_ProviderString != Engine2.m_ProviderString)
      {
      return Cmp()(Engine1.m_ProviderString, Engine2.m_ProviderString);
      }
   return CompAveragePredictTime<std::less<MIL_DOUBLE>>(Engine1, Engine2);
   }

template<typename Cmp>
bool CompDescription(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   if(Engine1.m_Description != Engine2.m_Description)
      {
      return Cmp()(Engine1.m_Description, Engine2.m_Description);
      }
   return CompAveragePredictTime<std::less<MIL_DOUBLE>>(Engine1, Engine2);
   }

template<typename Cmp>
bool CompPrecision(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   if(Engine1.m_PrecisionString != Engine2.m_PrecisionString)
      {
      return Cmp()(Engine1.m_PrecisionString, Engine2.m_PrecisionString);
      }
   return CompAveragePredictTime<std::less<MIL_DOUBLE>>(Engine1, Engine2);
   }

template<typename Cmp>
bool CompNbCores(const SPredictEngine& Engine1, const SPredictEngine& Engine2)
   {
   if(Engine1.m_NbCores != Engine2.m_NbCores)
      {
      return Cmp()(Engine1.m_NbCores, Engine2.m_NbCores);
      }
   return CompAveragePredictTime<std::less<MIL_DOUBLE>>(Engine1, Engine2);
   }

MIL_INT GetAveragePredictTime(MIL_INT NbPredictions, MIL_ID Classifier, MIL_ID TestBuffer, MIL_ID PredictResult, MIL_DOUBLE* pAveragePredictTime)
   {
   MIL_INT Status = M_COMPLETE;
   MIL_DOUBLE Millisec = 0.0;
   for(MIL_INT i = 0; i < NbPredictions && Status == M_COMPLETE; ++i)
      {
      MappTimer(M_DEFAULT, M_TIMER_RESET, M_NULL);
      MclassPredict(Classifier, TestBuffer, PredictResult, M_DEFAULT);
      const MIL_DOUBLE PredictTime = MappTimer(M_DEFAULT, M_TIMER_READ, M_NULL);

      MclassGetResult(PredictResult, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);

      Millisec += 1000.0 * PredictTime;
      }

   if(Status == M_COMPLETE)
      {
      *pAveragePredictTime = Millisec / NbPredictions;
      }
   else
      {
      *pAveragePredictTime = -1.0;
      }

   return Status;
   }

void PrintStatus(MIL_INT Status)
   {
   if(Status != M_COMPLETE)
      {
      MosPrintf(MIL_TEXT("The average predict time for the previous configuration is unavailable.\n"));
      MosPrintf(MIL_TEXT("The status returned was: "));
      switch(Status)
         {
         default:
         case M_INTERNAL_ERROR:
            MosPrintf(MIL_TEXT("M_INTERNAL_ERROR\n"));
            break;
         case M_PREDICT_NOT_PERFORMED:
            MosPrintf(MIL_TEXT("M_PREDICT_NOT_PERFORMED\n"));
            break;
         case M_CURRENTLY_PREDICTING:
            MosPrintf(MIL_TEXT("M_CURRENTLY_PREDICTING\n"));
            break;
         case M_STOPPED_BY_REQUEST:
            MosPrintf(MIL_TEXT("M_STOPPED_BY_REQUEST\n"));
            break;
         case M_TIMEOUT_REACHED:
            MosPrintf(MIL_TEXT("M_TIMEOUT_REACHED\n"));
            break;
         case M_NOT_ENOUGH_MEMORY:
            MosPrintf(MIL_TEXT("M_NOT_ENOUGH_MEMORY\n"));
            break;
         }
      }
   }

void EvaluatePredictEngines(MIL_ID MilApplication, MIL_ID Classifier, MIL_ID PredictResult, MIL_ID TestBuffer)
   {
   if(NETWORK == MIL_TEXT("CSNET"))
      {
      MclassControl(Classifier, M_DEFAULT, M_TARGET_IMAGE_SIZE_X, SIZE_X);
      MclassControl(Classifier, M_DEFAULT, M_TARGET_IMAGE_SIZE_Y, SIZE_Y);
      }

   MIL_STRING Description {};
   MIL_STRING ProviderString {};
   MIL_STRING PrecisionString {};

   // Default predict engine defined in MILConfig
   MIL_INT DefaultProviderIndex = 0;
   MclassInquire(Classifier, M_DEFAULT, M_PREDICT_ENGINE_USED + M_TYPE_MIL_INT + M_DEFAULT, &DefaultProviderIndex);
   GetPredictEngineInfo(Classifier, DefaultProviderIndex, &ProviderString, &Description, &PrecisionString);

   MosPrintf(MIL_TEXT("\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("Default Predict Engine:\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   if(DefaultProviderIndex == M_INVALID)
      {
      MosPrintf(MIL_TEXT("\nDefault predict engine index cannot be found, \nplease select a new one in MIL Config.\n"));
      }
   else
      {
      MosPrintf(MIL_TEXT("\nIdx|Provider|                Description                 |Prec\n"));
      MosPrintf(MIL_TEXT("---+--------+--------------------------------------------+----\n"));
      MosPrintf(MIL_TEXT("%3d|"), DefaultProviderIndex);
      MosPrintf(MIL_TEXT("%8.8s|"), ProviderString.c_str());
      MosPrintf(MIL_TEXT("%44.44s|"), Description.c_str());
      MosPrintf(MIL_TEXT("%4.4s|"), PrecisionString.c_str());
      }

   MosPrintf(MIL_TEXT("\n\n*******************************************************\n"));
   MosPrintf(MIL_TEXT("Available Predict Engines:\n"));
   MosPrintf(MIL_TEXT("*******************************************************\n"));

   MosPrintf(MIL_TEXT("\nPredicting with M_%s%s on images of size: %dx%dx%d\n"), NETWORK.c_str(), NETWORK_SIZE.c_str(), NUMBER_OF_BANDS, SIZE_X, SIZE_Y);

   MIL_STRING TableHeader    = MIL_TEXT("Idx| Provider |               Description                |Prec|Cores|Time (ms)");
   MIL_STRING TableSeperator = MIL_TEXT("---+----------+------------------------------------------+----+-----+---------");
   MosPrintf(MIL_TEXT("\n%s"), TableHeader.c_str());
   MosPrintf(MIL_TEXT("\n%s\n"), TableSeperator.c_str());

   std::set<MIL_INT> NbCores = GetNbCoreSet();

   std::vector<SPredictEngine> PredictEngines {};

   MIL_INT NbPredEngines = 0;
   MclassInquire(Classifier, M_DEFAULT, M_NUMBER_OF_PREDICT_ENGINES + M_TYPE_MIL_INT, &NbPredEngines);
   for(MIL_INT PredictEngineIndex = 0; PredictEngineIndex < NbPredEngines; ++PredictEngineIndex)
      {
      GetPredictEngineInfo(Classifier, PredictEngineIndex, &ProviderString, &Description, &PrecisionString);

      for(const auto& NbCore : NbCores)
         {
         if(NbCore == 1)
            {
            MappControlMp(MilApplication, M_MP_USE, M_DEFAULT, M_DISABLE, M_NULL);
            }
         else
            {
            MappControlMp(MilApplication, M_MP_USE, M_DEFAULT, M_ENABLE, M_NULL);
            MappControlMp(MilApplication, M_CORE_MAX, M_DEFAULT, NbCore, M_NULL);
            }

         if(ProviderString == MIL_TEXT("CUDA") && NbCore != 1)
            {
            continue;
            }

         MclassControl(Classifier, M_DEFAULT, M_PREDICT_ENGINE, PredictEngineIndex);
         MclassPreprocess(Classifier, M_DEFAULT);

         MIL_INT WarmUpPredictions = NUMBER_OF_PREDICTIONS / 10;
         MIL_DOUBLE AveragePredictTime = 0;
         MIL_INT Status = GetAveragePredictTime(WarmUpPredictions, Classifier, TestBuffer, PredictResult, &AveragePredictTime);

         if(Status == M_COMPLETE)
            {
            Status = GetAveragePredictTime(NUMBER_OF_PREDICTIONS - WarmUpPredictions, Classifier, TestBuffer, PredictResult, &AveragePredictTime);
            }

         SPredictEngine PredictEngine = SPredictEngine(PredictEngineIndex, ProviderString, Description, PrecisionString, NbCore, AveragePredictTime);
         PredictEngine.PrintInfo();

         if(Status == M_COMPLETE)
            {
            PredictEngines.push_back(PredictEngine);
            }

         PrintStatus(Status);
         }
      }
   MosPrintf(MIL_TEXT("%s\n\n"), TableSeperator.c_str());
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   char KeyVal = '6';
   char OldKeyVal {};
   bool UseGreaterComparison {};

   while(KeyVal == '1' ||
         KeyVal == '2' ||
         KeyVal == '3' ||
         KeyVal == '4' ||
         KeyVal == '5' ||
         KeyVal == '6')
      {
      MosPrintf(MIL_TEXT("Predicting with M_%s%s on images of size: %dx%dx%d\n\n"), NETWORK.c_str(), NETWORK_SIZE.c_str(), NUMBER_OF_BANDS, SIZE_X, SIZE_Y);

      MosPrintf(MIL_TEXT("You can now sort the table by the desired column:\n"));
      MosPrintf(MIL_TEXT("1) Index\n"));
      MosPrintf(MIL_TEXT("2) Provider\n"));
      MosPrintf(MIL_TEXT("3) Description\n"));
      MosPrintf(MIL_TEXT("4) Precision\n"));
      MosPrintf(MIL_TEXT("5) Number of Cores\n"));
      MosPrintf(MIL_TEXT("6) Average Predict Time (default)\n\n"));

      MosPrintf(MIL_TEXT("\n%s"), TableHeader.c_str());
      MosPrintf(MIL_TEXT("\n%s\n"), TableSeperator.c_str());

      if(OldKeyVal == KeyVal)
         {
         UseGreaterComparison = !UseGreaterComparison;
         }
      else
         {
         UseGreaterComparison = false;
         }

      switch(KeyVal)
         {
         default:
         case '6':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompAveragePredictTime<std::greater<MIL_DOUBLE>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompAveragePredictTime<std::less<MIL_DOUBLE>>);
            break;
         case '1':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompIndex<std::greater<MIL_INT>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompIndex<std::less<MIL_INT>>);
            break;
         case '2':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompProvider<std::greater<MIL_STRING>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompProvider<std::less<MIL_STRING>>);
            break;
         case '3':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompDescription<std::greater<MIL_STRING>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompDescription<std::less<MIL_STRING>>);
            break;
         case '4':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompPrecision<std::greater<MIL_STRING>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompPrecision<std::less<MIL_STRING>>);
            break;
         case '5':
            if(UseGreaterComparison)
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompNbCores<std::greater<MIL_INT>>);
            else
               std::sort(PredictEngines.begin(), PredictEngines.end(), CompNbCores<std::less<MIL_INT>>);
            break;
         }

      for(const auto& PredictEngine : PredictEngines)
         {
         PredictEngine.PrintInfo();
         }

      MosPrintf(MIL_TEXT("%s\n\n"), TableSeperator.c_str());

      MosPrintf(MIL_TEXT("Press <Enter> to exit.\n\n"));

      OldKeyVal = KeyVal;
      KeyVal = (char)MosGetch();
      }
   }
