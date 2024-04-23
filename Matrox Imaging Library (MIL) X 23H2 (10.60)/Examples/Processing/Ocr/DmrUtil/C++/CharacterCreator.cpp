/************************************************************************************/
/*
* File name: CharacterCreator.cpp
*
* Synopsis:  This file implements the class CCharCreator to
*            manage the creation of a dot matrix character
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include <math.h>
#include "FontUtil.h"
#include "CharacterCreator.h"

// Define the case pixel size.
static const MIL_INT CASE_SIZE = 60;

// Define the ratio of the circle's radius within the case.
const MIL_DOUBLE CIRCLE_RADIUS_RATIO = 0.8;

// Char display offset definition.
static const MIL_INT NAME_OFFSET_Y = 10;
static const MIL_INT HEX_OFFSET_Y = NAME_OFFSET_Y + 30;
static const MIL_INT CHAR_OFFSET_X = 30;
static const MIL_INT CHAR_OFFSET_Y = HEX_OFFSET_Y + 30;
/*****************************************************************************/
/* Constructor.                                                              */
/*****************************************************************************/
CCharCreator::CCharCreator(MIL_ID MilSystem, MIL_INT FontSizeX, MIL_INT FontSizeY)
   :m_MilDefaultDisplayBuffer(M_NULL),
   m_MilDisplayBuffer(M_NULL),
   m_MilCharacterBuffer(M_NULL),
   m_FontSizeX(FontSizeX),
   m_FontSizeY(FontSizeY)
   {
   // Allocate and setup the display.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilDisplay);
   MdispControl(m_MilDisplay, M_CENTER_DISPLAY,   M_ENABLE);
   MdispControl(m_MilDisplay, M_KEYBOARD_USE,     M_DISABLE);
   MdispControl(m_MilDisplay, M_MOUSE_USE,        M_DISABLE);
   MdispControl(m_MilDisplay, M_WINDOW_RESIZE,    M_FULL_SIZE);
#if M_MIL_USE_WINDOWS
   MdispControl(m_MilDisplay, M_WINDOW_SYSBUTTON, M_DISABLE);
#endif
   MdispControl(m_MilDisplay, M_TITLE, MIL_TEXT("Dot Matrix Character"));

   // Set the case pixel size: add +1 if CASE_SIZE is even.
   m_CaseSize = CASE_SIZE % 2 == 0 ? CASE_SIZE+ 1 : CASE_SIZE;

   // Calculate the display size.
   MIL_INT CharSizeX = FontSizeX * m_CaseSize + 1;
   MIL_INT CharSizeY = FontSizeY * m_CaseSize + 1;
   MIL_INT DisplaySizeX = CharSizeX + 2 * CHAR_OFFSET_X;
   MIL_INT DisplaySizeY = CharSizeY + 2 * CHAR_OFFSET_Y;
   MIL_INT Type = (8 + M_UNSIGNED);

   // Allocate all the display buffers.
   MbufAlloc2d(MilSystem, DisplaySizeX, DisplaySizeY, Type,
               M_IMAGE + M_PROC + M_DISP, &m_MilDefaultDisplayBuffer);
   MbufChild2d(m_MilDefaultDisplayBuffer, CHAR_OFFSET_X, CHAR_OFFSET_Y,
               CharSizeX, CharSizeY, &m_MilDefaultCharacterBuffer);
   MbufAlloc2d(MilSystem, DisplaySizeX, DisplaySizeY, Type,
               M_IMAGE + M_PROC + M_DISP, &m_MilDisplayBuffer);
   MbufChild2d(m_MilDisplayBuffer, CHAR_OFFSET_X, CHAR_OFFSET_Y,
               CharSizeX, CharSizeY, &m_MilCharacterBuffer);

   // Clear all the allocated buffers.
   MbufClear(m_MilDefaultDisplayBuffer, M_COLOR_WHITE);
   MbufClear(m_MilDisplayBuffer, M_COLOR_WHITE);

   // Allocate the arrays that hold the positions of the dots.
   MIL_INT NumberOfDots = FontSizeX * FontSizeY;
   m_DotsPosX.resize(NumberOfDots);
   m_DotsPosY.resize(NumberOfDots);

   // Draw the grid.
   DrawGrid();
   }

