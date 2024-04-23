#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##########################################################################
#
# File name: MseqProcess.py 
#
# Synopsis:  This program shows the use of the MseqProcess() and MseqFeed() functions
#            to perform real-time encoding of a sequence of captured images.
#
#            The user's preprocessing and compression code is written in a hook
#            function that will be called by MdigProcess() for each frame grabbed
#            (see ProcessingFunction()). The queueing for encoding of the next
#            frame is also made in that hook function to allow fully parallel
#            execution of the capture and the encoding.
#
#      Note: The average encoding time must be shorter than the grab time or
#            some frames will be missed. Missed frames are very frequent when
#            the encoding is done by software. Also, if the captured images
#            are not displayed and the frame count is not drawn or printed
#            in the hook function, the CPU usage is reduced significantly.
#
#            When encoding a 1080p source it is recommended to have your
#            MIL Non-Paged Memory set to at least 64MB.
#
# Copyright © Matrox Electronic Systems Ltd., 1992-2023.
# All Rights Reserved

from enum import Enum 
import mil as MIL

# Number of images in the buffering grab queue.
# Generally, increasing this number gives better real-time grab.
BUFFERING_SIZE_MAX = 20

# Target sequence file name and location.
# The temporary directory location can be reached with %temp% under Windows.
SEQUENCE_FILE = MIL.M_TEMP_DIR + "SeqProcess.mp4"

# Remote target sequence file name and location if Distributed MIL is used.
REMOTE_SEQUENCE_FILE = "remote:///" + SEQUENCE_FILE

class ProcessingHookOperation(Enum):
   DISPLAY = 0
   ENCODE = 1

# User's processing function hook data object.
class ProcessingHookDataStruct:
   def __init__(self, MilDigitizer, MilImageDisp, MilSeqContext, ProcessedImageCount, ProcessingOperation):
      self.MilDigitizer = MilDigitizer
      self.MilImageDisp = MilImageDisp
      self.MilSeqContext = MilSeqContext
      self.ProcessedImageCount = ProcessedImageCount
      self.ProcessingOperation = ProcessingOperation
      
# Optional encoding end function hook data object.
class EncodingFrameEndHookDataStruct:
   def __init__(self, EncodedImageCount):
      self.EncodedImageCount = EncodedImageCount
      
# Optional decoding end function hook data object.
class DecodingFrameEndHookDataStruct:
   def __init__(self, DecodedImageCount, MilImageDisp):
      self.DecodedImageCount = DecodedImageCount
      self.MilImageDisp = MilImageDisp

