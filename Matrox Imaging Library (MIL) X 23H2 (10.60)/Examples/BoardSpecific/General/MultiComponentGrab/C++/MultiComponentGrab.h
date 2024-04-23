/***************************************************************************************/
/*
* File name: MultiComponentGrab.h
*
* Copyright © Matrox Electronic Systems Ltd., 1992-2023.
* All Rights Reserved
*/
#include <mil.h>
#include <vector>
#include <map>
#include <iomanip>

#ifndef __MULTICOMPONENT_GRAB_H__
#define __MULTICOMPONENT_GRAB_H__

/* Per component data consisting of a MIL display and a MIL buffer. */
/* Used to display each image component in its own MIL display.     */
class ComponentData
   {
   public:
      ComponentData();
      ComponentData(MIL_ID MilBufferComponent, bool IsDisplayable, const MIL_STRING& CompName, const MIL_STRING& PfncName);

      void Free();

      /* Returns a string describing the component's properties. */
      MIL_STRING ToString() const;

      MIL_ID MilDisplay;
      MIL_ID MilImageDisp;
      MIL_INT SizeBand;
      MIL_INT SizeX;
      MIL_INT SizeY;
      MIL_INT Type;
      MIL_STRING ComponentName;
      MIL_STRING PixelFormatName;
   };

/* List of ComponentData, use to store per-component auxiliary data such as a MIL display. */
typedef std::map<MIL_STRING, ComponentData> ComponentDataList;
typedef ComponentDataList::iterator ComponentListIterator;

/* ComponentData default constructor */
inline ComponentData::ComponentData()
   {
   MilDisplay = M_NULL;
   MilImageDisp = M_NULL;
   }

/* ComponentData constructor. Allocates a MIL buffer and a MIL display. */
inline ComponentData::ComponentData(MIL_ID MilBufferComponent, bool IsDisplayable, const MIL_STRING& CompName, const MIL_STRING& PfncName)
   : MilDisplay(M_NULL), MilImageDisp(M_NULL)
   {
   MIL_ID MilSystem = M_NULL;
   MIL_INT SizeBit = 0;
   MIL_DOUBLE MaxValue = 0;
   static MIL_INT PositionX = -50, PositionY = -50;
   PositionX += 100;
   PositionY += 100;

   MbufInquire(MilBufferComponent, M_OWNER_SYSTEM, &MilSystem);
   MbufInquire(MilBufferComponent, M_SIZE_BAND, &SizeBand);
   MbufInquire(MilBufferComponent, M_SIZE_X, &SizeX);
   MbufInquire(MilBufferComponent, M_SIZE_Y, &SizeY);
   MbufInquire(MilBufferComponent, M_SIZE_BIT, &SizeBit);
   MbufInquire(MilBufferComponent, M_TYPE, &Type);
   MbufInquire(MilBufferComponent, M_MAX, &MaxValue);

   ComponentName = CompName;
   PixelFormatName = PfncName;

   if (IsDisplayable)
      {
      MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDisplay);
      MdispControl(MilDisplay, M_TITLE, ComponentName);
      MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_X, PositionX);
      MdispControl(MilDisplay, M_WINDOW_INITIAL_POSITION_Y, PositionY);

      if ((SizeBit > 8) && (SizeBand == 3))
         {
         MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, 16 + M_UNSIGNED,
                        M_IMAGE + M_DISP + M_PROC, &MilImageDisp);
         MdispControl(MilDisplay, M_VIEW_MODE, M_AUTO_SCALE);
         }
      else if (SizeBit > 8)
         {
         MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, 16 + M_UNSIGNED,
                        M_IMAGE + M_DISP + M_PROC, &MilImageDisp);

         MdispControl(MilDisplay, M_VIEW_MODE, M_BIT_SHIFT);
         if (MaxValue == 1023)
            MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, 10 - 8);
         else if (MaxValue == 4095)
            MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, 12 - 8);
         else if (MaxValue == 16383)
            MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, 14 - 8);
         else if (MaxValue == 65535)
            MdispControl(MilDisplay, M_VIEW_BIT_SHIFT, 16 - 8);
         }
      else
         {
         MbufAllocColor(MilSystem, SizeBand, SizeX, SizeY, 8 + M_UNSIGNED,
                        M_IMAGE + M_DISP + M_PROC, &MilImageDisp);
         }

      MbufClear(MilImageDisp, M_COLOR_BLACK);
      MdispSelect(MilDisplay, MilImageDisp);
      }
   }

/* Component data free routine. */
inline void ComponentData::Free()
   {
   if (MilDisplay != M_NULL)
      MdispFree(MilDisplay);
   if (MilImageDisp != M_NULL)
      MbufFree(MilImageDisp);
   }

/* Returns a string describing the component's properties. */
inline MIL_STRING ComponentData::ToString() const
   {
   MIL_STRING_STREAM String;

   String << std::setw(5) << SizeX << MIL_TEXT("x") << std::setw(4) << std::left << SizeY;

   String << std::setw(4);
   switch (Type)
      {
      case 8 + M_UNSIGNED:
         String << MIL_TEXT("8u");
         break;
      case 8 + M_SIGNED:
         String << MIL_TEXT("8");
         break;
      case 16 + M_UNSIGNED:
         String << MIL_TEXT("16u");
         break;
      case 16 + M_SIGNED:
         String << MIL_TEXT("16");
         break;
      case 32 + M_UNSIGNED:
         String << MIL_TEXT("32u");
         break;
      case 32 + M_SIGNED:
         String << MIL_TEXT("32");
         break;
      case 32 + M_FLOAT:
         String << MIL_TEXT("32f");
         break;
      case 64 + M_UNSIGNED:
         String << MIL_TEXT("64u");
         break;
      case 64 + M_SIGNED:
         String << MIL_TEXT("64");
         break;
      case 64 + M_FLOAT:
         String << MIL_TEXT("64f");
         break;
      }

   String << std::setw(2) << SizeBand <<  (SizeBand > 1 ? MIL_TEXT("bands") : MIL_TEXT("band"));
   return String.str();
   }

#endif
