/************************************************************************************/
/*
* File name: DmrShowFontFile.cpp
*
* Synopsis:  This program helps the user to display the characters
*            of a Dot Matrix Reader (SureDotOCR®) font file.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include "mil.h"
#include "../../DmrUtil/C++/VisualizeFont.h"


/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DmrShowFontFile\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program displays the characters of a\n")
             MIL_TEXT("Dot Matrix Reader (SureDotOCR) font file (.mdmrf).\n\n"));

   MosPrintf(MIL_TEXT("Note that predefined font files can be found in\n")
             MIL_TEXT("your \\Matrox Imaging\\Contexts installation directory.\n\n"));
   }

int MosMain()
   {
   PrintHeader();

   MIL_ID   MilApplication,    /* Application identifier. */
            MilSystem;         /* System identifier.      */

   // Allocate the application.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);

   // Alloc the system
   MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   // Allocate a dmr context to stream the font.
   MIL_ID MilDmrContext = MdmrAlloc(MilSystem, M_DOT_MATRIX, M_DEFAULT, M_NULL);

   // Ask User to press any key to create a font file or to press 'e' to exit.
   MIL_TEXT_CHAR MyChar = ' ';

   while(MyChar != 'e' || MyChar != 'E')
      {
      MosPrintf(MIL_TEXT("Press 'E' to exit, or press any other key to load a font file.\n\n"));

      /* Get the last character. */
      MyChar = (MIL_TEXT_CHAR)MosGetch();

      if ( (MyChar == 'e') || (MyChar == 'E') )
         break;

      // Import a font interactively.
      MappControl(M_ERROR, M_PRINT_DISABLE);
      MdmrImportFont(M_INTERACTIVE, M_DMR_FONT_FILE, MilDmrContext, M_NEW_LABEL,
                     M_IMPORT_ALL_CHARS, M_DEFAULT);
      MappControl(M_ERROR, M_PRINT_ENABLE);

      if (MdmrInquire(MilDmrContext, M_NUMBER_OF_FONTS, M_NULL) == 0)
         {
         MosPrintf(MIL_TEXT("File opening failed.\n"));
         MosPrintf(MIL_TEXT("Please check the context and make sure you have a MIL Full license.\n\n"));
         continue;
         }

      // Display all the character font in a single display.
      CVisualizeFont DisplayFont(MilSystem, MilDmrContext);

      DisplayFont.VisualizeFont();
      DisplayFont.SaveDisplayFontImage();

      // Remove the font.
      MdmrControl(MilDmrContext, M_FONT_DELETE, M_FONT_INDEX(0));
      }

   // Free the dmr context.
   MdmrFree(MilDmrContext);

   // Free system and application.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
   }

