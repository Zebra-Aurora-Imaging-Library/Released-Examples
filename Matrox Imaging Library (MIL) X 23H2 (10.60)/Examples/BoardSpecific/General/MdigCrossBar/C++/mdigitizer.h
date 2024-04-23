//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#define MAX_BUFFERING 2


class CDigitizer
   {
   public:

   CDigitizer(MIL_ID iMilSystem, MIL_INT DevNum, const MIL_STRING& DCFName, MIL_INT iIndex, MIL_INT iChannel = M_INVALID, bool iHookCameraPresent = false, MIL_ID iEventCameraPresent = M_NULL)
      {
      Init();
      m_Index           = iIndex;
      m_MilSystem       = iMilSystem;
      m_Channel         = iChannel;
      m_DigDeviceNumber = DevNum;
      m_IsAllocated     = false;
      m_EventCameraPresent = iEventCameraPresent;

      m_DCFName = DCFName;
      AllocateDigitizer(iMilSystem, DevNum, DCFName);

      if(iHookCameraPresent)
         AllocateDigitizerMinimal(iMilSystem, DevNum, DCFName);
      }

   ~CDigitizer()
      {
      Free();
      }

   bool IsAllocated()                  { return m_IsAllocated;}
   bool IsDeviceUsed()                 { return m_IsDeviceUsed;}
   MIL_INT IsCameraPresent()           { return m_CameraPresent;}
   MIL_ID System()                     { return m_MilSystem;}
   MIL_ID Digitizer()                  { return m_MilDigitizer;}
   MIL_ID MinimalDigitizer()           { return m_MilDitizerMinimal;}
   MIL_INT DeviceNumber()              { return m_DigDeviceNumber;}
   MIL_ID GetEvent()                   { return m_EventCameraPresent; }

   const MIL_STRING& GetDcfName()         { return m_DCFName;}
   const MIL_STRING& GetDetectedDcfName() { return m_DetectDCFName;}

   void  SetUserData(void *ipPrivateData) { m_PrivateData = ipPrivateData;}
   void *GetUserData()                 { return m_PrivateData;}

   MIL_INT Index()                     {return m_Index;}
   MIL_INT SizeX()                     {return m_SizeX;}
   MIL_INT SizeY()                     {return m_SizeY;}

   void Start(MIL_BUF_HOOK_FUNCTION_PTR UserProcessingFunctionPtr);
   void Stop();
   MIL_INT PrepareForGrabbing();
   MIL_INT StopGrabbing();

   void PrintDigitizerInfo();

   void    GrabDecimation(MIL_INT Decimation) { m_Decimation = Decimation;}
   MIL_INT GrabDecimation() {return m_Decimation;}

   void    MovingLine(bool  MovingLine) { m_EnableMovingLine = MovingLine;}
   bool    MovingLine() {return m_EnableMovingLine;}

   void GetStatistics(MIL_INT *FrameCount, MIL_DOUBLE *FrameRate, MIL_INT *FrameMissed);
   MIL_ID GetWorkBuffer() { return m_MilImageWork[m_GrabCount % MAX_BUFFERING]; }

   void SetSourceId(MIL_INT64 SourceId) { m_SourceId = SourceId;}
   MIL_INT64 GetSourceId() { return m_SourceId; }

   void FreeDigitizer();
   void UpdateDetectedDcfName();


   static MIL_INT MFTYPE DigProcessingFunction(MIL_INT HookType, MIL_ID HookId, void* HookDataPtr)
      {
      MIL_ID ModifiedBufferId;
      CDigitizer *pDig = (CDigitizer *)HookDataPtr;
      const long NB_LINE_RECT = 7;

      pDig->m_GrabCount++;
      if(pDig->m_Decimation && (pDig->m_GrabCount % pDig->m_Decimation != 0))
         return 0;

      MdigGetHookInfo(HookId, M_MODIFIED_BUFFER+M_BUFFER_ID, &ModifiedBufferId);

      MIL_STRING_STREAM Buf;
      Buf << MIL_TEXT("Dig:") << pDig->Index();
      MgraText(M_DEFAULT, ModifiedBufferId, 10, 30 , Buf.str());

      if(pDig->m_EnableMovingLine)
         {
         MgraRectFill(M_DEFAULT, ModifiedBufferId, pDig->m_GraLinePos, 2, pDig->m_GraLinePos + NB_LINE_RECT, pDig->SizeX()-NB_LINE_RECT);
         if(pDig->m_GraLinePos > pDig->SizeX() - NB_LINE_RECT)
            pDig->m_GraLineDir = -6;
         else if (pDig->m_GraLinePos < 12)
            pDig->m_GraLineDir = 6;

         pDig->m_GraLinePos += pDig->m_GraLineDir;
         }

      pDig->m_ProcessingFunctionPtr(HookType, HookId, HookDataPtr);
      return 0;
      }

   protected:

   void Init(bool iAll = true);
   bool AllocateDigitizer(MIL_ID iMilSystem, MIL_INT DevNum, const MIL_STRING& DCFName);
   bool AllocateDigitizerMinimal(MIL_ID iMilSystem, MIL_INT DevNum, const MIL_STRING& DCFName);
   bool AllocateBuffers(MIL_INT iIndex);
   void Free( bool iAll = true);

   static MIL_INT MFTYPE DigHookCameraPresent(MIL_INT hookType, MIL_ID eventId, void *userData);

   bool         m_IsAllocated;
   bool         m_IsDeviceUsed;
   MIL_INT      m_CameraPresent;
   MIL_ID       m_MilSystem;
   MIL_ID       m_MilDigitizer;
   MIL_ID       m_MilDitizerMinimal;
   MIL_ID       m_MilBuffers[MAX_BUFFERING];
   MIL_ID       m_MilImageWork[MAX_BUFFERING];
   MIL_INT      m_GrabCount;
   MIL_INT      m_Index;
   MIL_INT      m_Decimation;
   MIL_INT64    m_SourceId;

   void         *m_PrivateData;
   MIL_BUF_HOOK_FUNCTION_PTR m_ProcessingFunctionPtr;

   MIL_INT      m_SizeX;
   MIL_INT      m_SizeY;

   bool    m_EnableMovingLine;
   MIL_INT m_GraLinePos;
   MIL_INT m_GraLineDir;

   MIL_INT      m_DigDeviceNumber;
   MIL_STRING    m_DCFName;
   MIL_STRING    m_DetectDCFName;
   MIL_INT      m_Channel;
   MIL_ID       m_EventCameraPresent;

   MIL_INT m_LatencyFromDispIndex;
   bool m_GrabHalfFrameRate;

   };
