//***************************************************************************************
//
// File name: MeasOverviewExample.cpp 
//
// Synopsis:  This file contains the implementation of the CMeasOverviewExample class 
//            which manages simple measurement examples.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>
#include "MeasOverviewExample.h"
#include "ProfileDisplay.h"

//*****************************************************************************
// Constructor. Allocates the general mil objects and set up the displays.
//*****************************************************************************
CMeasOverviewExample::CMeasOverviewExample(bool UseProfileDisplay)
   : m_pProfileDisplay(NULL)
   {
   // Allocate the general mil objects.
   MappAlloc(M_DEFAULT, &m_MilApplication);
   m_MilSystem = M_DEFAULT_HOST;
   MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilDisplay);
   MgraAllocList(m_MilSystem, M_DEFAULT, &m_MilGraList);
   MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraList);

   // Allocate the profile display.
   if(UseProfileDisplay)
      {
      m_pProfileDisplay = new CProfileDisplay(m_MilSystem);
      
      // Move the display below the profile display.
      MdispControl(m_MilDisplay, M_WINDOW_INITIAL_POSITION_Y, m_pProfileDisplay->ProfileImageSizeY() + WINDOWS_OFFSET_Y);
      }
   }

//*****************************************************************************
// Destructor. Frees the general mil objects.
//*****************************************************************************
CMeasOverviewExample::~CMeasOverviewExample()
   {
   // Delete the profile display.
   if(m_pProfileDisplay)
      delete m_pProfileDisplay;

   // Free the mil objects.
   MgraFree(m_MilGraList);
   MdispFree(m_MilDisplay);
   if (m_MilSystem != M_DEFAULT_HOST)
      MsysFree(m_MilSystem);
   MappFree(m_MilApplication);
   }

//*****************************************************************************
// RunMeasCase. Runs one example case scenario.
//*****************************************************************************
void CMeasOverviewExample::RunMeasCase(MIL_CONST_TEXT_PTR ImageFile, MIL_INT MarkerType, const SMeasRegion& rMeasBox, SetupFunc pSetupFunc, MIL_INT MeasurementList, const SDrawList& rDrawList)
   {
   // Disable display updates and clear the graphic lists.
   MdispControl(m_MilDisplay, M_UPDATE, M_DISABLE);
   MgraClear(M_DEFAULT, m_MilGraList);
   if (m_pProfileDisplay)
      {
      m_pProfileDisplay->SetUpdate(M_DISABLE);
      m_pProfileDisplay->ClearAnnotations();
      }

   // Allocate a marker and set its region.
   MIL_ID MilMeasMarker = MmeasAllocMarker(m_MilSystem, MarkerType, M_DEFAULT, M_NULL);
   rMeasBox.SetMarkerRegion(MilMeasMarker, MarkerType);

   // Set up the marker.
   if(pSetupFunc)
      (pSetupFunc)(MilMeasMarker);

   // Restore the image, set a uniform calibration and display it.
   MIL_ID MilImage = MbufRestore(ImageFile, m_MilSystem, M_NULL);
   MIL_INT ImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);
   McalUniform(MilImage, 0.0, 0.0, 1.0, 1.0, 0.0, M_DEFAULT);
   MdispSelect(m_MilDisplay, MilImage);

   // Set up the drawing.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
   if(MarkerType != M_CIRCLE)
      MmeasSetMarker(MilMeasMarker, M_DRAW_PROFILE_SCALE_OFFSET, M_AUTO_SCALE_PROFILE, M_DEFAULT);

   // Find the marker.
   MmeasFindMarker(M_DEFAULT, MilImage, MilMeasMarker, MeasurementList);

   // Check the status.
   MIL_INT ValidFlag;
   MmeasGetResult(MilMeasMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
   if(ValidFlag == M_TRUE)
      {
      // Draw the results in the graphic list.
      rDrawList.DrawList(MilMeasMarker, m_MilGraList);

      // Create the profile.
      if(m_pProfileDisplay)
         m_pProfileDisplay->CreateProfile(MilImage, MilMeasMarker);
      }
   else
      MosPrintf(MIL_TEXT("Unable to find the marker..."));

   // Enable display updates.
   MdispControl(m_MilDisplay, M_UPDATE, M_ENABLE);
   if(m_pProfileDisplay)
      m_pProfileDisplay->SetUpdate(M_ENABLE);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free allocations.
   MbufFree(MilImage);
   MmeasFree(MilMeasMarker);
   }

//*****************************************************************************
// SetDisplayZoom. Sets the zoom of the display.
//*****************************************************************************
void CMeasOverviewExample::SetDisplayZoom(MIL_DOUBLE DisplayZoom)
   {
   MdispZoom(m_MilDisplay, DisplayZoom, DisplayZoom);
   }