/*****************************************************************************/
/* Destructor.                                                               */
/*****************************************************************************/
CCharCreator::~CCharCreator()
   {
   // Free the display buffers.
   MbufFree(m_MilDefaultCharacterBuffer);
   MbufFree(m_MilCharacterBuffer);
   MbufFree(m_MilDefaultDisplayBuffer);
   MbufFree(m_MilDisplayBuffer);

   // Free the display.
   MdispFree(m_MilDisplay);
   }

/*****************************************************************************/
/* DrawGrid. Draws the grid in the background of the character image         */
/*           representation                                                  */
/*****************************************************************************/
void CCharCreator::DrawGrid()
   {
   // Sets the foreground color the default graphic context.
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);

   // Calculate the total number of lines and allocate lines coordinates.
   MIL_INT NumberOfLines = m_FontSizeX + 1 + m_FontSizeY+ 1;
   vector<MIL_INT> LinesXStart(NumberOfLines);
   vector<MIL_INT> LinesYStart(NumberOfLines);
   vector<MIL_INT> LinesXEnd(NumberOfLines);
   vector<MIL_INT> LinesYEnd(NumberOfLines);

   // Calculate coordinates values of horizontal lines.
   MIL_INT Line = 0;
   for (MIL_INT j = 0; j <= m_FontSizeY; j++)
      {
      LinesYStart[Line] = j * m_CaseSize;
      LinesXStart[Line] = 0;
      LinesYEnd[Line]   = LinesYStart[Line];
      LinesXEnd[Line]   = m_FontSizeX * m_CaseSize;
      Line++;
      }

   // Calculate coordinates values of vertical lines.
   for (MIL_INT i = 0; i <= m_FontSizeX; i++)
      {
      LinesXStart[Line] = i * m_CaseSize;
      LinesYStart[Line] = 0;
      LinesXEnd[Line]   = LinesXStart[Line];
      LinesYEnd[Line]   = m_FontSizeY * m_CaseSize;
      Line++;
      }

   // Calculate the coordinates of dots.
   MIL_INT Dot = 0;
   for (MIL_INT j = 0; j < m_FontSizeY; j++)
      {
      for (MIL_INT i = 0; i < m_FontSizeX; i++)
         {
         m_DotsPosX[Dot] = (MIL_INT)(1 + m_CaseSize * (i + 0.5));
         m_DotsPosY[Dot] = (MIL_INT)(1 + m_CaseSize * (j + 0.5));
         Dot++;
         }
      }

   // Draw lines on the default display buffer.
   MgraLines(M_DEFAULT, m_MilDefaultCharacterBuffer, NumberOfLines,
             &LinesXStart[0], &LinesYStart[0], &LinesXEnd[0], &LinesYEnd[0], M_LINE_LIST);

   // Draw dots on the default display buffer.
   MgraDots(M_DEFAULT, m_MilDefaultCharacterBuffer, m_DotsPosX.size(),
            &m_DotsPosX[0], &m_DotsPosY[0], M_DEFAULT);
   }

