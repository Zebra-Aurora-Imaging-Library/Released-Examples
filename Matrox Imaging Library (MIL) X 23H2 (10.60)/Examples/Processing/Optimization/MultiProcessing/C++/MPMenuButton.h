//***************************************************************************************
//
// File name: MenuButton.h
//
// Synopsis:  Class that defines the buttons of menu in the MP processing example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MENUBUTTON_H
#define MENUBUTTON_H

//*****************************************************************************
// Class used to define the menu buttons of the MP processing example.
//*****************************************************************************
class CMPMenuButton
   {
   public:
      CMPMenuButton();
      ~CMPMenuButton();

      //Create/destroy functions for buttons
      void Create(MIL_CONST_TEXT_PTR Text, MIL_ID MilParentBuffer, 
                  RectStruct Rect, bool IsToggle=true);

      void Destroy();

      //Button control functions
      void Push(MIL_CONST_TEXT_PTR Text);
      void Disable(MIL_CONST_TEXT_PTR Text);
      inline void Enable(MIL_CONST_TEXT_PTR Text);

      //Button inquire functions
      inline bool IsPressed() const;
      inline bool IsToggle() const;
      inline const RectStruct& GetRect() const;
      inline bool IsDisabled() const;

   private:
      //Disallow copy
      CMPMenuButton(const CMPMenuButton&);
      CMPMenuButton& operator=(const CMPMenuButton&);

      MIL_ID m_MilButtonBuffer;
      MIL_ID m_MilGraphicsContext;
      MIL_ID m_MilPressedButton;
      MIL_ID m_MilUnpressedButton;
      MIL_ID m_MilButtonSystem;
      RectStruct m_ButtonRect;
      bool m_IsToggle;
      bool m_IsPressed;
      bool m_IsDisabled;
   };

//*****************************************************************************
// IsDisabled.  Returns whether the button is disabled or not.
//*****************************************************************************
inline bool CMPMenuButton::IsDisabled() const 
   { 
   return m_IsDisabled; 
   }

//*****************************************************************************
// IsPressed.  Returns whether the button is pressed or not.
//*****************************************************************************
inline bool CMPMenuButton::IsPressed() const 
   { 
   return m_IsPressed; 
   }

//*****************************************************************************
// IsToggle.  Returns whether it is a toggle button.
//*****************************************************************************
inline bool CMPMenuButton::IsToggle() const 
   { 
   return m_IsToggle; 
   }

//*****************************************************************************
// GetRect.  Returns the rectangle coordinates of the button.
//*****************************************************************************
inline const RectStruct& CMPMenuButton::GetRect() const
   { 
   return m_ButtonRect; 
   }
#endif
