//***************************************************************************************
// 
// File name: CalGenChessGrid.cpp  
//
// Synopsis:  This example generates an image of a calibration grid according to
//            the specifications in gridconfig.h. It can generate chessboard
//            grids with or without fiducials.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "common.h"

//****************************************************************************
// Example description.
//****************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("CalGenChessGrid\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example demonstrates how to generate an image of a\n")
             MIL_TEXT("calibration grid according to the user-defined specifications in\n")
             MIL_TEXT("gridconfig.h. It can generate chessboard grids with or without\n")
             MIL_TEXT("fiducials. The resulting grid can be used with the MIL camera\n")
             MIL_TEXT("calibration module (Mcal).\n\n")
             MIL_TEXT("Note, everything that is demonstrated by this example can be \n")
             MIL_TEXT("performed interactively using MIL CoPilot's \"Chessboard Grid Builder\".\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, buffer, code, display, graphics, image\n")
             MIL_TEXT("processing, system.\n\n")
  
             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//*****************************************************************************
// Draws a thick rectangle. The start and end coordinates determine the outer
// corners of the rectangle.
//*****************************************************************************
inline void DrawThickRect(MIL_ID ContextGraId, MIL_ID ImageId, MIL_DOUBLE Color,
                          MIL_INT ThicknessX, MIL_INT ThicknessY,
                          MIL_INT StartX, MIL_INT StartY,
                          MIL_INT EndX, MIL_INT EndY)
   {
   MgraColor(ContextGraId, Color);
   MgraRectFill(ContextGraId, ImageId, StartX           , StartY           , EndX               , StartY+ThicknessY-1);
   MgraRectFill(ContextGraId, ImageId, StartX           , EndY-ThicknessY+1, EndX               , EndY               );
   MgraRectFill(ContextGraId, ImageId, StartX           , StartY           , StartX+ThicknessX-1, EndY               );
   MgraRectFill(ContextGraId, ImageId, EndX-ThicknessX+1, StartY           , EndX               , EndY               );
   }

//*****************************************************************************
// Contains all necessary pixel dimensions to draw the grid image and its
// annotations.
//*****************************************************************************
struct AnnotationStruct
   {
   MIL_DOUBLE PixelsPerSquareX;        // number of pixels in the X direction for each grid square
   MIL_DOUBLE PixelsPerSquareY;        // number of pixels in the Y direction for each grid square
   MIL_INT    MaxPixelsPerSquareInt;   // maximum of PixelsPerSquareX and PixelsPerSquareY, rounded up; used as a base unit for some annotations

   MIL_INT    ImageSizeX;              // width  (in pixels) of the grid image, without annotations
   MIL_INT    ImageSizeY;              // height (in pixels) of the grid image, without annotations
   MIL_INT    FullSizeX;               // width  (in pixels) of the grid image, with    annotations
   MIL_INT    FullSizeY;               // height (in pixels) of the grid image, with    annotations

   MIL_INT    LeftBorder;              // number of pixels reserved for annotations on the left
   MIL_INT    RightBorder;             // number of pixels reserved for annotations on the right
   MIL_INT    TopBorder;               // number of pixels reserved for annotations on the top
   MIL_INT    BottomBorder;            // number of pixels reserved for annotations on the bottom

   MIL_INT    BorderThickness;         // thickness in pixels of the border separating the grid from the annotations

   MIL_INT    LastPixelOfGridX;        // X pixel coordinate of the lower-right corner of the grid (start of annotations)
   MIL_INT    LastPixelOfGridY;        // Y pixel coordinate of the lower-right corner of the grid (start of annotations)
   };

