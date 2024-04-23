// ******************************************************************************
//
// File name: vec3.h
//
// Synopsis:  Useful 3d vector operations.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
// ******************************************************************************

#pragma once

struct Vec3
   {
   double x, y, z;
   };

inline bool operator==(const Vec3& lhs, const Vec3& rhs)
   {
   return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
   }

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
   {
   return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
   }

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
   {
   return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
   }

inline Vec3 operator*(double d, const Vec3& v)
   {
   return { d * v.x, d * v.y, d * v.z };
   }

inline Vec3 operator/(const Vec3& v, double d)
   {
   return (1.0 / d) * v;
   }

inline double Dot(const Vec3& lhs, const Vec3& rhs)
   {
   return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
   }

inline Vec3 Cross(const Vec3& lhs, const Vec3& rhs)
   {
   return { lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x };
   }

inline double NormSqr(const Vec3& v)
   {
   return Dot(v, v);
   }

inline double Norm(const Vec3& v)
   {
   return sqrt(NormSqr(v));
   }

inline double Distance(const Vec3& v1, const Vec3& v2)
   {
   return Norm(v2 - v1);
   }

// Project vector 'a' onto a unit vector 'b'.
inline Vec3 ProjectUnit(const Vec3& a, const Vec3& b)
   {
   assert(abs(Norm(b) - 1.0) < 1e-6); // b should be a unit vector.
   return Dot(a, b) * b;
   }
