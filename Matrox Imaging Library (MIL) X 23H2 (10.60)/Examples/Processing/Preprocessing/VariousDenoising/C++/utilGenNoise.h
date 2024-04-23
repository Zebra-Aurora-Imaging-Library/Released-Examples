/************************************************************************************/
/*
 * File name: utilGenNoise.cpp 
 *
 * Synopsis:  Utility functions to add noise to an image
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;

/* Functions prototypes. */
void       AddGaussianNoise      (MIL_ID Source, MIL_ID Dest, MIL_INT DataSize, MIL_INT DataType, MIL_DOUBLE Var, MIL_DOUBLE Mean = 0.0);
void       AddPoissonNoise       (MIL_ID Source, MIL_ID Dest, MIL_INT DataSize, MIL_INT DataType);
void       AddSaltAndPepperNoise (MIL_ID Source, MIL_ID Dest, MIL_INT DataSizePerBand, MIL_INT NbBand, const MIL_INT DataType, MIL_DOUBLE NoiseDensity);
void       ComputeMSE            (MIL_ID ReferenceSignal, MIL_ID DenoisedSignal, MIL_INT DataSize, MIL_INT DataType, MIL_DOUBLE* MSE);
void       PutData               (MIL_ID BufferId, MIL_INT NbPixels, MIL_INT DataType, vector<MIL_DOUBLE>& pixelsValues);
void       GetData               (MIL_ID BufferId, MIL_INT NbPixels, MIL_INT DataType, vector<MIL_DOUBLE>& pixelsValues);
MIL_DOUBLE PickDouble            (MIL_DOUBLE min, MIL_DOUBLE max);
MIL_DOUBLE PickGauss             (MIL_DOUBLE m, MIL_DOUBLE v);

// Utility functions to get numerical limits from the data type of a pixel
template <typename PixelType> MIL_DOUBLE GetInf();
template <> MIL_DOUBLE GetInf<MIL_UINT8 >() { return static_cast<MIL_DOUBLE>( MIL_UINT8_MIN ); }
template <> MIL_DOUBLE GetInf<MIL_UINT16>() { return static_cast<MIL_DOUBLE>( MIL_UINT16_MIN); }
template <> MIL_DOUBLE GetInf<MIL_UINT32>() { return static_cast<MIL_DOUBLE>( MIL_UINT32_MIN); }
template <> MIL_DOUBLE GetInf<MIL_INT8  >() { return static_cast<MIL_DOUBLE>( MIL_INT8_MIN  ); }
template <> MIL_DOUBLE GetInf<MIL_INT16 >() { return static_cast<MIL_DOUBLE>( MIL_INT16_MIN ); }
template <> MIL_DOUBLE GetInf<MIL_INT32 >() { return static_cast<MIL_DOUBLE>( MIL_INT32_MIN ); }
template <> MIL_DOUBLE GetInf<MIL_FLOAT >() { return static_cast<MIL_DOUBLE>(-MIL_FLOAT_MAX ); }

template <typename PixelType> MIL_DOUBLE GetSup();
template <> MIL_DOUBLE GetSup<MIL_UINT8 >() { return static_cast<MIL_DOUBLE>(MIL_UINT8_MAX ); }
template <> MIL_DOUBLE GetSup<MIL_UINT16>() { return static_cast<MIL_DOUBLE>(MIL_UINT16_MAX); }
template <> MIL_DOUBLE GetSup<MIL_UINT32>() { return static_cast<MIL_DOUBLE>(MIL_UINT32_MAX); }
template <> MIL_DOUBLE GetSup<MIL_INT8  >() { return static_cast<MIL_DOUBLE>(MIL_INT8_MAX  ); }
template <> MIL_DOUBLE GetSup<MIL_INT16 >() { return static_cast<MIL_DOUBLE>(MIL_INT16_MAX ); }
template <> MIL_DOUBLE GetSup<MIL_INT32 >() { return static_cast<MIL_DOUBLE>(MIL_INT32_MAX ); }
template <> MIL_DOUBLE GetSup<MIL_FLOAT >() { return static_cast<MIL_DOUBLE>(MIL_FLOAT_MAX ); }

/*****************************************************************************/
/*            Pick a random MIL_DOUBLE value between min and max.            */
/*****************************************************************************/
inline double PickDouble(MIL_DOUBLE min, MIL_DOUBLE max)
   {
   return  (min + (((MIL_DOUBLE) rand() / RAND_MAX) * (max - min)));
   }

/*****************************************************************************/
/*     Get a random MIL_DOUBLE value of the normal distribution N(m, v).     */
/*****************************************************************************/
inline double PickGauss(MIL_DOUBLE m, MIL_DOUBLE v)
   {
   MIL_DOUBLE a;
   MIL_DOUBLE b;
   MIL_DOUBLE s;
   do
      {
      a = PickDouble(-1.0, 1.0);
      b = PickDouble(-1.0, 1.0);
      s = a*a + b*b; // s in [0;1[
      }
      while ((s >= 1.0) || (s == 0.0));

   MIL_DOUBLE x = sqrt(-2.0 * log(s) / s);
   if (PickDouble(0.0, 1.0) > 0.5)
      { x *= a; }
   else
      { x *= b; }

   return m + sqrt(v) * x;
   }

