﻿/////////////////////////////////////////////////////////////////////////////////////////////
//
// File name:  BlobMerge.cpp
//
// Synopsis:   This example shows two approaches to merge blob results from
//             vertically adjacent buffers.
//
//             1- Off-line analysis: blobs from all frames are extracted and accumulated into
//                a single result. The analysis of the blobs is performed thereafter.
//
//             2- On-line analysis: blobs from the current frame are extracted and merged with
//                the incomplete blobs from the previous frame. The analysis of the blobs of
//                the current frame is performed. The operation is repeated for every frame.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//////////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
 
// Example sequence file path and name.
#define  EXAMPLE_IMAGE_PATH   M_IMAGE_PATH MIL_TEXT("BlobAnalysis/BlobMerge/")
#define  SEQUENCE_FILE        EXAMPLE_IMAGE_PATH MIL_TEXT("BlobMerge.avi")

// X position offset of the second display for blob merge.
#define DISPLAY_X_OFFSET 300

// Off-line analysis functions.
void MFTYPE OfflineMergeHook(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
void OfflineBlobAnalysis(void* HookDataPtr);

// On-line analysis functions.
void MFTYPE OnlineMergeAndBlobAnalysisHook(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr);
void OnlineDrawBlobResults(void* HookDataPtr, MIL_ID BlobResultId);

// Main function prototypes.
void OfflineExample();
void OnlineExample();

// Processing function hook data structure.
typedef struct
   {
   MIL_ID MilGrabDisp;              // Display image for the last grabbed image buffer.
   MIL_ID MilBinImage;              // Binary image buffer for blob analysis.
   MIL_ID MilDispImage;             // Image to display blob merge.
   
   MIL_ID MilDisplayBlobMerge;      // Second display identifier for blob merge.
   MIL_ID MilGraphicList;           // Graphic list buffer identifier.

   MIL_ID MilBlobContext;           // Blob context identifier.
   MIL_ID MilPreviousBlobResult;    // Blob result identifier from the previous frame(s).
   MIL_ID MilCurrentBlobResult;     // Blob result identifier from the current frame.
   MIL_ID MilDestinationBlobResult; // Blob merge destination result identifier.

   MIL_INT SizeX;                   // Grab image size X.
   MIL_INT SizeY;                   // Grab image size y. 
   MIL_INT ImageCount;              // Grabbed image count.
   } HookDataStruct;

// Total number of grabbed images.
#define BUFFERING_SIZE  5

//******************************************************************************************
// Example description.
//******************************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
               MIL_TEXT("BlobMerge\n\n")
               MIL_TEXT("[SYNOPSIS]\n")
               MIL_TEXT("This example shows two approaches to merge blob results from\n")
               MIL_TEXT("vertically adjacent buffers.\n\n")
               MIL_TEXT("\n[MODULES USED]\n")
               MIL_TEXT("Modules used: application, system, display, buffer,\n")
               MIL_TEXT("blob, graphics, image processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//********************************************************************************************
// Main.
//********************************************************************************************
int MosMain(void)
   { 
   PrintHeader();

   // 1- Off-line merge example.
   OfflineExample();

   // 2- On-line merge example.
   OnlineExample();
   }


//*****************************************************************************************
// Off-line blob merge example.
//*****************************************************************************************
void OfflineExample()
   {
   MosPrintf(MIL_TEXT("\n------------------------------------------------------------------"));
   MosPrintf(MIL_TEXT("\n1- Off-line analysis:\n\n"));
   MosPrintf(MIL_TEXT("Blobs from all vertically adjacent buffers are merged into a single\n"));
   MosPrintf(MIL_TEXT("result buffer; subsequently, a blob analysis is performed.\n\n"));   
   MosPrintf(MIL_TEXT("\nPress <Enter> to start accumulating the blobs.\n\n"));
   MosGetch();

   MIL_ID   MilApplication          = M_NULL,   // Application identifier.
            MilSystem               = M_NULL,   // System identifier.
            MilDigitizer            = M_NULL,   // Digitizer identifier.
            MilDisplay              = M_NULL,   // Display identifier for the grabbed image.
            MilDisplayBlobMerge     = M_NULL,   // Second display identifier for blob merge.
            MilImage                = M_NULL,   // Grabbed image identifier.
            MilBinImage             = M_NULL,   // Binary image buffer identifier.
            MilDispImage            = M_NULL,   // Image to display blob merge.
            MilBlobContext          = M_NULL,   // Blob context identifier.
            MilPreviousBlobResult   = M_NULL,   // Blob result identifier from the previous 
                                                // frame(s).
            MilCurrentBlobResult    = M_NULL,   // Blob result identifier from the current
                                                // frame.
            MilDestinationBlobResult= M_NULL,   // Blob merge destination result identifier.
            MilGraphicList          = M_NULL;   // GraphicList identifier.
           
   MIL_ID   MilGrabBufferList[BUFFERING_SIZE] = { 0 };   // Array containing buffer identifiers
                                                         // of the grabbed images.
   
   HookDataStruct    UserHookData;                       // Hook data of processing functions.
   
   // Allocate application, system and display.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);
  
   // Allocate a display for blob merge.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplayBlobMerge);
   
   // Allocate a simulated digitizer to retrieve images from a sequence file.
   MdigAlloc(MilSystem, M_DEFAULT, SEQUENCE_FILE, M_EMULATED, &MilDigitizer);

   // Inquire image Size X and Size Y.
   MIL_INT SizeX    =   MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   MIL_INT SizeY    =   MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

   //Allocate and select the grabbed image for display
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_DISP, &MilImage);   
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Grab image"));
   MbufClear(MilImage, 0L);

   // Allocate a binary image buffer for blob analysis.
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);   

   // Allocate and display the merge.
   MbufAlloc2d(MilSystem, SizeX, SizeY*BUFFERING_SIZE, 8+M_UNSIGNED, M_IMAGE+M_DISP, 
                  &MilDispImage);
   MdispControl(MilDisplayBlobMerge, M_TITLE, MIL_TEXT("Off-line blob merge"));
   MdispControl(MilDisplayBlobMerge, M_WINDOW_INITIAL_POSITION_X, DISPLAY_X_OFFSET);   

   // Allocate a graphic List to draw the blob merge result.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);

   // Associate the graphic list to the display. 
   MdispControl(MilDisplayBlobMerge, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Allocate a blob context buffer.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);

   // Enable the feature calculations.  
   MblobControl(MilBlobContext, M_BOX, M_ENABLE);
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);
  
   // Allocate blob result buffers for blob merge.
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilPreviousBlobResult);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilCurrentBlobResult);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilDestinationBlobResult);

   // Allocate the grab buffers.
   MIL_INT MilGrabBufferListSize;
   for(MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE; 
         MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_GRAB+M_PROC, 
                     &MilGrabBufferList[MilGrabBufferListSize]);
      }

   // Fill hook function structure with the corresponding variables.
   UserHookData.MilGrabDisp               = MilImage;
   UserHookData.MilBinImage               = MilBinImage;
   UserHookData.MilBlobContext            = MilBlobContext;
   UserHookData.MilPreviousBlobResult     = MilPreviousBlobResult;
   UserHookData.MilCurrentBlobResult      = MilCurrentBlobResult;
   UserHookData.MilDestinationBlobResult  = MilDestinationBlobResult;
   UserHookData.MilDispImage              = MilDispImage;
   UserHookData.MilGraphicList            = MilGraphicList;
   UserHookData.SizeX                     = SizeX;
   UserHookData.SizeY                     = SizeY;
   UserHookData.ImageCount                = 0;

   // Select displays.
   MdispSelect(MilDisplayBlobMerge, MilDispImage);
   MdispSelect(MilDisplay, MilImage);

   // Grabs a specific number of frames, storing them sequentially in a list of buffers. 
   MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE, M_SEQUENCE, M_DEFAULT, 
                  (MIL_DIG_HOOK_FUNCTION_PTR)OfflineMergeHook, &UserHookData);

   // Off-line Blob analysis on the merged blobs.
   OfflineBlobAnalysis(&UserHookData);

   // Print a message and wait for a key press
   MosPrintf(MIL_TEXT("\nPress <Enter> to continue.\n\n"));
   MosGetch();

   // Free all allocations.
   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
   
   MblobFree(MilDestinationBlobResult);
   MblobFree(MilCurrentBlobResult);
   MblobFree(MilPreviousBlobResult);
   MblobFree(MilBlobContext); 
   MgraFree(MilGraphicList);
   MbufFree(MilDispImage);
   MbufFree(MilBinImage);
   MbufFree(MilImage);
   MdigFree(MilDigitizer);
   MdispFree(MilDisplayBlobMerge);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);
}