def MseqProcessExample():
   MilGrabBufferList = []

   # Allocate defaults.
   MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp = MIL.MappAllocDefault(MIL.M_DEFAULT)

   MilRemoteApplication = MIL.MsysInquire(MilSystem, MIL.M_OWNER_APPLICATION)
   MilSystemLocation = MIL.MsysInquire(MilSystem, MIL.M_LOCATION)
   
   # Inquire MIL licenses.
   LicenseModules = MIL.MappInquire(MilRemoteApplication, MIL.M_LICENSE_MODULES)
   if not LicenseModules & MIL.M_LICENSE_JPEGSTD:
      print("Need a Compression/Decompression license to run this example.")
      print("Press <Enter> to end.")
      MIL.MosGetch()

      MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)
      return 0
   
   # Allocate the grab buffers and clear them.
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   
   for i in range(BUFFERING_SIZE_MAX):
      MilGrabBufferList.append(MIL.MbufAllocColor(MilSystem, 
                                                  MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_BAND),
                                                  MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X),
                                                  MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y),
                                                  8 + MIL.M_UNSIGNED, 
                                                  MIL.M_IMAGE + MIL.M_GRAB + MIL.M_PROC))
      
      if MilGrabBufferList[i]:
         MIL.MbufClear(MilGrabBufferList[i], 0xFF)
      else:
         MilGrabBufferList.pop(i)
         break
   MilGrabBufferListSize = len(MilGrabBufferList)

   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)
   
   if MilGrabBufferListSize == 0:
      print("!!! No grab buffers have been allocated. Need to set more Non-Paged Memory. !!!")
      MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)
      print("Press <Enter> to end.")
      MIL.MosGetch()
      return 1
   
   # Initialize the User's processing function data structure only for Display.
   ProcessingUserHookData = ProcessingHookDataStruct(MilDigitizer, MilImageDisp, MIL.M_NULL, 0, ProcessingHookOperation.DISPLAY)

   # Start the sequence acquisition. The preprocessing and encoding hook function.
   # is called for every frame grabbed.
   ProcessingFunctionPtr = MIL.MIL_DIG_HOOK_FUNCTION_PTR(ProcessingFunction)

   # Start MdigProcess() to show the live camera output.
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_START, MIL.M_DEFAULT, ProcessingFunctionPtr, ProcessingUserHookData)
   
   # Print a message.
   print("\nH.264 IMAGE SEQUENCE COMPRESSION.")
   print("---------------------------------\n")
   print("Press <Enter> to start compression.")
   MIL.MosGetch() 

   # Stop MdigProcess().
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_STOP, MIL.M_DEFAULT, ProcessingFunctionPtr, ProcessingUserHookData)
   
   # Inquire the rate at which the frames were grabbed using MdigProcess()
   EncodingDesiredFrameRate = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)
   print("Grabbing frames at {EncodingDesiredFrameRate} frames/sec.".format(EncodingDesiredFrameRate=round(EncodingDesiredFrameRate,2)))
   
   # Creates a context for the H.264 compression engine. Compression will be done
   # using hardware or software depending on the system hardware configuration. 
   MilCompressContext = MIL.MseqAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_SEQ_COMPRESS, MIL.M_DEFAULT, MIL.M_DEFAULT)
   
   # Specify the destination of the compressed file and the target container type.
   # The last argument specifies to generate an MP4 file.
   MIL.MseqDefine(MilCompressContext, MIL.M_SEQ_OUTPUT(0) + MIL.M_SEQ_DEST(0), MIL.M_FILE,
                SEQUENCE_FILE if MilSystemLocation != MIL.M_REMOTE else REMOTE_SEQUENCE_FILE,
                MIL.M_FILE_FORMAT_MP4)
   
   # Set the compression context's settings.
   # Sets the compression context's settings to compress frames at any resolution under
   # 1920 x 1080. Any resolution higher than that will generate a warning that can be disa# Free all allocations.
   # using MseqControl with M_SETTING_AUTO_ADJUSTMENT. See documentation for more details.MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)
   
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_BIT_RATE_MODE, MIL.M_VARIABLE)         # MIL.M_VARIABLE 
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_BIT_RATE, 5000)                        # 5 Mbps bit rate
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_BIT_RATE_MAX, 5000)                    # 5 Mbps bit ratereturn 0
   
   if EncodingDesiredFrameRate != 0:
      MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_FRAME_RATE, EncodingDesiredFrameRate)    #60Hz frame rate.
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_FRAME_RATE_MODE, MIL.M_VARIABLE)            # Attempts to update the file header with the encoding frame rate
                                                                                                               # if lower than the specified frame rate.
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_PROFILE, MIL.M_PROFILE_HIGH)            # MIL.M_PROFILE_BASELINE, MIL.M_PROFILE_MAIN, MIL.M_PROFILE_HIGH
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_LEVEL, MIL.M_LEVEL_4_2)                 # MIL.M_LEVEL_1, MIL.M_LEVEL_1B, MIL.M_LEVEL_1_1, MIL.M_LEVEL_1_2, MIL.M_LEVEL_1_3,
   
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_STREAM_GROUP_OF_PICTURE_SIZE, 30)              # Interval between I-Frame
   
   # Initialize the optional encoding end function data structure.
   EncodingFrameEndUserHookData = EncodingFrameEndHookDataStruct(0)
   
   # Register the encoding end function to the sequence context. 
   FrameEncodingEndFunctionDelegate = MIL.MIL_SEQ_HOOK_FUNCTION_PTR(FrameEncodingEndFunction)
   MIL.MseqHookFunction(MilCompressContext, MIL.M_FRAME_END, FrameEncodingEndFunctionDelegate, EncodingFrameEndUserHookData)
   
   # Provide a sample image to initialize the encoding engine accordingly.
   MIL.MseqControl(MilCompressContext, MIL.M_CONTEXT, MIL.M_BUFFER_SAMPLE, MilGrabBufferList[0])
   
   # Disable error prints because MseqProcess() might not support the current input source.
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
   
   # Start the encoding process, waits for buffer to be fed for encoding.
   MIL.MseqProcess(MilCompressContext, MIL.M_START, MIL.M_ASYNCHRONOUS)
   
   if CheckMseqProcessError(MilApplication, MilCompressContext):
      # An error happened during MseqProcess() and we need to free the allocated resources.
      MIL.MseqProcess(MilCompressContext, MIL.M_STOP, MIL.M_NULL)
      
      SourceSizeX = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_X)
      SourceSizeY = MIL.MdigInquire(MilDigitizer, MIL.M_SIZE_Y)
      SourceFPS = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)
   
      print("Unable to perform H.264 encoding with the current input source of")
      print("{SourceSizeX} X {SourceSizeY} @ {SourceFPS} fps.".format(SourceSizeX = SourceSizeX, SourceSizeY = SourceSizeY, SourceFPS = round(SourceFPS,2)))
      print("\nExample parameters are optimized for sources of")
      print("1920 x 1080 @ 60 fps.")
      print("\nYou can try changing encoding parameters to better match your source.\n")
      
      print("Press <Enter> to end.")
      MIL.MosGetch()
      
      for i in range(MilGrabBufferListSize):
         MIL.MbufFree(MilGrabBufferList[i])
         MilGrabBufferList[i] = MIL.M_NULL
         
      MIL.MseqFree(MilCompressContext)
      MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)
      return 0
      
   # MseqProcess() is running without error, so re-enable the error print.
   MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)
   
   # Display the type of compression used.
   compressionType = "Live image capture and compression to file using "
   SeqSystemType = MIL.MseqInquire(MilCompressContext, MIL.M_CONTEXT, MIL.M_CODEC_TYPE)
   if SeqSystemType & MIL.M_HARDWARE:
      print(compressionType + "Hardware acceleration.")
   else:
      # MIL.M_SOFTWARE  MIL.M_QSV
      print(compressionType + "Software implementation.")
   
   # Set the sequence context id in the user hook data structure to start
   # feeding buffers for encoding in ProcessingFunction.
   ProcessingUserHookData.MilSeqContext = MilCompressContext
   ProcessingUserHookData.ProcessedImageCount = 0
   ProcessingUserHookData.ProcessingOperation = ProcessingHookOperation.ENCODE
   
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_START, MIL.M_DEFAULT, ProcessingFunctionPtr, ProcessingUserHookData)
   
   # NOTE: Now the main() is free to perform other tasks while the compression is executing.
   
   # Print a message and wait for a key press after a minimum number of frames.
   print("Press <Enter> to stop.\n")
   MIL.MosGetch()

   # Stop the processing. 
   MIL.MdigProcess(MilDigitizer, MilGrabBufferList, MilGrabBufferListSize, MIL.M_STOP + MIL.M_WAIT, MIL.M_DEFAULT, ProcessingFunctionPtr, ProcessingUserHookData)
   
   # Stop the encoding process.
   MIL.MseqProcess(MilCompressContext, MIL.M_STOP, MIL.M_WAIT)
   
   # Print statistics.
   ProcessFrameCount = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_COUNT)
   ProcessFrameRate = MIL.MdigInquire(MilDigitizer, MIL.M_PROCESS_FRAME_RATE)
   if ProcessFrameRate > 0:
      MsPerFrame = 1000.0 / ProcessFrameRate
      print("{ProcessFrameCount} frames encoded at {ProcessFrameRate} frames/sec ({MsPerFrame} ms/frame).\n".format(ProcessFrameCount=ProcessFrameCount, ProcessFrameRate=round(ProcessFrameRate,2), MsPerFrame=round(MsPerFrame,1)))
   
   SeqProcessFilePath = MIL.MseqInquire(MilCompressContext, MIL.M_SEQ_OUTPUT(0) + MIL.M_SEQ_DEST(0), MIL.M_STREAM_FILE_NAME)
   
   # Free the grab buffers and sequence context.


   for i in range(MilGrabBufferListSize):
      MIL.MbufFree(MilGrabBufferList[i])
      MilGrabBufferList[i] = MIL.M_NULL
   
   MIL.MseqFree(MilCompressContext)

   if ProcessFrameCount > 1:
      print("The video sequence file was written to:\n" + SeqProcessFilePath + ".\n")
      print("It can be played back using any compatible video player.")
      
      # Wait for a key to start the replay.
      print("Press <Enter> to replay encoded sequence.")
      MIL.MosGetch()
      
      MilDecompressContext = MIL.MseqAlloc(MilSystem, MIL.M_DEFAULT, MIL.M_SEQ_DECOMPRESS, MIL.M_DEFAULT, MIL.M_DEFAULT)
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_DISABLE)
      
      # Specify the destination of the compressed file and the target container type.
      # The last argument specifies to generate an MP4 file.
      MIL.MseqDefine(MilDecompressContext, MIL.M_SEQ_INPUT(0), MIL.M_FILE,
                            (SEQUENCE_FILE if MilSystemLocation != MIL.M_REMOTE else REMOTE_SEQUENCE_FILE),
                            MIL.M_FILE_FORMAT_MP4)
      
      if PrintMilErrorMessage(MilApplication):
         print("\nPress <Enter> to end.")
         MIL.MosGetch()
         MIL.MseqFree(MilDecompressContext)
         MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)
         return 0
         
      MIL.MappControl(MIL.M_DEFAULT, MIL.M_ERROR, MIL.M_PRINT_ENABLE)
      
      outputFrameRate = MIL.MseqInquire(MilDecompressContext, MIL.M_SEQ_INPUT(0), MIL.M_STREAM_FRAME_RATE)
      print("\nReplaying file at {outputFrameRate} frames/second.".format(outputFrameRate=round(outputFrameRate,2)))
      
      # Initialize the optional decoding end function data structure.
      DecodingFrameEndUserHookData = DecodingFrameEndHookDataStruct(0, MilImageDisp)

      # Register the decoding end function to the sequence context.
      FrameDecodingEndFunctionPtr = MIL.MIL_SEQ_HOOK_FUNCTION_PTR(FrameDecodingEndFunction)
      MIL.MseqHookFunction(MilDecompressContext, MIL.M_FRAME_END, FrameDecodingEndFunctionPtr, 
                       DecodingFrameEndUserHookData)

      # Start the decoding process, waits for buffer to be fed for encoding.
      MIL.MseqProcess(MilDecompressContext, MIL.M_START, MIL.M_ASYNCHRONOUS)

      # Print a message and wait for a key press after a minimum number of frames.
      print("Press <Enter> to stop.\n")
      MIL.MosGetch()

      # Stop the play back.
      MIL.MseqProcess(MilDecompressContext, MIL.M_STOP, MIL.M_NULL)
      MIL.MseqFree(MilDecompressContext)

   else:
      print("Did not record enough frames to be able to replay.")

   # Wait for a key to end.
   print("Press <Enter> to end.")
   MIL.MosGetch()

   # Release defaults. 
   MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MilDigitizer, MilImageDisp)

   return 0

