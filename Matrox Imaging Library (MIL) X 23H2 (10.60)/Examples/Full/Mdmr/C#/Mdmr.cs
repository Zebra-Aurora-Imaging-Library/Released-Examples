//******************************************************************************
//
// File name: Mdmr.cs
//
// Synopsis:  This program uses the Dot Matrix Reader (SureDotOCR®) module to read
//            a product expiry date and lot number printed using a CIJ printer.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//
using System;
using System.Text;
using Matrox.MatroxImagingLibrary;

namespace Mdmr
{
    class Program
    {
        // MIL file specifications.
        const string IMAGE_FILE_TO_READ = MIL.M_IMAGE_PATH + "ExpiryDateAndLot.mim";
        const string FONT_FILE_TO_IMPORT = MIL.M_CONTEXT_PATH + "ExpiryDateAndLotFont5x7.mdmrf";

        // Dot Matrix Reader settings.
        const double STRING_DOT_DIAMETER = 6.0;
        const double TEXT_BLOCK_WIDTH = 400;
        const double TEXT_BLOCK_HEIGHT = 100;
        static readonly MIL_INT EXPIRY_DATE_LENGTH = 7;
        static readonly MIL_INT LOT_NUMBER_LENGTH = 7;

        // Util.
        const int TEXT_MAX_SIZE = 128;

        static void Main(string[] args)
        {
            MIL_ID MilApplication = MIL.M_NULL;                                 // Application identifier.
            MIL_ID MilSystem = MIL.M_NULL;                                      // System identifier.
            MIL_ID MilDisplay = MIL.M_NULL;                                     // Display identifier.
            MIL_ID MilImage = MIL.M_NULL;                                       // Image buffer identifier.
            MIL_ID MilOverlay = MIL.M_NULL;                                     // Overlay image.
            MIL_ID MilDmrContext = MIL.M_NULL;                                  // Dot Matrix context identifier.
            MIL_ID MilDmrResult = MIL.M_NULL;                                   // Dot Matrix result identifier.

            MIL_INT NumberOfStrings = 0;                                        // Total number of strings to read.
            MIL_INT StringSize = 0;                                             // Number of strings characters.
            MIL_INT StringModelIndex = 0;                                       // String model index.
            MIL_INT Index = 0;                                                  // Result index.

            StringBuilder StringResult = new StringBuilder(TEXT_MAX_SIZE + 1);  // String of characters read.
            string PrintText;                                                   // Util text.

            // Print module name.
            Console.Write("\nDOT MATRIX READER (SureDotOCR) MODULE:\n");
            Console.Write("--------------------------------------\n\n");

            // Allocate defaults
            MIL.MappAllocDefault(MIL.M_DEFAULT, ref MilApplication, ref MilSystem, ref MilDisplay, MIL.M_NULL, MIL.M_NULL);

            // Restore the font definition image
            MIL.MbufRestore(IMAGE_FILE_TO_READ, MilSystem, ref MilImage);

            // Display the image and prepare for overlay annotations.
            MIL.MdispSelect(MilDisplay, MilImage);
            MIL.MdispControl(MilDisplay, MIL.M_OVERLAY, MIL.M_ENABLE);
            MIL.MdispInquire(MilDisplay, MIL.M_OVERLAY_ID, ref MilOverlay);

            // Allocate a new empty Dot Matrix Reader context.
            MIL.MdmrAlloc(MilSystem, MIL.M_DOT_MATRIX, MIL.M_DEFAULT, ref MilDmrContext);

            // Allocate a new empty Dot Matrix Reader result buffer.
            MIL.MdmrAllocResult(MilSystem, MIL.M_DOT_MATRIX, MIL.M_DEFAULT, ref MilDmrResult);

            // Import a Dot Matrix font.
            MIL.MdmrImportFont(FONT_FILE_TO_IMPORT, MIL.M_DMR_FONT_FILE, MilDmrContext, MIL.M_DEFAULT, MIL.M_NULL, MIL.M_DEFAULT);

            // Add a new string model definition for the product lot number.
            // -------------------------------------------------------------
            MIL.MdmrControl(MilDmrContext, MIL.M_STRING_ADD, MIL.M_DEFAULT);

            // Set the string model rank and size
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(0), MIL.M_DEFAULT, MIL.M_STRING_RANK, 1, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(0), MIL.M_DEFAULT, MIL.M_STRING_SIZE_MIN_MAX, LOT_NUMBER_LENGTH, LOT_NUMBER_LENGTH, MIL.M_NULL);

            // Add a new string model definition for the expiry date (YYYY MM DD).
            // -------------------------------------------------------------------
            MIL.MdmrControl(MilDmrContext, MIL.M_STRING_ADD, MIL.M_DEFAULT);

