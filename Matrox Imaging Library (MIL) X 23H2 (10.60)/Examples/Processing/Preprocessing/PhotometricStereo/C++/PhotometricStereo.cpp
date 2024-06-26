﻿//////////////////////////////////////////////////////////////////////////////////////////
// 
// File name: PhotometricStereo.cpp
// 
// Synopsis:  This program demonstrates use cases of surface albedo and curvature
//            in surface defect detection. Multiple images of the same object are captured,
//            each of which is taken from a different incident light angle.
// 
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <mil.h>
#include <vector>

#define IMAGE_DIR M_IMAGE_PATH MIL_TEXT("PhotometricStereo")


// Lighting information.
const MIL_INT NB_USE_CASE = 2;
const MIL_INT NB_IMG      = 4;
const MIL_DOUBLE ZENITH_ANGLE [NB_IMG] = { 65.92, 67.22, 66.80, 67.46 }; // in degrees
const MIL_DOUBLE AZIMUTH_ANGLE[NB_IMG] = { 0.0,   90.0,  180.0, 270.0 }; // in degrees
const MIL_DOUBLE LIGHT_NORM   [NB_IMG] = { 1.0,   1.0,   1.0,   1.0   };

static MIL_CONST_TEXT_PTR ILLUSTRATION_PATH = IMAGE_DIR MIL_TEXT("/LightOrientations.png");
static const MIL_INT ILLUSTRATION_DISPLAY_OFFSET_X = 660;

// Example description.                                                     
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("PhotometricStereo\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates the use of photometric stereo technology  \n")
             MIL_TEXT("for defect detection by capturing multiple images of the same object\n")
             MIL_TEXT("taken from different incident light angles.                         \n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, display, image processing,       \n")
             MIL_TEXT("registration, blob analysis, system.                                \n"));
   }

// Functions for each use case.
void ComputeAlbedoForDetection      (   MIL_ID               MilSysId, 
                                        MIL_ID               PsContextId,     
                                        std::vector<MIL_ID>* pImgVect, 
                                        MIL_ID*              pAlbedoId   );

MIL_INT ComputeCurvatureForDetection(   MIL_ID               MilSysId, 
                                        MIL_ID               PsContextId,  
                                        std::vector<MIL_ID>* pImgVect, 
                                        MIL_ID*              pCurvatureId);

void ComputeLocalShape              (   MIL_ID               MilSysId, 
                                        MIL_ID               PsContextId,  
                                        std::vector<MIL_ID>* pImgVect, 
                                        MIL_ID*              pLocalShapeId);

void AlbedoDefectExtraction         (      MIL_ID               MilSysId, 
                                           MIL_ID               MilDispId, 
                                     const std::vector<MIL_ID>& rImgVect, 
                                           MIL_ID               AlbedoId);
                                    
void CurvatureDefectExtraction      (      MIL_ID               MilSysId, 
                                           MIL_ID               MilDispId, 
                                     const std::vector<MIL_ID>& rImgVect, 
                                           MIL_ID               CurvatureId);

MIL_INT ComputeLocalShapesWithConstDrawRange(MIL_ID                            MilSysId,
                                             MIL_ID                            PsContextId,
                                             std::vector<std::vector<MIL_ID>*> VpImgVect,
                                             std::vector < MIL_ID* >           VpLocalShapeId,
                                             MIL_ID*                           pLocalShapeControlledRangeId);

void ShowResult                  (     MIL_ID               MilSysId, 
                                       MIL_ID               MilDisplayId,
                                       MIL_CONST_TEXT_PTR   pDesc, 
                                       std::vector<MIL_ID>* pImgVect,
                                       MIL_ID*              pAlbedoId, 
                                       MIL_ID*              pCurvatureId,
                                       MIL_ID*              pLocalShapeId);

void FreeImageBuffers            (     std::vector<MIL_ID>* pImgVect);