# Local defines. 
STRING_LENGTH_MAX = 20
STRING_POS_X      = 20
STRING_POS_Y      = 20

# User's processing function called every time a grab buffer is modified.  #
# ------------------------------------------------------------------------ #
def ProcessingFunction(HookType, HookId, HookDataPtr):
   UserHookDataPtr = HookDataPtr

   # Retrieve the MIL_ID of the grabbed buffer. 
   ModifiedBufferId = MIL.MdigGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

   if UserHookDataPtr.ProcessingOperation == ProcessingHookOperation.DISPLAY:
      # Update the display with the last captured image. 
      MIL.MbufCopy(ModifiedBufferId, UserHookDataPtr.MilImageDisp)
   
   elif UserHookDataPtr.ProcessingOperation == ProcessingHookOperation.ENCODE:
      # Increase the compressed images count.
      UserHookDataPtr.ProcessedImageCount += 1

      # Print and draw the frame count (comment this block to reduce CPU usage).
      print("Processing frame #{ProcessedImageCount} \r".format(ProcessedImageCount=UserHookDataPtr.ProcessedImageCount), end="")
      MIL.MgraText(MIL.M_DEFAULT, ModifiedBufferId, STRING_POS_X, STRING_POS_Y, str(UserHookDataPtr.ProcessedImageCount))

      # Enqueue the grabbed buffer for parallel encoding.
      MIL.MseqFeed(UserHookDataPtr.MilSeqContext, ModifiedBufferId, MIL.M_DEFAULT)

      # Update the display with the last captured image. 
      MIL.MbufCopy(ModifiedBufferId, UserHookDataPtr.MilImageDisp)
   
   return 0

