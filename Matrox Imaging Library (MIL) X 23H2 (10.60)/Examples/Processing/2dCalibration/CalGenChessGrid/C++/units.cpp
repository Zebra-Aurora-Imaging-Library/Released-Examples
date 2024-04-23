//***************************************************************************************
// 
// File name: units.cpp
//
// Synopsis:  This file implements functions related to units display and conversion.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "common.h"

//*****************************************************************************
// Converts the Units enum to a short string.
//*****************************************************************************
MIL_CONST_TEXT_PTR GetUnitName(UnitEnum Units)
   {
   switch (Units)
      {
      case Kilometers : return MIL_TEXT("km");
      case Meters     : return MIL_TEXT("m");
      case Centimeters: return MIL_TEXT("cm");
      case Millimeters: return MIL_TEXT("mm");
      case Micrometers: return MIL_TEXT("um");
      case Miles      : return MIL_TEXT("miles");
      case Feet       : return MIL_TEXT("ft");
      case Inches     : return MIL_TEXT("in");
      case Mils       : return MIL_TEXT("mils");

      default: throw MIL_TEXT("Unknown unit");
      }
   }

//*****************************************************************************
// Converts 1 world unit to inches. Useful for DPI-related computations.
//*****************************************************************************
MIL_DOUBLE GetInchesPerWorldUnit(UnitEnum Units)
   {
   const MIL_DOUBLE INCHES_PER_METER = 1000.0 / 25.4;

   switch (Units)
      {
      case Kilometers : return INCHES_PER_METER * 1000.0;
      case Meters     : return INCHES_PER_METER;
      case Centimeters: return INCHES_PER_METER / 100.0;
      case Millimeters: return INCHES_PER_METER / 1000.0;
      case Micrometers: return INCHES_PER_METER / 1000000.0;
      case Miles      : return 1760.0 * 3.0 * 12.0;
      case Feet       : return 12.0;
      case Inches     : return 1.0;
      case Mils       : return 1.0 / 1000.0;
      
      default: throw MIL_TEXT("Unknown unit");
      }
   }