//*******************************************************************************************
//Processing hook function - off-line blob merge, called every time a grab buffer is ready.
//*******************************************************************************************
void MFTYPE OfflineMergeHook(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   
   MIL_ID BufferId, tempId;
   HookDataStruct * UserHookDataPtr = (HookDataStruct *)HookDataPtr;

   // Retrieve the MIL_ID of the grab buffer. 
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &BufferId);

   // Copy the grab buffer to display. 
   MbufCopy(BufferId, UserHookDataPtr->MilGrabDisp);
   MbufCopyClip(BufferId, UserHookDataPtr->MilDispImage, 0,  UserHookDataPtr->SizeY * 
                  UserHookDataPtr->ImageCount);

   // Binarize using auto threshold, foreground black.
   MimBinarize(BufferId, UserHookDataPtr->MilBinImage, M_BIMODAL+M_LESS, M_NULL, M_NULL);   

   if (UserHookDataPtr->ImageCount > 0) 
      {
      // Calculate blobs on the current grab buffer.
      MosPrintf(MIL_TEXT("The blobs from the current frame are extracted"));
      MblobCalculate(UserHookDataPtr->MilBlobContext, UserHookDataPtr->MilBinImage, M_NULL,
                        UserHookDataPtr->MilCurrentBlobResult);

      // Merge the previous and current blob results into a destination blob result buffer.
      MosPrintf(MIL_TEXT(" and merged\ninto the destination result buffer.\n"));
      MblobMerge(UserHookDataPtr->MilPreviousBlobResult, UserHookDataPtr->MilCurrentBlobResult, 
                     UserHookDataPtr->MilDestinationBlobResult, M_MOVE);
      }
   else
      {
      // Calculate blobs on the first grab buffer.
      MosPrintf(MIL_TEXT("The blobs from the current frame are extracted.\n"));
      MblobCalculate(UserHookDataPtr->MilBlobContext, UserHookDataPtr->MilBinImage, M_NULL,
                        UserHookDataPtr->MilDestinationBlobResult);
      }

   // Draw merged blobs until the current grab buffer.
   MgraClear(M_DEFAULT, UserHookDataPtr->MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MblobDraw(M_DEFAULT, UserHookDataPtr->MilDestinationBlobResult, 
                  UserHookDataPtr->MilGraphicList, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);

   // Draw the line at the merging frontiers.
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   for(MIL_INT j=1; j<=UserHookDataPtr->ImageCount; j++)
      MgraLine(M_DEFAULT, UserHookDataPtr->MilGraphicList, 0,UserHookDataPtr->SizeY*j, 
                  UserHookDataPtr->SizeX-1, UserHookDataPtr->SizeY*j);

   // Swap the previous and merged destination blob result buffers for the next merge.
   tempId =  UserHookDataPtr->MilPreviousBlobResult;
   UserHookDataPtr->MilPreviousBlobResult = UserHookDataPtr->MilDestinationBlobResult;
   UserHookDataPtr->MilDestinationBlobResult = tempId;

   UserHookDataPtr->ImageCount++;
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to load the next frame.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Off-line blob Analysis function after off-line blob merge.
//*****************************************************************************
void OfflineBlobAnalysis(void* HookDataPtr)
   {
   HookDataStruct * UserHookDataPtr = (HookDataStruct *)HookDataPtr;

   MosPrintf(MIL_TEXT("\nAll the blobs have been extracted from all frames.\n"));

   // Removing the blobs touching the images borders. 
   MblobSelect(UserHookDataPtr->MilPreviousBlobResult, M_EXCLUDE,  M_BLOB_TOUCHING_IMAGE_BORDERS, M_NULL, M_NULL,M_NULL);

   // Draw the merged blobs.
   MgraClear(M_DEFAULT, UserHookDataPtr->MilGraphicList);
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MblobDraw(M_DEFAULT, UserHookDataPtr->MilPreviousBlobResult, 
                  UserHookDataPtr->MilGraphicList, M_DRAW_BLOBS, M_INCLUDED_BLOBS, M_DEFAULT);

   // Get the total number of blobs after merge.
   MIL_INT TotalBlobs;
   MblobGetResult(UserHookDataPtr->MilPreviousBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &TotalBlobs);
   MosPrintf(MIL_TEXT("\nThe top and bottom-touching partial blobs are excluded.\n"));
   MosPrintf(MIL_TEXT("Blob features are calculated and displayed.\n"));

   // Draw a cross at the center of gravity of each blob.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MblobDraw(M_DEFAULT, UserHookDataPtr->MilPreviousBlobResult, 
               UserHookDataPtr->MilGraphicList, M_DRAW_CENTER_OF_GRAVITY,
               M_INCLUDED_BLOBS, M_DEFAULT);
   };