//*****************************************************************************
// Computes all necessary pixel dimensions to draw the grid image and its
// annotations, according to the grid parameters previously computed.
//*****************************************************************************
AnnotationStruct ComputeAnnotationParameters(const GridInfoStruct& GridInfo)
   {
   AnnotationStruct Annotation;

   // Compute the number of pixels per square.
   if (DPI <= 0.0)
      throw MIL_TEXT("DPI must be positive");
   MIL_DOUBLE PixelsPerInch = DPI;
   MIL_DOUBLE PixelsPerWorldUnit = GetInchesPerWorldUnit(UNIT) * PixelsPerInch;
   Annotation.PixelsPerSquareX = GridInfo.SpacingX * PixelsPerWorldUnit;
   Annotation.PixelsPerSquareY = GridInfo.SpacingY * PixelsPerWorldUnit;
   Annotation.MaxPixelsPerSquareInt = static_cast<MIL_INT>(ceil(
      Annotation.PixelsPerSquareX > Annotation.PixelsPerSquareY ?
      Annotation.PixelsPerSquareX : Annotation.PixelsPerSquareY));

   // Compute the grid image size.
   Annotation.ImageSizeX = static_cast<MIL_INT>(ceil(GridInfo.NumSquaresX * Annotation.PixelsPerSquareX));
   Annotation.ImageSizeY = static_cast<MIL_INT>(ceil(GridInfo.NumSquaresY * Annotation.PixelsPerSquareY));

   // Compute the annotation border size.
   if (DRAW_ANNOTATIONS)
      {
      Annotation.BorderThickness = static_cast<MIL_INT>(ceil(BORDER_THICKNESS * Annotation.MaxPixelsPerSquareInt));
      Annotation.LeftBorder   = Annotation.MaxPixelsPerSquareInt; // 1 square worth of space for symmetry
      Annotation.RightBorder  = Annotation.MaxPixelsPerSquareInt; // 1 square worth of space for the arrow indicator
      Annotation.TopBorder    = Annotation.MaxPixelsPerSquareInt; // 1 square worth of space for the arrow indicator

      // Since ImageSizeX is used to determine the legend font size, and we want the vertical
      // space to be proportional to the font size, compute a bottom border size proportional
      // to ImageSizeX.
      Annotation.BottomBorder = static_cast<MIL_INT>(Annotation.ImageSizeX * BOTTOM_SPACE_FACTOR);
      }
   else
      {
      // No borders, since there are no annotations.
      Annotation.BorderThickness = 0;
      Annotation.LeftBorder      = 0;
      Annotation.RightBorder     = 0;
      Annotation.TopBorder       = 0;
      Annotation.BottomBorder    = 0;
      }

   // Compute the full image size (grid + annotations).
   Annotation.FullSizeX = Annotation.ImageSizeX + Annotation.LeftBorder + Annotation.RightBorder;
   Annotation.FullSizeY = Annotation.ImageSizeY + Annotation.TopBorder  + Annotation.BottomBorder;

   // Compute lower-right corner of grid.
   Annotation.LastPixelOfGridX = Annotation.LeftBorder + Annotation.ImageSizeX - 1;
   Annotation.LastPixelOfGridY = Annotation.TopBorder  + Annotation.ImageSizeY - 1;

   return Annotation;
   }

