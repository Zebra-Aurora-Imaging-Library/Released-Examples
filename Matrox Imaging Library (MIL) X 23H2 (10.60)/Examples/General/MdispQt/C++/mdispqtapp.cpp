//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#include "mdispqtapp.h"
#include "mainframe.h"
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QtGui>
#include <QMessageBox>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

MdispQtApp::MdispQtApp( int& argc, char** argv )
   : QApplication( argc, argv )
   , m_isCurrentlyHookedOnErrors(false)
{
   m_MilDigitizer = M_NULL;

   connect(this, SIGNAL(message(const QString&)), this, SLOT(OnErrorMessage(const QString&)),  Qt::BlockingQueuedConnection);
   
   // Allocate an application and system [CALL TO MIL]
   MappAllocDefault(M_DEFAULT, &m_MilApplication, &m_MilSystem, M_NULL, M_NULL, M_NULL);

   // Inquire MIL images path
   MIL_STRING ImagePath;
   MappInquire(M_DEFAULT, M_MIL_DIRECTORY_IMAGES, ImagePath);
#if M_MIL_USE_WINDOWS
   m_ImagePath = QString::fromStdWString(ImagePath);
#else
   m_ImagePath = QString::fromStdString(ImagePath);
#endif

   // Hook MIL error on function DisplayError() [CALL TO MIL]
   MappHookFunction(M_DEFAULT, M_ERROR_CURRENT,DisplayErrorExt,M_NULL);

   m_isCurrentlyHookedOnErrors = true;

   // Disable MIL error message to be displayed as the usual way [CALL TO MIL]
   MappControl(M_DEFAULT, M_ERROR,M_PRINT_DISABLE);

   // Inquire number of digitizers available on the system [CALL TO MIL]
   MsysInquire(m_MilSystem,M_DIGITIZER_NUM,&m_numberOfDigitizer);

   // Digitizer is available
   if (m_numberOfDigitizer)
   {
      // Allocate a digitizer [CALL TO MIL]
      MdigAlloc(m_MilSystem,M_DEFAULT,MIL_TEXT("M_DEFAULT"),M_DEFAULT,&m_MilDigitizer);

      // Inquire digitizer informations [CALL TO MIL]
      MdigInquire(m_MilDigitizer,M_SIZE_X,&m_digitizerSizeX);
      MdigInquire(m_MilDigitizer,M_SIZE_Y,&m_digitizerSizeY);
      MdigInquire(m_MilDigitizer,M_SIZE_BAND,&m_digitizerNbBands);
   }
 
   // Initialize the state of the grab
   m_isGrabStarted = false;

   // Initialize GUI
   MainFrame* mf = new MainFrame();
   //setMainWidget(mf);

   mf->show();

   mf->on_actionNew_triggered();

}

MdispQtApp::~MdispQtApp()
{
   //Free the digitizer [CALL TO MIL]
   if(m_MilDigitizer)
      MdigFree (m_MilDigitizer);

   //Free the system [CALL TO MIL]
   if(m_MilSystem)
      MsysFree (m_MilSystem);

   if(m_MilApplication)
   {
      // Enable MIL error message to be displayed as the usual way [CALL TO MIL]
      MappControl(M_DEFAULT, M_ERROR,M_PRINT_ENABLE);

      // Unhook MIL error on function DisplayError() [CALL TO MIL]
      if(m_isCurrentlyHookedOnErrors)
      {
         MappHookFunction(M_DEFAULT, M_ERROR_CURRENT+M_UNHOOK,DisplayErrorExt,M_NULL);
         m_isCurrentlyHookedOnErrors = false;
      }

      // Free the application [CALL TO MIL]
      MappFree(m_MilApplication);
   }
}


/////////////////////////////////////////////////////////////////////////
// MIL: Hook-handler function: DisplayError()
/////////////////////////////////////////////////////////////////////////

