//***************************************************************************************/
// 
// File name: HandEyeUtils.h
//
// Synopsis:  Utility classes and functions for the HandEyeValibration example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/

#include <mil.h>
#include <math.h>
#include <vector>

//*****************************************************************************
// Struct that represents a 3D vector.
//*****************************************************************************
struct Vec3
   {
   MIL_DOUBLE X;
   MIL_DOUBLE Y;
   MIL_DOUBLE Z;
   };

//*****************************************************************************
// Class that represents a Sphere.
//*****************************************************************************
class SphereGeo
   {
   public:
      MIL_DOUBLE Radius;
      Vec3 Center;

      void operator=(const SphereGeo& _other)
         {
         Radius = _other.Radius;
         Center = _other.Center;
         }
   };

//*****************************************************************************
// Struct that contains positional data.
//*****************************************************************************
struct Transform
   {
   MIL_DOUBLE m_PositionX;
   MIL_DOUBLE m_PositionY;
   MIL_DOUBLE m_PositionZ;
   MIL_DOUBLE m_RotationX;
   MIL_DOUBLE m_RotationY;
   MIL_DOUBLE m_RotationZ;
   };

//*****************************************************************************
// Struct that contains pose data.
//*****************************************************************************
struct SPoseData
   {
   MIL_CONST_TEXT_PTR m_PointCloudFile;
   Transform m_Tool;
   };

//*****************************************************************************
// Struct that contains sphere info.
//*****************************************************************************
class SphereStats
   {
   public:
      SphereStats():mBRadiusID(-1)
         {
         }
      SphereStats(const SphereGeo& Sphere):mBRadiusID(-1)
         {
         mSphere = Sphere;
         }

      void SetRadiusID(const std::vector<MIL_DOUBLE>& aRadiusClasses)
         {
         MIL_DOUBLE BestRadiusDistance = MIL_DOUBLE_MAX;
         for(MIL_INT r = 0; r < (MIL_INT)aRadiusClasses.size(); r++)
            {
            MIL_DOUBLE CurRadiusDistance = fabs(mSphere.Radius - aRadiusClasses[r]);
            if(CurRadiusDistance < BestRadiusDistance)
               {
               mBRadiusID = r;
               BestRadiusDistance = CurRadiusDistance;
               }
            }
         }

      SphereGeo mSphere;
      MIL_INT mBRadiusID;
      MIL_DOUBLE mScore;
   };

//*****************************************************************************
// Creates stream of sphere centers.
//*****************************************************************************
class PointStream
   {
   private:
      //Forbid Copy constructor
      PointStream(const PointStream&);
   public:
      PointStream(const std::vector<SphereStats>& Spheres)
         {
         Populate(Spheres);
         }
      void Populate(const std::vector<SphereStats>& Spheres)
         {
         mStreamX.resize(Spheres.size());
         mStreamY.resize(Spheres.size());
         mStreamZ.resize(Spheres.size());

         for(size_t i = 0; i < Spheres.size(); i++)
            {
            mStreamX[i] = (MIL_FLOAT)Spheres[i].mSphere.Center.X;
            mStreamY[i] = (MIL_FLOAT)Spheres[i].mSphere.Center.Y;
            mStreamZ[i] = (MIL_FLOAT)Spheres[i].mSphere.Center.Z;
            }

         }

      MIL_UNIQUE_BUF_ID CreateStreamBuffer(MIL_ID MilSystem)
         {
         void* data[3] = {mStreamX.data(),mStreamY.data(), mStreamZ.data()};
         return MbufCreateColor(MilSystem, 3, (MIL_INT)mStreamX.size(), 1, M_FLOAT + 32, M_ARRAY, M_HOST_ADDRESS + M_PITCH, (MIL_INT)mStreamX.size(), data, M_UNIQUE_ID);
         }


   private:
      std::vector<MIL_FLOAT> mStreamX;
      std::vector<MIL_FLOAT> mStreamY;
      std::vector<MIL_FLOAT> mStreamZ;
   };

//*****************************************************************************
// Functions declaration.
//*****************************************************************************
void IndexToColor(const size_t &Index, MIL_INT32 &ColorValue, MIL_STRING &ColorString);
MIL_DOUBLE CalculateMatrixDiscrepancy(MIL_ID MilSystem, MIL_ID MatrixID, const MIL_DOUBLE* ExpectedMatrix);
void DisplayMatrix(MIL_ID MilMatrix);
void AddComponentNormalsIfMissing(MIL_ID MilContainer);
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem);
