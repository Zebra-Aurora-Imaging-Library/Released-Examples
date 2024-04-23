﻿/*********************************************************************************/
/*
 * File name: MgraWeb.cpp
 *
 * Synopsis:  This program shows how to track a unique object using
 *            pattern recognition. It allocates a model in the field of
 *            view of the camera and finds it in a loop.
 *
 *            The interaction is done from the web page.
 *            Note:  Display update and annotations drawing can require
 *                   significant CPU usage.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#define HTTP_SERVER_PORT 9002L
#if M_MIL_USE_WINDOWS
#include <windows.h>
static HANDLE MdispWebClientHandle = NULL;

#else
static pid_t MdispWebClientHandle = 0;
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif

enum AppEnum {WEB_CLIENT=0, WEB_BROWSER, WEB_CSHARP_FORM, WEB_VB_FORM};

void LaunchApplication(AppEnum AppType);
void StartHttpServer(MIL_ID& HttpServerId);

/* Model specification. */
#define MODEL_WIDTH                   128L
#define MODEL_HEIGHT                  128L
#define MODEL_POS_X_INIT(TargetImage) (MbufInquire(TargetImage, M_SIZE_X, M_NULL)/2)
#define MODEL_POS_Y_INIT(TargetImage) (MbufInquire(TargetImage, M_SIZE_Y, M_NULL)/2)
#define RECTANGLE_ANGLE      0

/* Minimum score to consider the object found (in percent). */
#define MODEL_MIN_MATCH_SCORE       50.0

/* Drawing color */
#define DRAW_COLOR                  0xFF /* White */

#define SELECTION_RADIUS     10

#define TEXT_SIZE            2048
struct SParameter
   {
      MIL_INT PosX;
      MIL_INT PosY;
      MIL_INT Width;
      MIL_INT Height;
   };



#define CONTINUE_MSG_TAG 99991
#define STOP_MSG_TAG     99992
#define QUIT_MSG_TAG     99993

static bool ContinueReceived = false;
static bool StopReceived     = false;
static bool QuitReceived     = false;

bool MmodTrackingExample(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer,
                         MIL_ID MilDisplayImage, MIL_ID MilModelImage, MIL_ID MilMessageOutput, struct SParameter *Data);

void GetModelImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer,
                   MIL_ID MilDisplayImage, MIL_ID MilModelImage, MIL_ID MilMessageOutput, struct SParameter *Data);

MIL_INT MFTYPE MessageReceiveHandler(MIL_INT HookType, MIL_ID EventId, void* HookDataPtr)
   {
   if (HookType == M_MESSAGE_RECEIVED)
      {
      MIL_INT64 MsgStatus = 0, MsgTag = 0;
      MIL_INT MsgLength = 0;
      MIL_ID MsgId = 0;
      MobjGetHookInfo(EventId, M_OBJECT_ID, &MsgId);
      MobjInquire(MsgId, M_MESSAGE_LENGTH, &MsgLength);
      MIL_UINT8 *Data = new MIL_UINT8[MsgLength];
      memset(Data, 0, MsgLength);
      MobjMessageRead(MsgId, Data, MsgLength, M_NULL, &MsgTag, &MsgStatus, M_DEFAULT);
      if(MsgTag == CONTINUE_MSG_TAG)
         ContinueReceived = true;
      else if(MsgTag == STOP_MSG_TAG)
         StopReceived = true;
      else if(MsgTag == QUIT_MSG_TAG)
         QuitReceived = true;
      delete [] Data;
      }
   return 0;
   }

void WaitForContinue()
   {
   bool bContinue = true;
   while (bContinue)
      {
      if(ContinueReceived || QuitReceived)
         {
         bContinue = false;
         ContinueReceived = false;
         }
      MosSleep(1);
      }
   }

