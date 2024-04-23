//***************************************************************************************/
//
// File name: Meas2StepOverview.cpp
//
// Synopsis:  This program illustrates how a 2 step measurement
//            approach can be used to improve the accuracy
//
//            See the PrintHeader() function below for a detailed description.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <math.h>
#include "MeasOverviewExample.h"
#include "ProfileDisplay.h"


///***************************************************************************
// Example description.
///***************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("Meas2StepOverview\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program illustrates how a 2 step measurement approach can be used to\n")
             MIL_TEXT("improve accuracy. The 2 step approach will be performed in the following cases:\n")
             MIL_TEXT("   1. To improve the accuracy of the edge position and angle.\n")
             MIL_TEXT("   2. To improve the accuracy of the stripe width.\n\n")
             
             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphic, image processing, calibration, measurement.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//***************************************************************************
// Example images.
//***************************************************************************
static const MIL_CONST_TEXT_PTR IMAGE_FILE = EXAMPLE_IMAGE_PATH MIL_TEXT("MetalPieceRotatedThinned.tif");

//***************************************************************************
// Example declarations.
//***************************************************************************
void TwoStepPositionAngleAccuracyExample(MIL_ID MilSystem, 
                                         MIL_ID MilDisplay,
                                         MIL_ID MilGraList,
                                         MIL_ID MilImage,
                                         CProfileDisplay* pProfileDisplay);

void TwoStepWidthAccuracyExample(MIL_ID MilSystem, 
                                 MIL_ID MilDisplay,
                                 MIL_ID MilGraList,
                                 MIL_ID MilImage,
                                 CProfileDisplay* pProfileDisplay);

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Allocate the MIL objects.
   MIL_ID MilApplication      = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem           = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplay          = MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID MilGraList = MgraAllocList(MilSystem, M_DEFAULT, M_NULL);

   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList);

   // Allocate the profile display.
   CProfileDisplay* pProfileDisplay = new CProfileDisplay(MilSystem);

   // Move the display.
   MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, pProfileDisplay->ProfileImageSizeY() + WINDOWS_OFFSET_Y);

   // Print Header.
   PrintHeader();

   // Restore the image and calibrate it.
   MIL_ID MilImage = MbufRestore(IMAGE_FILE, MilSystem, M_NULL);
   McalUniform(MilImage, 0.0, 0.0, 1.0, 1.0, 0.0, M_DEFAULT);
   MdispSelect(MilDisplay, MilImage);

   // Start the two step position accuracy case.
   TwoStepPositionAngleAccuracyExample(MilSystem, MilDisplay, MilGraList, MilImage, pProfileDisplay);

   // Clear the annotations.
   MgraClear(M_DEFAULT, MilGraList);
   pProfileDisplay->Clear();

   // Start the two step width accuracy case.
   TwoStepWidthAccuracyExample(MilSystem, MilDisplay, MilGraList, MilImage, pProfileDisplay);

   // Delete the profile display.
   delete pProfileDisplay;

   // Free other allocations.
   MbufFree(MilImage);
   MgraFree(MilGraList);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

//*****************************************************************************
// Constants for position and angle accuracy case.
//*****************************************************************************
static const MIL_DOUBLE ROUGH_BOX_CENTER_X = 242;
static const MIL_DOUBLE ROUGH_BOX_CENTER_Y = 108;
static const MIL_DOUBLE ROUGH_BOX_WIDTH = 104;
static const MIL_DOUBLE ROUGH_BOX_HEIGHT = 286;
static const MIL_DOUBLE FINE_BOX_WIDTH = 30;
static const MIL_DOUBLE ROUGH_BOX_ANGLE = 277.5;
static const MIL_DOUBLE ROUGH_FILTER_SMOOTHNESS = 50;
static const MIL_DOUBLE ROUGH_MAX_ASSOCIATION_DISTANCE = 10;
static const MIL_DOUBLE FINE_MAX_ASSOCIATION_DISTANCE = 3;
static const MIL_DOUBLE NB_SUB_REGIONS = 7;
static const MIL_DOUBLE POSITION_ZOOM = 8;

