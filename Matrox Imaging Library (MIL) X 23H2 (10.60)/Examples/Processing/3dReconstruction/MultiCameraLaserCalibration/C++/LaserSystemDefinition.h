//*****************************************************************************/
// 
// File name: LaserSystemDefinition.h
// 
// Synopsis:  This file contains the definition of the structures that represent
//            the complete system calibration.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef LASER_SYSTEM_DEFINITION_H
#define LASER_SYSTEM_DEFINITION_H

//*****************************************************************************
// Camera calibration information structures.
//*****************************************************************************

// Basic camera calibration using a grid.
class SGridCal
   {
   public:
      SGridCal(MIL_INT Type, MIL_DOUBLE HintX, MIL_DOUBLE HintY, MIL_DOUBLE HintXAngle,
               MIL_INT RowNb, MIL_INT ColNb, MIL_DOUBLE RowSpacing, MIL_DOUBLE ColSpacing,
               MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY, MIL_DOUBLE OffsetZ, MIL_DOUBLE YaxisDir)
         : m_Type(Type), m_HintX(HintX), m_HintY(HintY), m_HintXAngle(HintXAngle),
           m_RowNb(RowNb), m_ColNb(ColNb), m_RowSpacing(RowSpacing), m_ColSpacing(ColSpacing),
           m_OffsetX(OffsetX), m_OffsetY(OffsetY), m_OffsetZ(OffsetZ), m_YaxisDir(YaxisDir)
         {}
      virtual ~SGridCal() {};

      virtual void CalibrateWithGrid(MIL_ID MilCamCal, MIL_ID MilGridImage, MIL_INT Operation) const
         {
         // Set the hint.
         McalControl(MilCamCal, M_GRID_HINT_PIXEL_X, m_HintX);
         McalControl(MilCamCal, M_GRID_HINT_PIXEL_Y, m_HintY);
         McalControl(MilCamCal, M_GRID_HINT_ANGLE_X, m_HintXAngle);

         // Calibrate.
         McalGrid(MilCamCal, MilGridImage, m_OffsetX, m_OffsetY, m_OffsetZ,
            m_RowNb, m_ColNb, m_RowSpacing, m_ColSpacing, Operation, m_Type);
         }

   private:
      MIL_INT     m_Type;
      MIL_DOUBLE  m_HintX;
      MIL_DOUBLE  m_HintY;
      MIL_DOUBLE  m_HintXAngle;
      MIL_INT     m_RowNb;
      MIL_INT     m_ColNb;
      MIL_DOUBLE  m_RowSpacing;
      MIL_DOUBLE  m_ColSpacing;
      MIL_DOUBLE  m_OffsetX;
      MIL_DOUBLE  m_OffsetY;
      MIL_DOUBLE  m_OffsetZ;
      MIL_DOUBLE  m_YaxisDir;
   };

// Camera calibration using a partial grid.
struct SPartialGridCal : public SGridCal
   {
   SPartialGridCal(MIL_DOUBLE HintX, MIL_DOUBLE HintY, MIL_DOUBLE HintXAngle,
                   MIL_DOUBLE RowSpacing, MIL_DOUBLE ColSpacing,
                   MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY, MIL_DOUBLE OffsetZ, MIL_DOUBLE YaxisDir)
      :SGridCal(M_CHESSBOARD_GRID, HintX, HintY, HintXAngle, M_UNKNOWN, M_UNKNOWN,
                RowSpacing, ColSpacing, OffsetX, OffsetY, OffsetZ, YaxisDir)
      {}

      virtual void CalibrateWithGrid(MIL_ID MilCamCal, MIL_ID MilGridImage, MIL_INT Operation) const
      {
      McalControl(MilCamCal, M_GRID_PARTIAL, M_ENABLE);
      SGridCal::CalibrateWithGrid(MilCamCal, MilGridImage, Operation);
      }
   };

// Camera calibration using a grid with a fiducial.
struct SFiducialGridCal : public SPartialGridCal
   {
   SFiducialGridCal(MIL_DOUBLE OffsetX, MIL_DOUBLE OffsetY, MIL_DOUBLE OffsetZ, MIL_DOUBLE YAxisDir)
      :SPartialGridCal(M_NONE, M_NONE, M_NONE, M_FROM_FIDUCIAL, M_FROM_FIDUCIAL,
                       OffsetX, OffsetY, OffsetZ, YAxisDir) 
      {}

   virtual void CalibrateWithGrid(MIL_ID MilCamCal, MIL_ID MilGridImage, MIL_INT Operation) const
      {
      McalControl(MilCamCal, M_GRID_FIDUCIAL, M_DATAMATRIX);
      SPartialGridCal::CalibrateWithGrid(MilCamCal, MilGridImage, Operation);
      }
   };

