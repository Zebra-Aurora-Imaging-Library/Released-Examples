/***************************************************************************************/
/*
* File name: AutomaticBlobSelection.h
*
* Synopsis:  This header contains the methods used to automatically select possible blobs
*            used for fixturing and interactively ask the users to select the desired one.
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/


struct SSelectBlobData
   {
   MIL_ID MilDisplay;
   MIL_ID MilOverlay;
   MIL_ID MilBlobResult;
   MIL_ID MilSelectedBlobEvent;
   MIL_INT* pPossibleBlobColors;
   MIL_INT SelectedBlobLabel;
   };

///***************************************************************************
// Function prototypes.
///***************************************************************************
typedef void (*BinarizeFuncPtr)(MIL_ID MilImage, MIL_ID MilSearchImage);
typedef void(*DeleteImpossibleBlobsFuncPtr)(MIL_ID MilBlobResult);

bool ChoosePossibleFixturingBlob(MIL_ID MilImage,
                                 MIL_ID MilSearchImage,
                                 MIL_ID MilDisplay,
                                 MIL_ID MilBlobResult,
                                 MIL_ID MilBlobContext,
                                 BinarizeFuncPtr BinarizeFunc,
                                 DeleteImpossibleBlobsFuncPtr DeleteImpossibleBlobsFunc,
                                 MIL_DOUBLE* BlobWidth,
                                 MIL_DOUBLE* BlobHeight);
MIL_INT MFTYPE HoverBlob(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
MIL_INT MFTYPE SelectBlob(MIL_INT HookType, MIL_ID MilEvent, void *pUserData);
MIL_DOUBLE HueToRGB(MIL_DOUBLE Temp1, MIL_DOUBLE Temp2, MIL_DOUBLE Hue);
void HSLToRGB(MIL_DOUBLE H, MIL_DOUBLE S, MIL_DOUBLE L, MIL_DOUBLE* R, MIL_DOUBLE* G, MIL_DOUBLE* B);



///***************************************************************************
// ChoosePossibleFixturingBlob.  Chooses the fixturing blob interactively.
///***************************************************************************
bool ChoosePossibleFixturingBlob(MIL_ID MilImage,
                                 MIL_ID MilSearchImage,
                                 MIL_ID MilDisplay,
                                 MIL_ID MilBlobResult,
                                 MIL_ID MilBlobContext,
                                 MIL_DOUBLE MinDimFactor,
                                 MIL_DOUBLE MaxDimFactor,
                                 MIL_DOUBLE MinFeretRatio,
                                 BinarizeFuncPtr BinarizeFunc,
                                 DeleteImpossibleBlobsFuncPtr DeleteImpossibleBlobsFunc,
                                 MIL_DOUBLE* pBlobWidth,
                                 MIL_DOUBLE* pBlobHeight)
   {
   bool Status = false;

   // Get the system.
   MIL_ID MilSystem = MbufInquire(MilImage, M_OWNER_SYSTEM, M_NULL);

   // Binarize the image.
   (*BinarizeFunc)(MilImage, MilSearchImage);

   // Calculate the blobs.
   MblobCalculate(MilBlobContext, MilSearchImage, M_NULL, MilBlobResult);

   // Delete blobs that do not meet the minimal criterion.
   (*DeleteImpossibleBlobsFunc)(MilBlobResult);

   MIL_INT NbBlobs;
   MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NbBlobs);

   if (NbBlobs)
      {
      // Get the min area box and blob labels.
      MIL_DOUBLE* pMinAreaWidth= new MIL_DOUBLE[NbBlobs];
      MIL_DOUBLE* pMinAreaHeight = new MIL_DOUBLE[NbBlobs];
      MIL_INT* pBlobLabels = new MIL_INT[NbBlobs];
      MIL_INT* pPossibleBlobLabels = new MIL_INT[NbBlobs];

      MblobGetResult(MilBlobResult, M_DEFAULT, M_MIN_AREA_BOX_WIDTH, pMinAreaWidth);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_MIN_AREA_BOX_HEIGHT, pMinAreaHeight);
      MblobGetResult(MilBlobResult, M_DEFAULT, M_LABEL_VALUE + M_TYPE_MIL_INT, pBlobLabels);

      // Get the possible blobs.
      MIL_INT NbPossibleBlobs = 0;
      MIL_INT MaxLabel = 0;
      for (MIL_INT BlobIdx = 0; BlobIdx < NbBlobs; BlobIdx++)
         {
         if (pMinAreaWidth[BlobIdx] / pMinAreaHeight[BlobIdx] > MinFeretRatio)
            {
            // Select the blobs that are similar to the current blob.
            MblobSelect(MilBlobResult, M_INCLUDE_ONLY, M_MIN_AREA_BOX_WIDTH, M_IN_RANGE, pMinAreaWidth[BlobIdx] * MinDimFactor, pMinAreaWidth[BlobIdx] * MaxDimFactor);
            MblobSelect(MilBlobResult, M_EXCLUDE, M_MIN_AREA_BOX_HEIGHT, M_OUT_RANGE, pMinAreaHeight[BlobIdx] * MinDimFactor, pMinAreaHeight[BlobIdx] * MaxDimFactor);

            MIL_INT NumbBlobs;
            MblobGetResult(MilBlobResult, M_DEFAULT, M_NUMBER + M_TYPE_MIL_INT, &NumbBlobs);
            if(NumbBlobs == 1)
               {
               pPossibleBlobLabels[NbPossibleBlobs++] = pBlobLabels[BlobIdx];
               if (pBlobLabels[BlobIdx] > MaxLabel)
                  MaxLabel = pBlobLabels[BlobIdx];
               }
            }
         }

      // If there are a few possible blobs.
      if (NbPossibleBlobs)
         {
         // Allocate a table that lists the color associated to each blob label.
         MIL_INT* pPossibleBlobColors = new MIL_INT[MaxLabel];

         // Get the overlay of the display.
         MIL_ID MilOverlay = MdispInquire(MilDisplay, M_OVERLAY_ID, M_NULL);
         MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

         // Exclude all the blobs.
         MblobSelect(MilBlobResult, M_EXCLUDE, M_ALL_BLOBS, M_NULL, M_NULL, M_NULL);

         // Draw the blobs in the overlay.
         MbufClear(MilSearchImage, 0);
         MIL_INT NbHue = NbPossibleBlobs > 32 ? 32 : NbPossibleBlobs;
         for (MIL_INT BlobIdx = 0; BlobIdx < NbPossibleBlobs; BlobIdx++)
            {
            MIL_INT CurLabel = pPossibleBlobLabels[BlobIdx];

            MblobSelect(MilBlobResult, M_INCLUDE, M_LABEL_VALUE, M_EQUAL, (MIL_DOUBLE)CurLabel, M_NULL);
            MIL_DOUBLE H = (MIL_DOUBLE)(BlobIdx % 32) / NbHue;
            MIL_DOUBLE R;
            MIL_DOUBLE G;
            MIL_DOUBLE B;
            HSLToRGB(H, 1.0, 0.5, &R, &G, &B);
            pPossibleBlobColors[CurLabel - 1] = M_RGB888(R * 255, G * 255, B * 255);
            MgraColor(M_DEFAULT, (MIL_DOUBLE)pPossibleBlobColors[CurLabel - 1]);
            MblobDraw(M_DEFAULT, MilBlobResult, MilOverlay, M_DRAW_BLOBS, CurLabel, M_DEFAULT);
            }
         // Delete all excluded blobs.
         MblobSelect(MilBlobResult, M_DELETE, M_EXCLUDED_BLOBS, M_NULL, M_NULL, M_NULL);

         // Print message.
         MosPrintf(MIL_TEXT("Move the mouse over the image and click to select one of the\n")
                   MIL_TEXT("identified unique blobs.\n\n"));

         // Hook the selection functions to the display.
         SSelectBlobData SelectBlobData;
         SelectBlobData.MilDisplay = MilDisplay;
         SelectBlobData.MilOverlay = MilOverlay;
         SelectBlobData.MilBlobResult = MilBlobResult;
         SelectBlobData.pPossibleBlobColors = pPossibleBlobColors;
         SelectBlobData.SelectedBlobLabel = 0;
         MthrAlloc(MilSystem, M_EVENT, M_NOT_SIGNALED + M_AUTO_RESET, M_NULL, M_NULL, &SelectBlobData.MilSelectedBlobEvent);
         MdispHookFunction(MilDisplay, M_MOUSE_MOVE, HoverBlob, &SelectBlobData);
         MdispHookFunction(MilDisplay, M_MOUSE_LEFT_BUTTON_UP, SelectBlob, &SelectBlobData);

         // Wait for a blob to be selected.
         MthrWait(SelectBlobData.MilSelectedBlobEvent, M_EVENT_WAIT, M_NULL);

         // Get the dimensions of the selected blob.
         MblobGetResult(MilBlobResult, SelectBlobData.SelectedBlobLabel, M_MIN_AREA_BOX_WIDTH, pBlobWidth);
         MblobGetResult(MilBlobResult, SelectBlobData.SelectedBlobLabel, M_MIN_AREA_BOX_HEIGHT, pBlobHeight);

         // Print message.
         MosPrintf(MIL_TEXT("A reference blob has been selected.\n\n")
                   MIL_TEXT("Press <Enter> to continue.\n\n"));
         MosGetch();

         // Free the event.
         MthrFree(SelectBlobData.MilSelectedBlobEvent);

         // Clear the overlay.
         MdispControl(MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);

         delete[] pPossibleBlobColors;

         Status = true;
         }

      delete[] pPossibleBlobLabels;
      delete[] pBlobLabels;
      delete[] pMinAreaWidth;
      delete[] pMinAreaHeight;
      }

   if (Status == false)
      MosPrintf(MIL_TEXT("The binary image doesn't contain any distinctive blobs.\n\n"));

   return Status;
   }

///***************************************************************************
// DeselectBlob.  Hook function that deselects the blobs.
///***************************************************************************
void DeselectBlob(SSelectBlobData* pSelectBlobData)
   {
   if (pSelectBlobData->SelectedBlobLabel != 0)
      {
      MgraColor(M_DEFAULT, (MIL_DOUBLE)pSelectBlobData->pPossibleBlobColors[pSelectBlobData->SelectedBlobLabel - 1]);
      MblobDraw(M_DEFAULT, pSelectBlobData->MilBlobResult, pSelectBlobData->MilOverlay, M_DRAW_BLOBS, pSelectBlobData->SelectedBlobLabel, M_DEFAULT);
      pSelectBlobData->SelectedBlobLabel = 0;
      }
   }

///***************************************************************************
// HoverBlob.  Hook function that selects the blob when the cursor moves over
//             it and deselect the blob once it is no longer over the
//             current selected blob.
///***************************************************************************
MIL_INT MFTYPE HoverBlob(MIL_INT HookType, MIL_ID MilEvent, void *pUserData)
   {
   SSelectBlobData* pSelectBlobData = (SSelectBlobData*)pUserData;

   MIL_DOUBLE MousePosX;
   MIL_DOUBLE MousePosY;
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_X, &MousePosX);
   MdispGetHookInfo(MilEvent, M_MOUSE_POSITION_BUFFER_Y, &MousePosY);

   // Get the label of the selected blob.
   MIL_INT BlobLabel = MblobGetLabel(pSelectBlobData->MilBlobResult, (MIL_INT)(MousePosX + 0.5), (MIL_INT)(MousePosY + 0.5), M_NULL);

   // If no blob was found.
   if (BlobLabel == M_NULL)
      {
      // Deselect the blob if one was selected.
      DeselectBlob(pSelectBlobData);
      }

   // If a new blob is selected.
   if (BlobLabel != pSelectBlobData->SelectedBlobLabel)
      {
      // Deselect the blob if one was selected.
      DeselectBlob(pSelectBlobData);

      // Select the new blob.
      pSelectBlobData->SelectedBlobLabel = BlobLabel;
      MgraColor(M_DEFAULT, M_COLOR_DARK_GREEN);
      MblobDraw(M_DEFAULT, pSelectBlobData->MilBlobResult, pSelectBlobData->MilOverlay, M_DRAW_BLOBS, pSelectBlobData->SelectedBlobLabel, M_DEFAULT);
      }

   return 0;
   }

///***************************************************************************
// SelectBlob.  Hook function that finally selects the blob when the mouse 
//              left click happens.
///***************************************************************************
MIL_INT MFTYPE SelectBlob(MIL_INT HookType, MIL_ID MilEvent, void *pUserData)
   {
   SSelectBlobData* pSelectBlobData = (SSelectBlobData*)pUserData;

   if (pSelectBlobData->SelectedBlobLabel != 0)
      {
      // Unhook the functions.
      MdispHookFunction(pSelectBlobData->MilDisplay, M_MOUSE_MOVE + M_UNHOOK, HoverBlob, pSelectBlobData);
      MdispHookFunction(pSelectBlobData->MilDisplay, M_MOUSE_LEFT_BUTTON_UP + M_UNHOOK, SelectBlob, pSelectBlobData);

      // Signal the event.
      MthrControl(pSelectBlobData->MilSelectedBlobEvent, M_EVENT_SET, M_SIGNALED);
      }

   return 0;
   }

///***************************************************************************
// HueToRGB.  Converts a Hue value to RGB value.
///***************************************************************************
MIL_DOUBLE HueToRGB(MIL_DOUBLE Temp1, MIL_DOUBLE Temp2, MIL_DOUBLE Hue)
   {
   if (Hue > 360.0)
      {
      Hue = Hue - 360.0;
      }
   else if (Hue < 0.0)
      {
      Hue = Hue + 360.0;
      }

   if (Hue < 60.0)
      {
      return (Temp1 + (Temp2 - Temp1) * Hue / 60.0);
      }
   else if (Hue < 180.0)
      {
      return Temp2;
      }
   else if (Hue < 240.0)
      {
      return (Temp1 + (Temp2 - Temp1) * (240.0 - Hue) / 60.0);
      }

   return Temp1;
   }

///***************************************************************************
// HSLToRGB.  Converts an HSL color to an RGB color. Input and output between 
//            0 and 1.
///***************************************************************************

void HSLToRGB(MIL_DOUBLE H, MIL_DOUBLE S, MIL_DOUBLE L, MIL_DOUBLE* R, MIL_DOUBLE* G, MIL_DOUBLE* B)
   {
   MIL_DOUBLE Temp1, Temp2;

   // Remap the angle between 0 and 360.
   H = H * 360.0;

   // Achromatic case.
   if (S == 0.0)
      {
      *R = *G = *B = L;
      }
   else
      {
      if (L <= 0.5)
         {
         Temp2 = L * (1.0 + S);
         }
      else
         {
         Temp2 = L + S - (L * S);
         }

      Temp1 = 2.0 * L - Temp2;

      *R = HueToRGB(Temp1, Temp2, H + 120.0);
      *G = HueToRGB(Temp1, Temp2, H);
      *B = HueToRGB(Temp1, Temp2, H - 120.0);
      }
   }