# Optional encoding end function called every time a buffer is finished being compressed. #
# ----------------------------------------------------------------------------------------#
def FrameEncodingEndFunction(HookType, HookId, HookDataPtr):
   UserHookDataPtr = HookDataPtr

   # Frame end hook post processing.
   if HookType == MIL.M_FRAME_END:

      # Increment a encoded frame counter 
      UserHookDataPtr.EncodedImageCount += 1

      # Retrieve the MIL_ID of the encoded buffer.
      CompressedBufferId = MIL.MseqGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

      # Retrieves the address of the encoded data.
      CompressedDataPtr = MIL.MbufInquire(CompressedBufferId, MIL.M_HOST_ADDRESS)

      # Retrieves the size in bytes of the encoded data. 
      CompressedDataSize = MIL.MbufInquire(CompressedBufferId, MIL.M_COMPRESSED_DATA_SIZE_BYTE)

      # -----------------------------------------------------------------------------------------------#
      # Here you can do any action with the encoded data, such as send the buffer through a network    #
      # stream. If the processing done on the compressed data is long, it is recommended to copy the   #
      # buffer and to process it in a separate thread to avoid blocking the compression's flow.        #
      # -----------------------------------------------------------------------------------------------#

   return 0

# Optional decoding end function called every time a buffer is finished being decompressed.
# ----------------------------------------------------------------------------------------
def FrameDecodingEndFunction(HookType, HookId, HookDataPtr):
   UserHookDataPtr = HookDataPtr

   # Frame end hook post processing. 
   if (HookType == MIL.M_FRAME_END):
      # Increment a encoded frame counter 
      UserHookDataPtr.DecodedImageCount += 1

      # Retrieve the MIL_ID of the encoded buffer.
      DecompressedBufferId = MIL.MseqGetHookInfo(HookId, MIL.M_MODIFIED_BUFFER + MIL.M_BUFFER_ID)

      # -----------------------------------------------------------------------------------------------#
      # Here you can do any action with the decoded buffer.                                            #
      # -----------------------------------------------------------------------------------------------#

      MIL.MbufCopy(DecompressedBufferId, UserHookDataPtr.MilImageDisp)

   return 0

