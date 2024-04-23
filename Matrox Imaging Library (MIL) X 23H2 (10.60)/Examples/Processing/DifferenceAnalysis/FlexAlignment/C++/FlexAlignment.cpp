/*************************************************************************************/
/*
 * File name: FlexAlignment.cpp
 *
 * Synopsis:  This example shows how to perform flexible alignment using the Pattern Matching
              and the Calibration module.
 *            The first part of the example shows how to perform rigid alignment.
 *            The second part of the example shows how to perform flexible alignment.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include <mil.h>
#include <memory>
#include "CAUtil.h"
#include "FlexRegistration.h"

/* Path to the template and target image. */
MIL_CONST_TEXT_PTR TEMPLATE_PATH = M_IMAGE_PATH MIL_TEXT("FlexAlignment//template.mim");
MIL_CONST_TEXT_PTR TARGET_PATH   = M_IMAGE_PATH MIL_TEXT("FlexAlignment//target.mim");

/* Constants. */
const MIL_INT DELTA_SEARCH             = 20;                /* Size of the search area for Pattern Matching.                                                                 */
const MIL_INT GRID_SIZE_X              = 8;                 /* Number of cells in x axis.                                                                                    */
const MIL_INT GRID_SIZE_Y              = 8;                 /* Number of cells in y axis.                                                                                    */
const MIL_INT GRID_MARGIN              = 4;                 /* Margin of the grid. Setting a margin ensures that the content of the cells is present in the target image.    */
const MIL_DOUBLE MODEL_MIN_SEPARATION  = 10.0;              /* Minimum distance in pixels between two models to be considered distinct. Must be smaller than DELTA_SEARCH.   */
const MIL_DOUBLE DIFF_THRESH_SCORE     = 8.0;               /* If two pattern matching occurrences are found for a cell,  their score difference must be larger than this    */
                                                            /* value in order to use the best occurrence. Otherwise, both occurrences are rejected and the cell is not used  */
                                                            /* for the calibration.                                                                                          */
const MIL_DOUBLE ACCEPTANCE_SCORE      = 80.0;              /* Specifies the acceptance level. If the match score is less than this level, it is not considered a match.     */
const MIL_DOUBLE CERTAINTY_SCORE       = 98.0;              /* Specifies the certainty level.  If the match score is greater than or equal to this level, a match is assumed */
                                                            /* without looking elsewhere in the image for a better match.                                                    */

/* Namespace. */
using std::auto_ptr;

/* Function declaration. */
bool RigidAlignment(MIL_ID TemplateBufferId, MIL_ID TargetBufferId, MIL_ID DstBufferId);

