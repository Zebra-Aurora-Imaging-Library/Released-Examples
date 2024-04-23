/*************************************************************************/
/*
* File name: enumfeatures.cpp
*
* Synopsis:  This example shows how to use MIL in order to enumerate
*            all the features in your GenICam(c) compliant device.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <vector>
#include <algorithm>

using namespace std;

/* String vector containing the feature names. */
vector<MIL_STRING> FeatureList;

/* Verifies if this example can run on the selected system. */
bool SystemSupportsGenICam(MIL_ID MilSystem);

/* Routine used to retrieve various info related to a feature. */
void GetFeatureProperties(MIL_ID MilDigitizer, const MIL_STRING& FeatureName);

/* Routine used to enumerate the device's GenICam(c) node tree. */
void EnumerateGenICamNodeTree(MIL_ID MilDigitizer, const MIL_STRING& Node,
   MIL_INT RecurseCount);

/* Print everything that is Guru level or less. */
#define VISIBILITY_LEVEL       M_FEATURE_VISIBILITY_GURU

/* Helper function used to convert a feature type into its string representation. */
MIL_STRING TypeToString(MIL_INT64 Type);
MIL_STRING VisibilityToString(MIL_INT64 Visibility);
MIL_STRING RepresentationToString(MIL_INT64 Representation);
/* Main function. */
int MosMain(void)
   {
   MIL_ID   MilApplication;
   MIL_ID   MilSystem     ;
   MIL_ID   MilDigitizer  ;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL, 
      &MilDigitizer, M_NULL);
   
   if (!SystemSupportsGenICam(MilSystem))
      {
      MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);
      return 1;
      }

   MosPrintf(MIL_TEXT("This example shows how to query various feature properties.\n\n"));
   MosPrintf(MIL_TEXT("Finally the example concludes with how to enumerate all the features\n")
             MIL_TEXT("present in your GenICam compliant device.\n\n"));

   // If we have a Camera-Link frame grabber, we must enable CLProtocol.
   if(MsysInquire(MilSystem, M_BOARD_TYPE, M_NULL) & M_CL)
      {
      MosPrintf(MIL_TEXT("When using a Camera-Link frame grabber, make sure you are using\n"));
      MosPrintf(MIL_TEXT("a GenICam compliant camera and the camera vendor supplied a CLProtocol dll.\n\n"));
      MosPrintf(MIL_TEXT("The CLProtocol device must be selected in MILConfig->Boards->Camera Link.\n\n"));

      // Enable CLProtocol.
      MdigControl(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID, MIL_TEXT("M_DEFAULT"));
      MdigControl(MilDigitizer, M_GC_CLPROTOCOL, M_ENABLE);
      }

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Getting feature properties and value for the \"PixelFormat\" ")
      MIL_TEXT("feature.\n\n"));
   /* Retrieve feature info related to the PixelFormat feature. */
   GetFeatureProperties(MilDigitizer, MIL_TEXT("PixelFormat"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue and enumerate all GenICam features.\n\n"));
   MosGetch();

#if M_MIL_USE_WINDOWS
   system("cls");
#endif
   /* Enumerate all features under the root node. */
   EnumerateGenICamNodeTree(MilDigitizer, MIL_TEXT("Root"), 0);

   MosPrintf(MIL_TEXT("\nFinished enumeration.\n\n"));
   MosPrintf(MIL_TEXT("Note: due to console width constraints, some strings printed might have")
      MIL_TEXT(" been\nclipped.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Starting the MIL Feature Browser... Please wait.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to quit.\n\n"));
   MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN+M_ASYNCHRONOUS);
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

/* Routine used to retrieve various info related to a feature. */
void GetFeatureProperties(MIL_ID MilDigitizer, const MIL_STRING& FeatureName)
   {
   MIL_STRING Str;
   /* Inquire the feature's name. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_NAME, FeatureName, M_TYPE_STRING, Str);
   MosPrintf(MIL_TEXT("FeatureName:        %s\n"), Str.c_str());

   /* Inquire the feature's display name string. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_DISPLAY_NAME, FeatureName, M_TYPE_STRING, Str);
   MosPrintf(MIL_TEXT("FeatureDisplayName: %s\n"), Str.c_str());

   /* Inquire the feature's tool tip string. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_TOOLTIP, FeatureName, M_TYPE_STRING, Str);
   MosPrintf(MIL_TEXT("FeatureTooltip:     %s\n"), Str.c_str());

   /* Inquire the feature's description string. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_DESCRIPTION, FeatureName, M_TYPE_STRING, Str);
   MosPrintf(MIL_TEXT("FeatureDescription: %s\n"), Str.c_str());

   MIL_INT64 FeatureType = 0;
   /* Inquire the feature's native data type. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_TYPE, FeatureName, M_TYPE_INT64, &FeatureType);
   MIL_INT FeatureSize = 0;
   /* Inquire the feature's native data size in bytes. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_SIZE, FeatureName, M_TYPE_MIL_INT, &FeatureSize);
   MIL_INT64 FeatureAccessMode = 0;
   /* Inquire the feature's access mode. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_ACCESS_MODE, FeatureName, M_TYPE_INT64, &FeatureAccessMode);
   MIL_INT64 FeatureVisibility = 0;
   /* Inquire the feature's visibility attribute. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VISIBILITY, FeatureName, M_TYPE_INT64, &FeatureVisibility);
   MIL_INT64 FeatureCachingMode = 0;
   /* Inquire the feature's caching mode attribute. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_CACHING_MODE, FeatureName, M_TYPE_INT64, &FeatureCachingMode);
   MIL_INT64 FeatureStreamable = 0;
   /* Inquire the feature's streamable attribute. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_STREAMABLE, FeatureName, M_TYPE_INT64, &FeatureStreamable);
   MIL_INT64 FeatureDeprecated  = 0;
   /* Inquire the feature's deprecated attribute. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_DEPRECATED, FeatureName, M_TYPE_INT64, &FeatureDeprecated);

   MosPrintf(MIL_TEXT("\nType:                       %s\n"), TypeToString(FeatureType).c_str());
   MosPrintf(MIL_TEXT("Size:                       %lld bytes\n"), (long long)FeatureSize);
   MosPrintf(MIL_TEXT("Readable:                   %s\n"),
      M_FEATURE_IS_READABLE(FeatureAccessMode)?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Writable:                   %s\n"),
      M_FEATURE_IS_WRITABLE(FeatureAccessMode)?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Visibility:                 %s\n"),
      VisibilityToString(FeatureVisibility).c_str());
   MosPrintf(MIL_TEXT("Available:                  %s\n"),
      M_FEATURE_IS_AVAILABLE(FeatureAccessMode)?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Implemented:                %s\n"),
      M_FEATURE_IS_IMPLEMENTED(FeatureAccessMode)?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Cachable:                   %s\n"),
      M_FEATURE_IS_CACHABLE(FeatureCachingMode)?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Streamable:                 %s\n"),
      FeatureStreamable?MIL_TEXT("true"):MIL_TEXT("false"));
   MosPrintf(MIL_TEXT("Deprecated:                 %s\n"),
      FeatureDeprecated?MIL_TEXT("true"):MIL_TEXT("false"));


   MIL_INT64 IntVal = 0;
   MIL_DOUBLE DoubleVal = 0;
   MIL_BOOL BoolVal = 0;
   MIL_UINT8* RegVal = NULL;
   MIL_INT64 RegLen = 0;
   MIL_STRING ValueAsString;

   /* Inquire the feature's value. */
   if(M_FEATURE_IS_READABLE(FeatureAccessMode))
      {
      /* For floating point type features inquire the feature's value range. */
      if(FeatureType == M_TYPE_DOUBLE)
         {
         MIL_DOUBLE Min = 0, Max = 0;
         MIL_INT64 Representation = 0;
         MdigInquireFeature(MilDigitizer, M_FEATURE_MIN, FeatureName, M_TYPE_DOUBLE, &Min);
         MdigInquireFeature(MilDigitizer, M_FEATURE_MAX, FeatureName, M_TYPE_DOUBLE, &Max);
         MdigInquireFeature(MilDigitizer, M_FEATURE_REPRESENTATION, FeatureName, M_TYPE_INT64, &Representation);
         MosPrintf(MIL_TEXT("Min:                        %f\n"), Min);
         MosPrintf(MIL_TEXT("Max:                        %f\n"), Max);
         MosPrintf(MIL_TEXT("Representation:             %s\n"),
            RepresentationToString(Representation).c_str());
         }
      /* For integer type features inquire the feature's value range. */
      else if(FeatureType == M_TYPE_INT64)
         {
         MIL_INT64 Min = 0, Max = 0, Inc = 0, Representation = 0;
         MdigInquireFeature(MilDigitizer, M_FEATURE_MIN, FeatureName, M_TYPE_INT64, &Min);
         MdigInquireFeature(MilDigitizer, M_FEATURE_MAX, FeatureName, M_TYPE_INT64, &Max);
         MdigInquireFeature(MilDigitizer, M_FEATURE_INCREMENT, FeatureName, M_TYPE_INT64, &Inc);
         MdigInquireFeature(MilDigitizer, M_FEATURE_REPRESENTATION, FeatureName, M_TYPE_INT64, &Representation);
         MosPrintf(MIL_TEXT("Min:                        %lld\n"), (long long)Min);
         MosPrintf(MIL_TEXT("Max:                        %lld\n"), (long long)Max);
         MosPrintf(MIL_TEXT("Inc:                        %lld\n"), (long long)Inc);
         MosPrintf(MIL_TEXT("Representation:             %s\n"),
            RepresentationToString(Representation).c_str());
         }

      /* Inquire the feature's value as a sting. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, ValueAsString);
      MosPrintf(MIL_TEXT("Value as string:            %s\n"), ValueAsString.c_str());

      /* Inquire the feature's value using it's native data type. */
      switch(FeatureType)
         {
         case M_TYPE_INT64:
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_INT64, &IntVal);
            MosPrintf(MIL_TEXT("Value:                      %lld (0x%llx)\n"), (long long)IntVal, (long long)IntVal);
            }
            break;
         case M_TYPE_DOUBLE:
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_DOUBLE, &DoubleVal);
            MosPrintf(MIL_TEXT("Value:                      %f\n"), DoubleVal);
            }
            break;
         case M_TYPE_STRING:
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, ValueAsString);
            MosPrintf(MIL_TEXT("Value:                      %s\n"), ValueAsString.c_str());
            }
            break;
         case M_TYPE_BOOLEAN:
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_BOOLEAN, &BoolVal);
            MosPrintf(MIL_TEXT("Value:                      %d\n"), (int)BoolVal);
            }
            break;
         case M_TYPE_ENUMERATION:
            {
            MIL_INT Size = 0;
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_INT64, &IntVal);
            MosPrintf(MIL_TEXT("Value:                      %lld (0x%llx)\n"), (long long)IntVal, (long long)IntVal);

            // Inquire enum entry names and display names
            MIL_INT Count = 0;
            MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_COUNT, FeatureName, M_TYPE_MIL_INT, &Count);
            for (MIL_INT i = 0, j = 0; i < Count; ++i)
               {
               MIL_INT64 AccessMode = 0;
               MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, FeatureName, M_TYPE_INT64, &AccessMode);
               if (M_FEATURE_IS_AVAILABLE(AccessMode))
                  {
                  MIL_STRING Name;
                  MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_NAME + i, FeatureName, M_TYPE_STRING, Name);
                  if (j++ == 0)
                     MosPrintf(MIL_TEXT("Enum Entry Names:           %s\n"), Name.c_str());
                  else
                     MosPrintf(MIL_TEXT("                            %s\n"), Name.c_str());
                  }
               }

            for (MIL_INT i = 0, j = 0; i < Count; ++i)
               {
               MIL_INT64 AccessMode = 0;
               MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_ACCESS_MODE + i, FeatureName, M_TYPE_INT64, &AccessMode);
               if (M_FEATURE_IS_AVAILABLE(AccessMode))
                  {
                  MIL_STRING Name;
                  MdigInquireFeature(MilDigitizer, M_FEATURE_ENUM_ENTRY_DISPLAY_NAME + i, FeatureName, M_TYPE_STRING, Name);
                  if (j++ == 0)
                     MosPrintf(MIL_TEXT("Enum Entry Display Names:   %s\n"), Name.c_str());
                  else
                     MosPrintf(MIL_TEXT("                            %s\n"), Name.c_str());
                  }
               }
            }
            break;
         case M_TYPE_REGISTER:
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_SIZE, FeatureName, M_TYPE_INT64, &RegLen);
            RegVal = new MIL_UINT8[(MIL_INT)RegLen];
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_UINT8+M_FEATURE_USER_ARRAY_SIZE(RegLen), RegVal);
            MosPrintf(MIL_TEXT("Value: "));
            for(int i=0; i<RegLen; i++)
               MosPrintf(MIL_TEXT(" %d "), (int)RegVal[i]);
            }
            break;

         case M_DEFAULT:
         case M_TYPE_CATEGORY:
         default:
            /* Command and category features types do not have feature values. */
            break;
         }
      }

   if(RegVal)
      delete [] RegVal;

   MosPrintf(MIL_TEXT("\n\n"));
   }

