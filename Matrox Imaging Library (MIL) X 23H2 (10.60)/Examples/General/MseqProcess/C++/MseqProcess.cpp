﻿/*************************************************************************************/
/*
 * File name: MseqProcess.cpp
 *
 * Synopsis:  This program shows the use of the MseqProcess() and MseqFeed() functions
 *            to perform real-time encoding of a sequence of captured images.
 *
 *            The user's preprocessing and compression code is written in a hook
 *            function that will be called by MdigProcess() for each frame grabbed
 *            (see ProcessingFunction()). The queueing for encoding of the next
 *            frame is also made in that hook function to allow fully parallel
 *            execution of the capture and the encoding.
 *
 *      Note: The average encoding time must be shorter than the grab time or
 *            some frames will be missed. Missed frames are very frequent when
 *            the encoding is done by software. Also, if the captured images
 *            are not displayed and the frame count is not drawn or printed
 *            in the hook function, the CPU usage is reduced significantly.
 *
 *            When encoding a 1080p source it is recommended to have your
 *            MIL Non-Paged Memory set to at least 64MB.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include <mil.h>

/* Number of images in the buffering grab queue.
   Generally, increasing this number gives better real-time grab.
   */
#define BUFFERING_SIZE_MAX 20

/* Target sequence file name and location. The temporary directory location can
   be reached with %temp% under Windows.
   */
#define SEQUENCE_FILE M_TEMP_DIR MIL_TEXT("SeqProcess.mp4")

/* Remote target sequence file name and location if Distributed MIL is used. */
#define REMOTE_SEQUENCE_FILE MIL_TEXT("remote:///") SEQUENCE_FILE

enum ProcessingHookOperation
   {
   DISPLAY,
   ENCODE,
   };

/* User's processing function prototype. */
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

/* User's processing function hook data structure. */
typedef struct
   {
   MIL_ID  MilDigitizer;
   MIL_ID  MilImageDisp;
   MIL_ID  MilSeqContext;
   MIL_INT ProcessedImageCount;
   ProcessingHookOperation ProcessingOperation;
   } ProcessingHookDataStruct;

/* Optional encoding end function prototype */
MIL_INT MFTYPE FrameEncodingEndFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

/* Optional decoding end function prototype */
MIL_INT MFTYPE FrameDecodingEndFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);

/* Function to display the details about errors from MseqProcess(), if any */
MIL_INT CheckMseqProcessError(MIL_ID MilApplication, MIL_ID MilCompressContext);
MIL_INT PrintMilErrorMessage(MIL_ID MilApplication);

/* Optional encoding end function hook data structure. */
typedef struct
   {
   MIL_INT EncodedImageCount;
   MIL_ID DecompressContextID;
   } EncodingFrameEndHookDataStruct;

/* Optional decoding end function hook data structure. */
typedef struct
   {
   MIL_INT DecodedImageCount;
   MIL_ID MilImageDisp;
   } DecodingFrameEndHookDataStruct;