// Camera calibration parameters.
struct SCameraCal
   {
   inline void CalibrateCameraInt(MIL_ID MilCamCal, MIL_ID MilIntrinsicGridImage) const
      {
      pIntrinsicCal->CalibrateWithGrid(MilCamCal, MilIntrinsicGridImage, M_FULL_CALIBRATION);
      }

   inline void CalibrateCameraExt(MIL_ID MilCamCal, MIL_ID MilExtrinsicGridImage) const
      {
      pExtrinsicCal->CalibrateWithGrid(MilCamCal, MilExtrinsicGridImage, M_DISPLACE_CAMERA_COORD);
      }

   MIL_INT          CamLabel;
   const SGridCal*  pIntrinsicCal;
   const SGridCal*  pExtrinsicCal;
   };

//*****************************************************************************
// Laser calibration information structures.
//*****************************************************************************

// Laser calibration parameters.
struct SLaserCal
   {
   inline void SetCalPlane(MIL_ID Mil3dmapContext, MIL_INT PlaneIndex) const
      {
      M3dmapControl(Mil3dmapContext, M_DEFAULT, M_CORRECTED_DEPTH, pCorrectedDepths[PlaneIndex]);
      }

   MIL_INT           LaserLabel;
   MIL_INT           NbPlanes;
   const MIL_DOUBLE* pCorrectedDepths;
   };

// Types of extraction child.
enum ExtractionChildType
   {
   eChild = 0,    // The source is a child of the camera image.
   ePartialScan   // The source is the complete camera image, but represents a partial scan.
   };

// The extraction child parameters.
struct SExtractionChild
   {
   inline void SetupExtractionChild(MIL_ID Mil3dmapContext) const
      {
      M3dmapControl(Mil3dmapContext, M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_X, OffsetX);
      M3dmapControl(Mil3dmapContext, M_DEFAULT, M_EXTRACTION_CHILD_OFFSET_Y, OffsetY);
      }

   inline void AllocExtractionChild(MIL_ID MilCameraImage, MIL_ID* pMilExtractionImage) const
      {
      MIL_INT ChildX = ChildType == ePartialScan ? 0 : OffsetX;
      MIL_INT ChildY = ChildType == ePartialScan ? 0 : OffsetY;
      MbufChild2d(MilCameraImage, ChildX, ChildY, SizeX, SizeY, pMilExtractionImage);
      }

   ExtractionChildType ChildType;
   MIL_INT             OffsetX;
   MIL_INT             OffsetY;
   MIL_INT             SizeX;
   MIL_INT             SizeY;
   };

// The laser line extraction parameters.
struct SLaserLineExtraction
   {
   inline void SetupLaserLineExtraction(MIL_ID Mil3dmapContext) const
      {
      MIL_ID MilLocatePeakContext;
      M3dmapInquire(Mil3dmapContext, M_DEFAULT, M_LOCATE_PEAK_1D_CONTEXT_ID + M_TYPE_MIL_ID,
                    &MilLocatePeakContext);
      MimControl(MilLocatePeakContext, M_PEAK_WIDTH_NOMINAL, PeakWidthNominal);
      MimControl(MilLocatePeakContext, M_PEAK_WIDTH_DELTA, PeakWidthDelta);
      MimControl(MilLocatePeakContext, M_MINIMUM_CONTRAST, MinimumContrast);
      MimControl(MilLocatePeakContext, M_SCAN_LANE_DIRECTION, ScanLaneDirection);
      }

   MIL_INT PeakWidthNominal;
   MIL_INT PeakWidthDelta;
   MIL_INT MinimumContrast;
   MIL_INT ScanLaneDirection;
   };

//*****************************************************************************
// Single system structure.
//*****************************************************************************
struct SSingleSystemCal
   {
   SCameraCal           CamCal;
   SExtractionChild     ExtractionChild;
   SLaserLineExtraction LaserLineExtraction;
   SLaserCal            LaserCal;
   };

#endif
