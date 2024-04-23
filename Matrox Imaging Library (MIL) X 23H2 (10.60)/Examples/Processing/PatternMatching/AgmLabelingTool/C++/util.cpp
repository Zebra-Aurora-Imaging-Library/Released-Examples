//***************************************************************************************
//
// File name: util.cpp
//
// Synopsis:  Implements useful functions for the labeling tool.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************
#include "util.h"

// ===========================================================================
std::vector<MIL_STRING> ListImagesInFolder(const MIL_STRING& FileToSearch)
   {

   MIL_INT NumberOfFiles;
   MappFileOperation(M_DEFAULT, FileToSearch, M_NULL, M_NULL, M_FILE_NAME_FIND_COUNT, M_DEFAULT, &NumberOfFiles);
   std::vector<MIL_STRING> FilesInFolder(NumberOfFiles);

   for(MIL_INT i = 0; i < NumberOfFiles; i++)
      {
      MIL_STRING Filename;
      MappFileOperation(M_DEFAULT, FileToSearch, M_NULL, M_NULL, M_FILE_NAME_FIND, i, Filename);
      FilesInFolder[i] = Filename;
      }
   return FilesInFolder;
   }

// ===========================================================================
SRectangle CvtGraRectangle(MIL_ID GraList, MIL_INT RectGraLabel)
   {
   SRectangle Box;
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_CORNER_TOP_LEFT_X, &Box.TopLeft.x);
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_CORNER_TOP_LEFT_Y, &Box.TopLeft.y);
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_CORNER_BOTTOM_RIGHT_X, &Box.BottomRight.x);
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_CORNER_BOTTOM_RIGHT_Y, &Box.BottomRight.y);
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_COLOR + M_TYPE_MIL_INT, &Box.Color);
   MgraInquireList(GraList, M_GRAPHIC_LABEL(RectGraLabel), M_DEFAULT, M_RESIZABLE + M_TYPE_MIL_INT, &Box.Resizable);
   return Box;
   }