MIL_INT MFTYPE MdispQtApp::DisplayErrorExt(MIL_INT /*HookType*/, MIL_ID EventId, void* /*UserDataPtr*/)
{
   ((MdispQtApp*) qApp)->DisplayError(EventId);
   return M_NULL;
}

long MFTYPE MdispQtApp::DisplayError( MIL_ID EventId )
{
   MIL_STRING  ErrorMessageFunction;
   MIL_STRING  ErrorMessage;
   MIL_STRING  ErrorSubMessage1;
   MIL_STRING  ErrorSubMessage2;
   MIL_STRING  ErrorSubMessage3;
   MIL_INT  NbSubCode;
   QString  QErrorMessage;

   //Retrieve error message [CALL TO MIL]
   MappGetHookInfo(EventId,M_MESSAGE+M_CURRENT_OPCODE,ErrorMessageFunction);
   MappGetHookInfo(EventId,M_MESSAGE+M_CURRENT,ErrorMessage);
   MappGetHookInfo(EventId,M_CURRENT_SUB_NB,&NbSubCode);

   if (NbSubCode > 2)
      MappGetHookInfo(EventId,M_MESSAGE+M_CURRENT_SUB_3,ErrorSubMessage3);
   if (NbSubCode > 1)
      MappGetHookInfo(EventId,M_MESSAGE+M_CURRENT_SUB_2,ErrorSubMessage2);
   if (NbSubCode > 0)
      MappGetHookInfo(EventId,M_MESSAGE+M_CURRENT_SUB_1,ErrorSubMessage1);
#if M_MIL_USE_LINUX
   QErrorMessage = ErrorMessageFunction.c_str();
#else
    QErrorMessage = QString::fromWCharArray(ErrorMessageFunction.c_str());
#endif
   QErrorMessage = QErrorMessage + "\n";
#if M_MIL_USE_LINUX
   QErrorMessage = QErrorMessage + ErrorMessage.c_str();
#else
   QErrorMessage = QErrorMessage + QString::fromWCharArray(ErrorMessage.c_str());
#endif

   if(NbSubCode > 0)
   {
      QErrorMessage = QErrorMessage + "\n";
#if M_MIL_USE_LINUX
      QErrorMessage = QErrorMessage + ErrorSubMessage1.c_str();
#else
      QErrorMessage = QErrorMessage + QString::fromWCharArray(ErrorSubMessage1.c_str());
#endif
   }

   if(NbSubCode > 1)
   {
      QErrorMessage = QErrorMessage + "\n";
#if M_MIL_USE_LINUX
      QErrorMessage = QErrorMessage + ErrorSubMessage2.c_str();
#else
      QErrorMessage = QErrorMessage + QString::fromWCharArray(ErrorSubMessage2.c_str());
#endif
   }

   if(NbSubCode > 2)
   {
      QErrorMessage = QErrorMessage + "\n";
#if M_MIL_USE_LINUX
      QErrorMessage = QErrorMessage + ErrorSubMessage3.c_str();
#else
      QErrorMessage = QErrorMessage + QString::fromWCharArray(ErrorSubMessage3.c_str());
#endif
   }

   QErrorMessage = QErrorMessage + "\n\n";
   QErrorMessage = QErrorMessage + "Do you want to continue error print?";
   
   QThread* current = QThread::currentThread();
   if (current != QCoreApplication::instance()->thread())
      emit message(QErrorMessage);
   else
      OnErrorMessage(QErrorMessage);
   
   return M_NULL;
}

void MdispQtApp::OnErrorMessage(const QString& QErrorMessage)
   {
   if ( QMessageBox::warning( 0, tr("MIL Error"), QErrorMessage, QMessageBox::Yes,
                              QMessageBox::No ) == QMessageBox::No )
      {
      MappHookFunction(M_DEFAULT, M_ERROR_CURRENT+M_UNHOOK,DisplayErrorExt,M_NULL);
      ((MdispQtApp*) qApp)->HookedOnErrors(false);
      }
   }

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_mdispqtapp.cpp"
#endif