# Checks if MseqProcess generated an error or a warning.                                              #
# This function prints out the MIL error message, if any.                                             #
# If a sequence context parameter has been modified, it means that only a warning has been generated. #
# If it is a warning, it displays the control that has been modified.                                 #
# If it is an error, it return M_YES to indicate that the example needs to be stopped.                #
# ----------------------------------------------------------------------------------------------------#
def CheckMseqProcessError(MilApplication, MilCompressContext):
   class MSEQ_PARAM:
      def __init__(self, ControlName, ControlType, OriginalValue, EffectiveValue):
         self.ControlName = ControlName
         self.ControlType = ControlType
         self.OriginalValue = OriginalValue
         self.EffectiveValue = EffectiveValue

   IsError = MIL.M_NO
   IsWarning = MIL.M_NO
   MilErrorCode = PrintMilErrorMessage(MilApplication)

   # MseqProcess generated an error, check in details if it is a warning or an error
   if MilErrorCode != MIL.M_NULL_ERROR:
      MseqParamList = []
      MseqParamList.append(MSEQ_PARAM("M_STREAM_BIT_RATE_MODE", MIL.M_STREAM_BIT_RATE_MODE, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_BIT_RATE", MIL.M_STREAM_BIT_RATE, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_BIT_RATE_MAX", MIL.M_STREAM_BIT_RATE_MAX, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_FRAME_RATE_MODE", MIL.M_STREAM_FRAME_RATE_MODE, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_QUALITY", MIL.M_STREAM_QUALITY, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_PROFILE", MIL.M_STREAM_PROFILE, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_LEVEL", MIL.M_STREAM_LEVEL, 0, 0))
      MseqParamList.append(MSEQ_PARAM("M_STREAM_GROUP_OF_PICTURE_SIZE", MIL.M_STREAM_GROUP_OF_PICTURE_SIZE, 0, 0))
      
      MseqParamListSize = len(MseqParamList)
      NumberOfModifiedParams = 0

      # Loop though the param list to find which one has been internally modified. 
      for ParamIndex in range(MseqParamListSize):
         
         # Query the original control values.
         MseqParamList[ParamIndex].OriginalValue = MIL.MseqInquire(MilCompressContext,
                                                                   MIL.M_CONTEXT,
                                                                   MseqParamList[ParamIndex].ControlType)

         # Query the effective control values. 
         MseqParamList[ParamIndex].EffectiveValue = MIL.MseqInquire(MilCompressContext,
                                                                    MIL.M_CONTEXT,
                                                                    MseqParamList[ParamIndex].ControlType | MIL.M_EFFECTIVE_VALUE)

         # If the original value is different than the effective value,
         # the error received is only a warning and processing can continue.

         if MseqParamList[ParamIndex].OriginalValue != MseqParamList[ParamIndex].EffectiveValue:
            if NumberOfModifiedParams == 0:
               print("\nParameter(s) that have been internally modified:")

            # Prints the Control type internally modified. 
            print("- {ControlName}\n".format(ControlName=MseqParamList[ParamIndex].ControlName))
            NumberOfModifiedParams += 1
            IsWarning = MIL.M_YES
         
      print()

      # If the error logged is not a warning, you cannot encode the current input source.
      #  The example needs to be stopped.
      if not IsWarning:
         IsError = MIL.M_YES

   return IsError
   