//*****************************************************************************
// Draws the border, the legend and reference point indicators.
//*****************************************************************************
void DrawAnnotations(MIL_ID ContextGraId, MIL_ID FullImageId, const GridInfoStruct& GridInfo, const AnnotationStruct& Annotation)
   {
   // Draw the border.
   DrawThickRect(ContextGraId, FullImageId, BORDER_COLOR,
                 Annotation.BorderThickness,
                 Annotation.BorderThickness,
                 Annotation.LeftBorder       - Annotation.BorderThickness,
                 Annotation.TopBorder        - Annotation.BorderThickness,
                 Annotation.LastPixelOfGridX + Annotation.BorderThickness,
                 Annotation.LastPixelOfGridY + Annotation.BorderThickness);

   // Determine the font size for the legend.
   MIL_INT FontSize = static_cast<MIL_INT>(FONT_SIZE_FACTOR * Annotation.ImageSizeX);
   
   if (FontSize <= 9)
      throw MIL_TEXT("Font size is too small, use a higher DPI");

   MgraFont(ContextGraId, MIL_FONT_NAME(FONT_NAME));
   MgraControl(ContextGraId, M_FONT_SIZE, FontSize);
   MgraColor(ContextGraId, FOREGROUND_COLOR);
   MgraControl(ContextGraId, M_BACKCOLOR, BACKGROUND_COLOR);

   // Prepare the legend text.
   const MIL_INT MAX_LEGEND_LENGTH = 256;
   MIL_TEXT_CHAR Legend[MAX_LEGEND_LENGTH];
   MIL_CONST_TEXT_PTR UnitName = GetUnitName(UNIT);
   MosSprintf(Legend, MAX_LEGEND_LENGTH,
              MIL_TEXT("Grid size: %g %s x %g %s%sRow/column number: %d x %d%sRow/column spacing: %g %s x %g %s"),
              GridInfo.GridSizeX, UnitName, GridInfo.GridSizeY, UnitName, SEPARATOR,
              (int)(GridInfo.NumSquaresY - 2 * NUM_SQUARES_FOR_QUIET_ZONE + 1),
              (int)(GridInfo.NumSquaresX - 2 * NUM_SQUARES_FOR_QUIET_ZONE + 1), SEPARATOR,
              GridInfo.SpacingY, UnitName, GridInfo.SpacingX, UnitName);
   
   // Draw the legend.
   MIL_INT TextVerticalOffset = static_cast<MIL_INT>(TEXT_VERTICAL_OFFSET * Annotation.ImageSizeX);
   MgraText(ContextGraId, FullImageId, Annotation.PixelsPerSquareX, Annotation.LastPixelOfGridY + TextVerticalOffset, Legend);

#if NUM_FIDUCIALS > 0
   const MIL_INT TRIANGLE_LENGTH_TO_WIDTH_RATIO = 2;
   // Draw triangles to indicate the grid reference point.
   MIL_INT RefPointSquareNoX = GridInfo.GetReferencePositionX();
   MIL_INT RefPointSquareNoY = GridInfo.GetReferencePositionY();
   MIL_INT RefPointPixelPosX = static_cast<MIL_INT>(RefPointSquareNoX * Annotation.PixelsPerSquareX) + Annotation.LeftBorder;
   MIL_INT RefPointPixelPosY = static_cast<MIL_INT>(RefPointSquareNoY * Annotation.PixelsPerSquareY) + Annotation.TopBorder;
   MIL_INT TriangleOffset = static_cast<MIL_INT>(ceil(TRIANGLE_OFFSET * Annotation.MaxPixelsPerSquareInt));
   MIL_INT TriangleLength = static_cast<MIL_INT>(ceil(TRIANGLE_LENGTH * Annotation.MaxPixelsPerSquareInt));
   MgraColor(ContextGraId, FOREGROUND_COLOR);
   MIL_INT TriangleVerticesX[3], TriangleVerticesY[3];

   // Draw the triangle to the top.
   TriangleVerticesX[0] = RefPointPixelPosX;
   TriangleVerticesX[1] = TriangleVerticesX[0] - TriangleLength / (2 * TRIANGLE_LENGTH_TO_WIDTH_RATIO);
   TriangleVerticesX[2] = TriangleVerticesX[0] + TriangleLength / (2 * TRIANGLE_LENGTH_TO_WIDTH_RATIO);

   TriangleVerticesY[0] = Annotation.TopBorder - TriangleOffset;
   TriangleVerticesY[1] = TriangleVerticesY[0] - TriangleLength;
   TriangleVerticesY[2] = TriangleVerticesY[1];

   MgraLines(ContextGraId, FullImageId, 3, TriangleVerticesX, TriangleVerticesY, M_NULL, M_NULL, M_POLYGON+M_FILLED);

   // Draw the triangle to the right or to the left, whichever is closest.
   if (RefPointSquareNoX <= GridInfo.NumSquaresX / 2)
      {
      // To the left.
      TriangleVerticesX[0] = Annotation.LeftBorder - TriangleOffset;
      TriangleVerticesX[1] = TriangleVerticesX[0] - TriangleLength;
      }
   else
      {
      // To the right.
      TriangleVerticesX[0] = Annotation.LastPixelOfGridX + TriangleOffset;
      TriangleVerticesX[1] = TriangleVerticesX[0] + TriangleLength;      
      }
   TriangleVerticesX[2] = TriangleVerticesX[1];

   TriangleVerticesY[0] = RefPointPixelPosY;
   TriangleVerticesY[1] = TriangleVerticesY[0] - TriangleLength / (2 * TRIANGLE_LENGTH_TO_WIDTH_RATIO);
   TriangleVerticesY[2] = TriangleVerticesY[0] + TriangleLength / (2 * TRIANGLE_LENGTH_TO_WIDTH_RATIO);

   MgraLines(ContextGraId, FullImageId, 3, TriangleVerticesX, TriangleVerticesY, M_NULL, M_NULL, M_POLYGON+M_FILLED);
#endif
   }

