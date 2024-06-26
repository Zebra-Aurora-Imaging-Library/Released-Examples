﻿/********************************************************************************/
/*
* File name: GenTL.cpp
*
* Synopsis:  This program demonstrates MIL features for GenICam's GenTL.
*            For more information about GenTL see the EMVA's GenICam section
*            at http://www.emva.org. The MIL M_SYSTEM_GENTL is a GenICam
*            GenTL consumer. As such it requires a third party supplied
*            GenTL producer to be installed.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

/* Headers. */
#include <mil.h>
#include <vector>

using namespace std;

/* Define a container class to hold MIL resources associated to a Device. */
class GenTLDevice
   {
   public:
      GenTLDevice()
         {
         MilSystem = M_NULL;
         MilDisplay = M_NULL;
         MilDigitizer = M_NULL;
         MilImage = M_NULL;
         Number = 0;
         }

      MIL_STRING Vendor;
      MIL_STRING Model;
      MIL_STRING TLType;
      MIL_INT Number;
      MIL_ID MilSystem;
      MIL_ID MilDisplay;
      MIL_ID MilDigitizer;
      MIL_ID MilImage;
   };

/**/
#define MAX_SYSTEMS    16

/* Utility function declarations. */

/* GenTL Producer selection and info functions. */
MIL_INT SelectGenTLProducerLib();
void DisplayProducerInfo(MIL_INT MilSystem);

/* GenTL interface and device discovery function. */
MIL_INT Discover(MIL_INT MilSystem, vector<GenTLDevice>& Devices);
MIL_INT DiscoverDevices(MIL_INT MilSystem, MIL_INT64 Interface, const MIL_STRING& TLType, vector<GenTLDevice>& Devices);

/* Device selection and usage functions. */
GenTLDevice SelectGenTLDevice(const vector<vector<GenTLDevice> >& Devices);
void UseGenTLDevice(GenTLDevice& Device);
void FreeGenTLDevice(GenTLDevice& Device);

/* Utility function used to disable error messages while cycling MIL system allocation. */
void DisableErrorPrint()   { MappControl(M_ERROR, M_PRINT_DISABLE); }
void EnableErrorPrint()    { MappControl(M_ERROR, M_PRINT_ENABLE); }