/*****************************************************************************/
/*                   Add Gaussian noise on the source image.                 */
/*****************************************************************************/
void AddGaussianNoise(MIL_ID Source, MIL_ID Dest, MIL_INT DataSize, MIL_INT DataType, MIL_DOUBLE Var, MIL_DOUBLE Mean)
   {
   vector<MIL_DOUBLE> pixels;

   /* Get pixels values from the source image. */
   pixels.resize(DataSize);
   GetData(Source, DataSize, DataType, pixels);

   /* Add Gaussian noise to the pixels values. */
   for(MIL_INT i = 0; i < DataSize; i++)
      pixels[i] += PickGauss(Mean, Var);

   /* Put modified pixels values in the destination image. */
   PutData(Dest, DataSize, DataType, pixels);
   }

/*****************************************************************************/
/*               Add Salt and Pepper noise on the source image.              */
/*****************************************************************************/
void AddSaltAndPepperNoise(MIL_ID Source, MIL_ID Dest, MIL_INT DataSizePerBand, MIL_INT NbBand, MIL_INT DataType, MIL_DOUBLE NoiseDensity)
   {
   MIL_DOUBLE InfBound = -1, SupBound = -1, tmpVal = -1;
   vector<MIL_DOUBLE> pixels;

   /* Defining min and max values of the image according to data type. */
   switch(DataType)
      {
      case 8 + M_UNSIGNED : { InfBound = MIL_UINT8_MIN ;  SupBound = MIL_UINT8_MAX ; } break;
      case 16 + M_UNSIGNED: { InfBound = MIL_UINT16_MIN;  SupBound = MIL_UINT16_MAX; } break;
      case 32 + M_UNSIGNED: { InfBound = MIL_UINT32_MIN;  SupBound = MIL_UINT32_MAX; } break;
      case 8 + M_SIGNED   : { InfBound = MIL_INT8_MIN  ;  SupBound = MIL_INT8_MAX  ; } break;
      case 16 + M_SIGNED  : { InfBound = MIL_INT16_MIN ;  SupBound = MIL_INT16_MAX ; } break;
      case 32 + M_SIGNED  : { InfBound = MIL_INT32_MIN ;  SupBound = MIL_INT32_MAX ; } break;
      case 32 + M_FLOAT   : { InfBound = MIL_FLOAT_MIN ;  SupBound = MIL_FLOAT_MAX ; } break;
      }

   /* Get pixels values from the source image. */
   pixels.resize(DataSizePerBand*NbBand);
   GetData(Source, DataSizePerBand*NbBand, DataType, pixels);

   /* Add Salt and Pepper noise to the pixels values. */
   const MIL_DOUBLE Limit1 = (NoiseDensity / (2.0*MIL_DOUBLE(NbBand)));
   const MIL_DOUBLE Limit2 = (NoiseDensity / (    MIL_DOUBLE(NbBand)));
   for(MIL_INT i = 0; i < (DataSizePerBand*NbBand); i++)
      {
      tmpVal = PickDouble(0.0, 1.0);
      if(tmpVal < Limit1)
         { pixels[i] = InfBound; }
      else if (tmpVal < Limit2)
         { pixels[i] = SupBound; }
      }

   /* Put modified pixels values in the destination image. */
   PutData(Dest, DataSizePerBand*NbBand, DataType, pixels);
   }

/*****************************************************************************/
/*                   Add Poisson noise on the source image.                  */
/*****************************************************************************/
void AddPoissonNoise(MIL_ID Source, MIL_ID Dest, MIL_INT DataSize, MIL_INT DataType)
   {
   vector<MIL_DOUBLE> pixels, modifiedpixels, t, g;
   vector<MIL_INT>    index; 

   /* Get pixels values from the source image. */
   pixels.resize(DataSize);   
   GetData(Source, DataSize, DataType, pixels);

   /* Compute Poisson noise to the pixels values. */
   index         .resize(DataSize);
   g             .resize(DataSize);
   modifiedpixels.resize(DataSize, -1.0);
   t             .resize(DataSize,  1.0);

   for(MIL_INT i = 0; i < DataSize; i++)
      {
      index[i] = i;
      g[i]     = exp(-pixels[i]);
      }

   while(index.size() > 0)
      {
      for(MIL_INT i = 0; i < MIL_INT(index.size()) ; i++)
         {
         modifiedpixels[index[i]]++;
         t [index[i]] *= PickDouble(0.0, 1.0);
         }

      MIL_INT u = 0;
      while(u < MIL_INT(index.size()))
         {
         if(t[index[u]] > g[index[u]])
            { u++; }
         else
            { index.erase(index.begin() + u); }
         }
      }

   /* Put modified pixels values in the destination image. */
   PutData(Dest, DataSize, DataType, modifiedpixels);
   }

