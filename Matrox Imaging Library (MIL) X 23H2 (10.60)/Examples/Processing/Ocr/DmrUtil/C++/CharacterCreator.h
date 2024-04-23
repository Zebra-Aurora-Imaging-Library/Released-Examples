/************************************************************************************/
/*
* File name: CharacterCreator.h
*
* Synopsis:  This file defines the CCharCreator class used to
*            manage the creation of a dot matrix character.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#ifndef CHARACTER_CREATOR
#define CHARACTER_CREATOR

class CCharCreator
   {
   public:
      CCharCreator(MIL_ID MilSystem, MIL_INT FontSizeX, MIL_INT FontSizeY);
      virtual ~CCharCreator();

      // Function to create the character.
      bool CreateCharacterInteractive(MIL_CONST_TEXT_PTR CharName, MIL_CONST_TEXT_PTR CharHexName,
                                      bool Clear);

      // Function to load a character.
      void LoadCharacter(const vector<MIL_UINT8>& rDotCharMatrix);

      // Function to draw the name of the character.
      static bool DrawCharacterName(MIL_ID MilDest, MIL_INT OffsetX, MIL_INT OffsetY,
                                    MIL_CONST_TEXT_PTR CharName);

      // Accessors.
      const vector< MIL_UINT8 > DotCharMatrix() const {return m_DotCharMatrix;}
      const MIL_ID CharacterImage() const {return m_MilCharacterBuffer;}
      const MIL_INT CaseSize() const {return m_CaseSize;}

   private:

      // Hook function that draws a circle when the mouse hover over a case.
      static MIL_INT MFTYPE HoverCaseHook(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
      MIL_INT HoverCase(MIL_ID MilEvent);

      // Hook function that modifies a case when the mouse is clicked until it is released.
      static MIL_INT MFTYPE HoverModifyCaseHook(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
      MIL_INT HoverModifyCase(MIL_ID MilEvent);

      // Hook function that checks if left-click mouse button is pressed.
      static MIL_INT MFTYPE MonitorPressButtonHook(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
      MIL_INT MonitorPressButton(MIL_ID MilEvent);

      // Hook function that checks if left-click mouse button is released.
      static MIL_INT MFTYPE MonitorReleaseButtonHook(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
      MIL_INT MonitorReleaseButton(MIL_ID MilEvent);

      // Function that draws the grid in the display image.
      void DrawGrid();

      // Function that draws the character.
      void DrawCharacterImage();

      // Function that calculates the position.
      bool CalculatePositions(MIL_DOUBLE PosX, MIL_DOUBLE PosY, MIL_INT* pCaseIndex,
                              MIL_INT* pPosInCaseX, MIL_INT* pPosInCaseY);

      // Function that changes the case.
      void ClearCase();
      void ModifyCase(MIL_ID MilEvent);

      MIL_ID m_MilSystem;
      MIL_ID m_MilDisplay;
      MIL_ID m_MilDefaultDisplayBuffer;
      MIL_ID m_MilDisplayBuffer;
      MIL_ID m_MilDefaultCharacterBuffer;
      MIL_ID m_MilCharacterBuffer;

      bool m_IsPressed;
      bool m_IsSelected;
      bool m_IsCleared;
      MIL_INT m_ClearCaseIndex;

      vector<MIL_UINT8> m_DotCharMatrix;

      MIL_INT m_CaseSize;
      MIL_INT m_FontSizeX;
      MIL_INT m_FontSizeY;
      vector<MIL_INT> m_DotsPosX;
      vector<MIL_INT> m_DotsPosY;
   };

#endif
