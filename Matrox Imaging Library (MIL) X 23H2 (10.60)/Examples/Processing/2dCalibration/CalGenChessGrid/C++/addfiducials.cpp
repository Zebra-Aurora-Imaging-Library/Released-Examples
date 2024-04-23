//***************************************************************************************
// 
// File name: addfiducials.cpp
//
// Synopsis:  This file implements AddFiducials(), declared in common.h.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include "common.h"

//*****************************************************************************
// Structure containing information to encode in a fiducial.
//*****************************************************************************
struct FiducialInfoStruct
   {
   UnitEnum Unit;
   MIL_INT  SpacingExponent;     // spacing exponent in [-5, 4]
   MIL_INT  SpacingMantissaX;    // mantissa for the X spacing; can be 1 or 3 digits long
   MIL_INT  SpacingMantissaY;    // mantissa for the Y spacing; can be 1 or 3 digits long
   MIL_INT  FiducialPositionX;   // X position of the fiducial relative to the grid reference point, in terms of grid squares
   MIL_INT  FiducialPositionY;   // Y position of the fiducial relative to the grid reference point, in terms of grid squares

   static const MIL_INT MAX_ENCODED_STRING_LENGTH = 32;     // real maximum is 16 digits, +1 for \0
   MIL_TEXT_CHAR EncodedString[MAX_ENCODED_STRING_LENGTH];  // all the information above, encoded in a Datamatrix string

   // Uses all members to create EncodedString.
   void Encode();
   };

//*****************************************************************************
// Basic digit encoding function. Starts at CurPtr and writes exactly NumDigits
// characters, which are the digits of ValueToEncode (left-padded with zeros).
// CurPtr is moved by the function.
//*****************************************************************************
static void EncodeAndMovePointer(MIL_INT ValueToEncode, MIL_INT NumDigits, MIL_TEXT_PTR& CurPtr)
   {
   MIL_INT PowerOf10 = 1;
   for (MIL_INT i = 1; i < NumDigits; ++i)
      PowerOf10 *= 10;

   for (MIL_INT i = 0; i < NumDigits; ++i)
      {
      MIL_INT DigitToEncode = ValueToEncode / PowerOf10;
      if (!(0 <= DigitToEncode && DigitToEncode <= 9))
         throw MIL_TEXT("ValueToEncode is larger than the maximum number of digits");

      MIL_TEXT_CHAR Char = static_cast<MIL_TEXT_CHAR>(MIL_TEXT('0') + DigitToEncode);
      *CurPtr++ = Char;
      ValueToEncode %= PowerOf10;
      PowerOf10 /= 10;
      }
   }

