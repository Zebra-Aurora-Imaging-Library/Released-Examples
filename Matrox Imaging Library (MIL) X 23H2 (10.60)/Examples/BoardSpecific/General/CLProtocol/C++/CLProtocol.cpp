﻿/********************************************************************************/
/* 
 * File name: CLProtocol.cpp 
 *
 * Synopsis:  This program demonstrates how to use GenICam® (through CLProtocol)
 *            on MIL Camera Link® systems.
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <vector>

using namespace std;

#if M_MIL_UNICODE_API

#ifndef MosStrtok
#define MosStrtok wcstok_s
#endif

#else

#if M_MIL_USE_LINUX
#ifndef MosStrtok
#define MosStrtok(A, B, C) strtok((A), (B))
#endif
#else
#ifndef MosStrtok
#define MosStrtok strtok_s
#endif
#endif

#endif

typedef struct  
   {
   MIL_INT NbDevIDs;
   MIL_INT DevIdStrLen;
   vector<MIL_STRING> DevIDs;
   long UserSelection;
   }CLProtocolDataStruct;

/* Enumerates device IDs supported by installed CLProtocol libraries. */
void CLProtocolEnumDeviceIDs(MIL_ID MilDigitizer, CLProtocolDataStruct& CLProtocolData);
/* Prompts user to select a CLProtocol device identifier matching his camera. */
void CLProtocolSelectDeviceID(MIL_ID MilDigitizer, CLProtocolDataStruct& CLProtocolData);

typedef enum { eDriverDirectory, eDriverFileName,  eManufacturer,
               eFamily,          eModel,           eVersion,      eSerialNumber} eCLProtocolDevIdFields;
/* Extract info from CLProtocol Device ID string. */
void CLProtocolExtractField(  const MIL_STRING& DeviceID, const eCLProtocolDevIdFields Field,
                              MIL_TEXT_PTR ExtractedData, MIL_INT DataSize);
#define STRLEN   256

