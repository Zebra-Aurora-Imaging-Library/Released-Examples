/************************************************************************************/
/*
* File name: DmrGenFontFile.cpp
*
* Synopsis:  This program helps users to generate a new Dot Matrix Reader
*            (SureDotOCR®) font file interactively.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/

#include <mil.h>
#include "../../DmrUtil/C++/FontUtil.h"
#include "../../DmrUtil/C++/CharacterCreator.h"
#include "../../DmrUtil/C++/VisualizeFont.h"

#if M_MIL_USE_LINUX
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#else
#include <tchar.h>
#endif
/*****************************************************************************/
/* Utility functions.                                                        */
/*****************************************************************************/

// Function that gets a user font size.
MIL_INT GetFontSize(MIL_CONST_TEXT_PTR SizeType);

// Function that removes any repeating characters.
#if M_MIL_USE_LINUX
std::u32string RemoveRepeatingChar(std::u32string Characters);
#else
mstring RemoveRepeatingChar(mstring Characters);
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
             MIL_TEXT("DmrGenFontFile\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This program helps create new Dot Matrix Reader \n")
             MIL_TEXT("(SureDotOCR) font files (.mdmrf) interactively.\n\n"));
   }

int MosMain()
   {
   PrintHeader();

   MIL_ID   MilApplication,    /* Application identifier. */
            MilSystem;         /* System identifier.      */

   MIL_TEXT_CHAR Characters[STRING_LENGTH_MAX] = MIL_TEXT("");

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
      MosPrintf(MIL_TEXT("Press 'E' to exit or press any other key to create a new font.\n\n"));

      // Get the last character.
      MyChar = (MIL_TEXT_CHAR)MosGetch();

      if (MyChar == 'e' || MyChar == 'E')
         break;

      // Ask user the font size of the application.
      MIL_INT FontSizeX = GetFontSize(MIL_TEXT("columns"));
      MIL_INT FontSizeY = GetFontSize(MIL_TEXT("rows"));

      // Add the font and set the size.
      MdmrControl(MilDmrContext, M_FONT_ADD, M_DEFAULT);
      MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                      M_FONT_SIZE_COLUMNS, (MIL_DOUBLE)FontSizeX, M_NULL);
      MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, M_NULL,
                      M_FONT_SIZE_ROWS, (MIL_DOUBLE)FontSizeY, M_NULL);
      MosPrintf(MIL_TEXT("The Dot Matrix size is %d rows x %d columns.\n\n"),
                FontSizeY, FontSizeX);

      bool AddCharacter = true;

      // Create the character creator.
      CCharCreator* pCharCreator = new CCharCreator(MilSystem, FontSizeX, FontSizeY);

      while(AddCharacter)
         {
         // Ask the user the character to create in the font.
         MosPrintf(MIL_TEXT("Type a character or string of characters to add, ")
                   MIL_TEXT("then press <Enter>: "));
#if M_MIL_USE_LINUX
         std::string InputCharacters;
         std::cin >> InputCharacters;
         std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
         std::u32string CharactersString = utf32conv.from_bytes(InputCharacters);
#else
         _tscanf_s(MIL_TEXT("%s"), Characters, (unsigned int) _countof(Characters));
         mstring CharactersString = Characters;
#endif
         MosPrintf(MIL_TEXT("\n"));


         if (CharactersString.size() == 0)
            continue;

         // Remove any repeating characters.
         if (CharactersString.size() > 1)
            CharactersString = RemoveRepeatingChar(CharactersString);

         for (MIL_UINT i = 0; i < CharactersString.size(); i++)
            {
            // Verify if the character is unique in the font file.
#if M_MIL_USE_LINUX
            mstring CharName = utf32conv.to_bytes(CharactersString[i]);
#else
            mstring CharName = CharactersString.substr(i, 1);
#endif
            MIL_INT NbChars = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT,
                                              M_NULL, M_NUMBER_OF_CHARS, M_NULL);
            if (NbChars > 0)
               {
               MIL_INT CharIndex = MdmrInquireFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT,
                                                   CharName.c_str(), M_CHAR_INDEX_VALUE, M_NULL);
               if (CharIndex != M_INVALID)
                  {
                  MosPrintf(MIL_TEXT("The CharValue %c is already defined in the font.\n\n"),
                            Characters[i]);
                  continue;
                  }
               }

            // Add a uninitialized character in the font.
            vector<MIL_UINT8> DotChar(FontSizeX * FontSizeY, 0xFF);
            MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, CharName.c_str(),
                            M_CHAR_ADD, M_DEFAULT, &DotChar[0]);

            // Get the hexadecimal char name.
            mstring CharHexName;
            FontUtil::GetCharName(MilDmrContext, CharName.c_str(), M_HEX_UTF16_FOR_ALL,
                                  &CharHexName);

            // Let the user create the character. Replace if not empty.
            if (pCharCreator->CreateCharacterInteractive(CharName.c_str(), CharHexName.c_str(), true))
               {
               // Put the modified character in the font.
               MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, CharName.c_str(),
                  M_CHAR_TEMPLATE, M_DEFAULT, &(pCharCreator->DotCharMatrix())[0]);
               }
            else
               {
               // Remove the character from the font.
               MdmrControlFont(MilDmrContext, M_FONT_INDEX(0), M_DEFAULT, CharName.c_str(),
                  M_CHAR_DELETE, M_DEFAULT, M_NULL);

               MosPrintf(MIL_TEXT("Unable to add new character because it was empty.\n\n"));
               }
            }

         // Display all the character font in a single display.
         CVisualizeFont DisplayFont(MilSystem, MilDmrContext);
         DisplayFont.VisualizeFont();

         // Ask if the user wants to add another character/characters.
         MosPrintf(MIL_TEXT("Press 'A' to add other characters or any ")
                   MIL_TEXT("other key to continue.\n\n"));

         // Get the last character.
         MyChar = (MIL_TEXT_CHAR)MosGetch();

         if (MyChar != MIL_TEXT('a') && MyChar != MIL_TEXT('A'))
            AddCharacter = false;
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