/*****************************************************************************
Main.
*****************************************************************************/
int MosMain(void)
   {
   MIL_ID MilApplication,   /* Application identifier.   */
      MilSystem,        /* System identifier.        */
      MilDisplay,       /* Display identifier.       */
      MilDigitizer,     /* Digitizer identifier.     */
      MilDisplayImage,  /* Display image identifier. */
      MilMessageOutput, /* Message identifier. */
      MilMessageInput,  /* Message identifier. */
      MilModelImage;    /* Model image identifier.   */
   
   MIL_ID MilHttpServer  = M_NULL;   
   SParameter DataParam;
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
   MappControl(M_DEFAULT, M_WEB_CONNECTION_PORT, 7682);
   MappControl(M_DEFAULT, M_WEB_CONNECTION, M_ENABLE);
	  	
   MIL_INT ImageSizeX = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);
   MIL_INT SizeBand   = MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL);
   
   MdispAlloc(MilSystem, M_DEFAULT,MIL_TEXT("M_DEFAULT"),M_WEB, &MilDisplay);
   /* Publish the display */
   MobjControl(MilDisplay, M_OBJECT_NAME, MIL_TEXT("Display"));
   MobjControl(MilDisplay, M_WEB_PUBLISH, M_READ_ONLY);

   /* Output message */
   MobjAlloc(MilSystem, M_MESSAGE_MAILBOX, M_OVERWRITE, &MilMessageOutput);
   MobjControl( MilMessageOutput, M_OBJECT_NAME, MIL_TEXT("MessageOutput")); // Message to send to clients
   MobjControl( MilMessageOutput, M_WEB_PUBLISH, M_READ_ONLY);
   
   /* Input message */
   MobjAlloc(MilSystem, M_MESSAGE_MAILBOX, M_QUEUE, &MilMessageInput);
   MobjControl( MilMessageInput, M_OBJECT_NAME, MIL_TEXT("MessageInput"));
   MobjControl( MilMessageInput, M_WEB_PUBLISH, M_READ_WRITE);
   MobjHookFunction( MilMessageInput, M_MESSAGE_RECEIVED, MessageReceiveHandler, M_NULL);

   /* Allocate a display image and show it */
   MbufAllocColor(MilSystem, SizeBand, ImageSizeX, ImageSizeY, 8, M_IMAGE+M_DISP+M_PROC+M_GRAB, &MilDisplayImage);
   
   MdispSelect(MilDisplay, MilDisplayImage);
   StartHttpServer(MilHttpServer);
   LaunchApplication(WEB_BROWSER);

   do
      {
         
      /* Allocate a model image buffer. */
      MbufAlloc2d(MilSystem,
                  MbufInquire(MilDisplayImage, M_SIZE_X, M_NULL),
                  MbufInquire(MilDisplayImage, M_SIZE_Y, M_NULL),
                  8, M_IMAGE+M_PROC, &MilModelImage);
         
      MosPrintf(MIL_TEXT("\nMODEL TRACKING:\n"));
      MosPrintf(MIL_TEXT("---------------\n\n"));
      MosPrintf(MIL_TEXT("\n\n"));
      MosPrintf(MIL_TEXT("This example demonstrates how to publish various MIL objects\nusing the MIL web API.\n"));
      MosPrintf(MIL_TEXT("It also shows how to interact with MIL display from a web browser.\n"));
      MosPrintf(MIL_TEXT("The example execution is controlled from the web client.\n"));
         
      bool Done = false;
      do
         {
         /* Get the model image. */
         GetModelImage(MilSystem, MilDisplay, MilDigitizer, MilDisplayImage, MilModelImage, MilMessageOutput, &DataParam);
            
         /* Finds the model using geometric model finder. */
         Done = MmodTrackingExample(MilSystem, MilDisplay, MilDigitizer,MilDisplayImage, MilModelImage, MilMessageOutput, &DataParam);
         } while (!Done);


      /* Free allocated buffers. */
      MbufFree(MilModelImage);
         
      MosPrintf(MIL_TEXT("Press a <Enter> to end the example.\n\n"));
      } while(!QuitReceived && !MosKbhit());


   MobjFree(MilHttpServer);
   MbufFree(MilDisplayImage);
   MdispFree(MilDisplay);
   
   MobjFree(MilMessageOutput);
   MobjFree(MilMessageInput);

   /* Free defaults. */
   MappFreeDefault(MilApplication, MilSystem, M_NULL, MilDigitizer, M_NULL);
      
   
   return 0;
   }


/*****************************************************************************
Get Model Image Function.
*****************************************************************************/

