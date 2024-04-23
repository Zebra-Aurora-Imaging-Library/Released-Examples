//***************************************************************************************
//
// File name: ProfileDisplay.cpp
//
// Synopsis:  This file contains the implementation of the CProfileDisplay class 
//            which displays the profile associated to a measurement marker.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>
#include "ProfileDisplay.h"

//*****************************************************************************
// Constructor. Allocate the general mil objects and set up the display.
//*****************************************************************************
CProfileDisplay::CProfileDisplay(MIL_ID MilSystem)
   :m_MilSystem(MilSystem)
   {
   // Allocate the profile display.
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &m_MilProfileDisplay);
   MgraAllocList(MilSystem, M_DEFAULT, &m_MilProfileGraList);
   MdispControl(m_MilProfileDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilProfileGraList);
   MdispControl(m_MilProfileDisplay, M_TITLE, MIL_TEXT("Profile display"));

   // Allocate the profile image.
   MbufAlloc2d(m_MilSystem, PROFILE_SIZE_X, PROFILE_SIZE_Y, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &m_MilProfileImage);
   MbufClear(m_MilProfileImage, 0);

   // Allocate the projection result.
   MimAllocResult(m_MilSystem, PROFILE_SIZE_X, M_PROJ_LIST, &m_MilProjResult);
   }

//*****************************************************************************
// Destructor. Frees the mil objects.
//*****************************************************************************
CProfileDisplay::~CProfileDisplay()
   {
   MimFree(m_MilProjResult);
   MbufFree(m_MilProfileImage);
   MgraFree(m_MilProfileGraList);
   MdispFree(m_MilProfileDisplay);
   }

//*****************************************************************************
// CreateProfile. Creates the edgevalue profile and intensity profile.
//*****************************************************************************
void CProfileDisplay::CreateProfile(MIL_ID MilImage, MIL_ID MilMeasMarker)
   {
   SetUpdate(M_DISABLE);

   // Allocate a child of the image to avoid modifying the calibration of the original image.
   MIL_ID MilImageChild = MbufChildColor(MilImage, 0, M_NULL);

   // Retrieve information about the found search region box.
   MIL_DOUBLE Corners[8];
   MIL_DOUBLE FoundBoxAngle;
   MmeasGetResult(MilMeasMarker, M_BOX_ANGLE_FOUND, &FoundBoxAngle, M_NULL);
   if(MmeasInquire(MilMeasMarker, M_ORIENTATION, M_NULL, M_NULL) == M_VERTICAL)
      {
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_TOP_LEFT, &Corners[0], &Corners[1]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_TOP_RIGHT, &Corners[2], &Corners[3]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_BOTTOM_RIGHT, &Corners[4], &Corners[5]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_BOTTOM_LEFT, &Corners[6], &Corners[7]);
      }
   else
      {
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_TOP_LEFT, &Corners[6], &Corners[7]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_TOP_RIGHT, &Corners[0], &Corners[1]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_BOTTOM_RIGHT, &Corners[2], &Corners[3]);
      MmeasGetResult(MilMeasMarker, M_BOX_CORNER_BOTTOM_LEFT, &Corners[4], &Corners[5]);
      FoundBoxAngle -= 90;
      }

   MIL_DOUBLE Dx = Corners[2] - Corners[0];
   MIL_DOUBLE Dy = Corners[3] - Corners[1]; 
   MIL_DOUBLE BoxSizeX = sqrt(Dx*Dx + Dy*Dy);
   Dx = Corners[4] - Corners[2];
   Dy = Corners[5] - Corners[3]; 
   MIL_DOUBLE BoxSizeY = sqrt(Dx*Dx + Dy*Dy); 

   // Calibrate the profile image.
   MIL_DOUBLE ScaleX = PROFILE_SIZE_X / (BoxSizeX+1);
   MIL_DOUBLE ScaleY = PROFILE_SIZE_Y / (BoxSizeY+1);
   McalUniform(m_MilProfileImage, 0.5*(1.0/ScaleX) - 0.5, 0.5*(1.0/ScaleY) - 0.5, 1.0/ScaleX, 1.0/ScaleY, 0.0, M_DEFAULT);
   
   // Warp the profile in the profile image.
   McalFixture(MilImageChild, M_NULL, M_MOVE_RELATIVE, M_POINT_AND_ANGLE, M_DEFAULT, Corners[0], Corners[1], FoundBoxAngle, M_DEFAULT);
   McalTransformImage(MilImageChild, m_MilProfileImage, M_NULL, M_BILINEAR, M_DEFAULT, M_WARP_IMAGE+M_USE_DESTINATION_CALIBRATION);

   // Set up the graphic context and calibration of the profile image.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_WORLD);
   McalRelativeOrigin(m_MilProfileImage, 0, 0, 0.0, -FoundBoxAngle, M_DEFAULT);
   McalFixture(m_MilProfileImage, M_NULL, M_MOVE_RELATIVE, M_POINT_AND_ANGLE, M_DEFAULT, -Corners[0], -Corners[1], 0.0, M_DEFAULT);

   // Draw the search region.
   MgraColor(M_DEFAULT, M_COLOR_MAGENTA);
   MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_SEARCH_REGION + M_DRAW_SEARCH_DIRECTION, M_DEFAULT, M_DEFAULT);

   // Draw the position in the profile.
   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_POSITION_IN_PROFILE + M_DRAW_IN_BOX, M_DEFAULT, M_DEFAULT);

   // If a stripe, draw the position of the edges.
   if(MmeasInquire(MilMeasMarker, M_MARKER_TYPE, M_NULL, M_NULL) == M_STRIPE)
      {
      MgraColor(M_DEFAULT, M_COLOR_YELLOW);
      MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_POSITION_IN_PROFILE + M_DRAW_IN_BOX + M_EDGE_FIRST, M_DEFAULT, M_DEFAULT);
      MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_POSITION_IN_PROFILE + M_DRAW_IN_BOX + M_EDGE_SECOND, M_DEFAULT, M_DEFAULT);
      }

   // Draw the edge profile.
   MgraColor(M_DEFAULT, PROFILE_BLUE);
   MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_EDGES_PROFILE + M_DRAW_IN_BOX, M_DEFAULT, M_DEFAULT);

   // Draw the minimum edgevalue threshold.
   MgraColor(M_DEFAULT, M_COLOR_LIGHT_GRAY);
   MmeasDraw(M_DEFAULT, MilMeasMarker, m_MilProfileGraList, M_DRAW_EDGEVALUE_MIN_IN_PROFILE + M_DRAW_IN_BOX, M_DEFAULT, M_DEFAULT);
   
   // Get the intensity profile.
   GetIntensityProfile();

   // Draw the intensity profile.
   MgraControl(M_DEFAULT, M_INPUT_UNITS, M_PIXEL);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MgraLines(M_DEFAULT, m_MilProfileGraList, PROFILE_SIZE_X, m_ProfileValuesPos, m_ProfileValues, M_NULL, M_NULL, M_POLYLINE);

   // Display the profile image.
   MdispSelect(m_MilProfileDisplay, m_MilProfileImage);

   MbufFree(MilImageChild);

   SetUpdate(M_ENABLE);
   }

