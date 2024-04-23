/**********************************************************************************/
/*
 *  File name: DMILObjectTrackingSlave.cpp
 *
 *  Synopsis:  This Slave DMIL function is called by the master function to do autonomous
 *             grab and processing and runs directly on target system. The processing loop 
 *             does object tracking using pattern matching.
 *            
 *    Note :   This example don't run as is under MIL lite because it uses high level modules
 *             to find the model position. The code can however be used as good example of 
 *             event based data exchange between the slave and the master of a DMIL cluster. 
 *
 *             The master function can be found in the DMILObjectTracking project.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved 
 */

/* MIL Header. */
#include <mil.h> 

/* Specify which pattern matching type to use (choose one). */
#define GEOMETRIC        0
#define GRAYSCALE        1
#define PATTERN_MATCHING_METHOD GRAYSCALE  

/* Target model specifications. */
#define MODEL_WIDTH           128L
#define MODEL_HEIGHT          128L
#define MODEL_POS_X_INIT      320L
#define MODEL_POS_Y_INIT      240L
#define MODEL_MIN_MATCH_SCORE 50.0

/* Number of grab images for Mdigprocess(). */
#define NB_TARGET_IMAGES      4


/* Processing functions prototypes. */
#ifdef __cplusplus
extern "C" {
#endif

void MFTYPE SlavePatternMatchingLoop(MIL_ID Func);
#ifdef __cplusplus
   }
#endif

/* Data Exchange structure between Host and Target processors. */
typedef struct
   {
   MIL_ID MilSystem;
   MIL_ID MilDisplay;
   MIL_ID MilDigitizer;
   MIL_ID MilImageDisp;
   MIL_ID MilImage[NB_TARGET_IMAGES]; 
   MIL_ID MilThread;
   MIL_ID MilDataExchangeBuffer; 
   MIL_ID MilDataExchangeBufferReadyEvent; 
   MIL_ID MilModelPositionOkEvent;
   MIL_ID MilStopProcessingEvent;
   MIL_ID MilModelContext;
   MIL_ID MilResult;   
   MIL_INT32 PatternMatchingMethod;
   MIL_INT32 DisplayUpdateFlag;
   MIL_INT32 Found;
   MIL_DOUBLE PosX;
   MIL_DOUBLE PosY;
   MIL_DOUBLE Score;
   MIL_DOUBLE Time;
   MIL_INT32 NbFindDone;
   MIL_INT32 Error;
   } DataExchangeStruct;

/* Error codes */
#define PATTERN_MATCHING_LOOP_ERROR_CODE 2

/* MdigProcess() hook function prototypes. */
MIL_INT MFTYPE ModelPositionningHook(MIL_INT HookType, MIL_ID EventId, void* DataExPtr);
MIL_INT MFTYPE GeometricPatternMatchingHook(MIL_INT HookType, MIL_ID EventId, void* DataExPtr);
MIL_INT MFTYPE GrayscalePatternMatchingHook(MIL_INT HookType, MIL_ID EventId, void* DataExPtr);

/******************************************************************************
 *  Slave function: 
 *      - This slave function does grab and processing autonomously and signals
 *        the controlling computer when results are available. 
 *        This prevents the controlling computer from having to send each MIL command 
 *        individually, reducing the inter-computer overhead, and it permits to return
 *        all the results at the same time.
 */

void MFTYPE SlavePatternMatchingLoop(MIL_ID Func)
{
  /* Variable declarations */
  MIL_ID MilDataExchangeBuffer;           
  long   n;               
  DataExchangeStruct DataEx;
  long  *ErrorReturnValuePtr = M_NULL;

  /* Read the parameter. */
  MfuncParamValue(Func, 1, &MilDataExchangeBuffer);

  /* Read the Data Exchange structure. */
  MbufGet(MilDataExchangeBuffer, &DataEx);
   
  /* Allocate multiple buffers for MdigProcess(). */
  for( n=0; n<NB_TARGET_IMAGES; n++)
    {
    MbufAlloc2d(DataEx.MilSystem,  
                MdigInquire(DataEx.MilDigitizer, M_SIZE_X, M_NULL),
                MdigInquire(DataEx.MilDigitizer, M_SIZE_Y, M_NULL),
                8L+M_UNSIGNED, 
                M_IMAGE+M_GRAB+M_PROC, &DataEx.MilImage[n]);
    }

   /* ------------------- */
   /* MODEL POSITIONNING. */

   /* Start the model positionning sequence. */
   DataEx.NbFindDone = 0;
   MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
               M_START, M_ASYNCHRONOUS, ModelPositionningHook, &DataEx);

   /* Wait until the Model Position Ok Event is set by the Host. */
   MthrWait(DataEx.MilModelPositionOkEvent, M_EVENT_WAIT, M_NULL);

   /* Stop the model positionning sequence. */
   MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
               M_STOP, M_SYNCHRONOUS, ModelPositionningHook, &DataEx);

   /* Grab a last reference image without annotations for the model definition. */
   MdigGrab(DataEx.MilDigitizer, DataEx.MilImage[0]);