            // Set the string model rank and size
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_DEFAULT, MIL.M_STRING_RANK, 0, MIL.M_DEFAULT, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_DEFAULT, MIL.M_STRING_SIZE_MIN_MAX, EXPIRY_DATE_LENGTH, EXPIRY_DATE_LENGTH, MIL.M_NULL);

            // Set the string model constraint for an expiry date (DDMMMYY).
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(0), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(1), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(2), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(3), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(4), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_LETTERS_UPPERCASE, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(5), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS, MIL.M_NULL);
            MIL.MdmrControlStringModel(MilDmrContext, MIL.M_STRING_INDEX(1), MIL.M_POSITION_IN_STRING(6), MIL.M_ADD_PERMITTED_CHARS_ENTRY, MIL.M_FONT_LABEL(MIL.M_ANY), MIL.M_DIGITS, MIL.M_NULL);

            Console.Write("A Dot Matrix Reader (SureDotOCR) context was set up to read\n" +
                          "an expiry date and a lot number from a target image.\n\n");

            // Set the Dot Matrix dot diameter.
            MIL.MdmrControl(MilDmrContext, MIL.M_DOT_DIAMETER, STRING_DOT_DIAMETER);

            // Set the maximum size of the string box.
            MIL.MdmrControl(MilDmrContext, MIL.M_TEXT_BLOCK_WIDTH, TEXT_BLOCK_WIDTH);
            MIL.MdmrControl(MilDmrContext, MIL.M_TEXT_BLOCK_HEIGHT, TEXT_BLOCK_HEIGHT);

            // Preprocess the context.
            MIL.MdmrPreprocess(MilDmrContext, MIL.M_DEFAULT);

            // Reading the string from a target image.
            MIL.MdmrRead(MilDmrContext, MilImage, MilDmrResult, MIL.M_DEFAULT);

            // Get number of strings read and show the result.
            MIL.MdmrGetResult(MilDmrResult, MIL.M_GENERAL, MIL.M_DEFAULT, MIL.M_STRING_NUMBER + MIL.M_TYPE_MIL_INT, ref NumberOfStrings);

            // Draw the read result.
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_GREEN);
            MIL.MdmrDraw(MIL.M_DEFAULT, MilDmrResult, MilOverlay, MIL.M_DRAW_STRING_CHAR_BOX, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);
            MIL.MgraColor(MIL.M_DEFAULT, MIL.M_COLOR_CYAN);
            MIL.MdmrDraw(MIL.M_DEFAULT, MilDmrResult, MilOverlay, MIL.M_DRAW_STRING_CHAR_POSITION, MIL.M_DEFAULT, MIL.M_DEFAULT, MIL.M_DEFAULT);

            if (NumberOfStrings > 0)
            {
                Console.Write("Result: {0} strings have been read:\n\n", NumberOfStrings);

                for (Index = 0; Index < NumberOfStrings; Index++)
                {
                    // Print the read result.
                    MIL.MdmrGetResult(MilDmrResult, Index, MIL.M_GENERAL, MIL.M_STRING_MODEL_INDEX + MIL.M_TYPE_MIL_INT, ref StringModelIndex);
                    MIL.MdmrGetResult(MilDmrResult, Index, MIL.M_GENERAL, MIL.M_STRING + MIL.M_STRING_SIZE + MIL.M_TYPE_MIL_INT, ref StringSize);
                    MIL.MdmrGetResult(MilDmrResult, Index, MIL.M_GENERAL, MIL.M_STRING, StringResult);

                    switch ((int)StringModelIndex)
                    {
                        case 0:
                            PrintText = string.Format(" LOT# : {0} ", StringResult);
                            MIL.MgraText(MIL.M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText);
                            Console.Write(" LOT# : {0}\n", StringResult);
                            break;

                        case 1:
                            PrintText = string.Format(" EXP. : {0} ", StringResult);
                            MIL.MgraText(MIL.M_DEFAULT, MilOverlay, 20, 20 + Index * 20, PrintText.ToString());
                            Console.Write(" EXPIRY DATE: {0}\n", StringResult);
                            break;

                        default:
                            Console.Write("Unexpected model index\n");
                            break;
                    }
                }
            }
            else
            {
                Console.Write("Error: no string found.\n");
            }

            // Pause to show results.
            Console.Write("\nPress <Enter> to end.\n\n");
            Console.ReadKey();

            // Free all allocations.
            MIL.MdmrFree(MilDmrContext);
            MIL.MdmrFree(MilDmrResult);
            MIL.MbufFree(MilImage);

            // Free defaults.
            MIL.MappFreeDefault(MilApplication, MilSystem, MilDisplay, MIL.M_NULL, MIL.M_NULL);
        }
    }
}