//****************************************************************************************
// On-line blob merge example
//****************************************************************************************
 void OnlineExample()
   {
   MosPrintf(MIL_TEXT("\n-----------------------------------------------------------------------"));
   MosPrintf(MIL_TEXT("\n2- On-line analysis:\n\n"));
   MosPrintf(MIL_TEXT("Blobs from the current frame are merged with the bottom-touching partial\n"));
   MosPrintf(MIL_TEXT("blobs from the previous frame. The bottom-touching partial blobs from\n"));
   MosPrintf(MIL_TEXT("the current frame are excluded. Then the merged blobs are analyzed.\n"));
   
   MosPrintf(MIL_TEXT("\nPress <Enter> to start the operation.\n\n"));
   MosGetch();

   MIL_ID   MilApplication          = M_NULL,   // Application identifier.
            MilSystem               = M_NULL,   // System identifier.
            MilDigitizer            = M_NULL,   // Digitizer identifier.
            MilDisplay              = M_NULL,   // Display identifier for the grabbed image.
            MilDisplayBlobMerge     = M_NULL,   // Second display identifier for blob merge.
            MilImage                = M_NULL,   // Grabbed image identifier.
            MilBinImage             = M_NULL,   // Binary image buffer identifier.
            MilDispImage            = M_NULL,   // Image to display blob merge.
            MilBlobContext          = M_NULL,   // Blob context identifier.
            MilPreviousBlobResult   = M_NULL,   // Blob result identifier from the previous 
                                                // frame(s).
            MilCurrentBlobResult    = M_NULL,   // Blob result identifier from the current 
                                                // frame.
            MilDestinationBlobResult= M_NULL,   // Blob merge destination result identifier.
            MilGraphicList          = M_NULL;   // GraphicList identifier.
   
   MIL_ID   MilGrabBufferList[BUFFERING_SIZE] = { 0 };   // Array containing buffer identifiers
                                                         // of the grabbed images.

   HookDataStruct    UserHookData;                       // Hook data of processing functions.

   // Allocate application, system and display.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Allocate a display for blob merge.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplayBlobMerge);

   // Allocate a simulated digitizer to retrieve images from a sequence file.
   MdigAlloc(MilSystem, M_DEFAULT, SEQUENCE_FILE, M_EMULATED, &MilDigitizer);
   
   // Inquire image Size X and Size Y.
   MIL_INT SizeX    =   MdigInquire(MilDigitizer, M_SIZE_X, M_NULL);
   MIL_INT SizeY    =   MdigInquire(MilDigitizer, M_SIZE_Y, M_NULL);

   //Allocate and select the grabbed image for display
   MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_DISP, &MilImage);
   MbufClear(MilImage, 0x0);
   MdispControl(MilDisplay, M_TITLE, MIL_TEXT("Grab image"));
   
   // Allocate a binary image buffer for blob analysis.
   MbufAlloc2d(MilSystem, SizeX, SizeY, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

   // Allocate a blob context buffer.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &MilBlobContext);
   // Enable the feature calculations.  
   MblobControl(MilBlobContext, M_BOX, M_ENABLE);
   MblobControl(MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);

   // Allocate blob result buffers for blob merge.
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilPreviousBlobResult);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilCurrentBlobResult);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &MilDestinationBlobResult);
   
   // Allocate and display the image.
   MbufAlloc2d(MilSystem, SizeX, SizeY*2, 8+M_UNSIGNED, M_IMAGE+M_DISP, &MilDispImage);
   MdispControl(MilDisplayBlobMerge, M_TITLE, MIL_TEXT("On-line blob merge"));
   MdispControl(MilDisplayBlobMerge, M_WINDOW_INITIAL_POSITION_X, DISPLAY_X_OFFSET); 
   MbufClear(MilDispImage, 0);

   // Allocate a graphic list to draw blob merge result.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   // Associate the graphic list to the display. 
   MdispControl(MilDisplayBlobMerge, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);

   // Fill hook function structure with the corresponding variables.
   UserHookData.MilGrabDisp               = MilImage;
   UserHookData.MilBinImage               = MilBinImage;
   UserHookData.MilBlobContext            = MilBlobContext;
   UserHookData.MilPreviousBlobResult     = MilPreviousBlobResult;
   UserHookData.MilCurrentBlobResult      = MilCurrentBlobResult;
   UserHookData.MilDestinationBlobResult  = MilDestinationBlobResult;
   UserHookData.MilDisplayBlobMerge       = MilDisplayBlobMerge;
   UserHookData.MilDispImage              = MilDispImage;
   UserHookData.MilGraphicList            = MilGraphicList;
   UserHookData.SizeX                     = SizeX;
   UserHookData.SizeY                     = SizeY;
   UserHookData.ImageCount                = 0;

   // Allocate the grab buffers.
   MIL_INT MilGrabBufferListSize;
   for(MilGrabBufferListSize = 0; MilGrabBufferListSize < BUFFERING_SIZE; 
         MilGrabBufferListSize++)
      {
      MbufAlloc2d(MilSystem, SizeX, SizeY, 8+M_UNSIGNED, M_IMAGE+M_GRAB+M_PROC, 
                     &MilGrabBufferList[MilGrabBufferListSize]);
      }
   
   // Select displays.
   MdispSelect(MilDisplay, MilImage);
   MdispSelect(MilDisplayBlobMerge, MilDispImage);     

   // Grabs a specific number of frames, storing them sequentially in a list of buffers. 
   MdigProcess(MilDigitizer, MilGrabBufferList, BUFFERING_SIZE, M_SEQUENCE, M_DEFAULT, 
                  (MIL_DIG_HOOK_FUNCTION_PTR)OnlineMergeAndBlobAnalysisHook, &UserHookData);

   // Print a message and wait for a key press
   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n\n"));
   MosGetch();

   // Free all allocations.
   MgraFree(MilGraphicList);
   MbufFree(MilDispImage);
   
   while(MilGrabBufferListSize > 0)
      MbufFree(MilGrabBufferList[--MilGrabBufferListSize]);
   
   MblobFree(MilDestinationBlobResult);
   MblobFree(MilCurrentBlobResult);
   MblobFree(MilPreviousBlobResult);
   MblobFree(MilBlobContext); 
   MbufFree(MilBinImage);
   MbufFree(MilImage);
   MdigFree(MilDigitizer);
   MdispFree(MilDisplayBlobMerge);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);}