void GetModelImage(MIL_ID MilSystem, MIL_ID MilDisplay, MIL_ID MilDigitizer,
                   MIL_ID MilDisplayImage, MIL_ID MilModelImage, MIL_ID MilMessageOutput, struct SParameter *Data)
   {
   MIL_ID     MilGraphicsList;           /* Graphics list identifier.       */
   MIL_ID     MilGraphicsContext;        /* Graphics context identifier.    */

   MIL_DOUBLE DrawColor = DRAW_COLOR;  /* Drawing color.      */

   /* Allocate a graphics list to hold the subpixel annotations. */
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicsList);

   /* Increase the selection radius for easier interactivity. */
   MgraControlList(MilGraphicsList, M_LIST, M_DEFAULT, M_SELECTION_RADIUS,
                   SELECTION_RADIUS);

   /* Associate the graphics list to the display for annotations. */
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicsList);

   /* Allocate a graphics context for the draw operations. */
   MgraAlloc(MilSystem, &MilGraphicsContext);


   /* Enable the interactive mode. */
   MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);


   Data->PosX   = MODEL_POS_X_INIT(MilDisplayImage) - (MODEL_WIDTH/2);
   Data->PosY   = MODEL_POS_Y_INIT(MilDisplayImage) - (MODEL_HEIGHT/2);
   Data->Width  = MODEL_WIDTH;
   Data->Height = MODEL_HEIGHT;
   
   /* Add a selectable rectangular region.*/
   MgraRectAngle(MilGraphicsContext, MilGraphicsList, Data->PosX,
                 Data->PosY,
                 Data->Width,
                 Data->Height,
                 RECTANGLE_ANGLE,
                 M_DEFAULT);
   
   MgraControlList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_GRAPHIC_SELECTED, M_TRUE);
   MgraControlList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_ROTATABLE, M_DISABLE);
   MgraControlList(MilGraphicsList, M_LIST, M_DEFAULT,  M_SELECTION_RADIUS, 30.0);
   /* Grab continuously. */
   char TextAscii[TEXT_SIZE];
   memset(TextAscii, 0, TEXT_SIZE);
   sprintf(TextAscii, "Model definition:\n\n"
            "Place a unique model to find in the marked rectangle.\n"
            "Then push the \"Continue\" button.\n"
            );
   MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, CONTINUE_MSG_TAG, M_DEFAULT);
   
   /* Grab a reference model image. */
   MdigGrabContinuous(MilDigitizer, MilDisplayImage);
   WaitForContinue();
   MdigHalt(MilDigitizer);

   /* Copy the grabbed image to the Model image to keep it. */
   MbufCopy(MilDisplayImage, MilModelImage);

   Data->PosX   = MgraInquireList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_POSITION_X, M_NULL);
   Data->PosY   = MgraInquireList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_POSITION_Y, M_NULL);
   Data->Width  = MgraInquireList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_RECTANGLE_WIDTH, M_NULL);
   Data->Height = MgraInquireList(MilGraphicsList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_RECTANGLE_HEIGHT, M_NULL);
   
   MdispControl(MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_DISABLE);
   MgraFree(MilGraphicsContext);
   MgraFree(MilGraphicsList);

   }


/*****************************************************************************
Tracking object with Geometric Model Finder module
*****************************************************************************/

#define MODEL_MAX_OCCURRENCES       16L

