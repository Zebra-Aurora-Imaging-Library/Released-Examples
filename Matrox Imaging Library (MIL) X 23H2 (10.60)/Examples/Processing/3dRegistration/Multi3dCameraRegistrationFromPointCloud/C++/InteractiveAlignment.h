//***************************************************************************************
//
// File name: InteractiveAlignment.h
//
// Synopsis:  Utility header that contains the functions to do a rough alignment
//            of the data from two point clouds using depth maps. The alignment is done
//            by providing interactively two 3d vectors in displays of the depth maps.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************

#pragma once

//****************************************************************************
// Structure representing an axis with a position and direction.
//****************************************************************************
struct SAxis
   {
   SAxis(MIL_DOUBLE X0, MIL_DOUBLE Y0, MIL_DOUBLE Z0, MIL_DOUBLE X1, MIL_DOUBLE Y1, MIL_DOUBLE Z1)
      {
      Vx = X1 - X0;
      Vy = Y1 - Y0;
      Vz = Z1 - Z0;
      MIL_DOUBLE Length = sqrt(Vx * Vx + Vy * Vy + Vz * Vz);
      Vx /= Length;
      Vy /= Length;
      Vz /= Length;
      X = 0.5*(X0 + X1);
      Y = 0.5*(Y0 + Y1);
      Z = 0.5*(Z0 + Z1);
      }

   MIL_DOUBLE X;
   MIL_DOUBLE Y;
   MIL_DOUBLE Z;
   MIL_DOUBLE Vx;
   MIL_DOUBLE Vy;
   MIL_DOUBLE Vz;
   };

//****************************************************************************
// Structure representing a 2d segment.
//****************************************************************************
struct SSegment
   {
   MIL_DOUBLE X0;
   MIL_DOUBLE Y0;
   MIL_DOUBLE X1;
   MIL_DOUBLE Y1;
   };

extern const SSegment EXAMPLE_ALIGN_DISPLAY_INIT[];


