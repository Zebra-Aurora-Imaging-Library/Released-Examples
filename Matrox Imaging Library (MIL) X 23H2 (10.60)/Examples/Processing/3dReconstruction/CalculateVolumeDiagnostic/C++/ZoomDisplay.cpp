//***************************************************************************************/
//
// File name: ZoomDisplay.cpp
//
// Synopsis:  Implementation of CZoomDisplay class used to display an image, overlaying
//            a zoomed portion around the mouse cursor location.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h>
#include "ExampleUtil.h"
#include "ZoomDisplay.h"

static const MIL_INT ZOOM_WINDOW_SIZE        = 7;
static const MIL_INT ZOOM_WINDOW_HALF_SIZE   = ZOOM_WINDOW_SIZE / 2;
static const MIL_INT ZOOM_PIXEL_SIZE         = 10;
static const MIL_INT ZOOM_SIZE               = ZOOM_WINDOW_SIZE * ZOOM_PIXEL_SIZE;
static const MIL_INT ZOOM_OFFSET_X           = 15;
static const MIL_INT ZOOM_OFFSET_Y           = 20;

//*****************************************************************************
// Constructor.
//*****************************************************************************
CZoomDisplay::CZoomDisplay(MIL_ID MilSystem,
                           MIL_INT OffsetX,
                           MIL_INT OffsetY,
                           MIL_INT SizeX,
                           MIL_INT SizeY,
                           MIL_CONST_TEXT_PTR DisplayName)
   {
   m_MilDisplay = AllocImageDisplay(MilSystem, OffsetX, OffsetY, SizeX, SizeY, DisplayName);
   m_MilZoomGraList = MgraAllocList(MilSystem, M_DEFAULT, M_UNIQUE_ID);

   // Set up the graphics context of the display.
   m_MilZoomGraContext = MgraAlloc(MilSystem, M_UNIQUE_ID);
   MgraControl(m_MilZoomGraContext, M_INPUT_UNITS, M_DISPLAY);

   // Set up the display.
   MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilZoomGraList);
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CZoomDisplay::~CZoomDisplay()
   {
   Deselect();
   }

//*****************************************************************************
// Selects the specified image and sets the selection process to apply
// when the mouse hovers over the displayed selected image.
//*****************************************************************************
void CZoomDisplay::Select(MIL_ID MilImage, ISelectionProcessing* pSelectionProcess)
   {
   Deselect();

   m_MilSelectedImage = MbufClone(MilImage, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                                  M_DEFAULT, M_DEFAULT, M_COPY_SOURCE_DATA, M_UNIQUE_ID);

   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
   MdispSelect(m_MilDisplay, m_MilSelectedImage);
   MdispControl(m_MilDisplay, M_SCALE_DISPLAY, M_ONCE);

   MgraClear(M_DEFAULT, m_MilZoomGraList);

   m_pSelectionProcess = pSelectionProcess;

   MIL_DOUBLE ZoomX;
   MIL_DOUBLE ZoomY;
   MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_X, &ZoomX);
   MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_Y, &ZoomY);
   MdispZoom(m_MilDisplay, ZoomX * 0.9, ZoomY * 0.9);
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);

   MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE, MouseMoveHook, this);
   }

//*****************************************************************************
// Deselects the image and unhooks the mouse move callback.
//*****************************************************************************
void CZoomDisplay::Deselect()
   {
   if(m_MilSelectedImage)
      {
      MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE + M_UNHOOK, MouseMoveHook, this);
      m_MilSelectedImage.reset();
      }
   }

//*****************************************************************************
// Gets the zoom window information.
//*****************************************************************************
SZoomWindow CZoomDisplay::GetZoomWindow(MIL_ID MilEvent,
                                        MIL_DOUBLE MouseBufferPosX,
                                        MIL_DOUBLE MouseBufferPosY)
   {
   SZoomWindow ZoomWindow;

   // Position the zoom drawing.
   auto OneDimPosition = [&](MIL_INT DisplayPositionInfo,
                             MIL_INT DisplaySizeInquire,
                             MIL_INT PosOffset)
      {
      // Get the center position of the display.
      MIL_INT DisplayDimPos = MdispInquire(m_MilDisplay, DisplaySizeInquire, M_NULL);
      MIL_DOUBLE Center = 0.5 * DisplayDimPos;

      // Get the position of the mouse.
      MIL_INT MouseDisplayPos;
      MdispGetHookInfo(MilEvent, DisplayPositionInfo, &MouseDisplayPos);

      // Return the position of the zoom.
      MIL_DOUBLE ZoomDisplayPos = MouseDisplayPos > Center ?
                                  (MouseDisplayPos - PosOffset - 0.5 * ZOOM_SIZE) :
                                  (MouseDisplayPos + PosOffset + 0.5 * ZOOM_SIZE);
      return (MIL_INT)std::round(ZoomDisplayPos);
      };

   ZoomWindow.PosX = OneDimPosition(M_MOUSE_POSITION_X, M_SIZE_X, ZOOM_OFFSET_X);
   ZoomWindow.PosY = OneDimPosition(M_MOUSE_POSITION_Y, M_SIZE_Y, ZOOM_OFFSET_Y);

   // Get the pixels of the neighborhood.
   MIL_INT MouseBufferPosXInt = (MIL_INT)std::round(MouseBufferPosX);
   MIL_INT MouseBufferPosYInt = (MIL_INT)std::round(MouseBufferPosY);

   MIL_INT DataStartX = MouseBufferPosXInt - ZOOM_WINDOW_HALF_SIZE;
   MIL_INT DataStartY = MouseBufferPosYInt - ZOOM_WINDOW_HALF_SIZE;
   MIL_INT DataEndX = MouseBufferPosXInt + ZOOM_WINDOW_HALF_SIZE;
   MIL_INT DataEndY = MouseBufferPosYInt + ZOOM_WINDOW_HALF_SIZE;

   std::vector<MIL_DOUBLE> ZoomWindowX;
   std::vector<MIL_DOUBLE> ZoomWindowY;
   for(MIL_INT y = DataStartY; y <= DataEndY; y++)
      {
      for(MIL_INT x = DataStartX; x <= DataEndX; x++)
         {
         ZoomWindowX.push_back((MIL_DOUBLE)x);
         ZoomWindowY.push_back((MIL_DOUBLE)y);
         }
      }

   ZoomWindow.DataCenterX = MouseBufferPosXInt;
   ZoomWindow.DataCenterY = MouseBufferPosYInt;

   // Here we assume that the selected image is of type MIL_UINT8.
   ZoomWindow.Values.resize(ZOOM_WINDOW_SIZE * ZOOM_WINDOW_SIZE, 255);
   MbufGetList(m_MilSelectedImage, M_DEFAULT, ZoomWindowX, ZoomWindowY,
               M_NEAREST_NEIGHBOR + M_OVERSCAN_DISABLE, ZoomWindow.Values);

   return ZoomWindow;
   }