bool MmodTrackingExample(MIL_ID MilSystem,    MIL_ID MilDisplay,
                         MIL_ID MilDigitizer, MIL_ID MilDisplayImage, MIL_ID MilModelImage, MIL_ID MilMessageOutput, struct SParameter *Data)
   {
   MIL_ID     MilImage[2]={M_NULL,M_NULL},  /* Processing image buffer identifiers. */
      SearchContext,                /* Search context identifier.           */
         Result;                       /* Result identifier.                   */
      MIL_DOUBLE DrawColor = DRAW_COLOR;       /* Model drawing color.                 */
      MIL_INT    Found;                        /* Number of models found.              */
      MIL_INT    NbFindDone = 0;               /* Number of loops to find model done.  */
      MIL_DOUBLE OrgX = 0.0, OrgY = 0.0;       /* Original center of model.            */
      MIL_DOUBLE Score[MODEL_MAX_OCCURRENCES], /* Model correlation score.             */
         x[MODEL_MAX_OCCURRENCES],     /* Model X position.                    */
         y[MODEL_MAX_OCCURRENCES],     /* Model Y position.                    */
         Angle[MODEL_MAX_OCCURRENCES], /* Model occurrence angle.              */
         Scale[MODEL_MAX_OCCURRENCES]; /* Model occurrence scale.              */
      MIL_DOUBLE Time = 0.0;                   /* Timer.                               */
      bool Done = true;
   
      /* Display model image. */
      MbufCopy(MilModelImage, MilDisplayImage);
   
   
      /* Allocate a context and define a geometric model. */
      MmodAlloc(MilSystem, M_GEOMETRIC, M_DEFAULT, &SearchContext);
      MmodDefine(SearchContext, M_IMAGE, MilModelImage,
                 (MIL_DOUBLE)Data->PosX,
                 (MIL_DOUBLE)Data->PosY,
                 (MIL_DOUBLE)Data->Width,
                 (MIL_DOUBLE)Data->Height);

      /* Allocate result. */
      MmodAllocResult(MilSystem, M_DEFAULT, &Result);

      /* Draw a box around the model. */
      MgraColor(M_DEFAULT, DrawColor);
      MmodDraw(M_DEFAULT, SearchContext, MilDisplayImage, M_DRAW_BOX, M_DEFAULT, M_ORIGINAL);

      /* Set speed to VERY HIGH for fast but less precise search. */
      MmodControl(SearchContext, M_CONTEXT, M_SPEED, M_VERY_HIGH);

      /* Set minimum acceptance for the search. */
      MmodControl(SearchContext, M_DEFAULT, M_ACCEPTANCE, MODEL_MIN_MATCH_SCORE);
   
      MappControl(M_ERROR, M_PRINT_DISABLE);
      /* Preprocess model. */
      MmodPreprocess(SearchContext, M_DEFAULT);
      if(MappGetError(M_GLOBAL, 0) != M_NULL_ERROR)
         {
         goto end;
         }
      MappControl(M_ERROR, M_PRINT_ENABLE);
   
      /* Inquire about center of model. */
      MmodInquire(SearchContext, M_DEFAULT, M_ORIGINAL_X, &OrgX);
      MmodInquire(SearchContext, M_DEFAULT, M_ORIGINAL_Y, &OrgY);

      char TextAscii[TEXT_SIZE];
      memset(TextAscii, 0, TEXT_SIZE);
      sprintf(TextAscii,"The Geometric target model was defined.\n"
               "Model dimensions: %ld x %ld.\n"
               "Model center:     X=%.2f, Y=%.2f.\n"
               "Model is scale and rotation independent.\n"
               "Push \"Continue\" button to continue execution\n",
               MODEL_WIDTH, MODEL_HEIGHT, OrgX, OrgY);
      MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, 2, M_DEFAULT);

      WaitForContinue();

      /* Allocate 2 grab buffers. */
      MbufAlloc2d(MilSystem,
                  MbufInquire(MilModelImage, M_SIZE_X, M_NULL),
                  MbufInquire(MilModelImage, M_SIZE_Y, M_NULL),
                  8,
                  M_IMAGE+M_GRAB+M_PROC,
                  &MilImage[0]);
      MbufAlloc2d(MilSystem,
                  MbufInquire(MilModelImage, M_SIZE_X, M_NULL),
                  MbufInquire(MilModelImage, M_SIZE_Y, M_NULL),
                  8,
                  M_IMAGE+M_GRAB+M_PROC,
                  &MilImage[1]);

      /* Grab continuously grab and perform the find operation using double buffering. */
      memset(TextAscii, 0, TEXT_SIZE);
      sprintf(TextAscii,"\nContinuously finding the Geometric Model.\n"
               "Push \"Restart\" to stop finding and restart the example.\n" 
               "Push \"Quit\" to end the example.\n");
      MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, 2, M_DEFAULT);

      /* Grab a first target image into first buffer (done twice for timer reset accuracy). */
      MdigControl(MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);
      MdigGrab(MilDigitizer, MilImage[0]);
      MdigGrab(MilDigitizer, MilImage[1]);
      MappTimer(M_DEFAULT, M_TIMER_RESET, &Time);

      /* Loop, processing one buffer while grabbing the other. */
      do
         {
         /* Grab a target image into the other buffer. */
         MdigGrab(MilDigitizer, MilImage[NbFindDone%2]);

         /* Read the time. */
         MappTimer(M_DEFAULT, M_TIMER_READ, &Time);

         /* Find model. */
         MmodFind(SearchContext, MilImage[(NbFindDone+1)%2], Result);

         /* Get the number of occurrences found. */
         MmodGetResult(Result, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT, &Found);

         /* Print a message based on the score. */
         if ( (Found >= 1) && (Found < MODEL_MAX_OCCURRENCES) )
            {
            /* Get results. */
            MmodGetResult(Result, M_DEFAULT, M_POSITION_X, x);
            MmodGetResult(Result, M_DEFAULT, M_POSITION_Y, y);
            MmodGetResult(Result, M_DEFAULT, M_SCALE, Scale);
            MmodGetResult(Result, M_DEFAULT, M_ANGLE, Angle);
            MmodGetResult(Result, M_DEFAULT, M_SCORE, Score);

            /* Draw a box and a cross where the model was found and print the results. */
            MmodDraw(M_DEFAULT, Result, MilImage[(NbFindDone+1)%2],
                     M_DRAW_BOX+M_DRAW_POSITION+M_DRAW_EDGES, M_DEFAULT, M_DEFAULT);
            memset(TextAscii, 0, TEXT_SIZE);
            sprintf(TextAscii,"Found: X=%6.1f, Y=%6.1f, Angle=%6.1f, Scale=%5.2f,Score=%5.1f%% (%5.1f fps).\n",
                     x[0], y[0], Angle[0], Scale[0], Score[0], (NbFindDone+1)/Time);
            MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, STOP_MSG_TAG, M_DEFAULT);

            }
         else
            {
            /* Print the "not found" message. */
            memset(TextAscii, 0, TEXT_SIZE);
            sprintf(TextAscii, "Not found! (score<%5.1f%%)                          "
                     "(%5.1f fps).\n", MODEL_MIN_MATCH_SCORE, (NbFindDone+1)/Time);
            MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, STOP_MSG_TAG, M_DEFAULT);
            }

         /* Copy target image to the display. */
         MbufCopy(MilImage[NbFindDone%2], MilDisplayImage);

         /* Increment the counter. */
         NbFindDone++;
         } 
      while (!StopReceived && !QuitReceived);

      memset(TextAscii, 0, TEXT_SIZE);
      MobjMessageWrite(MilMessageOutput, (MIL_UINT8 *)TextAscii, TEXT_SIZE, STOP_MSG_TAG, M_DEFAULT);

      StopReceived = false;
   
      MosPrintf(MIL_TEXT("\n\n"));
   
   end:
      /* Wait for the end of last grab. */
      MdigGrabWait(MilDigitizer, M_GRAB_END);

      /* Free all allocations. */
      MmodFree(Result);
      MmodFree(SearchContext);
      if(MilImage[1])
         MbufFree(MilImage[1]);
      if(MilImage[0])
         MbufFree(MilImage[0]);
   
      return Done;
   }
                