/* Main function. */
/* ---------------*/
int MosMain(void)
   {
   MIL_ID MilApplication;
   MIL_ID MilRemoteApplication;
   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_ID MilDisplay;
   MIL_ID MilImageDisp;
   MIL_ID MilGrabBufferList[BUFFERING_SIZE_MAX] = { 0 };
   MIL_ID MilCompressContext;
   MIL_ID MilDecompressContext;
   MIL_INT LicenseModules = 0;
   MIL_INT MilSystemLocation;
   MIL_INT MilGrabBufferListSize;
   MIL_INT ProcessFrameCount = 0;
   MIL_INT NbFrames = 0, n = 0;
   MIL_DOUBLE EncodingDesiredFrameRate = 0.0;
   MIL_DOUBLE ProcessFrameRate = 0.0;
   MIL_STRING SeqProcessFilePath;
   ProcessingHookDataStruct ProcessingUserHookData;
   EncodingFrameEndHookDataStruct EncodingFrameEndUserHookData;
   DecodingFrameEndHookDataStruct DecodingFrameEndUserHookData;
   MIL_INT SeqSystemType = M_NULL;

   /* Allocate defaults. */
   MappAllocDefault(M_DEFAULT, &MilApplication, &MilSystem, &MilDisplay,
                    &MilDigitizer, &MilImageDisp);

   MsysInquire(MilSystem, M_OWNER_APPLICATION, &MilRemoteApplication);
   MilSystemLocation = MsysInquire(MilSystem, M_LOCATION, M_NULL);

   /* Inquire MIL licenses. */
   MappInquire(MilRemoteApplication, M_LICENSE_MODULES, &LicenseModules);
   if (!(LicenseModules & (M_LICENSE_JPEGSTD)))
      {
      MosPrintf(MIL_TEXT("Need a Compression/Decompression license to run this example.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();

      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 0;
      }

   /* Allocate the grab buffers and clear them. */
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   for (MilGrabBufferListSize = 0;
        MilGrabBufferListSize < BUFFERING_SIZE_MAX;
        MilGrabBufferListSize++
        )
      {
      MbufAllocColor(MilSystem, MdigInquire(MilDigitizer, M_SIZE_BAND, M_NULL),
                     MdigInquire(MilDigitizer, M_SIZE_X, M_NULL),
                     MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL),
                     8 + M_UNSIGNED, M_IMAGE + M_GRAB,
                     &MilGrabBufferList[MilGrabBufferListSize]
                     );

      if (MilGrabBufferList[MilGrabBufferListSize])
         {
         MbufClear(MilGrabBufferList[MilGrabBufferListSize], 0xFF);
         }
      else
         break;
      }
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);


   if (MilGrabBufferListSize == 0)
      {
      MosPrintf(MIL_TEXT("!!! No grab buffers have been allocated. Need to set more Non-Paged Memory. !!!\n"));

      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      return 1;
      }

   /* Initialize the User's processing function data structure only for Display. */
   ProcessingUserHookData.MilDigitizer = MilDigitizer;
   ProcessingUserHookData.MilSeqContext = M_NULL;
   ProcessingUserHookData.MilImageDisp = MilImageDisp;
   ProcessingUserHookData.ProcessedImageCount = 0;
   ProcessingUserHookData.ProcessingOperation = DISPLAY;

   /* Start MdigProcess() to show the live camera output. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction, &ProcessingUserHookData);

   /* Print a message. */
   MosPrintf(MIL_TEXT("\nH.264 IMAGE SEQUENCE COMPRESSION.\n"));
   MosPrintf(MIL_TEXT("---------------------------------\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to start compression.\n"));
   MosGetch();

   /* Stop MdigProcess(). */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_STOP, M_DEFAULT, ProcessingFunction, &ProcessingUserHookData);

   /* Inquire the rate at which the frames were grabbed using MdigProcess()*/
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &EncodingDesiredFrameRate);
   MosPrintf(MIL_TEXT("Grabbing frames at %.2f frames/sec.\n"), EncodingDesiredFrameRate);

   /* Creates a context for the H.264 compression engine. Compression will be done
      using hardware or software depending on the system hardware configuration.
      */
   MseqAlloc(MilSystem, M_DEFAULT, M_SEQ_COMPRESS, M_DEFAULT,
             M_DEFAULT, &MilCompressContext);

   /* Specify the destination of the compressed file and the target container type.
      The last argument specifies to generate an MP4 file.
      */
   MseqDefine(MilCompressContext, M_SEQ_OUTPUT(0) + M_SEQ_DEST(0), M_FILE,
              (MilSystemLocation != M_REMOTE ? SEQUENCE_FILE : REMOTE_SEQUENCE_FILE),
              M_FILE_FORMAT_MP4);

   /* Set the compression context's settings. */
   /* Sets the compression context's settings to compress frames at any resolution under
      1920 x 1080. Any resolution higher than that will generate a warning that can be disabled
     using MseqControl with M_SETTING_AUTO_ADJUSTMENT. See documentation for more details.
      */
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_BIT_RATE_MODE, M_VARIABLE);   // M_VARIABLE or M_CONSTANT
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_BIT_RATE_MAX, 25000);         // 25 Mbps bit rate
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_BIT_RATE, 10000);             // 10 Mbps bit rate
   
   if (EncodingDesiredFrameRate != 0)
      MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_FRAME_RATE, EncodingDesiredFrameRate);
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_FRAME_RATE_MODE, M_VARIABLE); // Attempts to update the file header with the encoding frame rate
                                                                                     // if lower than the specified frame rate.
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_PROFILE, M_PROFILE_HIGH);     // M_PROFILE_BASELINE, M_PROFILE_MAIN, M_PROFILE_HIGH
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_LEVEL, M_LEVEL_4_2);          // M_LEVEL_1, M_LEVEL_1B, M_LEVEL_1_1, M_LEVEL_1_2, M_LEVEL_1_3,
                                                                                     // M_LEVEL_2, M_LEVEL_2_1, M_LEVEL_2_2,
                                                                                     // M_LEVEL_3, M_LEVEL_3_1, M_LEVEL_3_2,
                                                                                     // M_LEVEL_4, M_LEVEL_4_1, M_LEVEL_4_2,
                                                                                     // M_LEVEL_5, M_LEVEL_5_1
   MseqControl(MilCompressContext, M_CONTEXT, M_STREAM_GROUP_OF_PICTURE_SIZE, 30);   // Interval between I-Frame

   /* Initialize the optional encoding end function data structure. */
   EncodingFrameEndUserHookData.EncodedImageCount = 0;

   /* Register the encoding end function to the sequence context. */
   MseqHookFunction(MilCompressContext, M_FRAME_END, FrameEncodingEndFunction,
                    &EncodingFrameEndUserHookData);

   /* Provide a sample image to initialize the encoding engine accordingly. */
   MseqControl(MilCompressContext, M_CONTEXT, M_BUFFER_SAMPLE, MilGrabBufferList[0]);

   /* Disable error prints because MseqProcess() might not support the current input source. */
   MappControl(M_ERROR, M_PRINT_DISABLE);

   /* Start the encoding process, waits for buffer to be fed for encoding. */
   MseqProcess(MilCompressContext, M_START, M_ASYNCHRONOUS);

   /* Checks if an error has been logged by MseqProcess(). If so, stop the example. */
   if (CheckMseqProcessError(MilApplication, MilCompressContext))
      {
      /* An error happened during MseqProcess() and we need to free the allocated resources. */
      MseqProcess(MilCompressContext, M_STOP, M_NULL);

      MIL_INT SourceSizeX, SourceSizeY;
      MIL_DOUBLE SourceFPS;

      MdigInquire(MilDigitizer, M_SIZE_X, &SourceSizeX);
      MdigInquire(MilDigitizer, M_SIZE_Y, &SourceSizeY);
      MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &SourceFPS);

      MosPrintf(MIL_TEXT("Unable to perform H.264 encoding with the current input source of\n"));
      MosPrintf(MIL_TEXT("%d X %d @ %.2f fps.\n"), (int)SourceSizeX, (int)SourceSizeY, SourceFPS);
      MosPrintf(MIL_TEXT("\nExample parameters are optimized for sources of\n"));
      MosPrintf(MIL_TEXT("1920 x 1080 @ 60 fps.\n"));
      MosPrintf(MIL_TEXT("\nYou can try changing encoding parameters to better match your source.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();

      while (MilGrabBufferListSize > 0)
         {
         MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
         MilGrabBufferList[MilGrabBufferListSize] = M_NULL;
         }

      MseqFree(MilCompressContext);
      MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
      return 0;
      }

   /* MseqProcess() is running without error, so re-enable the error print. */
   MappControl(M_ERROR, M_PRINT_ENABLE);

   /* Display the type of compression used. */
   MosPrintf(MIL_TEXT("Live image capture and compression to file using "));
   MseqInquire(MilCompressContext, M_CONTEXT, M_CODEC_TYPE, &SeqSystemType);
   if (SeqSystemType & M_HARDWARE)
      MosPrintf(MIL_TEXT("Hardware acceleration.\n"));
   else // M_SOFTWARE + M_QSV
      MosPrintf(MIL_TEXT("Software implementation.\n"));

   /* Set the sequence context id in the user hook data structure to start
      feeding buffers for encoding in ProcessingFunction.
      */
   ProcessingUserHookData.MilSeqContext = MilCompressContext;
   ProcessingUserHookData.ProcessedImageCount = 0;
   ProcessingUserHookData.ProcessingOperation = ENCODE;

   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, M_START, M_DEFAULT, ProcessingFunction, &ProcessingUserHookData);

   /* NOTE: Now the main() is free to perform other tasks while the compression is executing. */
   /* --------------------------------------------------------------------------------------- */

   /* Print a message and wait for a key press after a minimum number of frames. */
   MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
   MosGetch();

   /* Stop the processing. */
   MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize,
               M_STOP + M_WAIT, M_DEFAULT, ProcessingFunction, &ProcessingUserHookData);

   /* Stop the encoding process */
   MseqProcess(MilCompressContext, M_STOP, M_WAIT);

   /* Print statistics. */
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_COUNT, &ProcessFrameCount);
   MdigInquire(MilDigitizer, M_PROCESS_FRAME_RATE, &ProcessFrameRate);
   MosPrintf(MIL_TEXT("%d frames encoded at %.2f frames/sec (%.1f ms/frame).\n\n"),
             (int)ProcessFrameCount, ProcessFrameRate, 1000.0 / ProcessFrameRate);
   MseqInquire(MilCompressContext, M_SEQ_OUTPUT(0) + M_SEQ_DEST(0), M_STREAM_FILE_NAME, SeqProcessFilePath);

   /* Free the grab buffers and sequence context. */
   while (MilGrabBufferListSize > 0)
      {
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
      MilGrabBufferList[MilGrabBufferListSize] = M_NULL;
      }

   MseqFree(MilCompressContext);

   if(ProcessFrameCount > 1)
      {
      MosPrintf(MIL_TEXT("The video sequence file was written to:\n%s.\n\n"), SeqProcessFilePath.c_str());
      MosPrintf(MIL_TEXT("It can be played back using any compatible video player.\n"));

      /* Wait for a key to start the replay. */
      MosPrintf(MIL_TEXT("Press <Enter> to replay encoded sequence.\n"));
      MosGetch();


      MseqAlloc(MilSystem, M_DEFAULT, M_SEQ_DECOMPRESS, M_DEFAULT,
                M_DEFAULT, &MilDecompressContext);

      MappControl(M_ERROR, M_PRINT_DISABLE);

      /* Specify the destination of the compressed file and the target container type.
         The last argument specifies to generate an MP4 file.
         */
      MseqDefine(MilDecompressContext, M_SEQ_INPUT(0), M_FILE,
         (MilSystemLocation != M_REMOTE ? SEQUENCE_FILE : REMOTE_SEQUENCE_FILE),
                 M_FILE_FORMAT_MP4);

      if(PrintMilErrorMessage(MilApplication))
         {
         MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
         MosGetch();
         MseqFree(MilDecompressContext);
         MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);
         return 0;
         }
      MappControl(M_ERROR, M_PRINT_ENABLE);

      MIL_DOUBLE outputFrameRate = 0.0;
      MseqInquire(MilDecompressContext, M_SEQ_INPUT(0), M_STREAM_FRAME_RATE, &outputFrameRate);
      MosPrintf(MIL_TEXT("\nReplaying file at %.2f frames/second.\n"), outputFrameRate);

      /* Initialize the optional decoding end function data structure. */
      DecodingFrameEndUserHookData.DecodedImageCount = 0;
      DecodingFrameEndUserHookData.MilImageDisp = MilImageDisp;

      /* Register the decoding end function to the sequence context. */
      MseqHookFunction(MilDecompressContext, M_FRAME_END, FrameDecodingEndFunction,
                       &DecodingFrameEndUserHookData);

      /* Start the decoding process, waits for buffer to be fed for encoding. */
      MseqProcess(MilDecompressContext, M_START, M_ASYNCHRONOUS);

      /* Print a message and wait for a key press after a minimum number of frames. */
      MosPrintf(MIL_TEXT("Press <Enter> to stop.\n\n"));
      MosGetch();

      /* Stop the play back. */
      MseqProcess(MilDecompressContext, M_STOP, M_NULL);
      MseqFree(MilDecompressContext);

      }
   else
      {
      MosPrintf(MIL_TEXT("Did not record enough frames to be able to replay.\n"));
      }

   /* Wait for a key to end. */
   MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
   MosGetch();

   /* Release defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp);

   return 0;
   }

/* Local defines. */
#define STRING_LENGTH_MAX  20
#define STRING_POS_X       20
#define STRING_POS_Y       20

