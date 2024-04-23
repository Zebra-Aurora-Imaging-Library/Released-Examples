/*****************************************************************
 *
 * File name: MDispWindowQt.cpp
 *
 * Synopsis:  This program displays a welcoming message in a user-defined
 *            window and grabs (if supported) in it. It uses
 *            the MIL system and the MdispSelectWindow() function
 *            to display the MIL buffer in a user-created client window.
 *
 *            This version uses the Qt library to create the client window.
 *
 * Copyright © Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 *
 ****************************************************************/


#include "MdispWindowQt.h"
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QApplication>
#include <QAction>
#include <QMainWindow>
#include <QMessageBox>
#include <QToolBar>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

#include <mil.h>


#if STATIC_QT5
#include <QtPlugin>
#if !M_MIL_USE_LINUX
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#else
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif
#endif

#if M_MIL_USE_LINUX
#include <X11/Xlib.h>
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


/* Window title. */
#define MIL_APPLICATION_NAME      "MIL Application"

/* Default image dimensions. */
#define DEFAULT_IMAGE_SIZE_X       640
#define DEFAULT_IMAGE_SIZE_Y       480
#define DEFAULT_IMAGE_SIZE_BAND      1


/*****************************************************************
 *
 * Name:         MilApplication()
 *
 * synopsis:     This function is the core of the MIL application that
 *               is executed when this program is started. See main()
 *               below for the program entry point.
 *
 *               It uses MIL to display a welcoming message in the
 *               specified user window and to grab in it if it is
 *               supported by the target system.
 *
 ****************************************************************/
void MilApplication(PaintArea *Area)
   {
   /* MIL variables */
   MIL_ID MilApplication,  /* MIL Application identifier.  */
      MilDisplay,
      MilSystem,       /* MIL System identifier.       */
      MilDigitizer,    /* MIL Digitizer identifier.    */
      MilImage;        /* MIL Image buffer identifier. */

   MIL_INT BufSizeX    = 0;
   MIL_INT BufSizeY    = 0;
   MIL_INT BufSizeBand = 0;

   /* Allocate a MIL application. */
   MappAlloc(M_NULL, M_DEFAULT, &MilApplication);

   /* Allocate a MIL system. */
   MsysAlloc(M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, M_DEFAULT, &MilSystem);

   /* Allocate a MIL display. */
   MdispAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_WINDOWED, &MilDisplay);

   /* Allocate a MIL digitizer if supported and sets the target image size. */
   if (MsysInquire(MilSystem, M_DIGITIZER_NUM, M_NULL) > 0)
      {
      MdigAlloc(MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &MilDigitizer);
      MdigInquire(MilDigitizer, M_SIZE_X,    &BufSizeX);
      MdigInquire(MilDigitizer, M_SIZE_Y,    &BufSizeY);
      MdigInquire(MilDigitizer, M_SIZE_BAND, &BufSizeBand);
      /* Resize the display window */
      if((BufSizeX > DEFAULT_IMAGE_SIZE_X) || (BufSizeY > DEFAULT_IMAGE_SIZE_Y))
         {
         if(Area->parentWidget())
            {
            MilWindow* MainWindow = (MilWindow*)Area->parentWidget();
            MainWindow->resize(BufSizeX, BufSizeY+ MainWindow->ToolBar()->height());
            }
         }
      }
   else
      {
      MilDigitizer = M_NULL;
      BufSizeX     = DEFAULT_IMAGE_SIZE_X;
      BufSizeY     = DEFAULT_IMAGE_SIZE_Y;
      BufSizeBand  = DEFAULT_IMAGE_SIZE_BAND;
      }

   /* Allocate a MIL buffer. */
   MbufAllocColor(MilSystem, BufSizeBand, BufSizeX, BufSizeY, 8+M_UNSIGNED,
                  (MilDigitizer ? M_IMAGE+M_DISP+M_GRAB : M_IMAGE+M_DISP), &MilImage);

   /* Clear the buffer */
   MbufClear(MilImage,0);

   /* Select the MIL buffer to be displayed in the user-specified window */
   MdispSelectWindow(MilDisplay, MilImage, (MIL_WINDOW_HANDLE)Area->UserWindowHandle());

   /* Print a string in the image buffer using MIL.
      Note: When a MIL buffer is modified using a MIL command, the display
      automatically updates the window passed to MdispSelectWindow().
   */
   MgraFont(M_DEFAULT, M_FONT_DEFAULT_LARGE);
   MgraText(M_DEFAULT, MilImage, (BufSizeX/8)*2, BufSizeY/2,
            MIL_TEXT(" Welcome to MIL !!! "));
   MgraRect(M_DEFAULT, MilImage, ((BufSizeX/8)*2)-60, (BufSizeY/2)-80,
            ((BufSizeX/8)*2)+370, (BufSizeY/2)+100);
   MgraRect(M_DEFAULT, MilImage, ((BufSizeX/8)*2)-40, (BufSizeY/2)-60,
            ((BufSizeX/8)*2)+350, (BufSizeY/2)+80);
   MgraRect(M_DEFAULT, MilImage, ((BufSizeX/8)*2)-20, (BufSizeY/2)-40,
            ((BufSizeX/8)*2)+330, (BufSizeY/2)+60);

   /* Open a message box to wait for a key. */
   QMessageBox::information(0, "MIL application example",
                            "\"Welcome to MIL !!!\" was printed", QMessageBox::Ok);

   /* Grab in the user window if supported. */
   if (MilDigitizer)
      {
      /* Grab continuously. */
      MdigGrabContinuous(MilDigitizer, MilImage);

      /* Open a message box to wait for a key. */
      QMessageBox::information(0, "MIL application example",
                               "Continuous grab in progress", QMessageBox::Ok);

      /* Stop continuous grab. */
      MdigHalt(MilDigitizer);
      }
   Area->setAttribute(Qt::WA_OpaquePaintEvent, false);
   Area->setAttribute(Qt::WA_PaintOnScreen, false);

   /* Remove the MIL buffer from the display. */
   MdispSelect(MilDisplay, M_NULL);

   /* Free allocated objects. */
   MbufFree(MilImage);


   if (MilDigitizer)
      MdigFree(MilDigitizer);
   MdispFree(MilDisplay);
   MsysFree(MilSystem);
   MappFree(MilApplication);
   }


