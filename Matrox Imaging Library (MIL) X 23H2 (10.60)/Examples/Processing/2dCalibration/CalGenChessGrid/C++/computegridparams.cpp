//***************************************************************************************
// 
// File name: computegridparams.cpp
//
// Synopsis:  This file implements ComputeGridParameters(), declared in common.h.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "common.h"

//*****************************************************************************
// Returns the base 10 exponent of the given value.
//*****************************************************************************
static inline MIL_INT GetExponent(MIL_DOUBLE Value)
   {
   return static_cast<MIL_INT>(floor(log10(Value)));
   }

//*****************************************************************************
// Returns the divisor that is used to get the mantissa out of a value, for the
// given exponent.
//*****************************************************************************
static inline MIL_DOUBLE ComputeRoundingPowerOf10(MIL_INT Exponent)
   {
   if (SPACING_ROUNDING == RoundUpTo3Digits)
      Exponent -= 2; // mantissa will have two more digits
   return pow(10.0, static_cast<int>(Exponent));
   }

//*****************************************************************************
// Given an exponent, returns the mantissa of the given value, rounded up
// according to the rounding rule chosen.
//*****************************************************************************
static MIL_INT GetRoundedUpMantissa(MIL_DOUBLE Value, MIL_INT Exponent)
   {
   MIL_DOUBLE Mantissa = Value / ComputeRoundingPowerOf10(Exponent);
   MIL_DOUBLE RoundedMantissa;
   if (SPACING_ROUNDING == RoundUpTo1or2or5)
      {
      if (Mantissa <= 1.0)
         RoundedMantissa = 1.0;
      else if (Mantissa <= 2.0)
         RoundedMantissa = 2.0;
      else if (Mantissa <= 5.0)
         RoundedMantissa = 5.0;
      else
         RoundedMantissa = 10.0;
      }
   else
      {
      RoundedMantissa = ceil(Mantissa);
      }

   return static_cast<MIL_INT>(RoundedMantissa);
   }

//*****************************************************************************
// While rounding up, the number of digits in the mantissa can increase. This
// function is used to reduce it back (and increase the exponent accordingly).
//*****************************************************************************
static void AdjustExponent(MIL_INT* pExponent, MIL_INT* pMantissa)
   {
   MIL_INT MaxMantissa = (SPACING_ROUNDING == RoundUpTo3Digits ? 1000 : 10);
   if (*pMantissa >= MaxMantissa)
      {
      *pMantissa /= 10;
      ++*pExponent;
      }
   }

//*****************************************************************************
// Given the final exponent and rounded up mantissa, compute the spacing to
// be used in the calibration grid.
//*****************************************************************************
static inline MIL_DOUBLE ComputeSpacing(MIL_INT SpacingExponent, MIL_INT SpacingMantissa)
   {
   return SpacingMantissa * ComputeRoundingPowerOf10(SpacingExponent);
   }

//*****************************************************************************
// Ensure that the mantissa has the correct number of digits.
//*****************************************************************************
static void ValidateMantissa(MIL_INT Mantissa)
   {
   bool IsValid = false;
   switch (SPACING_ROUNDING)
      {
      case RoundUpTo3Digits:
         if (1 <= Mantissa && Mantissa < 1000)
            IsValid = true;
         break;
      case RoundUpTo1Digit:
         if (1 <= Mantissa && Mantissa < 10)
            IsValid = true;
         break;
      case RoundUpTo1or2or5:
         if (Mantissa == 1 || Mantissa == 2 || Mantissa == 5)
            IsValid = true;
         break;
      }
   if (!IsValid)
      throw MIL_TEXT("Mantissa does not have the correct number of digits");
   }