/* User's processing function called every time a grab buffer is modified. */
/* ------------------------------------------------------------------------*/
MIL_INT MFTYPE ProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   ProcessingHookDataStruct *UserHookDataPtr = (ProcessingHookDataStruct *)HookDataPtr;
   MIL_ID ModifiedBufferId;
   MIL_TEXT_CHAR Text[STRING_LENGTH_MAX] = { MIL_TEXT('\0'), };

   /* Retrieve the MIL_ID of the grabbed buffer. */
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &ModifiedBufferId);

   switch (UserHookDataPtr->ProcessingOperation)
      {
      case DISPLAY:
         /* Update the display with the last captured image. */
         MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
         break;

      case ENCODE:
         /* Increase the compressed images count. */
         UserHookDataPtr->ProcessedImageCount++;

         /* Print and draw the frame count (comment this block to reduce CPU usage). */
         MosPrintf(MIL_TEXT("Processing frame #%d.\r"), (int)UserHookDataPtr->ProcessedImageCount);
         MosSprintf(Text, STRING_LENGTH_MAX, MIL_TEXT("%d"), (int)UserHookDataPtr->ProcessedImageCount);
         MgraText(M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, Text);

         /* Enqueue the grabbed buffer for parallel encoding. */
         MseqFeed(UserHookDataPtr->MilSeqContext, ModifiedBufferId, M_DEFAULT);

         /* Update the display with the last captured image. */
         MbufCopy(ModifiedBufferId, UserHookDataPtr->MilImageDisp);
         break;
      }
   return 0;
   }

