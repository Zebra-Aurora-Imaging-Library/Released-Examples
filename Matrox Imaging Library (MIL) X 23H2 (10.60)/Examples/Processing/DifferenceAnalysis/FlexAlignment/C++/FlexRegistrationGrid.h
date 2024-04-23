//*************************************************************************************
//
// Synopsis: Declaration of the flex registration grid.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
/************************************************************************************/
#pragma once

#include <mil.h>
#include "UniformGrid.h"
#include "FinderItem.h"
#include <vector>

using std::vector;

/* Enums     */
enum EFlexDrawOperation { DRAW_MODEL, DRAW_SEARCH_REGION, DRAW_RESULT_BOX, DRAW_INDEX, DRAW_SCORE };

/* Constants */
const MIL_INT DEFAULT_DELTA_SEARCH               = 5;
const MIL_DOUBLE DEFAULT_MODEL_MIN_SEPARATION    = 20.0;
const MIL_DOUBLE DEFAULT_ACCEPTANCE_SCORE        = 75.0;
const MIL_DOUBLE DEFAULT_CERTAINTY_SCORE         = 90.0;
const MIL_DOUBLE DEFAULT_SCORE_DIFFERENCE_THRESH = 10.0;

class CFlexRegistrationGrid
   {
   public:
      CFlexRegistrationGrid(MIL_INT  SizeX, MIL_INT SizeY, MIL_INT Margin) :
         m_CellSizeX(0L), m_CellSizeY(0L), m_DeltaSearch(DEFAULT_DELTA_SEARCH), 
         m_ModelMinSeparation(DEFAULT_MODEL_MIN_SEPARATION), m_AcceptanceScore(DEFAULT_ACCEPTANCE_SCORE),
         m_CertaintyScore(DEFAULT_CERTAINTY_SCORE), m_ScoreDifferenceThresh(DEFAULT_SCORE_DIFFERENCE_THRESH),
         m_FlexRegistrationGrid(SizeX, SizeY, Margin)
         {};
      ~CFlexRegistrationGrid();

      /* Getter   */
      MIL_INT GetSizeX() const { return m_FlexRegistrationGrid.GetSizeX(); }
      MIL_INT GetSizeY() const { return m_FlexRegistrationGrid.GetSizeY(); }
      MIL_INT GetMargin() const { return m_FlexRegistrationGrid.GetMargin(); }
      CFinderItem& GetElement(MIL_INT Index) { return m_FlexRegistrationGrid.GetElement(Index); }
      CFinderItem& GetElement(MIL_INT Row, MIL_INT Col) { return m_FlexRegistrationGrid.GetElement(Row, Col); }

      /* Setter   */
      void SetSizeX(MIL_INT NewSizeX);
      void SetSizeY(MIL_INT NewSizeY);
      void SetMargin(MIL_INT Margin);
      void SetSize(MIL_INT NewSizeX, MIL_INT NewSizeY);
      void SetDeltaSearch(MIL_INT NewDeltaSearch) { m_DeltaSearch = NewDeltaSearch; UpdateSearchRegion(); }
      void SetModelMinSeparation(MIL_DOUBLE ModelMinSeparation) { m_ModelMinSeparation = ModelMinSeparation; UpdateModelMinSeparation(); }
      void SetAcceptanceAndCertaintyScore(MIL_DOUBLE AcceptanceScore, MIL_DOUBLE CertaintyScore) { m_AcceptanceScore = AcceptanceScore; m_CertaintyScore = CertaintyScore; UpdateAcceptanceAndCertaintyScore(); }
      void SetScoreDifferenceThresh(MIL_DOUBLE ScoreDifferenceThresh) { m_ScoreDifferenceThresh = ScoreDifferenceThresh;}

      void SetElement(MIL_INT Index, const CFinderItem Element) {m_FlexRegistrationGrid.SetElement(Index, Element);}
      void SetElement(MIL_INT Row, MIL_INT Col, const CFinderItem Element) {m_FlexRegistrationGrid.SetElement(Row, Col, Element);}

      /* Others   */
      void Calculate(MIL_ID TargetBufferId, MIL_ID DstCalId);
      void Draw(MIL_ID DstBufferId, EFlexDrawOperation FlexDrawOperation);
      void ClearGrid();
      void UpdateGrid(MIL_ID TemplateBufferId);
      
   private:
      MIL_INT m_CellSizeX;
      MIL_INT m_CellSizeY;
      MIL_INT m_DeltaSearch;
      MIL_DOUBLE m_ModelMinSeparation;
      MIL_DOUBLE m_AcceptanceScore;
      MIL_DOUBLE m_CertaintyScore;
      MIL_DOUBLE m_ScoreDifferenceThresh;
      CUniformGrid<CFinderItem> m_FlexRegistrationGrid;

      void UpdateSearchRegion();
      void UpdateModelMinSeparation();
      void UpdateAcceptanceAndCertaintyScore();
    };