//*******************************************************************************************
// Processing hook function - on-line blob merge, called every time a grab buffer is ready.
//*******************************************************************************************
void MFTYPE OnlineMergeAndBlobAnalysisHook(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
   {
   HookDataStruct * UserHookDataPtr = (HookDataStruct *)HookDataPtr;

   MIL_ID BufferId, TempId;

   // Retrieve the MIL_ID of the grab buffer. 
   MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &BufferId);

   // Copy the grab buffer to display. 
   MbufCopy(BufferId, UserHookDataPtr->MilGrabDisp);

   if(UserHookDataPtr->ImageCount > 0)
      {
      // Copy current image to display.
      MbufCopyClip(BufferId, UserHookDataPtr->MilDispImage, 0, UserHookDataPtr->SizeY);
      MdispControl(UserHookDataPtr->MilDisplayBlobMerge, M_UPDATE, M_ENABLE);

      MosPrintf(MIL_TEXT("The blobs from the current frame are extracted.\n"));

      MimBinarize(BufferId, UserHookDataPtr->MilBinImage, M_BIMODAL+M_LESS, M_NULL, M_NULL);
      MblobCalculate(UserHookDataPtr->MilBlobContext, UserHookDataPtr->MilBinImage, M_NULL,
                        UserHookDataPtr->MilCurrentBlobResult);

      MosPrintf(MIL_TEXT("The blobs are merged with the bottom-touching blobs from the")); 
      MosPrintf(MIL_TEXT(" previous frame.\n"));
      
      MblobSelect(UserHookDataPtr->MilPreviousBlobResult, M_INCLUDE_ONLY, M_EXCLUDED_BLOBS, 
                     M_NULL, M_NULL, M_NULL);
      MblobMerge(UserHookDataPtr->MilPreviousBlobResult, UserHookDataPtr->MilCurrentBlobResult, 
                     UserHookDataPtr->MilDestinationBlobResult, M_COPY);

      // Draw the extracted blobs.
      OnlineDrawBlobResults(HookDataPtr, UserHookDataPtr->MilDestinationBlobResult);

      MosPrintf(MIL_TEXT("\nPress <Enter> to load the next frame.\n\n"));
      MosGetch();

      MdispControl(UserHookDataPtr->MilDisplayBlobMerge, M_UPDATE, M_DISABLE);
      MbufCopy(BufferId, UserHookDataPtr->MilDispImage);
      }
   else
      {

      // Copy the first grab frame to display.
      MbufCopy(BufferId, UserHookDataPtr->MilDispImage);
   
      // Binarize then calculate blobs on the current frame.
      MosPrintf(MIL_TEXT("The blobs from the current frame are extracted.\n"));

      MimBinarize(BufferId, UserHookDataPtr->MilBinImage, M_BIMODAL+M_LESS, M_NULL, M_NULL);
      MblobCalculate(UserHookDataPtr->MilBlobContext, UserHookDataPtr->MilBinImage, M_NULL,
                        UserHookDataPtr->MilCurrentBlobResult);

      // Draw the extracted blobs.
      OnlineDrawBlobResults(HookDataPtr, UserHookDataPtr->MilCurrentBlobResult);

      MosPrintf(MIL_TEXT("\nPress <Enter> to load the next frame.\n\n"));
      MosGetch();
      }   

   // Set previous blob result.
   TempId = UserHookDataPtr->MilPreviousBlobResult;
   UserHookDataPtr->MilPreviousBlobResult = UserHookDataPtr->MilCurrentBlobResult;         
   UserHookDataPtr->MilCurrentBlobResult = TempId;

   // Exclude the blobs touching the image borders in the current frame for the next merge.
   MblobSelect(UserHookDataPtr->MilPreviousBlobResult, M_EXCLUDE, M_BLOB_TOUCHING_IMAGE_BORDERS, 
                  M_NULL, M_NULL, M_NULL);

   UserHookDataPtr->ImageCount++;   
   }

