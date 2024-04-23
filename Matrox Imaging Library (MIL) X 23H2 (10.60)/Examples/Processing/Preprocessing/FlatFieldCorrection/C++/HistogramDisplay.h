//***************************************************************************************/
// 
// File name: HistogramDisplay.h
//
// Synopsis:  Declaration of the CHistogramDisplay class that displays the histogram of
//            an image.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef _HISTOGRAM_DISPLAY_H
#define _HISTOGRAM_DISPLAY_H

class CHistogramDisplay
   {
   public:

      // Constructor.
      CHistogramDisplay(MIL_ID MilSystem, MIL_CONST_TEXT_PTR Title = NULL, MIL_INT TitleColor = M_COLOR_WHITE);

      // Destructor.
      virtual ~CHistogramDisplay();

      // Preprocessing function of the histogram display.
      // based on a typical image.
      void Preprocess(MIL_ID MilTypicalImage);

      // Function to show the display.
      void Show();
      void Hide();

      // Function that updates the histogram.
      MIL_DOUBLE Update(MIL_ID MilImage, MIL_DOUBLE MaxVal);

      // Function that sets the window initial position.
      void SetWindowInitialPosition(MIL_INT WindowPosX, MIL_INT WindowPosY);

      // Accessor to the graphic list.
      MIL_ID GetGraList() const {return m_MilGraList;}

      // Accessor to histogram information.
      MIL_INT GetHistSizeX() const { return m_HistSizeX; }
      MIL_INT GetHistSizeY() const { return m_HistSizeY; }
      MIL_INT GetHistImageSizeX() const { return m_HistImageSizeX; }
      MIL_INT GetHistImageSizeY() const { return m_HistImageSizeY; }

   private:

      // Function that allocates and frees the MIL Objects.
      void AllocateHistObjects();
      void FreeHistObjects();

      // Functions that draws the display title.
      void DrawDisplayTitle();

      // The owner system.
      MIL_ID m_MilSystem;

      // The Mil Histogram result.
      MIL_ID m_MilHistResult;
      MIL_ID m_MilHistValues;
      MIL_DOUBLE* m_pHistValues[3];

      // The Mil display objects.
      MIL_ID m_MilDisplay;
      MIL_ID m_MilBackImage;
      MIL_ID m_MilGraList;
      MIL_ID m_MilGraContext;
      
      // The index array of the histogram.
      MIL_UINT32  m_NbEntries;
      MIL_DOUBLE* m_pHistIndexes;

      // The size of the histogram.
      MIL_INT m_HistSizeX;
      MIL_INT m_HistSizeY;
      MIL_INT m_HistImageSizeX;
      MIL_INT m_HistImageSizeY;

      // Title informations.
      MIL_INT m_TitleColor;
      MIL_TEXT_PTR m_Title;
   };

#endif // _HISTOGRAM_DISPLAY_H
