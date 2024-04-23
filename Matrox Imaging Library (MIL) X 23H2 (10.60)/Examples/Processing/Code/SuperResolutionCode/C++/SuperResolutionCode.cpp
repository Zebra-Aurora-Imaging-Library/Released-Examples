﻿//**********************************************************************************/
//
// File name: SuperResolutionCode.cpp
//
// Synopsis:  This example aligns a sequence of images containing bar codes
//            and combines them with a super-resolution process to form
//            an enhanced image, which is then used to decode the bar codes.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#include <mil.h>

//***********************************************************************************
// Example description.
//***********************************************************************************
void PrintHeader()   
   {
   MosPrintf(MIL_TEXT("[EXAMPLE NAME]\n")
             MIL_TEXT("SuperResolutionCode\n\n")

             MIL_TEXT("[SYNOPSIS]\n")
             MIL_TEXT("This example aligns a sequence of images containing bar codes\n")
             MIL_TEXT("and combines them with a super-resolution process to form\n")
             MIL_TEXT("an enhanced image, which is then used to decode the bar codes.\n\n")

             MIL_TEXT("[MODULES USED]\n")
             MIL_TEXT("Modules used: application, system, display, buffer,\n")
             MIL_TEXT("graphics, sequence, registration, code reader.\n\n")

             MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();
   }

//***********************************************************************************
// Structures declarations.
//***********************************************************************************

// Hold the description of the sequence to be processed.
struct SSequenceDescription
   {
   MIL_CONST_TEXT_PTR Filename;
   MIL_INT            StartImage;
   MIL_INT            NumberOfImages;
   MIL_INT            ChildOffsetX;
   MIL_INT            ChildOffsetY;
   MIL_INT            ChildSizeX;
   MIL_INT            ChildSizeY;
   MIL_DOUBLE         PsfRadius;
   MIL_INT            PsfType;
   MIL_DOUBLE         Smoothness;
   };

// Processing object.
class CSuperResolution
   {
   public:
      CSuperResolution(MIL_ID MilSystem);
      ~CSuperResolution();

      void InitializeWithFirstImage(MIL_ID MilFirstImage, 
                                    MIL_INT NumberOfImages,
                                    MIL_INT ChildOffsetX, MIL_INT ChildOffsetY,
                                    MIL_INT ChildSizeX, MIL_INT ChildSizeY);
      bool AddImage(MIL_ID MilImage);
      void SuperResolution(MIL_DOUBLE PsfRadius, MIL_INT PsfType, MIL_DOUBLE Smoothness);

   protected:
      void InitializeDisplay(MIL_ID MilFirstImage, MIL_INT ChildSizeX, MIL_INT ChildSizeY);
      void DrawCurrentResult();

      MIL_ID m_MilSystem;

      // Registration context and result.
      MIL_ID m_MilRegContext;
      MIL_ID m_MilRegResult;

      // Images of the sequence.
      MIL_ID* m_MilSequenceImageTable;
      MIL_ID* m_MilPartialSequenceImageTable;
      MIL_INT m_NumberOfImagesInTable;
      MIL_INT m_NumberOfAllocatedTableElements;

      // Various images for display.
      MIL_ID m_MilDisplay;
      MIL_ID m_MilOverlayImage;
      MIL_ID m_MilFullDisplayImage;
      MIL_ID m_MilTrackingImage;
      MIL_ID m_MilZoomedWithSuperResolutionImage;
      MIL_ID m_MilTrackingOverlayImage;

      MIL_INT m_FirstImageChildOffsetX;
      MIL_INT m_FirstImageChildOffsetY;
   };

//***********************************************************************************
// Constants definitions.
//***********************************************************************************
// Note that the sequence is imaged with a pretty good focus, therefore
// the Point Spread Function (PSF) is simply the size of one CCD pixel
// (square of "radius" = 0.5 pixel).
// 
// All the images have low noise, the super-resolution smoothness can be
// reduced less than its default behavior of 50.

#define EXAMPLE_SEQUENCE_PATH   M_IMAGE_PATH             MIL_TEXT("SuperResolutionCode/")
#define SEQUENCE_FILE           EXAMPLE_SEQUENCE_PATH    MIL_TEXT("code_far.avi")

// The sequence data struct for super-resolution.
static const SSequenceDescription Sequences = {
   SEQUENCE_FILE, // Filename
   0,             // StartImage
   8,             // NumberOfImages, can be from 2 to 8.
   80,            // ChildOffsetX
   10,            // ChildOffsetY
   200,           // ChildSizeX
   110,           // ChildSizeY
   0.5,           // PsfRadius
   M_SQUARE,      // PsfType
   30.0           // Smoothness
};

// Mosaic scale factor for super-resolution.
static const MIL_DOUBLE MOSAIC_SCALE = 2.0;

// Utility function.
template <class T>
inline T Max(T InValue1, T InValue2)
   {
   if (InValue1 > InValue2)
      return InValue1;
   else 
      return InValue2;
   }

//***********************************************************************************
// Main.
//***********************************************************************************
int MosMain(void)
{
   // Allocate MIL objects. 
   MIL_ID MilApplication = MappAlloc(M_NULL, M_DEFAULT, M_NULL);
   MIL_ID MilSystem = MsysAlloc(M_DEFAULT, M_SYSTEM_HOST, M_DEFAULT, M_DEFAULT, &MilSystem);

   // Main application code here. 
   PrintHeader();

   {
   // Open sequence.
   MbufImportSequence(Sequences.Filename, M_DEFAULT, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_OPEN);

   MIL_INT NumberOfImagesInSequence = MbufDiskInquire(Sequences.Filename, M_NUMBER_OF_IMAGES, M_NULL);

   MIL_DOUBLE FrameRate;
   MbufDiskInquire(Sequences.Filename, M_FRAME_RATE, &FrameRate);

   // Restore first image.
   MIL_ID MilSequenceImage;
   MbufImportSequence(Sequences.Filename, M_DEFAULT, M_RESTORE, MilSystem, &MilSequenceImage, 
      Sequences.StartImage, 1, M_READ);
   MIL_INT FrameIndex = 1;

   // Allocate processing object.
   CSuperResolution SuperResolution(MilSystem);
   SuperResolution.InitializeWithFirstImage(MilSequenceImage, Sequences.NumberOfImages, 
      Sequences.ChildOffsetX, Sequences.ChildOffsetY, Sequences.ChildSizeX, Sequences.ChildSizeY);

   MosPrintf(MIL_TEXT("Super-resolution requires sub-pixel edge displacement between")
      MIL_TEXT(" the source\nimages. This displacement can be generated by slightly changing")
      MIL_TEXT(" the lens\nfocus between image acquisitions.\n\n")
      MIL_TEXT("A sequence of images was captured with different focus distances\n")
      MIL_TEXT("using a liquid lens in order to maintain a constant magnification factor.\n\n")
      MIL_TEXT("A rectangular region defines the ROI used to perform the image alignment.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   MosPrintf(MIL_TEXT("Aligning...\n"));

   MIL_DOUBLE PreviousTime;
   MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &PreviousTime);

   bool Success = true;
   while (Success && FrameIndex < Sequences.NumberOfImages)
      {
      // Read and process next image in sequence.
      MbufImportSequence(Sequences.Filename, M_DEFAULT, 
         M_LOAD, M_NULL, &MilSequenceImage, 
         M_DEFAULT, 1, M_READ);

      Success = SuperResolution.AddImage(MilSequenceImage);
      FrameIndex++;

      // Wait to have a proper frame rate.
      MIL_DOUBLE EndTime;
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &EndTime);
      MIL_DOUBLE WaitTime = (1.0/FrameRate) - (EndTime - PreviousTime);
      if (WaitTime > 0)
         MappTimer(M_DEFAULT, M_TIMER_WAIT, &WaitTime);
      MappTimer(M_DEFAULT, M_TIMER_READ+M_SYNCHRONOUS, &PreviousTime);
      }

   MosPrintf(MIL_TEXT("\n%d images have been aligned with the first one.\n\n"), Sequences.NumberOfImages-1);

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Perform image enhancement with super-resolution.
   SuperResolution.SuperResolution(Sequences.PsfRadius, Sequences.PsfType, Sequences.Smoothness);

   MosPrintf(MIL_TEXT("\nPress <Enter> to end.\n"));
   MosGetch();

   // Free image and close sequence.
   MbufFree(MilSequenceImage);
   MbufImportSequence(Sequences.Filename, M_DEFAULT, M_NULL, M_NULL, M_NULL, M_NULL, M_NULL, M_CLOSE);
   }

   // Free MIL objects.
   MsysFree(MilSystem);
   MappFree(MilApplication);

   return 0;
}

//***********************************************************************************
// Constructor of the processing object.
// Most initialization will be done later in InitializeWithFirstImage.
//***********************************************************************************
CSuperResolution::CSuperResolution(MIL_ID MilSystem) :
   m_MilSystem                         (MilSystem),
   m_MilRegContext                     (M_NULL),
   m_MilRegResult                      (M_NULL),

   m_MilSequenceImageTable             (NULL),
   m_MilPartialSequenceImageTable      (NULL),
   m_NumberOfImagesInTable             (0),
   m_NumberOfAllocatedTableElements    (0),

   m_MilDisplay                        (M_NULL),
   m_MilOverlayImage                   (M_NULL),
   m_MilFullDisplayImage               (M_NULL),
   m_MilTrackingImage                  (M_NULL),
   m_MilZoomedWithSuperResolutionImage (M_NULL),
   m_MilTrackingOverlayImage           (M_NULL),
   m_FirstImageChildOffsetX            (0),
   m_FirstImageChildOffsetY            (0)
   {
   }

//***********************************************************************************
// Destructor of the processing object.
// Free all objects.
//***********************************************************************************
CSuperResolution::~CSuperResolution()
   {
   // Free all allocated MIL objects.
   while (m_NumberOfImagesInTable)
      {
      m_NumberOfImagesInTable--;
      MbufFree(m_MilSequenceImageTable[m_NumberOfImagesInTable]);
      }
   delete [] m_MilPartialSequenceImageTable;
   delete [] m_MilSequenceImageTable;

   if (m_MilRegResult != M_NULL)
      MregFree(m_MilRegResult);
   if (m_MilRegContext != M_NULL)
      MregFree(m_MilRegContext);

   MbufFree(m_MilTrackingOverlayImage);
   MbufFree(m_MilZoomedWithSuperResolutionImage);
   MbufFree(m_MilTrackingImage);
   MbufFree(m_MilFullDisplayImage);
   MdispFree(m_MilDisplay);
   }

//***********************************************************************************
// Initialize the processing object.
//
// The processing object will be ready to align a series of images with the
// child specified when calling this function.
//***********************************************************************************
void CSuperResolution::InitializeWithFirstImage(MIL_ID MilFirstImage, 
                                                MIL_INT NumberOfImages,
                                                MIL_INT ChildOffsetX, MIL_INT ChildOffsetY,
                                                MIL_INT ChildSizeX, MIL_INT ChildSizeY)
   {
   // Allocate registration context and result.
   MregAlloc(m_MilSystem, M_STITCHING, M_DEFAULT, &m_MilRegContext);
   MregAllocResult(m_MilSystem, M_DEFAULT, &m_MilRegResult);

   MregControl(m_MilRegContext, M_CONTEXT, M_NUMBER_OF_REGISTRATION_ELEMENTS, NumberOfImages);

   m_FirstImageChildOffsetX = ChildOffsetX;
   m_FirstImageChildOffsetY = ChildOffsetY;

   // Allocate an array to store the images of the sequence.
   m_MilSequenceImageTable        = new MIL_ID[NumberOfImages];
   m_MilPartialSequenceImageTable = new MIL_ID[NumberOfImages];
   m_NumberOfAllocatedTableElements = NumberOfImages;
   m_NumberOfImagesInTable = 0;

   // Initialize the display.
   InitializeDisplay(MilFirstImage, ChildSizeX, ChildSizeY);
   MbufCopy(MilFirstImage, m_MilFullDisplayImage);

   MgraRectAngle(M_DEFAULT, m_MilOverlayImage, ChildOffsetX, ChildOffsetY, ChildSizeX-1,
      ChildSizeY-1, 0, M_CORNER_AND_DIMENSION);

   // Keep a copy of the specified child of the first image.
   // All the other images of the sequence will be aligned with this child.
   MbufAlloc2d(m_MilSystem, ChildSizeX, ChildSizeY, 8+M_UNSIGNED, M_IMAGE+M_PROC,
      &(m_MilSequenceImageTable[0]));
   m_NumberOfImagesInTable++;

   MbufCopyClip(MilFirstImage, m_MilSequenceImageTable[0], -ChildOffsetX, -ChildOffsetY);

   // Dummy calculate, just to initialize the result.
   MregCalculate(m_MilRegContext, m_MilSequenceImageTable, 
      m_MilRegResult, m_NumberOfImagesInTable, M_DEFAULT);

   // Specify the initial rough location of the next image
   // with respect to the child of the first image.
   MregSetLocation(m_MilRegContext, m_NumberOfImagesInTable, 0, M_POSITION_XY, 
      static_cast<MIL_DOUBLE>(-ChildOffsetX), static_cast<MIL_DOUBLE>(-ChildOffsetY), 
      M_DEFAULT, M_DEFAULT, M_DEFAULT);
   }

//***********************************************************************************
// Add one image to the sequence of images to align.
//
// The image will be copied in the internal array of images to be processed.
// Registration is used such that the child specified in InitializeWithFirstImage
// is aligned in the new image.
//***********************************************************************************
bool CSuperResolution::AddImage(MIL_ID MilImage)
   {
   if (m_NumberOfImagesInTable >= m_NumberOfAllocatedTableElements)
      return false;

   // Display the image.
   MbufCopy(MilImage, m_MilFullDisplayImage);

   // Set the mosaic offsets.
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_STATIC_INDEX, m_NumberOfImagesInTable-1);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_X, 0);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_Y, 0);

   // Keep a copy of the sequence image.
   m_MilSequenceImageTable[m_NumberOfImagesInTable] = MbufAlloc2d(
      MbufInquire(MilImage, M_OWNER_SYSTEM, M_NULL),
      MbufInquire(MilImage, M_SIZE_X      , M_NULL),
      MbufInquire(MilImage, M_SIZE_Y      , M_NULL),
      MbufInquire(MilImage, M_TYPE        , M_NULL),
      M_IMAGE+M_PROC,
      M_NULL);
   MbufCopy(MilImage, m_MilSequenceImageTable[m_NumberOfImagesInTable]);
   m_NumberOfImagesInTable++;

   // Create an array of images that contains M_NULL in all elements
   // except for image 0 and current image.
   m_MilPartialSequenceImageTable[0] = m_MilSequenceImageTable[0];
   for (MIL_INT i = 1; i < m_NumberOfImagesInTable-1; i++)
      m_MilPartialSequenceImageTable[i] = M_NULL;
   m_MilPartialSequenceImageTable[m_NumberOfImagesInTable-1] = m_MilSequenceImageTable[m_NumberOfImagesInTable-1];

   // Alignment: use translation only.
   MregControl(m_MilRegContext, M_CONTEXT, M_TRANSFORMATION_TYPE, M_TRANSLATION);

   MregCalculate(m_MilRegContext, m_MilPartialSequenceImageTable, 
      m_MilRegResult, m_NumberOfImagesInTable, M_DEFAULT);

   MIL_INT Status;
   MregGetResult(m_MilRegResult, M_GENERAL, M_RESULT+M_TYPE_MIL_INT, &Status);
   if (Status != M_SUCCESS)
      return false;

   // The alignment of this image is done. Do not re-do the alignment of this image
   // when we will receive other images in the sequence.
   // Simply copy the results of the alignment to the registration
   // context. Since this image will not be part of m_MilPartialSequenceImageTable
   // in future calls to MregCalculate, its alignment will not be recalculated.
   MregSetLocation(m_MilRegContext, M_ALL, M_UNCHANGED, M_COPY_REG_RESULT, m_MilRegResult,
      M_DEFAULT, M_DEFAULT, M_DEFAULT, M_DEFAULT);

   if (m_NumberOfImagesInTable < m_NumberOfAllocatedTableElements)
      {
      // The initial rough location of the next image of the sequence is copied
      // from the alignment result of the just aligned image.
      MregSetLocation(m_MilRegContext, 
         m_NumberOfImagesInTable, 0,                            // The next image will be aligned with image 0.
         M_COPY_REG_RESULT,                                     // using the initial location specified by the result.
         m_MilRegResult,                                        // of the alignment just performed.
         static_cast<MIL_DOUBLE>(m_NumberOfImagesInTable-1), 0, // of the just aligned image with image 0.
         M_DEFAULT, M_DEFAULT);
      }

   // Draw the result.
   DrawCurrentResult();

   return true;
   }