/*****************************************************************************/
/* Example description.
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n"));
   MosPrintf(MIL_TEXT("FlexAlignment\n\n"));

   MosPrintf(MIL_TEXT("[SYNOPSIS]\n"));
   MosPrintf(MIL_TEXT("This example shows how to perform flexible alignment using\n"));
   MosPrintf(MIL_TEXT("the Pattern Matching and the Calibration modules.\n\n"));

   MosPrintf(MIL_TEXT("[MODULES USED]\n"));
   MosPrintf(MIL_TEXT("Modules used: Display, Graphics, Pattern Matching, Calibration\n")); 
   MosPrintf(MIL_TEXT("and Registration.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

int main(void)
   {
   /* Print example information in console. */
   PrintHeader();

   MIL_ID MilApplication = M_NULL,   /* Application identifier.               */
      MilSystem          = M_NULL,   /* System Identifier.                    */
      MilDisplay1        = M_NULL,   /* Display identifier 1.                 */
      MilDisplay2        = M_NULL,   /* Display identifier 2.                 */
      MilDisplay3        = M_NULL,   /* Display identifier 3.                 */
      MilGraList1        = M_NULL,   /* Graphic List identifier for Display 1.*/
      MilGraList2        = M_NULL,   /* Graphic List identifier for Display 2.*/
      TemplateBufferId   = M_NULL;   /* Template buffer identifier.           */

   /* Structure with minimal buffer information. */                                                             
   SBuffer TargetBufferId,           /* Target buffer identifier.                                                    */
      RigidAlignedBuffer,            /* Rigid aligned buffer result identifier.                                      */
      FlexAlignedBuffer,             /* Flexible aligned buffer result identifier.                                   */
      Diff,                          /* Difference between the original target and the original template.            */
      RigidDiff,                     /* Difference between the rigid alignment result and the original template.     */
      FlexDiff;                      /* Difference between the flexible alignment result and the original template.  */

   MIL_BOOL RigidAlignmentStatus;

   /* Allocate defaults. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   /* Allocate and prepare displays. */
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay1);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay2);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay3);

   MIL_STRING DisplayTitle1 = MIL_TEXT("Template Image");
   MIL_STRING DisplayTitle2 = MIL_TEXT("Target Image");
   MIL_STRING DisplayTitle3 = MIL_TEXT("Difference Result");
   MIL_STRING DisplayTitle4 = MIL_TEXT("Rigidly Aligned Target Image");
   MIL_STRING DisplayTitle5 = MIL_TEXT("Rigid Alignment Difference Result");
   MIL_STRING DisplayTitle6 = MIL_TEXT("Flexibly Aligned Target Image");
   MIL_STRING DisplayTitle7 = MIL_TEXT("Flexible Alignment Difference Result");

   MdispControl(MilDisplay1, M_TITLE, DisplayTitle1);
   MdispControl(MilDisplay2, M_TITLE, DisplayTitle2);
   MdispControl(MilDisplay3, M_TITLE, DisplayTitle3);

   MdispZoom(MilDisplay1, 0.5, 0.5);
   MdispControl(MilDisplay1, M_WINDOW_INITIAL_POSITION_X, 0L);

   MdispZoom(MilDisplay2, 0.5, 0.5);
   MdispControl(MilDisplay2, M_WINDOW_INITIAL_POSITION_X, 600L);

   MdispZoom(MilDisplay3, 0.5, 0.5);
   MdispControl(MilDisplay3, M_WINDOW_INITIAL_POSITION_X, 1200L);

   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList1);
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraList2);
   MdispControl(MilDisplay1, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList1);
   MdispControl(MilDisplay2, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraList2);

   /* Restore the template and target image. */
   TemplateBufferId = MbufRestore(TEMPLATE_PATH, MilSystem, M_NULL);
   RestoreAndConvert(MilSystem, TARGET_PATH, TargetBufferId, 8 + M_UNSIGNED);

   /* Compute the difference between the template image and the target image. */
   CloneBuffer(TemplateBufferId, Diff);
   MimArith(TargetBufferId.Id, TemplateBufferId, Diff.Id, M_SUB_ABS);

   MdispSelect(MilDisplay1, TemplateBufferId);
   MdispSelect(MilDisplay2, TargetBufferId.Id);
   MdispSelect(MilDisplay3, Diff.Id);

   /* Pause to show the template and target image. */
   MosPrintf(MIL_TEXT("The graphic patterns displayed are printed on a flexible material.\n"));
   MosPrintf(MIL_TEXT("The flexibility of the material causes local deformations in the design.\n"));
   MosPrintf(MIL_TEXT("A subtraction operation shows that the images are not aligned.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to perform rigid alignment.\n\n"));

   MosGetch();

   /* Clone to allocate memory for rigid alignment result buffer. */
   CloneBuffer(TargetBufferId, RigidAlignedBuffer);

   /* Perform rigid aligment. */
   RigidAlignmentStatus = RigidAlignment(TemplateBufferId, TargetBufferId.Id, RigidAlignedBuffer.Id);

   /* Compute the difference between the template image and the rigidly aligned target result. */
   CloneBuffer(TemplateBufferId, RigidDiff);
   MimArith(RigidAlignedBuffer.Id, TemplateBufferId, RigidDiff.Id, M_SUB_ABS);

   MdispControl(MilDisplay2, M_TITLE, DisplayTitle4);
   MdispSelect(MilDisplay2, RigidAlignedBuffer.Id);
   MdispControl(MilDisplay3, M_TITLE, DisplayTitle5);
   MdispSelect(MilDisplay3, RigidDiff.Id);

   /* Pause to show the rigid alignment results. */
   MosPrintf(MIL_TEXT("1- Rigid Alignment:\n"));
   MosPrintf(MIL_TEXT("-------------------\n"));
   MosPrintf(MIL_TEXT("The target image is aligned to the template image by performing\n"));
   MosPrintf(MIL_TEXT("rotation and translation operations on the target image.\n"));
   MosPrintf(MIL_TEXT("This method does not provide a good alignment due to the local deformations.\n\n"));
   MosPrintf(MIL_TEXT("Press <Enter> to perform flexible alignment.\n\n"));

   MosGetch();

   if (RigidAlignmentStatus)
      {
      /* Allocate and set the flex registration object. We define a GRID_SIZE_X x GRID_SIZE_Y grid, a search region 
         of size DELTA_SEARCH x DELTA_SEARCH pixels. */
      CFlexRegistration FlexRegistration(GRID_SIZE_X, GRID_SIZE_Y, GRID_MARGIN);
      FlexRegistration.SetDeltaSearch(DELTA_SEARCH);                                      /* DEFAULT = 5. */
      FlexRegistration.SetModelMinSeparation(MODEL_MIN_SEPARATION);                       /* DEFAULT = 20. */
      FlexRegistration.SetAcceptanceAndCertaintyScore(ACCEPTANCE_SCORE, CERTAINTY_SCORE); /* DEFAULT = 75 and 90 respectively. */
      FlexRegistration.SetScoreDifferenceThresh(DIFF_THRESH_SCORE);                       /* DEFAULT = 8. */
      FlexRegistration.SetTemplateBufferId(TemplateBufferId);

      /* Match every cell position in the rigidly aligned target image to a cell in the template image and 
         using these position to define a calibration context. */
      MbufCopy(RigidAlignedBuffer.Id, TargetBufferId.Id);
      FlexRegistration.Calculate(TargetBufferId.Id);
     
      /* Clone to allocate memory for flexible alignment result buffer. */
      CloneBuffer(TargetBufferId, FlexAlignedBuffer);

      /* Perform flexible aligment transforming the target image using the previously defined
         calibration context. */
      FlexRegistration.Transform(TargetBufferId.Id, FlexAlignedBuffer.Id);

      /* Compute the difference between the target image and the flexibly aligned result. */
      CloneBuffer(TemplateBufferId, FlexDiff);
      MimArith(FlexAlignedBuffer.Id, TemplateBufferId, FlexDiff.Id, M_SUB_ABS);

      /* Draw the grid on template image. */
      MdispControl(MilDisplay1, M_UPDATE, M_DISABLE);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      FlexRegistration.Draw(MilGraList1, DRAW_MODEL);
      MdispControl(MilDisplay1, M_UPDATE, M_ENABLE);

      /* Draw the result. */
      MdispControl(MilDisplay2, M_UPDATE, M_DISABLE);
      MgraColor(M_DEFAULT, M_COLOR_BLUE);
      FlexRegistration.Draw(MilGraList2, DRAW_SEARCH_REGION);
      MgraColor(M_DEFAULT, M_COLOR_GREEN);
      FlexRegistration.Draw(MilGraList2, DRAW_RESULT_BOX);
      MdispControl(MilDisplay2, M_UPDATE, M_ENABLE);
      MdispSelect(MilDisplay3, M_NULL);

      /* Pause to show the template, rigidly aligned target image, and the grids. */
      MosPrintf(MIL_TEXT("2- Flexible Alignment:\n"));
      MosPrintf(MIL_TEXT("----------------------\n"));
      MosPrintf(MIL_TEXT("The template image is first partitioned into child images.\n"));
      MosPrintf(MIL_TEXT("Each child is then matched in the rigidly aligned target image\n"));
      MosPrintf(MIL_TEXT("using the Pattern Matching module.\n"));
      MosPrintf(MIL_TEXT("This establishes a list of corresponding position between the two images.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to see the flexible alignment result.\n\n"));

      MosGetch();

      /* Change title, remove graphics, and select the result difference of the flexible alignment. */
      MgraControlList(MilGraList1, M_ALL, M_DEFAULT, M_DELETE, M_DEFAULT);
      MgraControlList(MilGraList2, M_ALL, M_DEFAULT, M_DELETE, M_DEFAULT);
      MdispControl(MilDisplay2, M_TITLE, DisplayTitle6);
      MdispSelect(MilDisplay2, FlexAlignedBuffer.Id);
      MdispControl(MilDisplay3, M_TITLE, DisplayTitle7);
      MdispSelect(MilDisplay3, FlexDiff.Id);

      /* Pause to show the flexible alignment results. */
      MosPrintf(MIL_TEXT("These positions are then used to locally align the target image\n"));
      MosPrintf(MIL_TEXT("with the template image using a calibration context.\n"));
      MosPrintf(MIL_TEXT("The alignment between the target and template image is improved.\n\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n\n"));

      MosGetch();
      }
      /* Rigid alignement fail. */
   else
      {
      MosPrintf(MIL_TEXT("Rigid alignment failed.\n"));
      }

   /* Free all allocations. */
   MgraFree(MilGraList1);
   MgraFree(MilGraList2);
   MdispFree(MilDisplay2);
   MdispFree(MilDisplay3);
   MbufFree(Diff.Id);  
   MbufFree(RigidDiff.Id);  
   MbufFree(TargetBufferId.Id);
   MbufFree(FlexAlignedBuffer.Id);
   MbufFree(RigidAlignedBuffer.Id);
   MbufFree(TemplateBufferId);
   MbufFree(FlexDiff.Id);

   /* Free defaults. */
   MappFreeDefault(MilApplication, MilSystem, MilDisplay1, M_NULL, M_NULL);

   return 0;
   }

