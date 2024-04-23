//*************************************************************************************
//
// Synopsis: Declaration of the uniform grid class item (Mpat for now)
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

#include <mil.h>
#include <vector>

using std::vector;

template <class T>
class CUniformGrid 
   {
   public:
      CUniformGrid(MIL_INT SizeX, MIL_INT SizeY, MIL_INT Margin) { SetSize(SizeX, SizeY); SetMargin(Margin); }
      virtual ~CUniformGrid();

      /* Getter   */
      virtual MIL_INT GetSizeX() const { return m_SizeX; }
      virtual MIL_INT GetSizeY() const { return m_SizeY; }
      virtual MIL_INT GetMargin() const { return m_Margin; }
      virtual T& GetElement(MIL_INT Index) { return m_Elements[Index]; }
      virtual T& GetElement(MIL_INT Row, MIL_INT Col) { return m_Elements[Row*m_SizeX + Col]; }


      /* Setter   */
      virtual void SetSizeX(MIL_INT NewSizeX) { m_SizeX = NewSizeX; m_Elements.resize(m_SizeX * m_SizeY); }
      virtual void SetSizeY(MIL_INT NewSizeY) { m_SizeY = NewSizeY; m_Elements.resize(m_SizeX * m_SizeY); }
      virtual void SetSize(MIL_INT NewSizeX, MIL_INT NewSizeY) { m_SizeX = NewSizeX; m_SizeY = NewSizeY; m_Elements.resize(m_SizeX * m_SizeY); }
      virtual void SetMargin(MIL_INT Margin) { m_Margin = Margin; }
      
      virtual void SetElement(MIL_INT Index, const T Element) { m_Elements[Index] = Element; }
      virtual void SetElement(MIL_INT Row, MIL_INT Col, const T Element) { m_Elements[Row*m_SizeX + Col] = Element; }

      virtual void Clear() { m_Elements.clear(); }

   private:
      vector<T> m_Elements;
      MIL_INT m_SizeX;
      MIL_INT m_SizeY;
      MIL_INT m_Margin;
   };
   
/* Default Destructor   */
template <class T>
CUniformGrid<T>::~CUniformGrid()
   {
   Clear();
   }