/* Optional encoding end function called every time a buffer is finished being compressed. */
/* ----------------------------------------------------------------------------------------*/
MIL_INT MFTYPE FrameEncodingEndFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   EncodingFrameEndHookDataStruct * UserHookDataPtr = (EncodingFrameEndHookDataStruct *)HookDataPtr;

   /* Frame end hook post processing. */
   if (HookType == M_FRAME_END)
      {
      MIL_ID CompressedBufferId;
      void* CompressedDataPtr = M_NULL;
      MIL_INT CompressedDataSize = 0;

      /* Increment a encoded frame counter */
      UserHookDataPtr->EncodedImageCount++;

      /* Retrieve the MIL_ID of the encoded buffer. */
      MseqGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &CompressedBufferId);

      /* Retrieves the address of the encoded data. */
      MbufInquire(CompressedBufferId, M_HOST_ADDRESS, &CompressedDataPtr);

      /* Retrieves the size in bytes of the encoded data. */
      MbufInquire(CompressedBufferId, M_COMPRESSED_DATA_SIZE_BYTE, &CompressedDataSize);

      /* -----------------------------------------------------------------------------------------------*/
      /* Here you can do any action with the encoded data, such as send the buffer through a network    */
      /* stream. If the processing done on the compressed data is long, it is recommended to copy the   */
      /* buffer and to process it in a separate thread to avoid blocking the compression's flow.        */
      /* -----------------------------------------------------------------------------------------------*/

      }

   return 0;
   }

