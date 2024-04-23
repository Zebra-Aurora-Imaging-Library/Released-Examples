//***************************************************************************************/
// 
// File name: Surfacefinder.h  
//
// Synopsis: Header file for SurfaceFinder example.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//*************************************************************************************/
#ifndef _SURFACE_FINDER_H
#define _SURFACE_FINDER_H

#include <mil.h>

class CSurfaceFinder
   {
   public:
      CSurfaceFinder(MIL_ID MilSystem):m_MilSystem(MilSystem) {}
      ~CSurfaceFinder() = default;

      void AllocateDisplays();

      void ShowContainers(MIL_ID MilModelContainer, MIL_ID MilSceneContainer, MIL_INT View);

      MIL_INT64 PreprocessModel(MIL_ID MilContext);

      void Find(MIL_ID MilContext, MIL_ID MilContainer);

      MIL_ID GetSceneGraphicsList() { return m_SceneGraphicsList; }
      MIL_ID GetResult() { return m_MilResult; }
      void AllocateResult();

      MIL_INT64 ShowResults();

      MIL_INT64 DrawInScene(MIL_ID MilDrawContext);

      void ClearScene(MIL_INT64 Label)
         {
         M3dgraRemove(m_SceneGraphicsList, Label, M_DEFAULT);
         }
   private:

      MIL_ID               m_MilSystem;
      MIL_UNIQUE_3DDISP_ID m_MilDisplayModel;        /* 3D Mil Display of model.*/
      MIL_UNIQUE_3DDISP_ID m_MilDisplayProcessModel; /* 3D Mil Display of the preprocessed model.*/
      MIL_UNIQUE_3DDISP_ID m_MilDisplayScene;        /* 3D Mil Display of scene.*/
      MIL_ID               m_ModelGraphicsList;      /* 3D graphics list. */
      MIL_ID               m_ProcessModelGraphicsList;
      MIL_ID               m_SceneGraphicsList;
      MIL_INT              m_View;
      MIL_UNIQUE_3DMOD_ID  m_MilResult;              /*3D surface result. */
      MIL_DOUBLE           m_ComputationTime;
   };


#endif // !_SURFACE_FINDER_H
