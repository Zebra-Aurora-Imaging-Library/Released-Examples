//*************************************************************************************
//
// Synopsis: Structure of 2D and 3D points for customers application.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

template <typename T>
struct CPoint2D
   {
   CPoint2D() {}
   CPoint2D(T x_, T y_) : x(x_), y(y_) {}

   T x;
   T y;
   };

template <typename T>
struct CPoint3D
   {
   CPoint3D() {}
   CPoint3D(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

   T x;
   T y;
   T z;
   };

typedef CPoint2D<MIL_INT> CiPoint2D; 
typedef CPoint3D<MIL_INT> CiPoint3D;
typedef CPoint2D<MIL_DOUBLE> CdPoint2D;
typedef CPoint3D<MIL_DOUBLE> CdPoint3D;