//*****************************************************************************
// Draws the zoom window.
//*****************************************************************************
void CZoomDisplay::DrawZoomWindow(const SZoomWindow& ZoomWindow)
   {
   MgraClear(M_DEFAULT, m_MilZoomGraList);

   if(ZoomWindow.CenterElementValue() != 255)
      {
      MIL_INT ZoomDataStartX = ZoomWindow.PosX - ZOOM_WINDOW_HALF_SIZE * ZOOM_PIXEL_SIZE;
      MIL_INT ZoomDataStartY = ZoomWindow.PosY - ZOOM_WINDOW_HALF_SIZE * ZOOM_PIXEL_SIZE;
      MIL_INT ZoomDataEndX = ZoomWindow.PosX + ZOOM_WINDOW_HALF_SIZE * ZOOM_PIXEL_SIZE;
      MIL_INT ZoomDataEndY = ZoomWindow.PosY + ZOOM_WINDOW_HALF_SIZE * ZOOM_PIXEL_SIZE;
      MIL_INT DataIdx = 0;
      for(MIL_INT y = ZoomDataStartY; y <= ZoomDataEndY; y += ZOOM_PIXEL_SIZE)
         {
         for(MIL_INT x = ZoomDataStartX; x <= ZoomDataEndX; x += ZOOM_PIXEL_SIZE)
            {
            auto Gray = ZoomWindow.Values[DataIdx];
            MIL_DOUBLE Color = 255;
            if(Gray != 255)
               {
               auto LutR = STATUS_LUT[Gray * 3];
               auto LutG = STATUS_LUT[Gray * 3 + 1];
               auto LutB = STATUS_LUT[Gray * 3 + 2];
               Color = M_RGB888(LutR, LutG, LutB);
               }            
            
            MgraControl(m_MilZoomGraContext, M_COLOR, Color);
            MgraRectAngle(m_MilZoomGraContext, m_MilZoomGraList, x, y,
                          ZOOM_PIXEL_SIZE, ZOOM_PIXEL_SIZE, 0, M_CENTER_AND_DIMENSION + M_FILLED);
            if(DataIdx != ZoomWindow.CenterElementIndex())
               {
               MgraControl(m_MilZoomGraContext, M_COLOR, M_COLOR_BLUE);
               MgraRectAngle(m_MilZoomGraContext, m_MilZoomGraList, x, y,
                             ZOOM_PIXEL_SIZE, ZOOM_PIXEL_SIZE, 0, M_CENTER_AND_DIMENSION);
               }
            DataIdx++;
            }
         }
      MgraControl(m_MilZoomGraContext, M_COLOR, M_COLOR_WHITE);
      MgraArcFill(m_MilZoomGraContext, m_MilZoomGraList, ZoomWindow.PosX, ZoomWindow.PosY,
                  0.25 * ZOOM_PIXEL_SIZE, 0.25 * ZOOM_PIXEL_SIZE, 0, 360);
      MgraRectAngle(m_MilZoomGraContext, m_MilZoomGraList, ZoomWindow.PosX, ZoomWindow.PosY,
                    ZOOM_PIXEL_SIZE, ZOOM_PIXEL_SIZE, 0, M_CENTER_AND_DIMENSION);
      }
   }

//*****************************************************************************
// Callback that is called when the mouse moves in the display.
//*****************************************************************************
void CZoomDisplay::MouseMove(MIL_ID MilEvent)
   {
   MIL_DOUBLE MouseBufferPosX;
   MIL_DOUBLE MouseBufferPosY;
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_X, &MouseBufferPosX);
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_Y, &MouseBufferPosY);

   // Get the zoom window information.
   auto ZoomWindow = GetZoomWindow(MilEvent, MouseBufferPosX, MouseBufferPosY);

   // Disable the updates.
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);

   // Draw the zoom in the graphics list.
   DrawZoomWindow(ZoomWindow);

   // Call the process to perform with a new selection.
   m_pSelectionProcess->ProcessSelection(ZoomWindow.CenterElementValue(),
                                         ZoomWindow.DataCenterX,
                                         ZoomWindow.DataCenterY);

   // Enable the updates.
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
   }
