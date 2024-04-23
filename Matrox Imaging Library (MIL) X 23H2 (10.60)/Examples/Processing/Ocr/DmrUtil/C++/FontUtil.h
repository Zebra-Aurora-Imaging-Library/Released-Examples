/************************************************************************************/
/*
* File name: FontUtil.h
*
* Synopsis:  This file contains utility functions to manage font in dmr context and
*            font files.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#ifndef FONT_UTIL
#define FONT_UTIL

#include <vector>
#include <string>
using namespace std;
typedef basic_string<MIL_TEXT_CHAR> mstring;

namespace FontUtil
   {
   void ExportFont(MIL_ID MilDmrContext, MIL_INT FontIndex);
   void GetCharName(MIL_ID MilDmrContext, MIL_CONST_TEXT_PTR UserCharName,
      MIL_INT ControlFlag, mstring* pCharName);
   void GetCharName(MIL_ID MilDmrContext, MIL_INT CharIndex,
      MIL_INT ControlFlag, mstring* pCharName);
   }

#endif
