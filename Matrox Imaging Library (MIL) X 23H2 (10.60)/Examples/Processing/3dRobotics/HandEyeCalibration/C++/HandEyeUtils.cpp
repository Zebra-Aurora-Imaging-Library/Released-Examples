//***************************************************************************************/
// 
// File name: HandEyeUtils.cpp  
//
// Synopsis:  This file defines some utilities functions.
//
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//**************************************************************************************/

#include "HandEyeUtils.h"

//*****************************************************************************
// Binds index from 0 to 4 to a color
//*****************************************************************************
void IndexToColor(const size_t &Index, MIL_INT32 &ColorValue, MIL_STRING &ColorString)
   {
   switch(Index)
      {
      case 0:
      {
      ColorValue = M_COLOR_GREEN;
      ColorString = MIL_TEXT("Green");
      break;
      }
      case 1:
      {
      ColorValue = M_COLOR_BLACK;
      ColorString = MIL_TEXT("Black");
      break;
      }
      case 2:
      {
      ColorValue = M_COLOR_BLUE;
      ColorString = MIL_TEXT("Blue");
      break;
      }
      case 3:
      {
      ColorValue = M_COLOR_MAGENTA;
      ColorString = MIL_TEXT("Magenta");
      break;
      }
      default:
         MosPrintf(MIL_TEXT("Error - Unhandled case!"));
      }
   }

//*****************************************************************************
// Calculates an error measurement between 2 matrices
//*****************************************************************************
MIL_DOUBLE CalculateMatrixDiscrepancy(MIL_ID MilSystem, MIL_ID MilMatrix, const MIL_DOUBLE* ExpectedMatrixValues)
   {
   auto MilDiscrepancyMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   auto MilExpectedMatrix = M3dgeoAlloc(MilSystem, M_TRANSFORMATION_MATRIX, M_DEFAULT, M_UNIQUE_ID);
   M3dgeoMatrixPut(MilExpectedMatrix, M_DEFAULT, ExpectedMatrixValues);

   // Discrepancy matrix is the composition of the expected matrix inverse with the matrix we want to evaluate
   M3dgeoMatrixSetTransform(MilDiscrepancyMatrix.get(), M_INVERSE, MilExpectedMatrix.get(), M_DEFAULT, M_DEFAULT, M_DEFAULT, M_ASSIGN);
   M3dgeoMatrixSetTransform(MilDiscrepancyMatrix.get(), M_COMPOSE_TWO_MATRICES, MilMatrix, MilDiscrepancyMatrix, M_DEFAULT, M_DEFAULT, M_ASSIGN);
   std::vector<MIL_DOUBLE> DiscrepMatValues;
   M3dgeoMatrixGet(MilDiscrepancyMatrix, M_DEFAULT, DiscrepMatValues);

   // The discrepancy itself is the magnitude of the translational part of the discrepancy matrix
   MIL_DOUBLE DiscrepancyValue = sqrt(DiscrepMatValues[3] * DiscrepMatValues[3] + DiscrepMatValues[7] * DiscrepMatValues[7] + DiscrepMatValues[11] * DiscrepMatValues[11]);
   return DiscrepancyValue;
   }

//*****************************************************************************
// Print a Matrix in the console
//*****************************************************************************
void DisplayMatrix(MIL_ID MilMatrix)
   {
   std::vector<MIL_DOUBLE> MatrixVec;
   M3dgeoMatrixGet(MilMatrix, M_DEFAULT, MatrixVec);

   for(int k = 0; k < 16; k++)
      {
      std::wstring SignSpacer = (MatrixVec[k] >= 0) ? L" " : L"";
      MosPrintf(MIL_TEXT("%s%4.4f   "), SignSpacer.c_str(), MatrixVec[k]);
      if((k + 1) % 4 == 0)
         {
         MosPrintf(MIL_TEXT("\n"));
         }
      }
   }

//*****************************************************************************
// Adds the component M_COMPONENT_NORMALS_MIL if it's not found.
//*****************************************************************************
void AddComponentNormalsIfMissing(MIL_ID MilContainer)
   {
   MIL_ID MilNormals = MbufInquireContainer(MilContainer, M_COMPONENT_NORMALS_MIL, M_COMPONENT_ID, M_NULL);
   if(MilNormals != M_NULL)
      return;
   MIL_INT SizeX = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_X, M_NULL);
   MIL_INT SizeY = MbufInquireContainer(MilContainer, M_COMPONENT_RANGE, M_SIZE_Y, M_NULL);
   if(SizeX < 50 || SizeY < 50)
      M3dimNormals(M_NORMALS_CONTEXT_TREE, MilContainer, MilContainer, M_DEFAULT);
   else
      M3dimNormals(M_NORMALS_CONTEXT_ORGANIZED, MilContainer, MilContainer, M_DEFAULT);
   }

//*****************************************************************************
// Allocates a 3D display and returns its MIL identifier.
//*****************************************************************************
MIL_UNIQUE_3DDISP_ID Alloc3dDisplayId(MIL_ID MilSystem)
   {
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);
   auto MilDisplay3D = M3ddispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_UNIQUE_ID);
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // Display error if 3d display not available
   if(!MilDisplay3D)
      {
      MosPrintf(MIL_TEXT("\n")
                MIL_TEXT("The current system does not support the 3D display.\n")
                MIL_TEXT("Press any key to continue.\n"));
      MosGetch();
      }
   return MilDisplay3D;
   }