#if (!M_MIL_LITE)
  /* --------------------------- */
  /* GEOMETRIC PATTERN MATCHING. */

  if (DataEx.PatternMatchingMethod == GEOMETRIC)
     {
     /* Allocate a geometric model finder context to hold the model. */
     MmodAlloc(DataEx.MilSystem, M_GEOMETRIC, M_DEFAULT, &DataEx.MilModelContext);

     /* Define a geometric model from the last grabbed image. */
     MmodDefine(DataEx.MilModelContext, M_IMAGE, DataEx.MilImage[0],
                (long)(MODEL_POS_X_INIT+0.5) - (MODEL_WIDTH/2)  - 1,
                (long)(MODEL_POS_Y_INIT+0.5) - (MODEL_HEIGHT/2) - 1,
                MODEL_WIDTH, MODEL_HEIGHT);

     /* Allocate a result buffer. */
     MmodAllocResult(DataEx.MilSystem, M_DEFAULT, &DataEx.MilResult);

     /* If no allocation error, set the model search parameters, 
        preprocess the model and start the processing.
     */
     if((DataEx.MilModelContext != M_NULL) && (DataEx.MilResult != M_NULL))
        {
        MmodControl(DataEx.MilModelContext, M_CONTEXT, M_NUMBER, 1);
        MmodControl(DataEx.MilModelContext, M_CONTEXT, M_SPEED, M_VERY_HIGH);
        MmodControl(DataEx.MilModelContext, M_CONTEXT, M_ACCURACY, M_MEDIUM);
        MmodControl(DataEx.MilModelContext, M_ALL, M_ACCEPTANCE, MODEL_MIN_MATCH_SCORE);
        MmodPreprocess(DataEx.MilModelContext, M_DEFAULT);
        DataEx.Error      = M_FALSE;
        DataEx.NbFindDone = 0;
     
        /* Start the Geometric pattern matching sequence. */
        MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
                    M_START, M_ASYNCHRONOUS, GeometricPatternMatchingHook, &DataEx);

        /* Here this thread is free to do any other task. The MdigProcess() function 
           is running autonomously and passing the necessary results to the Host via  
           the data exchange buffer.
        */
 
        /* Wait until the Stop Processing Event is set by the Host. */
        MthrWait(DataEx.MilStopProcessingEvent, M_EVENT_WAIT, M_NULL);

        /* Stop the pattern matching sequence. */
        MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
                    M_STOP, M_SYNCHRONOUS, GeometricPatternMatchingHook, &DataEx);
        }
     else
        {
        /* Report a MIL error. */
        MfuncErrorReport(Func, M_FUNC_ERROR+PATTERN_MATCHING_LOOP_ERROR_CODE, 
                         MIL_TEXT("Error during target processing loop allocations."),
                         M_NULL, M_NULL, M_NULL
                        );
        DataEx.Error = M_TRUE;
        }

     /* Free the model finder objects */
     if (DataEx.MilResult != M_NULL) MmodFree(DataEx.MilResult);
     if(DataEx.MilModelContext != M_NULL) MmodFree(DataEx.MilModelContext);
     }

  /* --------------------------- */
  /* GRAYSCALE PATTERN MATCHING. */

  else
     {
     MpatAlloc(DataEx.MilSystem, M_NORMALIZED, M_DEFAULT, &DataEx.MilModelContext);
     MpatDefine(DataEx.MilModelContext, M_REGULAR_MODEL, DataEx.MilImage[0],
                (MODEL_POS_X_INIT + 0.5) - (MODEL_WIDTH/2) - 1,
                (MODEL_POS_Y_INIT + 0.5) - (MODEL_HEIGHT/2) - 1,
                MODEL_WIDTH, MODEL_HEIGHT, M_DEFAULT);

    
     /* Allocate result. */
     MpatAllocResult(DataEx.MilSystem, M_DEFAULT, &DataEx.MilResult);
    
     /* If no allocation error, set the model search parameters, 
        preprocess the Target model and set the error state to false.
     */
     if((DataEx.MilModelContext != M_NULL) && (DataEx.MilResult != M_NULL))
        {
        MpatControl(DataEx.MilModelContext, 0, M_ACCEPTANCE, MODEL_MIN_MATCH_SCORE);
        MpatControl(DataEx.MilModelContext, 0, M_SPEED, M_HIGH);
        MpatControl(DataEx.MilModelContext, 0, M_ACCURACY, M_LOW);
        MpatPreprocess(DataEx.MilModelContext, M_DEFAULT, DataEx.MilImage[0]);
        DataEx.Error      = M_FALSE;
        DataEx.NbFindDone = 0;
     
        /* Start the Geometric pattern matching sequence. */
        MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
                    M_START, M_ASYNCHRONOUS, GrayscalePatternMatchingHook, &DataEx);

        /* Here this thread is free to do any other task. The MdigProcess() function 
           is running autonomously and passing the necessary results to the Host via  
           the data exchange buffer.
        */
 
        /* Wait until the Stop Processing Event is set by the Host. */
        MthrWait(DataEx.MilStopProcessingEvent, M_EVENT_WAIT, M_NULL);

        /* Stop the pattern matching sequence. */
        MdigProcess(DataEx.MilDigitizer, DataEx.MilImage, NB_TARGET_IMAGES, 
                    M_STOP, M_SYNCHRONOUS, GrayscalePatternMatchingHook, &DataEx);
        }
     else
        {
        /* Report a MIL error. */
        MfuncErrorReport(Func, M_FUNC_ERROR+PATTERN_MATCHING_LOOP_ERROR_CODE, 
                         MIL_TEXT("Error during target processing loop model allocations."),
                                                            M_NULL, M_NULL, M_NULL);
        DataEx.Error = M_TRUE;
        }
    
     /* Free pattern matching objects */
     if (DataEx.MilResult != M_NULL) MpatFree(DataEx.MilResult);
     if(DataEx.MilModelContext != M_NULL) MpatFree(DataEx.MilModelContext);
     }
