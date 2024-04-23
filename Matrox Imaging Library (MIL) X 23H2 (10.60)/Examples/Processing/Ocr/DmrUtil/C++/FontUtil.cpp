/************************************************************************************/
/*
* File name: FontUtil.cpp
*
* Synopsis:  This file contains utility functions to manage font in dmr context and
*            font files.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include "FontUtil.h"

/*****************************************************************************/
/* ExportFont. Export the font of the context, prompting the user            */
/*             if something went wrong.                                      */
/*****************************************************************************/
void FontUtil::ExportFont(MIL_ID MilDmrContext, MIL_INT FontIndex)
   {
   MIL_INT ExportError = M_NULL_ERROR;
   do
      {
      MappControl(M_ERROR, M_PRINT_DISABLE);
      MdmrExportFont(M_INTERACTIVE, M_DMR_FONT_FILE, MilDmrContext, M_FONT_INDEX(FontIndex),
                     M_DEFAULT);
      MappGetError(M_DEFAULT, M_CURRENT, &ExportError);
      MappControl(M_ERROR, M_PRINT_ENABLE);
      if (ExportError != M_NULL_ERROR)
         {
         MosPrintf(MIL_TEXT("Created font file could not be exported!\n\n"));
         MosPrintf(MIL_TEXT("Press 'R' to retry or any other key to continue.\n\n"));
         MIL_TEXT_CHAR MyChar = (MIL_TEXT_CHAR)MosGetch();
         if (MyChar != MIL_TEXT('r') && MyChar != MIL_TEXT('R'))
            ExportError = M_NULL_ERROR;
         }
      } while (ExportError != M_NULL_ERROR);
   }

/*****************************************************************************/
/* GetCharName. Gets the character name from a dmr context.                  */
/*****************************************************************************/
void FontUtil::GetCharName(MIL_ID MilDmrContext, MIL_CONST_TEXT_PTR UserCharName,
   MIL_INT ControlFlag, mstring* pCharName)
   {
   MIL_INT CharNameSize = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, UserCharName,
      M_CHAR_NAME + ControlFlag + M_STRING_SIZE, M_NULL);
   pCharName->resize(CharNameSize-1);
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, UserCharName,
      M_CHAR_NAME + ControlFlag, &(*pCharName)[0]);
   }

void FontUtil::GetCharName(MIL_ID MilDmrContext, MIL_INT CharIndex,
   MIL_INT ControlFlag, mstring* pCharName)
   {
   MIL_INT CharNameSize = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), CharIndex, M_NULL,
      M_CHAR_NAME + ControlFlag + M_STRING_SIZE, M_NULL);
   pCharName->resize(CharNameSize-1);
   MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), CharIndex, M_NULL,
      M_CHAR_NAME + ControlFlag, &(*pCharName)[0]);
   }
