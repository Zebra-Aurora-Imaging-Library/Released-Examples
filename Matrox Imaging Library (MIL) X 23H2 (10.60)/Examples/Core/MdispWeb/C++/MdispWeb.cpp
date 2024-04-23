﻿/***************************************************************************************/
/*
 * File name: MdispWeb.cpp
 *
 * Synopsis:  This program shows how to use web publishing.
 *            An html file is provided to see content in a web browser.
 *
 *
 *      Note: The example is a modified version of MdigProcess example.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>
#include <string>

#define HTTP_SERVER_PORT 9001L

#if M_MIL_USE_WINDOWS
#include <windows.h>
static HANDLE MdispWebClientHandle = NULL;
#else
static pid_t MdispWebClientHandle = 0;
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

/* Uncomment this line to allow remote access when starting http server */
/* needs admin permission */
//#define ALLOW_REMOTE_ACCESS 1

/* Number of images in the buffering grab queue.
   Generally, increasing this number gives a better real-time grab.
*/
#define BUFFERING_SIZE_MAX 22

#define TEXT_SIZE  2048

enum AppEnum {WEB_CLIENT=0, WEB_BROWSER, WEB_CSHARP_FORM, WEB_VB_FORM, WEB_GTK_WEBKIT};

void LaunchApplication(AppEnum AppType);
void StartHttpServer(MIL_ID& HttpServerId);
bool FoundApplication(AppEnum AppType);


/* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
void StartGrab(MIL_ID MilDigitizer, MIL_ID MilSystem, MIL_ID MilMessage, MIL_ID MilImageDisp);



/* Main function. */
/* ---------------*/