/* Routine used to enumerate the device's GenICam(c) node tree. */
void EnumerateGenICamNodeTree(MIL_ID MilDigitizer, const MIL_STRING& Node,
   MIL_INT RecurseCount)
   {
   MIL_INT NodeCount = 0, SubNodeCount = 0;
   MIL_STRING FeatureName;
   MIL_STRING FeatureValue;
   MIL_STRING Format;
   MIL_STRING_STREAM stream;

   if(RecurseCount == 0)
      {
      MosPrintf(MIL_TEXT("%-40.39s%-19.18s%20.19s\n"), MIL_TEXT("Feature Name"),
         MIL_TEXT("Feature Type"), MIL_TEXT("Feature Value"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------------")
         MIL_TEXT("-------------\n\n"));
      }

   /* Prepare printf input format string. */
   for(MIL_INT i=0; i<RecurseCount; i++)
      stream << MIL_TEXT("   ");
   stream << MIL_TEXT("%-") << 40-(RecurseCount*3) << MIL_TEXT(".") << 39-(RecurseCount*3) <<
      MIL_TEXT("s%-19.18s%20.19s") << endl;
   Format = stream.str();

   /* Inquire the number of elements under this node. */
   MdigInquireFeature(MilDigitizer, M_SUBFEATURE_COUNT, Node, M_TYPE_MIL_INT, &NodeCount);
   for(MIL_INT i=0; i<NodeCount; i++)
      {
      MIL_INT64 Type = 0, AccessMode = 0, Visibility = 0;
      /* For each element under this node inquire it's string name. */
      MdigInquireFeature(MilDigitizer, M_SUBFEATURE_NAME+i, Node, M_TYPE_STRING, FeatureName);
      /* For each element under this node inquire it's type. */
      MdigInquireFeature(MilDigitizer, M_SUBFEATURE_TYPE+i, Node, M_TYPE_INT64, &Type);
      /* For each element under this node inquire it's read property. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_ACCESS_MODE, FeatureName, M_TYPE_INT64, &AccessMode);
      /* For each element under this node inquire it's visibility property. */
      MdigInquireFeature(MilDigitizer, M_FEATURE_VISIBILITY, FeatureName, M_TYPE_INT64, &Visibility);

      /* Validate that the feature is actually implemented on this specific device. */
      if(M_FEATURE_IS_IMPLEMENTED(AccessMode))
         {
         /* Read the feature's value only if it's a value feature and is readable. */
         if ((Type != M_DEFAULT && Type != M_TYPE_CATEGORY && Type != M_TYPE_REGISTER) &&
            M_FEATURE_IS_READABLE(AccessMode))
            {
            MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, FeatureName, M_TYPE_STRING, FeatureValue);
            }

         /* Features under a selector will appear twice. Filter out the double. */
         if(find(FeatureList.begin(), FeatureList.end(), FeatureName) == FeatureList.end())
            {
            /* Feature not in FeatureList, insert it at the end. */
            FeatureList.push_back(FeatureName);


            /* Inquire if this feature has child nodes. */
            MdigInquireFeature(MilDigitizer, M_SUBFEATURE_COUNT, FeatureName, M_TYPE_MIL_INT, &SubNodeCount);

            /* Print the feature name if visibility level ok. */
            if(Visibility <= VISIBILITY_LEVEL)
               MosPrintf(Format.c_str(), FeatureName.c_str(), TypeToString(Type).c_str(),
                  !FeatureValue.empty() ? FeatureValue.c_str() : MIL_TEXT(" "));

            /* If child nodes exist enumerate them. */
            if(SubNodeCount != 0)
               EnumerateGenICamNodeTree(MilDigitizer, FeatureName, RecurseCount+1);
            }
         }
      }
   }

