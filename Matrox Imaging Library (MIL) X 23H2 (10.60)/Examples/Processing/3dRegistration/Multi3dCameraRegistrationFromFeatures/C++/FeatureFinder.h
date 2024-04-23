//***************************************************************************************/
//
// File name: FeatureFinder.h
//
// Synopsis:  Implementation of the different feature finders to do the 3d registration
//            from feature.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

//*****************************************************************************
// The definition of the IFeatureFinder interface class.
//*****************************************************************************
class IFeatureFinder
   {
   public:
      IFeatureFinder() {};
      virtual bool FindFeatures(MIL_ID MilPointCloud, std::vector<MIL_DOUBLE>* pImagePointsX, std::vector<MIL_DOUBLE>* pImagePointsY) = 0;
   };

//*****************************************************************************
// The definition of the CGridFeatureFinder class.
//*****************************************************************************
class CGridFeatureFinder: public IFeatureFinder
   {
   public:
      CGridFeatureFinder(MIL_ID MilSystem)
         {
         m_MilCalContext = McalAlloc(MilSystem, M_LINEAR_INTERPOLATION, M_DEFAULT, M_UNIQUE_ID);
         McalControl(m_MilCalContext, M_GRID_FIDUCIAL, M_DATAMATRIX);
         McalControl(m_MilCalContext, M_GRID_PARTIAL, M_ENABLE);
         }

      virtual bool FindFeatures(MIL_ID MilPointCloud, std::vector<MIL_DOUBLE>* pImagePointsX, std::vector<MIL_DOUBLE>* pImagePointsY)
         {
         // Locate the grid in the reflectance.
         MIL_ID Reflectance = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);
         McalGrid(m_MilCalContext, Reflectance, 0, 0, 0, M_UNKNOWN, M_UNKNOWN, M_FROM_FIDUCIAL, M_FROM_FIDUCIAL, M_DEFAULT, M_CHESSBOARD_GRID);
         McalAssociate(M_NULL, Reflectance, M_DEFAULT);

         MIL_INT Status = McalInquire(m_MilCalContext, M_CALIBRATION_STATUS, M_NULL);
         if(Status == M_CALIBRATED)
            {
            // Get the grid's image points.
            McalInquire(m_MilCalContext, M_CALIBRATION_IMAGE_POINTS_X, *pImagePointsX);
            McalInquire(m_MilCalContext, M_CALIBRATION_IMAGE_POINTS_Y, *pImagePointsY);

            return true;
            }
         else
            return false;
         }

   private:
      MIL_UNIQUE_CAL_ID m_MilCalContext;
   };

//*****************************************************************************
// The definition of the CDataMatrixFeatureFinder class.
//*****************************************************************************
class CDatamatrixFeatureFinder: public IFeatureFinder
   {
   public:
      CDatamatrixFeatureFinder(MIL_ID MilSystem)
         {
         // Allocate a code reader context and result buffer.
         m_MilCodeContext = McodeAlloc(MilSystem, M_DEFAULT, M_IMPROVED_RECOGNITION, M_UNIQUE_ID);
         m_MilCodeResult = McodeAllocResult(MilSystem, M_DEFAULT, M_UNIQUE_ID);

         // Add a data matrix model to the context.
         McodeModel(m_MilCodeContext, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, M_NULL);
         }

      virtual bool FindFeatures(MIL_ID MilPointCloud, std::vector<MIL_DOUBLE>* pImagePointsX, std::vector<MIL_DOUBLE>* pImagePointsY)
         {
         // Locate the data matrix in the reflectance.
         MIL_ID Reflectance = MbufInquireContainer(MilPointCloud, M_COMPONENT_REFLECTANCE, M_COMPONENT_ID, M_NULL);
         auto MilReflectanceGray = MbufChildColor(Reflectance, M_RED, M_UNIQUE_ID);
         McodeRead(m_MilCodeContext, MilReflectanceGray, m_MilCodeResult);

         MIL_INT ReadStatus = 0;
         McodeGetResult(m_MilCodeResult, M_GENERAL, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &ReadStatus);
         if(ReadStatus == M_STATUS_READ_OK)
            {
            // Get the corners of the code.
            const MIL_INT CORNER_X[4] = {M_TOP_LEFT_X, M_TOP_RIGHT_X,
                                         M_BOTTOM_RIGHT_X, M_BOTTOM_LEFT_X};
            const MIL_INT CORNER_Y[4] = {M_TOP_LEFT_Y, M_TOP_RIGHT_Y,
                                         M_BOTTOM_RIGHT_Y, M_BOTTOM_LEFT_Y};

            for(MIL_INT c = 0; c < 4; c++)
               {
               McodeGetResult(m_MilCodeResult, 0, M_DEFAULT, CORNER_X[c], &(*pImagePointsX)[c]);
               McodeGetResult(m_MilCodeResult, 0, M_DEFAULT, CORNER_Y[c], &(*pImagePointsY)[c]);
               }
            return true;
            }
         else
            return false;
         }

   private:
      MIL_UNIQUE_CODE_ID m_MilCodeContext;
      MIL_UNIQUE_CODE_ID m_MilCodeResult;
   };
