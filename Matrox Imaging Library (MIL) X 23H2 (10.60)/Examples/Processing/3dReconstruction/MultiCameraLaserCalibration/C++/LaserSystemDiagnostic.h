//*****************************************************************************/
// 
// File name: LaserVisualDiagnostic.h
// 
// Synopsis:  This file contains the definition of the CLaserSysDiag 
//            class that is used to manage the display of the laser system
//            calibration.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef LASER_VISUAL_DIAGNOSTIC_H
#define LASER_VISUAL_DIAGNOSTIC_H

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_INT    M3D_Position_Y = 400;
static const MIL_DOUBLE BUTTON_SIZE = 64.0;
static const MIL_INT    LASER_NB_GRAPHIC = 4;
static const MIL_INT    BUTTON_NB_GRAPHIC = 2;
static const MIL_INT    SYS_NB_GRAPHIC = LASER_NB_GRAPHIC + BUTTON_NB_GRAPHIC;
static const MIL_INT    SYS_LABEL_SIZE = 10;

enum SysDispType
   {
   eSingle = 0,
   eMulti,
   eAll,
   eNbDispType
   };

static MIL_CONST_TEXT_PTR DISP_TYPE_TEXT[eNbDispType] = 
   {
   MIL_TEXT("Single"),
   MIL_TEXT("Multi"),
   MIL_TEXT("All")
   };

//*****************************************************************************
// CLaserSysDiag. Declaration of the class that manages the diagnostic
//                display of the example.
//*****************************************************************************
class CLaserSysDiag
   {
   public:
      // Constructor.
      CLaserSysDiag(MIL_ID MilSystem, MIL_INT NbSystem);
      
      // Destructor.
      virtual ~CLaserSysDiag();

      // Utility functions.
      void UpdateDisplayAndWait();
      void Update(MIL_INT Update);
      void Hide3dDisplay();
      void ClearAll(MIL_INT64 Label);

      // Diagnostic functions.
      void DiagnoseCamCal(MIL_ID MilGridImage, MIL_ID MilCal);
      void DiagnoseLaserLineExtraction(MIL_ID MilExtractionImage,  MIL_ID Mil3dmapLaserData);
      MIL_INT64 DiagnoseSingleCalibration(MIL_ID Mil3dmapContext, MIL_ID MilAllPlanesImage);
      void DiagnoseFullCalibration(const vector<MIL_ID>& Mil3dmapContext,
                                   const vector<MIL_ID>& MilAllPlanesImage);

      // Function to go in interactive mode.
      void StartInteractive(CLaserSysConfig& rCfg,
                            const vector<MIL_ID>& Mil3dmapContextSingle,
                            const vector<MIL_ID>& Mil3dmapContextMulti,
                            const vector<MIL_ID>& MilAllPlanesImage);
      void EndInteractive();

   private:

      // Diagnostic functions.
      void DiagnoseLaserCalibration(MIL_ID Mil3dmapContext, MIL_ID MilAllPlanesImage);

      // Interactive functions.
      static MIL_INT MFTYPE GraphicSelectedHook(MIL_INT HookType, MIL_ID MilEvent,
                                                void *pUserData);
      MIL_INT MFTYPE GraphicSelected(MIL_ID MilEvent);
      MIL_INT SelectedGraphicIndex() const;

      // Drawing functions.
      void DrawLaserCalibrationAnnotations(MIL_ID Mil3dmapContext);

      // General display members.
      MIL_INT m_NbSystem;
      MIL_ID m_MilDisplay;
      MIL_ID m_MilGraList;
      MIL_ID m_MilGraContext;
      MIL_INT m_SelectedSystemIndex;
      SysDispType m_DispType;

      MIL_ID m_MilDisp3d;
      MIL_ID m_MilGraphicList3d;

      // Interactive members.
      bool m_InteractiveStarted;
      vector<MIL_ID> m_Mil3dmapContextAll;
      vector<MIL_ID> m_MilAllPlanesImage;
      MIL_INT m_StartByDispType[eNbDispType];
      MIL_INT m_NbSystemByDispType[eNbDispType];
   };