/* Helper function used to convert a feature type into its string representation. */
MIL_STRING TypeToString(MIL_INT64 Type)
   {
   MIL_STRING sType;

   switch(Type)
      {
   case M_TYPE_INT64:
      sType = MIL_TEXT("M_TYPE_INT64");
      break;
   case M_TYPE_DOUBLE:
      sType = MIL_TEXT("M_TYPE_DOUBLE");
      break;
   case M_TYPE_BOOLEAN:
      sType = MIL_TEXT("M_TYPE_BOOLEAN");
      break;
   case M_TYPE_STRING:
      sType = MIL_TEXT("M_TYPE_STRING");
      break;
   case M_TYPE_ENUMERATION:
      sType = MIL_TEXT("M_TYPE_ENUMERATION");
      break;
   case M_TYPE_COMMAND:
      sType = MIL_TEXT("M_TYPE_COMMAND");
      break;
   case M_TYPE_REGISTER:
      sType = MIL_TEXT("M_TYPE_REGISTER");
      break;
   case M_TYPE_CATEGORY:
      sType = MIL_TEXT("M_TYPE_CATEGORY");
      break;
   default:
      sType = MIL_TEXT("M_NULL");
      break;
      }

   return sType;
   }

MIL_STRING VisibilityToString(MIL_INT64 Visibility)
   {
   MIL_STRING sVisibility;

   switch(Visibility)
      {
      case M_FEATURE_VISIBILITY_BEGINNER:
         sVisibility = MIL_TEXT("Beginner");
         break;
      case M_FEATURE_VISIBILITY_EXPERT:
         sVisibility = MIL_TEXT("Expert");
         break;
      case M_FEATURE_VISIBILITY_GURU:
         sVisibility = MIL_TEXT("Guru");
         break;
      case M_FEATURE_VISIBILITY_INVISIBLE:
         sVisibility = MIL_TEXT("Invisible");
         break;
      }

   return sVisibility;
   }

MIL_STRING RepresentationToString(MIL_INT64 Representation)
   {
   MIL_STRING sRepresentation;

   switch(Representation)
      {
      case M_FEATURE_REPRESENTATION_LINEAR:
         sRepresentation = MIL_TEXT("Linear");
         break;
      case M_FEATURE_REPRESENTATION_LOGARITHMIC:
         sRepresentation = MIL_TEXT("Logarithmic");
         break;
      case M_FEATURE_REPRESENTATION_BOOLEAN:
         sRepresentation = MIL_TEXT("Boolean");
         break;
      case M_FEATURE_REPRESENTATION_PURE_NUMBER:
         sRepresentation = MIL_TEXT("Pure number");
         break;
      case M_FEATURE_REPRESENTATION_HEX_NUMBER:
         sRepresentation = MIL_TEXT("Hex number");
         break;
      case M_FEATURE_REPRESENTATION_IPV4_ADDRESS:
         sRepresentation = MIL_TEXT("IPv4 address");
         break;
      case M_FEATURE_REPRESENTATION_MAC_ADDRESS:
         sRepresentation = MIL_TEXT("MAC address");
         break;
      }

   return sRepresentation;
   }
