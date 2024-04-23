/*************************************************************************************/
/*
 * File name: FlexRegistration.cpp
 *
 * Synopsis:  Implementation of the FlexRegistration class
 *
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include "FlexRegistration.h"

/*************************************************************************************/
/*
 * Class             : CFlexRegistration
 *
 * Name              : CFlexRegistration()
 *
 * Access            : Public
 *
 * Synopsis          : Constructor
 *
 * Parameters        : SizeX, SizeY and Margin of the grid
 *
 */
CFlexRegistration::CFlexRegistration(MIL_INT SizeX /*= DEFAULT_SIZE_X*/, MIL_INT SizeY /*= DEFAULT_SIZE_X*/, MIL_INT Margin /*= DEFAULT_MARGIN*/) :
   m_FlexRegistrationGrid(SizeX, SizeY, Margin)
   {
   /* Allocate the calibration context. */
   m_CalId = McalAlloc(M_DEFAULT_HOST, 
      M_LINEAR_INTERPOLATION,
      M_DEFAULT, M_NULL);  
   }


/*************************************************************************************/
/*
 * Class             : CFlexRegistration
 *
 * Name              : Calculate()
 *
 * Access            : Public
 *
 * Synopsis          : Calculate the flex
 *
 * Parameters        : Target buffer on which to compute the
 *                   : flex transformation relative to the template
 *
 */
void CFlexRegistration::Calculate(MIL_ID TargetBufferId)
   {
   m_FlexRegistrationGrid.Calculate(TargetBufferId, m_CalId);
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistration
 *
 * Name              : Draw()
 *
 * Access            : Public
 *
 * Synopsis          : Draw flex registration information
 *
 * Parameters        : Destination buffer and draw operation to perform
 *
 */
void CFlexRegistration::Draw(MIL_ID DstBufferId, EFlexDrawOperation FlexDrawOperation)
   {
   m_FlexRegistrationGrid.Draw(DstBufferId, FlexDrawOperation);
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistration
 *
 * Name              : UpdateGrid()
 *
 * Access            : Public
 *
 * Synopsis          : Update the flex grid when the template buffer changed
 *
 */
void CFlexRegistration::UpdateGrid()
   {
   if (M_NULL != m_TemplateId)
      {
      /* Flush the previous grid and update. */
      m_FlexRegistrationGrid.ClearGrid();
      m_FlexRegistrationGrid.UpdateGrid(m_TemplateId);
      }
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistration
 *
 * Name              : Transform()
 *
 * Access            : Public
 *
 * Synopsis          : Align the src buffer on the template buffer
 *
 */
void CFlexRegistration::Transform(MIL_ID SrcBufferId, MIL_ID DstBufferId)
   {
   McalUniform(DstBufferId, 0, 0, 1, 1, 0, M_DEFAULT);
   McalTransformImage(SrcBufferId, DstBufferId, m_CalId, M_BILINEAR+M_OVERSCAN_CLEAR, M_DEFAULT, M_WARP_IMAGE + M_USE_DESTINATION_CALIBRATION);
   }