//*****************************************************************************
// Takes a filled FiducialInfoStruct and generates its EncodedString member,
// according to the fiducial encoding supported by the MIL calibration module.
//*****************************************************************************
void FiducialInfoStruct::Encode()
   {
   const MIL_INT NEED_2_SPACINGS_BIT          = (1 << 3);
   const MIL_INT NEED_3_DIGITS_SPACINGS_BIT   = (1 << 4);
   const MIL_INT NEED_2_DIGITS_POSITIONS_BIT  = (1 << 5);
   const MIL_INT NEED_3_DIGITS_POSITIONS_BIT  = (1 << 6);

   const MIL_INT SPACING_EXPONENT_OFFSET = 5;

   if (!(-5 <= SpacingExponent && SpacingExponent <= 4))
      throw MIL_TEXT("Spacing exponent is out of the range [-5, 4]");
   if (!(0 <= SpacingMantissaX && SpacingMantissaX <= 999))
      throw MIL_TEXT("X spacing's mantissa cannot be represented on 3 digits");
   if (!(0 <= SpacingMantissaY && SpacingMantissaY <= 999))
      throw MIL_TEXT("Y spacing's mantissa cannot be represented on 3 digits");
   if (!(-500 <= FiducialPositionX && FiducialPositionX <= 499))
      throw MIL_TEXT("The fiducial X position is out of the range [-500, 499]");
   if (!(-500 <= FiducialPositionY && FiducialPositionY <= 499))
      throw MIL_TEXT("The fiducial Y position is out of the range [-500, 499]");

   // Determine the configuration field first.
   MIL_INT ConfigField = 0; // bit 0-2 must be set to 0.
   
   if (SpacingMantissaX != SpacingMantissaY)
      ConfigField |= NEED_2_SPACINGS_BIT; // bit set because we need 2 spacing fields

   if (SpacingMantissaX >= 10 || SpacingMantissaY >= 10)
      ConfigField |= NEED_3_DIGITS_SPACINGS_BIT; // bit set because we need 3 digits for spacings

   if (FiducialPositionX != 0 || FiducialPositionY != 0)
      {
      if ( -50 <= FiducialPositionX && FiducialPositionX <= 49 &&
           -50 <= FiducialPositionY && FiducialPositionY <= 49 )
         ConfigField |= NEED_2_DIGITS_POSITIONS_BIT; // bit set because we need 2 digits for positions
      else if ( -500 <= FiducialPositionX && FiducialPositionX <= 499 &&
                -500 <= FiducialPositionY && FiducialPositionY <= 499 )
         ConfigField |= NEED_3_DIGITS_POSITIONS_BIT; // bit set because we need 3 digits for positions
      }
   // else no bit set indicates there is no position field

   // The fiducial is valid. Ready to start encoding.
   MIL_TEXT_PTR CurPtr = EncodedString;
   EncodeAndMovePointer(ConfigField, 2, CurPtr);
   EncodeAndMovePointer(static_cast<MIL_INT>(Unit), 1, CurPtr);
   EncodeAndMovePointer(SpacingExponent + SPACING_EXPONENT_OFFSET, 1, CurPtr);

   MIL_INT NumSpacingDigits = ((ConfigField & NEED_3_DIGITS_SPACINGS_BIT) != 0 ? 3 : 1);
   EncodeAndMovePointer(SpacingMantissaX, NumSpacingDigits, CurPtr);
   if ((ConfigField & NEED_2_SPACINGS_BIT) != 0)
      EncodeAndMovePointer(SpacingMantissaY, NumSpacingDigits, CurPtr);

   if ((ConfigField & (NEED_2_DIGITS_POSITIONS_BIT | NEED_3_DIGITS_POSITIONS_BIT)) != 0)
      {
      MIL_INT NumPositionDigits = ((ConfigField & NEED_3_DIGITS_POSITIONS_BIT) != 0 ? 3 : 2);
      MIL_INT PositionOffset    = ((ConfigField & NEED_3_DIGITS_POSITIONS_BIT) != 0 ? 500 : 50);
      EncodeAndMovePointer(FiducialPositionX + PositionOffset, NumPositionDigits, CurPtr);
      EncodeAndMovePointer(FiducialPositionY + PositionOffset, NumPositionDigits, CurPtr);
      }

   // Terminate string.
   *CurPtr++ = MIL_TEXT('\0');
   if (CurPtr - EncodedString >= FiducialInfoStruct::MAX_ENCODED_STRING_LENGTH)
      throw MIL_TEXT("The fiducial encoded string is too long (buffer overrun)");
   }

//*****************************************************************************
// Class that uses the MIL code module to generate the datamatrix fiducials.
// Encapsulates all MIL objects, so that they are correctly freed, even in the
// presence of exceptions.
//*****************************************************************************
class CDatamatrixDrawer
   {
   public:
      CDatamatrixDrawer(MIL_ID SysId, MIL_ID GridImageId);
      ~CDatamatrixDrawer();

      void Draw(MIL_INT ChildStartX,
                MIL_INT ChildStartY,
                MIL_INT ChildSizeX,
                MIL_INT ChildSizeY, 
                MIL_INT FilenameIndex,
                const FiducialInfoStruct& FiducialInfo);

   private:
      MIL_ID m_SysId;         // system on which to allocate MIL objects
      MIL_ID m_CodeContextId; // code context used to draw fiducials
      MIL_ID m_CodeModelId;   // specific code model in the code context
      MIL_ID m_DestChildId;   // child buffer on top of the grid image passed to the constructor
      MIL_ID m_CodeImageId;   // temporary image buffer used as destination of McodeWrite()
   };