/* Optional decoding end function called every time a buffer is finished being decompressed. */
/* ----------------------------------------------------------------------------------------*/
MIL_INT MFTYPE FrameDecodingEndFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   DecodingFrameEndHookDataStruct * UserHookDataPtr = (DecodingFrameEndHookDataStruct *)HookDataPtr;

   /* Frame end hook post processing. */
   if (HookType == M_FRAME_END)
      {
      MIL_ID DecompressedBufferId;

      /* Increment a encoded frame counter */
      UserHookDataPtr->DecodedImageCount++;

      /* Retrieve the MIL_ID of the encoded buffer. */
      MseqGetHookInfo(HookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &DecompressedBufferId);

      /* -----------------------------------------------------------------------------------------------*/
      /* Here you can do any action with the decoded buffer.                                            */
      /* -----------------------------------------------------------------------------------------------*/

      MbufCopy(DecompressedBufferId, UserHookDataPtr->MilImageDisp);
      }

   return 0;
   }

/* Checks if MseqProcess generated an error or a warning.                                              */
/* This function prints out the MIL error message, if any.                                             */
/* If a sequence context parameter has been modified, it means that only a warning has been generated. */
/* If it is a warning, it displays the control that has been modified.                                 */
/* If it is an error, it return M_YES to indicate that the example needs to be stopped.                */
/* ----------------------------------------------------------------------------------------------------*/
MIL_INT CheckMseqProcessError(MIL_ID MilApplication, MIL_ID MilCompressContext)
   {
   struct MSEQ_PARAM
      {
      MIL_CONST_TEXT_PTR ControlName;
      MIL_INT ControlType;
      MIL_INT OriginalValue;
      MIL_INT EffectiveValue;
      };

   MIL_INT IsError = M_NO;
   MIL_INT IsWarning = M_NO;
   MIL_INT MilErrorCode = PrintMilErrorMessage(MilApplication);

   /* MseqProcess generated an error, check in details if it is a warning or an error*/
   if (MilErrorCode != M_NULL_ERROR)
      {
      MSEQ_PARAM MseqParamList[] = {
            { MIL_TEXT("M_STREAM_BIT_RATE_MODE"), M_STREAM_BIT_RATE_MODE, 0, 0 },
            { MIL_TEXT("M_STREAM_BIT_RATE"), M_STREAM_BIT_RATE, 0, 0 },
            { MIL_TEXT("M_STREAM_BIT_RATE_MAX"), M_STREAM_BIT_RATE_MAX, 0, 0 },
            { MIL_TEXT("M_STREAM_FRAME_RATE_MODE"), M_STREAM_FRAME_RATE_MODE, 0, 0 },
            { MIL_TEXT("M_STREAM_QUALITY"), M_STREAM_QUALITY, 0, 0 },
            { MIL_TEXT("M_STREAM_PROFILE"), M_STREAM_PROFILE, 0, 0 },
            { MIL_TEXT("M_STREAM_LEVEL"), M_STREAM_LEVEL, 0, 0 },
            { MIL_TEXT("M_STREAM_GROUP_OF_PICTURE_SIZE"), M_STREAM_GROUP_OF_PICTURE_SIZE, 0, 0 }};
      MIL_INT MseqParamListSize = (sizeof(MseqParamList) / sizeof(MseqParamList[0]));
      MIL_INT NumberOfModifiedParams = 0;

      /* Loop though the param list to find which one has been internally modified. */
      for (MIL_INT ParamIndex = 0; ParamIndex < MseqParamListSize; ParamIndex++)
         {
         /* Query the original control values. */
         MseqInquire(MilCompressContext,
                     M_CONTEXT,
                     MseqParamList[ParamIndex].ControlType,
                     &MseqParamList[ParamIndex].OriginalValue);

         /* Query the effective control values. */
         MseqInquire(MilCompressContext,
                     M_CONTEXT,
                     MseqParamList[ParamIndex].ControlType | M_EFFECTIVE_VALUE,
                     &MseqParamList[ParamIndex].EffectiveValue);

         /* If the original value is different than the effective value,
            the error received is only a warning and processing can continue.
            */
         if (MseqParamList[ParamIndex].OriginalValue != MseqParamList[ParamIndex].EffectiveValue)
            {
            if (NumberOfModifiedParams == 0)
               MosPrintf(MIL_TEXT("\nParameter(s) that have been internally modified:\n"));

            /* Prints the Control type internally modified. */
            MosPrintf(MIL_TEXT("- %s\n"), MseqParamList[ParamIndex].ControlName);
            NumberOfModifiedParams++;
            IsWarning = M_YES;
            }
         }
      MosPrintf(MIL_TEXT("\n"));

      /* If the error logged is not a warning, you cannot encode the current input source.
         The example needs to be stopped.
         */
      if (!IsWarning)
         IsError = M_YES;
      }

   return IsError;
   }

