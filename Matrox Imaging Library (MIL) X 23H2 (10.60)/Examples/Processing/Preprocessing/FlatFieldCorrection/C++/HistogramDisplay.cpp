//***************************************************************************************/
// 
// File name: HistogramDisplay.cpp
//
// Synopsis:  Implementation of the CHistogramDisplay class that displays the histogram of
//            an image.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "mil.h"
#include "HistogramDisplay.h"

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_INT MAX_LEGEND_DIGIT = 4;
static const MIL_INT HIST_BORDER_X = (MAX_LEGEND_DIGIT + 6) * 8 + 1;
static const MIL_INT HIST_BORDER_Y = 32;
static const MIL_INT DEFAULT_HIST_SIZE_X = 256;
static const MIL_INT DEFAULT_HIST_SIZE_Y = 196;

//*****************************************************************************
// Constructor.
//*****************************************************************************
CHistogramDisplay::CHistogramDisplay(MIL_ID MilSystem, MIL_CONST_TEXT_PTR Title /* = NULL */, MIL_INT TitleColor /* = M_COLOR_WHITE */)
   : m_MilSystem(MilSystem),

     m_MilHistResult(M_NULL),
     m_MilDisplay(M_NULL),
     m_MilBackImage(M_NULL),
     m_MilGraList(M_NULL),
     m_MilGraContext(M_NULL),

     m_NbEntries(0),
     m_pHistIndexes(NULL),
     
     m_TitleColor(TitleColor),
     m_Title(NULL),

     m_HistSizeX(DEFAULT_HIST_SIZE_X),
     m_HistSizeY(DEFAULT_HIST_SIZE_Y)
   {
   m_pHistValues[0] = NULL;
   m_pHistValues[1] = NULL;
   m_pHistValues[2] = NULL;

   // Allocate the display.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilDisplay);
   
   // Allocate the graphic list and associate it with the display.
   MgraAllocList(MilSystem, M_DEFAULT, &m_MilGraList);
   MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraList);

   // Allocate a graphic context and set the text alignment.
   MgraAlloc(m_MilSystem, &m_MilGraContext);

   // Save the display title.
   if(Title)
      {
      MdispControl(m_MilDisplay, M_TITLE, Title);
      MIL_INT TitleSize = MosStrlen(Title)+1;
      m_Title = new MIL_TEXT_CHAR[TitleSize];
      MosStrcpy(m_Title, TitleSize, Title);
      }
   }

//*****************************************************************************
// Destructor.
//*****************************************************************************
CHistogramDisplay::~CHistogramDisplay()
   {
   // Free the Mil objects.
   FreeHistObjects();
   
   // Free the graphic context.
   MgraFree(m_MilGraContext);

   // Free the graphic list.
   MgraFree(m_MilGraList);

   // Free the display.
   MdispFree(m_MilDisplay);
   
   // delete the title.
   if(m_Title)
      delete [] m_Title;
   }

//*****************************************************************************
// Function that allocates the Mil objects.
//*****************************************************************************
void CHistogramDisplay::AllocateHistObjects()
   {
   // Free the Mil Objects.
   FreeHistObjects();   

   // Allocate the histograms index array. The last 2 values are used to fill the polygon.
   m_pHistIndexes   = new MIL_DOUBLE[m_NbEntries + 2];

   // Set the index array.
   MIL_DOUBLE InvPixelSizeX = (MIL_DOUBLE)m_HistSizeX / m_NbEntries;
   for(MIL_UINT HistIdx = 0; HistIdx < m_NbEntries; HistIdx++)
      m_pHistIndexes[HistIdx] = HistIdx * InvPixelSizeX;
   m_pHistIndexes[m_NbEntries]   = (m_NbEntries-1) * InvPixelSizeX;
   m_pHistIndexes[m_NbEntries+1] = 0;

   // Allocate the histograms values arrays.
   for (MIL_INT BandIdx = 0; BandIdx < 3; BandIdx++)
      {
      m_pHistValues[BandIdx] = new MIL_DOUBLE[m_NbEntries + 2];
      m_pHistValues[BandIdx][m_NbEntries] = 0;
      m_pHistValues[BandIdx][m_NbEntries + 1] = 0;
      }

   // Allocate the histogram result.
   MimAllocResult(m_MilSystem, m_NbEntries, M_HIST_LIST, &m_MilHistResult);

   // Allocate the background image.
   m_HistImageSizeX = m_HistSizeX + 2 * HIST_BORDER_X;
   m_HistImageSizeY = m_HistSizeY + (MIL_INT)(1.5 * HIST_BORDER_Y);

   MbufAlloc2d(m_MilSystem, m_HistImageSizeX, m_HistImageSizeY, 8+M_UNSIGNED, M_IMAGE+M_DISP, &m_MilBackImage);
   MbufClear(m_MilBackImage, 0);
   }