//*****************************************************************************
// Constructor.
//*****************************************************************************
CDatamatrixDrawer::CDatamatrixDrawer(MIL_ID SysId, MIL_ID GridImageId)
   : m_SysId        (SysId),
     m_CodeContextId(M_NULL),
     m_CodeModelId  (M_NULL),
     m_CodeImageId  (M_NULL),
     m_DestChildId  (M_NULL)
   {
   McodeAlloc(m_SysId, M_DEFAULT, M_DEFAULT, &m_CodeContextId);
   McodeModel(m_CodeContextId, M_ADD, M_DATAMATRIX, M_NULL, M_DEFAULT, &m_CodeModelId);
   McodeControl(m_CodeModelId, M_ERROR_CORRECTION, M_ECC_200);

   MbufChild2d(GridImageId, 0, 0, 1, 1, &m_DestChildId);
   }

//*****************************************************************************
// Destructor. Ensures all MIL objects are correctly freed, even in the
// presence of exceptions.
//*****************************************************************************
CDatamatrixDrawer::~CDatamatrixDrawer()
   {
   if (m_DestChildId   != M_NULL) MbufFree (m_DestChildId  );
   if (m_CodeImageId   != M_NULL) MbufFree (m_CodeImageId  );
   if (m_CodeContextId != M_NULL) McodeFree(m_CodeContextId);
   }

//*****************************************************************************
// Uses the MIL code module to generate the datamatrix fiducial that encodes
// the string in FiducialInfo, rescales it and copies it in the destination.
//*****************************************************************************
void CDatamatrixDrawer::Draw(MIL_INT ChildStartX,
                             MIL_INT ChildStartY,
                             MIL_INT ChildSizeX,
                             MIL_INT ChildSizeY, 
                             MIL_INT FilenameIndex,
                             const FiducialInfoStruct& FiducialInfo)
   {
   // Determine the image size needed for McodeWrite().
   MIL_ID WriteResultId = McodeAllocResult(m_SysId, M_CODE_WRITE_RESULT, M_NULL);
   McodeWrite(m_CodeModelId, M_NULL, FiducialInfo.EncodedString, M_DEFAULT, WriteResultId);

   MIL_INT CodeSizeX, CodeSizeY;

   McodeGetResult(WriteResultId, M_WRITE_SIZE_X + M_TYPE_MIL_INT, &CodeSizeX);
   McodeGetResult(WriteResultId, M_WRITE_SIZE_Y + M_TYPE_MIL_INT, &CodeSizeY);

   if (CodeSizeX != CodeSizeY) // MimResize(M_FILL_DESTINATION) should not be used
      throw MIL_TEXT("This example expects a square datamatrix");

   // Allocate a temporary image and draw the fiducial.
   // Free the image from a previous call, if necessary.
   if (m_CodeImageId != M_NULL)
      MbufFree(m_CodeImageId);
   MbufAlloc2d(m_SysId, CodeSizeX, CodeSizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC, &m_CodeImageId);
   McodeWrite(m_CodeModelId, m_CodeImageId, FiducialInfo.EncodedString, M_DEFAULT, WriteResultId);

   // Free code write result buffer
   McodeFree(WriteResultId);

   // Save the fiducial image.
   const MIL_INT MAX_FILENAME_LEN = 256;
   MIL_TEXT_CHAR Filename[MAX_FILENAME_LEN];
   MosSprintf(Filename, MAX_FILENAME_LEN, OUTPUT_CODE_NAME, (int)FilenameIndex);
   MbufExport(Filename, OUTPUT_FILE_FORMAT, m_CodeImageId);

   MosPrintf(MIL_TEXT("  At position (%d, %d): saved as '%s'\n"),
             (int)FiducialInfo.FiducialPositionX, (int)FiducialInfo.FiducialPositionY, Filename);

   // Change the fiducial colors.
   MIL_DOUBLE CodeForegroundColor =   0.0;
   MIL_DOUBLE CodeBackgroundColor = 255.0;

   if (FOREGROUND_COLOR == CodeBackgroundColor)
      {
      MimArith(m_CodeImageId, M_NULL, m_CodeImageId, M_NOT);
      CodeForegroundColor = 255.0;
      CodeBackgroundColor =   0.0;
      }

   if (FOREGROUND_COLOR != CodeForegroundColor)
      MimClip(m_CodeImageId, m_CodeImageId, M_EQUAL, CodeForegroundColor, M_NULL, FOREGROUND_COLOR, M_NULL);
   if (BACKGROUND_COLOR != CodeBackgroundColor)
      MimClip(m_CodeImageId, m_CodeImageId, M_EQUAL, CodeBackgroundColor, M_NULL, BACKGROUND_COLOR, M_NULL);

   // Set the child in the grid image according to the given parameters.
   MbufChildMove(m_DestChildId, ChildStartX, ChildStartY, ChildSizeX, ChildSizeY, M_DEFAULT);

   // Scale the fiducial and write it in the destination image.
   MimResize(m_CodeImageId, m_DestChildId, M_FILL_DESTINATION, M_FILL_DESTINATION, M_NEAREST_NEIGHBOR + M_OVERSCAN_FAST);
   }

