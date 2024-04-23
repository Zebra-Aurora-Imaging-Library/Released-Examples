//***************************************************************************************/
// 
// File name: ScriptPreprocessing.cpp  
//
// Synopsis:  This example shows how to execute MIL code within a script language.
//            A sequence of preprocessing operations followed by a blob analysis
//            operation is applied to the grabbed image. The preprocessing step
//            is executed within a script (C#, and Python based on availability).
//            
//
// Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

/* Utility macro. */
#define EXAMPLE_SCRIPT_PATH                       M_IMAGE_PATH MIL_TEXT("ScriptPreprocessing/")
#define EXAMPLE_INTERPRETER_PATH                  M_INSTALL_DIR MIL_TEXT("tools/") 

/* Lists of all the script languages to execute in this example. */
static const MIL_INT NB_SCRIPT_LANGUAGE = 7;
static MIL_CONST_TEXT_PTR SCRIPT_INTERPRETERS[NB_SCRIPT_LANGUAGE] =
   {
   M_INTERPRETER_C_PYTHON3X,
   M_INTERPRETER_C_PYTHON310,
   M_INTERPRETER_C_PYTHON39,
   M_INTERPRETER_C_PYTHON38,
   M_INTERPRETER_C_PYTHON37,
   M_INTERPRETER_C_PYTHON36,
   M_INTERPRETER_CSHARP
   };
static MIL_CONST_TEXT_PTR SCRIPT_INTERPRETER_NAMES[NB_SCRIPT_LANGUAGE] =
   {
   MIL_TEXT("Python Any 3.X"),
   MIL_TEXT("Python 3.10"),
   MIL_TEXT("Python 3.9"),
   MIL_TEXT("Python 3.8"),
   MIL_TEXT("Python 3.7"),
   MIL_TEXT("Python 3.6"),
   MIL_TEXT("C#")
   };
static MIL_CONST_TEXT_PTR SCRIPT_PATHS[NB_SCRIPT_LANGUAGE] =
   {
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.py"),
   EXAMPLE_SCRIPT_PATH MIL_TEXT("ScriptPreprocessing.cs")
   };
static MIL_INT SCRIPT_OPCODES[NB_SCRIPT_LANGUAGE] =
   {
   M_SCRIPT_MODULE_1 + 1L,
   M_SCRIPT_MODULE_1 + 2L,
   M_SCRIPT_MODULE_1 + 3L,
   M_SCRIPT_MODULE_1 + 4L,
   M_SCRIPT_MODULE_1 + 5L,
   M_SCRIPT_MODULE_1 + 6L,
   M_SCRIPT_MODULE_1 + 7L,
   };

#define PREPROCESSING_FUNCTION_NAME          MIL_TEXT("Preprocessing")                                             
#define SCRIPT_FUNCTION_NAME                 MIL_TEXT("PreprocessingFunction")

#define BUFFERING_SIZE_MAX 22

MIL_INT MFTYPE HookFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
typedef struct
   {
   MIL_ID MilDisplay;
   MIL_ID  MilImageDisp;
   MIL_INT ProcessedImageCount;
   MIL_ID PreprocessedImage;
   MIL_ID GraphicList;
   MIL_ID MilBlobContext;
   MIL_ID MilBlobResult;
   MIL_CONST_TEXT_PTR InterpreterPath;
   MIL_CONST_TEXT_PTR ScriptPath;
   MIL_INT ScriptOpcode;
   } HookDataStruct;

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
      MIL_TEXT("ScriptPreprocessing\n\n")

      MIL_TEXT("[SYNOPSIS]\n")
      MIL_TEXT("This example shows how to execute MIL code within a script language.\n")
      MIL_TEXT("A sequence of preprocessing operations followed by a blob analysis\n")
      MIL_TEXT("operation is applied to the grabbed image. The preprocessing step\n")
      MIL_TEXT("is executed within a script (C# and Python based on availability).\n\n")

      MIL_TEXT("[MODULES USED]\n")
      MIL_TEXT("Modules used: application, system, display, buffer,\n")
      MIL_TEXT("digitizer, image processing, blob analysis\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   
   }