//*****************************************************************************
// SetUpdate. Set whether to update the display.
//*****************************************************************************
void CProfileDisplay::SetUpdate(MIL_INT Update)
   {
   MdispControl(m_MilProfileDisplay, M_UPDATE, Update);
   }

//*****************************************************************************
// Clear. Clears the display.
//*****************************************************************************
void CProfileDisplay::Clear()
   {
   MdispSelect(m_MilProfileDisplay, M_NULL);
   ClearAnnotations();
   }

//*****************************************************************************
// ClearAnnotations. Clears the annotations.
//*****************************************************************************
void CProfileDisplay::ClearAnnotations()
   {
   MgraClear(M_DEFAULT, m_MilProfileGraList);
   }

//*****************************************************************************
// CreateProfile. Creates the edgevalue profile and intensity profile.
//*****************************************************************************
void CProfileDisplay::GetIntensityProfile()
   {
   MimProjection(m_MilProfileImage, m_MilProjResult, M_0_DEGREE, M_DEFAULT, M_NULL);
   MimGetResult(m_MilProjResult, M_VALUE + M_TYPE_MIL_DOUBLE, m_ProfileValues);
   for(MIL_INT ProfileValueIdx = 0; ProfileValueIdx < PROFILE_SIZE_X; ProfileValueIdx++)
      {
      m_ProfileValues[ProfileValueIdx] = PROFILE_SIZE_Y - m_ProfileValues[ProfileValueIdx] / 255;
      m_ProfileValuesPos[ProfileValueIdx] = (MIL_DOUBLE)ProfileValueIdx;
      }
   }

//*****************************************************************************
// ProfileImageSizeY. Returns the size y of the profile image displayed.
//*****************************************************************************
MIL_INT CProfileDisplay::ProfileImageSizeY() const
   {
   return PROFILE_SIZE_Y;
   }