#endif

  /* Free the grab buffers */
  for( n=0; n<NB_TARGET_IMAGES; n++)
    {
    MbufFree(DataEx.MilImage[n]);
    }

}

/******************************************************************************
 *  Model positionning hook function: 
 *      - This hook function is called locally every time MdigProcess does a grab
 *        to draw the position that will be used for the model definition.
 */

MIL_INT MFTYPE ModelPositionningHook(MIL_INT HookType, MIL_ID EventId, void* DataExVoidPtr) 
{
    DataExchangeStruct *DataExPtr = (DataExchangeStruct *)DataExVoidPtr;
    MIL_ID GrabBufferId;

    /* Retrieve the MIL_ID of the grabbed buffer. */
    MdigGetHookInfo(EventId, M_MODIFIED_BUFFER+M_BUFFER_ID, &GrabBufferId);

    /* Draw a rectangle in the Overlay around the position of the model to define. */
    MgraRect(M_DEFAULT, GrabBufferId,
              (long)(MODEL_POS_X_INIT+0.5) - (MODEL_WIDTH/2)  - 2,
              (long)(MODEL_POS_Y_INIT+0.5) - (MODEL_HEIGHT/2) - 2,
              (long)(MODEL_POS_X_INIT+0.5) + (MODEL_WIDTH/2)  + 1,
              (long)(MODEL_POS_Y_INIT+0.5) + (MODEL_HEIGHT/2) + 1);

    /* Copy the image to the display. */
    MbufCopy(GrabBufferId, DataExPtr->MilImageDisp);

    /* Increment find operation count. */
    DataExPtr->NbFindDone++;


    return (M_NULL);
}


/******************************************************************************
 *  Geometric pattern matching hook function: 
 *      - This hook function is called locally every time MdigProcess does a grab.
 *        This avoid to the Host computer to send each processing command individually, 
 *        reducing the inter-computer overhead. It also signals to the host when all the
 *        results are available.
 */