/* Launch a specifc application. */
/* ----------------------------- */
void LaunchApplication(AppEnum AppType)
   {
   MIL_TEXT_CHAR Buffer[TEXT_SIZE];
   memset(Buffer,0,TEXT_SIZE);
#if M_MIL_USE_LINUX
   if(AppType == WEB_BROWSER)
      {
      if (access("/usr/bin/gio", X_OK) == 0)
         MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("gio open http://localhost:%ld/mgraweb.html >/dev/null 2>/dev/null"), HTTP_SERVER_PORT);
      else
         MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("xdg-open http://localhost:%ld/mgraweb.html >/dev/null 2>/dev/null"), HTTP_SERVER_PORT);
      
      system(Buffer);
      }
   else
      {
      MosPrintf(MIL_TEXT("Invalid application type !!!\n"));
      }
#else
   if(AppType == WEB_BROWSER)
      {     
      MosSprintf(Buffer, TEXT_SIZE, MIL_TEXT("http://localhost:%ld/mgraweb.html"), HTTP_SERVER_PORT);
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
   MobjControl(HttpServerId, M_HTTP_PORT, HTTP_SERVER_PORT);

   /* set http root document */
   ExamplePath+=MIL_TEXT("Core/MdispWebClient/C++/html");
   MobjControl(HttpServerId, M_HTTP_ROOT_DIRECTORY, ExamplePath);
   MobjControl(HttpServerId, M_HTTP_START, M_DEFAULT);  
   }