/* Main function. */
int MosMain(void)
   { 
   MIL_ID MilApplication,  /* Application identifier.  */
          MilSystem,       /* System identifier.       */
          MilDisplay,      /* Display identifier.      */
          MilDigitizer,    /* Digitizer identifier.    */ 
          MilImage;        /* Image buffer identifier. */
   MIL_INT BoardType = 0, SystemNum = 0, DigitizerNum = 0;
   CLProtocolDataStruct CLProtocolData;
   MIL_STRING SystemDescriptor, VendorName, ModelName;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem,
                             &MilDisplay, &MilDigitizer, &MilImage);

   /* Make sure we are running on a Radient Camera Link.. */
   MsysInquire(MilSystem, M_BOARD_TYPE, &BoardType);
   if(!(BoardType & M_CL))
      {
      MosPrintf(MIL_TEXT("This example program can only be used with a Camera Link system type\n"));
      MosPrintf(MIL_TEXT("such as Matrox Solios, Matrox Radient or Matrox Rapixo Camera Link boards.\n"));
      MosPrintf(MIL_TEXT("Please ensure that the default system type is set accordingly in ")
                MIL_TEXT("MIL Config.\n"));
      MosPrintf(MIL_TEXT("-------------------------------------------------------------\n\n"));
      MosPrintf(MIL_TEXT("Press <enter> to quit.\n"));
      MosGetch();

      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);
      return 1;
      }

   /* Print a message. */
   MosPrintf(MIL_TEXT("This example shows how to use GenICam with Camera Link devices.\n\n"));
   MosPrintf(MIL_TEXT("GenICam is supported with Camera Link devices as long as your camera\n"));
   MosPrintf(MIL_TEXT("vendor supplies a standard compliant CLProtocol dll or your device\n\n"));
   MosPrintf(MIL_TEXT("supports GenCP.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to enumerate the device identifiers supported by\n\n"));
   MosPrintf(MIL_TEXT("installed CLProtocol libraries.\n\n"));


   MosGetch();

   /* Enumerates device IDs supported by installed CLProtocol libraries. */
   CLProtocolEnumDeviceIDs(MilDigitizer, CLProtocolData);

   if(CLProtocolData.NbDevIDs == 0)
      {
      MosPrintf(MIL_TEXT("\nNo CLProtocol libraries have been found.\n"));
      MosPrintf(MIL_TEXT("Make sure the CLProtocol library supplied by your camera vendor is\n"));
      MosPrintf(MIL_TEXT("properly installed.\n\n"));

      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);
      return 1;
      }

   /* Inquire system and digitizer numbers. */
   MsysInquire(MilSystem, M_NUMBER, &SystemNum);
   MdigInquire(MilDigitizer, M_NUMBER, &DigitizerNum);

   /* Inquire the system descriptor for printf below. */
   MsysInquire(MilSystem, M_SYSTEM_DESCRIPTOR, SystemDescriptor);

   /* Ask the user to select the correct device identifier matching his camera. */
   MosPrintf(MIL_TEXT("\nPlease select the CLProtocol device identifier of the camera connected to:\n"));
   MosPrintf(MIL_TEXT("%s M_DEV%d digitizer M_DEV%d (0-%d)\n"), SystemDescriptor.c_str(), (int)SystemNum, (int)DigitizerNum, (int)CLProtocolData.NbDevIDs);

   /* Prompt user to select a CLProtocol device identifier matching his camera. */
   CLProtocolSelectDeviceID(MilDigitizer, CLProtocolData);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nNow showing the camera's features through the feature browser window.\n"));
   MosPrintf(MIL_TEXT("You can use the feature browser to change camera parameters.\n\n"));
   
   /* At this point the CLProtocol (and GenICam®) should be properly initialized. */
   /* Pop up the camera's feature browser.                                        */
   MdigControl(MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN+M_ASYNCHRONOUS);

   /* Grab continuously. */
   MdigGrabContinuous(MilDigitizer, MilImage);

   /* Print a message. */
   MosPrintf(MIL_TEXT("Press <Enter> to use MdigInquireFeature to read \"DeviceVendorName\" and\n"));
   MosPrintf(MIL_TEXT("\"DeviceModelName\" features from the camera.\n\n"));
   MosPrintf(MIL_TEXT("Note: an error will be generated if the features do not exist in your camera\n\n"));
   MosGetch();

   /* Use MdigInquireFeature to read data from the camera.              */
   /* Note: MdigControlFeature can be used to write data to the camera. */
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceVendorName"), M_TYPE_STRING, VendorName);
   MdigInquireFeature(MilDigitizer, M_FEATURE_VALUE, MIL_TEXT("DeviceModelName"), M_TYPE_STRING, ModelName);

   /* Print a message. */
   MosPrintf(MIL_TEXT("Vendor:\t%s\nModel:\t%s\n\n"), VendorName.c_str(), ModelName.c_str());

   /* When a key is pressed, halt. */
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
   MosGetch();

   /* Stop continuous grab. */
   MdigHalt(MilDigitizer);

   /* Free defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImage);

   return 0;
   }

/* Enumerates device IDs supported by installed CLProtocol libraries. */
void CLProtocolEnumDeviceIDs(MIL_ID MilDigitizer, CLProtocolDataStruct& CLProtocolData)
   {
   MIL_TEXT_PTR NextToken = M_NULL;
   MIL_TEXT_CHAR Field[7][256];

   /* Inquire the number of registered CLProtocol device IDs. */
   MdigInquire(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID_NUM, &CLProtocolData.NbDevIDs);
   /* Inquire the maximum string length we will need to store the device IDs. */
   MdigInquire(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID_SIZE_MAX, &CLProtocolData.DevIdStrLen);

   MosPrintf(MIL_TEXT("Installed CLProtocol devices found:\n\n"));

   if(CLProtocolData.NbDevIDs)
      {
      /* Allocate memory for Device ID storage. */
      CLProtocolData.DevIDs.assign(CLProtocolData.NbDevIDs, MIL_TEXT(""));

      MosPrintf(MIL_TEXT("%2s|%22.21s|%10.9s|%20.19s|%20.19s\n"), MIL_TEXT("Nb"), MIL_TEXT("CLProtocol File Name"), MIL_TEXT("Vendor"), MIL_TEXT("Family"), MIL_TEXT("Model"));
      MosPrintf(MIL_TEXT("--+----------------------+----------+--------------------+--------------------\n\n"));
      /* For each device, inquire it's DeviceID. */
      for(MIL_INT i=0; i<CLProtocolData.NbDevIDs; i++)
         {
         /* Allocate memory for Device ID storage. */
         MdigInquire(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID+i, CLProtocolData.DevIDs[i]);

         /* Tokenize the string to make printing more readable. */
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eDriverDirectory, Field[0], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eDriverFileName,  Field[1], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eManufacturer,    Field[2], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eFamily,          Field[3], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eModel,           Field[4], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eVersion,         Field[5], 256);
         CLProtocolExtractField(CLProtocolData.DevIDs[i], eSerialNumber,    Field[6], 256);

         MosPrintf(MIL_TEXT("%.2d %22.21s %10.9s %20.19s %20.19s\n"), i, Field[1], Field[2], Field[3], Field[4]);
         }

      MosPrintf(MIL_TEXT("\n%.2d Use Default from MilConfig.\n"), CLProtocolData.NbDevIDs);
      }
   }