//*****************************************************************************
// Loops through all the fiducials, as specified in gridconfig.h, encodes the
// grid information, generates datamatrix fiducials and adds them to the
// destination grid image.
//*****************************************************************************
void AddFiducials(MIL_ID GridImageId,
                  const GridInfoStruct& GridInfo,
                  MIL_DOUBLE PixelsPerSquareX,
                  MIL_DOUBLE PixelsPerSquareY)
   {
#if NUM_FIDUCIALS > 0
   // Minimum number of squares around the fiducial.
   const MIL_INT FIDUCIAL_SPACING = 1;

   // Compute min/max logical positions (inclusive) for validation.
   MIL_INT MinPosX = NUM_SQUARES_FOR_QUIET_ZONE;
   MIL_INT MinPosY = NUM_SQUARES_FOR_QUIET_ZONE;
   MIL_INT MaxPosX = GridInfo.NumSquaresX - NUM_SQUARES_FOR_QUIET_ZONE; 
   MIL_INT MaxPosY = GridInfo.NumSquaresY - NUM_SQUARES_FOR_QUIET_ZONE; 

   MIL_INT RefPointPosX = GridInfo.GetReferencePositionX();
   MIL_INT RefPointPosY = GridInfo.GetReferencePositionY();

   if ( !(MinPosX <= RefPointPosX && RefPointPosX <= MaxPosX) ||
        !(MinPosY <= RefPointPosY && RefPointPosY <= MaxPosY) )
      throw MIL_TEXT("The grid reference point falls outside the grid");

   // Copy relevant grid information in the fiducial.
   FiducialInfoStruct FiducialInfo;
   FiducialInfo.Unit              = UNIT;
   FiducialInfo.SpacingExponent   = GridInfo.SpacingExponent;
   FiducialInfo.SpacingMantissaX  = GridInfo.SpacingMantissaX;
   FiducialInfo.SpacingMantissaY  = GridInfo.SpacingMantissaY;

   // Create MIL code objects to draw the fiducials.
   MIL_ID SysId;
   MbufInquire(GridImageId, M_OWNER_SYSTEM, &SysId);
   CDatamatrixDrawer DatamatrixDrawer(SysId, GridImageId);

   MosPrintf(MIL_TEXT("Fiducials:\n"));
   MosPrintf(MIL_TEXT("----------\n"));

   for (MIL_INT i = 0; i < NUM_FIDUCIALS; ++i)
      {
      // Compute fiducial bounding box, in terms of squares.
      if (!(2 <= FIDUCIAL_SIZE[i] && FIDUCIAL_SIZE[i] <= 3))
         throw MIL_TEXT("Unsupported fiducial size (must be 2 or 3)");

      MIL_INT FiducialMinX = RefPointPosX + FIDUCIAL_POS_X[i];
      MIL_INT FiducialMinY = RefPointPosY + FIDUCIAL_POS_Y[i];
      MIL_INT FiducialMaxX = FiducialMinX + FIDUCIAL_SIZE[i];
      MIL_INT FiducialMaxY = FiducialMinY + FIDUCIAL_SIZE[i];

      // Check that there is enough space around the fiducial.
      MIL_INT SafeFiducialMinX = FiducialMinX - FIDUCIAL_SPACING;
      MIL_INT SafeFiducialMinY = FiducialMinY - FIDUCIAL_SPACING;
      MIL_INT SafeFiducialMaxX = FiducialMaxX + FIDUCIAL_SPACING;
      MIL_INT SafeFiducialMaxY = FiducialMaxY + FIDUCIAL_SPACING;

      if ( !(MinPosX <= FiducialMinX && FiducialMaxX <= MaxPosX) ||
           !(MinPosY <= FiducialMinY && FiducialMaxY <= MaxPosY) )
         throw MIL_TEXT("The fiducial falls outside the grid");

      if ( !(MinPosX <= SafeFiducialMinX && SafeFiducialMaxX <= MaxPosX) ||
           !(MinPosY <= SafeFiducialMinY && SafeFiducialMaxY <= MaxPosY) )
         throw MIL_TEXT("The fiducial is too close to the grid border");

      for (MIL_INT j = 0; j < NUM_FIDUCIALS; ++j)
         {
         if (i == j)
            continue;

         MIL_INT OtherFiducialMinX = RefPointPosX + FIDUCIAL_POS_X[j];
         MIL_INT OtherFiducialMinY = RefPointPosY + FIDUCIAL_POS_Y[j];
         MIL_INT OtherFiducialMaxX = OtherFiducialMinX + FIDUCIAL_SIZE[j];
         MIL_INT OtherFiducialMaxY = OtherFiducialMinY + FIDUCIAL_SIZE[j];

         if ( !( OtherFiducialMaxX <= SafeFiducialMinX || SafeFiducialMaxX <= OtherFiducialMinX ||
                 OtherFiducialMaxY <= SafeFiducialMinY || SafeFiducialMaxY <= OtherFiducialMinY ) )
            throw MIL_TEXT("Fiducials are overlapping");
         }

      // Encode the fiducial information.
      FiducialInfo.FiducialPositionX = FIDUCIAL_POS_X[i];
      FiducialInfo.FiducialPositionY = FIDUCIAL_POS_Y[i];
      FiducialInfo.Encode();
   
      // Compute the pixel bounding box in the grid image.
      MIL_INT ChildStartX = static_cast<MIL_INT>((FiducialMinX + FIDUCIAL_INDENT) * PixelsPerSquareX + 0.5);
      MIL_INT ChildStartY = static_cast<MIL_INT>((FiducialMinY + FIDUCIAL_INDENT) * PixelsPerSquareY + 0.5);
      MIL_INT ChildEndX   = static_cast<MIL_INT>((FiducialMaxX - FIDUCIAL_INDENT) * PixelsPerSquareX + 0.5);
      MIL_INT ChildEndY   = static_cast<MIL_INT>((FiducialMaxY - FIDUCIAL_INDENT) * PixelsPerSquareY + 0.5);
      MIL_INT ChildSizeX  = ChildEndX - ChildStartX + 1;
      MIL_INT ChildSizeY  = ChildEndY - ChildStartY + 1;

      // Create a child buffer around the bounding box and fill it with the datamatrix.
      DatamatrixDrawer.Draw(ChildStartX, ChildStartY, ChildSizeX, ChildSizeY, i, FiducialInfo);
      }

   MosPrintf(MIL_TEXT("\n"));
#endif
   }