//*****************************************************************************
// Compute a zoom factor so that the grid image can fit in the screen.
//*****************************************************************************
void SetZoomFactor(MIL_ID DispId, MIL_INT GridSizeX, MIL_INT GridSizeY)
   {
   MIL_DOUBLE ZoomFactor = 1.0;
   if (GridSizeX > MAX_DISPLAY_SIZE_X)
      ZoomFactor = static_cast<MIL_DOUBLE>(MAX_DISPLAY_SIZE_X) / GridSizeX;
   if (GridSizeY > MAX_DISPLAY_SIZE_Y)
      {
      MIL_DOUBLE MaxZoomFactor = static_cast<MIL_DOUBLE>(MAX_DISPLAY_SIZE_Y) / GridSizeY;
      if (MaxZoomFactor < ZoomFactor)
         ZoomFactor = MaxZoomFactor;
      }
   if (ZoomFactor < 1.0)
      MdispZoom(DispId, ZoomFactor, ZoomFactor);
   }

//*****************************************************************************
// Structure containing all MIL objects. Ensures that all objects will be
// correctly freed, even in the presence of exceptions.
//*****************************************************************************
struct MILObjectsStruture
   {
   MIL_ID AppId;        // application context
   MIL_ID SysId;        // system
   MIL_ID ContextGraId; // graphics context
   MIL_ID FullImageId;  // image buffer of the grid, with annotations if applicable
   MIL_ID GridImageId;  // child buffer of FullImageId, contains only the grid
   MIL_ID DispId;       // display

   // Constructor cleanly initializes all identifiers.
   MILObjectsStruture()
      : AppId       (M_NULL),
        SysId       (M_NULL),
        ContextGraId(M_NULL),
        FullImageId (M_NULL),
        GridImageId (M_NULL),
        DispId      (M_NULL)
      {
      
      }

   // Destructor ensures all objects are freed, even in the presence of exceptions.
   ~MILObjectsStruture()
      {
      if (DispId       != M_NULL) MdispFree(DispId     );
      if (GridImageId  != M_NULL) MbufFree(GridImageId );
      if (FullImageId  != M_NULL) MbufFree(FullImageId );
      if (ContextGraId != M_NULL) MgraFree(ContextGraId);
      if (SysId        != M_NULL) MsysFree(SysId       );
      if (AppId        != M_NULL) MappFree(AppId       );
      }
   };