//*****************************************************************************
// Preprocessing function, calling the specified script. 
//*****************************************************************************
MIL_INT64 Preprocessing(MIL_ID Source, MIL_ID Destination, MIL_CONST_TEXT_PTR InterpreterPath, MIL_CONST_TEXT_PTR ScriptPath, MIL_INT Opcode)
   {
   MIL_ID FuncId;
   MIL_INT64 ReturnValue = -1;

   /* Initialize the Preprocessing Scripting function. */
   MfuncAllocScript(MIL_TEXT("Preprocessing"),
      3,
      InterpreterPath,
      ScriptPath,
      SCRIPT_FUNCTION_NAME,
      Opcode,
      M_LOCAL+M_SYNCHRONOUS_FUNCTION,
      &FuncId
      );
   
   MfuncParamMilId(FuncId, 1, Source, M_IMAGE, M_IN);
   MfuncParamMilId(FuncId, 2, Destination, M_IMAGE, M_OUT);
   MfuncParamArrayMilInt64(FuncId, 3, &ReturnValue, 1, M_OUT);

   MfuncCall(FuncId);
   
   MfuncFree(FuncId);
   
   return ReturnValue;
   }
  
//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain()
   {
   MIL_ID MilApplication;
   MIL_ID MilSystem;
   MIL_ID MilDisplay;
   MIL_ID MilDigitizer;
   MIL_ID MilImageDisp;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_INT MilGrabBufferListSize;
   MIL_INT SizeX = 0, SizeY = 0, n = 0;
   MIL_INT64 Success = -1;
   MIL_INT ProcessFrameCount = 0;
   MIL_DOUBLE ProcessFrameRate = 0;
   HookDataStruct UserHookData;
   bool InterpreterStatuses[NB_SCRIPT_LANGUAGE];
   
   /* Allocate defaults. */
   MappAlloc(MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
 
   SizeX = MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   SizeY = MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);
   MbufAllocColor(MilSystem,
      3,
      SizeX,
      SizeY,
      8 + M_UNSIGNED,
      M_IMAGE + M_GRAB + M_DISP + M_PROC,
      &MilImageDisp);

   /* Allocate the graphic list. */
   UserHookData.GraphicList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, UserHookData.GraphicList);
   MdispControl(MilDisplay, M_UPDATE, M_DISABLE);
   
   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE_MAX; MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem,
         SizeX,
         SizeY,
         8 + M_UNSIGNED,
         M_IMAGE + M_GRAB + M_PROC,
         &MilGrabBufferList[MilGrabBufferListSize]);
      if (MilGrabBufferList[MilGrabBufferListSize])
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   /* Free buffers to leave space for possible temporary buffers. */
   for (n = 0; n < 2 && MilGrabBufferListSize; n++)
      {
      MilGrabBufferListSize--;
      MbufFree(MilGrabBufferList[MilGrabBufferListSize]);
      }

   /* Allocate blob objects. */
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &UserHookData.MilBlobContext);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &UserHookData.MilBlobResult);
   MblobControl(UserHookData.MilBlobContext, M_BOX, M_ENABLE);
   MblobControl(UserHookData.MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);

   /* Initialize the user's processing function data structure. */
   UserHookData.MilDisplay = MilDisplay;
   UserHookData.ProcessedImageCount = 0;
   UserHookData.MilImageDisp = MilImageDisp;

   /* Keep the last buffer for the preprocessing. */
   UserHookData.PreprocessedImage = MilGrabBufferList[--MilGrabBufferListSize];

   /* Print header. */
   PrintHeader();

   /* Message for script initialization, which can take some time. */
   MosPrintf(MIL_TEXT("Loading resources...\n\n"));

   MdigGrab(MilDigitizer, MilGrabBufferList[0]);
  
   /* For each Script language, try to run the Preprocessing function once. */
   /* If no errors are reported, the script is working correctly. */
   for (n = 0; n < NB_SCRIPT_LANGUAGE; n++)
      {
      UserHookData.InterpreterPath = SCRIPT_INTERPRETERS[n];
      UserHookData.ScriptPath = SCRIPT_PATHS[n];
      UserHookData.ScriptOpcode = SCRIPT_OPCODES[n];

      /* If errors are reported during the preprocessing, the interpreter language is */
      /* probably not available or has not been installed. */
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE); 
      Success = Preprocessing(MilGrabBufferList[0],
                              UserHookData.PreprocessedImage,
                              UserHookData.InterpreterPath,
                              UserHookData.ScriptPath,
                              UserHookData.ScriptOpcode);
      MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

      InterpreterStatuses[n] = (Success == M_NULL);

      if(!InterpreterStatuses[n])
         {
         MIL_STRING ScriptPath = MIL_STRING(EXAMPLE_INTERPRETER_PATH) + MIL_STRING(SCRIPT_INTERPRETERS[n]);
         MIL_INT FilePresent = M_NO;

         MappFileOperation(M_DEFAULT, ScriptPath, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);

         if(FilePresent == M_YES)
            {
            MosPrintf(MIL_TEXT("\nThis example cannot run with %s since the installation was not found. \n"), SCRIPT_INTERPRETER_NAMES[n]);
            }
         }

      }

   /* For each successfully initialized script, start the MdigProcess loop. */
   for (n = 0; n < NB_SCRIPT_LANGUAGE; n++)
      {
      if (InterpreterStatuses[n])
         {
         UserHookData.InterpreterPath = SCRIPT_INTERPRETERS[n];
         UserHookData.ScriptPath = SCRIPT_PATHS[n];
         UserHookData.ScriptOpcode = SCRIPT_OPCODES[n];

         MosPrintf(MIL_TEXT("\nExecution using %s:\n"), SCRIPT_INTERPRETER_NAMES[n]);
         MosPrintf(MIL_TEXT("---------------------------\n"));
         MosPrintf(MIL_TEXT("The sequence of preprocessing operations is interpreted runtime within a \n%s script.\n\n"),
            SCRIPT_INTERPRETER_NAMES[n]);
         MosPrintf(MIL_TEXT("Press <Enter> to start. \n\n"));
         MosGetch();

         MdispSelect(MilDisplay, MilImageDisp);

         /* Start the processing. The processing function is called with every frame grabbed. */
         /* At the start of the processing function, the script function is called to preprocess the grabbed frame. */
         MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, HookFunction, &UserHookData);

         MosPrintf(MIL_TEXT("Press <Enter> to stop. \n\n"));
         MosGetch();

         MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_STOP, M_DEFAULT, HookFunction, &UserHookData);

         /* Print statistics. */
         MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
         MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
         MosPrintf(MIL_TEXT("%d frames grabbed at %.1f frames/sec (%.1f ms/frame).\n\n"),
            (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
         }
      }

   if (UserHookData.ProcessedImageCount == 0)
      {
      MosPrintf(MIL_TEXT("\nNo script interpreter could be initialized. To use the Python interpreter, you\n")
                MIL_TEXT("must have a valid Python installation of the same version than the interpreter.\n")
                MIL_TEXT("To use the C# you must install MIL for .NET.\n")
                MIL_TEXT("Please refer to the MIL User Manual and www.python.org for more information.\n\n"));
      }
   
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
   MosGetch();

   while (MilGrabBufferListSize >= 0)
      MbufFree(MilGrabBufferList[MilGrabBufferListSize--]);
      
   MblobFree(UserHookData.MilBlobContext);
   MblobFree(UserHookData.MilBlobResult);
   MgraFree(UserHookData.GraphicList);
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
   return 0;
   }

