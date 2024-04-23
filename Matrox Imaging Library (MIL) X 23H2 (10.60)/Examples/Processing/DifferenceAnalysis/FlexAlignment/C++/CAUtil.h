//*************************************************************************************
//
// Synopsis: Util class and function for customers application.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

#include <vector>
#include <time.h>
#include <mil.h>

using std::vector;

struct SBuffer
   {
   SBuffer() { Sx = 0; Sy = 0; Sb = 0; Id = M_NULL; }
   SBuffer(MIL_ID MilBuffer) { Wrap(MilBuffer); }

   void Wrap(MIL_ID MilBuffer) { Sx = MbufInquire(MilBuffer, M_SIZE_X, M_NULL);
                                 Sy = MbufInquire(MilBuffer, M_SIZE_Y, M_NULL);
                                 Sb = MbufInquire(MilBuffer, M_SIZE_BAND, M_NULL);
                                 Id = MilBuffer;}

   MIL_INT Sx;          /* SizeX                */
   MIL_INT Sy;          /* SizeY                */
   MIL_INT Sb;          /* SizeBand             */
   MIL_ID Id;           /* MIL Id of the buffer */
   };

/* Load a buffer from the disk and store minimal information in NewBuffer. */
static MIL_INT LoadBuffer(const MIL_ID MilSystem, MIL_CONST_TEXT_PTR Filename, SBuffer &NewBuffer)
   {
   /* Restore the buffer. */ 
   MbufRestore(Filename, MilSystem, &NewBuffer.Id);

   /* Inquire useful information. */
   MbufInquire(NewBuffer.Id, M_SIZE_X, &NewBuffer.Sx);
   MbufInquire(NewBuffer.Id, M_SIZE_Y, &NewBuffer.Sy);
   MbufInquire(NewBuffer.Id, M_SIZE_BAND, &NewBuffer.Sb);

   return MappGetError(M_GLOBAL, M_NULL);
   }

/* Load the buffer from the disk and convert to 8U if needed. */
static MIL_INT RestoreAndConvert(const MIL_ID MilSystem, MIL_CONST_TEXT_PTR Filename, SBuffer &NewProcBuffer, MIL_INT Type=8+M_UNSIGNED)
   {
   /* Load the buffer in a temporary buffer. */
   SBuffer TmpBuffer;
   LoadBuffer(MilSystem, Filename, TmpBuffer);

   NewProcBuffer.Sx = TmpBuffer.Sx;
   NewProcBuffer.Sy = TmpBuffer.Sy;
   NewProcBuffer.Sb = TmpBuffer.Sb;

   /* Allocate a unsigned 8 bit buffer for processing. */
   MbufAlloc2d(MilSystem, NewProcBuffer.Sx, NewProcBuffer.Sy, Type, M_IMAGE+M_PROC+M_DISP,  &NewProcBuffer.Id);
   
   /* Convert input buffer to luminance. */
   if(NewProcBuffer.Sb>1)
      MimConvert(TmpBuffer.Id, NewProcBuffer.Id, M_RGB_TO_L);
   else
      MbufCopy(TmpBuffer.Id, NewProcBuffer.Id);

   MbufFree(TmpBuffer.Id);
   return MappGetError(M_DEFAULT, M_GLOBAL, M_NULL);;
   }

/* Copy the source buffer to the destination buffer with data. */
static void CloneBuffer(const SBuffer &SrcBuffer, SBuffer &DstBuffer, bool WithData=false)
   {
   if(!WithData)
      MbufClone(SrcBuffer.Id, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, &DstBuffer.Id);
   else
      MbufClone(SrcBuffer.Id, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, &DstBuffer.Id);

   DstBuffer.Sx = SrcBuffer.Sx;
   DstBuffer.Sy = SrcBuffer.Sy;
   DstBuffer.Sb = SrcBuffer.Sb;
   }
