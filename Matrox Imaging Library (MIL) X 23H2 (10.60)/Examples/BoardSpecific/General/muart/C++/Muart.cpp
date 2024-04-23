﻿/***********************************************************************************/
/*
 * File name: Muart.cpp
 *
 * Synopsis:  This program allows you to test all the UART features
 *            to read and write data.  The user must physically link
 *            the UART connector on the Matrox board to the specified
 *            COM port on the same computer.
 *
 * Note :     This example will only run with Matrox boards that support
 *            the UART features. These boards are:
 *            Matrox Morphis, Matrox Solios, Matrox Rapixo CL,and Matrox Radient eVCL.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

/* Removes the warning when we use scanf() in Visual Studio 2005. */
#define _CRT_SECURE_NO_DEPRECATE

/* Headers. */
#include <mil.h>
#include <windows.h>

#define IMAGE_FILE         M_IMAGE_PATH MIL_TEXT("Bird.mim")

/* Uart configuration parameters. */
/* See MIL hardware specific notes for a complete list of available baud rates. */
#define CONFIG_SPEED       38400

/* Only 1 or 2 stop bits are allowed. */
#define CONFIG_STOP_BITS   1

/* Only 7 or 8 data bits are allowed. */
#define CONFIG_DATA_LENGTH 8

/* Parity can be set to M_DISABLE, M_ODD or M_EVEN. */
#define CONFIG_PARITY      M_DISABLE

/* SystemInfo Struct. */
#define MAX_PORTS          50
typedef struct
   {
   MIL_TEXT_CHAR  ComPortName[1][MAX_PATH];
   MIL_TEXT_CHAR  System[1][MAX_PATH];
   MIL_INT        Device;
   MIL_UINT       UartNumber;
   }SystemInfo;

/* Hooked Data Struct. */
typedef struct
   {
   MIL_ID         SystemId;
   MIL_ID         ReceiveBufferId;
   MIL_INT8 *     ReceiveBuffer;
   MIL_UINT       ReadSize;
   MIL_INT        ReadPosition;
   SystemInfo     *Matrox;
   }UartHook;

/* Function prototypes. */
MIL_INT MFTYPE ReadHook(MIL_INT , MIL_ID , void*);
MIL_INT InitializeComPort(HANDLE *hCom, MIL_INT InterfaceType, SystemInfo *SysInfo);
void InitializeMatroxUart(MIL_ID *MilSystem, SystemInfo *SysInfo, MIL_INT *UartInterfaceType);
void ReadFromComPort(HANDLE hCom, MIL_INT8 *Buffer, MIL_UINT SizeToRead,
                     OVERLAPPED *Overlapped);
void WriteToComPort(HANDLE hCom, MIL_INT8 *Buffer, MIL_UINT SizeToWrite,
                    OVERLAPPED *Overlapped);
bool EnumerateComPorts(SystemInfo *SysInfo);

