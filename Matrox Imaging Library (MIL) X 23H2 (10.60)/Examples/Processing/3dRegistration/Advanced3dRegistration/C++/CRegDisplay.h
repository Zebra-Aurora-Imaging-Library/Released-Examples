//*******************************************************************************
// 
// File name: CRegDisplay.h
//
// Synopsis:  Class in charge of displaying registration iterations.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************

#include <mil.h>
#include <atomic>

//-------------------------------------------------------------------------------
// 
// Class CCameraOrientation
//
// Utility class that holds camera orientation information.
//-------------------------------------------------------------------------------
struct CCameraOrientation
   {
   public:
      CCameraOrientation(MIL_ID MilDisplay)
         {
         M3ddispGetView(MilDisplay, M_AZIM_ELEV_ROLL, &m_Azimuth, &m_Elevation, &m_Roll, M_DEFAULT);
         }

      CCameraOrientation(MIL_DOUBLE Azimuth,
                        MIL_DOUBLE Elevation,
                        MIL_DOUBLE Roll):
         m_Azimuth(Azimuth),
         m_Elevation(Elevation),
         m_Roll(Roll)
         {}

      void ApplyToDisplay(MIL_ID MilDisplay) const
         {
         M3ddispSetView(MilDisplay, M_AZIM_ELEV_ROLL, m_Azimuth, m_Elevation, m_Roll, M_DEFAULT);
         }

      MIL_DOUBLE m_Azimuth;
      MIL_DOUBLE m_Elevation;
      MIL_DOUBLE m_Roll;
   };

//-------------------------------------------------------------------------------
// 
// Class CCameraParameters
//
// Utility class that holds the camera view matrix.
//-------------------------------------------------------------------------------
struct CCameraParameters
   {
   public:
      CCameraParameters(MIL_ID MilSystem, MIL_ID MilDisplay)
         {
         // The current view.
         m_CameraMatrix = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
         M3ddispCopy(MilDisplay, m_CameraMatrix, M_VIEW_MATRIX, M_DEFAULT);
         }

      void ApplyToDisplay(MIL_ID MilDisplay) const
         {
         M3ddispSetView(MilDisplay, M_VIEW_MATRIX, m_CameraMatrix, M_DEFAULT, M_DEFAULT, M_DEFAULT);
         }

      MIL_UNIQUE_3DGEO_ID m_CameraMatrix;
   };

//-------------------------------------------------------------------------------
// 
// Class CWindowParameters
//
// Utility class that holds window parameters.
//-------------------------------------------------------------------------------
struct CWindowParameters
   {
   public:
      CWindowParameters(MIL_STRING Title, MIL_INT PosX, MIL_INT PosY, MIL_INT SizeX, MIL_INT SizeY):
         m_Title(Title),
         m_PositionX(PosX),
         m_PositionY(PosY),
         m_SizeX(SizeX),
         m_SizeY(SizeY)
         {
         }

      void ApplyToDisplay(MIL_ID MilDisplay) const
         {
         M3ddispControl(MilDisplay, M_TITLE, m_Title.c_str());
         M3ddispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, m_PositionX);
         M3ddispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, m_PositionY);
         M3ddispControl(MilDisplay, M_SIZE_X, m_SizeX);
         M3ddispControl(MilDisplay, M_SIZE_Y, m_SizeY);
         }

      MIL_STRING m_Title;
      MIL_INT m_PositionX;
      MIL_INT m_PositionY;
      MIL_INT m_SizeX;
      MIL_INT m_SizeY;
   };

//-------------------------------------------------------------------------------
// 
// Class CRegDisplay
//
// Utility class that visualizes 3D registration steps.
//-------------------------------------------------------------------------------
class CRegDisplay
   {
   public:
      enum class VisualizationMode{RUN, SINGLE};

      CRegDisplay(MIL_ID MilRefContainer, MIL_ID MilTargetContainer, MIL_ID MilContext,
                  const CWindowParameters& WindowParams,
                  const CCameraParameters& CameraParameters);
      ~CRegDisplay();

      CRegDisplay(const CRegDisplay&) = delete;
      CRegDisplay& operator=(const CRegDisplay&) = delete;

      void End();

      void ShowNextStep();
      void ShowPreviousStep();
      void Run();
      MIL_ID GetMilDisplayID();

   private:
      static MIL_UINT32 MFTYPE ProcessDisplayThread(void* pUserData);

      MIL_INT m_LoopIteration = 0;     //Iteration counter for the loop mode.
      MIL_INT m_TargetIteration = 0;   //Iteration counter for the single mode.
      VisualizationMode m_Mode = VisualizationMode::RUN;

      std::atomic<bool> m_Running = {false};
      MIL_UNIQUE_THR_ID m_MilDisplayThread;
      MIL_UNIQUE_3DREG_ID m_MilRegResult;
      MIL_UNIQUE_3DREG_ID m_MilDrawContext;
      MIL_UNIQUE_3DDISP_ID m_MilDisplay;
      MIL_INT m_NumIterations=0;
   };


//-------------------------------------------------------------------------------
// 
// Class CDisplayController
//
// Utility class that provides keyboard controls to the displays.
//-------------------------------------------------------------------------------
class CDisplayController
   {
   public:

      CDisplayController();
      ~CDisplayController();

      CDisplayController(const CDisplayController&) = delete;
      CDisplayController& operator=(const CRegDisplay&) = delete;

      void RegisterDisplay(CRegDisplay* Display);

      void Start(bool IsFinalDisplay);
      void End();

   private:
      static MIL_UINT32 MFTYPE ProcessControlThread(void* pUserData);

      std::vector<CRegDisplay*> m_RegisteredDisplay;
      std::atomic<bool> m_Running = {false};
      MIL_UNIQUE_THR_ID m_MilControlThread;
      
   };