//*****************************************************************************
// Perform super-resolution on the sequence of images and display the results.
// Also show the result of averaging all the images, for comparison purpose.
//*****************************************************************************
// Maximum length of the string to read.
#define STRING_LENGTH_MAX              64L

// Width and height given to the text to write.
#define TEXT_WIDTH           140
#define TEXT_HEIGHT          20

// The Y offset to draw the code from the result y position of the code.
#define DRAW_CODE_Y_OFFSET   30

void CSuperResolution::SuperResolution(MIL_DOUBLE PsfRadius, MIL_INT PsfType, MIL_DOUBLE Smoothness)
   {
   // Clear the overlay.
   MbufClear(m_MilTrackingOverlayImage, (MIL_DOUBLE)(MdispInquire(m_MilDisplay, M_TRANSPARENT_COLOR, M_NULL)));

   // Setup the registration result to do super-resolution with all images.
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_STATIC_INDEX, 0);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_SCALE , MOSAIC_SCALE);

   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_X, 0);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_Y, 0);

   MregControl(m_MilRegResult, M_GENERAL, M_SR_PSF_RADIUS, PsfRadius);
   MregControl(m_MilRegResult, M_GENERAL, M_SR_PSF_TYPE  , PsfType);
   MregControl(m_MilRegResult, M_GENERAL, M_SR_SMOOTHNESS, Smoothness);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_COMPOSITION, M_SUPER_RESOLUTION);

   // Composes the mosaic into the super-resolution result image.
   MregTransformImage(m_MilRegResult, m_MilSequenceImageTable, m_MilZoomedWithSuperResolutionImage, 
      m_NumberOfImagesInTable, M_BILINEAR, M_DEFAULT);

   MgraText(M_DEFAULT, m_MilOverlayImage, MbufInquire(m_MilZoomedWithSuperResolutionImage, M_ANCESTOR_OFFSET_X, M_NULL)
      +MbufInquire(m_MilZoomedWithSuperResolutionImage, M_SIZE_X, M_NULL)-TEXT_WIDTH,
      MbufInquire(m_MilZoomedWithSuperResolutionImage, M_SIZE_Y, M_NULL), MIL_TEXT("Super-resolution"));

   MosPrintf(MIL_TEXT("The aligned images are combined in a higher resolution image using\n")
      MIL_TEXT("a super-resolution process.\n\n"));

   MosPrintf(MIL_TEXT("Press <Enter> to continue.\n\n"));
   MosGetch();

   // Allocate code objects.
   MIL_ID Barcode = McodeAlloc(m_MilSystem, M_DEFAULT, M_DEFAULT, M_NULL);
   MIL_ID BarModel = McodeModel(Barcode, M_ADD, M_CODE128, M_NULL, M_DEFAULT, M_NULL);
   MIL_ID CodeResults = McodeAllocResult(m_MilSystem, M_DEFAULT, M_NULL);
   
   // Read the code128 code in the super-resolution image.
   McodeRead(Barcode, m_MilZoomedWithSuperResolutionImage, CodeResults);

   // Get decoding status.
   MIL_INT  BarcodeStatus;
   McodeGetResult(CodeResults, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &BarcodeStatus);

   MIL_TEXT_CHAR BarcodeString[STRING_LENGTH_MAX];
   MIL_DOUBLE PositionX, PositionY;

   // Check if decoding was successful.
   if (BarcodeStatus == M_STATUS_READ_OK)
      {
      // Get decoded string and position.
      McodeGetResult(CodeResults, 0, M_GENERAL, M_STRING, BarcodeString);
      McodeGetResult(CodeResults, 0, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(CodeResults, 0, M_GENERAL, M_POSITION_Y, &PositionY);

      McodeDraw(M_DEFAULT, CodeResults, m_MilTrackingOverlayImage, M_DRAW_POSITION, M_ALL, M_GENERAL, M_DEFAULT);
      MgraText(M_DEFAULT, m_MilTrackingOverlayImage, PositionX, PositionY+DRAW_CODE_Y_OFFSET, BarcodeString);
      }

   MIL_INT NumOfModels = McodeInquire(Barcode, M_NUMBER_OF_CODE_MODELS, M_NULL);
   McodeModel(Barcode, M_DELETE, M_NULL, M_NULL, M_DEFAULT, &BarModel);
   NumOfModels = McodeInquire(Barcode, M_NUMBER_OF_CODE_MODELS, M_NULL);
   McodeModel(Barcode, M_ADD, M_4_STATE, M_NULL, M_DEFAULT, M_NULL);
   McodeControl(Barcode, M_ENCODING, M_ENC_AUSTRALIA_MAIL_RAW);
   // Read the 4-state code in the super-resolution image.
   McodeRead(Barcode, m_MilZoomedWithSuperResolutionImage, CodeResults);

   // Get decoding status.
   McodeGetResult(CodeResults, M_GENERAL, M_GENERAL, M_STATUS+M_TYPE_MIL_INT, &BarcodeStatus);

   // Check if decoding was successful.
   if (BarcodeStatus == M_STATUS_READ_OK)
      {
      // Get decoded string and position.
      McodeGetResult(CodeResults, 0, M_GENERAL, M_STRING, BarcodeString);
      McodeGetResult(CodeResults, 0, M_GENERAL, M_POSITION_X, &PositionX);
      McodeGetResult(CodeResults, 0, M_GENERAL, M_POSITION_Y, &PositionY);

      McodeDraw(M_DEFAULT, CodeResults, m_MilTrackingOverlayImage, M_DRAW_POSITION, M_ALL, M_GENERAL, M_DEFAULT);
      MgraText(M_DEFAULT, m_MilTrackingOverlayImage, PositionX, PositionY+DRAW_CODE_Y_OFFSET, BarcodeString);
      }

   MosPrintf(MIL_TEXT("The codes are read from the resulting super-resolution image.\n"));

   // Free code objects.
   McodeFree(CodeResults);
   McodeFree(Barcode);
   }