/*****************************************************************************/
/* CreateCharacterInteractive. Interactively creates the character using a   */
/*                             MIL display and mouse hooks.                  */
/*****************************************************************************/
bool CCharCreator::CreateCharacterInteractive(MIL_CONST_TEXT_PTR CharName,
                                              MIL_CONST_TEXT_PTR CharHexName, bool Clear)
   {
   // Initialize font matrix data.
   m_IsPressed = false;
   m_IsSelected = false;
   m_IsCleared = true;
   m_ClearCaseIndex = 0;

   // Copy data of the default display buffer to display buffer if the
   // font display has to be cleared.
   if (Clear)
      {
      MbufCopy(m_MilDefaultDisplayBuffer, m_MilDisplayBuffer);
      m_DotCharMatrix = vector<MIL_UINT8>(m_FontSizeX * m_FontSizeY, 0);
      }
   MdispSelect(m_MilDisplay, m_MilDisplayBuffer);

   // Make the text background transparent.
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   // Sets the foreground color the default graphic context.
   MgraColor(M_DEFAULT, M_COLOR_BLACK);

   // Control the text size.
   MgraControl(M_DEFAULT, M_FONT_SIZE, 16);
   MgraFont(M_DEFAULT, MIL_FONT_NAME(M_FONT_DEFAULT_TTF MIL_TEXT(":Bold")));

   // Draw the char string.
   mstring CharNameString = MIL_TEXT("CharValue: ");
   CharNameString += CharName;
   DrawCharacterName(m_MilDisplayBuffer, CHAR_OFFSET_X, NAME_OFFSET_Y, CharNameString.c_str());

   // Draw the char hex string.
   mstring CharHexNameString = MIL_TEXT("HexCharValue: ");
   CharHexNameString += CharHexName;
   MgraText(M_DEFAULT, m_MilDisplayBuffer, CHAR_OFFSET_X, HEX_OFFSET_Y, CharHexNameString.c_str());

   // Enable display update.
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);

   // Hook functions to display events.
   MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE, HoverCaseHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN, MonitorPressButtonHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_DOUBLE_CLICK, MonitorPressButtonHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE, HoverModifyCaseHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_BUTTON_UP, MonitorReleaseButtonHook, this);

   // Wait for the user to press any key.
   MosPrintf(MIL_TEXT("The Dot Matrix Character grid is displayed.\n")
             MIL_TEXT("Use your mouse to interactively click on the cells\n")
             MIL_TEXT("to define the dots that represent the character.\n"));
   MosPrintf(MIL_TEXT("Press any key to complete the edition of the character.\n\n"));
   MosGetch();

   // Unhook the functions.
   MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE + M_UNHOOK, HoverCaseHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_BUTTON_DOWN + M_UNHOOK, MonitorPressButtonHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_DOUBLE_CLICK + M_UNHOOK, MonitorPressButtonHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE + M_UNHOOK, HoverModifyCaseHook, this);
   MdispHookFunction(m_MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, MonitorReleaseButtonHook, this);

   // Hide the display.
   MdispSelect(m_MilDisplay, M_NULL);

   // Check the character for at least one dot.
   bool HasOneDot = false;
   for (MIL_UINT i = 0; i < m_DotCharMatrix.size() && !HasOneDot; i++)
      HasOneDot = m_DotCharMatrix[i] == 0xFF;

   return HasOneDot;
   }

/*****************************************************************************/
/* LoadCharacter. Load a a character matrix into the character creator.      */
/*                Once the matrix is loaded the character is drawn.          */
/*****************************************************************************/
void CCharCreator::LoadCharacter(const vector<MIL_UINT8>& rDotCharMatrix)
   {
   m_DotCharMatrix = rDotCharMatrix;
   DrawCharacterImage();
   }

/*****************************************************************************/
/* DrawCharacterImage. Draws an images representing the current dot matrix.  */
/*****************************************************************************/
void CCharCreator::DrawCharacterImage()
   {
   MbufCopy(m_MilDefaultDisplayBuffer, m_MilDisplayBuffer);

   for (MIL_UINT i = 0; i < m_DotCharMatrix.size(); i++)
      {
      if (!m_DotCharMatrix[i])
            continue;

      // Calculate the center of the case coordinate.
      MIL_INT CenterCasePos = m_CaseSize / 2;

      // Calculate the circle radius.
      MIL_DOUBLE CircleRad = CIRCLE_RADIUS_RATIO * (MIL_DOUBLE)CenterCasePos;

      // Set the foreground color of the default graphic context.
      MgraColor(M_DEFAULT, M_COLOR_BLACK);

      // Draw fill circle.
      MgraArcFill(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[i], m_DotsPosY[i],
                  CircleRad, CircleRad, 0.0, 360.0);
      }
   }

/*****************************************************************************/
/* DrawCharacterName. Draws the character name in the specified destination  */
/*                    disabling the errors.
/*****************************************************************************/
bool CCharCreator::DrawCharacterName(MIL_ID MilDest, MIL_INT OffsetX, MIL_INT OffsetY,
                                     MIL_CONST_TEXT_PTR CharName)
   {
   MappControl(M_ERROR, M_PRINT_DISABLE);
   MgraText(M_DEFAULT, MilDest, OffsetX, OffsetY, CharName);
   MIL_INT DrawError = MappGetError(M_DEFAULT, M_CURRENT, M_NULL);
   MappControl(M_ERROR, M_PRINT_ENABLE);
   return DrawError == M_NULL_ERROR;
   }

/*****************************************************************************/
/* HoverCaseHook. Hook function that draws a circle when the mouse hover     */
/*                over a case.                                               */
/*****************************************************************************/
MIL_INT MFTYPE CCharCreator::HoverCaseHook(MIL_INT HookType, MIL_ID MilEvent, void *pUserData)
   {
   CCharCreator* pCharCreator = (CCharCreator*) pUserData;
   return pCharCreator->HoverCase(MilEvent);
   }
