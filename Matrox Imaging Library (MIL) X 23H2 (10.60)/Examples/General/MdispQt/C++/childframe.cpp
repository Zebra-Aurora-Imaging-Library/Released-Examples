//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#include "childframe.h"
#include "mdispqtview.h"
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QtGui>
#include <QLabel>
#include <QScrollArea>
#include <QStatusBar>
#include <QMdiArea>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

ChildFrame::ChildFrame( QWidget* parent )
   : QMainWindow(parent)
   {
   m_Ready = false;
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowIcon(QIcon(":/images/imaging.png"));

   // create the view and set it's parent
   m_View = new MdispQtView(this);
   setWindowTitle(m_View->filename());
   
   setCentralWidget(m_View);

   m_FramerateIndicator = new QLabel( statusBar() );
   statusBar()->addWidget(m_FramerateIndicator);

   m_ScaleIndicator = new QLabel( statusBar() );
   statusBar()->addWidget(m_ScaleIndicator);

   m_MouseIndicator = new QLabel ( statusBar());
   statusBar()->addWidget(m_MouseIndicator);

   UpdateStatusBarWithFrameRate(0.0);
   UpdateStatusBarWithScale(1.0, 1.0);
   UpdateStatusBarWithMousePosition(0, 0, 0.0, 0.0);

   connect( view(), SIGNAL(zoomFactorChanged(double, double)),   SLOT(UpdateStatusBarWithScale(double, double))     );
   connect( view(), SIGNAL(frameRateChanged(double)),    SLOT(UpdateStatusBarWithFrameRate(double)) );
   connect( view(), SIGNAL(filenameChanged(const QString&)), SLOT(setWindowTitle(const QString&))           );
   connect( view(), SIGNAL(mousePositionChanged(long, long, double, double)), SLOT(UpdateStatusBarWithMousePosition(long, long, double, double)));
   connect( view(), SIGNAL(sizeChanged(long, long)), SLOT(UpdateContentSize(long, long)));
   connect( view(), SIGNAL(filenameChanged(const QString& )), SLOT(setWindowTitle(QString)));

   installEventFilter(this);
   }

MdispQtView* ChildFrame::view()
   {
   return m_View;
   }

void ChildFrame::closeEvent( QCloseEvent* e )
   {
   if ( view()->close() )
      {
      emit childClosedSignal();
      e->accept();
      }
   else
      {
      e->ignore();
      }
   }
bool ChildFrame::eventFilter(QObject* /*object*/, QEvent *e)
   { 
   if (m_View && (e->type() == QEvent::ParentChange || e->type() == QEvent::Show)) 
       {
       m_View->SelectWindow();
       }
   return false;
    }

////////////////////////////////////////////////////////////////////////
// MIL: Used to update status bar with frame rate
////////////////////////////////////////////////////////////////////////
void ChildFrame::UpdateStatusBarWithFrameRate(double CurrentRate)
   {
   QString strCurrentRate = tr("Display Updates: %1 fps").arg( CurrentRate, 0, 'f', 2 );
   m_FramerateIndicator->setText(strCurrentRate);
   }

////////////////////////////////////////////////////////////////////////
// MIL: Used to update status bar with zoom factor
////////////////////////////////////////////////////////////////////////
void ChildFrame::UpdateStatusBarWithScale(double CurrentScaleX, double CurrentScaleY)
   {
   QString strCurrentScale = tr("%1,%2").arg( CurrentScaleX, 0, 'f', 4).arg(CurrentScaleY, 0, 'f', 4 );
   m_ScaleIndicator->setText(strCurrentScale);
   }

////////////////////////////////////////////////////////////////////////
// MIL: Used to update status bar with Mouse Position
////////////////////////////////////////////////////////////////////////
void ChildFrame::UpdateStatusBarWithMousePosition(long DispX, long DispY, double BufX, double BufY)
   {
   QString strCurrentMousePosition;

   strCurrentMousePosition=tr("M:(%1,%2)->(%3,%4)").arg(DispX,3).arg(DispY,3).arg(BufX, 0,'f',2).arg(BufY, 0,'f',2);
   m_MouseIndicator->setText(strCurrentMousePosition);

   }

QSize ChildFrame::sizeHint() const
   {
   QSize St(0,statusBar()->height());

   if(m_View)
      return m_View->sizeHint() +  St;
   else
      return QSize(300, 200);
   }

void ChildFrame::UpdateContentSize(long SizeX, long SizeY)
   {
   Q_UNUSED(SizeX);
   Q_UNUSED(SizeY);
   parentWidget()->adjustSize();
   }

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_childframe.cpp"
#endif