int main(int argc, char *argv[])
   {
   MIL_ID MilApplication = M_NULL;
   MIL_ID MilSystem      = M_NULL;
   MIL_ID MilDigitizer   = M_NULL;
   MIL_ID MilDisplay     = M_NULL;
   MIL_ID MilMessage     = M_NULL;
   MIL_ID MilImageDisp   = M_NULL;
   MIL_ID MilHttpServer  = M_NULL;
   MIL_INT Selection = -1;
   MIL_INT Done = 0;
   
   char TextAscii[TEXT_SIZE];
   memset(TextAscii, 0, TEXT_SIZE);

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, M_NULL,
                    &MilDigitizer, M_NULL);

   if(MsysInquire(MilSystem, M_LOCATION, M_NULL) == M_REMOTE)
      {
      MosPrintf(MIL_TEXT("This example is not supported on a DMIL system (Distributed MIL)\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);      
      return 0;
      }
   
   /* Allow web publishing */
   MappControl(M_DEFAULT, M_WEB_CONNECTION, M_ENABLE);
	  	
   MIL_INT ImageSizeX = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);
   MIL_INT SizeBand   = MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL);
   
   MdispAlloc(MilSystem, M_DEFAULT,MIL_TEXT("M_DEFAULT"),M_WEB, &MilDisplay);
   /* Publish the display */
   MobjControl(MilDisplay, M_OBJECT_NAME, MIL_TEXT("Display"));
   MobjControl(MilDisplay, M_WEB_PUBLISH, M_READ_ONLY);

   /* Allocate a display image and show it */
   MbufAllocColor(MilSystem, SizeBand, ImageSizeX, ImageSizeY, 8, M_IMAGE+M_DISP+M_PROC+M_GRAB, &MilImageDisp);
   MdispSelect(MilDisplay, MilImageDisp);  
     
   /* Allocate a MIL message */
   MobjAlloc(MilSystem, M_MESSAGE_MAILBOX, M_OVERWRITE, &MilMessage);
   MobjControl( MilMessage, M_OBJECT_NAME, MIL_TEXT("Message")); // Message to send to clients
   MobjControl( MilMessage, M_WEB_PUBLISH, M_READ_ONLY);

    /* Start the http server */
   StartHttpServer(MilHttpServer);
   
   do
      {
      
      /* Print a message. */
      MosPrintf(MIL_TEXT("\n\n"));
      MosPrintf(MIL_TEXT("This example demonstrates how to publish various MIL objects\nusing the MIL web API.\n"));
      MosPrintf(MIL_TEXT("It also shows how to access them from different types of external clients.\n"));
      MosPrintf(MIL_TEXT("A web browser using the MIL javascript API and a standalone application\n"));
      MosPrintf(MIL_TEXT("that uses the MIL web C/C++ API.\n\n"));
   
      /* Launch client example.*/
      do
         {
         MosPrintf(MIL_TEXT("Select an option to visualize the published objects:\n"));
         MosPrintf(MIL_TEXT("1) A local web browser showing: \"http://localhost:%ld/mdispweb.html\".\n"), HTTP_SERVER_PORT);
         if(FoundApplication(WEB_CLIENT))
            MosPrintf(MIL_TEXT("2) A standalone C/C++ desktop client application. \n"));
#if !M_MIL_USE_LINUX
         if(FoundApplication(WEB_CSHARP_FORM))
            MosPrintf(MIL_TEXT("3) A C# web browser form client application. \n"));
         if(FoundApplication(WEB_VB_FORM))
            MosPrintf(MIL_TEXT("4) A VB web browser form client application. \n"));
#else
         if(FoundApplication(WEB_GTK_WEBKIT))
            MosPrintf(MIL_TEXT("3) A Gtk WebKit browser client application. \n"));
#endif
         MosPrintf(MIL_TEXT("0) End the example.\n"));
                       
         Selection = MosGetch();
         Done = 1;
         switch(Selection)
            {
            case '1':
               {
               LaunchApplication(WEB_BROWSER);
               }
               break;
	
            case '2':
               if(FoundApplication(WEB_CLIENT))
                  LaunchApplication(WEB_CLIENT);
               else
                  {
                  MosPrintf(MIL_TEXT("\nInvalid selection !.\n\n"));
                  Done = 0;
                  }
               break;
#if !M_MIL_USE_LINUX
            case '3':
               if(FoundApplication(WEB_CSHARP_FORM))
                  LaunchApplication(WEB_CSHARP_FORM);
               else
                  {
                  MosPrintf(MIL_TEXT("\nInvalid selection !.\n\n"));
                  Done = 0;
                  }
               break;

            case '4':
               if(FoundApplication(WEB_VB_FORM))
                  LaunchApplication(WEB_VB_FORM);
               else
                  {
                  MosPrintf(MIL_TEXT("\nInvalid selection !.\n\n"));
                  Done = 0;
                  }
               break;
#else
            case '3':
               if(FoundApplication(WEB_GTK_WEBKIT))
                  LaunchApplication(WEB_GTK_WEBKIT);
               else
                  {
                  MosPrintf(MIL_TEXT("\nInvalid selection !.\n\n"));
                  Done = 0;
                  }
               break;
               
#endif
            case '0':
               break;
         
            default:
               MosPrintf(MIL_TEXT("\nInvalid selection !.\n\n"));
               Done = 0;
               break;
            }
         } while(!Done);

      if(Selection != '0')
         {
         MobjMessageWrite(MilMessage, (MIL_UINT8 *)TextAscii, TEXT_SIZE, 1, M_DEFAULT);
         StartGrab(MilDigitizer,MilSystem,MilMessage,MilImageDisp);
         }
      
      /* close web c/c++ client application */
      if (MdispWebClientHandle)
         {
#if M_MIL_USE_WINDOWS
         TerminateProcess(MdispWebClientHandle, NULL);
         CloseHandle(MdispWebClientHandle);
#else
         kill(MdispWebClientHandle, SIGTERM);
#endif
         MdispWebClientHandle = M_NULL;
         }


       /* empty message */
      MobjMessageWrite(MilMessage, (MIL_UINT8 *)TextAscii, TEXT_SIZE, 2, M_DEFAULT);

      /* stop http server */

#if M_MIL_USE_WINDOWS
   system("cls");
#else
   system("clear");
#endif
      
      } while(Selection != '0');
   
   MobjFree(MilHttpServer);
   MobjFree(MilMessage);
   MbufFree(MilImageDisp);
   MdispFree(MilDisplay);

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);


   return 0;
   }

/* User's processing function hook data structure. */
typedef struct
   {
      MIL_ID  MilDigitizer;
      MIL_ID  MilImageDisp;
      MIL_ID  MilMessage;
      MIL_INT ProcessedImageCount;
   } HookDataStruct;
 
