/************************************************************************************/
/*
* File name: VisualizeFont.cpp
*
* Synopsis:  This file implements a class CVisualizeFont to
             display all the characters of a DMR font file.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include <math.h>
#include <algorithm>
#include "FontUtil.h"
#include "CharacterCreator.h"
#include "VisualizeFont.h"

// Define the maximum pixel size X and Y of the display buffer.
static const MIL_INT MAX_DISPLAY_SIZE_X = 1600;

// Define the Y offset that will be used to draw Text in the display buffer.
static const MIL_INT TEXT_SIZE_Y = 20;
static const MIL_INT MIN_CHAR_SPACE = 20;
static const MIL_INT MIN_CHAR_SIZE_X = 120;

// Define the scale factor for resizing the temporary image buffer.
const MIL_DOUBLE SCALE_FACTOR = 0.333;

/*****************************************************************************/
/* Constructor.                                                              */
/* This constructor not only allocates the display buffer, but also draws    */
/* all the character font located in the font.                               */
/*****************************************************************************/
CVisualizeFont::CVisualizeFont(MIL_ID MilSystem, MIL_ID MilDmrContext)
   : m_NumberOfChar(0),
     m_ValidFontFile(true)
   {
   // Get the number of characters of the font.
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                   M_NUMBER_OF_CHARS + M_TYPE_MIL_INT, &m_NumberOfChar);

   // Return if the font file contains no character.
   if (m_NumberOfChar == 0)
      {
      m_ValidFontFile = false;
      return;
      }

   // Get the font size of the existing font.
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                   M_FONT_SIZE_COLUMNS + M_TYPE_MIL_INT, &m_FontSizeX);
   if (m_FontSizeX == M_DEFAULT)
      MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                      M_FONT_SIZE_COLUMNS + M_TYPE_MIL_INT + M_DEFAULT, &m_FontSizeX);
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                      M_FONT_SIZE_ROWS + M_TYPE_MIL_INT, &m_FontSizeY);
   if (m_FontSizeY == M_DEFAULT)
      MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                      M_FONT_SIZE_ROWS + M_TYPE_MIL_INT + M_DEFAULT, &m_FontSizeY);

   // Allocate the character creator.
   m_pCharCreator = new CCharCreator(MilSystem, m_FontSizeX, m_FontSizeY);

   // Calculate the display buffer size and the child buffer size.
   CalculateDisplaySize();

   // Allocate the display.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilDisplay);
   MdispControl(m_MilDisplay, M_TITLE, MIL_TEXT("Dot Matrix Font"));

   // Allocate the main display buffers.
   MbufAlloc2d(MilSystem, m_DisplaySizeX, m_DisplaySizeY, 8 + M_UNSIGNED,
               M_IMAGE + M_PROC + M_DISP, &m_MilDisplayBuffer);

   // Clear the allocated display buffer.
   MbufClear(m_MilDisplayBuffer, M_COLOR_WHITE);

   // Draw all the character font to the display buffer.
   DrawAllCharFontToDisplay(MilDmrContext);
   }

/*****************************************************************************/
/* Destructor of CVisualizeFont class.                                       */
/*****************************************************************************/
CVisualizeFont::~CVisualizeFont()
   {
   if (m_ValidFontFile)
      {
      // Unselect the display buffer from the display.
      MdispSelect(m_MilDisplay, M_NULL);

      // Free the display.
      MdispFree(m_MilDisplay);

      // Free all the image Buffers.
      MbufFree(m_MilDisplayBuffer);

      delete m_pCharCreator;
      }
   }

/*****************************************************************************/
/* VisualizeFont. Function that displays the main display buffer.            */
/*****************************************************************************/
void CVisualizeFont::VisualizeFont()
   {
   if (m_ValidFontFile)
      {
      // Select the display buffer to display.
      MdispSelect(m_MilDisplay, m_MilDisplayBuffer);
      }
   }

/*****************************************************************************/
/* SaveDisplayFontImage. Saves the display buffer interactively.             */
/*****************************************************************************/
void CVisualizeFont::SaveDisplayFontImage()
   {
   if (m_ValidFontFile)
      {
      MIL_TEXT_CHAR MyChar = ' ';

      // Ask if the user wants to add another character/characters.
      MosPrintf(MIL_TEXT("Press 'S' to save the display font or any other key to continue.\n\n"));

      // Get the last character.
      MyChar = (MIL_TEXT_CHAR)MosGetch();

      if ((MyChar != 's') && (MyChar != 'S'))
         return;

      MIL_INT ExportError = M_NULL_ERROR;
      do
         {
         MappControl(M_ERROR, M_PRINT_DISABLE);
         MbufSave(M_INTERACTIVE, m_MilDisplayBuffer);
         MappGetError(M_DEFAULT, M_CURRENT, &ExportError);
         MappControl(M_ERROR, M_PRINT_ENABLE);
         if (ExportError != M_NULL_ERROR)
            {
            MosPrintf(MIL_TEXT("Displayed font image could not be exported!\n\n"));
            MosPrintf(MIL_TEXT("Press <Enter> to retry or any other key to continue.\n\n"));
            if (MosGetch() != MIL_TEXT('\r'))
               ExportError = M_NULL_ERROR;
            }
         else
            MosPrintf(MIL_TEXT("The display font has been succesfully saved.\n\n"));
         } while (ExportError != M_NULL_ERROR);
      }
   }

