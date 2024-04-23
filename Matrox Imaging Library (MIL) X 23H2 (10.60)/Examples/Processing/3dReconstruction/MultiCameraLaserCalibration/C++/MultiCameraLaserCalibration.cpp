//*****************************************************************************/
// 
// File name: MultiCameraLaserCalibration.cpp
// 
// Synopsis:  This program calibrates a multi camera laser system.
//            It also diagnoses the calibration process to validate  
//            the accuracy of the calibration result.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>
#include <vector>
#include <map>
using std::vector;
using std::map;
#include "LaserSystemConfiguration.h"
#include "LaserSystemDiagnostic.h"

/****************************************************************************
Example description.
****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("MultiCameraLaserCalibration\n\n")
             
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to calibrate a multi camera laser\n")
             MIL_TEXT("system. For each step of the calibration, the application\n")
             MIL_TEXT("provides some diagnostics to validate the accuracy of the\n")
             MIL_TEXT("calibration result\n\n")

             MIL_TEXT("Duplicate and change LaserSystemConfiguration.h to test your own \n")
             MIL_TEXT("configuration.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: 3D Map, Application, Buffer, Calibration, Display,\n")
             MIL_TEXT("Graphics, System, 3D Display, 3D Graphics and Image Processing.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Constants.
//*****************************************************************************
static const MIL_INT LASER_PLANES_SLEEP = 250; // in ms
static const MIL_INT MAX_PATH = 256;
static const MIL_INT SINGLE_LABEL_OFFSET = 1000;

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   // Print Header.
   PrintHeader();

   // Allocate the laser system configuration.
   CLaserSysConfig Cfg;

   // Allocate defaults.
   MIL_ID MilApplication = MappAlloc(M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   
   // Allocate the visual diagnostic display.
   CLaserSysDiag* pDiagDisp = new CLaserSysDiag(MilSystem, NB_SYSTEMS);

   // Show the default scanning setup.
   MIL_ID MilSetupDisplay;
   MIL_ID MilSetupImage;
   if(IS_DEFAULT_SCANNING_SYSTEM)
      {
      MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilSetupDisplay);
      MbufRestore(SETUP_IMAGE, MilSystem, &MilSetupImage);
      MdispSelect(MilSetupDisplay, MilSetupImage);
      MosPrintf(MIL_TEXT("An illustration of the setup to calibrate is displayed.\n\n")
                MIL_TEXT("Press <Enter> to continue.\n\n"));
      MosGetch();
      }

   // Allocate the camera calibration objects and calibrate them.
   vector<MIL_ID> MilCal(NB_CAMERAS);
   map<MIL_INT, MIL_ID> MilCalLabelMap;
   for(MIL_INT c = 0; c < NB_CAMERAS; c++)
      {
      // Get a reference to the camera.
      const SCameraCal& rCam = Cfg.CamCal(c);

      MosPrintf(MIL_TEXT("CAMERA%i CALIBRATION\n")
                MIL_TEXT("---------------------\n\n"),
                rCam.CamLabel);

      // Allocate the camera.
      McalAlloc(MilSystem, M_TSAI_BASED, M_DEFAULT, &MilCal[c]);
      MilCalLabelMap[rCam.CamLabel] = MilCal[c];

      // Calibrate the intrinsic parameters of the camera.
      MIL_ID MilGridImage = MbufRestore(CAMERA_INT_PARAMS_IMAGE[c], MilSystem, M_NULL);
      rCam.CalibrateCameraInt(MilCal[c], MilGridImage);

      MosPrintf(MIL_TEXT("The camera was fully calibrated.\n\n"));

      // Diagnose the camera calibration.
      pDiagDisp->DiagnoseCamCal(MilGridImage, MilCal[c]);
      pDiagDisp->UpdateDisplayAndWait();
            
      MbufFree(MilGridImage);
      }

   // Calibrate the extrinsic parameters of the cameras, if necessary.
   for(MIL_INT c = 0; c < NB_CAMERAS; c++)
      {
      // Get a reference to the camera.
      const SCameraCal& rCam = Cfg.CamCal(c);

      if(rCam.pExtrinsicCal != NULL && CAMERA_EXT_PARAMS_IMAGE[c] != NULL)
         {
         MosPrintf(MIL_TEXT("CAMERA%i EXTRINSIC PARAMETERS CALIBRATION\n")
                   MIL_TEXT("-----------------------------------------\n\n"),
                   rCam.CamLabel);

         // Calibrate the extrinsic parameters of the camera.
         MIL_ID MilGridImage = MbufRestore(CAMERA_EXT_PARAMS_IMAGE[c], MilSystem, M_NULL);
         rCam.CalibrateCameraExt(MilCal[c], MilGridImage);

         MosPrintf(MIL_TEXT("The camera or grid was moved and the camera position was\n")
                   MIL_TEXT("recalibrated.\n\n"));

         // Diagnose the camera calibration.
         pDiagDisp->DiagnoseCamCal(MilGridImage, MilCal[c]);
         pDiagDisp->UpdateDisplayAndWait();

         MbufFree(MilGridImage);
         }
      }

   // Allocate the 3D systems arrays.
   vector<MIL_ID> Mil3dmapContextSingle(NB_SYSTEMS);
   vector<MIL_ID> Mil3dmapContextMulti(NB_SYSTEMS);
   vector<MIL_ID> Mil3dmapCalData(NB_SYSTEMS);
   vector<MIL_ID> Mil3dmapCamCal(NB_SYSTEMS);
   vector<MIL_ID> MilAllPlanesImage(NB_SYSTEMS);
   MIL_INT64 DrawLabel = 0;
   // Extract the laser line calibration.
   for(MIL_INT s = 0; s < NB_SYSTEMS; s++)
      {
      // Get a reference to the system.
      const SSingleSystemCal& rSys = Cfg.System(s);

      MosPrintf(MIL_TEXT("SYSTEM CAMERA%i_LASER%i CALIBRATION\n")
                MIL_TEXT("-----------------------------------\n\n"),
                rSys.CamCal.CamLabel,
                rSys.LaserCal.LaserLabel);

      // Allocate the 3dmap systems calibration objects.
      MIL_INT CLabel = M_CAMERA_LABEL(rSys.CamCal.CamLabel);
      MIL_INT LLabel =  M_LASER_LABEL(rSys.LaserCal.LaserLabel);
      MIL_INT SingleLLabel = M_LASER_LABEL(SINGLE_LABEL_OFFSET + rSys.LaserCal.LaserLabel);
      M3dmapAlloc(MilSystem, M_LASER, M_CALIBRATED_CAMERA_LINEAR_MOTION + CLabel + SingleLLabel,
                  &Mil3dmapContextSingle[s]);
      M3dmapAlloc(MilSystem, M_LASER, M_CALIBRATED_CAMERA_LINEAR_MOTION + CLabel + LLabel,
                  &Mil3dmapContextMulti[s]);
      M3dmapAllocResult(MilSystem, M_LASER_CALIBRATION_DATA, M_DEFAULT, &Mil3dmapCalData[s]);

      // Get the identifier of the camera calibration of the single system.
      Mil3dmapCamCal[s] = MilCalLabelMap[rSys.CamCal.CamLabel];

      // Disable the display updates.
      pDiagDisp->Update(M_DISABLE);

      MIL_ID MilCameraImage;
      MIL_ID MilExtractionImage;
      for(MIL_INT p = 0; p < rSys.LaserCal.NbPlanes; p++)
         {
         // Load the image and setup the extraction if extracting with the first plane.
         MIL_TEXT_CHAR ImageFile[MAX_PATH];
         MosSprintf(ImageFile, MAX_PATH, SYS_LASER_CAL_IMAGES, s, p);
         if(p == 0)
            {
            MbufRestore(ImageFile, MilSystem, &MilCameraImage);
            rSys.LaserLineExtraction.SetupLaserLineExtraction(Mil3dmapContextSingle[s]);
            rSys.ExtractionChild.SetupExtractionChild(Mil3dmapContextSingle[s]);
            rSys.ExtractionChild.SetupExtractionChild(Mil3dmapContextMulti[s]);
            rSys.ExtractionChild.AllocExtractionChild(MilCameraImage, &MilExtractionImage);
            MbufClone(MilExtractionImage, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT,
                      M_COPY_SOURCE_DATA, &MilAllPlanesImage[s]);
            }
         else
            {
            MbufLoad(ImageFile, MilCameraImage);
            MimArith(MilExtractionImage, MilAllPlanesImage[s], MilAllPlanesImage[s], M_MAX);
            }

         // Set the corrected depth.
         rSys.LaserCal.SetCalPlane(Mil3dmapContextSingle[s], p);

         // Extract the laser line.
         M3dmapAddScan(Mil3dmapContextSingle[s], Mil3dmapCalData[s], MilExtractionImage,
                       M_NULL, M_NULL, M_DEFAULT, M_DEFAULT);
         
         pDiagDisp->DiagnoseLaserLineExtraction(MilExtractionImage, Mil3dmapCalData[s]);
         pDiagDisp->Update(M_ENABLE);
         if(p == 0)
            {
            MosPrintf(MIL_TEXT("The peaks extracted from the laser line are displayed.\n\n")
                      MIL_TEXT("Press <Enter> to extract lines at other depths.\n\n"));
            MosGetch();
            }
         else
            MosSleep(LASER_PLANES_SLEEP);
         pDiagDisp->Update(M_DISABLE);
         }
      // Free the camera image.
      MbufFree(MilExtractionImage);
      MbufFree(MilCameraImage);

      // Calibrate the single laser system.
      M3dmapCalibrate(Mil3dmapContextSingle[s], Mil3dmapCalData[s], Mil3dmapCamCal[s], M_DEFAULT);
      
      // Diagnose the 3d calibration.
      MosPrintf(MIL_TEXT("The calibration of the single camera-laser system is displayed.\n\n"));
      pDiagDisp->Update(M_DISABLE);
      DrawLabel = pDiagDisp->DiagnoseSingleCalibration(Mil3dmapContextSingle[s], MilAllPlanesImage[s]);
      pDiagDisp->UpdateDisplayAndWait();

      // Hide the 3d display.
      pDiagDisp->Hide3dDisplay();
      pDiagDisp->ClearAll(DrawLabel);
      }

   MosPrintf(MIL_TEXT("MULTI SYSTEM CALIBRATION\n")
             MIL_TEXT("--------------------------\n\n"));

   // Calibrate the complete system.
   M3dmapCalibrateMultiple(&Mil3dmapContextMulti[0], &Mil3dmapCalData[0], &Mil3dmapCamCal[0],
                           NB_SYSTEMS, M_DEFAULT);

   MosPrintf(MIL_TEXT("The calibration of the complete camera-laser system is displayed.\n\n"));

   // Diagnose the full calibration.
   pDiagDisp->DiagnoseFullCalibration(Mil3dmapContextMulti, MilAllPlanesImage);

   // Go interactive.
   pDiagDisp->StartInteractive(Cfg, Mil3dmapContextSingle, Mil3dmapContextMulti,
                               MilAllPlanesImage);
   pDiagDisp->UpdateDisplayAndWait();
   pDiagDisp->EndInteractive();

   // Free camera calibrations.
   for(MIL_INT c = 0; c < NB_CAMERAS; c++)
      McalFree(MilCal[c]);

   // Free systems data.
   for(MIL_INT s = 0; s < NB_SYSTEMS; s++)
      {
      MbufFree(MilAllPlanesImage[s]);
      M3dmapFree(Mil3dmapContextSingle[s]);
      M3dmapFree(Mil3dmapContextMulti[s]);
      M3dmapFree(Mil3dmapCalData[s]);
      }

   if(IS_DEFAULT_SCANNING_SYSTEM)
      {
      MbufFree(MilSetupImage);
      MdispFree(MilSetupDisplay);
      }

   // Delete the diagnostic display.
   delete pDiagDisp;
   
   // Free allocations.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }
