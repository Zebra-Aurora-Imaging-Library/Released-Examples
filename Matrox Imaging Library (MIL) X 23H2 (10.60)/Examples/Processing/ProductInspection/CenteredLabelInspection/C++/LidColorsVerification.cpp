//***************************************************************************************
// 
// File name: LidColorsVerification.cpp
//
// Synopsis: This file contains the declaration of the CLidColorsVerification class
//           which is the inspection task used to verify the colors of the lid.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include "LidColorsVerification.h"

//*****************************************************************************
// Constructor.
//*****************************************************************************
CLidColorsVerification::CLidColorsVerification(MIL_CONST_TEXT_PTR ColContextPath, 
                                               const MIL_INT* ExpectedMatches, 
                                               MIL_INT ColorConversion /* = M_NONE */, 
                                               CInspectionTask* FixtureProvider /* = M_NULL */, 
                                               CInspectionTask* ImageProvider /* = M_NULL */, 
                                               CInspectionTask* RegionProvider /* = M_NULL */)
 : CColorMatchTask(ColContextPath, ExpectedMatches, ColorConversion, FixtureProvider, ImageProvider, RegionProvider)
   {
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CLidColorsVerification::~CLidColorsVerification()
   {
   }

//*****************************************************************************
// Get the best match label
//*****************************************************************************
MIL_CONST_TEXT_PTR CLidColorsVerification::GetBestMatchLabel(MIL_INT MatchIdx) const
   {
   if(MatchIdx == -1)
      return MIL_TEXT("Color Fail");
   else
      return MIL_TEXT("Color Pass");
   }
