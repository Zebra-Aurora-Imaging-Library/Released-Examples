//***************************************************************************************
// 
// File name: common.h 
//
// Synopsis:  This file includes all the necessary headers and declares the 
//            principal functions.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef COMMON_H
#define COMMON_H

// MIL header
#include <mil.h>

// Standard headers
#include <cmath>
#include <cstdlib>

// Constant used in gridconfig.h
static const MIL_INT CENTER = M_DEFAULT; 

// Example headers
#include "units.h"
#include "gridconfig.h"

//*****************************************************************************
// Structure holding pixel-independent information about a calibration grid.
//*****************************************************************************
struct GridInfoStruct
   {
   MIL_INT    NumSquaresX; // number of squares (incl. quiet zone) along the grid's X axis
   MIL_INT    NumSquaresY; // number of squares (incl. quiet zone) along the grid's Y axis
   MIL_DOUBLE SpacingX;    // size, in world units (see UNIT), of one grid square along the grid's X axis
   MIL_DOUBLE SpacingY;    // size, in world units (see UNIT), of one grid square along the grid's Y axis
   MIL_DOUBLE GridSizeX;   // total grid size (incl. quiet zone) in world units (see UNIT) along its X axis
   MIL_DOUBLE GridSizeY;   // total grid size (incl. quiet zone) in world units (see UNIT) along its Y axis

   // Spacing information to be encoded in fiducials.
   MIL_INT    SpacingExponent;   // Common exponent of SpacingX and SpacingY, in [-5, 4]
   MIL_INT    SpacingMantissaX;  // Mantissa for SpacingX; if more than 1 digit, this is actually 100*mantissa
   MIL_INT    SpacingMantissaY;  // Mantissa for SpacingY; if more than 1 digit, this is actually 100*mantissa

   // Helper functions
#if NUM_FIDUCIALS > 0
   // Returns the index of the square intersection (along the X axis) used as
   // grid reference point.
   inline MIL_INT GetReferencePositionX() const
      {
      if (REF_POINT_POS_X == CENTER)
         return NumSquaresX / 2;
      else
         return REF_POINT_POS_X + NUM_SQUARES_FOR_QUIET_ZONE;
      }

   // Returns the index of the square intersection (along the Y axis) used as
   // grid reference point.
   inline MIL_INT GetReferencePositionY() const
      {
      if (REF_POINT_POS_Y == CENTER)
         return NumSquaresY / 2;
      else
         return REF_POINT_POS_Y + NUM_SQUARES_FOR_QUIET_ZONE;
      }
#endif
   };

// Fill a GridInfoStruct according to the parameters in gridconfig.h.
GridInfoStruct ComputeGridParameters();

// According to the fiducial number and positions in gridconfig.h, encode the
// grid information in each fiducial, and draw the fiducials at the correct
// position in the grid image.
void AddFiducials(MIL_ID GridImageId,
                  const GridInfoStruct& GridInfo,
                  MIL_DOUBLE PixelsPerSquareX,
                  MIL_DOUBLE PixelsPerSquareY);

#endif // COMMON_H