PaintArea::PaintArea(QWidget* parent)
   : QWidget(parent)
   , m_UserWindowHandle(0)
   {
   }

void PaintArea::startMil()
   {
   setAttribute(Qt::WA_OpaquePaintEvent);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   m_UserWindowHandle = winId();
   MilApplication(this);
   repaint();
   }

bool PaintArea::event(QEvent* e )
   {

#if QT_VERSION >= 0x040602 && M_MIL_USE_LINUX
   if(e->type() == QEvent::WinIdChange || e->type() == QEvent::Show)
#else
      if(e->type() == QEvent::Show)
#endif
         m_UserWindowHandle = winId();
   return QWidget::event(e);
   }

QSize PaintArea::sizeHint() const
   {
   return QSize(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_SIZE_Y);
   }

MilWindow::MilWindow()
   : QMainWindow(),
     m_PaintArea(0)
   {

   setWindowTitle(MIL_APPLICATION_NAME);

   QAction *startAct = new QAction(tr("&Start"), this);
   startAct->setShortcut(tr("Ctrl+s"));
   connect(startAct, SIGNAL(triggered()), this, SLOT(start()));

   m_Tools = new QToolBar(tr("Tool Bar"), this);
   m_Tools->addAction(startAct);
   addToolBar(m_Tools);

   m_PaintArea = new PaintArea(this);
   m_PaintArea->resize(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_SIZE_Y);
   setCentralWidget(m_PaintArea);

   }

void MilWindow::start()
   {
   m_PaintArea->startMil();
   }


/*****************************************************************
 *
 *   Name:     main()
 *
 *   Synopsis: Call initialization function, processes message loop.
 *
 ****************************************************************/
int main(int argc, char* argv[])
   {
#if M_MIL_USE_LINUX
   XInitThreads();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)   
   qInstallMessageHandler(MessageOutput);
#endif
#endif
   QApplication a(argc, argv);
   MilWindow *w = new MilWindow;
   w->show();
   return a.exec();
   }

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_MdispWindowQt.cpp"
#endif