/* Start the Grab */
void StartGrab(MIL_ID MilDigitizer, MIL_ID MilSystem, MIL_ID MilMessage, MIL_ID MilImageDisp)
   {
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount   = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   MIL_INT NbFrames = 0;
   HookDataStruct UserHookData;

   MosPrintf(MIL_TEXT("\nLIVE GRAB BEING PUBLISHED.\n\n"));
   MosPrintf(MIL_TEXT("-----------------------------\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start processing.\n\n"));

   /* Grab continuously on the display and wait for a key press. */
   MdigGrabContinuous(MilDigitizer, MilImageDisp);
   MosGetch();

   /* Halt continuous grab. */
   MdigHalt(MilDigitizer);

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0; MilGrabBufferListSize<BUFFERING_SIZE_MAX;
        MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
                  MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                  MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                  8 + M_UNSIGNED,
                  M_IMAGE + M_GRAB + M_PROC,
                  &MilGrabBufferList[MilGrabBufferListSize]);
      if (MilGrabBufferList[MilGrabBufferListSize])
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);


   /* Initialize the user's processing function data structure. */
   UserHookData.MilDigitizer        = MilDigitizer;
   UserHookData.MilImageDisp        = MilImageDisp;
   UserHookData.MilMessage          = MilMessage;
   UserHookData.ProcessedImageCount = 0;

   /* Start the processing. The processing function is called with every frame grabbed. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_START, M_DEFAULT, ProcessingFunction, &UserHookData);


   /* Here the main() is free to perform other tasks while the processing is executing. */
   /* --------------------------------------------------------------------------------- */


   /* Print a message and wait for a key press after a minimum number of frames. */
   MosPrintf(MIL_TEXT("\nLIVE PROCESSING BEING PUBLISHED.\n"));
   MosPrintf(MIL_TEXT("-----------------------------\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to stop.        \n\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_STOP, M_DEFAULT, ProcessingFunction, &UserHookData);

   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT,  &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE,   &ProcessFrameRate);
   MosPrintf(MIL_TEXT("\n\n%d frames grabbed at %.1f frames/sec (%.1f ms/frame).\n"),
             (int)ProcessFrameCount, ProcessFrameRate, 1000.0/ProcessFrameRate);
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();
   
   /* Free the grab buffers. */
   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);

   }

/* User's processing function called every time a grab buffer is ready. */
/* -------------------------------------------------------------------- */

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX]= {MIL_TEXT('\0'),};


   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);

   /* Increment the frame counter. */
   UserHookDataPtr->ProcessedImageCount++;

   /* Print and draw the frame count (remove to reduce CPU usage). */
   MosPrintf(MIL_TEXT("Processing frame #%d.\r"), (int)UserHookDataPtr->ProcessedImageCount);
   MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%d"), 
              (int)UserHookDataPtr->ProcessedImageCount);
   MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

   char TextAscii[TEXT_SIZE];
   memset(TextAscii, 0, TEXT_SIZE);
   sprintf(TextAscii, "Processing frame #%d",(int)UserHookDataPtr->ProcessedImageCount);
   MobjMessageWrite(UserHookDataPtr->MilMessage, (MIL_UINT8 *)TextAscii, TEXT_SIZE, 1, M_DEFAULT);
   
   /* Execute the processing and update the display. */
   MimArith(ModifiedBufferId, M_NULL, UserHookDataPtr->MilImageDisp, M_NOT);

   return 0;
   }   
                