/*****************************************************************************/
/* CalculateDisplaySize. Calculates the display size and the child buffer    */
/*                       size.                                               */
/*****************************************************************************/
void CVisualizeFont::CalculateDisplaySize(void)
   {
   // Calculate the child buffer size.
   MIL_INT CharSizeX = MbufInquire(m_pCharCreator->CharacterImage(), M_SIZE_X, M_NULL);
   MIL_INT CharSizeY = MbufInquire(m_pCharCreator->CharacterImage(), M_SIZE_Y, M_NULL);
   m_SizeChildX = (MIL_INT)(SCALE_FACTOR * (MIL_DOUBLE)CharSizeX);
   m_SizeChildY = (MIL_INT)(SCALE_FACTOR * (MIL_DOUBLE)CharSizeY);

   // Adapt the m_CharSpaceX based on the MIN_CAR_SIZE
   m_CharSpaceX = MIN_CHAR_SPACE;
   if (m_SizeChildX < MIN_CHAR_SIZE_X)
      m_CharSpaceX += (MIN_CHAR_SIZE_X - m_SizeChildX)/2;

   // Calculate the display size.
   MIL_INT NbColumnsX = (MAX_DISPLAY_SIZE_X + m_CharSpaceX) / (m_SizeChildX + m_CharSpaceX);
   NbColumnsX = (NbColumnsX > m_NumberOfChar) ? m_NumberOfChar : NbColumnsX;
   NbColumnsX = std::max(NbColumnsX, (MIL_INT)1);
   MIL_INT NbSpaceX = NbColumnsX - 1;
   m_DisplaySizeX = NbColumnsX * m_SizeChildX + (NbSpaceX + 2) * m_CharSpaceX;
   MIL_INT NbRows = (MIL_INT)ceil((MIL_DOUBLE)m_NumberOfChar / NbColumnsX);
   MIL_INT NbSpaceY = NbRows - 1;
   m_DisplaySizeY = NbRows * (m_SizeChildY + TEXT_SIZE_Y) + (NbSpaceY + 2) * MIN_CHAR_SPACE;
   }

/*****************************************************************************/
/* DrawCharFontToDisplay. Draws the char font image buffer to the display    */
/*                        buffer.                                            */
/*****************************************************************************/
void CVisualizeFont::DrawCharFontToDisplay(MIL_ID MilDmrContext, MIL_INT Index,
                                           MIL_INT OffsetX, MIL_INT OffsetY)
   {
   // Get the character template.
   vector<MIL_UINT8> DotCharMatrix(m_FontSizeX * m_FontSizeY);
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), Index, M_NULL,
                   M_CHAR_TEMPLATE + M_TYPE_MIL_UINT8, &DotCharMatrix[0]);

   // Draw fill circles in the temporary display buffer.
   CCharCreator CharCreator(M_DEFAULT_HOST, m_FontSizeX, m_FontSizeY);
   CharCreator.LoadCharacter(DotCharMatrix);

   // Allocate a child buffer in the main display buffer.
   MbufChild2d(m_MilDisplayBuffer, OffsetX, OffsetY + TEXT_SIZE_Y,
               m_SizeChildX, m_SizeChildY, &m_MilDisplayChildBuffer);

   // Resize the temporary display image buffer to the child buffer.
   MimResize(CharCreator.CharacterImage(), m_MilDisplayChildBuffer,
             M_FILL_DESTINATION, M_FILL_DESTINATION, M_AVERAGE + M_OVERSCAN_DISABLE);

   // Make the text background transparent.
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   // Sets the foreground color the default graphic context.
   MgraColor(M_DEFAULT, M_COLOR_BLACK);

   // Control the text size.
   MgraControl(M_DEFAULT, M_FONT_SIZE, 16);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   MgraFont(M_DEFAULT, MIL_FONT_NAME(M_FONT_DEFAULT_TTF MIL_TEXT(":Bold")));
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   mstring CharName;
   FontUtil::GetCharName(MilDmrContext, Index, M_NULL, &CharName);
   mstring CharHexName;
   FontUtil::GetCharName(MilDmrContext, Index, M_HEX_UTF16_FOR_ALL, &CharHexName);
   mstring FullName = CharName + MIL_TEXT(" (") + CharHexName + MIL_TEXT(")");
   if (!(CCharCreator::DrawCharacterName(m_MilDisplayBuffer, OffsetX, OffsetY, FullName.c_str())))
      {
      FullName = MIL_TEXT("  (") + CharHexName + MIL_TEXT(")");
      MgraText(M_DEFAULT, m_MilDisplayBuffer, OffsetX, OffsetY, FullName.c_str());
      }

   // Free the child buffer.
   MbufFree(m_MilDisplayChildBuffer);
   }

/*****************************************************************************/
/* DrawAllCharFontToDisplay. Draws all the character font to the display     */
/*                           buffer.                                         */
/*****************************************************************************/
void CVisualizeFont::DrawAllCharFontToDisplay(MIL_ID MilDmrContext)
   {
   MIL_INT OffsetX = m_CharSpaceX;
   MIL_INT OffsetY = MIN_CHAR_SPACE;

   for (MIL_INT i = 0; i < m_NumberOfChar; i++)
      {
      if (OffsetX > m_DisplaySizeX - m_CharSpaceX - 1)
         {
         OffsetY += m_SizeChildY + TEXT_SIZE_Y + MIN_CHAR_SPACE;
         OffsetX = m_CharSpaceX;
         }

      // Draw the character font to the display buffer.
      DrawCharFontToDisplay(MilDmrContext, i, OffsetX, OffsetY);

      OffsetX += m_SizeChildX + m_CharSpaceX;
      }
   }