void OnlineDrawBlobResults(void* HookDataPtr, MIL_ID BlobResultId)
   {
   HookDataStruct * UserHookDataPtr = (HookDataStruct *)HookDataPtr;

   // Exclude the blobs touching the borders before drawing and retrieving the results.
   MblobSelect(BlobResultId, M_EXCLUDE, M_BLOB_TOUCHING_IMAGE_BORDERS , M_NULL, M_NULL, M_NULL);

   // Draw the merged blobs.
   MgraColor(M_DEFAULT, M_COLOR_BLUE);
   MgraClear(M_DEFAULT, UserHookDataPtr->MilGraphicList);
   MblobDraw(M_DEFAULT, BlobResultId, UserHookDataPtr->MilGraphicList, M_DRAW_BLOBS, 
               M_INCLUDED_BLOBS, M_DEFAULT);

   // Draw center of gravity of each blob.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MblobDraw(M_DEFAULT, BlobResultId, UserHookDataPtr->MilGraphicList, M_DRAW_CENTER_OF_GRAVITY,
               M_INCLUDED_BLOBS, M_DEFAULT);

   // Draw the line at the merging frontier.
   MgraColor(M_DEFAULT, M_COLOR_YELLOW);
   MgraLine(M_DEFAULT, UserHookDataPtr->MilGraphicList, 0, UserHookDataPtr->SizeY, 
               UserHookDataPtr->SizeX-1, UserHookDataPtr->SizeY);

   // On-line blob analysis- Get the total number of blobs right after each merge.
   MIL_INT TotalBlobs;
   MblobGetResult(BlobResultId, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &TotalBlobs);
   MosPrintf(MIL_TEXT("%d blobs have been found and analyzed from acquisition #%d.\n\n"), 
               TotalBlobs, UserHookDataPtr->ImageCount+1);   
   }
