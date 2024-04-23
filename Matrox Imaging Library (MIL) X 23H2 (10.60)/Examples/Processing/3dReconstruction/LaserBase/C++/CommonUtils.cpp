//*******************************************************************************
// 
// File name: CommonUtils.cpp  
//
// Synopsis:  Utility functions common to all classes.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************
#include "BaseCommon.h"

//*******************************************************************************
bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName)
   {
   MIL_INT FilePresent;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if (FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The footage needed to run this example is missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));

      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));
      MosGetch();
      }

   return (FilePresent == M_YES);
   }

//*******************************************************************************
bool UserPressedEscape()
   {
   static MIL_INT ESC_CHAR_KEY = 27;
   if(MosKbhit())
      {
      MIL_INT ch = MosGetch();
      return (ch == ESC_CHAR_KEY);
      }
   return false;
   }

//*******************************************************************************
bool UserPressedEnter()
   {
   if(MosKbhit())
      {
      MIL_INT ch = MosGetch();
      return ((ch == MIL_TEXT('\r')) || (ch == MIL_TEXT('\n')));
      }
   return false;
   }

//*******************************************************************************
void PrintGrabProgress(MIL_INT Num, MIL_INT Div)
   {
   MIL_DOUBLE p = ((MIL_DOUBLE) Num / (MIL_DOUBLE) (Div-1)) * 100.0;
   MosPrintf(MIL_TEXT("Acquisition: %3d %%\r"), (int) (p + 0.5));
   }
