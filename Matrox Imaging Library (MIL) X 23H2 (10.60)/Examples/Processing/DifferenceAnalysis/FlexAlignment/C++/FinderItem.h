//*************************************************************************************
//
// Synopsis: Declaration of class that encapsulate a finder item (Mpat)
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

#include <mil.h>
#include "Point.h"
#include "Rect.h"

/*************************************************************************************/
/*
 * Class             : CFinderItem
 *
 * Synopsis          : Encapsulate a finder item object (Context and result)
 *  
 */
class CFinderItem
   {
   public:
      CFinderItem();
      virtual ~CFinderItem();

      /* Getter   */
      MIL_ID GetPatternMatchingId() const {return m_PatternMatchingId;}
      MIL_ID GetResultId() const {return m_ResultId;}
      CiRect GetRect() const {return m_Rect;}
      CdPoint2D GetCenter() const {return m_Center;}

      /* Setter   */
      void SetPatternMatchingId(MIL_ID PatternMachingId)  { m_PatternMatchingId = PatternMachingId;}
      void SetResultId(MIL_ID ResultId)  {m_ResultId = ResultId;}
      void SetRect(MIL_INT OffX, MIL_INT OffY, MIL_INT SizeX, MIL_INT SizeY)  {m_Rect.OffX = OffX; m_Rect.OffY = OffY; m_Rect.SizeX = SizeX; m_Rect.SizeY = SizeY;}   
      void SetCenter(MIL_DOUBLE CenterX, MIL_DOUBLE CenterY)  {m_Center.x = CenterX; m_Center.y = CenterY;}   
      void SetCenter();

      /* Init     */
      void Init();

   private:
      MIL_ID m_PatternMatchingId;  /* Not owned by the object */
      MIL_ID m_ResultId;           /* Not owned by the object */
      CiRect m_Rect;
      CdPoint2D m_Center;
      bool m_Status;               /* True = searchable       */
   };