//*****************************************************************************
// Position and angle accuracy case.
//*****************************************************************************
void TwoStepPositionAngleAccuracyExample(MIL_ID MilSystem, 
                                         MIL_ID MilDisplay,
                                         MIL_ID MilGraList,
                                         MIL_ID MilImage,
                                         CProfileDisplay* pProfileDisplay)
   {
   MosPrintf(MIL_TEXT("1. To improve the accuracy of the edge position and angle.\n\n")
             
             MIL_TEXT("In this case, the edge transition is not exactly aligned\n")
             MIL_TEXT("with the search region. This affects the precision of the position\n")
             MIL_TEXT("and angle found. Using a 2 step measurement approach helps to improve\n")
             MIL_TEXT("both the accuracy of the found position and angle.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();             

   // Allocate the measurement marker.
   MIL_ID MilEdgeMarker = MmeasAllocMarker(MilSystem, M_EDGE, M_DEFAULT, M_NULL);

   // Set up the marker.
   MmeasSetMarker(MilEdgeMarker, M_BOX_CENTER, ROUGH_BOX_CENTER_X, ROUGH_BOX_CENTER_Y);
   MmeasSetMarker(MilEdgeMarker, M_BOX_SIZE, ROUGH_BOX_WIDTH, ROUGH_BOX_HEIGHT);
   MmeasSetMarker(MilEdgeMarker, M_BOX_ANGLE, ROUGH_BOX_ANGLE, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_FILTER_SMOOTHNESS, ROUGH_FILTER_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_POLARITY, M_POSITIVE, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_SUB_REGIONS_NUMBER, NB_SUB_REGIONS, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_MAX_ASSOCIATION_DISTANCE, ROUGH_MAX_ASSOCIATION_DISTANCE, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_SEARCH_REGION_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_DRAW_PROFILE_SCALE_OFFSET, M_AUTO_SCALE_PROFILE, M_DEFAULT);

   // Find the marker.
   MmeasFindMarker(M_DEFAULT, MilImage, MilEdgeMarker, M_DEFAULT);

   // Get the status.
   MIL_INT ValidFlag;
   MmeasGetResult(MilEdgeMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
   if(ValidFlag == M_TRUE)
      {
      // Draw the edge annotation in the image.
      EDGE_DRAW_LIST.DrawList(MilEdgeMarker, MilGraList);

      // Create the profile.
      pProfileDisplay->CreateProfile(MilImage, MilEdgeMarker);

      MosPrintf(MIL_TEXT("The rough position and angle of the edge was found.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Get the position and angle of the marker.
      MIL_DOUBLE RoughPosX;
      MIL_DOUBLE RoughPosY;
      MIL_DOUBLE RoughAngle;
      MmeasGetResult(MilEdgeMarker, M_POSITION, &RoughPosX, &RoughPosY);
      MmeasGetResult(MilEdgeMarker, M_ANGLE, &RoughAngle, M_NULL);

      // Move the box at the rough location.
      MmeasSetMarker(MilEdgeMarker, M_BOX_CENTER, RoughPosX, RoughPosY);
      MmeasSetMarker(MilEdgeMarker, M_BOX_SIZE, FINE_BOX_WIDTH, ROUGH_BOX_HEIGHT);
      MmeasSetMarker(MilEdgeMarker, M_BOX_ANGLE, RoughAngle-90, M_NULL);
      MmeasSetMarker(MilEdgeMarker, M_MAX_ASSOCIATION_DISTANCE, FINE_MAX_ASSOCIATION_DISTANCE, M_NULL);

      // Find the edge with high precision.
      MmeasFindMarker(M_DEFAULT, MilImage, MilEdgeMarker, M_DEFAULT);

      // Get the status.
      MmeasGetResult(MilEdgeMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
      if(ValidFlag == M_TRUE)
         {
         // Draw the edge annotations in the image.
         EDGE_DRAW_LIST.DrawList(MilEdgeMarker, MilGraList);

         // Create the profile.
         pProfileDisplay->ClearAnnotations();
         pProfileDisplay->CreateProfile(MilImage, MilEdgeMarker);

         // Get the position and angle of the marker.
         MIL_DOUBLE FinePosX;
         MIL_DOUBLE FinePosY;
         MIL_DOUBLE FineAngle;
         MmeasGetResult(MilEdgeMarker, M_POSITION, &FinePosX, &FinePosY);
         MmeasGetResult(MilEdgeMarker, M_ANGLE, &FineAngle, M_NULL);

         MosPrintf(MIL_TEXT("The precise position and angle of the edge was found\n")
                   MIL_TEXT("in a second region whose position is based on the rough edge found.\n\n")
                   MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();

         // Print the result.
         MosPrintf(MIL_TEXT("          |-------------------|-------------------|\n")
                   MIL_TEXT("          |       Rough       |      Refined      |\n")
                   MIL_TEXT("|---------|-------------------|-------------------|\n")
                   MIL_TEXT("|   Pos   |  (%6.2f,%-6.2f)  |  (%6.2f,%-6.2f)  |\n")
                   MIL_TEXT("|---------|-------------------|-------------------|\n")
                   MIL_TEXT("|  Angle  |%12.2f       |%12.2f       |\n")
                   MIL_TEXT("|-----------------------------|-------------------|\n\n"),
                   RoughPosX, RoughPosY, FinePosX, FinePosY,
                   RoughAngle, FineAngle);
         }
      }

   if(ValidFlag == M_FALSE)
      MosPrintf(MIL_TEXT("Unable to find the marker...\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the marker.
   MmeasFree(MilEdgeMarker);
   };


//***************************************************************************
// Constants for width and accuracy case.
//***************************************************************************
static const MIL_DOUBLE STRIPE_BOX_CENTER_X = 269;
static const MIL_DOUBLE STRIPE_BOX_CENTER_Y = 175;
static const MIL_DOUBLE STRIPE_BOX_WIDTH = 72;
static const MIL_DOUBLE STRIPE_BOX_HEIGHT = 29;
static const MIL_DOUBLE STRIPE_BOX_ANGLE = 5;
static const MIL_DOUBLE STRIPE_FILTER_SMOOTHNESS= 90;
static const MIL_DOUBLE DISPLAY_ZOOM = 16;

//***************************************************************************
// Width accuracy case.
//***************************************************************************
void TwoStepWidthAccuracyExample(MIL_ID MilSystem, 
                                 MIL_ID MilDisplay,
                                 MIL_ID MilGraList,
                                 MIL_ID MilImage,
                                 CProfileDisplay* pProfileDisplay)
                                 
   {
   MosPrintf(MIL_TEXT("2. To improve the accuracy of the stripe width.\n\n")
             
             MIL_TEXT("In this case, a really thin stripe is found using a marker whose\n")
             MIL_TEXT("smoothness parameter, given the size of the stripe, causes edge\n")
             MIL_TEXT("displacement. The width is then overestimated. By using a second step to\n")
             MIL_TEXT("measure each edge individually, a more accurate width can be calculated.\n\n")
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch(); 

   // Get the size of the image.
   MIL_INT ImageSizeX = MbufInquire(MilImage, M_SIZE_X, M_NULL);
   MIL_INT ImageSizeY = MbufInquire(MilImage, M_SIZE_Y, M_NULL);

   // Allocate the stripe measurement marker.
   MIL_ID MilStripeMarker = MmeasAllocMarker(MilSystem, M_STRIPE, M_DEFAULT, M_NULL);
   MIL_ID MilEdgeMarker = MmeasAllocMarker(MilSystem, M_EDGE, M_DEFAULT, M_NULL);

   // Set up the marker.
   MmeasSetMarker(MilStripeMarker, M_BOX_CENTER, STRIPE_BOX_CENTER_X, STRIPE_BOX_CENTER_Y);
   MmeasSetMarker(MilStripeMarker, M_BOX_SIZE, STRIPE_BOX_WIDTH, STRIPE_BOX_HEIGHT);
   MmeasSetMarker(MilStripeMarker, M_BOX_ANGLE, STRIPE_BOX_ANGLE, M_NULL);
   MmeasSetMarker(MilStripeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilStripeMarker, M_FILTER_SMOOTHNESS, STRIPE_FILTER_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilStripeMarker, M_SEARCH_REGION_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilStripeMarker, M_DRAW_PROFILE_SCALE_OFFSET, M_AUTO_SCALE_PROFILE, M_DEFAULT);
   MmeasSetMarker(MilEdgeMarker, M_FILTER_TYPE, M_SHEN, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_FILTER_SMOOTHNESS, STRIPE_FILTER_SMOOTHNESS, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_SEARCH_REGION_INPUT_UNITS, M_WORLD, M_NULL);
   MmeasSetMarker(MilEdgeMarker, M_DRAW_PROFILE_SCALE_OFFSET, M_AUTO_SCALE_PROFILE, M_DEFAULT);
   
   // Find the marker.
   MmeasFindMarker(M_DEFAULT, MilImage, MilStripeMarker, M_DEFAULT);

   // Get the status.
   MIL_INT ValidFlag;
   MmeasGetResult(MilStripeMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
   if(ValidFlag == M_TRUE)
      {
      // Draw the edge annotations in the image.
      STRIPE_SIMPLE_DRAW_LIST.DrawList(MilStripeMarker, MilGraList);

      // Create the profile.
      pProfileDisplay->CreateProfile(MilImage, MilStripeMarker);

      MosPrintf(MIL_TEXT("The position and width of the stripe was found.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();

      // Get the position of the edges and the angle of the stripe.
      MIL_DOUBLE StripePosX;
      MIL_DOUBLE StripePosY;
      MIL_DOUBLE EdgePosX[2];
      MIL_DOUBLE EdgePosY[2];
      MIL_DOUBLE StripeEdgeStartX[2];
      MIL_DOUBLE StripeEdgeStartY[2];
      MIL_DOUBLE StripeEdgeEndX[2];
      MIL_DOUBLE StripeEdgeEndY[2];
      MIL_DOUBLE StripeAngle;
      MIL_DOUBLE StripeWidth;
      MmeasGetResult(MilStripeMarker, M_POSITION, &StripePosX, &StripePosY);
      MmeasGetResult(MilStripeMarker, M_EDGE_START + M_EDGE_FIRST, &StripeEdgeStartX[0], &StripeEdgeStartY[0]);
      MmeasGetResult(MilStripeMarker, M_EDGE_START + M_EDGE_SECOND, &StripeEdgeStartX[1], &StripeEdgeStartY[1]);
      MmeasGetResult(MilStripeMarker, M_EDGE_END + M_EDGE_FIRST, &StripeEdgeEndX[0], &StripeEdgeEndY[0]);
      MmeasGetResult(MilStripeMarker, M_EDGE_END + M_EDGE_SECOND, &StripeEdgeEndX[1], &StripeEdgeEndY[1]);
      MmeasGetResult(MilStripeMarker, M_ANGLE, &StripeAngle, M_NULL);
      MmeasGetResult(MilStripeMarker, M_STRIPE_WIDTH, &StripeWidth, M_NULL);

      // Find the first edge accurately. The box width is from the start of the first transition 
      // to the start of the second transition. One pixel is added on each side to have enough data.
      MIL_DOUBLE Dx = StripeEdgeStartX[1] - StripeEdgeStartX[0];
      MIL_DOUBLE Dy = StripeEdgeStartY[1] - StripeEdgeStartY[0];
      MIL_DOUBLE BoxWidth = sqrt(Dx*Dx + Dy*Dy) + 2;
      MmeasSetMarker(MilEdgeMarker, M_BOX_SIZE, BoxWidth, STRIPE_BOX_HEIGHT);
      MmeasSetMarker(MilEdgeMarker, M_BOX_CENTER, (StripeEdgeStartX[0] + StripeEdgeStartX[1])/2, (StripeEdgeStartY[0] + StripeEdgeStartY[1])/2);
      MmeasSetMarker(MilEdgeMarker, M_BOX_ANGLE, StripeAngle-90, M_NULL);
      MmeasFindMarker(M_DEFAULT, MilImage, MilEdgeMarker, M_DEFAULT);

      // Get the status.
      MmeasGetResult(MilEdgeMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
      if(ValidFlag == M_TRUE)
         {
         EDGE_DRAW_LIST.DrawList(MilEdgeMarker, MilGraList);

         // Create the profile.
         pProfileDisplay->ClearAnnotations();
         pProfileDisplay->CreateProfile(MilImage, MilEdgeMarker);

         // Get the position of the first edge.
         MmeasGetResult(MilEdgeMarker, M_POSITION, &EdgePosX[0], &EdgePosY[0]);

         // Zoom on the stripe.
         MdispZoom(MilDisplay, DISPLAY_ZOOM, DISPLAY_ZOOM);
         MdispPan(MilDisplay, StripePosX - ImageSizeX*0.5/DISPLAY_ZOOM, StripePosY - ImageSizeY*0.5/DISPLAY_ZOOM);

         MosPrintf(MIL_TEXT("The first edge position was refined.\n\n")
                   MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();

         // Find the second edge accurately.
         Dx = StripeEdgeEndX[1] - StripeEdgeEndX[0];
         Dy = StripeEdgeEndY[1] - StripeEdgeEndY[0];
         BoxWidth = sqrt(Dx*Dx + Dy*Dy) + 2;
         MmeasSetMarker(MilEdgeMarker, M_BOX_SIZE, BoxWidth, STRIPE_BOX_HEIGHT);
         MmeasSetMarker(MilEdgeMarker, M_BOX_CENTER, (StripeEdgeEndX[0] + StripeEdgeEndX[1])/2, (StripeEdgeEndY[0] + StripeEdgeEndY[1])/2);
         MmeasFindMarker(M_DEFAULT, MilImage, MilEdgeMarker, M_DEFAULT);

         // Get the status.
         MmeasGetResult(MilEdgeMarker, M_VALID_FLAG + M_TYPE_MIL_INT, &ValidFlag, M_NULL);
         if(ValidFlag == M_TRUE)
            {
            EDGE_DRAW_LIST.DrawList(MilEdgeMarker, MilGraList);
            
            // Create the profile.
            pProfileDisplay->ClearAnnotations();
            pProfileDisplay->CreateProfile(MilImage, MilEdgeMarker);

            // Get the position of the second edge.
            MmeasGetResult(MilEdgeMarker, M_POSITION, &EdgePosX[1], &EdgePosY[1]);
            Dx = EdgePosX[1] - EdgePosX[0];
            Dy = EdgePosY[1] - EdgePosY[0];
            MIL_DOUBLE FineStripeWidth = sqrt(Dx*Dx + Dy*Dy);

            MosPrintf(MIL_TEXT("The second edge position was refined.\n\n")
                      MIL_TEXT("Press <Enter> to continue.\n\n"));
            MosGetch();

            // Print the result.
            MosPrintf(MIL_TEXT("          |-------------------|-------------------|\n")
                      MIL_TEXT("          |       Rough       |      ReFined      |\n")
                      MIL_TEXT("|---------|-------------------|-------------------|\n")
                      MIL_TEXT("|  Width  |%12.2f       |%12.2f       |\n")
                      MIL_TEXT("|---------|-------------------|-------------------|\n\n"),
                      StripeWidth, FineStripeWidth);
            }
         }
      }

    
   if(ValidFlag == M_FALSE)
      MosPrintf(MIL_TEXT("Unable to find the marker...\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Free the marker.
   MmeasFree(MilEdgeMarker);
   MmeasFree(MilStripeMarker);
   }