#if (!M_MIL_LITE)
MIL_INT MFTYPE GeometricPatternMatchingHook(MIL_INT HookType, MIL_ID EventId, void* DataExVoidPtr) 
{
    DataExchangeStruct *DataExPtr = (DataExchangeStruct *)DataExVoidPtr;
    MIL_ID GrabBufferId;

    /* Retrieve the MIL_ID of the grabbed buffer. */
    MdigGetHookInfo(EventId, M_MODIFIED_BUFFER+M_BUFFER_ID, &GrabBufferId);

    /* Synchronize and start the timer. */
    if (DataExPtr->NbFindDone == 0)
       MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);

    /* Find model. */
    MmodFind(DataExPtr->MilModelContext, GrabBufferId, DataExPtr->MilResult);

    /* Increment find operation count. */
    DataExPtr->NbFindDone++;

    /* Copy the results in the result structure to be read by the Local system. */
    MmodGetResult(DataExPtr->MilResult, M_DEFAULT, M_NUMBER+M_TYPE_MIL_INT32, &DataExPtr->Found);
    MmodGetResult(DataExPtr->MilResult, M_DEFAULT, M_POSITION_X, &DataExPtr->PosX);
    MmodGetResult(DataExPtr->MilResult, M_DEFAULT, M_POSITION_Y, &DataExPtr->PosY);
    MmodGetResult(DataExPtr->MilResult, M_DEFAULT, M_SCORE,      &DataExPtr->Score);

    /* If required, copy the processed image with the model position drawn to the display. */
    if (DataExPtr->DisplayUpdateFlag)
       {
       if (DataExPtr->Found)
          MmodDraw(M_DEFAULT, DataExPtr->MilResult, GrabBufferId,
                   M_DRAW_POSITION+M_DRAW_BOX, M_DEFAULT, M_DEFAULT);
       else
          MgraText(M_DEFAULT, GrabBufferId, 
                   MODEL_POS_X_INIT, MODEL_POS_Y_INIT, MIL_TEXT("  MODEL NOT FOUND ?  "));
       MbufCopy(GrabBufferId, DataExPtr->MilImageDisp);
       }
         
    /* Read the elapsed time. */
    MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &DataExPtr->Time);

    /* Wait for previous results to be read (event set).*/
    MthrWait(DataExPtr->MilDataExchangeBufferReadyEvent, M_EVENT_WAIT, M_NULL);

    /* Write the new results (This also triggers the Host MbufHookFunction() callback. */
    MbufPut(DataExPtr->MilDataExchangeBuffer, DataExPtr);

    return (M_NULL);
}
#endif

/******************************************************************************
 *  Grayscale pattern matching hook function: 
 *      - This hook function is called locally every time MdigProcess does a grab.
 *        This avoid to the Host computer to send each processing command individually, 
 *        reducing the inter-computer overhead. It also signals to the host when all the
 *        results are available.
 */
#if (!M_MIL_LITE)
MIL_INT MFTYPE GrayscalePatternMatchingHook(MIL_INT HookType, MIL_ID EventId, void* DataExVoidPtr) 
{
    DataExchangeStruct *DataExPtr = (DataExchangeStruct *)DataExVoidPtr;
    MIL_ID GrabBufferId;
    MIL_DOUBLE NbFound;

    /* Retrieve the MIL_ID of the grabbed buffer. */
    MdigGetHookInfo(EventId, M_MODIFIED_BUFFER+M_BUFFER_ID, &GrabBufferId);

    /* Synchronize and start the timer. */
    if (DataExPtr->NbFindDone == 0)
       MappTimer(M_DEFAULT, M_TIMER_RESET+M_SYNCHRONOUS, M_NULL);
         
    /* Find model. */
    MpatFind(DataExPtr->MilModelContext, GrabBufferId, DataExPtr->MilResult);
    
    /* Increment find operation count. */
    DataExPtr->NbFindDone++;
    
    /* Get the results. */
    
    MpatGetResult(DataExPtr->MilResult, M_GENERAL, M_NUMBER, &NbFound);
    DataExPtr->Found = (MIL_INT32)NbFound;
    MpatGetResult(DataExPtr->MilResult, M_DEFAULT, M_POSITION_X, &DataExPtr->PosX);
    MpatGetResult(DataExPtr->MilResult, M_DEFAULT, M_POSITION_Y, &DataExPtr->PosY);
    MpatGetResult(DataExPtr->MilResult, M_DEFAULT, M_SCORE, &DataExPtr->Score);
    
    /* If required, update the display with the processed image and the 
       model position drawn in it. 
     */
    if (DataExPtr->DisplayUpdateFlag)
       {
       if (DataExPtr->Found)
          MpatDraw(M_DEFAULT, DataExPtr->MilResult, GrabBufferId, 
                              M_DRAW_BOX+M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);
       else
          MgraText(M_DEFAULT, GrabBufferId, 
                   MODEL_POS_X_INIT, MODEL_POS_Y_INIT, MIL_TEXT("  MODEL NOT FOUND ?  "));
       MbufCopy(GrabBufferId, DataExPtr->MilImageDisp);
       }  
    
    /* Read the elapsed time. */
    MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &DataExPtr->Time);

    /* Wait for previous results to be read (event set).*/
    MthrWait(DataExPtr->MilDataExchangeBufferReadyEvent, M_EVENT_WAIT, M_NULL);

    /* Write the new results (This also triggers the Host MbufHookFunction() callback. */
    MbufPut(DataExPtr->MilDataExchangeBuffer, DataExPtr);

    return (M_NULL);
}
#endif
