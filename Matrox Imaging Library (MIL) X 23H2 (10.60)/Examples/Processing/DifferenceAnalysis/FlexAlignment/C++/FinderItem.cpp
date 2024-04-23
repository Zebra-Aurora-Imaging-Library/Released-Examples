/*************************************************************************************/
/*
 * File name: FinderItem.cpp
 *
 * Synopsis:  Implementation of the CFinderItem class
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include "FinderItem.h"
#include "Rect.h"

/*************************************************************************************/
/*
 * Class             : CFinderItem
 *
 * Name              : CFinderItem()
 *
 * Access            : Public
 *
 * Synopsis          : Constructor
 *
 */
CFinderItem::CFinderItem() :
   m_PatternMatchingId(M_NULL),
   m_ResultId(M_NULL),
   m_Rect(0L, 0L, 0L, 0L),
   m_Center(0.0, 0.0)
   {
   /* Init();. */
   }

/*************************************************************************************/
/*
 * Class             : CFinderItem
 *
 * Name              : Init()
 *
 * Access            : Public
 *
 * Synopsis          : Initialize the class
 *
 */
void CFinderItem::Init()
   {
   m_PatternMatchingId = M_NULL;
   m_ResultId = M_NULL;
   SetRect(0L, 0L, 0L, 0L);
   SetCenter(0.0, 0.0);
   }

/*************************************************************************************/
/*
 * Class             : CFinderItem
 *
 * Name              : ~CFinderItem()
 *
 * Access            : Public
 *
 * Synopsis          : Destructor
 *
 */
CFinderItem::~CFinderItem()
   {
   Init();
   }

/*************************************************************************************/
/*
 * Class             : CFinderItem
 *
 * Name              : SetCenter()
 *
 * Access            : Public
 *
 * Synopsis          : Set the center position of the finder item
 *
 */
void CFinderItem::SetCenter()
   {
   /* Set the center of the rect (MbufControlNeighor style). */
   m_Center.x = MIL_DOUBLE(m_Rect.OffX + ((m_Rect.SizeX - 1) / 2));
   m_Center.y = MIL_DOUBLE(m_Rect.OffY + ((m_Rect.SizeY - 1) / 2));

   if (M_NULL != m_PatternMatchingId)
      {
      MpatControl(m_PatternMatchingId, 0, M_REFERENCE_X, m_Center.x);
      MpatControl(m_PatternMatchingId, 0, M_REFERENCE_Y, m_Center.y);
      }
   }