MIL_INT CCharCreator::HoverCase(MIL_ID MilEvent)
   {
   if (m_IsPressed)
      return 0;

   // Get the cursor position.
   MIL_DOUBLE MousePosX;
   MIL_DOUBLE MousePosY;
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_X, &MousePosX);
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_Y, &MousePosY);

   // Get the position of the case.
   MIL_INT CaseIndex, PosInCaseX, PosInCaseY;
   bool IsValid = CalculatePositions(MousePosX, MousePosY, &CaseIndex,
                                     &PosInCaseX, &PosInCaseY);

   if (!IsValid || m_DotCharMatrix[CaseIndex])
      {
      ClearCase();
      return 0;
      }

   // Calculate the center of the case coordinate.
   MIL_INT CenterCasePos = m_CaseSize / 2;

   // Calculate the absolute difference of the position in case with the center.
   MIL_DOUBLE DiffX = fabs((MIL_DOUBLE)(CenterCasePos - PosInCaseX));
   MIL_DOUBLE DiffY = fabs((MIL_DOUBLE)(CenterCasePos - PosInCaseY));

   // Calculate the distance.
   MIL_DOUBLE Distance = sqrt(DiffX * DiffX + DiffY * DiffY);

   // Calculate the circle radius.
   MIL_DOUBLE CircleRad = CIRCLE_RADIUS_RATIO * (MIL_DOUBLE)CenterCasePos;

   if (Distance > CircleRad)
      {
      ClearCase();
      return 0;
      }

   if (m_ClearCaseIndex != CaseIndex)
      ClearCase();

   if (!m_IsCleared)
      return 0;

   m_ClearCaseIndex = CaseIndex;
   m_IsCleared = false;

   // Set the foreground color of the default graphic context.
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);

   // Draw a circle.
   MgraArc(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[CaseIndex], m_DotsPosY[CaseIndex],
           CircleRad, CircleRad, 0.0, 360.0);

   return 0;
   }

/*****************************************************************************/
/* HoverModifyCaseHook. Hook function that modifies a case when the mouse is */
/*                      clicked until it is released.                        */
/*****************************************************************************/
MIL_INT MFTYPE CCharCreator::HoverModifyCaseHook(MIL_INT HookType, MIL_ID MilEvent,
                                                 void *pUserData)
   {
   CCharCreator* pCharCreator = (CCharCreator*) pUserData;
   return pCharCreator->HoverModifyCase(MilEvent);
   }
MIL_INT CCharCreator::HoverModifyCase(MIL_ID MilEvent)
   {
   if (!m_IsPressed)
      return 0;
   ModifyCase(MilEvent);
   return 0;
   }

/*****************************************************************************/
/* MonitorPressButtonHook. Hook function that checks if left-click mouse     */
/*                         button is pressed.                                */
/*****************************************************************************/
MIL_INT MFTYPE CCharCreator::MonitorPressButtonHook(MIL_INT HookType, MIL_ID MilEvent,
                                                    void *pUserData)
   {
   CCharCreator* pCharCreator = (CCharCreator*) pUserData;
   return pCharCreator->MonitorPressButton(MilEvent);
   }
MIL_INT CCharCreator::MonitorPressButton(MIL_ID MilEvent)
   {
   m_IsPressed = true;
   ModifyCase(MilEvent);
   m_IsCleared = true;
   return 0;
   }

/*****************************************************************************/
/* MonitorReleaseButtonHook. Hook function that checks if left-click mouse   */
/*                           button is released.                             */
/*****************************************************************************/
MIL_INT MFTYPE CCharCreator::MonitorReleaseButtonHook(MIL_INT HookType, MIL_ID MilEvent,
                                                      void *pUserData)
   {
   CCharCreator* pCharCreator = (CCharCreator*) pUserData;
   return pCharCreator->MonitorReleaseButton(MilEvent);
   }
MIL_INT CCharCreator::MonitorReleaseButton(MIL_ID MilEvent)
   {
   m_IsPressed = false;
   m_IsSelected = false;
   return 0;
   }

