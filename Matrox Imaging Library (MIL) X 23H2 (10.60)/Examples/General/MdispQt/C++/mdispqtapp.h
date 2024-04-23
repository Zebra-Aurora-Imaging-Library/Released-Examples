//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MDISPQTAPP_H
#define MDISPQTAPP_H
#include <mil.h>
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QApplication>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

class MdispQtView;
class MdispQtApp : public QApplication
{
   Q_OBJECT

public:
   MdispQtApp( int& argc, char** argv );
   virtual ~MdispQtApp();

   MIL_INT     m_digitizerSizeX;    // Digitizer input width
   MIL_INT     m_digitizerSizeY;    // Digitizer input heigh
   MIL_INT     m_digitizerNbBands;  // Number of input color bands of the digitizer
   bool     m_isGrabStarted;     // State of the grab
   MdispQtView* m_pGrabView;     // Pointer to the view that has the grab
   MIL_INT     m_numberOfDigitizer; // Number of digitizers available on the system

   MIL_ID   m_MilApplication;    // Application identifier.
   MIL_ID   m_MilSystem;         // System identifier.
   MIL_ID   m_MilDigitizer;      // Digitizer identifier.

   QString  m_ImagePath;         // MIL Image Path;

   long MFTYPE DisplayError( MIL_ID EventId );
   void HookedOnErrors(bool IsHooked){m_isCurrentlyHookedOnErrors = IsHooked;}

signals:
   void message(const QString& Str);
private slots:
   void OnErrorMessage(const QString& Str);

private:
   static MIL_INT MFTYPE DisplayErrorExt(MIL_INT HookType, MIL_ID EventId, void* UserDataPtr);

   bool m_isCurrentlyHookedOnErrors;
};

#endif // MDISPQTAPP_H