//////////////////////////////////////////////////////////////////////////////////////////
int MosMain()
   {
   PrintHeader();

   // Allocate MIL objects.
   MIL_ID MilApplicationId = MappAlloc (M_DEFAULT, M_NULL);
   MIL_ID MilSystemId      = MsysAlloc (M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID MilDisplayId     = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(MilDisplayId, M_VIEW_MODE, M_AUTO_SCALE);

   // Show illustration of light orientations.
   MIL_ID IllustrationDispId  = MdispAlloc(MilSystemId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MIL_ID IllustrationImageId = MbufRestore(ILLUSTRATION_PATH, MilSystemId, M_NULL);
   MdispControl(IllustrationDispId, M_TITLE, MIL_TEXT("Light orientations"));
   MdispControl(IllustrationDispId, M_WINDOW_INITIAL_POSITION_X, ILLUSTRATION_DISPLAY_OFFSET_X);
   MdispSelect(IllustrationDispId, IllustrationImageId);

   // Set lighting info to photometric stereo context.
   MIL_ID PsContextId = MregAlloc(MilSystemId, M_PHOTOMETRIC_STEREO, M_DEFAULT, M_NULL);

   // Disable timeout.
   MregControl(PsContextId, M_CONTEXT, M_TIMEOUT, M_DISABLE);

   MregControl(PsContextId, M_ALL, M_LIGHT_VECTOR_TYPE, M_SPHERICAL);
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_1, ZENITH_ANGLE [i]);
      MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_2, AZIMUTH_ANGLE[i]);
      MregControl(PsContextId, i, M_LIGHT_VECTOR_COMPONENT_3, LIGHT_NORM   [i]);
      }

   // Iterate through each use case.
   std::vector<MIL_ID> ImgVect(NB_IMG, M_NULL);
   MIL_ID AlbedoId     = M_NULL;
   MIL_ID CurvatureId  = M_NULL;
   MIL_ID LocalShapeId = M_NULL;
   MIL_CONST_TEXT_PTR CaseDescPtr = 0;
   MIL_INT Status = -1;

   // Compute albedo for defect detection
   MosPrintf    (MIL_TEXT("\n(1) Using surface albedo technology for leather defect detection:   \n")
                 MIL_TEXT(  "--------------------------------------------------------------------\n"));
   CaseDescPtr = MIL_TEXT(  "Surface albedo is able to capture material reflection variations,   \n")
                 MIL_TEXT(  "therefore, for this example image of textured leather, defects are  \n")
                 MIL_TEXT(  "more detectable in the albedo image compared to intensity version.  \n");
   ComputeAlbedoForDetection(MilSystemId, PsContextId, &ImgVect, &AlbedoId);
   ShowResult(MilSystemId, MilDisplayId, CaseDescPtr, &ImgVect, &AlbedoId, &CurvatureId, &LocalShapeId);
   MosPrintf(MIL_TEXT("\n"));

   // Compute curvature for defect detection
   MosPrintf    (MIL_TEXT("\n(2) Using surface curvature technology for package defect detection:\n")
                 MIL_TEXT(  "--------------------------------------------------------------------\n"));
   CaseDescPtr = MIL_TEXT(  "Defects on a smooth surface often lead to abrupt changes in surface \n")
                 MIL_TEXT(  "curvature values. Therefore, for this example image of a smooth     \n")
                 MIL_TEXT(  "surface, defects are more defectable in the curvature version of the\n")
                 MIL_TEXT(  "image, compared to the intensity version, which is more vulnerable  \n")
                 MIL_TEXT(  "to lighting conditions.                                             \n");
   Status = ComputeCurvatureForDetection(MilSystemId, PsContextId, &ImgVect, &CurvatureId);

   if(Status == M_COMPLETE)
      {
      ShowResult(MilSystemId, MilDisplayId, CaseDescPtr, &ImgVect, &AlbedoId, &CurvatureId, &LocalShapeId);
      MosPrintf(MIL_TEXT("\n"));
      }
   else
      {
      // Free image buffers.
      FreeImageBuffers(&ImgVect);
      MosPrintf(MIL_TEXT("\n MregCalculate didn't complete. Check result status value for more informations.\n"));
      }

   // Compute local shape for structural content extraction
   MosPrintf    (MIL_TEXT("\n(3) Using local shape technology for structural content extraction: \n")
                 MIL_TEXT(  "------------------------------------------------------------------- \n"));
   CaseDescPtr = MIL_TEXT(  "Local shape images capture changes on an object's surface. Such     \n")
                 MIL_TEXT(  "changes usually cause difficulties for single lighting image        \n")
                 MIL_TEXT(  "acquisition, due to either specularity on the reflecting surface, or\n")
                 MIL_TEXT(  "shadows cast by surface variations. Local shape images benefit from \n")
                 MIL_TEXT(  "multiple acquisitions to extract structural content, which can      \n")
                 MIL_TEXT(  "facilitate further image analysis.                                  \n");
   ComputeLocalShape(MilSystemId, PsContextId, &ImgVect, &LocalShapeId);
   ShowResult(MilSystemId, MilDisplayId, CaseDescPtr, &ImgVect, &AlbedoId, &CurvatureId, &LocalShapeId);
   MosPrintf(MIL_TEXT("\n"));


   MIL_ID LocalShape_first  = M_NULL;
   MIL_ID LocalShape_second_auto  = M_NULL;
   MIL_ID LocalShape_second_Controlled = M_NULL;
   std::vector<MIL_ID> ImgVect_first (NB_IMG, M_NULL);
   std::vector<MIL_ID> ImgVect_second(NB_IMG, M_NULL);

   std::vector<std::vector<MIL_ID>*> VpImgVect(2, M_NULL);
   VpImgVect[0] = &ImgVect_first;
   VpImgVect[1] = &ImgVect_second;

   std::vector < MIL_ID* >  VpLocalShapeId(2, M_NULL);
   VpLocalShapeId[0] = &LocalShape_first;
   VpLocalShapeId[1] = &LocalShape_second_auto;


   // Compute local shape with constant draw range
   MosPrintf    (MIL_TEXT("\n(4) Compute local shape with constant draw range:                   \n")
                 MIL_TEXT(  "--------------------------------------------------------------------\n"));
   Status = ComputeLocalShapesWithConstDrawRange(MilSystemId, PsContextId, VpImgVect, VpLocalShapeId, &LocalShape_second_Controlled);

   if(Status == M_COMPLETE)
      {
      CaseDescPtr = MIL_TEXT("First local shape drawn with auto remap factor.\n");
      ShowResult(MilSystemId, MilDisplayId, CaseDescPtr, &ImgVect_first, &AlbedoId, &CurvatureId, &LocalShape_first);

      CaseDescPtr = MIL_TEXT("Second local shape drawn with auto remap factor.\n");
      ShowResult(MilSystemId, MilDisplayId, CaseDescPtr, &ImgVect_second, &AlbedoId, &CurvatureId, &LocalShape_second_auto);

      MosPrintf(MIL_TEXT("\n"));
      MosPrintf(MIL_TEXT("Second local shape drawn with first local shape remap factor.\n"));
      MdispSelect(MilDisplayId, LocalShape_second_Controlled);
      MosPrintf(MIL_TEXT("Press any key to continue...\n"));
      MosGetch();
      }
   else
      {
      FreeImageBuffers(&ImgVect_first);
      FreeImageBuffers(&ImgVect_second);
      MosPrintf(MIL_TEXT("\n MregCalculate didn't complete. Check result status value for more informations.\n"));
      }
   

   // Free local shape second fixed.
   MbufFree(LocalShape_second_Controlled);

   MosPrintf(MIL_TEXT("\n"));

   // Free MIL objects.
   MdispFree(IllustrationDispId);
   MbufFree(IllustrationImageId);

   MregFree (PsContextId);
   MdispFree(MilDisplayId);
   MsysFree (MilSystemId);
   MappFree (MilApplicationId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Compute albedo for surface defect detection.
void ComputeAlbedoForDetection(MIL_ID               MilSysId, 
                               MIL_ID               PsContextId, 
                               std::vector<MIL_ID>* pImgVect, 
                               MIL_ID*              pAlbedo)
   {
   std::vector<MIL_ID>& ImgVect  = *pImgVect;
   MIL_ID&              AlbedoId = *pAlbedo;

   // Get image samples.
   MIL_TEXT_CHAR ImgName[256];
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {      
      MosSprintf (ImgName, 256, MIL_TEXT("%s/Leather_%03d.mim"), IMAGE_DIR, (long)(AZIMUTH_ANGLE[i]));
      MbufRestore(ImgName, MilSysId, &ImgVect[i]);
      }

   // Compute albedo
   MIL_INT ImgSizeX = MbufInquire(ImgVect[0], M_SIZE_X, M_NULL);
   MIL_INT ImgSizeY = MbufInquire(ImgVect[0], M_SIZE_Y, M_NULL);
   MbufAlloc2d(MilSysId, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &AlbedoId);
   MbufClear(AlbedoId, 0.0);
   MregControl(PsContextId, M_CONTEXT, M_DRAW_WITH_NO_RESULT, M_DRAW_ALBEDO_IMAGE);
   MregCalculate(PsContextId, &ImgVect[0], AlbedoId, NB_IMG, M_DEFAULT);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Compute curvature for defect detection.
MIL_INT ComputeCurvatureForDetection(MIL_ID               MilSysId, 
                                     MIL_ID               PsContextId, 
                                     std::vector<MIL_ID>* pImgVect, 
                                     MIL_ID*              pCurvatureId)
   {
   std::vector<MIL_ID>& ImgVect     = *pImgVect;
   MIL_ID&              CurvatureId = *pCurvatureId;

   // Get image samples.
   MIL_TEXT_CHAR ImgName[256];
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MosSprintf(ImgName, 256, MIL_TEXT("%s/Matroxlogo_%03d.mim"), IMAGE_DIR, (long)(AZIMUTH_ANGLE[i]));
      MbufRestore(ImgName, MilSysId, &ImgVect[i]);
      }

   // Allocate curvature image
   MIL_INT ImgSizeX = MbufInquire(ImgVect[0], M_SIZE_X, M_NULL);
   MIL_INT ImgSizeY = MbufInquire(ImgVect[0], M_SIZE_Y, M_NULL);
   MbufAlloc2d(MilSysId, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &CurvatureId);
   MbufClear(CurvatureId, 0.0);

   // Set compute parameter
   MIL_ID PsRstId = MregAllocResult(MilSysId, M_PHOTOMETRIC_STEREO_RESULT, M_NULL);
   MregControl(PsContextId, M_CONTEXT, M_GAUSSIAN_CURVATURE, M_ENABLE);
   MregCalculate(PsContextId, &ImgVect[0], PsRstId, NB_IMG, M_DEFAULT);

   //  Check if Calculate completed correctly 
   MIL_INT Status;
   MregGetResult(PsRstId, M_DEFAULT, M_STATUS+M_TYPE_MIL_INT, &Status);

   if(Status==M_COMPLETE)
      {
      MregDraw(M_DEFAULT, PsRstId, CurvatureId, M_DRAW_GAUSSIAN_CURVATURE_IMAGE, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      // Free curvature.
      MbufFree(CurvatureId);
      CurvatureId = M_NULL;
      }

   MregFree(PsRstId);
   return Status;
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Defect extraction from albedo result.
void AlbedoDefectExtraction(MIL_ID                     MilSysId, 
                            MIL_ID                     MilDispId, 
                            const std::vector<MIL_ID>& rImgVect, 
                            MIL_ID                     AlbedoId)
   {
   // Compute threshold value
   MIL_DOUBLE AlbedoMeanVal = 0.0;
   MIL_DOUBLE AlbedoStdVal  = 0.0;
   MIL_ID StatCntxId = MimAlloc      (MilSysId, M_STATISTICS_CONTEXT, M_DEFAULT,           M_NULL);
   MIL_ID StatRstId  = MimAllocResult(MilSysId, M_DEFAULT,            M_STATISTICS_RESULT, M_NULL);
   MimControl(StatCntxId, M_STAT_MEAN,               M_ENABLE);
   MimControl(StatCntxId, M_STAT_STANDARD_DEVIATION, M_ENABLE);
   MimStatCalculate(StatCntxId, AlbedoId, StatRstId, M_DEFAULT);
   MimGetResult(StatRstId, M_STAT_MEAN + M_TYPE_MIL_DOUBLE,               &AlbedoMeanVal);
   MimGetResult(StatRstId, M_STAT_STANDARD_DEVIATION + M_TYPE_MIL_DOUBLE, &AlbedoStdVal);
   MIL_DOUBLE Alpha = 3.0;
   MIL_DOUBLE ThreshVal = AlbedoMeanVal - Alpha * AlbedoStdVal;   // threshold = mean - 3*sigma
   MimFree(StatRstId);
   MimFree(StatCntxId);
   
   // Detect the defection.
   MimBinarize(AlbedoId, AlbedoId, M_GREATER, ThreshVal, M_NULL);
   MimErode(AlbedoId, AlbedoId, 3, M_GRAYSCALE);

   // Compute blob info.
   MIL_ID BlobCntxId = MblobAlloc      (MilSysId, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID BlobRstId  = MblobAllocResult(MilSysId, M_DEFAULT, M_DEFAULT, M_NULL);

   MblobControl(BlobCntxId, M_FOREGROUND_VALUE, M_ZERO); 
   MblobCalculate(BlobCntxId, AlbedoId, M_NULL, BlobRstId);
   MdispSelect(MilDispId, rImgVect[0]);
   
   MIL_ID DispOverlayId = MdispInquire(MilDispId, M_OVERLAY_ID, M_NULL);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MblobDraw(M_DEFAULT, BlobRstId, DispOverlayId, M_DRAW_BLOBS_CONTOUR, M_DEFAULT, M_DEFAULT);

   MosPrintf(MIL_TEXT("\nExtracted defects are displayed on one of the acquired images."));
   MosPrintf(MIL_TEXT("\nPress any key to continue...\n"));   
   MosGetch();
   MdispControl(MilDispId, M_OVERLAY_CLEAR, M_DEFAULT);

   // Free local objects.
   MblobFree(BlobRstId);
   MblobFree(BlobCntxId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Defect extraction from curvature result
void CurvatureDefectExtraction(MIL_ID                     MilSysId, 
                               MIL_ID                     MilDispId, 
                               const std::vector<MIL_ID>& rImgVect, 
                               MIL_ID                     CurvatureId)
   {
   // Specify curvature standard variation.
   MIL_DOUBLE CurvatureStd = 0.0;
   MIL_ID StatRstId  = MimAllocResult(MilSysId, M_DEFAULT,            M_STATISTICS_RESULT, M_NULL);
   MimStatCalculate(M_STAT_CONTEXT_STANDARD_DEVIATION, CurvatureId, StatRstId, M_DEFAULT);
   MimGetResult(StatRstId, M_STAT_STANDARD_DEVIATION + M_TYPE_MIL_DOUBLE, &CurvatureStd);
   MimFree(StatRstId);

   // Ignore small curvature variation, and take most dominant surface changes.
   MIL_DOUBLE BuffMaxVal    = (MIL_DOUBLE)MbufInquire(CurvatureId, M_MAX, M_NULL);
   MIL_DOUBLE BuffMinVal    = (MIL_DOUBLE)MbufInquire(CurvatureId, M_MIN, M_NULL);
   MIL_DOUBLE BuffMiddleVal = BuffMinVal + (BuffMaxVal - BuffMinVal) * 0.5;
   MIL_DOUBLE ThreshCoef    = 3.0;
   MIL_DOUBLE ThreshMinVal  = BuffMiddleVal - ThreshCoef * CurvatureStd;
   MIL_DOUBLE ThreshMaxVal  = BuffMiddleVal + ThreshCoef * CurvatureStd;
   MimBinarize(CurvatureId, CurvatureId, M_OUT_RANGE, ThreshMinVal, ThreshMaxVal);
   MimOpen  (CurvatureId, CurvatureId, 1, M_GRAYSCALE);
   MimDilate(CurvatureId, CurvatureId, 3, M_GRAYSCALE);

   // Compute blob info.
   MIL_ID BlobCntxId = MblobAlloc      (MilSysId, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID BlobRstId  = MblobAllocResult(MilSysId, M_DEFAULT, M_DEFAULT, M_NULL);
   MblobControl  (BlobCntxId, M_BOX,     M_ENABLE);
   MblobCalculate(BlobCntxId, CurvatureId, M_NULL, BlobRstId);
   MdispSelect(MilDispId, rImgVect[0]);

   MIL_ID DispOverlayId = MdispInquire(MilDispId, M_OVERLAY_ID, M_NULL);
   MgraColor(M_DEFAULT, M_COLOR_RED);
   MblobDraw(M_DEFAULT, BlobRstId, DispOverlayId, M_DRAW_BOX, M_DEFAULT, M_DEFAULT);
   
   MosPrintf(MIL_TEXT("\nExtracted defects are displayed on one of the acquired images."));
   MosPrintf(MIL_TEXT("\nPress any key to continue...\n"));   
   MosGetch();
   MdispControl(MilDispId, M_OVERLAY_CLEAR, M_DEFAULT);

   // Free local objects.
   MblobFree(BlobRstId);
   MblobFree(BlobCntxId);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Compute local shape for structural content extraction.
void ComputeLocalShape(MIL_ID               MilSysId, 
                       MIL_ID               PsContextId, 
                       std::vector<MIL_ID>* pImgVect, 
                       MIL_ID*              pLocalShape)
   {
   std::vector<MIL_ID>& ImgVect  = *pImgVect;
   MIL_ID&              LocalShapeId = *pLocalShape;

   // Get image samples.
   MIL_TEXT_CHAR ImgName[256];
   for (MIL_INT i = 0; i < NB_IMG; ++i)
      {      
      MosSprintf (ImgName, 256, MIL_TEXT("%s/PlasticAdapter_%03d.mim"), IMAGE_DIR, (long)(AZIMUTH_ANGLE[i]));
      MbufRestore(ImgName, MilSysId, &ImgVect[i]);
      }

   // Compute local shape.
   MIL_INT ImgSizeX = MbufInquire(ImgVect[0], M_SIZE_X, M_NULL);
   MIL_INT ImgSizeY = MbufInquire(ImgVect[0], M_SIZE_Y, M_NULL);
   MbufAlloc2d(MilSysId, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &LocalShapeId);
   MbufClear(LocalShapeId, 0.0);
   MregControl(PsContextId, M_CONTEXT, M_DRAW_WITH_NO_RESULT,M_DRAW_LOCAL_SHAPE_IMAGE);
   MregCalculate(PsContextId, &ImgVect[0], LocalShapeId, NB_IMG, M_DEFAULT);
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Compute local shape with constant draw range.
MIL_INT ComputeLocalShapesWithConstDrawRange(MIL_ID                            MilSysId,
                                             MIL_ID                            PsContextId,
                                             std::vector<std::vector<MIL_ID>*> VpImgVect,
                                             std::vector < MIL_ID* >           VpLocalShapeId,
                                             MIL_ID*                           pLocalShapeControlledFactorId)
   {
   MIL_TEXT_CHAR ImgName[256];
   MIL_INT ImgSizeX;
   MIL_INT ImgSizeY;
   MIL_DOUBLE AutoFactor;

   MIL_ID PsRstId = MregAllocResult(MilSysId, M_PHOTOMETRIC_STEREO_RESULT, M_NULL);
   MregControl(PsContextId, M_CONTEXT, M_LOCAL_SHAPE, M_ENABLE);

   for(MIL_INT Indice = 0; Indice < MIL_INT(VpImgVect.size()); Indice++)
      {
      std::vector<MIL_ID>& ImgVect      = *VpImgVect[Indice];
      MIL_ID&              LocalShapeId = *VpLocalShapeId[Indice];

      // Get image samples.
      for(MIL_INT i = 0; i < NB_IMG; ++i)
         {
         MosSprintf(ImgName, 256, MIL_TEXT("%s/ProductInfo_%d_%03d.mim"), IMAGE_DIR, (long)(Indice), (long)(AZIMUTH_ANGLE[i]));
         MbufRestore(ImgName, MilSysId, &ImgVect[i]);
         }

      // Allocate local shape image.
      ImgSizeX = MbufInquire(ImgVect[0], M_SIZE_X, M_NULL);
      ImgSizeY = MbufInquire(ImgVect[0], M_SIZE_Y, M_NULL);

      MbufAlloc2d(MilSysId, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, &LocalShapeId);
      MbufClear(LocalShapeId, 0.0);

      // Compute local shape.
      MregCalculate(PsContextId, &ImgVect[0], PsRstId, NB_IMG, M_DEFAULT);

      //  Check if Calculate completed correctly
      MIL_INT Status;
      MregGetResult(PsRstId, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);

      if(Status == M_COMPLETE)
         {
         // Draw computed local shape with auto remap factor.
         MregDraw(M_DEFAULT, PsRstId, LocalShapeId, M_DRAW_LOCAL_SHAPE_IMAGE, M_DEFAULT, M_DEFAULT);
         }
      else
         {
         // Free local shape.
         MbufFree(LocalShapeId);
         LocalShapeId = M_NULL;

         MregFree(PsRstId);
         return Status;
         }
      

      if(Indice == 0)
         {
         // Save range remap factor of the first local shape draw.
         MregGetResult(PsRstId, M_GENERAL, M_RANGE_FACTOR_LOCAL_SHAPE, &AutoFactor);
         }
      }

   // Allocate local shape image.
   MbufAlloc2d(MilSysId, ImgSizeX, ImgSizeY, 8 + M_UNSIGNED, M_IMAGE + M_PROC + M_DISP, pLocalShapeControlledFactorId);
   MbufClear(*pLocalShapeControlledFactorId, 0.0);

   // Control draw to use saved range remap factor.
   MregControl(PsRstId, M_GENERAL, M_DRAW_REMAP_FACTOR_MODE, M_USER_DEFINED);
   MregControl(PsRstId, M_GENERAL, M_DRAW_REMAP_FACTOR_VALUE, AutoFactor);

   //  Check if Calculate completed correctly
   MIL_INT Status;
   MregGetResult(PsRstId, M_DEFAULT, M_STATUS + M_TYPE_MIL_INT, &Status);

   if(Status == M_COMPLETE)
      {
      // Draw last local shape with controlled factor range.
      MregDraw(M_DEFAULT, PsRstId, *pLocalShapeControlledFactorId, M_DRAW_LOCAL_SHAPE_IMAGE, M_DEFAULT, M_DEFAULT);
      }
   else
      {
      // Free local shape.
      MbufFree(*pLocalShapeControlledFactorId);
      *pLocalShapeControlledFactorId = M_NULL;
      }

   MregFree(PsRstId);
   return Status;
   }


//////////////////////////////////////////////////////////////////////////////////////////
// Free image buffers.
void FreeImageBuffers( std::vector<MIL_ID>* pImgVect)
   {
   std::vector<MIL_ID>& ImgVect = *pImgVect;
   for(MIL_INT i = 0; i < NB_IMG; ++i)
      {
      MbufFree(ImgVect[i]);
      ImgVect[i] = M_NULL;
      }
   }

//////////////////////////////////////////////////////////////////////////////////////////
// Result visualization.
void ShowResult(MIL_ID MilSysId, 
                MIL_ID MilDisplayId, 
                MIL_CONST_TEXT_PTR pDesc, 
                std::vector<MIL_ID>* pImgVect, 
                MIL_ID* pAlbedoId, 
                MIL_ID* pCurvatureId,
                MIL_ID* pLocalShapeId)
   {
   std::vector<MIL_ID>& ImgVect      = *pImgVect;
   MIL_ID&              AlbedoId     = *pAlbedoId;
   MIL_ID&              CurvatureId  = *pCurvatureId;
   MIL_ID&              LocalShapeId = *pLocalShapeId;

   // Show image samples.
   MosPrintf(MIL_TEXT("\nDisplaying images acquired with different lighting directions.\n"));
   for (MIL_INT ImgIdx = 0; ImgIdx < NB_IMG; ImgIdx++)
      {
      MdispSelect(MilDisplayId, ImgVect[ImgIdx]);
      MosPrintf(MIL_TEXT("\rImage %d of %d. Press any key to continue..."), (long) ImgIdx+1, (long) NB_IMG);
      MosGetch();
      }
   MosPrintf(MIL_TEXT("\n\n"));
   MosPrintf(pDesc);

   // Show albedo result.
   if (M_NULL != AlbedoId)
      {
      MdispSelect(MilDisplayId, AlbedoId);
      MosPrintf(MIL_TEXT("Press any key to continue...\n"));
      MosGetch();

      // Defect extraction from albedo image.
      AlbedoDefectExtraction(MilSysId, MilDisplayId, ImgVect, AlbedoId);

      // Free albedo.
      MbufFree(AlbedoId);
      AlbedoId = M_NULL;
      }

   if (M_NULL != CurvatureId)
      {
      // Visualize surface curvature.
      MdispSelect(MilDisplayId, CurvatureId);
      MosPrintf(MIL_TEXT("Press any key to continue...\n"));
      MosGetch();

      // Show extracted defect from curvature.
      CurvatureDefectExtraction(MilSysId, MilDisplayId, ImgVect, CurvatureId);

      // Free curvature.
      MbufFree(CurvatureId);
      CurvatureId = M_NULL;
      }

   if (M_NULL != LocalShapeId)
      {
      // Visualize local shape.
      MdispSelect(MilDisplayId, LocalShapeId);
      MosPrintf(MIL_TEXT("Press any key to continue...\n"));
      MosGetch();

      // Free local shape.
      MbufFree(LocalShapeId);
      LocalShapeId = M_NULL;
      }  

   FreeImageBuffers(pImgVect);
   }