//*****************************************************************************
// Initialize a display that will show 
// - The sequence image.
// - A result of the tracking of the child of the first image in all 
//   the sequence image.
// - The result of super-resolution and of averaging of all images of the
//   sequence.
//*****************************************************************************
void CSuperResolution::InitializeDisplay(MIL_ID MilFirstImage, MIL_INT ChildSizeX, MIL_INT ChildSizeY)
   {
   // Allocate display.
   m_MilDisplay = MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, M_NULL);
   MdispControl(m_MilDisplay, M_OVERLAY, M_ENABLE);

   MIL_INT SequenceImageSizeX = MbufInquire(MilFirstImage, M_SIZE_X, M_NULL);
   MIL_INT SequenceImageSizeY = MbufInquire(MilFirstImage, M_SIZE_Y, M_NULL);

   // Find the size of the zoomed child.
   MIL_INT ZoomedChildSizeX = static_cast<MIL_INT>(ChildSizeX * MOSAIC_SCALE);
   MIL_INT ZoomedChildSizeY = static_cast<MIL_INT>(ChildSizeY * MOSAIC_SCALE);

   // Allocate full display image.
   MIL_INT FullDisplayImageSizeX = SequenceImageSizeX + Max(ZoomedChildSizeX, SequenceImageSizeX);
   MIL_INT FullDisplayImageSizeY = Max(SequenceImageSizeY, ZoomedChildSizeY);
   m_MilFullDisplayImage = MbufAlloc2d(m_MilSystem, FullDisplayImageSizeX, FullDisplayImageSizeY,
      8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, M_NULL);
   MbufClear(m_MilFullDisplayImage, M_COLOR_BLACK);

   MdispSelect(m_MilDisplay, m_MilFullDisplayImage);

   MdispInquire(m_MilDisplay, M_OVERLAY_ID, &m_MilOverlayImage);
   MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

   MgraColor(M_DEFAULT, M_COLOR_GREEN);
   MgraText(M_DEFAULT, m_MilOverlayImage, SequenceImageSizeX-TEXT_WIDTH, SequenceImageSizeY-TEXT_HEIGHT, MIL_TEXT("Source image"));

   // Allocate children in the full display image to show current image aligned
   // with first image and to show zoomed version of the sequence.
   m_MilTrackingImage = MbufChild2d(m_MilFullDisplayImage, SequenceImageSizeX, 0,
      SequenceImageSizeX, SequenceImageSizeY, M_NULL);
   m_MilTrackingOverlayImage = MbufChild2d(m_MilOverlayImage, SequenceImageSizeX, 0,
      SequenceImageSizeX, SequenceImageSizeY, M_NULL);

   m_MilZoomedWithSuperResolutionImage = MbufChild2d(m_MilFullDisplayImage, SequenceImageSizeX, 0,
      ZoomedChildSizeX, ZoomedChildSizeY, M_NULL);
   }