/* Launch a specifc application. */
/* ----------------------------- */
void LaunchApplication(AppEnum AppType)
   {
   MIL_TEXT_CHAR Buffer[TEXT_SIZE];
   memset(Buffer,0,TEXT_SIZE);
   MIL_STRING ExamplePath;
   MappInquire(M_DEFAULT, M_MIL_DIRECTORY_EXAMPLES, ExamplePath);
   
#if M_MIL_USE_LINUX
   if(AppType == WEB_CLIENT)
      {
      ExamplePath+= MIL_TEXT("Core/MdispWebClient/C++/MdispWebClient");
      MdispWebClientHandle = fork();
      if(MdispWebClientHandle < 0)
         {
         MosPrintf(MIL_TEXT("Launching MdispWebClient failed !!! (%s)\n"), strerror(errno));
         MdispWebClientHandle = 0;
         }
      else if (MdispWebClientHandle == 0)
         {
         execl (ExamplePath.c_str(), "MdispWebClient", "ws://localhost:7681", NULL);
         MosPrintf(MIL_TEXT("Cannot start C/++ client example !!! (%s)\n"), strerror(errno));
         }
      }  
   else if(AppType == WEB_BROWSER)
      {
      if (access("/usr/bin/gio", X_OK) == 0)
         MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("gio open  http://localhost:%ld/mdispweb.html >/dev/null 2>/dev/null"), HTTP_SERVER_PORT);
      else
         MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("xdg-open  http://localhost:%ld/mdispweb.html >/dev/null 2>/dev/null"), HTTP_SERVER_PORT);
      system(Buffer);
      }
   else if(AppType == WEB_GTK_WEBKIT)
      {
      ExamplePath+= MIL_TEXT("Core/MdispWebKitGtk/C++/MdispWebKitGtk");
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("http://localhost:%ld/mdispweb.html"), HTTP_SERVER_PORT);
      MdispWebClientHandle = fork();
      if(MdispWebClientHandle < 0)
         {
         MosPrintf(MIL_TEXT("Launching MdispWebClient failed !!! (%s)\n"), strerror(errno));
         MdispWebClientHandle = 0;
         }
      else if (MdispWebClientHandle == 0)
         {
#if M_MIL_USE_ARM
	 // On some Arm boards the Mesa/OpenGL is not correctly configured which generates this error : 
	 // libEGL warning: DRI2: failed to authenticate
	 // Disable it to fix GDK rendering.
         setenv("LIBGL_DRI2_DISABLE", "true",1);
#endif	 
         close(0);
         close(1);
         close(2);
         int fd0,fd1,fd2;
         // attach standard file descriptors to /dev/null
         fd0 = open("/dev/null", O_RDWR);
         fd1 = dup(fd0);
         fd2 = dup(fd0) ;
         execl (ExamplePath.c_str(), "MdispWebKitGtk", Buffer, NULL);
         MosPrintf(MIL_TEXT("Cannot start Webkit Gtk example !!! (%s)\n"), strerror(errno));
         }
      }
   else
      {
      MosPrintf(MIL_TEXT("Invalid application type !!!\n"));
      }
#else
   if(AppType == WEB_CLIENT)
      {
      ExamplePath+=MIL_TEXT("Core\\MdispWebClient\\C++\\precompiled\\MdispWebClient.exe");
      STARTUPINFO StartInfo;
      PROCESS_INFORMATION ProcessInfo;
      memset(&StartInfo, 0, sizeof(StartInfo));
      memset(&ProcessInfo, 0, sizeof(ProcessInfo));
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("ws://localhost:%ld"), HTTP_SERVER_PORT);
      if(CreateProcess(ExamplePath.c_str(), Buffer, 0, 0, 0, 0, 0, 0, &StartInfo, &ProcessInfo))
         MdispWebClientHandle = ProcessInfo.hProcess;
      else
         MosPrintf(MIL_TEXT("Cannot start C/++ client example !!!\n"));
      
      }
   else if(AppType == WEB_CSHARP_FORM)
      {
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("%sCore\\MdispWebForm\\C#\\precompiled\\MdispWebForm.exe http://localhost:%ld/mdispweb.html"), ExamplePath.c_str(), HTTP_SERVER_PORT);
      STARTUPINFO StartInfo;
      PROCESS_INFORMATION ProcessInfo;
      memset(&StartInfo, 0, sizeof(StartInfo));
      memset(&ProcessInfo, 0, sizeof(ProcessInfo));
      if(CreateProcess(0, Buffer, 0, 0, 0, 0, 0, 0, &StartInfo, &ProcessInfo))
         MdispWebClientHandle = ProcessInfo.hProcess;
      else
         MosPrintf(MIL_TEXT("Cannot start C# web browser form example !!!\n"));
      }
   else if(AppType == WEB_VB_FORM)
      {
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("%sCore\\MdispWebForm\\VB\\precompiled\\MdispWebForm.exe http://localhost:%ld/mdispweb.html"),ExamplePath.c_str(), HTTP_SERVER_PORT);
      STARTUPINFO StartInfo;
      PROCESS_INFORMATION ProcessInfo;
      memset(&StartInfo, 0, sizeof(StartInfo));
      memset(&ProcessInfo, 0, sizeof(ProcessInfo));
      if(CreateProcess(0, Buffer,0, 0, 0, 0, 0, 0, &StartInfo, &ProcessInfo))
         MdispWebClientHandle = ProcessInfo.hProcess;
      else
         MosPrintf(MIL_TEXT("Cannot start VB web browser form example !!!\n"));
      }  
   else if(AppType == WEB_BROWSER)
      {     
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("http://localhost:%ld/mdispweb.html"), HTTP_SERVER_PORT);
      ShellExecute(NULL, NULL, Buffer, NULL, NULL, SW_SHOWNORMAL);     
      }
   else
      {
      MosPrintf(MIL_TEXT("Invalid application type !!!\n"));
      }      
