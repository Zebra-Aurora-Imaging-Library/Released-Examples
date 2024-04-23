//***************************************************************************************
// 
// File name: units.h 
//
// Synopsis:  This file declares functions related to units display and conversion.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef UNITS_H
#define UNITS_H

//*****************************************************************************
// Supported world units. The enum numerical values are important: they match
// the digit encoded in the fiducials.
//*****************************************************************************
enum UnitEnum
   {
   Kilometers = 0,
   Meters,
   Centimeters,
   Millimeters,
   Micrometers,
   Miles,
   Feet,
   Inches,
   Mils
   // "Unknown" is not supported by this example
   };

// Returns the short name associated to the units.
MIL_CONST_TEXT_PTR GetUnitName(UnitEnum Units);

// Converts 1 unit into inches.
MIL_DOUBLE GetInchesPerWorldUnit(UnitEnum Units);

#endif // UNITS_H
