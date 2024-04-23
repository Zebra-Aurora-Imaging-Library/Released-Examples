/*************************************************************************************/
/*
 * File name: FlexRegistrationGrid.cpp
 *
 * Synopsis:  Implementation of the FlexRegistrationGrid class
 *
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include "FlexRegistrationGrid.h"
#include "Rect.h"
#include "MathUtil.h"

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 * 
 * Name              : ~CFlexRegistrationGrid()
 * 
 * Access            : Public
 *
 * Synopsis          : Destructor
 *
 */
CFlexRegistrationGrid::~CFlexRegistrationGrid()
   {
   ClearGrid();
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : UpdateGrid()
 *
 * Access            : Public
 *
 * Synopsis          : Update the grid
 *
 * Parameters        : TemplateBufferId
 *
 * Comments          : Divide and template buffer in a SizeX*SizeY grid
 *                   : and create a pattern matching context (FinderItem) for each grid element
 *
 */
void CFlexRegistrationGrid::UpdateGrid(MIL_ID TemplateBufferId)
   {
   /* Inquire the grid size. */
   MIL_INT GridSizeX = GetSizeX();
   MIL_INT GridSizeY = GetSizeY();
   MIL_INT Margin = GetMargin();
   /* Inquire buffer size. */
   MIL_INT TemplateBufferSizeX = MbufInquire(TemplateBufferId, M_SIZE_X, M_NULL);
   MIL_INT TemplateBufferSizeY = MbufInquire(TemplateBufferId, M_SIZE_Y, M_NULL);

   m_CellSizeX = (TemplateBufferSizeX - (2 * Margin)) / GridSizeX;     /* omit extra pixels. */
   m_CellSizeY = (TemplateBufferSizeY - (2 * Margin)) / GridSizeY;     

   MIL_INT OffX, OffY;
   MIL_ID PatternId, ResultId;
   for(MIL_INT Row = 0; Row < GridSizeY; Row++)
      {
      for(MIL_INT Col = 0; Col < GridSizeX; Col++)
         {
         /* Get a reference to the item. */
         CFinderItem &UpdatedItem = m_FlexRegistrationGrid.GetElement(Row, Col);

         /* Compute offsets. We add a margin to define models far from the border to ensure their content is also present in the target image. */
         OffX = (Col * m_CellSizeX)+ Margin;
         OffY = (Row * m_CellSizeY)+ Margin;

         UpdatedItem.SetRect(OffX, OffY, m_CellSizeX, m_CellSizeY);
         UpdatedItem.SetCenter();

         /* Allocate the pattern matching context of the FinderItem . */
         PatternId = MpatAlloc(M_DEFAULT_HOST, M_DEFAULT, M_DEFAULT, M_NULL);
         MpatDefine(PatternId, M_REGULAR_MODEL, TemplateBufferId, OffX, OffY, m_CellSizeX, m_CellSizeY, M_DEFAULT);
         UpdatedItem.SetPatternMatchingId(PatternId);

         /* Allocate the result buffer of the FinderItem. */
         ResultId = MpatAllocResult(M_DEFAULT_HOST, M_DEFAULT, M_NULL);
         UpdatedItem.SetResultId(ResultId);

         /* Update the search region of the FinderIdem. */
         MpatControl(PatternId, 0, M_SEARCH_OFFSET_X, M_Round(UpdatedItem.GetCenter().x - m_DeltaSearch / 2));
         MpatControl(PatternId, 0, M_SEARCH_OFFSET_Y, M_Round(UpdatedItem.GetCenter().y - m_DeltaSearch / 2));
         MpatControl(PatternId, 0, M_SEARCH_SIZE_X, m_DeltaSearch);
         MpatControl(PatternId, 0, M_SEARCH_SIZE_Y, m_DeltaSearch);

         /* Other controls. */
         MpatControl(PatternId, M_ALL, M_ACCURACY, M_MEDIUM);
         MpatControl(PatternId, M_ALL, M_SPEED, M_MEDIUM);
         MpatControl(PatternId, M_ALL, M_ACCEPTANCE, m_AcceptanceScore);
         MpatControl(PatternId, M_ALL, M_CERTAINTY, m_CertaintyScore);
         MpatControl(PatternId, M_ALL, M_FIRST_LEVEL, M_AUTO_CONTENT_BASED);
         /* We set the maximum number of target occurences to 2. If the context finds 2 occurences with close scores values the model will */
         /* not be included in the calibration process (see Calculate() for more details). */
         MpatControl(PatternId, M_ALL, M_NUMBER, 2);
         /* We set the minimum separation in percentage between two occurences to be considered distinct. */
         MpatControl(PatternId, M_ALL, M_MIN_SEPARATION_X, m_ModelMinSeparation / MIL_DOUBLE(m_CellSizeX) * 100.0);
         MpatControl(PatternId, M_ALL, M_MIN_SEPARATION_Y, m_ModelMinSeparation / MIL_DOUBLE(m_CellSizeY) * 100.0);
         }
      }
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : SetSizeX()
 *
 * Access            : Public
 *
 * Synopsis          : Set the X size of the grid
 *
 * Parameters        : NewSizeX
 *
 */
void CFlexRegistrationGrid::SetSizeX( MIL_INT NewSizeX )
   {
   /* Update the cell size. */
   MIL_INT OldGridSizeX = GetSizeX(); /* Old size. */
   m_CellSizeX = m_CellSizeX * OldGridSizeX / NewSizeX;

   m_FlexRegistrationGrid.SetSizeX(NewSizeX); 
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : SetSizeY()
 *
 * Access            : Public
 *
 * Synopsis          : Set the Y size of the grid
 *
 * Parameters        : NewSizeY
 *
 */
void CFlexRegistrationGrid::SetSizeY( MIL_INT NewSizeY )
   {
   /* Update the cell size. */
   MIL_INT OldGridSizeY = GetSizeY(); /* Old size. */
   m_CellSizeY = m_CellSizeY * OldGridSizeY / NewSizeY;

   m_FlexRegistrationGrid.SetSizeY(NewSizeY); 
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : SetMargin()
 *
 * Access            : Public
 *
 * Synopsis          : Set the margin of the grid
 *
 * Parameters        : Margin
 *
 */
void CFlexRegistrationGrid::SetMargin(MIL_INT Margin)
   {
   m_FlexRegistrationGrid.SetMargin(Margin);
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : SetSize()
 *
 * Access            : Public
 *
 * Synopsis          : Set the X , Y size of the grid
 *
 * Parameters        : NewSizeX, NewSizeY
 *
 */
void CFlexRegistrationGrid::SetSize( MIL_INT NewSizeX, MIL_INT NewSizeY )
   {
   MIL_INT OldGridSizeX = GetSizeX(); /* Old size. */
   MIL_INT OldGridSizeY = GetSizeY(); 

   m_CellSizeX = m_CellSizeX * OldGridSizeX / NewSizeX;
   m_CellSizeY = m_CellSizeY * OldGridSizeY / NewSizeY;

   m_FlexRegistrationGrid.SetSizeX(NewSizeX); 
   m_FlexRegistrationGrid.SetSizeY(NewSizeY); 
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : Draw()
 *
 * Access            : Public
 *
 * Synopsis          : Draw flex grid information in the DstGraList
 *
 * Parameters        : DstGraList, DrawOperation
 *
 */
void CFlexRegistrationGrid::Draw(MIL_ID DstGraList, EFlexDrawOperation FlexDrawOperation)
   {
   /* Retrieve the size of the grid. */
   MIL_INT SizeX = GetSizeX();
   MIL_INT SizeY = GetSizeY();

   MIL_INT CurrentPatternMatchingId = M_NULL;
   MIL_INT CurrentResultId = M_NULL;

   switch(FlexDrawOperation)
      {
      case DRAW_MODEL:
         {
         for(MIL_INT Row = 0; Row < SizeY; Row++) 
            for(MIL_INT Col = 0; Col < SizeX; Col++)
               {
               CFinderItem &CurrentFinderItem = GetElement(Row, Col);

               MIL_INT OffX = CurrentFinderItem.GetRect().OffX;
               MIL_INT OffY = CurrentFinderItem.GetRect().OffY;
               MIL_INT SizeX = CurrentFinderItem.GetRect().SizeX;
               MIL_INT SizeY = CurrentFinderItem.GetRect().SizeY;
               MIL_DOUBLE CenterX = CurrentFinderItem.GetCenter().x;
               MIL_DOUBLE CenterY = CurrentFinderItem.GetCenter().y;

               /* Draw the rect. */
               MgraRect(M_DEFAULT, 
                  DstGraList,
                  OffX,
                  OffY,
                  OffX+SizeX-1,
                  OffY+SizeY-1);

               /* Draw the center. */
               MgraDot(M_DEFAULT,
                  DstGraList,
                  M_Round(CenterX),
                  M_Round(CenterY));
               }     
            break;
         }

      case DRAW_SEARCH_REGION:
         {
         for(MIL_INT Row = 0; Row < SizeY; Row++) 
            for(MIL_INT Col = 0; Col < SizeX; Col++)
               {
               CFinderItem &CurrentFinderItem = GetElement(Row, Col);

               MIL_DOUBLE CenterX = CurrentFinderItem.GetCenter().x;
               MIL_DOUBLE CenterY = CurrentFinderItem.GetCenter().y;

               /* Draw the rect. */
               MgraRect(M_DEFAULT, 
                  DstGraList,
                  M_Round(CenterX - m_DeltaSearch / 2),
                  M_Round(CenterY - m_DeltaSearch / 2),
                  M_Round(CenterX + m_DeltaSearch / 2),
                  M_Round(CenterY + m_DeltaSearch / 2));
                  
               /* Draw the center. */
               MgraDot(M_DEFAULT,
                  DstGraList,
                  M_Round(CenterX),
                  M_Round(CenterY));
               }     
            break;
         }

      case DRAW_RESULT_BOX:
         {
         for(MIL_INT Row = 0; Row < SizeY; Row++) 
            for(MIL_INT Col = 0; Col < SizeX; Col++)
               {
               CFinderItem &CurrentFinderItem = GetElement(Row, Col);
               CurrentPatternMatchingId = CurrentFinderItem.GetPatternMatchingId();
               CurrentResultId = CurrentFinderItem.GetResultId();

               MIL_DOUBLE Num;
               MIL_DOUBLE Scores[2] = {};
               MpatGetResult(CurrentResultId, M_DEFAULT, M_NUMBER, &Num);
               MpatGetResult(CurrentResultId, M_DEFAULT, M_SCORE, Scores);
               MIL_DOUBLE Diff = Scores[0] - Scores[1];

               if (Num == 1 || (Num == 2 && Diff > m_ScoreDifferenceThresh))
                  {
                  MpatDraw(M_DEFAULT, CurrentResultId, DstGraList, M_DRAW_BOX, 0, M_DEFAULT);
                  MpatDraw(M_DEFAULT, CurrentResultId, DstGraList, M_DRAW_POSITION, 0, M_DEFAULT);
                  }
               else
                  {
                  /* Retrieve the position and size of the model box. */
                  MIL_INT OffX = CurrentFinderItem.GetRect().OffX;
                  MIL_INT OffY = CurrentFinderItem.GetRect().OffY;
                  MIL_INT SizeX = CurrentFinderItem.GetRect().SizeX;
                  MIL_INT SizeY = CurrentFinderItem.GetRect().SizeY;
                  MIL_DOUBLE CenterX = CurrentFinderItem.GetCenter().x;
                  MIL_DOUBLE CenterY = CurrentFinderItem.GetCenter().y;


                  /* Force the draw of not found model to red. */
                  MIL_INT CurrentColor = MgraInquire(M_DEFAULT, M_COLOR, M_NULL);

                  if(Num == 0)
                     MgraColor(M_DEFAULT, M_COLOR_RED);
                  else
                     MgraColor(M_DEFAULT, M_COLOR_MAGENTA);

                  /* Draw the rect. */
                  MgraRect(M_DEFAULT, 
                     DstGraList,
                     OffX,
                     OffY,
                     OffX+SizeX-1,
                     OffY+SizeY-1);

                  /* Draw the center. */
                  MgraDot(M_DEFAULT,
                     DstGraList,
                     M_Round(CenterX),
                     M_Round(CenterY));
                  
                  /* Reset to previous color. */
                  MgraColor(M_DEFAULT, MIL_DOUBLE(CurrentColor));
                  }
               }     
            break;
         }
      case DRAW_INDEX:
         {
         for(MIL_INT Row = 0; Row < SizeY; Row++) 
            for(MIL_INT Col = 0; Col < SizeX; Col++)
               {
               CFinderItem &CurrentFinderItem = GetElement(Row, Col);
               CurrentResultId = CurrentFinderItem.GetResultId();

               MIL_TEXT_CHAR TmpBuffer[50];
               MosSprintf(TmpBuffer, 50, MIL_TEXT("%d"), Row*GetSizeX()+Col);
               MIL_DOUBLE Num;
               MpatGetResult(CurrentResultId, M_DEFAULT, M_NUMBER, &Num);
               if ( Num > 0)
                  {
                  /* Get the position of the result. */
                  MIL_DOUBLE PosX = 0.0, PosY = 0.0;

                  MpatGetResult(CurrentResultId, 0, M_POSITION_X, &PosX);
                  MpatGetResult(CurrentResultId, 0,  M_POSITION_Y, &PosY);

                  MgraText(M_DEFAULT, 
                     DstGraList, 
                     M_Round(PosX),
                     M_Round(PosY),
                     TmpBuffer);
                  }
               else
                  {
                  /* Retrieve the center of the model box. */
                  MIL_DOUBLE CenterX = CurrentFinderItem.GetCenter().x;
                  MIL_DOUBLE CenterY = CurrentFinderItem.GetCenter().y;
                  
                  MgraText(M_DEFAULT, 
                     DstGraList, 
                     M_Round(CenterX),
                     M_Round(CenterY),
                     TmpBuffer);
                  }
               }

         }
         break;

      case DRAW_SCORE:
         {
         for(MIL_INT Row = 0; Row < SizeY; Row++) 
            for(MIL_INT Col = 0; Col < SizeX; Col++)
               {
               CFinderItem &CurrentFinderItem = GetElement(Row, Col);
               CurrentResultId = CurrentFinderItem.GetResultId();

               MIL_TEXT_CHAR TmpBuffer[50];

               MIL_DOUBLE Num;
               MpatGetResult(CurrentResultId, M_DEFAULT, M_NUMBER, &Num);
               if (Num > 0)
                  {
                  for(int i = 0; i < Num; i++)
                     {
                     /* Get the position of the result. */
                     MIL_DOUBLE Score = 0.0, PosX = 0.0, PosY = 0.0;

                     MpatGetResult(CurrentResultId, i, M_POSITION_X, &PosX);
                     MpatGetResult(CurrentResultId, i, M_POSITION_Y, &PosY);
                     MpatGetResult(CurrentResultId, i, M_SCORE, &Score);

                     MosSprintf(TmpBuffer, 50, MIL_TEXT("%.2f"), Score);

                     MgraText(M_DEFAULT,
                              DstGraList,
                              M_Round(PosX),
                              M_Round(PosY),
                              TmpBuffer);
                     }
                  }
               else
                  {
                  /* Retrieve the center of the model box. */
                  MIL_DOUBLE CenterX = CurrentFinderItem.GetCenter().x;
                  MIL_DOUBLE CenterY = CurrentFinderItem.GetCenter().y;

                  MosSprintf(TmpBuffer, 50, MIL_TEXT("NF")); /* Not found. */
                  
                  MgraText(M_DEFAULT, 
                     DstGraList, 
                     M_Round(CenterX),
                     M_Round(CenterY),
                     TmpBuffer);
                  }
               }

         }
         break;

      default:
         break;
      }
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : ClearGrid()
 *
 * Access            : Public
 *
 * Synopsis          : Clear the flex grid
 *
 * Comments          : Remove all the finder item from the grid
 *
 */
void CFlexRegistrationGrid::ClearGrid()
   {
   /* Inquire the grid size. */
   MIL_INT GridSizeX = GetSizeX();
   MIL_INT GridSizeY = GetSizeY();

   MIL_ID CurPatternMatchingId = M_NULL;
   MIL_ID CurResultId = M_NULL;

   for(MIL_INT Row = 0; Row < GridSizeY; Row++)
      {
      for(MIL_INT Col = 0; Col < GridSizeX; Col++)
         {
         /* Reinitialized the finder item. */
         CFinderItem &CurClearedItem = m_FlexRegistrationGrid.GetElement(Row, Col);

         CurPatternMatchingId = CurClearedItem.GetPatternMatchingId();
         CurResultId = CurClearedItem.GetResultId();

         if(M_NULL != CurPatternMatchingId)
            MpatFree(CurPatternMatchingId);

         if(M_NULL != CurResultId)
            MpatFree(CurResultId);

         /* Reinitialized. */
         CurClearedItem.Init();
         }
      }

   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : UpdateSearchRegion()
 *
 * Access            : Public
 *
 * Synopsis          : Update the search region of each finder item
 *                   : in the grid
 */
void CFlexRegistrationGrid::UpdateSearchRegion()
   {
   /* Inquire the grid size. */
   MIL_INT GridSizeX = GetSizeX();
   MIL_INT GridSizeY = GetSizeY();

   /* Retrieve the PatternMatchingId of the first element to test if a context is already associated. */
   if (M_NULL != m_FlexRegistrationGrid.GetElement(0, 0).GetPatternMatchingId())
      {
      for(MIL_INT Row = 0; Row < GridSizeY; Row++) 
         for(MIL_INT Col = 0; Col < GridSizeX; Col++)
            {
            /* Get a reference to the item. */
            CFinderItem &UpdatedItem = m_FlexRegistrationGrid.GetElement(Row, Col);

            /* Update the search region of the FinderIdem. */
            MpatControl(UpdatedItem.GetPatternMatchingId(), 0, M_SEARCH_OFFSET_X, M_Round(UpdatedItem.GetCenter().x - m_DeltaSearch / 2));
            MpatControl(UpdatedItem.GetPatternMatchingId(), 0, M_SEARCH_OFFSET_Y, M_Round(UpdatedItem.GetCenter().y - m_DeltaSearch / 2));
            MpatControl(UpdatedItem.GetPatternMatchingId(), 0, M_SEARCH_SIZE_X, m_DeltaSearch);
            MpatControl(UpdatedItem.GetPatternMatchingId(), 0, M_SEARCH_SIZE_Y, m_DeltaSearch);
            }
      }
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : UpdateModelMinSeparation()
 *
 * Access            : Public
 *
 * Synopsis          : Update the minimum sepration parameter of each finder item
 *                   : in the grid
 *
 */
void CFlexRegistrationGrid::UpdateModelMinSeparation()
   {
   /* Inquire the grid size. */
   MIL_INT GridSizeX = GetSizeX();
   MIL_INT GridSizeY = GetSizeY();

   /* Retrieve the PatternMatchingId of the first element to test if a context is already associated. */
   if(M_NULL != m_FlexRegistrationGrid.GetElement(0, 0).GetPatternMatchingId())
      {
      for(MIL_INT Row = 0; Row < GridSizeY; Row++)
         for(MIL_INT Col = 0; Col < GridSizeX; Col++)
            {
            /* Get a reference to the item. */
            CFinderItem &UpdatedItem = m_FlexRegistrationGrid.GetElement(Row, Col);

            /* Update the model minimum separation of the FinderIdem. */
            MpatControl(UpdatedItem.GetPatternMatchingId(), M_ALL, M_MIN_SEPARATION_X, m_ModelMinSeparation / MIL_DOUBLE(m_CellSizeX) * 100.0);
            MpatControl(UpdatedItem.GetPatternMatchingId(), M_ALL, M_MIN_SEPARATION_Y, m_ModelMinSeparation / MIL_DOUBLE(m_CellSizeY) * 100.0);
            }
      }
   }

/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : UpdateAcceptanceAndCertaintyScore()
 *
 * Access            : Public
 *
 * Synopsis          : Update the acceptance and certainty score of each finder item
 *                   : in the grid
 *
 */
void CFlexRegistrationGrid::UpdateAcceptanceAndCertaintyScore()
   {
   /* Inquire the grid size. */
   MIL_INT GridSizeX = GetSizeX();
   MIL_INT GridSizeY = GetSizeY();

   /* Retrieve the PatternMatchingId of the first element to test if a context is already associated. */
   if(M_NULL != m_FlexRegistrationGrid.GetElement(0, 0).GetPatternMatchingId())
      {
      for(MIL_INT Row = 0; Row < GridSizeY; Row++)
         for(MIL_INT Col = 0; Col < GridSizeX; Col++)
            {
            /* Get a reference to the item. */
            CFinderItem &UpdatedItem = m_FlexRegistrationGrid.GetElement(Row, Col);

            /* Update the model minimum separation of the FinderIdem. */
            MpatControl(UpdatedItem.GetPatternMatchingId(), M_ALL, M_ACCEPTANCE, m_AcceptanceScore);
            MpatControl(UpdatedItem.GetPatternMatchingId(), M_ALL, M_CERTAINTY, m_CertaintyScore);
            }
      }
   }
/*************************************************************************************/
/*
 * Class             : CFlexRegistrationGrid
 *
 * Name              : Calculate()
 *
 * Access            : Public
 *
 * Synopsis          : Associate every finder item with a target position in the target buffer 
 *                   : and prepare a calibration context.
 *
 * Parameters        : TargetBufferId, DstCalId
 *
 */
void CFlexRegistrationGrid::Calculate(MIL_ID TargetBufferId, MIL_ID DstCalId)
   {
   /* Count found point in the target. */
   MIL_INT FoundCounter = 0;

   /* Inquire the grid size. */
   MIL_INT GridSizeX = m_FlexRegistrationGrid.GetSizeX();
   MIL_INT GridSizeY = m_FlexRegistrationGrid.GetSizeY();

   /* Allocate array for the McalList call. */
   MIL_INT ArraySize = GridSizeX * GridSizeY;

   /* Those arrays have the maximum possible size. */
   MIL_DOUBLE *XTemplateArray = new MIL_DOUBLE[ArraySize];
   MIL_DOUBLE *YTemplateArray = new MIL_DOUBLE[ArraySize];
   MIL_DOUBLE *XTargetArray = new MIL_DOUBLE[ArraySize];
   MIL_DOUBLE *YTargetArray = new MIL_DOUBLE[ArraySize];

   /* For each element in the grid, search for occurrence of the model in the search region. */
   for(MIL_INT Row = 0; Row < GridSizeY; Row++) 
      for(MIL_INT Col = 0; Col < GridSizeX; Col++)
         {
         CFinderItem &CurFinderItem = m_FlexRegistrationGrid.GetElement(Row, Col);

         MIL_ID ResultId = CurFinderItem.GetResultId();
         MIL_INT PatternMatchingId = CurFinderItem.GetPatternMatchingId();

         /* Fill target array. */
         MpatPreprocess(PatternMatchingId, M_DEFAULT, M_NULL);
         MpatFind(PatternMatchingId, TargetBufferId, ResultId);

         MIL_DOUBLE Num;
         MIL_DOUBLE Scores[2] = {};
         MpatGetResult(ResultId, M_DEFAULT, M_NUMBER, &Num);
         MpatGetResult(ResultId, M_DEFAULT, M_SCORE, Scores);
         MIL_DOUBLE Diff = Scores[0] - Scores[1];

         /* If one occurrence is found, retrieve the position. Or, if two occurrences are found and the score difference */
         /* is higher than the threshold, retrieve the position of the first occurence (higher score).                   */
         if (Num == 1 || (Num == 2 && Diff > m_ScoreDifferenceThresh))
            {
            /* Fill template array. */
            XTemplateArray[FoundCounter]  = CurFinderItem.GetCenter().x;
            YTemplateArray[FoundCounter]  = CurFinderItem.GetCenter().y;

            /* Fill target array. */
            MpatGetResult(ResultId, 0, M_POSITION_X, &(XTargetArray[FoundCounter]));
            MpatGetResult(ResultId, 0,  M_POSITION_Y, &(YTargetArray[FoundCounter]));

            FoundCounter++;
            }
         }

      /* Perform the linear interpolation calibration using the positions found. */
      McalList(DstCalId, 
         XTargetArray, 
         YTargetArray,
         XTemplateArray, 
         YTemplateArray,
         M_NULL, 
         FoundCounter,
         M_LINEAR_INTERPOLATION,
         M_DEFAULT);

      /* Free previously allocated array. */
      delete [] XTemplateArray;
      delete [] YTemplateArray;
      delete [] XTargetArray;
      delete [] YTargetArray;
   }
