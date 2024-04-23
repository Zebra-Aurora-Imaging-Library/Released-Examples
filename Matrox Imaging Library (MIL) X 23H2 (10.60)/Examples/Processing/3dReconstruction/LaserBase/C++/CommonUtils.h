//*******************************************************************************
//
// File name: CommonUtils.h
//
// Synopsis:  File with common utility functions.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef COMMONUTILS_H
#define COMMONUTILS_H

template <typename T> T Min(T a, T b) { return ((a <= b) ? a : b); }
template <typename T> T Max(T a, T b) { return ((a >= b) ? a : b); }

bool CheckForRequiredMILFile(MIL_CONST_TEXT_PTR FileName);
bool UserPressedEscape();
bool UserPressedEnter();
void PrintGrabProgress(MIL_INT Num, MIL_INT Div);

//****************************************************************************
// CheckForRequiredMILFile. Check for required files to run the example.    
//****************************************************************************
inline bool CheckForRequiredMILFile(const MIL_STRING& FileName)
   {
   MIL_INT FilePresent = M_NO;

   MappFileOperation(M_DEFAULT, FileName, M_NULL, M_NULL, M_FILE_EXISTS, M_DEFAULT, &FilePresent);
   if(FilePresent == M_NO)
      {
      MosPrintf(MIL_TEXT("The files needed to run this example are missing. You need \n")
                MIL_TEXT("to obtain and apply a separate specific update to have it.\n\n"));
      }

   return (FilePresent == M_YES);
   }

#endif