//*****************************************************************************
// Display the result of the alignment of the child of the first image of the
// sequence with the current image of the sequence.
//*****************************************************************************
void CSuperResolution::DrawCurrentResult()
   {
   // Draw the current image as aligned with image 0.
   MIL_INT i;
   for (i = 0; i < m_NumberOfImagesInTable-1; i++) // Create an array of images that contains.
      m_MilPartialSequenceImageTable[i] = M_NULL;  // M_NULL in all elements.
   // except for the current image.
   m_MilPartialSequenceImageTable[m_NumberOfImagesInTable-1] = m_MilSequenceImageTable[m_NumberOfImagesInTable-1];

   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_COMPOSITION, M_LAST_IMAGE);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_X, m_FirstImageChildOffsetX);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_OFFSET_Y, m_FirstImageChildOffsetY);
   MregControl(m_MilRegResult, M_GENERAL, M_MOSAIC_STATIC_INDEX, 0);
   MregTransformImage(m_MilRegResult, m_MilPartialSequenceImageTable, m_MilTrackingImage, m_NumberOfImagesInTable,
      M_BILINEAR+M_OVERSCAN_CLEAR, M_DEFAULT);
 
   if(m_NumberOfImagesInTable==2)
      {
      MregDraw(M_DEFAULT, m_MilRegResult, m_MilTrackingOverlayImage, M_DRAW_BOX, 0, M_DEFAULT);
      MgraText(M_DEFAULT, m_MilTrackingOverlayImage, MbufInquire(m_MilTrackingOverlayImage, M_SIZE_X, M_NULL)-TEXT_WIDTH,
         MbufInquire(m_MilTrackingOverlayImage, M_SIZE_Y, M_NULL)-TEXT_HEIGHT, MIL_TEXT("Realigned image"));
      }
   }
