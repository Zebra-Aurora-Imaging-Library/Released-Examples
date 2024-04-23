/***************************************************************************************/
/*
 * File name: webclientqt.cpp
 *
 * Synopsis:  This program shows how to use web publishing.
 *
 *
 *
 * Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */

#include "webclientqt.h"
#include "webclient.h"

/* default ws url */
#define MILWEB_URL                MIL_TEXT("ws://localhost:7681")

/* Window title. */
#define MIL_APPLICATION_NAME      "MdispWebClient"

/* Default image dimensions. */
#define DEFAULT_IMAGE_SIZE_X       640
#define DEFAULT_IMAGE_SIZE_Y       480

QVBoxLayout *layout = NULL;


#if STATIC_QT5
#include <QtPlugin>
#if !M_MIL_USE_LINUX
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#else
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif
#endif

#if M_MIL_USE_LINUX
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

/* display message handler */
void DisplayMessage(MIL_UINT8 *MsgData, MIL_INT MsgLength, MIL_INT64 /*MsgTag*/, void *UserData)
   {
   MilWebWidget *msgWidget = (MilWebWidget *)UserData;
   if(msgWidget && msgWidget->IsVisible() && MsgData && MsgLength > 0)
      {
      QString Result = QString::fromLatin1((const char *)MsgData);
      msgWidget->RedrawText(Result);
      }
   }

/* display image handler */
void DisplayImage(MIL_UINT8 *Data, MIL_INT SizeByte, MIL_INT SizeX, MIL_INT SizeY, MIL_INT PitchByte, void *UserData)
   {
   MilWebWidget *dispWidget = (MilWebWidget *)UserData;
   if(dispWidget && dispWidget->IsVisible() && Data && SizeByte > 0 && SizeX > 0 && SizeY > 0)
      {
      if (dispWidget->SizeX() != SizeX || dispWidget->SizeY() != SizeY)
         {
         dispWidget->ResizeImage(SizeX, SizeY);
         return;
         }      
      QImage image((const unsigned char*)Data, SizeX, SizeY, PitchByte,QImage::Format_RGB32);
      dispWidget->RedrawImage(image);
      }
   }

/* Open new connection */
/* and connect to published objects */
MIL_ID StartConnection(MIL_CONST_TEXT_PTR Url)
   {
   MIL_ID AppId  = M_NULL;
   MIL_ID DispId = M_NULL;
   MIL_ID MsgId  = M_NULL;

   MilWeb::MappOpenConnection(Url, M_DEFAULT, M_DEFAULT, &AppId);
   if(AppId)
      {
      //MilWeb::MappControl(AppId, M_ERROR, M_PRINT_DISABLE);
      MilWeb::MappInquireConnection(AppId, M_WEB_PUBLISHED_NAME, MIL_TEXT("Message"), M_DEFAULT, &MsgId);
      MilWeb::MappInquireConnection(AppId, M_WEB_PUBLISHED_NAME, MIL_TEXT("Display"), M_DEFAULT, &DispId);

      if(DispId)
         {
         QLabel *dispWidget = new MilWebWidget(DispId, M_DISPLAY, "display");
         dispWidget->setFocusPolicy(Qt::ClickFocus);
         layout->addWidget(dispWidget);
         dispWidget->setFocus();
         MilWeb::MdispControl(DispId, M_WEB_PUBLISHED_FORMAT, M_BGR32);
         MilWeb::MobjHookFunction(DispId, M_UPDATE_WEB, UpdateHookHandler, dispWidget);
         }

      if(MsgId)
         {
         QLabel *msgWidget = new MilWebWidget(MsgId,M_MESSAGE_MAILBOX, "message");
         layout->addWidget(msgWidget);
         MilWeb::MobjHookFunction(MsgId, M_UPDATE_WEB,  UpdateHookHandler, msgWidget);
         }
      }
   return AppId;
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
   MIL_ID AppId = M_NULL;
   MIL_CONST_TEXT_PTR url = MILWEB_URL;
#if M_MIL_USE_LINUX
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)   
   qInstallMessageHandler(MessageOutput);
#endif
#endif
   
   QApplication a(argc, argv);
#if M_MIL_USE_WINDOWS
   int win_argc;
   LPWSTR* win_argv = CommandLineToArgvW(GetCommandLineW(),&win_argc);
   if(win_argc > 1)
      url = win_argv[1];
#else
   if(argc >1)
      url = argv[1];
#endif
   QWidget *mainWidget = new QWidget;
   layout = new QVBoxLayout;
   mainWidget->setLayout(layout);
   mainWidget->resize(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_SIZE_Y);
   
   mainWidget->show();

   AppId = StartConnection(url);

   a.exec();

   EndConnection(AppId);
   
#if M_MIL_USE_WINDOWS
   if(win_argv)
      LocalFree(win_argv);
#endif
   return 0;
   }

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_webclientqt.cpp"
#endif
