/*************************************************************************************/
/*
 * Filename          : FlexRegistration.h
 * Content           : Declaration of the FlexRegistration class
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#pragma once

#include <mil.h>
#include <memory>
#include "FlexRegistrationGrid.h"
#include "MathUtil.h"

/* Constants */
const MIL_INT DEFAULT_SIZE_X = 10;
const MIL_INT DEFAULT_SIZE_Y = 10;
const MIL_INT DEFAULT_MARGIN = 5;

class CFlexRegistration
   {
   public:
      CFlexRegistration(MIL_INT SizeX = DEFAULT_SIZE_X, MIL_INT SizeY = DEFAULT_SIZE_X, MIL_INT Margin = DEFAULT_MARGIN);
      ~CFlexRegistration() {McalFree(m_CalId);}

      /* Setter */
      void SetTemplateBufferId(MIL_ID TemplateBufferId) {m_TemplateId = TemplateBufferId; UpdateGrid();}
      void SetSizeX(MIL_INT SizeX) {m_FlexRegistrationGrid.SetSizeX(SizeX); UpdateGrid();}
      void SetSizeY(MIL_INT SizeY) {m_FlexRegistrationGrid.SetSizeY(SizeY); UpdateGrid();}
      void SetMargin(MIL_INT Margin) {m_FlexRegistrationGrid.SetMargin(Margin); UpdateGrid();}
      void SetDeltaSearch(MIL_INT DeltaSearch) {m_FlexRegistrationGrid.SetDeltaSearch(M_Max(DeltaSearch, 1L));}
      void SetModelMinSeparation(MIL_DOUBLE ModelMinSeparation) {m_FlexRegistrationGrid.SetModelMinSeparation(M_Max(ModelMinSeparation, 1.0L));}
      void SetAcceptanceAndCertaintyScore(MIL_DOUBLE AcceptanceScore, MIL_DOUBLE CertaintyScore) {m_FlexRegistrationGrid.SetAcceptanceAndCertaintyScore(AcceptanceScore, CertaintyScore);}
      void SetScoreDifferenceThresh(MIL_DOUBLE ScoreDifferenceThresh) { m_FlexRegistrationGrid.SetScoreDifferenceThresh(ScoreDifferenceThresh);}

      /* Others */
      void Calculate(MIL_ID TargetBufferId);
      void Draw(MIL_ID DstBufferId, EFlexDrawOperation FlexDrawOperation);
      void Transform(MIL_ID SrcBufferId, MIL_ID DstBufferId);

   private:
      MIL_ID m_TemplateId;           /* Not own by the object */ 
      MIL_ID m_CalId;                /* Own by the object     */
      CFlexRegistrationGrid m_FlexRegistrationGrid;

      void UpdateGrid();

      /* Disable copy and assignment */
      CFlexRegistration(const CFlexRegistration&);
      CFlexRegistration& operator=(const CFlexRegistration&);
   };