//*****************************************************************************
// Constructor.
//*****************************************************************************
CLaserSysDiag::CLaserSysDiag(MIL_ID MilSystem, MIL_INT NbSystem)
   : m_InteractiveStarted(false),
     m_NbSystem(NbSystem),
     m_Mil3dmapContextAll(NbSystem*2),
     m_MilAllPlanesImage(NbSystem*2)
   {
   // Allocate objects for the display.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilDisplay);
   MdispControl(m_MilDisplay, M_CENTER_DISPLAY, M_DISABLE);
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);

   MgraAllocList(MilSystem, M_DEFAULT, &m_MilGraList);
   MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraList);

   MgraAlloc(MilSystem, &m_MilGraContext);

   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   m_MilDisp3d = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_NULL);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   if(m_MilDisp3d)
      {
      M3ddispControl(m_MilDisp3d, M_WINDOW_INITIAL_POSITION_Y, M3D_Position_Y);

      //Adjust the 3D display view. 
      M3ddispSetView(m_MilDisp3d, M_AZIM_ELEV_ROLL, 50, 180, 0, M_DEFAULT);

      MIL_UNIQUE_3DGEO_ID MatrixId = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoMatrixSetTransform(MatrixId, M_TRANSLATION, 0, 0, 100, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
      M3ddispInquire(m_MilDisp3d, M_3D_GRAPHIC_LIST_ID, &m_MilGraphicList3d);
      MIL_INT64 MilGrid = M3dgraGrid(m_MilGraphicList3d, M_ROOT_NODE, M_SIZE_AND_SPACING, MatrixId, 500, 500, 25, 25, M_DEFAULT);
      M3dgraControl(m_MilGraphicList3d, MilGrid, M_OPACITY, 30);
      M3dgraControl(m_MilGraphicList3d, M_DEFAULT_SETTINGS, M_FONT_SIZE, 15);
      }
   // Set the start and number of system for each type 3d display.
   m_StartByDispType[eSingle] = 0;
   m_StartByDispType[eMulti]  = NbSystem;
   m_StartByDispType[eAll]    = 0;
   m_NbSystemByDispType[eSingle] = 1;
   m_NbSystemByDispType[eMulti]  = NbSystem;
   m_NbSystemByDispType[eAll]    = NbSystem *2;

   m_SelectedSystemIndex = 0;
   m_DispType = eMulti;
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CLaserSysDiag::~CLaserSysDiag()
   {
   if(m_MilDisp3d)
      M3ddispFree(m_MilDisp3d);
   MgraFree(m_MilGraContext);
   MgraFree(m_MilGraList);
   MdispFree(m_MilDisplay);
   }

//*****************************************************************************
// UpdateDisplayAndWait. Updates the display and waits for the user to press a 
//                       Key.
//*****************************************************************************
void CLaserSysDiag::UpdateDisplayAndWait()
   {
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
   }

//*****************************************************************************
// Update. Set whether to enable or disable the updates.
//*****************************************************************************
void CLaserSysDiag::Update(MIL_INT Update)
   {
   MdispControl(m_MilDisplay, M_UPDATE, Update);
   }

//*****************************************************************************
// Hide3dDisplay. Hide the 3D display.
//*****************************************************************************
void CLaserSysDiag::Hide3dDisplay()
   {
   if(m_MilDisp3d)
      M3ddispSelect(m_MilDisp3d, M_NULL, M_CLOSE, M_DEFAULT);
   }
void CLaserSysDiag::ClearAll(MIL_INT64 Label)
   {
   if(!m_MilDisp3d)
      return;
   if(Label == 0)
      Label = M_ALL;
   M3dgraRemove(m_MilGraphicList3d, Label, M_DEFAULT);
   if(Label == M_ALL)
      {
      MIL_UNIQUE_3DGEO_ID MatrixId = M3dgeoAlloc(M_DEFAULT_HOST, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
      M3dgeoMatrixSetTransform(MatrixId, M_TRANSLATION, 0, 0, 100, M_DEFAULT, M_COMPOSE_WITH_CURRENT);
      M3ddispInquire(m_MilDisp3d, M_3D_GRAPHIC_LIST_ID, &m_MilGraphicList3d);
      MIL_INT64 MilGrid = M3dgraGrid(m_MilGraphicList3d, M_ROOT_NODE, M_SIZE_AND_SPACING, MatrixId, 500, 500, 25, 25, M_DEFAULT);
      M3dgraControl(m_MilGraphicList3d, MilGrid, M_OPACITY, 30);
      }
   }
//*****************************************************************************
// DiagnoseCamCal. Outputs the drawing of the calibration result as well as the
//                 calibration error.
//*****************************************************************************
void CLaserSysDiag::DiagnoseCamCal(MIL_ID MilGridImage, MIL_ID MilCal)
   {
   // Clear the previous annotations.
   MgraClear(M_DEFAULT, m_MilGraList);
   
   // Draw calibration points in green.
   MgraColor(m_MilGraContext, M_COLOR_GREEN);
   McalDraw(m_MilGraContext, MilCal, m_MilGraList,
            M_DRAW_IMAGE_POINTS, M_DEFAULT, M_DEFAULT);
   MosPrintf(MIL_TEXT("The calibration points extracted from the image are displayed\n")
             MIL_TEXT("in green.\n\n"));

   // Show some error information.
   MIL_DOUBLE AveragePixelError, MaximumPixelError, AverageWorldError, MaximumWorldError;
   McalInquire(MilCal, M_AVERAGE_PIXEL_ERROR, &AveragePixelError);
   McalInquire(MilCal, M_MAXIMUM_PIXEL_ERROR, &MaximumPixelError);
   McalInquire(MilCal, M_AVERAGE_WORLD_ERROR, &AverageWorldError);
   McalInquire(MilCal, M_MAXIMUM_WORLD_ERROR, &MaximumWorldError);

   // Draw coordinate system in gray.
   MgraColor(m_MilGraContext, M_COLOR_CYAN);
   McalDraw(m_MilGraContext, MilCal, m_MilGraList,
            M_DRAW_ABSOLUTE_COORDINATE_SYSTEM + M_DRAW_AXES, M_DEFAULT, M_DEFAULT);

   // Draw calibration points in red.
   MgraColor(m_MilGraContext, M_COLOR_RED);
   McalDraw(m_MilGraContext, MilCal, m_MilGraList, M_DRAW_WORLD_POINTS, M_DEFAULT, M_DEFAULT);
   
   // Select the grid image.
   MdispSelect(m_MilDisplay, MilGridImage);
   
   MosPrintf(MIL_TEXT("The calibration points, transformed using the calibration context,\n")
             MIL_TEXT("are displayed in red.\n\n"));
   MosPrintf(MIL_TEXT("Pixel error\n   average: %.3g pixels\n   maximum: %.3g pixels\n"),
             AveragePixelError, MaximumPixelError);
   MosPrintf(MIL_TEXT("World error\n   average: %.3g mm\n   maximum: %.3g mm\n\n"),
             AverageWorldError, MaximumWorldError);
   }

//*****************************************************************************
// DiagnoseLaserLineExtraction. Outputs the drawing of the laser line extraction.
//*****************************************************************************
void CLaserSysDiag::DiagnoseLaserLineExtraction(MIL_ID MilExtractionImage,
                                                MIL_ID Mil3dmapLaserData)
   {
   // Clear the previous annotations.
   MgraClear(M_DEFAULT, m_MilGraList);

   // Draw the extracted line.
   MgraColor(m_MilGraContext, M_COLOR_RED);
   M3dmapDraw(m_MilGraContext, Mil3dmapLaserData, m_MilGraList,
              M_DRAW_PEAKS_LAST, M_DEFAULT, M_DEFAULT);

   // Select the extraction image.
   MdispSelect(m_MilDisplay, MilExtractionImage);
   }

//*****************************************************************************
// DiagnoseFullCalibration. Diagnose the complete 3d system.
//*****************************************************************************
void CLaserSysDiag::DiagnoseFullCalibration(const vector<MIL_ID>& Mil3dmapContext,
                                            const vector<MIL_ID>& MilAllPlanesImage)
   {
   DiagnoseLaserCalibration(Mil3dmapContext[0], MilAllPlanesImage[0]);
   MIL_INT size = Mil3dmapContext.size();
   if(m_MilDisp3d)
      {
      for(MIL_INT i = 0; i < size; ++i)
         {
         M3dmapDraw3d(M_DEFAULT, Mil3dmapContext[i], M_DEFAULT, m_MilGraphicList3d, M_DEFAULT, MilAllPlanesImage[i], M_DEFAULT);
         }
      }
   }
//*****************************************************************************
// DiagnoseFullCalibration. Diagnose the 3d calibration of a single system.
//*****************************************************************************
MIL_INT64 CLaserSysDiag::DiagnoseSingleCalibration(MIL_ID Mil3dmapContext,
                                                   MIL_ID MilAllPlanesImage)
   {
   MIL_INT64 ParentLabel = 0;
   DiagnoseLaserCalibration(Mil3dmapContext, MilAllPlanesImage);

   if(m_MilDisp3d)
      {
      ParentLabel = M3dmapDraw3d(M_DEFAULT, Mil3dmapContext, M_DEFAULT, m_MilGraphicList3d, M_DEFAULT, MilAllPlanesImage, M_DEFAULT);
      M3ddispSelect(m_MilDisp3d, M_NULL, M_OPEN, M_DEFAULT);
      }
   return ParentLabel;
   }

//*****************************************************************************
// DiagnoseFullCalibration. Outputs the drawings associated to a laser calibration.
//*****************************************************************************
void CLaserSysDiag::DiagnoseLaserCalibration(MIL_ID Mil3dmapContext,
                                             MIL_ID MilAllPlanesImage)
   {
   // Clear the previous annotations.
   MgraClear(M_DEFAULT, m_MilGraList);

   // Draw the diagnostic annotations.
   DrawLaserCalibrationAnnotations(Mil3dmapContext);

   // Select the all planes image.
   MdispSelect(m_MilDisplay, MilAllPlanesImage);

   MosPrintf(MIL_TEXT("The laser plane has been fitted on the extracted laser line(s).\n")
      MIL_TEXT("   Green: extracted laser line(s).\n")
      MIL_TEXT("   Red:   expected line(s) on the fitted laser plane.\n\n"));

   // Print fit RMS error.
   MIL_DOUBLE FitRMSError;
   M3dmapInquire(Mil3dmapContext, M_DEFAULT, M_FIT_RMS_ERROR, &FitRMSError);
   MosPrintf(MIL_TEXT("Fit RMS error: %.3g mm\n\n"), FitRMSError);
   }

//*****************************************************************************
// DiagnoseFullCalibration. Draws the annotations to diagnose a laser system 
//                          calibration.
//*****************************************************************************
void CLaserSysDiag::DrawLaserCalibrationAnnotations(MIL_ID Mil3dmapContext)
   {
   // Show fitted lines in red.
   MgraColor(m_MilGraContext, M_COLOR_RED);
   M3dmapDraw(m_MilGraContext, Mil3dmapContext, m_MilGraList,
              M_DRAW_CALIBRATION_LINES, M_DEFAULT, M_DEFAULT);

   // Show all extracted laser lines in green.
   MgraColor(m_MilGraContext, M_COLOR_GREEN);
   M3dmapDraw(m_MilGraContext, Mil3dmapContext, m_MilGraList,
              M_DRAW_CALIBRATION_PEAKS, M_DEFAULT, M_DEFAULT);
   }

//*****************************************************************************
// StartInteractive. Start the diagnostic display interactivity that lets the
//                   user compare the calibrations of single, multiple, and
//                   all systems.
//*****************************************************************************
void CLaserSysDiag::StartInteractive(CLaserSysConfig& rCfg,
                                     const vector<MIL_ID>& Mil3dmapContextSingle,
                                     const vector<MIL_ID>& Mil3dmapContextMulti,
                                     const vector<MIL_ID>& MilAllPlanesImage)
   {
   if(!m_InteractiveStarted)
      {
      m_InteractiveStarted = true;

      // Get a copy of the systems.
      for(MIL_INT s = 0; s < m_NbSystem; s++)
         {
         m_Mil3dmapContextAll[s] = Mil3dmapContextSingle[s];
         m_MilAllPlanesImage[s] = MilAllPlanesImage[s];
         m_Mil3dmapContextAll[m_NbSystem + s] = Mil3dmapContextMulti[s];
         m_MilAllPlanesImage[m_NbSystem + s] = MilAllPlanesImage[s];
         }

      // Draw the annotations of the system.
      MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
      MgraClear(M_DEFAULT, m_MilGraList);
      MgraControl(m_MilGraContext, M_BACKGROUND_MODE, M_TRANSPARENT);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_VERTICAL, M_CENTER);
      MgraControl(m_MilGraContext, M_SELECTABLE, M_DISABLE);
      for(MIL_INT s = 0; s < m_NbSystem; s++)
         {         
         DrawLaserCalibrationAnnotations(m_Mil3dmapContextAll[s]);
         DrawLaserCalibrationAnnotations(m_Mil3dmapContextAll[m_NbSystem + s]);
         }

      MgraControl(m_MilGraContext, M_SELECTABLE, M_ENABLE);
      MgraControl(m_MilGraContext, M_INPUT_UNITS, M_DISPLAY);
      MIL_TEXT_CHAR SystemText[SYS_LABEL_SIZE];
      for(MIL_INT s = 0; s < m_NbSystem; s++)
         {
         // Draw the buttons of the system.
         const SSingleSystemCal& rSys = rCfg.System(s);
         MgraColor(m_MilGraContext, M_COLOR_DARK_RED);
         MgraRectFill(m_MilGraContext, m_MilGraList,
                      (s+1)*BUTTON_SIZE, 0, (s+2)*BUTTON_SIZE, BUTTON_SIZE);
         MgraColor(m_MilGraContext, M_COLOR_WHITE);
         MosSprintf(SystemText, SYS_LABEL_SIZE, MIL_TEXT("C%iL%i"),
                    rSys.CamCal.CamLabel, rSys.LaserCal.LaserLabel);
         MgraText(m_MilGraContext, m_MilGraList,
                  (s+1.5)*BUTTON_SIZE, 0.5*BUTTON_SIZE, SystemText);
         }

      // Draw the mode button of the system.
      MgraColor(m_MilGraContext, M_COLOR_WHITE);
      MgraRectFill(m_MilGraContext, m_MilGraList, 0, 0, BUTTON_SIZE, BUTTON_SIZE);
      MgraColor(m_MilGraContext, M_COLOR_BLACK);
      MgraText(m_MilGraContext, m_MilGraList,
               0.5 * BUTTON_SIZE, 0.5 * BUTTON_SIZE, MIL_TEXT("Multi"));

      // Set the section modified hook.
      MgraHookFunction(m_MilGraList, M_GRAPHIC_SELECTION_MODIFIED,
                       CLaserSysDiag::GraphicSelectedHook, this);
      MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);

      // Set the selection on the first multi system.
      MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(m_NbSystem*4), M_DEFAULT,
                      M_GRAPHIC_SELECTED, M_TRUE);

       MosPrintf(MIL_TEXT("Interaction is now possible with the display to visualize the\n")
         MIL_TEXT("calibrations results:\n\n")
         MIL_TEXT("  - Click on the first button to change the result display mode:\n")
         MIL_TEXT("    - Single: The calibration of the single system whose view is\n")
         MIL_TEXT("              selected is displayed alone.\n")
         MIL_TEXT("    - Multi:  The calibration of all systems together is displayed.\n")
         MIL_TEXT("    - All:    The calibrations of all single systems, and of all\n")
         MIL_TEXT("              systems together, are displayed.\n\n")
         MIL_TEXT("  - Click on a system view selection button to change which\n ")
         MIL_TEXT("    system's calibration peaks and lines are displayed.\n\n"));
      }
   }
