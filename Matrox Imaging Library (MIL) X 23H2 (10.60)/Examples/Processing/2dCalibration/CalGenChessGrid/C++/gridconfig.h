//***************************************************************************************
// 
// File name: gridconfig.h 
//
// Synopsis:  This file includes configuration variables that can be changed to
//            alter the generated grid image.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef GRID_CONFIG_H
#define GRID_CONFIG_H

// MODIFY VALUES IN THIS FILE ACCORDING TO YOUR SETUP.

//*****************************************************************************
// General configuration.

// Image file extension to use. M_PNG is recommended for its lossless compression.
static const MIL_INT64 OUTPUT_FILE_FORMAT = M_PNG;

// Output file names. Must use the extension above.
#define SAVE_PATH    MIL_TEXT("")
static const MIL_CONST_TEXT_PTR OUTPUT_GRID_NAME = SAVE_PATH MIL_TEXT("ChessGrid.png");
static const MIL_CONST_TEXT_PTR OUTPUT_CODE_NAME = SAVE_PATH MIL_TEXT("Fiducial_%d.png"); // must contain a %d

// "Dots per inch", number of pixels to generate per printed inch.
// Usually, its the printer resolution.
static const MIL_DOUBLE DPI = 600.0;

// Units used to express all dimensions and spacings. Gets encoded in the fiducials.
static const UnitEnum UNIT = Millimeters;

//*****************************************************************************
// Configuration for ComputeGridParameters().

// If you know the number of squares in the chessboard grid, set this define to 1.
// Otherwise, set it to 0 and specify the camera resolution and field of view.
// Note that, if set to 0, RowSpacing and ColumnSpacing will be set to the same value.
#define SPECIFY_NUM_SQUARES_DIRECTLY 0

// Desired grid dimensions in world units, excluding the quiet zone.
// This should cover the entire field of view of all the cameras in the system.
static const MIL_DOUBLE MIN_GRID_SIZE_X = 200.0;
static const MIL_DOUBLE MIN_GRID_SIZE_Y = 150.0;

#if !SPECIFY_NUM_SQUARES_DIRECTLY

   // Minimum size of a chessboard square in the camera image. A minimum of 40 pixels is
   // recommended to ensure correct fiducial decoding.
   static const MIL_INT MIN_CAMERA_PIXELS_PER_SQUARE = 40;

   // Field of view of the camera, in world units, on the calibration plane, along the camera's
   // X axis. If you are using multiple cameras, specify the largest field of view.
   static const MIL_DOUBLE CAMERA_FOV_X = 200.0;

   // Number of pixels along the camera's X axis. If you are using multiple cameras, specify
   // the smallest resolution.
   static const MIL_INT CAMERA_RESOLUTION_X = 1280;

#else

   // Desired number of grid rows and columns. Choose numbers so that each chessboard
   // square covers at least 40x40 pixels in the camera images.
   static const MIL_INT MAX_NUM_SQUARES_X = 20;
   static const MIL_INT MAX_NUM_SQUARES_Y = 15;

   // Enforce square chessboard cells, i.e. RowSpacing == ColumnSpacing.
   // If true, desired row and column spacings will be computed independently,
   // and the largest will be used for both spacings.
   // If true, the encoded string (and thus the fiducial) is smaller.
   static bool ENFORCE_SQUARE_CELLS = true;

#endif

// The desired spacings will be rounded according to the forms below.
// The spacing exponent n is always in {-5..4}.
enum SpacingRoundingEnum
   {
   RoundUpTo3Digits,    // a.bc x 10^n,   a, b and c in {0..9}  (larger  fiducial)
   RoundUpTo1Digit,     // a    x 10^n,   a in {1..9}           (smaller fiducial)
   RoundUpTo1or2or5     // a    x 10^n,   a in {1,2,5}          (smaller fiducial, same size as RoundUpTo1Digit)
   };
static const SpacingRoundingEnum SPACING_ROUNDING = RoundUpTo1or2or5;