//****************************************************************************
// Axis display
//****************************************************************************
const MIL_INT MAX_AXIS_DISPLAY_SIZE_X = 640;
class CAxisDisplay
   {
   public:

      CAxisDisplay(MIL_ID MilPointCloud, MIL_INT WindowPosition, MIL_DOUBLE WindowZoom = 1.0, MIL_INT RefSegmentIndex = 0) : m_WindowZoom(WindowZoom)
         {
         // Generate the depth maps of the point clouds.
         m_MilDepthMap = GenerateDepthMap(MilPointCloud);
         MIL_INT SizeX = MbufInquire(m_MilDepthMap, M_SIZE_X, M_NULL);
         MIL_INT SizeY = MbufInquire(m_MilDepthMap, M_SIZE_Y, M_NULL);

         // Allocate the axis displays.
         m_MilDisplay = MdispAlloc(M_DEFAULT_HOST, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
         m_MilGraList = MgraAllocList(M_DEFAULT_HOST, M_DEFAULT, M_UNIQUE_ID);
         MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraList);
         MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
         MdispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_X, WindowPosition);

         // Sets the window initial size x.
         m_WindowInitialSizeX = (MIL_INT)(SizeX * m_WindowZoom);
         if(m_WindowInitialSizeX > MAX_AXIS_DISPLAY_SIZE_X)
            {
            m_WindowInitialSizeX = MAX_AXIS_DISPLAY_SIZE_X;
            m_WindowZoom = (MIL_DOUBLE)m_WindowInitialSizeX / SizeX;
            MdispZoom(m_MilDisplay, m_WindowZoom, m_WindowZoom);
            }

         // Select the depth map.
         MdispSelect(m_MilDisplay, m_MilDepthMap);

         // Put the line in the middle of the depth map.
         MgraLine(M_DEFAULT, m_MilGraList, (SizeX - 1) * 0.25, (SizeY - 1) * 0.5, (SizeX - 1) * 0.75, (SizeY - 1) * 0.5);
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_DRAW_DIRECTION, M_PRIMARY_DIRECTION);

         // Set the color map LUT.
         m_MilColorMapLut = MbufAllocColor(M_DEFAULT_HOST, 3, 65535, 1, 8 + M_UNSIGNED, M_LUT, M_UNIQUE_ID);
         MgenLutFunction(m_MilColorMapLut, M_COLORMAP_TURBO + M_LAST_GRAY, M_DEFAULT, M_RGB888(128, 128, 128), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         MdispLut(m_MilDisplay, m_MilColorMapLut);

         // Set the reference segment. Only true is the source if the default example samples.
         if(RefSegmentIndex >= 0)
            SetLine(EXAMPLE_ALIGN_DISPLAY_INIT[RefSegmentIndex]);
         CheckValidSegment();

         MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
         }

      void SetLine(MIL_DOUBLE X0, MIL_DOUBLE Y0, MIL_DOUBLE X1, MIL_DOUBLE Y1)
         {
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), 0, M_POSITION_X, X0);
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), 0, M_POSITION_Y, Y0);
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), 1, M_POSITION_X, X1);
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), 1, M_POSITION_Y, Y1);
         }

      void SetLine(const SSegment& Segment)
         {
         SetLine(Segment.X0, Segment.Y0, Segment.X1, Segment.Y1);
         }

      void StartInteractivity()
         {
         MgraHookFunction(m_MilGraList, M_GRAPHIC_MODIFIED, CheckValidSegmentHook, this);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);
         }

      void StopInteractivity()
         {
         MgraHookFunction(m_MilGraList, M_GRAPHIC_MODIFIED + M_UNHOOK, CheckValidSegmentHook, this);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_DISABLE);
         }

      SAxis GetAxis() const
         {
         MIL_DOUBLE OriginX;
         MIL_DOUBLE OriginY;
         MIL_DOUBLE OriginZ;
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), 0, M_POSITION_X, &OriginX);
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), 0, M_POSITION_Y, &OriginY);

         McalTransformCoordinate3dList(m_MilDepthMap, M_PIXEL_COORDINATE_SYSTEM, M_RELATIVE_COORDINATE_SYSTEM, 1, &OriginX, &OriginY, M_NULL, &OriginX, &OriginY, &OriginZ, M_DEPTH_MAP);

         MIL_DOUBLE EndX;
         MIL_DOUBLE EndY;
         MIL_DOUBLE EndZ;
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), 1, M_POSITION_X, &EndX);
         MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), 1, M_POSITION_Y, &EndY);

         McalTransformCoordinate3dList(m_MilDepthMap, M_PIXEL_COORDINATE_SYSTEM, M_RELATIVE_COORDINATE_SYSTEM, 1, &EndX, &EndY, M_NULL, &EndX, &EndY, &EndZ, M_DEPTH_MAP);
         return { OriginX, OriginY, OriginZ, EndX, EndY, EndZ };
         }

      bool CheckValidSegment()
         {
         for (MIL_INT p = 0; p < 2; p++)
            {
            MIL_DOUBLE X;
            MIL_DOUBLE Y;
            MIL_DOUBLE Z;
            MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), p, M_POSITION_X, &X);
            MgraInquireList(m_MilGraList, M_GRAPHIC_INDEX(0), p, M_POSITION_Y, &Y);

            McalTransformCoordinate3dList(m_MilDepthMap, M_PIXEL_COORDINATE_SYSTEM, M_RELATIVE_COORDINATE_SYSTEM, 1, &X, &Y, M_NULL, &X, &Y, &Z, M_DEPTH_MAP);
            if (X == M_INVALID_POINT)
               {
               MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_COLOR, M_COLOR_BLACK);
               MgraControlList(m_MilGraList, M_LIST, M_DEFAULT, M_SELECTED_COLOR, M_COLOR_BLACK);
               return false;
               }
            }
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(0), M_DEFAULT, M_COLOR, M_COLOR_WHITE);
         MgraControlList(m_MilGraList, M_LIST, M_DEFAULT, M_SELECTED_COLOR, M_COLOR_WHITE);
         return true;
         }

      MIL_DOUBLE WindowZoom() const
         {
         return m_WindowZoom;
         }

      MIL_INT WindowInitialSizeX() const
         {
         return m_WindowInitialSizeX;
         }
   private:
      static MIL_INT MFTYPE CheckValidSegmentHook(MIL_INT, MIL_ID, void *pUserData)
         {
         ((CAxisDisplay*)pUserData)->CheckValidSegment();
         return 0;
         }

      MIL_UNIQUE_BUF_ID m_MilColorMapLut;
      MIL_UNIQUE_BUF_ID m_MilDepthMap;
      MIL_UNIQUE_DISP_ID m_MilDisplay;
      MIL_UNIQUE_GRA_ID m_MilGraList;
      MIL_INT m_WindowInitialSizeX;
      MIL_DOUBLE m_WindowZoom;
   };

