/*************************************************************************/
/*
* File name: FeatureChangeHook.cpp
*
* Synopsis:  This example shows how to use MIL in order to hook a MIL
*            callback function to GenICam(tm) feature change events.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <vector>

/******************************************************************************/
/* Example description.                                                       */
/******************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n\n"));
   MosPrintf(MIL_TEXT("FeatureChangeHook\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n\n"));
   MosPrintf(
      MIL_TEXT("This example shows how to hook a MIL callback function to GenICam feature change\n")\
      MIL_TEXT("events. Press <Enter> to start\n\n")\
   );
   }

typedef struct _HookDataStruct
   {
   _HookDataStruct()
      {
      FeatureChangeNotificationCount = 0;
      }

   MIL_INT FeatureChangeNotificationCount;
   } HookDataStruct;

using namespace std;
/* Feature names to hook to. */
vector<MIL_STRING> FeatureNames;
vector<MIL_STRING> InitFeatureNames(MIL_ID MilDigitizer);


/* Verifies if this example can run on the selected system. */
bool SystemSupportsGenICam(MIL_ID MilSystem);

/* Routine used to register a MIL hook to feature changes. */
void TestFeatureChangeHook(MIL_ID MilDigitizer, MIL_INT Mode, HookDataStruct& HookData);

/* MIL hook function called when a feature's value or property changes. */
MIL_INT MFTYPE FeatureChangeFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

/* Main function. */
int MosMain(void)
   {
   MIL_ID   MilApplication;
   MIL_ID   MilSystem     ;
   MIL_ID   MilDigitizer  ;
   HookDataStruct HookData;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, 
      &MilDigitizer, M_NULL);

   if (!SystemSupportsGenICam(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);
      return 1;
      }

   PrintHeader();
   MosGetch();

   FeatureNames = InitFeatureNames(MilDigitizer);
   if (FeatureNames.size() == 0)
      {
      MosPrintf(MIL_TEXT("This example program can only be used with devices that support specific\n"));
      MosPrintf(MIL_TEXT("feature names.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);
      return 1;
      }

   /* Routine used to hook a function to and to invalidate a feature.*/
   /* This will trigger feature change hooks only on selected features. */
   TestFeatureChangeHook(MilDigitizer, M_DEFAULT, HookData);

   /* Routine used to hook a function to and to invalidate a feature.*/
   /* This will trigger feature change hooks on all features. */
   TestFeatureChangeHook(MilDigitizer, M_ALL, HookData);

   if (HookData.FeatureChangeNotificationCount == 0)
      {
      MosPrintf(MIL_TEXT("Did not detect any feature changes\n\n"));
      }

   MosPrintf(MIL_TEXT("Press <Enter> to quit.\n"));
   MosGetch();

   MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);

   return 0;
   }

/* Verifies if this example can run on the selected system. */
bool SystemSupportsGenICam(MIL_ID MilSystem)
   {
   MIL_INT GenICamSupport = M_FALSE;

   MsysInquire(MilSystem, M_GENICAM_AVAILABLE, &GenICamSupport);
   if (GenICamSupport == M_FALSE)
      {
      MosPrintf(MIL_TEXT("This example program can only be used with the Matrox Driver for ")
                MIL_TEXT("GenICam.\n"));
      MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
                MIL_TEXT("MIL Config.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();
      }

   return GenICamSupport == M_TRUE;
   }

/* Routine used to hook a function to and to invalidate a feature.*/
void TestFeatureChangeHook(MIL_ID MilDigitizer, MIL_INT Mode, HookDataStruct& HookData)
   {
   std::vector<MIL_INT64> Value(FeatureNames.size(), 0), MinValue(FeatureNames.size(), 0);

   /* To test feature change notification we must modify a feature; lets use the first feature  */
   /* in the FeatureNames array. First backup inquire the feature's minimum and current values. */
   /* Note: depending on the device's XML description file, writing to a single feature.        */
   /*       can trigger multiple feature change events.                                         */
   for (size_t i = 0; i < FeatureNames.size(); i++)
      {
      MdigInquireFeature(MilDigitizer, M_FEATURE_MIN, FeatureNames[i], M_TYPE_INT64, &MinValue[i]);
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureNames[i], M_TYPE_INT64, &Value[i]);
      }

   if (Mode == M_ALL)
      {
      MosPrintf(MIL_TEXT("Hooking to all features.\n"));

      /* Hook a MIL function to all GenICam features. */
      MdigHookFunction(MilDigitizer, M_FEATURE_CHANGE + M_ALL, FeatureChangeFunction, &HookData);
      }
   else
      {
      /* Iterate through all features in the FeatureNames array and indicate that we want to hook to these */
      /* features.                                                                                         */
      MIL_INT Control = M_ENABLE;
      for (size_t i = 0; i < FeatureNames.size(); i++)
         {
         MosPrintf(MIL_TEXT("Hooking to \"%s\" feature.\n"), FeatureNames[i].c_str());
         MdigControlFeature(MilDigitizer, M_FEATURE_CHANGE_HOOK, FeatureNames[i], M_TYPE_MIL_INT, &Control);
         }

      /* Hook a MIL function to GenICam feature change */
      MdigHookFunction(MilDigitizer, M_FEATURE_CHANGE, FeatureChangeFunction, &HookData);
      }

   MosPrintf(MIL_TEXT("\nPress <Enter> to trigger feature change notification.\n\n"));
   MosGetch();

   for (size_t i = 0; i < FeatureNames.size(); i++)
      {
      MosPrintf(MIL_TEXT("Writing \"%lld\" to the \"%s\" feature to trigger feature change notification.\n\n"),
         (long long)MinValue[i], FeatureNames[i].c_str());

      /* Change the feature's value.                                                            */
      /* The callback will get called and information related to the features that changed will */
      /* be printed.                                                                            */
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, FeatureNames[i], M_TYPE_INT64, &MinValue[i]);
      }

   if (Mode == M_ALL)
      {
      /* Unhook from GenICam feature change */
      MdigHookFunction(MilDigitizer, M_FEATURE_CHANGE + M_ALL + M_UNHOOK, FeatureChangeFunction, &HookData);
      }
   else
      {
      MIL_INT Control = M_DISABLE;
      /* Unhook from GenICam feature change */
      MdigHookFunction(MilDigitizer, M_FEATURE_CHANGE + M_UNHOOK, FeatureChangeFunction, &HookData);

      /* Disable feature change hooks on the feature we previously specified. */
      for (size_t i = 0; i < FeatureNames.size(); i++)
         MdigControlFeature(MilDigitizer, M_FEATURE_CHANGE_HOOK, FeatureNames[i], M_TYPE_MIL_INT, &Control);
      }

   for (size_t i = 0; i < FeatureNames.size(); i++)
      {
      /* Restore the feature's original value. */
      MdigControlFeature(MilDigitizer, M_FEATURE_VALUE, FeatureNames[i], M_TYPE_INT64, &Value[i]);
      }
   }