/* Function to perform rigid aligment. */
bool RigidAlignment(MIL_ID TemplateBufferId, MIL_ID TargetBufferId, MIL_ID DstBufferId)
   {
   bool AlignSucceed = false;

   /* Allocate registration context and result context. */
   MIL_ID RegContextId = MregAlloc(M_DEFAULT_HOST, M_CORRELATION, M_DEFAULT, M_NULL);
   MIL_ID RegResultId = MregAllocResult(M_DEFAULT_HOST, M_DEFAULT, M_NULL);

   /* Allow rotation and translation. */
   MregControl(RegContextId, M_CONTEXT, M_TRANSFORMATION_TYPE, M_TRANSLATION_ROTATION);

   /* Temporary array of input ID. */
   MIL_ID ImageArray[2] = { TemplateBufferId, TargetBufferId };

   /* Perform the registration. */
   MregCalculate(RegContextId, ImageArray, RegResultId, 2, M_DEFAULT);
   MIL_INT Result;
   MregGetResult(RegResultId, M_GENERAL, M_RESULT + M_TYPE_MIL_INT, &Result);
   
   if (M_SUCCESS == Result)
      {
      AlignSucceed = true;

      /* Retrieve the transformation matrix. */
      MIL_ID TransMatrixId;
      MregGetResult(RegResultId, 1, M_TRANSFORMATION_MATRIX_ID + M_TYPE_MIL_ID, &TransMatrixId);
      MimWarp(TargetBufferId, DstBufferId, TransMatrixId, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR + M_OVERSCAN_CLEAR);
      }
   MregFree(RegResultId);
   MregFree(RegContextId);

   return AlignSucceed;
   }