MIL_INT MFTYPE HookFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   MIL_INT64 PreprocReturnValue = -1;
   MIL_ID CurrentImage = 0;
   HookDataStruct *UserHookDataPtr = (HookDataStruct *)HookDataPtr;
   
   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &CurrentImage);
   
   UserHookDataPtr->ProcessedImageCount++;

   /* Call the preprocessing script function.  */
   if (Preprocessing(CurrentImage, 
                     UserHookDataPtr->PreprocessedImage,
                     UserHookDataPtr->InterpreterPath, 
                     UserHookDataPtr->ScriptPath,
                     UserHookDataPtr->ScriptOpcode) == M_NULL)
      {
      /* Execute the processing and update the display. */
      MblobCalculate(UserHookDataPtr->MilBlobContext, UserHookDataPtr->PreprocessedImage, M_NULL, UserHookDataPtr->MilBlobResult);

      /* Show the results in the display. */
      MbufCopy(CurrentImage, UserHookDataPtr->MilImageDisp);
      MgraClear(M_DEFAULT, UserHookDataPtr->GraphicList);
      MIL_INT NbBlobs;
      MblobGetResult(UserHookDataPtr->MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NbBlobs);
      if (NbBlobs > 0)
         {
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         MblobDraw(M_DEFAULT, UserHookDataPtr->MilBlobResult, UserHookDataPtr->GraphicList, M_DRAW_BLOBS, M_DEFAULT, M_DEFAULT);
         MgraColor(M_DEFAULT, M_COLOR_BLUE);
         MblobDraw(M_DEFAULT, UserHookDataPtr->MilBlobResult, UserHookDataPtr->GraphicList, M_DRAW_CENTER_OF_GRAVITY, M_DEFAULT, M_DEFAULT);
         }
      MdispControl(UserHookDataPtr->MilDisplay, M_UPDATE, M_NOW);
      }
   else
      {
      MosPrintf(MIL_TEXT("An error was returned by the preprocessing script function.\n\n"));
      }

   return 0;
   }