//*****************************************************************************
// Function that frees the Mil objects.
//*****************************************************************************
void CHistogramDisplay::FreeHistObjects()
   {
   // Delete the histogram arrays.
   delete [] m_pHistIndexes;
   delete [] m_pHistValues[0];
   delete [] m_pHistValues[1];
   delete [] m_pHistValues[2];

   // Free the histogram result.
   if(m_MilHistResult)
      {
      MimFree(m_MilHistResult);
      m_MilHistResult = M_NULL;
      }   

   // Free the background image.
   if(m_MilBackImage)
      {
      MbufFree(m_MilBackImage);
      m_MilBackImage = M_NULL;
      }  
   }

//*****************************************************************************
// Preprocessing function.
//*****************************************************************************
void CHistogramDisplay::Preprocess(MIL_ID MilTypicalImage)
   {
   // Get the image information.
   MIL_INT ImageSizeBit = MbufInquire(MilTypicalImage, M_SIZE_BIT, M_NULL);
   
   if(m_NbEntries == 0 || m_NbEntries != (1 << ImageSizeBit))
      {
      // Set the number of entries.
      m_NbEntries = 1 << ImageSizeBit;

      // Reallocate the histogram objects.
      AllocateHistObjects();
           
      // Draw the title of the display.
      DrawDisplayTitle();
      }
   }

//*****************************************************************************
// Show and hide functions.
//*****************************************************************************
void CHistogramDisplay::Show()
   {
   // Select the image on the display
   MdispSelect(m_MilDisplay, m_MilBackImage);
   }

void CHistogramDisplay::Hide()
   {
   // Deselect the image on the display
   MdispSelect(m_MilDisplay, M_NULL);
   }

//*****************************************************************************
// Function that sets the window initial position.
//*****************************************************************************
void CHistogramDisplay::SetWindowInitialPosition(MIL_INT WindowPosX, MIL_INT WindowPosY)
   {
   MdispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_X, WindowPosX);
   MdispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_Y, WindowPosY);
   }

//*****************************************************************************
// Function that draws the display title.
//*****************************************************************************
void CHistogramDisplay::DrawDisplayTitle()
   {
   if(m_Title)
      {
      MgraControl(m_MilGraContext, M_INPUT_UNITS, M_DISPLAY);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_HORIZONTAL, M_LEFT);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_VERTICAL, M_TOP);
      MgraColor(m_MilGraContext, (MIL_DOUBLE)m_TitleColor);
      MgraText(m_MilGraContext, m_MilGraList, 0, 0, m_Title);
      MgraControl(m_MilGraContext, M_INPUT_UNITS, M_PIXEL);
      }
   }