//*****************************************************************************
// Main.
//*****************************************************************************
int MosMain(void)
   {
   PrintHeader();

   try
      {
      // Allocate MIL objects.
      MILObjectsStruture MILObjects;
      MappAlloc(M_NULL, M_DEFAULT, &MILObjects.AppId);
      MsysAlloc(MILObjects.AppId, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MILObjects.SysId);
      MgraAlloc(MILObjects.SysId, &MILObjects.ContextGraId);

      // Compute all grid and annotation parameters according to gridconfig.h.
      GridInfoStruct   GridInfo   = ComputeGridParameters();
      AnnotationStruct Annotation = ComputeAnnotationParameters(GridInfo);

      // Allocate the full image.
      MbufAlloc2d(MILObjects.SysId, Annotation.FullSizeX, Annotation.FullSizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &MILObjects.FullImageId);
      MbufChild2d(MILObjects.FullImageId, Annotation.LeftBorder, Annotation.TopBorder, Annotation.ImageSizeX, Annotation.ImageSizeY, &MILObjects.GridImageId);
      MbufClear(MILObjects.FullImageId, BACKGROUND_COLOR);

      // Create the chessboard pattern.
      MgraColor(MILObjects.ContextGraId, FOREGROUND_COLOR);
      for (MIL_INT y = 0; y < GridInfo.NumSquaresY; ++y)
         {
         for (MIL_INT x = 0; x < GridInfo.NumSquaresX; ++x)
            {
            if ((x & 0x1) == (y & 0x1)) // if this is a black cell
               {
               MgraRectFill(MILObjects.ContextGraId, MILObjects.GridImageId,
                            x * Annotation.PixelsPerSquareX, y * Annotation.PixelsPerSquareY,
                            (x+1) * Annotation.PixelsPerSquareX - 1, (y+1) * Annotation.PixelsPerSquareY - 1);
               }
            }
         }

      // Create the quiet zone.
      DrawThickRect(MILObjects.ContextGraId, MILObjects.GridImageId, BACKGROUND_COLOR,
                    static_cast<MIL_INT>(QUIET_ZONE_BORDER * Annotation.PixelsPerSquareX),
                    static_cast<MIL_INT>(QUIET_ZONE_BORDER * Annotation.PixelsPerSquareY),
                    0, 0, Annotation.ImageSizeX-1, Annotation.ImageSizeY-1);

      // Draw fiducials, if any.
      AddFiducials(MILObjects.GridImageId, GridInfo, Annotation.PixelsPerSquareX, Annotation.PixelsPerSquareY);

      // If enabled, draw the grid border, the reference point indicators and the legend.
      if (DRAW_ANNOTATIONS)
         DrawAnnotations(MILObjects.ContextGraId, MILObjects.FullImageId, GridInfo, Annotation);

      // Save the grid image with the correct DPI.
      MbufControl(MILObjects.FullImageId, M_RESOLUTION_X, DPI);
      MbufControl(MILObjects.FullImageId, M_RESOLUTION_Y, DPI);
      MbufExport(OUTPUT_GRID_NAME, OUTPUT_FILE_FORMAT, MILObjects.FullImageId);

      // Show the image and print some information.
      MdispAlloc(MILObjects.SysId, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MILObjects.DispId);
      SetZoomFactor(MILObjects.DispId, Annotation.FullSizeX, Annotation.FullSizeY);
      MdispSelect(MILObjects.DispId, MILObjects.FullImageId);

      MosPrintf(MIL_TEXT("Image saved:\n"));
      MosPrintf(MIL_TEXT("------------\n"));
      MosPrintf(MIL_TEXT("  Name: '%s'\n"), OUTPUT_GRID_NAME);
      MosPrintf(MIL_TEXT("  Size: %d x %d\n"), (int)Annotation.FullSizeX, (int)Annotation.FullSizeY);
      MosPrintf(MIL_TEXT("\n"));
      MosPrintf(MIL_TEXT("To print this image correctly:\n"));
      MosPrintf(MIL_TEXT("  - Set your printer resolution to %d DPI or higher.\n"), (int)DPI);
      MosPrintf(MIL_TEXT("  - Print with software that takes the DPI into account.\n"));
      MosPrintf(MIL_TEXT("  - Disable any 'fit' or 'scale' option in the print dialog.\n"));
      MosPrintf(MIL_TEXT("  - Verify the printed grid dimensions.\n"));
      MosPrintf(MIL_TEXT("\n"));
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();

      // All MIL objects are freed here, in MILObjectsStruture's destructor.
      }
   catch (MIL_CONST_TEXT_PTR ErrorMessage)
      {
      MosPrintf(MIL_TEXT("\nERROR:\n  %s.\n\n"), ErrorMessage);
      MosPrintf(MIL_TEXT("Press <Enter> to end.\n"));
      MosGetch();
      }

   return 0;
   }