//****************************************************************************
// Aligns the two point clouds using axis displays.
//****************************************************************************
MIL_UNIQUE_3DGEO_ID AlignDepthMapPair(MIL_ID MilRefPointCloud, MIL_ID MilToAlignPointCloud, MIL_INT RefSegmentIndex, MIL_INT AlignSegmentIndex)
   {
   // Setup the alignment display.
   CAxisDisplay RefDisplay(MilRefPointCloud, 0, 1.0, RefSegmentIndex);
   CAxisDisplay AlignDisplay(MilToAlignPointCloud, RefDisplay.WindowInitialSizeX(), RefDisplay.WindowZoom(), AlignSegmentIndex);
  
   RefDisplay.StartInteractivity();
   AlignDisplay.StartInteractivity();

   bool ValidAxis = true;
   do
      {
      MosPrintf(MIL_TEXT("Action required:\n"));
      MosPrintf(MIL_TEXT("Use the interactive display to provide a\n"));
      MosPrintf(MIL_TEXT("pre-alignment hint between the point clouds.\n"));
      MosPrintf(MIL_TEXT("Align of the extremities of the arrows onto\n"));
      MosPrintf(MIL_TEXT("common valid data points in each display.\n"));
      MosPrintf(MIL_TEXT("Press <Enter> when done.\n\n"));
      MosGetch();

      ValidAxis = RefDisplay.CheckValidSegment() && AlignDisplay.CheckValidSegment();
      } while (!ValidAxis);

   RefDisplay.StopInteractivity();
   AlignDisplay.StopInteractivity();

   // Get the two axis.
   auto RefAxis = RefDisplay.GetAxis();
   auto AlignAxis = AlignDisplay.GetAxis();

   // Compute the rotation matrix as a rotation around the Z axis and rotation with an axis perpendicular to the reference axis.
   MIL_DOUBLE Rz = atan2(AlignAxis.Vx*RefAxis.Vy - AlignAxis.Vy * RefAxis.Vx, AlignAxis.Vx*RefAxis.Vx + AlignAxis.Vy * RefAxis.Vy) * DIV_180_PI;
   MIL_DOUBLE RAngle = (asin(RefAxis.Vz) - asin(AlignAxis.Vz)) * DIV_180_PI;
   MIL_DOUBLE RAxisVz = 0;
   MIL_DOUBLE RAxisVx = RefAxis.Vy;
   MIL_DOUBLE RAxisVy = -RefAxis.Vx;
   auto MilTransformationMatrix = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixSetTransform(MilTransformationMatrix, M_TRANSLATION, -AlignAxis.X, -AlignAxis.Y, -AlignAxis.Z, M_DEFAULT, M_ASSIGN);
   M3dgeoMatrixSetTransform(MilTransformationMatrix, M_ROTATION_Z, Rz, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
   M3dgeoMatrixSetTransform(MilTransformationMatrix, M_ROTATION_AXIS_ANGLE, RAxisVx, RAxisVy, RAxisVz, RAngle, M_COMPOSE_WITH_CURRENT);
   M3dgeoMatrixSetTransform(MilTransformationMatrix, M_TRANSLATION, RefAxis.X, RefAxis.Y, RefAxis.Z, M_DEFAULT, M_COMPOSE_WITH_CURRENT);

   return MilTransformationMatrix;
   }
