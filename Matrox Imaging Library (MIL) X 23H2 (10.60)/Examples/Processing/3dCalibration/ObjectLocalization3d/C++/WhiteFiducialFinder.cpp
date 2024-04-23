//***************************************************************************************
//
// File name: WhiteFiducialFinder.cpp
//
// Synopsis:  Implements the CWhiteFiducialFinder class.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved


#include "common.h"
#include "WhiteFiducialFinder.h"

// All arrays will be resized dynamically. This is the initial size of the arrays.
static const MIL_INT STARTING_ARRAY_SIZE = 8;

// Expected criterion of the white fiducial blob.
static const MIL_DOUBLE MIN_WHITE_AREA = 5.0;

// Settings for the draw operation.
static const MIL_DOUBLE CROSS_COLOR = M_COLOR_RED;
static const MIL_DOUBLE CROSS_HALF_LENGTH = 3.0;

//*****************************************************************************
// Constructor. Allocate and setup MIL objects.
//*****************************************************************************
CWhiteFiducialFinder::CWhiteFiducialFinder(MIL_ID MilSystem)
   {
   // Allocate MIL objects.
   MblobAlloc(MilSystem, M_DEFAULT, M_DEFAULT, &m_MilBlobContext);
   MblobAllocResult(MilSystem, M_DEFAULT, M_DEFAULT, &m_MilBlobResult);

   // Note: these two controls must complement the CBlackFiducialFinder.
   MblobControl(m_MilBlobContext, M_FOREGROUND_VALUE, M_NONZERO); // complement CBlackFiducialFinder, which is M_ZERO
   MblobControl(m_MilBlobContext, M_CONNECTIVITY, M_4_CONNECTED); // complement CBlackFiducialFinder, which is 8-connected

   // Set the features once and for all.
   MblobControl(m_MilBlobContext, M_CENTER_OF_GRAVITY + M_BINARY, M_ENABLE);

   // Manage memory.
   AllocateArrays(STARTING_ARRAY_SIZE);
   m_FiducialIndex = NOT_FOUND;
   }

//*****************************************************************************
// Destructor. Free all MIL objects and memory.
//*****************************************************************************
CWhiteFiducialFinder::~CWhiteFiducialFinder()
   {
   DestroyArrays();
   MblobFree(m_MilBlobResult);
   MblobFree(m_MilBlobContext);
   }

//*****************************************************************************
// Find the white fiducial, i.e. the hole in the black fiducial.
// Returns true if the white fiducial is found.
//*****************************************************************************
bool CWhiteFiducialFinder::Find(MIL_ID MilBinarizedImage, MIL_DOUBLE ApproxBlobX, MIL_DOUBLE ApproxBlobY)
   {
   bool WhiteFiducialFound = false;

   // Find all white blobs in the region around the black fiducial.
   MblobCalculate(m_MilBlobContext, MilBinarizedImage, M_NULL, m_MilBlobResult);

   // Remove white blobs that are too small.
   MblobSelect(m_MilBlobResult, M_DELETE, M_AREA, M_LESS, MIN_WHITE_AREA, M_NULL);

   MIL_INT NbWhiteBlobs;
   MblobGetResult(m_MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NbWhiteBlobs);

   if (NbWhiteBlobs > 0)
      {
      // Get all white blobs CoG.
      ReserveArraySpace(NbWhiteBlobs);
      MblobGetResult(m_MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_X+M_BINARY, m_CoGXArray);
      MblobGetResult(m_MilBlobResult, M_DEFAULT, M_CENTER_OF_GRAVITY_Y+M_BINARY, m_CoGYArray);

      // Find the white blob whose CoG is nearest the given approximate position.
      MIL_DOUBLE MinDist = DBL_MAX;
      for (MIL_INT BlobIdx = 0; BlobIdx < NbWhiteBlobs; ++BlobIdx)
         {
         MIL_DOUBLE dx = m_CoGXArray[BlobIdx] - ApproxBlobX;
         MIL_DOUBLE dy = m_CoGYArray[BlobIdx] - ApproxBlobY;
         MIL_DOUBLE Dist = sqrt(dx*dx + dy*dy);

         if (Dist < MinDist)
            {
            MinDist = Dist;
            m_FiducialIndex = BlobIdx;
            }
         }

      WhiteFiducialFound = true;
      }

   return WhiteFiducialFound;
   }

//*****************************************************************************
// Draw a cross on the white fiducial location, with a longer branch in the
// direction of the black fiducial CoG (so that it gives the direction in
// which to read the pharmacode). The image is assumed to be fixtured already.
//*****************************************************************************
void CWhiteFiducialFinder::Draw(MIL_ID MilOverlayImage) const
   {
   const MIL_INT NB_LINES = 2;

   // Draw in fixtured pixel units.
   MIL_DOUBLE XStart[NB_LINES] = {   -CROSS_HALF_LENGTH,                0.0};
   MIL_DOUBLE YStart[NB_LINES] = {                  0.0, -CROSS_HALF_LENGTH};
   MIL_DOUBLE XEnd  [NB_LINES] = {2.0*CROSS_HALF_LENGTH,                0.0};
   MIL_DOUBLE YEnd  [NB_LINES] = {                  0.0,  CROSS_HALF_LENGTH};

   MgraColor(M_DEFAULT, CROSS_COLOR);
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
   MgraLines(M_DEFAULT, MilOverlayImage, NB_LINES, XStart, YStart, XEnd, YEnd, M_DEFAULT);
   }

//*****************************************************************************
// If there is not enough space in the arrays, free and reallocate them. Else,
// do nothing.
//*****************************************************************************
void CWhiteFiducialFinder::ReserveArraySpace(MIL_INT MinArraySize)
   {
   if (m_ArraySize < MinArraySize)
      {
      DestroyArrays();
      AllocateArrays(MinArraySize);
      }
   }

//*****************************************************************************
// Set new size for the dynamic arrays and allocate them.
//*****************************************************************************
void CWhiteFiducialFinder::AllocateArrays(MIL_INT ArraySize)
   {
   m_CoGXArray = new MIL_DOUBLE[ArraySize];
   m_CoGYArray = new MIL_DOUBLE[ArraySize];
   m_ArraySize = ArraySize;
   }

//*****************************************************************************
// Free the memory of the dynamic arrays.
//*****************************************************************************
void CWhiteFiducialFinder::DestroyArrays()
   {
   delete [] m_CoGYArray;
   delete [] m_CoGXArray;
   }