/*****************************************************************************/
/* RemoveRepeatingChar. Removes any repeating characters.                    */
/*****************************************************************************/
#if M_MIL_USE_LINUX
std::u32string RemoveRepeatingChar(std::u32string Characters)
   {
   std::u32string UniqueCharacters;

   for (MIL_UINT i = 0; i < Characters.size(); i++)
      {
      if (UniqueCharacters.find_first_of(Characters[i]) == std::u32string::npos)
         UniqueCharacters += Characters[i];
      }

   return UniqueCharacters;
   }
#else
mstring RemoveRepeatingChar(mstring Characters)
   {
   mstring UniqueCharacters = MIL_TEXT("");

   for (MIL_UINT i = 0; i < Characters.size(); i++)
      {
      if (UniqueCharacters.find_first_of(Characters[i]) == mstring::npos)
         UniqueCharacters += Characters[i];
      }

   return UniqueCharacters;
   }
#endif

/*****************************************************************************/
/* GetFontSize. Asks the user for a font size.                               */
/*****************************************************************************/
MIL_INT GetFontSize(MIL_CONST_TEXT_PTR SizeType)
   {
   char InputStream[STRING_LENGTH_MAX] = "";
   int FontSize = 0;
   while ((FontSize <= 0) || (FontSize > 100))
      {
      MosPrintf(MIL_TEXT("Enter the matrix number of %s: "), SizeType);
#if M_MIL_USE_LINUX
      fgets(InputStream, sizeof(InputStream), stdin);
      sscanf(InputStream, "%d", &FontSize);
#else
      fgets(InputStream, sizeof(InputStream), stdin);
      sscanf_s(InputStream, "%d", &FontSize);
#endif
      if ((FontSize <= 0) || (FontSize > 100))
         {
         MosPrintf(MIL_TEXT("Invalid value...\n"));         
         }
      }

   MosPrintf(MIL_TEXT("\n"));
   return (MIL_INT) FontSize;
   }
