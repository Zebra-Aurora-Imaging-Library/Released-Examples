﻿/************************************************************************************/
/*
* File name: DmrEditFontFile.cpp
*
* Synopsis:  This program helps the user to edit a Dot Matrix Reader (SureDotOCR®) font.
*            The user can also add new characters to the font.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include "../../DmrUtil/C++/FontUtil.h"
#include "../../DmrUtil/C++/CharacterCreator.h"
#include "../../DmrUtil/C++/VisualizeFont.h"

#if M_MIL_USE_WINDOWS
#include <tchar.h>
#endif
/*****************************************************************************/
/* Constants.                                                                */
/*****************************************************************************/

// Maximum length of the string to read.
static const MIL_INT STRING_LENGTH_MAX = 255;

/*****************************************************************************/
/* Example description.                                                      */
/*****************************************************************************/
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("DmrEditFontFile\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program helps edit a Dot Matrix Reader (SureDotOCR) font (.mdmrf).\n")
             MIL_TEXT("The user can modify a character or add a new character to\n")
             MIL_TEXT("the font file interactively.\n\n"));

   MosPrintf(MIL_TEXT("Note that predefined font files can be found in\n")
             MIL_TEXT("your \\Matrox Imaging\\Contexts installation directory.\n\n"));
   }

int MosMain()
   {
   PrintHeader();

   MIL_ID   MilApplication,           /* Application identifier. */
            MilSystem;                /* System identifier.      */

   MIL_TEXT_CHAR UserCharName[STRING_LENGTH_MAX] = MIL_TEXT("");

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
      MosPrintf(MIL_TEXT("Press 'E' to exit or press any other key to edit a font.\n\n"));

      // Get the last character.
      MyChar = (MIL_TEXT_CHAR)MosGetch();

      if (MyChar == 'e' || MyChar == 'E')
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

      // Get the font size of the application.
      MIL_INT FontSizeX = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT,
                                          M_NULL, M_FONT_SIZE_COLUMNS, M_NULL);
      MIL_INT FontSizeY = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT,
                                          M_NULL, M_FONT_SIZE_ROWS, M_NULL);

      // Create the character creator class that will be used to generate characters.
      CCharCreator* pCharCreator = new CCharCreator(MilSystem, FontSizeX, FontSizeY);

      bool AddCharacter = true;
      while(AddCharacter)
         {
         // Display all the character font in a single display.
         CVisualizeFont DisplayFont(MilSystem, MilDmrContext);
         DisplayFont.VisualizeFont();

         // Ask if the user wants to edit another character/characters.
         MosPrintf(MIL_TEXT("Press 'D' to edit a character or press any ")
                     MIL_TEXT("other key to continue.\n\n"));

         // Get the last character.
         MyChar = (MIL_TEXT_CHAR)MosGetch();

         if (MyChar != MIL_TEXT('d') && MyChar != MIL_TEXT('D'))
            {
            AddCharacter = false;
            break;
            }

         // Ask the user to type a character to edit.
         MosPrintf(MIL_TEXT("Type a character value to edit then press <Enter>: "));
#if M_MIL_USE_LINUX
         fgets(UserCharName, STRING_LENGTH_MAX, stdin);
         UserCharName[strcspn(UserCharName, MIL_TEXT("\n"))] = MIL_TEXT('\0');
#else
         _fgetts(UserCharName, _countof(UserCharName), stdin);
         UserCharName[_tcscspn(UserCharName, MIL_TEXT("\n"))] = MIL_TEXT('\0');
#endif
         MosPrintf(MIL_TEXT("\n"));

         // Find the character in the existing font.
         MappControl(M_ERROR, M_PRINT_DISABLE);
         MIL_INT CharIndex = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT,
                                             UserCharName, M_CHAR_INDEX_VALUE, M_NULL);
         MIL_INT InquireFontError = MappGetError(M_DEFAULT, M_CURRENT, M_NULL);
         MappControl(M_ERROR, M_PRINT_ENABLE);

         // If the character name was valid.
         if (InquireFontError == M_NULL_ERROR)
            {
            bool NewChar = CharIndex == M_INVALID;
            vector<MIL_UINT8> DotChar(FontSizeX * FontSizeY, 0xFF);
            if (NewChar)
               MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, UserCharName,
                               M_CHAR_ADD, M_DEFAULT, &DotChar[0]);
            else
               {
               // Get the character matrix and load it in the character creator.
               MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, UserCharName,
                               M_CHAR_TEMPLATE + M_TYPE_MIL_UINT8, &DotChar[0]);
               pCharCreator->LoadCharacter(DotChar);
               }

            // Get the char name and the hexadecimal char name.
            mstring CharName;
            FontUtil::GetCharName(MilDmrContext, UserCharName, M_NULL, &CharName);
            mstring CharHexName;
            FontUtil::GetCharName(MilDmrContext, UserCharName, M_HEX_UTF16_FOR_ALL, &CharHexName);

            // Create the character. Add it if not empty.
            if (pCharCreator->CreateCharacterInteractive(CharName.c_str(), CharHexName.c_str(), NewChar))
               {
               // Put the modified character in the font.
               MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, UserCharName,
                               M_CHAR_TEMPLATE, M_DEFAULT, &(pCharCreator->DotCharMatrix())[0]);
               }
            else if (NewChar)
               {
               // Remove the character from the font if it is new.
               MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, CharName.c_str(),
                               M_CHAR_DELETE, M_DEFAULT, M_NULL);
               MosPrintf(MIL_TEXT("Unable to add new character because it was empty.\n\n"));
               }
            else
               MosPrintf(MIL_TEXT("Unable to replace character because the new character was empty.\n\n"));

            }
         else
            MosPrintf(MIL_TEXT("Unable to edit the supplied character. The character may ")
                      MIL_TEXT("be invalid.\n\n"));
         }
      delete pCharCreator;

      // Export the font.
      FontUtil::ExportFont(MilDmrContext, 0);

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