//*****************************************************************************
// Function that updates the histogram with the image.
//*****************************************************************************
MIL_DOUBLE CHistogramDisplay::Update(MIL_ID MilImage, MIL_DOUBLE MaxVal)
   {
   // Disable the display updates
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);

   // Preprocess the display if necessary.
   Preprocess(MilImage);
     
   // Clear the graphic list.
   MgraClear(M_DEFAULT, m_MilGraList);

   // Get the number of bands of the image.
   MIL_INT SizeBand = MbufInquire(MilImage, M_SIZE_BAND, M_NULL);

   MIL_DOUBLE MaxValToUse = MaxVal;
   for (MIL_INT BandIdx = 0; BandIdx < SizeBand; BandIdx++)
      {
      // Get the current band.
      MIL_ID MilBand = MbufChildColor(MilImage, BandIdx, M_NULL);

      // Calculate the histogram.
      MimHistogram(MilBand, m_MilHistResult);

      // Get the histogram values.
      MimGetResult(m_MilHistResult, M_VALUE + M_TYPE_MIL_DOUBLE, m_pHistValues[BandIdx]);

      // Get the maximum histogram value if required.
      if (MaxVal == M_NULL)
         {
         for (MIL_UINT ValIdx = 0; ValIdx < m_NbEntries; ValIdx++)
            { MaxValToUse = (MaxValToUse < m_pHistValues[BandIdx][ValIdx]) ? m_pHistValues[BandIdx][ValIdx] : MaxValToUse; }
         }

      // Free the band.
      MbufFree(MilBand);
      }

   for (MIL_INT BandIdx = 0; BandIdx < SizeBand; BandIdx++)
      {
      // Calculate the histogram value to draw.
      MIL_DOUBLE InvPixelSizeY = (MIL_DOUBLE)m_HistSizeY / MaxValToUse;
      for (MIL_UINT ValIdx = 0; ValIdx < m_NbEntries; ValIdx++)
         m_pHistValues[BandIdx][ValIdx] = -m_pHistValues[BandIdx][ValIdx] * InvPixelSizeY;

      // Draw the legend.
      MgraColor(m_MilGraContext, M_COLOR_MAGENTA);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_HORIZONTAL, M_RIGHT);
      MgraControl(m_MilGraContext, M_TEXT_ALIGN_VERTICAL, M_CENTER);
      MIL_TEXT_CHAR Legend[MAX_LEGEND_DIGIT + 10];
      MosSprintf(Legend, MAX_LEGEND_DIGIT + 10, MIL_TEXT("%.*g"), MAX_LEGEND_DIGIT, (MIL_DOUBLE)MaxValToUse);
      MgraText(m_MilGraContext, m_MilGraList, HIST_BORDER_X - 1, HIST_BORDER_Y, Legend);

      // Draw the polygon.
      const MIL_DOUBLE PolygonColors[3] = { M_COLOR_RED, M_COLOR_GREEN, M_COLOR_BLUE };
      MgraControl(m_MilGraContext, M_DRAW_OFFSET_X, -HIST_BORDER_X);
      MgraControl(m_MilGraContext, M_DRAW_OFFSET_Y, -(HIST_BORDER_Y + m_HistSizeY));
      MgraColor(m_MilGraContext, PolygonColors[BandIdx]);
      MIL_INT ControlFlag = M_POLYLINE;
      MIL_INT NbEntriesToUse = m_NbEntries;
      if (SizeBand == 1)
         {
         ControlFlag = M_POLYGON + M_FILLED;
         NbEntriesToUse = m_NbEntries + 2;
         }
      MgraLines(m_MilGraContext, m_MilGraList, NbEntriesToUse, m_pHistIndexes, m_pHistValues[BandIdx], M_NULL, M_NULL, ControlFlag);
      MgraControl(m_MilGraContext, M_DRAW_OFFSET_X, 0);
      MgraControl(m_MilGraContext, M_DRAW_OFFSET_Y, 0);

      // Draw the axis.
      MgraColor(m_MilGraContext, M_COLOR_YELLOW);
      MgraLine(m_MilGraContext, m_MilGraList, HIST_BORDER_X, HIST_BORDER_Y, HIST_BORDER_X, HIST_BORDER_Y + m_HistSizeY);
      MgraLine(m_MilGraContext, m_MilGraList, HIST_BORDER_X, HIST_BORDER_Y + m_HistSizeY,
         HIST_BORDER_X + m_HistSizeX, HIST_BORDER_Y + m_HistSizeY);
      }

   // Draw the title.
   DrawDisplayTitle();

   // Enable the display updates
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);

   return MaxValToUse;
   }
