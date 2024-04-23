//*************************************************************************************
//
// Synopsis: Structure of rectangle shapes for customers application.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

template <typename T>
struct CRect
   {
   CRect() {}
   CRect(T OffX_, T OffY_, T SizeX_, T SizeY_) : OffX(OffX_), OffY(OffY_), SizeX(SizeX_), SizeY(SizeY_) {}

   T OffX;
   T OffY;
   T SizeX;
   T SizeY;
   };

typedef CRect<MIL_INT> CiRect; 
typedef CRect<MIL_DOUBLE> CdRect; 