/*****************************************************************************/
/* CalculatePositions. Calculates the position of the case into which the    */
/*                     mouse is clicking or hovering. Returns whether the    */
/*                     mouse is inside the matrix.                           */
/*****************************************************************************/
bool CCharCreator::CalculatePositions(MIL_DOUBLE PosX, MIL_DOUBLE PosY, MIL_INT* pCaseIndex, MIL_INT* pPosInCaseX, MIL_INT* pPosInCaseY)
   {
   MIL_INT CaseX = (MIL_INT)(floor((PosX - (MIL_DOUBLE)CHAR_OFFSET_X - 0.5) /
                             (MIL_DOUBLE)(m_CaseSize)));
   MIL_INT CaseY = (MIL_INT)(floor((PosY - (MIL_DOUBLE)CHAR_OFFSET_Y - 0.5) /
                             (MIL_DOUBLE)(m_CaseSize)));

   if ((CaseX >= 0 && CaseX < m_FontSizeX) && (CaseY >= 0 && CaseY < m_FontSizeY))
      {
      *pPosInCaseX = ((MIL_INT)PosX - CHAR_OFFSET_X) - CaseX * m_CaseSize;
      *pPosInCaseY = ((MIL_INT)PosY - CHAR_OFFSET_Y) - CaseY * m_CaseSize;
      }
   else
      return false;

   *pCaseIndex = CaseX + CaseY * m_FontSizeX;
   return true;
   }

/*****************************************************************************/
/* ModifyCase. Modifies the case.                                            */
/*****************************************************************************/
void CCharCreator::ModifyCase(MIL_ID MilEvent)
   {
   // Get the cursor position.
   MIL_DOUBLE MousePosX;
   MIL_DOUBLE MousePosY;
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_X, &MousePosX);
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_Y, &MousePosY);

   // Get the position of the case.
   MIL_INT CaseIndex, PosInCaseX, PosInCaseY;
   bool IsValid = CalculatePositions(MousePosX, MousePosY, &CaseIndex, &PosInCaseX, &PosInCaseY);

   if (!IsValid)
      return;

   // Calculate the center of the case coordinate.
   MIL_INT CenterCasePos = m_CaseSize / 2;

   // Calculate the absolute difference of the position in case with the center.
   MIL_DOUBLE DiffX = fabs((MIL_DOUBLE)(CenterCasePos - PosInCaseX));
   MIL_DOUBLE DiffY = fabs((MIL_DOUBLE)(CenterCasePos - PosInCaseY));

   // Calculate the distance.
   MIL_DOUBLE Distance = sqrt(DiffX * DiffX + DiffY * DiffY);

   // Calculate the circle radius.
   MIL_DOUBLE CircleRad = CIRCLE_RADIUS_RATIO * (MIL_DOUBLE)CenterCasePos;

   if (Distance > CircleRad)
      {
      m_IsSelected = false;
      return;
      }

   if (m_IsSelected)
      return;

   if (m_DotCharMatrix[CaseIndex])
      {
      // Set the foreground color of the default graphic context.
      MgraColor(M_DEFAULT, M_COLOR_WHITE);

      // Disable display update.
      MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);

      // Draw a filled white circle to cover the previous circle.
      MgraArcFill(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[CaseIndex], m_DotsPosY[CaseIndex],
                  CircleRad, CircleRad, 0.0, 360.0);

      // Set the foreground color of the default graphic context.
      MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);

      // Draw a dot.
      MgraDot(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[CaseIndex], m_DotsPosY[CaseIndex]);

      // Enable display update.
      MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);

      m_DotCharMatrix[CaseIndex] = 0;
      }
   else
      {
      // Set the foreground color of the default graphic context.
      MgraColor(M_DEFAULT, M_COLOR_BLACK);

      // Draw a filled white circle to cover the previous circle.
      MgraArcFill(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[CaseIndex], m_DotsPosY[CaseIndex],
                  CircleRad, CircleRad, 0.0, 360.0);

      m_DotCharMatrix[CaseIndex] = 0xFF;
      }

   m_IsSelected = true;
   }

/*****************************************************************************/
/* ClearCase. Clears the last non selected case.                             */
/*****************************************************************************/
void CCharCreator::ClearCase()
   {
   if (m_IsCleared || m_DotCharMatrix[m_ClearCaseIndex])
      return;

   // Calculate the center of the case coordinate.
   MIL_INT CenterCasePos = m_CaseSize / 2;

   // Calculate the circle radius.
   MIL_DOUBLE CircleRad = CIRCLE_RADIUS_RATIO * (MIL_DOUBLE)CenterCasePos;

   // Set the foreground color of the default graphic context.
   MgraColor(M_DEFAULT, M_COLOR_WHITE);

   // Draw a circle.
   MgraArc(M_DEFAULT, m_MilCharacterBuffer, m_DotsPosX[m_ClearCaseIndex], m_DotsPosY[m_ClearCaseIndex],
           CircleRad, CircleRad, 0.0, 360.0);

   m_IsCleared = true;
   }