#endif
   }

/* Strat the Http Server. */
/* ----------------------------- */
void StartHttpServer(MIL_ID& HttpServerId)
   {
   MIL_TEXT_CHAR Buffer[TEXT_SIZE];
   memset(Buffer,0,TEXT_SIZE);
   MIL_STRING ExamplePath;
   MappInquire(M_DEFAULT, M_MIL_DIRECTORY_EXAMPLES, ExamplePath);

   /* Allocate the Http Server */
   HttpServerId = MobjAlloc(M_DEFAULT_HOST, M_HTTP_SERVER, M_DEFAULT, M_NULL);
#if ALLOW_REMOTE_ACCESS
   MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("http://0.0.0.0:%ld"), HTTP_SERVER_PORT);
   MobjControl(HttpServerId, M_HTTP_ADDRESS, Buffer);
#else
   MobjControl(HttpServerId, M_HTTP_PORT, HTTP_SERVER_PORT);
#endif
   /* set http root document */
   ExamplePath+=MIL_TEXT("Core/MdispWebClient/C++/html");
   MobjControl(HttpServerId, M_HTTP_ROOT_DIRECTORY, ExamplePath);
   MobjControl(HttpServerId, M_HTTP_START, M_DEFAULT);  
   }

/* Found a specifc application. */
/* ----------------------------- */
bool FoundApplication(AppEnum AppType)
   {
   MIL_INT FileExists = M_NO;
   MIL_STRING ExamplePath;
   MappInquire(M_DEFAULT, M_MIL_DIRECTORY_EXAMPLES, ExamplePath);

#if M_MIL_USE_LINUX   
   if(AppType == WEB_CLIENT)
      {
      ExamplePath+= MIL_TEXT("Core/MdispWebClient/C++/MdispWebClient");
      MappFileOperation(M_DEFAULT, ExamplePath, M_NULL, M_NULL, M_FILE_EXISTS, M_NULL, &FileExists);
      }
   else if(AppType == WEB_BROWSER)
      {
      return true;
      }
   else if(AppType == WEB_GTK_WEBKIT)
      {
      ExamplePath+= MIL_TEXT("Core/MdispWebKitGtk/C++/MdispWebKitGtk");
      MappFileOperation(M_DEFAULT, ExamplePath, M_NULL, M_NULL, M_FILE_EXISTS, M_NULL, &FileExists);
      }
#else
   if(AppType == WEB_CLIENT)
      {
      ExamplePath+= MIL_TEXT("Core\\MdispWebClient\\C++\\precompiled\\MdispWebClient.exe");
      MappFileOperation(M_DEFAULT, ExamplePath, M_NULL, M_NULL, M_FILE_EXISTS, M_NULL, &FileExists);
      }
   else if(AppType == WEB_CSHARP_FORM)
      {
      ExamplePath+= MIL_TEXT("Core\\MdispWebForm\\C#\\precompiled\\MdispWebForm.exe");
      MappFileOperation(M_DEFAULT, ExamplePath, M_NULL, M_NULL, M_FILE_EXISTS, M_NULL, &FileExists);
      }
   else if(AppType == WEB_VB_FORM)
      {
      ExamplePath+= MIL_TEXT("Core\\MdispWebForm\\VB\\precompiled\\MdispWebForm.exe");
      MappFileOperation(M_DEFAULT, ExamplePath, M_NULL, M_NULL, M_FILE_EXISTS, M_NULL, &FileExists);
      }
    else if(AppType == WEB_BROWSER)
      {
      return true;
      }
#endif
   return (FileExists == M_YES);
   }


