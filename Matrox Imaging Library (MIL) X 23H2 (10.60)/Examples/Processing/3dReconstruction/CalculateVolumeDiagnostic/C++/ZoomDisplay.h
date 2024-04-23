//***************************************************************************************/
//
// File name: ZoomDisplay.h
//
// Synopsis:  Declaration of CZoomDisplay class used to display an image, overlaying
//            a zoomed portion around the mouse cursor location.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#ifndef ZOOM_DISPLAY_H
#define ZOOM_DISPLAY_H

// Structure that holds the zoom overlay window.
struct SZoomWindow
   {
   MIL_INT   CenterElementIndex() const { return Values.size() / 2; }
   MIL_UINT8 CenterElementValue() const { return Values[CenterElementIndex()]; }   

   std::vector<MIL_UINT8> Values;
   MIL_INT PosX;
   MIL_INT PosY;
   MIL_INT DataCenterX;
   MIL_INT DataCenterY;
   };

// Interface to the processing to be done on the pixel selected by the mouse.
class ISelectionProcessing
   {
   public:
      virtual void ProcessSelection(MIL_INT SelectedValue,
                                    MIL_INT SelectedPosX,
                                    MIL_INT SelectedPosY) = 0;
   };

// Zoom display class declaration.
class CZoomDisplay
   {
   public:
      CZoomDisplay(MIL_ID MilSystem,
                   MIL_INT OffsetX,
                   MIL_INT OffsetY,
                   MIL_INT SizeX,
                   MIL_INT SizeY,
                   MIL_CONST_TEXT_PTR DIsplayName);
      virtual ~CZoomDisplay();

      void Select(MIL_ID MilImage, ISelectionProcessing* pSelectionProcessing);
      void Deselect();

      static MIL_INT MFTYPE MouseMoveHook(MIL_INT /*HookType*/, MIL_ID MilEvent, void* UserDataPtr)
         {
         CZoomDisplay* ZoomDisplay = (CZoomDisplay*)UserDataPtr;
         ZoomDisplay->MouseMove(MilEvent);
         return 0;
         }
      operator MIL_ID() const& { return m_MilDisplay; }

   private:

      void MouseMove(MIL_ID MilEvent);

      SZoomWindow GetZoomWindow(MIL_ID MilEvent, MIL_DOUBLE MouseBufferPosX, MIL_DOUBLE MouseBufferPosY);
      void DrawZoomWindow(const SZoomWindow& ZoomWindow);

      MIL_UNIQUE_DISP_ID m_MilDisplay;
      MIL_UNIQUE_GRA_ID  m_MilZoomGraList;
      MIL_UNIQUE_GRA_ID  m_MilZoomGraContext;

      MIL_UNIQUE_BUF_ID  m_MilSelectedImage;

      ISelectionProcessing* m_pSelectionProcess;      
   };

#endif //ZOOM_DISPLAY

