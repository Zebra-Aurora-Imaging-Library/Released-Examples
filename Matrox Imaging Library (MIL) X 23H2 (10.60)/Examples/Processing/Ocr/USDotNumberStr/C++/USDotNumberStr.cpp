﻿/****************************************************************************************/
//
// File name: USDotNumberStr.cpp
//
// Synopsis:  This example demonstrates how to use the String Reader module to read a U.S.
//            Department of Transportation number. A vertical top-hat filtering is applied
//            before the read operation to enhance image quality.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//***************************************************************************************/

#include <mil.h> 

// Source files specifications.
#define EXAMPLE_PATH                   M_IMAGE_PATH MIL_TEXT("USDotNumberStr/")
#define IMAGE_FILE                     EXAMPLE_PATH MIL_TEXT("USDotNumberStr.mim")
MIL_CONST_TEXT_PTR STRING_FONT  =    EXAMPLE_PATH MIL_TEXT("USDotNumberStr.msr");

// Kernel data definition.
#define STRUCT_ELEM_WIDTH     1
#define STRUCT_ELEM_HEIGHT    7
#define STRUCT_ELEM_DEPTH     32
#define ITERATION_NB          5

// Function declaration.
void ReadString(MIL_ID MilImage, MIL_ID MilGraphicList, MIL_CONST_TEXT_PTR StringFont);

//*****************************************************************************
// Example description.
//*****************************************************************************
void PrintHeader()
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("USDotNumberStr\n\n")
             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to use the String Reader module to read a U.S.\n")
             MIL_TEXT("Department of Transportation number. A vertical top-hat filtering is applied\n")
             MIL_TEXT("before the read operation to enhance image quality.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("image processing, string reader.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n"));
   MosGetch();
   }

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   MIL_ID   MilApplication,      // Application identifier.
            MilSystem,           // System identifier.
            MilDisplay,          // Display identifier.
            MilGraphicList,      // Graphic list identifier.
            MilImage,            // Image buffer identifier.
            MilDispImage,        // Display Image buffer identifier.
            MilStructElem;       // Custom structuring element identifier.

   MIL_DOUBLE  AnnotationColor = M_COLOR_GREEN;

   // Allocate objects.
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);
   MsysAlloc(MilApplication, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   // Allocate a graphic list to hold the subpixel annotations to draw.
   MgraAllocList(MilSystem, M_DEFAULT, &MilGraphicList);
   // Associate the graphic list to the display.
   MdispControl(MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, MilGraphicList);
   MgraColor(M_DEFAULT, AnnotationColor);

   // Print header.
   PrintHeader();

   // Restore and display the original image.
   MbufRestore(IMAGE_FILE, MilSystem, &MilImage);
   MbufRestore(IMAGE_FILE, MilSystem, &MilDispImage);
   MdispSelect(MilDisplay, MilDispImage);

   // Pause to show the original image.
   MosPrintf(MIL_TEXT("\nThe original image is displayed.\n\n")
   MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate a MIL structuring element.
   MbufAlloc2d(MilSystem, STRUCT_ELEM_WIDTH, STRUCT_ELEM_HEIGHT, STRUCT_ELEM_DEPTH+M_UNSIGNED,
      M_STRUCT_ELEMENT, &MilStructElem);
   MbufClear(MilStructElem, 0);

   // Apply the top-hat filter.
   MimMorphic(MilImage, MilDispImage, MilStructElem, M_TOP_HAT, ITERATION_NB, M_GRAYSCALE);

   MosPrintf(MIL_TEXT("A top-hat filtering using a custom %dx%d structuring element is applied\n"),
      STRUCT_ELEM_WIDTH,STRUCT_ELEM_HEIGHT);
   MosPrintf(MIL_TEXT("to improve the background uniformity.\n\n")
   MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Read the string.
   ReadString(MilDispImage, MilGraphicList, STRING_FONT);
   MosPrintf(MIL_TEXT("Press <Enter> to finish.\n"));
   MosGetch();

   // Free allocated resources.
   MbufFree(MilStructElem);
   MbufFree(MilDispImage);
   MbufFree(MilImage);
   MgraFree(MilGraphicList);

   MdispFree(MilDisplay);
   MappFreeDefault(MilApplication, MilSystem, M_NULL, M_NULL, M_NULL);
   return 0;
   }

void ReadString(MIL_ID MilImage, MIL_ID MilGraphicList, MIL_CONST_TEXT_PTR StringFont)
   {
   MIL_ID         MilStrContext,             // String context identifier.
                  MilStrResult;              // String result buffer identifier.
   MIL_TEXT_CHAR  StringResult[20];          // String of characters.
   MIL_DOUBLE     Score;                     // String score.

   MIL_ID MilSystem = MbufInquire(MilImage, M_OWNER_SYSTEM, M_NULL);

   // Restore the context. 
   MstrRestore(StringFont, MilSystem, M_DEFAULT, &MilStrContext);
   // Preprocess the context.
   MstrPreprocess(MilStrContext, M_DEFAULT);

   // Allocate a new empty String Reader result buffer.
   MstrAllocResult(MilSystem, M_DEFAULT, &MilStrResult);

   // Read the string.
   MstrRead(MilStrContext, MilImage, MilStrResult);

   // Get and print out the result.
   MstrGetResult(MilStrResult, 0, M_STRING+M_TYPE_TEXT_CHAR, StringResult);
   MstrGetResult(MilStrResult, 0, M_STRING_SCORE, &Score);

   MosPrintf(MIL_TEXT("The string starting with \"USDOT\" is read using a pre-defined context.\n")
             MIL_TEXT(" -----------------------------\n")
             MIL_TEXT(" String                  Score\n")
             MIL_TEXT(" -----------------------------\n"));
   MosPrintf(MIL_TEXT(" %s             %.1f\n\n"), StringResult, Score);

   // Draw the string and the box.
   MstrDraw(M_DEFAULT, MilStrResult, MilGraphicList, M_DRAW_STRING_BOX+M_DRAW_STRING, M_ALL,
      M_NULL, M_DEFAULT); 

   // Free allocated resources.
   MstrFree(MilStrResult);
   MstrFree(MilStrContext);
   }