/*****************************************************************************/
/*    Template utility function for the GetData function.                    */
/*****************************************************************************/
template <typename PixelType>
void GetDataT(MIL_ID BufferId, MIL_INT DataSize, vector<MIL_DOUBLE>& pixelsValues)
   {
   /* Get pixels values from the source image. */
   vector<PixelType> pixels;
   pixels.resize(DataSize);
   MbufGet(BufferId, &pixels[0]);

   /* Convert from the original data type to MIL_DOUBLE. */
   for(MIL_INT i = 0; i < DataSize; i++)
      { pixelsValues[i] = MIL_DOUBLE(pixels[i]); }
   }

/*****************************************************************************/
/*                 Get pixels values from the source buffer.                 */
/*****************************************************************************/
void GetData(MIL_ID BufferId, MIL_INT NbPixels, MIL_INT DataType, vector<MIL_DOUBLE>& pixelsValues)
   {
   switch(DataType)
      {
      case 8 + M_UNSIGNED  : { GetDataT<MIL_UINT8 >(BufferId, NbPixels, pixelsValues); } break;
      case 16 + M_UNSIGNED : { GetDataT<MIL_UINT16>(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_UNSIGNED : { GetDataT<MIL_UINT32>(BufferId, NbPixels, pixelsValues); } break;
      case 8 + M_SIGNED    : { GetDataT<MIL_INT8  >(BufferId, NbPixels, pixelsValues); } break;
      case 16 + M_SIGNED   : { GetDataT<MIL_INT16 >(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_SIGNED   : { GetDataT<MIL_INT32 >(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_FLOAT    : { GetDataT<MIL_FLOAT >(BufferId, NbPixels, pixelsValues); } break;
      }
   }

/*****************************************************************************/
/*    Template utility function for the PutData function                     */
/*****************************************************************************/
template <typename PixelType>
void PutDataT(MIL_ID BufferId, MIL_INT NbPixels, vector<MIL_DOUBLE>& pixelsValues)
   {
   /* Defining min and max values of the image according to data type. */
   MIL_DOUBLE InfBound = GetInf<PixelType>();
   MIL_DOUBLE SupBound = GetSup<PixelType>();

   vector<PixelType> pixels;
   pixels.resize(NbPixels);
   for(MIL_INT i = 0; i < NbPixels; i++)
      {
      /* Constraint pixels values to between the min and the max of the destination buffer data type. */ 
      pixelsValues[i] = (pixelsValues[i] > SupBound) ? SupBound : pixelsValues[i];
      pixelsValues[i] = (pixelsValues[i] < InfBound) ? InfBound : pixelsValues[i];

      /* Convert from MIL_DOUBLE to the destination buffer data type. */
      pixels[i] = static_cast<PixelType>(pixelsValues[i]);
      }

   /* Put pixels values in the destination image. */
   MbufPut(BufferId, &pixels[0]);
   }

/*****************************************************************************/
/*               Put pixels values in the destination buffer.                */
/*****************************************************************************/
void PutData(MIL_ID BufferId, MIL_INT NbPixels, MIL_INT DataType, vector<MIL_DOUBLE>& pixelsValues)
   {
   switch(DataType)
      {
      case 8 + M_UNSIGNED  : { PutDataT<MIL_UINT8 >(BufferId, NbPixels, pixelsValues); } break;
      case 16 + M_UNSIGNED : { PutDataT<MIL_UINT16>(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_UNSIGNED : { PutDataT<MIL_UINT32>(BufferId, NbPixels, pixelsValues); } break;
      case 8 + M_SIGNED    : { PutDataT<MIL_INT8  >(BufferId, NbPixels, pixelsValues); } break;
      case 16 + M_SIGNED   : { PutDataT<MIL_INT16 >(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_SIGNED   : { PutDataT<MIL_INT32 >(BufferId, NbPixels, pixelsValues); } break;
      case 32 + M_FLOAT    : { PutDataT<MIL_FLOAT >(BufferId, NbPixels, pixelsValues); } break;
      }
   }

/*****************************************************************************/
/*                         Compute Mean Square Error                         */
/*****************************************************************************/
void ComputeMSE(MIL_ID ReferenceSignal, MIL_ID DenoisedSignal, MIL_INT DataSize, MIL_INT DataType, MIL_DOUBLE * MSE)
   {
   vector <MIL_DOUBLE> RefCoeff, DenoisedCoeff;

   /* Get pixels values from the reference and the denoised image. */
   RefCoeff     .resize(DataSize);
   DenoisedCoeff.resize(DataSize);
   GetData(ReferenceSignal, DataSize, DataType, RefCoeff     );
   GetData(DenoisedSignal , DataSize, DataType, DenoisedCoeff);

   /* Compute Mean Square Error. */
   *MSE = 0;
   for(MIL_INT i = 0; i < DataSize; i++)
      { *MSE += (RefCoeff[i] - DenoisedCoeff[i]) * (RefCoeff[i] - DenoisedCoeff[i]); }
   *MSE /= RefCoeff.size();
   }