//*****************************************************************************
// EndInteractive. End the interactive mode.
//*****************************************************************************
void CLaserSysDiag::EndInteractive()
   {
   MgraHookFunction(m_MilGraList, M_GRAPHIC_SELECTION_MODIFIED + M_UNHOOK,
                    CLaserSysDiag::GraphicSelectedHook, this);
   m_InteractiveStarted = false;
   }

//*****************************************************************************
// SelectedGraphicIndex. Get the graphic index of the currently selected button.
//*****************************************************************************
MIL_INT CLaserSysDiag::SelectedGraphicIndex() const 
   {
   return m_NbSystem * LASER_NB_GRAPHIC + m_SelectedSystemIndex * BUTTON_NB_GRAPHIC;
   }

//*****************************************************************************
// GraphicSelected. Graphic list callback to select the system view in the display.
//*****************************************************************************
MIL_INT MFTYPE CLaserSysDiag::GraphicSelectedHook(MIL_INT HookType,
                                                  MIL_ID MilEvent,
                                                  void *pUserData)
   {
   CLaserSysDiag* pLaserSystemDiag = (CLaserSysDiag*) pUserData;
   return pLaserSystemDiag->GraphicSelected(MilEvent);
   }

MIL_INT CLaserSysDiag::GraphicSelected(MIL_ID MilEvent)
   {
   // Disable the display updates.
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);

   // Deselect the selected element.
   MgraControlList(m_MilGraList, M_ALL, M_DEFAULT, M_GRAPHIC_SELECTED, M_FALSE);

   // Get the label of the selected graphic.
   MIL_INT GraphicLabel;
   MgraGetHookInfo(MilEvent, M_GRAPHIC_LABEL_VALUE, &GraphicLabel);

   // If a graphic was selected.
   if(GraphicLabel != M_NO_LABEL)
      {
      // Get the position X of the button to get the index of the clicked system.
      MIL_INT PosX = MgraInquireList(m_MilGraList, M_GRAPHIC_LABEL(GraphicLabel), M_DEFAULT,
                     M_POSITION_X, M_NULL);
      MIL_INT ButtonIndex = (MIL_INT)(PosX / BUTTON_SIZE);

      // If changing the display type.
      if(ButtonIndex == 0)
         {
         // Change the display type.
         m_DispType = (SysDispType)(((MIL_INT)m_DispType + 1) % eNbDispType);
         MIL_INT DispTypeLabelIndex = SYS_NB_GRAPHIC * m_NbSystem + 1;
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(DispTypeLabelIndex), M_DEFAULT,
                         M_DELETE, M_DEFAULT);
         MgraColor(m_MilGraContext, M_COLOR_BLACK);
         MgraText(m_MilGraContext, m_MilGraList,
                  0.5 * BUTTON_SIZE, 0.5 * BUTTON_SIZE, DISP_TYPE_TEXT[m_DispType]);
         }

      if(ButtonIndex <= m_NbSystem)
         {
         // Reset the current button.
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(SelectedGraphicIndex()), M_DEFAULT,
                         M_COLOR, M_COLOR_DARK_RED);

         // Set the new button.
         m_SelectedSystemIndex = ButtonIndex == 0 ? m_SelectedSystemIndex : ButtonIndex-1;
         MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(SelectedGraphicIndex()), M_DEFAULT,
                         M_COLOR, M_COLOR_GREEN);

         // Make the correct drawings visible.
         for(MIL_INT s = 0; s < m_NbSystem; s++)
            {
            bool Visible = (s == m_SelectedSystemIndex);
            MIL_INT DrawingStartIndex = s * LASER_NB_GRAPHIC;
            
            MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(DrawingStartIndex)    , M_DEFAULT,
                            M_VISIBLE, (Visible && m_DispType != eMulti) ? M_TRUE : M_FALSE);
            MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(DrawingStartIndex + 1), M_DEFAULT,
                            M_VISIBLE, (Visible && m_DispType != eMulti) ? M_TRUE : M_FALSE);

            MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(DrawingStartIndex + 2), M_DEFAULT,
                            M_VISIBLE, (Visible && m_DispType == eMulti) ? M_TRUE : M_FALSE);
            MgraControlList(m_MilGraList, M_GRAPHIC_INDEX(DrawingStartIndex + 3), M_DEFAULT,
                            M_VISIBLE, (Visible && m_DispType == eMulti) ? M_TRUE : M_FALSE);
            }
         }

      // Set the 3d systems.
      MIL_INT AllStartIndex = (m_DispType == eSingle) ? m_SelectedSystemIndex 
                                                      : m_StartByDispType[m_DispType];
      if(m_MilDisp3d)
         {
         MIL_INT64 DrawLabel = 0;
         ClearAll(DrawLabel);
         
         if(m_DispType == eSingle)
            DrawLabel = M3dmapDraw3d(M_DEFAULT, m_Mil3dmapContextAll[AllStartIndex], M_DEFAULT, m_MilGraphicList3d, M_DEFAULT, m_MilAllPlanesImage[AllStartIndex], M_DEFAULT);
         else
            {
            DrawLabel = M_ALL;
            for(MIL_INT i = 0; i < m_NbSystemByDispType[m_DispType]; ++i)
               {
               M3dmapDraw3d(M_DEFAULT, m_Mil3dmapContextAll[AllStartIndex + i], M_DEFAULT, m_MilGraphicList3d, M_DEFAULT, m_MilAllPlanesImage[AllStartIndex + i], M_DEFAULT);
               }
            }
         M3ddispSelect(m_MilDisp3d, M_NULL, M_OPEN, M_DEFAULT);
         // Set the selected system.
         MdispSelect(m_MilDisplay, m_MilAllPlanesImage[m_SelectedSystemIndex]);
         MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
         }
      }
   return 0;
   }

#endif