/* Prompts user to select a CLProtocol device identifier matching his camera. */
void CLProtocolSelectDeviceID(MIL_ID MilDigitizer, CLProtocolDataStruct& CLProtocolData)
   {
   bool Done = false;
   char InputStream[64] = { '\0' };
   do
      {
      fgets(InputStream, sizeof(InputStream), stdin);

#if M_MIL_USE_WINDOWS
      int result = sscanf_s(InputStream, "%ld", &CLProtocolData.UserSelection);
#else
      int result = sscanf(InputStream, "%ld", &CLProtocolData.UserSelection);
#endif

      if((result == 1) && (CLProtocolData.UserSelection >= 0) && (CLProtocolData.UserSelection <= CLProtocolData.NbDevIDs))
         Done = true;
      else
         MosPrintf(MIL_TEXT("Invalid selection, please try again.\n"));
      }
      while(!Done);

      /* Apply the selected device identifier. */
      if(CLProtocolData.UserSelection == CLProtocolData.NbDevIDs)
         MdigControl(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID, MIL_TEXT("M_DEFAULT"));
      else
         MdigControl(MilDigitizer, M_GC_CLPROTOCOL_DEVICE_ID, CLProtocolData.DevIDs[CLProtocolData.UserSelection]);
      /* Initialize the CLProtocol driver and GenICam®. */
      /* If an error occurs, it is most likely that the wrong CLProtocol device identifier has been selected. */
      MdigControl(MilDigitizer, M_GC_CLPROTOCOL, M_ENABLE);
   }

/* Extract field from CLProtocol DeviceID string. */
void CLProtocolExtractField(const MIL_STRING& DeviceID, const eCLProtocolDevIdFields Field, MIL_TEXT_PTR ExtractedData, MIL_INT DataSize)
   {
   MIL_INT Cnt = 0;
   MIL_STRING TempString = DeviceID;
   MIL_TEXT_PTR Token = NULL, NextToken = NULL;
   MIL_TEXT_CHAR FieldDelimitor[] = { MIL_TEXT("#") };

   switch(Field)
      {
      case eDriverDirectory:
         Cnt = 0; break;
      case eDriverFileName:
         Cnt = 1; break;
      case eManufacturer:
         Cnt = 2; break;
      case eFamily:
         Cnt = 3; break;
      case eModel:
         Cnt = 4; break;
      case eVersion:
         Cnt = 5; break;
      case eSerialNumber:
         Cnt = 6; break;
      }

   Token = MosStrtok(&TempString[0], FieldDelimitor, &NextToken);
   for(MIL_INT i=0; i<Cnt && Token!=NULL; i++)
      {
      Token = MosStrtok(NULL, FieldDelimitor, &NextToken);
      }

   if(Token)
      MosStrcpy(ExtractedData, DataSize, Token);
   else
      ExtractedData[0] = '\0';
   }