# Print the current error code in the console
def PrintMilErrorMessage(MilApplication):
   MilErrorSubCode = []
   MilErrorSubMsg = []

   MilErrorMsg = MIL.MappGetError(MilApplication, MIL.M_CURRENT + MIL.M_MESSAGE)
   MilErrorCode = MIL.MappGetError(MilApplication, MIL.M_CURRENT)

   if MilErrorCode != MIL.M_NULL_ERROR:
      # Collects Mil error messages and sub-messages
      subCount = MIL.MappGetError(MilApplication, MIL.M_CURRENT_SUB_NB)
      MilErrorSubCode.append(MIL.MappGetError(MilApplication,
                                              MIL.M_CURRENT_SUB_1,
                                             ))
      MilErrorSubMsg.append(MIL.MappGetError(MilApplication,
                                             MIL.M_CURRENT_SUB_1 + MIL.M_MESSAGE,
                                             ))
      MilErrorSubCode.append(MIL.MappGetError(MilApplication,
                                              MIL.M_CURRENT_SUB_2,
                                             ))
      MilErrorSubMsg.append(MIL.MappGetError(MilApplication,
                                             MIL.M_CURRENT_SUB_2 + MIL.M_MESSAGE,
                                             ))
      MilErrorSubCode.append(MIL.MappGetError(MilApplication,
                                              MIL.M_CURRENT_SUB_3,
                                             ))
      MilErrorSubMsg.append(MIL.MappGetError(MilApplication,
                                             MIL.M_CURRENT_SUB_3 + MIL.M_MESSAGE,
                                             ))
      print("\nMseqProcess generated a warning or an error:")
      print("  {MilErrorMsg}\n".format(MilErrorMsg=MilErrorMsg))
      for i in range(subCount):
         if MilErrorSubCode[i]:
            print("  {ErrorMsg}".format(ErrorMsg=MilErrorSubMsg[i]))

   return MilErrorCode


if __name__ == "__main__":
   MseqProcessExample()
