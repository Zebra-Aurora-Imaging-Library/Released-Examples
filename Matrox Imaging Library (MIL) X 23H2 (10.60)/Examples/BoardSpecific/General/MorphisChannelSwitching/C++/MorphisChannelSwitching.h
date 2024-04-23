// MorphisChannelSwitching.h : main header file for the PROJECT_NAME application
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#pragma once

#ifndef __AFXWIN_H__
   #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"      // main symbols


// CMorphisChannelSwitchingApp:
// See MorphisChannelSwitching.cpp for the implementation of this class
//

class CMorphisChannelSwitchingApp : public CWinApp
   {
public:
   CMorphisChannelSwitchingApp();

// Overrides
public:
   virtual BOOL InitInstance();

// Implementation

   DECLARE_MESSAGE_MAP()
   };

extern CMorphisChannelSwitchingApp theApp;
