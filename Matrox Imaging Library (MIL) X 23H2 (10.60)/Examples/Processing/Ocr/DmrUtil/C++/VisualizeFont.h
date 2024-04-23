/************************************************************************************/
/*
* File name: VisualizeFont.h
*
* Synopsis:  This file defines the CVisualizeFont class used to
*            display all the characters of a DMR font.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#ifndef VISUALIZE_FONT_H
#define VISUALIZE_FONT_H

// Forward declares.
class CCharCreator;

class CVisualizeFont
   {
   public:
      // Constructor.
      CVisualizeFont(MIL_ID MilSystemID, MIL_ID MilDmrContext);

      // Destructor.
      ~CVisualizeFont(void);

      void VisualizeFont(void);
      void SaveDisplayFontImage(void);

   private:
      // Function that calculates the display buffer size and the child buffer size.
      void CalculateDisplaySize(void);

      // Function that draws the character font image buffer to the display buffer.
      void DrawCharFontToDisplay(MIL_ID MilDmrContext, MIL_INT Index,
                                 MIL_INT OffsetX, MIL_INT OffsetY);

      // Function that draws all the character font to the display buffer.
      void DrawAllCharFontToDisplay(MIL_ID MilDmrContext);

      MIL_ID                m_MilDisplay;
      MIL_ID                m_MilDisplayBuffer;
      MIL_ID                m_MilDisplayChildBuffer;

      MIL_INT               m_FontSizeX;
      MIL_INT               m_FontSizeY;
      MIL_INT               m_DisplaySizeX;
      MIL_INT               m_DisplaySizeY;
      MIL_INT               m_SizeChildX;
      MIL_INT               m_SizeChildY;
      MIL_INT               m_CharSpaceX;
      MIL_INT               m_NumberOfChar;
      bool                  m_ValidFontFile;

      CCharCreator* m_pCharCreator;
   };


#endif // VISUALIZE_FONT_H