/* Main function. */
int MosMain(void)
   {
   MIL_ID MilApplication,                 /* Application identifier.  */
          MilSystem[MAX_SYSTEMS] = { 0 }; /* System identifier.       */

   /* vector to a vector of devices. */
   /* There is one vector of devices per MIL system allocated.*/
   vector<vector<GenTLDevice> > Devices(MAX_SYSTEMS, vector<GenTLDevice>());
   MIL_INT NumDevices = 0, Selection = 0;

   /* Allocate the MIL application module. */
   MappAlloc(M_DEFAULT, &MilApplication);

   MosPrintf(MIL_TEXT("This example shows how to enumerate GenTL producer libraries.\n"));
   MosPrintf(MIL_TEXT("It then proceeds to enumerate interfaces and devices.\n"));
   MosPrintf(MIL_TEXT("Finally the example allocates a device and starts a grab.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   
   /* Select a GenTL producer library. */
   Selection = SelectGenTLProducerLib();
   if (Selection == -1)
      {
      MappFree(MilApplication);
      return 0;
      }

   /* Allocate a M_SYSTEM_GENTL system until there are no more to allocate. */
   for (MIL_INT i = M_DEV0; i < MAX_SYSTEMS; i++)
      {
      /* Prevent error printing if allocation fails. */
      if (i != M_DEV0)
         DisableErrorPrint();
      MsysAlloc(M_SYSTEM_GENTL, i + M_GENTL_PRODUCER(Selection), M_DEFAULT, &MilSystem[i]);
      if (i != M_DEV0)
         EnableErrorPrint();
      
      if (MilSystem[i] != M_NULL)
         {
         if (i == M_DEV0)
            {
            /* Print information related to the GenTL producer library selected. */
            DisplayProducerInfo(MilSystem[i]);
            MosPrintf(MIL_TEXT("\n-------------------- Detecting GenTL Interfaces and devices --------------------\n"));
            }

         MosPrintf(MIL_TEXT("\n----------------------------------- System %d -----------------------------------"), (int)i);
         /* Discover GenTL interfaces and devices. */
         NumDevices += Discover(MilSystem[i], Devices[i]);
         
         MosPrintf(MIL_TEXT("\nPress <Enter> to open the system's feature browser.\n"));
         MosGetch();
         /* Display the system's feature browser. */
         MsysControl(MilSystem[i], M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
         }
      else
         break;
      }
   
   if (NumDevices)
      {
      MosPrintf(MIL_TEXT("\nPress <Enter> to select and grab from a device.\n"));
      MosGetch();

      /* Select a device to use. */
      GenTLDevice Device = SelectGenTLDevice(Devices);
      if (Device.MilSystem)
         {
         /* Allocate and start acquisition. */
         UseGenTLDevice(Device);
         
         MosPrintf(MIL_TEXT("Press <Enter> to quit.\n"));
         MosGetch();
         
         /* Free resources associated to the device. */
         FreeGenTLDevice(Device);
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("\nPress <Enter> to quit.\n"));
      MosGetch();
      }

   /* Free allocated MIL systems. */
   for (MIL_INT i = 0; i < MAX_SYSTEMS; i++)
      {
      if (MilSystem[i])
         MsysFree(MilSystem[i]);
      }

   /* Free MIL application module. */
   MappFree(MilApplication);
   return 0;
   }

/* Used to enumerate and select a GenTL producer library. */
MIL_INT SelectGenTLProducerLib()
   {
   MIL_INT NumLibraries = 0;
   int Selection = 1;

   MosPrintf(MIL_TEXT("---------------------- Detecting installed GenTL producers ---------------------\n"));
   
   /* Inquire the number of installed GenTL producers. */
   MappInquire(M_GENTL_PRODUCER_COUNT, &NumLibraries);
   if (NumLibraries == 0)
      {
      MosPrintf(MIL_TEXT("A third party software component, a GenTL Producer, is missing.\n"));
      MosPrintf(MIL_TEXT("Exiting.\n"));
      return -1;
      }
   else
      {
      MosPrintf(MIL_TEXT("Found the following GenTL producer libraries: \n\n"));

      /* Get the installed GenTL producer libraries. */
      MIL_INT Size = 0;
      for (MIL_INT i = 0; i < NumLibraries; i++)
         {
         MIL_STRING Descriptor;
         MappInquire(M_GENTL_PRODUCER_DESCRIPTOR + i, Descriptor);
         MosPrintf(MIL_TEXT("%2.d %s.\n"), i + 1, Descriptor.c_str());
         }

      /* Ask the user to select a GenTL producer to use. */
      if (NumLibraries > 1)
         {
         MosPrintf(MIL_TEXT("\nWhich GenTL producer do you want to use? "));
         do
            {
            MOs_scanf_s(MIL_TEXT("%d"), &Selection);
            if (Selection > NumLibraries)
               MosPrintf(MIL_TEXT("Invalid selection.\n"));
            } while (Selection > NumLibraries);
         }

      MosPrintf(MIL_TEXT("\n"));
      }

   return Selection - 1;
   }

/* Used to display GenTL producer information. */
void DisplayProducerInfo(MIL_INT MilSystem)
   {
   MIL_STRING Vendor, Model, Version, Type;

   /* Get the GenTL Producer info via the GenTL System module XML. */
   MsysInquireFeature(MilSystem, M_GENTL_SYSTEM + M_FEATURE_VALUE, MIL_TEXT("TLVendorName"), M_TYPE_STRING, Vendor);
   MsysInquireFeature(MilSystem, M_GENTL_SYSTEM + M_FEATURE_VALUE, MIL_TEXT("TLModelName"), M_TYPE_STRING, Model);
   MsysInquireFeature(MilSystem, M_GENTL_SYSTEM + M_FEATURE_VALUE, MIL_TEXT("TLVersion"), M_TYPE_STRING, Version);
   MsysInquireFeature(MilSystem, M_GENTL_SYSTEM + M_FEATURE_VALUE, MIL_TEXT("TLType"), M_TYPE_STRING, Type);

   MosPrintf(MIL_TEXT("\n-------------------------- GenTL producer information --------------------------\n"));
   MosPrintf(MIL_TEXT("Vendor:               %s.\n"), Vendor.c_str());
   MosPrintf(MIL_TEXT("Model:                %s.\n"), Model.c_str());
   MosPrintf(MIL_TEXT("Version:              %s.\n"), Version.c_str());
   MosPrintf(MIL_TEXT("Transport layer type: %s.\n"), Type.c_str());
   }

/* Used to enumerate GenTL interface modules and device modules. */
/* Devices will be added to the vector of devices.               */
MIL_INT Discover(MIL_INT MilSystem, vector<GenTLDevice>& Devices)
   {
   MIL_INT NumInterfaces = 0;
   MIL_STRING InterfaceType;
   MIL_INT DeviceCount = 0;

   /* Get the number of GenTL interfaces associated to this MIL system. */
   MsysInquire(MilSystem, M_GENTL_INTERFACE_COUNT, &NumInterfaces);
   if (NumInterfaces == 0)
      {
      MosPrintf(MIL_TEXT("No GenTL interfaces found.\n"));
      MosPrintf(MIL_TEXT("Make sure your GenTL Producer drivers are properly installed.\n"));
      return 0;
      }

   /* For each GenTL interface inquire its transport layer type */
   /* and discover devices associated to the interface.         */
   for (MIL_INT64 i = 0; i < NumInterfaces; i++)
      {
      MsysInquireFeature(MilSystem, M_GENTL_INTERFACE_NUMBER(i) + M_FEATURE_VALUE, MIL_TEXT("InterfaceType"), M_TYPE_STRING, InterfaceType);

      MosPrintf(MIL_TEXT("\n%s Interface%lld.\n"), InterfaceType.c_str(), (long long)i);

      DeviceCount += DiscoverDevices(MilSystem, M_GENTL_INTERFACE_NUMBER(i), InterfaceType, Devices);
      }

   return DeviceCount;
   }

/* Used to enumerate GenTL device modules.         */
/* Devices will be added to the vector of devices. */
MIL_INT DiscoverDevices(MIL_INT MilSystem, MIL_INT64 Interface, const MIL_STRING& TLType, vector<GenTLDevice>& Devices)
   {
   MIL_INT NumDevices = 0;

   /* Get the number of devices associated to the Interface. */
   MsysInquire(MilSystem, Interface + M_GENTL_DEVICE_COUNT, &NumDevices);

   if (NumDevices == 0)
      {
      MosPrintf(MIL_TEXT("\tNo devices found.\n"));
      MosPrintf(MIL_TEXT("\tMake sure a device is connected to this interface and\n"));
      MosPrintf(MIL_TEXT("\tthat your GenTL Producer's drivers are properly installed.\n"));
      }

   /* For each device inquire its vendor info and add the device to the device vector. */
   for (MIL_INT64 i = 0; i < NumDevices; i++)
      {
      GenTLDevice Device;
      Device.MilSystem = MilSystem;
      Device.Number = Devices.size();
      Device.TLType = TLType;
      MsysControlFeature(MilSystem, Interface + M_FEATURE_VALUE, MIL_TEXT("DeviceSelector"), M_TYPE_INT64, &i);
      MsysInquireFeature(MilSystem, Interface + M_FEATURE_VALUE, MIL_TEXT("DeviceVendorName"), M_TYPE_STRING, Device.Vendor);
      MsysInquireFeature(MilSystem, Interface + M_FEATURE_VALUE, MIL_TEXT("DeviceModelName"), M_TYPE_STRING, Device.Model);
     
      Devices.push_back(Device);

      MosPrintf(MIL_TEXT("\tDevice%lld: %s %s.\n"), (long long)i, Device.Vendor.c_str(), Device.Model.c_str());
      }

   return NumDevices;
   }

/* Used to select a Device to use. */
GenTLDevice SelectGenTLDevice(const vector<vector<GenTLDevice> >& Devices)
   {
   int Selection = -1;
   int DeviceCount = 0;
   MosPrintf(MIL_TEXT("\n--------------------------------Device Selection -------------------------------\n"));
   /* Print all devices from all systems specifying their TLType. */
   for (MIL_INT i = 0; i < (MIL_INT)Devices.size(); i++)
      {
      for (MIL_INT j = 0; j < (MIL_INT)Devices[i].size(); j++)
         {
         MosPrintf(MIL_TEXT("%2.d (%s) %s %s.\n"), ++DeviceCount, Devices[i][j].TLType.c_str(), Devices[i][j].Vendor.c_str(), Devices[i][j].Model.c_str());
         }
      }

   /* Ask the user to select a device to use. */
   if (DeviceCount > 1)
      {
      MosPrintf(MIL_TEXT("\nWhich device do you want to use?\n"));

      do
         {
         MOs_scanf_s(MIL_TEXT("%d"), &Selection);
         if (Selection > (MIL_INT)Devices.size())
            MosPrintf(MIL_TEXT("Invalid selection.\n"));
         } while (Selection > (MIL_INT)Devices.size());
      }
   else
      Selection = 1;

   DeviceCount = 0;
   /* Return the device to use. */
   for (MIL_INT i = 0; i < (MIL_INT)Devices.size(); i++)
      {
      for (MIL_INT j = 0; j < (MIL_INT)Devices[i].size(); j++)
         {
         DeviceCount++;
         if (DeviceCount == Selection)
            return Devices[i][j];
         }
      }

   return GenTLDevice();
   }

/* Used to allocate and start acquisition from a device. */
void UseGenTLDevice(GenTLDevice& Device)
   {
   MIL_INT64 DataFormat = 0;
   MIL_INT SizeBit = 0;
   MosPrintf(MIL_TEXT("\nAllocating device and starting acquisition.\n"));

   /* Allocate MIL resources required to grab and display the acquired data. */
   MdigAlloc(Device.MilSystem, Device.Number, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &Device.MilDigitizer);
   if (Device.MilDigitizer)
      {
      MdispAlloc(Device.MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &Device.MilDisplay);

      MdigControl(Device.MilDigitizer, M_GC_FEATURE_BROWSER, M_OPEN + M_ASYNCHRONOUS);
      MdigInquire(Device.MilDigitizer, M_SOURCE_DATA_FORMAT, &DataFormat);
      MdigInquire(Device.MilDigitizer, M_SIZE_BIT, &SizeBit);
      MbufAllocColor(Device.MilSystem,
                     MdigInquire(Device.MilDigitizer, M_SIZE_BAND, M_NULL),
                     MdigInquire(Device.MilDigitizer, M_SIZE_X, M_NULL),
                     MdigInquire(Device.MilDigitizer, M_SIZE_Y, M_NULL),
                     MdigInquire(Device.MilDigitizer, M_TYPE, M_NULL),
                     M_IMAGE + M_DISP + M_GRAB + DataFormat,
                     &Device.MilImage);

      if (SizeBit != 8)
         {
         MdispControl(Device.MilDisplay, M_VIEW_MODE, M_BIT_SHIFT);
         MdispControl(Device.MilDisplay, M_VIEW_BIT_SHIFT, SizeBit - 8);
         }
      MdispSelect(Device.MilDisplay, Device.MilImage);
      MdigGrabContinuous(Device.MilDigitizer, Device.MilImage);
      }
   }

/* Used to stop acquisition and free device resources. */
void FreeGenTLDevice(GenTLDevice& Device)
   {
   if (Device.MilDigitizer)
      {
      MdigHalt(Device.MilDigitizer);
      MdigFree(Device.MilDigitizer);
      MdispFree(Device.MilDisplay);
      MbufFree(Device.MilImage);
      }
   }
