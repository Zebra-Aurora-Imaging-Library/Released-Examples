//*******************************************************************************
// 
// File name: CDisplay.h
//
// Synopsis:  Class in charge of managing the 2D/3D displays for 3D  
//            examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*******************************************************************************

#include <mil.h>

class CDisplay
   {
   public:
      CDisplay(MIL_ID MilSystem);

      CDisplay(const CDisplay&) = delete;
      CDisplay& operator=(const CDisplay&) = delete;

      void Alloc3dDisplayId();
      void Size(MIL_INT SizeX, MIL_INT SizeY);
      void PositionX(MIL_INT PositionX);
      void DisplayContainer(MIL_ID MilConatiner, bool UseLut);
      void Title(MIL_STRING Title);
      void SetView(MIL_INT64 Mode, MIL_DOUBLE Param1, MIL_DOUBLE Param2, MIL_DOUBLE Param3);
      MIL_INT64 Draw(MIL_ID MilResult);
      void UpdateDisplay(MIL_ID MilContainer, bool UseLut);
      void Clear(MIL_INT64 Label);
      void FreeDisplay();
   private:
      void GetGraphicListId();
         
      MIL_ID    m_MilSystem      = M_NULL;
      MIL_ID    m_MilDisplay     = M_NULL;
      MIL_ID    m_MilGraphicList = M_NULL;
      MIL_INT64 m_DisplayType    = M_NULL;
      MIL_ID    m_Lut            = M_NULL;
      MIL_ID    m_MilDepthMap    = M_NULL;
      MIL_ID    m_IntensityMap   = M_NULL;

   };