int MosMain(void)
   {
   MIL_ID         MilApplication,         /* MIL Application identifier.            */
                  MilSystem,              /* MIL System identifier.                 */
                  MilDisplaySource,       /* MIL Display identifier.                */
                  MilDisplayReceive,      /* MIL Display identifier.                */
                  MilSourceImage,         /* MIL Source image identifier.           */
                  MilReceivedImage;       /* MIL Received image through UART.       */
   MIL_INT64      SourceFormat;           /* Buffer format of source image.         */
   OVERLAPPED     Overlapped;             /* Windows overlapped structure.          */
   HANDLE         hCom;                   /* Windows COM port handle.               */
   MIL_INT8       *SourceBuffer,          /* Pointer to address of source buffer.   */
                  *ReceiveBuffer;         /* Pointer to address of receive buffer.  */
   MIL_INT        SourceImageSize,        /* Size of source image.                  */
                  UartInterfaceType;      /* Type of UART interface being used.     */
   MIL_UINT32     SentSize;               /* Number of bytes written.               */
   MIL_UINT       lNumBytesTransferred;   /* Number of bytes transferred.           */
   long volatile  DoSomething = 0;        /* Dummy variable.                        */
   volatile       UartHook UserStruct;    /* UartHook structure object.             */

   /* Initialize SystemInfo structure and enumerate COM ports. */
   SystemInfo Matrox;
   bool validChoice = EnumerateComPorts(&Matrox);
   if(!validChoice)
      return 0;

   /* Initialize overlapped structure. */
   memset(&Overlapped, 0, sizeof(Overlapped));
   Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

   /* MIL allocations. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication );
   MsysAlloc(M_DEFAULT, Matrox.System[0], Matrox.Device, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT,
              &MilDisplaySource);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT,
              &MilDisplayReceive);

   /* Allocate MilSourceImage and load an image in it. */
   MbufRestore(IMAGE_FILE, MilSystem, &MilSourceImage);

   /* Allocate the receive buffer with the same attributes as the source image. */
   MbufInquire(MilSourceImage, M_EXTENDED_ATTRIBUTE, &SourceFormat);
   MbufAllocColor(MilSystem,
      (MIL_INT)MbufInquire(MilSourceImage, M_SIZE_BAND, M_NULL),
      (MIL_INT)MbufInquire(MilSourceImage, M_SIZE_X, M_NULL),
      (MIL_INT)MbufInquire(MilSourceImage, M_SIZE_Y, M_NULL),
      (MIL_INT)MbufInquire(MilSourceImage, M_TYPE, M_NULL),
      SourceFormat,
      &MilReceivedImage);
   MbufClear(MilReceivedImage, M_COLOR_BLACK);

   /* Inquire the size in bytes of the source image. */
   SourceImageSize = MbufInquire(MilSourceImage, M_SIZE_X, M_NULL) * MbufInquire(MilSourceImage, M_SIZE_Y, M_NULL) * MbufInquire(MilSourceImage, M_SIZE_BAND, M_NULL);
   /* Inquire the virtual address of the source buffer.  */
   MbufInquire(MilSourceImage,   M_HOST_ADDRESS,   &SourceBuffer);
   /* Inquire the virtual address of the receive buffer.  */
   MbufInquire(MilReceivedImage, M_HOST_ADDRESS,   &ReceiveBuffer);

   /* Print text on the displays and associate buffers. */
   MdispControl(MilDisplaySource,  M_TITLE, MIL_TEXT("MIL Source Image"));
   MdispControl(MilDisplayReceive, M_TITLE,
                MIL_TEXT("MIL Received Image through UART."));
   MdispSelect(MilDisplaySource, MilSourceImage);
   MdispSelect(MilDisplayReceive, MilReceivedImage);

   /* Initialize Matrox UART */
   InitializeMatroxUart(&MilSystem, &Matrox, &UartInterfaceType);

   /* Initialize the OS mapped COM port as the other UART. */
   if(!InitializeComPort(&hCom, UartInterfaceType, &Matrox))
      return 0;

   /* Test to send data from the Matrox UART. */
   MosPrintf(MIL_TEXT("The program will now send data from your Matrox board to your ")
             MIL_TEXT("com port.\n"));
   MosPrintf(MIL_TEXT("%lld bytes will be sent, this may take some time.\n"), (long long)SourceImageSize);
   MosPrintf(MIL_TEXT("Press <Enter> to send data.\n"));
   MosGetch();

   /* Setting the number of bytes to transfer. */
   MsysControl(MilSystem, M_UART_WRITE_STRING_SIZE + M_UART_NB(Matrox.UartNumber),
               SourceImageSize);

   /* Sending data through the Matrox UART. This call is asynchronous. */
   MsysControl(MilSystem, M_UART_WRITE_STRING + M_UART_NB(Matrox.UartNumber),
               (MIL_INT)SourceBuffer);

   /* Read the data from the COM port. */
   ReadFromComPort(hCom, ReceiveBuffer, SourceImageSize, &Overlapped);

   /* Wait for the asynchronous write operation to finish and retrieve
   the actual number of bytes transferred. */
   lNumBytesTransferred = 0;
   MsysInquire(MilSystem, M_UART_BYTES_WRITTEN + M_UART_NB(Matrox.UartNumber),
               &lNumBytesTransferred);

   /* Display the result. */
   MosPrintf(MIL_TEXT("\n\nImage received through COM port\n\n"));
   /* Let the MIL image buffer know that it has been updated. */
   MbufControl(MilReceivedImage, M_MODIFIED, M_DEFAULT);

   /* Test reading data from the Matrox board UART with M_UART_READ_STRING. */
   MosPrintf(MIL_TEXT("The program will now send data from your com port to your ")
             MIL_TEXT("Matrox board.\n"));
   MosPrintf(MIL_TEXT("%lld bytes will be sent, this may take some time.\n"), (long long)SourceImageSize);
   MosPrintf(MIL_TEXT("Press <Enter> to send data.\n"));
   MosGetch();

   MbufClear(MilReceivedImage, M_COLOR_BLACK);

   /* Setting the delimiter to the standard '\0' character. */
   MsysControl(MilSystem, M_UART_STRING_DELIMITER + M_UART_NB(Matrox.UartNumber), M_DEFAULT);

   /* This control is useful only if M_UART_READ_STRING_SIZE is set to M_DEFAULT. */
   MsysControl(MilSystem, M_UART_READ_STRING_MAXIMUM_SIZE + M_UART_NB(Matrox.UartNumber),
               SourceImageSize);

   /* Setting the receive buffer size to the source buffer size. Setting
   M_UART_READ_STRING_SIZE to M_DEFAULT would result in reading until
   the delimiter character is received or until M_UART_READ_STRING_MAXIMUM_SIZE
   is reached, whichever happens first. Note that this mode of operation
   (using M_DEFAULT) will result in a loss of performance. Performance is
   improved if the actual number of characters to read is specified. */
   MsysControl(MilSystem, M_UART_READ_STRING_SIZE + M_UART_NB(Matrox.UartNumber),
               SourceImageSize);

   /* Reading incoming data through the Matrox board UART. This call is asynchronous. */
   MsysControl(MilSystem, M_UART_READ_STRING + M_UART_NB(Matrox.UartNumber),
               ReceiveBuffer);

   /* Write the data with the COM port. */
   WriteToComPort(hCom, SourceBuffer, SourceImageSize, &Overlapped);

   /* Wait for the asynchronous read operation to finish and retrieve
   the actual number of bytes received. */
   lNumBytesTransferred = 0;
   MsysInquire(MilSystem, M_UART_BYTES_READ + M_UART_NB(Matrox.UartNumber),
               &lNumBytesTransferred);

   /* Display the result. */
   MosPrintf(MIL_TEXT("\n\nImage received through MIL Uart: \n\n"));
   /* Let the MIL image buffer know that it has been updated. */
   MbufControl(MilReceivedImage, M_MODIFIED, M_DEFAULT);

   /* Test to read data from the Matrox board UART with a hook function. */
   MosPrintf(MIL_TEXT("The program will now send data from your COM port to your ")
             MIL_TEXT("Matrox board\n"));
   MosPrintf(MIL_TEXT("and read the data through a MIL hook function.\n"));
   MosPrintf(MIL_TEXT("%lld bytes will be sent, this may take some time.\n"), (long long)SourceImageSize);
   MosPrintf(MIL_TEXT("Press <Enter> to send data.\n"));
   MosGetch();

   MbufClear(MilReceivedImage, M_COLOR_BLACK);

   /* Initialize UART hook structure. */
   UserStruct.SystemId        = MilSystem;
   UserStruct.ReceiveBufferId = MilReceivedImage;
   UserStruct.ReceiveBuffer   = ReceiveBuffer;
   UserStruct.ReadSize        = SourceImageSize;
   UserStruct.ReadPosition    = 0;
   UserStruct.Matrox          = &Matrox;

   /* Hook the function to M_UART_DATA_RECEIVED. */
   MsysHookFunction(MilSystem, M_UART_DATA_RECEIVED + M_UART_NB(Matrox.UartNumber), ReadHook,
                    (void *)(&UserStruct));

   /* Send data through the Windows COM port. */
   ResetEvent(Overlapped.hEvent);
   WriteFile(hCom, SourceBuffer, (DWORD)SourceImageSize, &SentSize, &Overlapped);

   /* At this point the CPU is free to do other tasks and the incoming data */
   /* will be read during this time. Here, the CPU will only wait for the   */
   /* end of the data receive function.                                     */
   while(UserStruct.ReadPosition < SourceImageSize)
      {
      DoSomething = DoSomething;
      MosSleep(1);
      }

   /* Wait for the COM port to finish writing. */
   GetOverlappedResult(hCom, &Overlapped, &SentSize, TRUE);

   /* Display the result. */
   MosPrintf(MIL_TEXT("\n\nFinished reading data from hook function.\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Unhook functions. */
   MsysHookFunction(MilSystem, M_UART_DATA_RECEIVED + M_UNHOOK + M_UART_NB(Matrox.UartNumber),
                    ReadHook, (void *)(&UserStruct));

   /* Free allocations. */
   CloseHandle(hCom);
   CloseHandle(Overlapped.hEvent);
   MbufFree(MilReceivedImage);
   MbufFree(MilSourceImage);
   MdispFree(MilDisplaySource);
   MdispFree(MilDisplayReceive);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

void InitializeMatroxUart(MIL_ID *MilSystem, SystemInfo *SysInfo, MIL_INT *UartInterfaceType)
   {
   SystemInfo *Matrox = (SystemInfo *) SysInfo;

   /* Changing UART configuration on Matrox board. */
   /* M_UART_NB(M_DEVn) accesses the n UART number on this system. */
   MsysControl(*MilSystem,
               M_UART_PARITY         + M_UART_NB(Matrox->UartNumber), CONFIG_PARITY     );
   MsysControl(*MilSystem,
               M_UART_SPEED          + M_UART_NB(Matrox->UartNumber), CONFIG_SPEED      );
   MsysControl(*MilSystem,
               M_UART_DATA_SIZE      + M_UART_NB(Matrox->UartNumber), CONFIG_DATA_LENGTH);
   MsysControl(*MilSystem,
               M_UART_STOP_BITS      + M_UART_NB(Matrox->UartNumber), CONFIG_STOP_BITS  );
   MsysInquire(*MilSystem,
               M_UART_INTERFACE_TYPE + M_UART_NB(Matrox->UartNumber), UartInterfaceType);
   }

MIL_INT InitializeComPort(HANDLE *hCom, MIL_INT InterfaceType, SystemInfo *SysInfo)
   {
   SECURITY_ATTRIBUTES SecAttr;  /* Windows security attributes struct.    */
   COMMTIMEOUTS        Timeouts; /* Windows COM port timeouts struct.      */
   DCB                 dcb;      /* Windows COM port configuration struct. */

   SecAttr.lpSecurityDescriptor = NULL;
   SecAttr.bInheritHandle = TRUE;
   SecAttr.nLength = sizeof(SecAttr);

   /* Opening Windows COM port handle. */
   *hCom = CreateFile(SysInfo->ComPortName[0],
           GENERIC_READ | GENERIC_WRITE,
           FILE_SHARE_READ | FILE_SHARE_WRITE,
           &SecAttr,
           OPEN_EXISTING,
           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
           NULL);

   if(*hCom == INVALID_HANDLE_VALUE)
      {
      MosPrintf(MIL_TEXT(" Unable to open com port.\n"));
      return M_FALSE;
      }

   if(!GetCommState(*hCom, &dcb))
      {
      MosPrintf(MIL_TEXT(" Unable to get com port state.\n"));
      return M_FALSE;
      }

   if(!GetCommTimeouts(*hCom, &Timeouts))
      {
      MosPrintf(MIL_TEXT(" Unable to get com port timeouts.\n"));
      return M_FALSE;
      }

   /* Changing Windows COM port configuration. */
   dcb.BaudRate = CONFIG_SPEED;
   dcb.ByteSize = CONFIG_DATA_LENGTH;

   if(CONFIG_STOP_BITS == 1)
      dcb.StopBits = ONESTOPBIT;
   else if(CONFIG_STOP_BITS == 2)
      dcb.StopBits = TWOSTOPBITS;

   if(CONFIG_PARITY == M_DISABLE)
      dcb.Parity = NOPARITY;
   else if (CONFIG_PARITY == M_EVEN)
      dcb.Parity = EVENPARITY;
   else if (CONFIG_PARITY == M_ODD)
      dcb.Parity = ODDPARITY;

   if(InterfaceType == M_RS232)
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
   else if(InterfaceType == M_RS485)
      /* Specifies that the RTS line will be high if bytes are available for transmission.
      After all buffered bytes have been sent, the RTS line will be low.
      This is required for RS485 interfaces. */
      dcb.fRtsControl = RTS_CONTROL_TOGGLE;

   if (!SetCommState(*hCom, &dcb))
      {
      MosPrintf(MIL_TEXT("Unable to set com port to desired configuration.\n"));
      return M_FALSE;
      }

   /* Set default read and write timeouts (in ms.). */
   Timeouts.ReadIntervalTimeout           = 50;
   Timeouts.ReadTotalTimeoutConstant      = 0;
   Timeouts.ReadTotalTimeoutMultiplier    = 0;
   Timeouts.WriteTotalTimeoutConstant     = 0;
   Timeouts.WriteTotalTimeoutMultiplier   = 50;

   if(!SetCommTimeouts(*hCom, &Timeouts))
      {
      MosPrintf(MIL_TEXT("Unable to set com port timeouts.\n"));
      return M_FALSE;
      }

   return M_TRUE;
   }

void ReadFromComPort(HANDLE hCom, MIL_INT8 *Buffer, MIL_UINT SizeToRead,
                     OVERLAPPED *Overlapped)
   {
   MIL_UINT32 ReadSize = 0;
   MIL_UINT32 TransferSize = 0;

   ResetEvent(Overlapped->hEvent);

   while(TransferSize < SizeToRead)
      {
      /* Reading in 1KB blocks from the COM port. */
      ReadFile(hCom, Buffer+TransferSize, 1024, &ReadSize, Overlapped);
      GetOverlappedResult(hCom, Overlapped, &ReadSize, TRUE);
      TransferSize += ReadSize;
      MosPrintf(MIL_TEXT("%lld bytes read. (%.2f%% completed.)\r"),
                (long long)TransferSize, (float)TransferSize/(float)SizeToRead * 100.0);
      }
   }

void WriteToComPort(HANDLE hCom, MIL_INT8 *Buffer, MIL_UINT SizeToWrite,
                    OVERLAPPED *Overlapped)
   {
   MIL_UINT32 WriteSize = 0;
   MIL_UINT32 TransferSize = 0;
   MIL_UINT32 TransactionSize = 1024;

   ResetEvent(Overlapped->hEvent);

   while(TransferSize < SizeToWrite)
      {
      /* Writing in 1KB blocks with the COM port. */
      WriteFile(hCom, Buffer+TransferSize, TransactionSize, &WriteSize, Overlapped);
      GetOverlappedResult(hCom, Overlapped, &WriteSize, TRUE);
      TransferSize += WriteSize;
      MosPrintf(MIL_TEXT("%lld bytes written. (%.2f%% completed.)\r"),
                (long long)TransferSize, (float)TransferSize/(float)SizeToWrite * 100.0);

      if(SizeToWrite - TransferSize < TransactionSize)
         TransactionSize = (MIL_UINT32)(SizeToWrite - TransferSize);
      }
   }

/* UART read hook function. */
MIL_INT MFTYPE ReadHook(MIL_INT HookType, MIL_ID EventId, void* UserStructPtr)
   {
   UartHook *Params = (UartHook*) UserStructPtr;
   MIL_INT Pending = 0;
   MIL_UINT lNumBytesTransferred = 0;

   /* Inquire the number of bytes pending in the UART receive buffer. */
   MsysInquire(Params->SystemId, M_UART_DATA_PENDING + M_UART_NB(Params->Matrox->UartNumber),
               &Pending);

   while(Pending)
      {
      /* Read the data that is pending in the UART receive buffer. */
      MsysControl(Params->SystemId,
                  M_UART_READ_STRING_SIZE + M_UART_NB(Params->Matrox->UartNumber), Pending);
      MsysControl(Params->SystemId, M_UART_READ_STRING + M_UART_NB(Params->Matrox->UartNumber),
                  Params->ReceiveBuffer + Params->ReadPosition);
      lNumBytesTransferred = 0;
      MsysInquire(Params->SystemId, M_UART_BYTES_READ + M_UART_NB(Params->Matrox->UartNumber),
                  &lNumBytesTransferred);

      /* Notify buffer that it has been modified. */
      MbufControl(Params->ReceiveBufferId, M_MODIFIED, M_DEFAULT);

      /* Adjust read position and continue. */
      Params->ReadPosition += lNumBytesTransferred;
      MosPrintf(MIL_TEXT("%lld bytes read. (%.2f%% completed.)\r"), (long long)Params->ReadPosition,
                (float)Params->ReadPosition / (float)Params->ReadSize * 100.0);
      MsysInquire(Params->SystemId,
                  M_UART_DATA_PENDING + M_UART_NB(Params->Matrox->UartNumber), &Pending);
      }
   return 0;
   }

bool EnumerateComPorts( SystemInfo *SysInfo )
   {
   SystemInfo *Matrox = (SystemInfo *) SysInfo; /* Pointer to SysInfo structure.    */
   MIL_ID   MilApplication,                     /* MIL Application identifier.      */
            MilSystem;                          /* MIL System identifier.           */
   MIL_INT  ComPortNumber = 0,                  /* Inquired COM port number.        */
            MaxPortNumber = 0,                  /* Remember the highest port.       */
            MinPortNumber = MAX_PORTS,          /* Remember the lowest port.        */
            MatroxDevice[MAX_PORTS],            /* Storage for device numbers.      */
            MatroxUartNumber[MAX_PORTS];        /* Storage for UART numbers.        */
   int      NameIndex,                          /* System to allocate.              */
            DeviceNumber,                       /* Device number to allocate.       */
            UartNumber,                         /* UART number.                     */
            NumberOfPortsFound = 0,             /* Remember number of ports found.  */
            PortNumberExists[MAX_PORTS] = {0},  /* Remember which ports were found. */
            EntryCount = 0,                     /* Used to enumerate Windows ports. */
            nIndex[2],                          /* Used in for loops.               */
            UserChoice = 0,                     /* Used to store user input.        */
            MatroxPortIndex = -1,               /* Used as an index pointer.        */
            WindowsPortIndex = -1,              /* Used as an index pointer.        */
            SelectedMatroxPort = -1,            /* Used to store user selection.    */
            SelectedWindowsPort = -1;           /* Used to store user selection.    */

   /* Registry specific variables. */
   const HKEY  MAIN_KEY = HKEY_LOCAL_MACHINE;
   MIL_CONST_TEXT_PTR SUB_KEY = MIL_TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");
   const       REGSAM KEY_PERMISSIONS = KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE;
   FILETIME    DummyFileTime;
   DWORD       DummyLength = MAX_PATH;
   HKEY        CurKey;

   /* Enumeration specific variables. */
   const unsigned MAX_SYSTEM_TYPE = 4;
   MIL_TEXT_CHAR  EntryName[MAX_PORTS][MAX_PATH] = {MIL_TEXT("")};
   MIL_TEXT_CHAR  EntryData[MAX_PORTS][MAX_PATH] = {MIL_TEXT("")};
   MIL_TEXT_CHAR  TempSignalFormat[1][MAX_PATH] = {MIL_TEXT("")};
   MIL_TEXT_CHAR  MilSystemNames[MAX_SYSTEM_TYPE][25] = {{M_SYSTEM_MORPHIS},
                                           {M_SYSTEM_SOLIOS},
                                           {M_SYSTEM_RAPIXOCL},
                                           {M_SYSTEM_RADIENTEVCL}
                                           };
   MIL_TEXT_CHAR  SystemNames[MAX_SYSTEM_TYPE][25] = {{MIL_TEXT("Matrox Morphis")},
                                        {MIL_TEXT("Matrox Solios")},
                                        {MIL_TEXT("Matrox Rapixo CL")},
                                        {MIL_TEXT("Matrox Radient eV")}
                                        };
   MIL_TEXT_CHAR  MatroxPorts[MAX_PORTS][MAX_PATH];
   MIL_TEXT_CHAR  WindowsPorts[MAX_PORTS][MAX_PATH] = {{0}};
   MIL_TEXT_CHAR  MatroxSignalFormat[MAX_PORTS][MAX_PATH];
   MIL_TEXT_CHAR  MatroxBoardName[MAX_PORTS][MAX_PATH];
   MIL_TEXT_CHAR  MatroxSystem[MAX_PORTS][MAX_PATH];

   /* Enumerating Matrox COM ports.  */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MosPrintf(MIL_TEXT("Enumerating Matrox COM ports.\n"));
   MosPrintf(MIL_TEXT("-----------------------------------------------------\n"));
   for(NameIndex = 0; NameIndex<MAX_SYSTEM_TYPE; NameIndex++)
      {
      for(DeviceNumber=0; DeviceNumber<M_DEV16; DeviceNumber++)
         {
         MsysAlloc(M_DEFAULT, MilSystemNames[NameIndex], M_DEV0 + DeviceNumber, M_DEFAULT, &MilSystem);
         if(MilSystem)
            {
            MIL_INT SystemType;
            MIL_INT BoardType;
            MIL_TEXT_CHAR BoardName[1][50];

            MsysInquire(MilSystem, M_SYSTEM_TYPE, &SystemType);
            MsysInquire(MilSystem, M_BOARD_TYPE, &BoardType);

            switch(SystemType)
               {
               case M_SYSTEM_SOLIOS_TYPE:
               case M_SYSTEM_RADIENTEVCL_TYPE:
               case M_SYSTEM_RAPIXOCL_TYPE:
                  if ((BoardType & M_CL) == M_CL)
                     {
                     MosStrcpy(TempSignalFormat[0], MAX_PATH, MIL_TEXT("LVDS"));
                     MosSprintf(BoardName[0], 50, MIL_TEXT("%s/CL"), SystemNames[NameIndex]);
                     if ((BoardType & M_SFCL) == M_SFCL)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Full"));
                     else if ((BoardType & M_DBCL) == M_DBCL)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Dual Base"));
                     else if ((BoardType & M_SMCL  ) == M_SMCL)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Medium"));
                     }
                  else if((BoardType & M_XA) == M_XA)
                     {
                     MosStrcpy(TempSignalFormat[0], MAX_PATH, MIL_TEXT("RS232"));
                     MosSprintf(BoardName[0], 50, MIL_TEXT("%s/XA"), SystemNames[NameIndex]);
                     if ((BoardType & M_QA) == M_QA)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Quad"));
                     else if ((BoardType & M_DA) == M_DA)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Dual"));
                     else if ((BoardType & M_SA) == M_SA)
                        MosStrcat(BoardName[0], 50, MIL_TEXT(" Single"));
                     }
                  break;

               case M_SYSTEM_MORPHIS_TYPE:
                  MosStrcpy(TempSignalFormat[0], MAX_PATH, MIL_TEXT("RS485"));
                  MosSprintf(BoardName[0], 50, MIL_TEXT("%s"), SystemNames[NameIndex]);
                  if ((BoardType & M_2VD) == M_2VD)
                     MosStrcat(BoardName[0], 50, MIL_TEXT(" 2VD (Dual)"));
                  else if ((BoardType & M_4VD) == M_4VD)
                     MosStrcat(BoardName[0], 50, MIL_TEXT(" 4VD (Quad)"));

               default:
                  // nothing to do.
                  break;
               }

            for(UartNumber=0; UartNumber<4; UartNumber++)
               {
               ComPortNumber = 0;
               MsysInquire(MilSystem, M_COM_PORT_NUMBER+M_UART_NB(UartNumber), &ComPortNumber);
               if(ComPortNumber)
                  {
                  MIL_TEXT_CHAR ComPort[50];
                  PortNumberExists[NumberOfPortsFound++] = (int)ComPortNumber;
                  MosPrintf(MIL_TEXT("%2d) "), MatroxPortIndex + 2);
                  MosSprintf(ComPort, 50, MIL_TEXT("COM%d"), (int)ComPortNumber);
                  MatroxPortIndex++;
                  MatroxDevice[MatroxPortIndex] = DeviceNumber;
                  MatroxUartNumber[MatroxPortIndex] = UartNumber;
                  MosStrcpy(MatroxSystem[MatroxPortIndex], MAX_PATH,
                            MilSystemNames[NameIndex]);
                  MosStrcpy(MatroxSignalFormat[MatroxPortIndex], MAX_PATH,
                            TempSignalFormat[0]);
                  MosStrcpy(MatroxPorts[MatroxPortIndex], MAX_PATH, ComPort);
                  MosStrcpy(MatroxBoardName[MatroxPortIndex], MAX_PATH, BoardName[0]);
                  MosPrintf(MIL_TEXT("%s\t: DEV%d, UART%d = %s\n"),
                            BoardName[0], (int)DeviceNumber, (int)UartNumber, ComPort);
                  if (ComPortNumber > MaxPortNumber)
                     MaxPortNumber = ComPortNumber;
                  if (ComPortNumber < MinPortNumber)
                     MinPortNumber = ComPortNumber;
                  }
               else
                  break;
               }
            MsysFree(MilSystem);
            MilSystem = M_NULL;
            }
         else
            break;
         }
      }
   MosPrintf(MIL_TEXT("\n"));
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);
   MappFree(MilApplication);

   /* Enumerating Windows COM ports.  */
   MosPrintf(MIL_TEXT("Enumerating Windows COM ports.\n"));
   MosPrintf(MIL_TEXT("-------------------------------\n"));
   RegOpenKeyEx(MAIN_KEY, SUB_KEY, 0, KEY_PERMISSIONS, &CurKey);
   while (RegEnumValue(CurKey,
                       EntryCount,
                       EntryName[EntryCount],
                       &DummyLength,
                       NULL,
                       NULL,
                       NULL,
                       (MIL_UINT32 *) &DummyFileTime ) != ERROR_NO_MORE_ITEMS)
      {
      EntryCount++;
      DummyLength = MAX_PATH;
      }

   /* Get data from the entries. */
   for (nIndex[0] = 0; nIndex[0] < EntryCount; nIndex[0]++)
      {
      int nEntryExists = 0;
      DummyLength = MAX_PATH * sizeof(MIL_TEXT_CHAR);
      RegQueryValueEx(CurKey,
                      EntryName[nIndex[0]],
                      NULL,
                      NULL,
                      (LPBYTE)EntryData[nIndex[0]],
                      &DummyLength);

      for (nIndex[1] = 0; nIndex[1] <= (int) MatroxPortIndex; nIndex[1]++)
         {
         if (MosStrcmp(EntryData[nIndex[0]], MatroxPorts[nIndex[1]]) == 0)
            nEntryExists = 1;
         }
      if (!nEntryExists)
         {
         WindowsPortIndex++;
         MosStrcpy(WindowsPorts[WindowsPortIndex], MAX_PATH, EntryData[nIndex[0]]);
         MosPrintf(MIL_TEXT("%2d) "), MatroxPortIndex + WindowsPortIndex + 2);
         MosPrintf(MIL_TEXT("%s "), EntryName[nIndex[0]]);
         MosPrintf(MIL_TEXT(" \t= %s\n"), EntryData[nIndex[0]]);
         }
      }

   MosPrintf(MIL_TEXT("\n"));
   RegCloseKey(CurKey);

   /* Get the desired Matrox COM port number for the source. */
   MosPrintf(MIL_TEXT("Please specify the Matrox COM Port index to use.\n"));
   char UserInput[32];
   do
      {
      MosPrintf(MIL_TEXT("Valid entries are from 1 to %d, (Q)uit: "), (int)(MatroxPortIndex + 1));
      scanf("%s", UserInput);
      sscanf(UserInput, "%d", &UserChoice);
      if (UserChoice >= 1 && UserChoice <= (int)MatroxPortIndex + 1)
         {
         MosPrintf(MIL_TEXT("\n%d) %s, DEV%d, %s selected.\n"),
                    (int)UserChoice,
                    MatroxBoardName[UserChoice - 1],
                    (int)(MatroxDevice[UserChoice - 1]),
                    MatroxPorts[UserChoice - 1]);
         SelectedMatroxPort = UserChoice - 1;

         }
      else
         {
         char inputChar;
         sscanf(UserInput, "%c", &inputChar);
         if(inputChar == 'q'  ||
            inputChar == 'Q'  )
            {
            UserChoice = -1;
            }
         else
            {
            MosPrintf(MIL_TEXT("Invalid selection. "));
            UserChoice = 0;
            }
         }
      }while (UserChoice == 0);

   if(UserChoice != -1)
      {
      /* Get the second COM port to complete the transfer. */
      MosPrintf(MIL_TEXT("\nPlease specify the other COM Port index to use.\n"));
      UserChoice = 0;
      char UserInput[32];
      do
         {
         MosPrintf(MIL_TEXT("Valid entries are from 1 to %d, (Q)uit: "),
            (int)((MatroxPortIndex + 1) + (WindowsPortIndex + 1)));
         scanf("%s", UserInput);
         sscanf(UserInput, "%d", &UserChoice);
         if ((UserChoice >= 1) &&
            (UserChoice <= (int)MatroxPortIndex + 1 + (int)WindowsPortIndex + 1))
            {
            if ((UserChoice - 1) > (int)MatroxPortIndex)
               MosPrintf(MIL_TEXT("\n%d) Windows %s selected.\n"),
               (int)UserChoice,
               WindowsPorts[(UserChoice - 1) - (MatroxPortIndex) - 1]);
            else
               MosPrintf(MIL_TEXT("\n%d) %s, DEV%d, %s selected.\n"),
               (int)UserChoice,
               MatroxBoardName[UserChoice - 1],
               (int)MatroxDevice[UserChoice - 1],
               MatroxPorts[UserChoice - 1]);

            /* Make sure that a different COM port was selected. */
            if (UserChoice == (int)SelectedMatroxPort + 1)
               {
               MosPrintf(MIL_TEXT("\nYou can not select the same port twice. Select a different ")
                  MIL_TEXT("COM port.\n"));
               UserChoice = 0;
               }

            /* Make sure signal formats are the same. */
            else
               {
               MIL_TEXT_CHAR TempSignalFormat[1][MAX_PATH];
               if ((UserChoice - 1) > (int)MatroxPortIndex)
                  MosStrcpy(TempSignalFormat[0], MAX_PATH, MIL_TEXT("RS232"));
               else
                  MosStrcpy(TempSignalFormat[0], MAX_PATH, MatroxSignalFormat[UserChoice - 1]);
               if (MosStrcmp(TempSignalFormat[0], MatroxSignalFormat[SelectedMatroxPort]) != 0)
                  {
                  MosPrintf(MIL_TEXT("\nIncompatible formats. \n"));

                  if ((UserChoice - 1) > (int)MatroxPortIndex)
                     MosPrintf(MIL_TEXT("The Windows %s uses the RS232 format.\nThe %s uses the ")
                     MIL_TEXT("%s format. \n"),
                     WindowsPorts[(UserChoice - 1) - (MatroxPortIndex) - 1],
                     MatroxBoardName[SelectedMatroxPort],
                     MatroxSignalFormat[SelectedMatroxPort]);
                  else
                     MosPrintf(MIL_TEXT("The %s uses the %s format.\n")
                     MIL_TEXT("The %s uses the %s format. \n"),
                     MatroxBoardName[UserChoice - 1],
                     MatroxSignalFormat[UserChoice - 1],
                     MatroxBoardName[SelectedMatroxPort],
                     MatroxSignalFormat[SelectedMatroxPort]);
                  MosPrintf(MIL_TEXT("Select a compatible board.\n\n"));
                  UserChoice = 0;
                  }
               else
                  {
                  if ((UserChoice - 1) > (int)MatroxPortIndex)
                     {
                     SelectedWindowsPort = (UserChoice - 1) - (MatroxPortIndex) - 1;
                     MosSprintf(Matrox->ComPortName[0], MAX_PATH, MIL_TEXT("\\\\.\\%s"),
                        WindowsPorts[SelectedWindowsPort]);
                     }
                  else
                     {
                     SelectedWindowsPort = (UserChoice - 1) + (MatroxPortIndex);
                     MosSprintf(Matrox->ComPortName[0], MAX_PATH, MIL_TEXT("\\\\.\\%s"),
                        MatroxPorts[SelectedWindowsPort - MatroxPortIndex]);
                     }
                  }
               }
            }
         else
            {
            char inputChar;
            sscanf(UserInput, "%c", &inputChar);
            if(inputChar == 'q'  ||
               inputChar == 'Q'  )
               {
               UserChoice = -1;
               }
            else
               {
               MosPrintf(MIL_TEXT("Invalid selection. "));
               UserChoice = 0;
               }
            }
         }while (UserChoice == 0);

         if(UserChoice != -1)
            {
            MosPrintf(MIL_TEXT("\n"));

            /* Fill structure with user's entries. */
            MosStrcpy(Matrox->System[0], MAX_PATH, MatroxSystem[SelectedMatroxPort]);
            Matrox->Device = MatroxDevice[SelectedMatroxPort];
            Matrox->UartNumber = MatroxUartNumber[SelectedMatroxPort];
            }
      }

   if(UserChoice == -1)
      return false;
   else
      return true;
   }