/* Print the current error code in the console */
MIL_INT PrintMilErrorMessage(MIL_ID MilApplication)
   {
   MIL_INT           MilErrorCode;
   MIL_STRING        MilErrorMsg;
   MIL_INT           MilErrorSubCode[3];
   MIL_STRING        MilErrorSubMsg[3];
   MilErrorCode = MappGetError(MilApplication, M_CURRENT + M_MESSAGE, MilErrorMsg);
   if (MilErrorCode != M_NULL_ERROR)
      {
      /* Collects Mil error messages and sub-messages */
      MIL_INT subCount = 3;
      MappGetError(MilApplication, M_CURRENT_SUB_NB, &subCount);
      MilErrorSubCode[0] = MappGetError(MilApplication,
                                        M_CURRENT_SUB_1 + M_MESSAGE,
                                        MilErrorSubMsg[0]);
      MilErrorSubCode[1] = MappGetError(MilApplication,
                                        M_CURRENT_SUB_2 + M_MESSAGE,
                                        MilErrorSubMsg[1]);
      MilErrorSubCode[2] = MappGetError(MilApplication,
                                        M_CURRENT_SUB_3 + M_MESSAGE,
                                        MilErrorSubMsg[2]);

      MosPrintf(MIL_TEXT("\nMseqProcess generated a warning or an error:\n"));
      MosPrintf(MIL_TEXT("  %s\n"), MilErrorMsg.c_str());
      for (int i = 0; i < subCount; i++)
         {
         if (MilErrorSubCode[i]) MosPrintf(MIL_TEXT("  %s\n"), MilErrorSubMsg[i].c_str());
         }
      }
   return MilErrorCode;
   }