//*****************************************************************************
// Configuration for AddFiducials().

// Number of fiducials to be inserted in the grid. Can be 0.
// A grid with multiple fiducials is useful for setups with multiple cameras
// having non-overlapping field-of-views.
#define NUM_FIDUCIALS 1

#if NUM_FIDUCIALS > 0

// Position of the grid reference point. This is the calibration point whose
// world coordinates are the grid offsets passed to McalGrid(), usually 0.
// So, usually, this is the origin of the absolute coordinate system.
// Set this position in terms of grid squares.
//   - (0, 0) would be the top-left calibration point
//   - use CENTER to specify the grid center
static const MIL_INT REF_POINT_POS_X = CENTER;
static const MIL_INT REF_POINT_POS_Y = CENTER;

// Position of the fiducial's top-left corner with respect to the grid reference
// point, in terms of grid squares.
static const MIL_INT FIDUCIAL_POS_X[NUM_FIDUCIALS] = {0};
static const MIL_INT FIDUCIAL_POS_Y[NUM_FIDUCIALS] = {0};

// Fiducial size, in number of squares. Can be 2 (for 2x2) or 3 (for 3x3).
// 2x3 and 3x2 fiducials are supported by the calibration module, but can't be
// generated in this example.
static const MIL_INT FIDUCIAL_SIZE[NUM_FIDUCIALS] = {2};

#endif

//*****************************************************************************
// Annotations

// Indicates whether to draw annotations outside the grid:
//   - a faint border indicating the quiet zone around the grid
//   - the grid dimensions, number of calibration points and spacings as text
//   - row and column triangle markers for the grid reference point
// Look at DrawAnnotations() in CalGenChessGrid.cpp to disable specific annotations.
static const bool DRAW_ANNOTATIONS = true;

// Colors used to draw the chessboard, the fiducials and the annotations.
static const MIL_DOUBLE FOREGROUND_COLOR =   0.0;
static const MIL_DOUBLE BACKGROUND_COLOR = 255.0;
static const MIL_DOUBLE BORDER_COLOR     = 208.0;

// Font used to write the legend.
static const MIL_CONST_TEXT_PTR FONT_NAME = M_FONT_DEFAULT_TTF;

//*****************************************************************************
// Display

// Maximum size of the MIL display showing the generated calibration grid.
// This is used to determine the correct zoom level.
static const MIL_INT MAX_DISPLAY_SIZE_X = 1280;
static const MIL_INT MAX_DISPLAY_SIZE_Y = 720;

//*****************************************************************************
// You should not need to change these constants.

static const MIL_INT    NUM_SQUARES_FOR_QUIET_ZONE   = 1;    // add one square outside the grid
static const MIL_DOUBLE QUIET_ZONE_BORDER            = 0.5;  // leave 0.5 empty square on the border
static const MIL_DOUBLE FIDUCIAL_INDENT              = 0.4;  // leave 0.4 square before starting fiducial
static const MIL_DOUBLE FONT_SIZE_FACTOR             = 0.01; // determine the font size as a fraction of the image width, to ensure text will fit horizontally
static const MIL_DOUBLE TEXT_VERTICAL_OFFSET         = 0.01; // text has a vertical offset from the border proportional to the image width
static const MIL_DOUBLE BOTTOM_SPACE_FACTOR          = 0.03; // leave space at the bottom for the legend; space is proportional to the image width
static const MIL_DOUBLE BORDER_THICKNESS             = 0.05; // the border thickness is 5% of a square
static const MIL_DOUBLE TRIANGLE_OFFSET              = 0.15; // triangle indicators start 0.15 square from the grid, 0.10 square from the end of the border
static const MIL_DOUBLE TRIANGLE_LENGTH              = 0.70; // triangle indicator length is 0.7 square

static const MIL_CONST_TEXT_PTR SEPARATOR = MIL_TEXT("       "); // text spacing used in the legend

#endif // GRID_CONFIG_H
