// MorphisChannelSwitchingDlg.h : header file
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

const MIL_INT CAMERA_PRESENT_TIME_IN_MS =  50;
const MIL_INT MAX_DIGITIZERS = 4;

const MIL_INT Channel[] = { M_CH0, M_CH1, M_CH2,  M_CH3,
                            M_CH4, M_CH5, M_CH6,  M_CH7,
                            M_CH8, M_CH9, M_CH10, M_CH11,
                            M_CH12, M_CH13, M_CH14, M_CH15};
const MIL_INT Device[] =   {M_DEV0, M_DEV1, M_DEV2, M_DEV3};

// Structure containing information on the digitizer settings.
typedef struct _DIG_DEVICE_INFO
   {
   _DIG_DEVICE_INFO()
      {
      Init();
      }

   void Init()
      {
      memset(this, 0, sizeof(_DIG_DEVICE_INFO));
      }
   void * pMorphisChannelSwitchingDlg;

   MIL_ID MilSystem;
   MIL_ID MilDigitizer;
   MIL_INT DeviceNumber;

   // Statistics:
   MIL_INT NbrFramesGrabbed;
   double AvgFrameRateCurrent;
   double AvgFrameRate;

   } DIG_DEVICE_INFO;

// Structure containing information on the channel settings.
typedef struct _CHANNEL_INFO
   {
   _CHANNEL_INFO()
      {
      Init();
      }

   void Init()
      {
      IsEnabled = true;
      pDigInfo = NULL;
      Index = 0;
      MilChannel = NULL;
      MilGrabBuffer = M_NULL;
      UseAutomaticInputGain = true;

      NbrFramesGrabbed = 0;
      AvgFrameRate = 0;
      GrabStartMode = M_FIELD_START;
      LockSensitivity = 60;
      GainLuma = 20;
      Contrast = 128;
      Brightness = 128;
      }

   DIG_DEVICE_INFO *pDigInfo;
   bool       IsEnabled;
   MIL_INT    Index;
   MIL_INT    MilChannel;
   MIL_ID     MilImageDisp;
   MIL_ID     MilGrabBuffer;

   CString   OverlayText;
   MIL_INT   LockSensitivity;
   bool      UseAutomaticInputGain;
   MIL_INT   GainLuma;
   MIL_INT   Contrast;
   MIL_INT   Brightness;
   MIL_INT   GrabStartMode;

   // Statistics:
   MIL_INT   NbrFramesGrabbed;
   double    AvgFrameRate;

   } CHANNEL_INFO;

// CMorphisChannelSwitchingDlg dialog
class CMorphisChannelSwitchingDlg : public CDialog
   {
   // Construction
   public:
   CMorphisChannelSwitchingDlg(CWnd* pParent = NULL);   // standard constructor
   ~CMorphisChannelSwitchingDlg();
   // Dialog Data
   enum { IDD = IDD_MORPHISCHANNELSWITCHING_DIALOG };

   protected:
   virtual void DoDataExchange(CDataExchange* pDX);   // DDX/DDV support


   // Implementation
   protected:
   HICON m_hIcon;

   // MIL variables.
   MIL_ID m_MilApplication;                  // Application identifier.
   MIL_ID m_MilSystem;                       // System identifier.
   MIL_ID m_MilDisplay;                      // Display identifier.
   MIL_ID m_MilImageDisp;                    // Image buffer identifier.
   MIL_ID m_MilImageDispChild[16];           // Display childs.
   HANDLE m_ThreadHandle[MAX_DIGITIZERS];    // Thread handle of each channel switching thread.
   MIL_INT           m_Exit;                 // Set to true to stop channel switching threads.
   CHANNEL_INFO      m_ChannelInfo[16];      // Contains channel settings.
   DIG_DEVICE_INFO   m_DigInfo[MAX_DIGITIZERS]; // Contains digitizer settings.
   MIL_INT m_NumberOfChannels;               // Maximum number of cameras selected.
   MIL_INT m_SelectedChannel;                // Currently selected camera. -1 == all channels.
   MIL_INT m_GrabFieldNum;                   // Selects field or frame acquisition mode.
   CRITICAL_SECTION  Lock;

   // Dialog text.
   CStatic LockSensitivity;
   CStatic GainLuma;
   CStatic Contrast;
   CStatic Brightness;
   CButton SetAllChannels;
   CEdit NumberOfChannels;

   // Dialog slider controls.
   CSliderCtrl SliderLockSensitivity;
   CSliderCtrl SliderGainLuma;
   CSliderCtrl SliderContrast;
   CSliderCtrl SliderBrightness;
   CComboBox   ComboDigitizers;

   void UpdateChannelSettings();
   void UpdateChannelSettings(MIL_INT Channel);
   void UpdateDisplayChannelSettings(MIL_INT Channel);

   // Generated message map functions
   virtual BOOL OnInitDialog();
   virtual void OnCancel();
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();
   DECLARE_MESSAGE_MAP()

   public:

   void AllocateMILSystem();
   void FreeMILSystem();

   afx_msg void OnNMReleasedcaptureSliderLockSensitivity(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMReleasedcaptureSliderGainLuma(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnNMReleasedcaptureSliderBrightness(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedRadio();
   afx_msg void OnBnClickedCheckSetAllChannels();
   afx_msg void OnBnClickedButtonStart();
   afx_msg void OnBnClickedButtonStop();
   afx_msg void OnCbnSelchangeComboDigToUse();
   afx_msg void OnBnClickedRadioField();
   afx_msg void OnBnClickedCheckAutoGain();
   afx_msg void OnBnClickedChannelEnabled();
   afx_msg void OnEnChangeEditOverlay();
   afx_msg void OnTimer(UINT_PTR nIDEvent);

   // Channel switching thread.
   static unsigned int ChannelSwitchingThread(void *TParam);
   };
