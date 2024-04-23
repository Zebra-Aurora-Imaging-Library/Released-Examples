//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QApplication>
#endif
#include "mainframe.h"
#include "mdispqtapp.h"

#if STATIC_QT5
#include <QtPlugin>
#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#else
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin)
#endif
#endif
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

#if M_MIL_USE_LINUX
#include <X11/Xlib.h>
#undef Bool
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void MessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
   {
   QByteArray localMsg = msg.toLocal8Bit();
   switch (type)
      {
      case QtDebugMsg:
         fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
         break;
      case QtWarningMsg:
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)         
      case QtInfoMsg:
#endif
         break;
      case QtCriticalMsg:
         fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
         break;
      case QtFatalMsg:
         fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
         abort();
      }
   }
#endif
#endif

int main(int argc, char *argv[])
{
#if M_MIL_USE_LINUX
   XInitThreads();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)   
   qInstallMessageHandler(MessageOutput);
#endif
#else
   qputenv("QT_QPA_PLATFORM", "windows:nowmpointer");
#endif

   MdispQtApp app( argc, argv );
   return app.exec();

}

#if (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "qrc_mdispqt.cpp"
#endif