//*****************************************************************************
// According to the user options in gridconfig.h, choose the grid dimensions,
// number of squares and spacings.
//*****************************************************************************
GridInfoStruct ComputeGridParameters()
   {
   GridInfoStruct GridInfo;

   if (MIN_GRID_SIZE_X <= 0.0 || MIN_GRID_SIZE_Y <= 0.0)
      throw MIL_TEXT("Grid size must be positive");

   // Print input parameters.
   MIL_CONST_TEXT_PTR UnitName = GetUnitName(UNIT);
   MosPrintf(MIL_TEXT("Grid input parameters\n"));
   MosPrintf(MIL_TEXT("---------------------\n"));
   MosPrintf(MIL_TEXT("  Minimum grid size (w/o quiet zone): %g %s x %g %s\n"), MIN_GRID_SIZE_X, UnitName, MIN_GRID_SIZE_Y, UnitName);
#if SPECIFY_NUM_SQUARES_DIRECTLY
   MosPrintf(MIL_TEXT("  Maximum number of grid squares:     %d x %d\n"), (int)MAX_NUM_SQUARES_X, (int)MAX_NUM_SQUARES_Y);
   MosPrintf(MIL_TEXT("  Enforce square chessboard cells:    %s\n"), ENFORCE_SQUARE_CELLS ? MIL_TEXT("Yes") : MIL_TEXT("No"));
#else
   MosPrintf(MIL_TEXT("  Camera field of view along X:       %g %s\n"), CAMERA_FOV_X, UnitName);
   MosPrintf(MIL_TEXT("  Camera resolution along X:          %d pixels\n"), (int)CAMERA_RESOLUTION_X);
#endif
   MosPrintf(MIL_TEXT("  Spacing rounding mode:              "));
   switch (SPACING_ROUNDING)
      {
      case RoundUpTo3Digits: MosPrintf(MIL_TEXT("3-digits mantissa\n")); break;
      case RoundUpTo1Digit:  MosPrintf(MIL_TEXT("1-digit mantissa\n")); break;
      case RoundUpTo1or2or5: MosPrintf(MIL_TEXT("1-digit mantissa (1, 2 or 5)\n")); break;
      default: throw MIL_TEXT("Unknown rounding mode");
      }
   MosPrintf(MIL_TEXT("\n"));

   // Compute the minimum spacings according to the minimum grid size and other parameters.
#if SPECIFY_NUM_SQUARES_DIRECTLY
   if (MAX_NUM_SQUARES_X < 3)
      throw MIL_TEXT("Number of squares must be at least 3");
   if (MAX_NUM_SQUARES_Y < 3)
      throw MIL_TEXT("Number of squares must be at least 3");
   MIL_DOUBLE DesiredSpacingX = MIN_GRID_SIZE_X / static_cast<MIL_DOUBLE>(MAX_NUM_SQUARES_X);
   MIL_DOUBLE DesiredSpacingY = MIN_GRID_SIZE_Y / static_cast<MIL_DOUBLE>(MAX_NUM_SQUARES_Y);
   bool EnforceSquareCells = ENFORCE_SQUARE_CELLS;
#else
   if (CAMERA_FOV_X <= 0.0)
      throw MIL_TEXT("Camera field of view must be positive");
   if (CAMERA_RESOLUTION_X <= 0)
      throw MIL_TEXT("Camera resolution must be positive");
   MIL_INT MaxSquaresInCamera = CAMERA_RESOLUTION_X / MIN_CAMERA_PIXELS_PER_SQUARE;
   MIL_DOUBLE DesiredSpacingX = CAMERA_FOV_X / MaxSquaresInCamera;
   MIL_DOUBLE DesiredSpacingY = DesiredSpacingX;
   bool EnforceSquareCells = true;
#endif

   // Choose the exponent using the largest spacing, then compute and round the mantissae of
   // both spacings.
   if (DesiredSpacingX >= DesiredSpacingY)
      {
      GridInfo.SpacingExponent  = GetExponent(DesiredSpacingX);
      GridInfo.SpacingMantissaX = GetRoundedUpMantissa(DesiredSpacingX, GridInfo.SpacingExponent);
      // Rounding up might have increased the number of digits in the mantissa.
      AdjustExponent(&GridInfo.SpacingExponent, &GridInfo.SpacingMantissaX);
      if (EnforceSquareCells)
         GridInfo.SpacingMantissaY = GridInfo.SpacingMantissaX;
      else
         GridInfo.SpacingMantissaY = GetRoundedUpMantissa(DesiredSpacingY, GridInfo.SpacingExponent);
      }
   else
      {
      GridInfo.SpacingExponent  = GetExponent(DesiredSpacingY);
      GridInfo.SpacingMantissaY = GetRoundedUpMantissa(DesiredSpacingY, GridInfo.SpacingExponent);
      // Rounding up might have increased the number of digits in the mantissa.
      AdjustExponent(&GridInfo.SpacingExponent, &GridInfo.SpacingMantissaY);
      if (EnforceSquareCells)
         GridInfo.SpacingMantissaX = GridInfo.SpacingMantissaY;
      else
         GridInfo.SpacingMantissaX = GetRoundedUpMantissa(DesiredSpacingX, GridInfo.SpacingExponent);
      }

   // Compute back the spacings to be used from the rounded mantissae and exponent.
   GridInfo.SpacingX = ComputeSpacing(GridInfo.SpacingExponent, GridInfo.SpacingMantissaX);
   GridInfo.SpacingY = ComputeSpacing(GridInfo.SpacingExponent, GridInfo.SpacingMantissaY);

   ValidateMantissa(GridInfo.SpacingMantissaX);
   ValidateMantissa(GridInfo.SpacingMantissaY);

   // Compute the final number of squares using the rounded spacings.
   GridInfo.NumSquaresX = static_cast<MIL_INT>(ceil(MIN_GRID_SIZE_X / GridInfo.SpacingX));
   GridInfo.NumSquaresY = static_cast<MIL_INT>(ceil(MIN_GRID_SIZE_Y / GridInfo.SpacingY));

   if (GridInfo.NumSquaresX < 2) GridInfo.NumSquaresX = 2;
   if (GridInfo.NumSquaresY < 2) GridInfo.NumSquaresY = 2;

   // Include one square on each side for the quiet zone.
   GridInfo.NumSquaresX += 2 * NUM_SQUARES_FOR_QUIET_ZONE;
   GridInfo.NumSquaresY += 2 * NUM_SQUARES_FOR_QUIET_ZONE;

   // Compute the final grid size, including quiet zone.
   GridInfo.GridSizeX = GridInfo.NumSquaresX * GridInfo.SpacingX;
   GridInfo.GridSizeY = GridInfo.NumSquaresY * GridInfo.SpacingY;

   // Print output parameters.
   MosPrintf(MIL_TEXT("Computed grid parameters\n"));
   MosPrintf(MIL_TEXT("------------------------\n"));
   MosPrintf(MIL_TEXT("  Grid size (with quiet zone): %g %s x %g %s\n"),
             GridInfo.GridSizeX, UnitName, GridInfo.GridSizeY, UnitName);
   MosPrintf(MIL_TEXT("  Number of grid squares:      %d x %d\n"),
             (int)(GridInfo.NumSquaresX - 2 * NUM_SQUARES_FOR_QUIET_ZONE),
             (int)(GridInfo.NumSquaresY - 2 * NUM_SQUARES_FOR_QUIET_ZONE));
   if (GridInfo.SpacingX == GridInfo.SpacingY)
      MosPrintf(MIL_TEXT("  Spacing:                     %g %s\n"), GridInfo.SpacingX, UnitName);
   else
      MosPrintf(MIL_TEXT("  Spacings:                    %g %s x %g %s\n"), GridInfo.SpacingX, UnitName, GridInfo.SpacingY, UnitName);
   MosPrintf(MIL_TEXT("\n"));

   return GridInfo;
   }