/* MIL hook function called when a feature's value or property changes. */
MIL_INT MFTYPE FeatureChangeFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct* HookDataStructPtr = (HookDataStruct*)HookDataPtr;
   MIL_ID MilDigitizer = M_NULL;
   MIL_STRING FeatureName;

   /* Inquire the MIL Digitizer ID associated to the GenICam device. */
   MdigGetHookInfo(HookId, M_DIGITIZER_ID, &MilDigitizer);

   MosPrintf(MIL_TEXT("***** Received feature change notification *****\n"));
   HookDataStructPtr->FeatureChangeNotificationCount++;

   if (MilDigitizer != M_NULL)
      {
      MIL_INT64 Mode = 0;
         
      /* Inquire the feature name that triggered this hook. */
      MdigGetHookInfo(HookId, M_GC_FEATURE_CHANGE_NAME, FeatureName);

      /* Determine the feature's access mode to see if we can read its value. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_ACCESS_MODE, FeatureName, M_TYPE_INT64, &Mode);
      if (M_FEATURE_IS_READABLE(Mode))
         {
         /* Feature is readable, read its value. */
         MIL_STRING Value;
         MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, Value);
         MosPrintf(MIL_TEXT("\"%s\" feature value: \"%s\".\n\n"), FeatureName.c_str(), Value.c_str());
         }
      else
         MosPrintf(MIL_TEXT("\"%s\" feature.\n\n"), FeatureName.c_str());
      }

   return 0;
   }

/* Validate that features used by the example actually exist.. */
vector<MIL_STRING> InitFeatureNames(MIL_ID MilDigitizer)
   {
   vector<MIL_STRING> Names;
   MIL_BOOL FeaturePresent = M_FALSE;
   MIL_INT64 FeatureAccessMode = 0;

   MdigInquireFeature(MilDigitizer, M_FEATURE_PRESENT, MIL_TEXT("Width"), M_TYPE_BOOLEAN, &FeaturePresent);
   if (FeaturePresent)
      {
      /* Inquire the feature's access mode. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_ACCESS_MODE, MIL_TEXT("Width"), M_TYPE_INT64, &FeatureAccessMode);
      if (M_FEATURE_IS_WRITABLE(FeatureAccessMode))
         {
         Names.push_back(MIL_TEXT("Width"));
         }
      }
   MdigInquireFeature(MilDigitizer, M_FEATURE_PRESENT, MIL_TEXT("Height"), M_TYPE_BOOLEAN, &FeaturePresent);
   if (FeaturePresent)
      {
      /* Inquire the feature's access mode. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_ACCESS_MODE, MIL_TEXT("Height"), M_TYPE_INT64, &FeatureAccessMode);
      if (M_FEATURE_IS_WRITABLE(FeatureAccessMode))
         {
         Names.push_back(MIL_TEXT("Height"));
         }
      }

   return Names;
   }
