//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include "mainframe.h"
#include "ui_mainframe.h"
#include "aboutbox.h"
#include "childframe.h"
#include "mdispqtview.h"
#include "mdispqtapp.h"

#include <QMdiArea>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QApplication>
#include <QDesktopWidget>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

MainFrame::MainFrame(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::MainFrame)
   {
   ui->setupUi(this);
#if M_MIL_USE_WINDOWS
   ui->actionX11Annotation->setIcon(QIcon(":/images/gdi.png"));
#else
   ui->actionX11Annotation->setVisible(false);
#endif
   m_ViewModeComboBox = new QComboBox;
   m_ViewModeComboBox->addItem(tr("M_DEFAULT"),     VIEW_MODE_DEFAULT);
   m_ViewModeComboBox->addItem(tr("M_TRANSPARENT"), VIEW_MODE_TRANSPARENT);
   m_ViewModeComboBox->addItem(tr("M_AUTO_SCALE"),  VIEW_MODE_AUTO_SCALE);
   m_ViewModeComboBox->addItem(tr("M_MULTI_BYTES"), VIEW_MODE_MULTI_BYTES);
   m_ViewModeComboBox->addItem(tr("M_BIT_SHIFT:2"), VIEW_MODE_BIT_SHIFT2);
   m_ViewModeComboBox->addItem(tr("M_BIT_SHIFT:4"), VIEW_MODE_BIT_SHIFT4);
   m_ViewModeComboBox->addItem(tr("M_BIT_SHIFT:8"), VIEW_MODE_BIT_SHIFT8);
   ui->DispToolBar->addWidget(m_ViewModeComboBox);

   m_MdiArea = new QMdiArea;
   setCentralWidget(m_MdiArea);
   setAttribute(Qt::WA_DeleteOnClose);

   connect( ui->actionExit,   SIGNAL(triggered()), qApp, SLOT(closeAllWindows()) );
   connect( ui->menuHelp,    SIGNAL(aboutToShow()), this, SLOT(windowMenuAboutToShow()) );
   connect( ui->action_Cascade, SIGNAL(triggered()), m_MdiArea, SLOT(cascadeSubWindows()) );
   connect( ui->action_Tile, SIGNAL(triggered()), m_MdiArea, SLOT(tileSubWindows()) );
   connect( m_MdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(windowActivated(QMdiSubWindow *)) );
   m_WindowMapper = new QSignalMapper(this);
   connect(m_WindowMapper, SIGNAL(mapped(QWidget *)),  this, SLOT(windowMenuActivated(QWidget *)));
   connect(m_ViewModeComboBox, SIGNAL(activated(int)), this, SLOT(ViewModeChanged(int)));

   QRect deskgeom = QApplication::desktop()->availableGeometry(this);
   resize(deskgeom.width() / 2, 2*deskgeom.height() / 3);
   updateActions(NULL);
   }

MainFrame::~MainFrame()
   {
   delete ui;
   }

void MainFrame::changeEvent(QEvent *e)
   {
   QMainWindow::changeEvent(e);
   switch (e->type()) {
      case QEvent::LanguageChange:
         ui->retranslateUi(this);
         break;
      default:
         break;
      }
   }

ChildFrame* MainFrame::activeChild()
   {
   QMdiSubWindow *activeSubWindow = m_MdiArea->activeSubWindow();
   if(activeSubWindow)
      return qobject_cast<ChildFrame *>(activeSubWindow->widget());

   return NULL;
   }

ChildFrame *MainFrame::CreateChildFrame()
   {
   ChildFrame* cf = new ChildFrame;
   cf->MdiArea(m_MdiArea);
   m_MdiArea->addSubWindow(cf);
   connect(cf, &ChildFrame::childClosedSignal, [=]() {
      updateActions(nullptr);
      });
   // to add actions
   return cf;
   }

//////////////////////////////////////////
// Action Handler
void MainFrame::on_actionAbout_triggered()
   {
   AboutBox about(this);
   about.exec();
   }


void MainFrame::closeEvent( QCloseEvent* e )
   {
   m_MdiArea->closeAllSubWindows();
   if (m_MdiArea->currentSubWindow())
      e->ignore();
   else
      e->accept();
   }

void MainFrame::on_actionNew_triggered()
   {
   ChildFrame* cf = CreateChildFrame();
   if ( !cf->view()->newDoc() )
      {
      cf->close();
      }
   else
      {
      cf->show();
      }

   }

void MainFrame::on_actionOpen_triggered()
   {
   MdispQtApp* app = (MdispQtApp*)qApp;
   QString fn = QFileDialog::getOpenFileName( this,
                                              tr("Open File"),
                                              app->m_ImagePath,
                                              tr("Images (*.mim *.bmp *.tif *.jpg *.jp2 *.png)"));
   if ( !fn.isEmpty() )
      {
      ChildFrame* cf = CreateChildFrame();
      if ( !cf->view()->load(fn) )
         {
         QMessageBox::warning( this, tr("MdispQt"),
                               tr("Could not load image from \"%1\".").arg(fn),
                               QMessageBox::Ok | QMessageBox::Default,
                               QMessageBox::NoButton );
         cf->close();
         }
      else
         {
         QString Path = QFileInfo(fn).path();
         if (!Path.isEmpty())
            app->m_ImagePath = Path;
         cf->show();
         }
      }
   }

void MainFrame::on_actionSave_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->save();
      }
   }

void MainFrame::on_actionSaveAs_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->saveAs();
      }
   }

void MainFrame::on_actionClose_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->close();
      if (m_MdiArea->currentSubWindow())
         m_MdiArea->currentSubWindow()->close();

      updateActions(NULL);
      }
   }

void MainFrame::on_actionViewStdToolbar_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->statusBar()->setVisible(on);
      }
   else
      {
      statusBar()->setVisible(on);
      }
   }


void MainFrame::on_actionGrabStart_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->GrabStart();
      updateActions(cf);
      }
   }

void MainFrame::on_actionGrabStop_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->GrabStop();
      updateActions(cf);
      }
   }


void MainFrame::on_actionShowHideOverlay_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->Overlay(on);
      updateActions(cf);
      }
   }

void MainFrame::on_actionRestrictedCursor_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->RestrictCursor(on);
      updateActions(cf);
      }
   }


void MainFrame::on_actionX11Annotation_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->X11Annotations(on);
      updateActions(cf);
      }
   }


void MainFrame::on_actionGraphicsAnnotations_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->GraphicsAnnotations(on);
      updateActions(cf);
      }
   }
void MainFrame::on_actionZoomIn_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ZoomIn();
      updateActions(cf);
      }
   }


void MainFrame::on_actionZoomOut_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ZoomOut();
      updateActions(cf);
      }
   }



void MainFrame::on_actionNoZoom_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->NoZoom();
      updateActions(cf);
      }
   }

void MainFrame::on_actionScaleDisplay_triggered(bool on)
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ScaleDisplay(on);
      updateActions(cf);
      }
   }

void MainFrame::ViewModeChanged(int Mode)
   {
   switch(Mode)
      {
      case VIEW_MODE_DEFAULT:
         on_actionViewDefault_triggered();
         break;

      case VIEW_MODE_TRANSPARENT:
         on_actionViewTransparent_triggered();
         break;

      case VIEW_MODE_AUTO_SCALE:
         on_actionViewAutoScale_triggered();
         break;

      case VIEW_MODE_MULTI_BYTES:
         on_actionVieewMultiBytes_triggered();
         break;

      case VIEW_MODE_BIT_SHIFT2:
         on_actionViewBitShift2_triggered();
         break;

      case VIEW_MODE_BIT_SHIFT4:
         on_actionViewBitShift4_triggered();
         break;

      case VIEW_MODE_BIT_SHIFT8:
         on_actionViewBitShift8_triggered();
         break;

      default:
         on_actionViewDefault_triggered();
         break;
      }
   }

void MainFrame::on_actionViewDefault_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_DEFAULT);
      updateActions(cf);
      }
   }

void MainFrame::on_actionViewTransparent_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_TRANSPARENT);
      updateActions(cf);
      }
   }

void MainFrame::on_actionViewAutoScale_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_AUTO_SCALE);
      updateActions(cf);
      }
   }

void MainFrame::on_actionVieewMultiBytes_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_MULTI_BYTES);
      updateActions(cf);
      }
   }

void MainFrame::on_actionViewBitShift2_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_BIT_SHIFT, 2);
      updateActions(cf);
      }
   }

void MainFrame::on_actionViewBitShift4_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_BIT_SHIFT, 4);
      updateActions(cf);
      }
   }

void MainFrame::on_actionViewBitShift8_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeViewMode(M_BIT_SHIFT, 8);
      updateActions(cf);
      }
   }


void MainFrame::on_actionDMILASyncDisable_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(false,M_DISABLE);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILASync1_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,1);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILASync5_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,5);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILASync10_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,10);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILASync15_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,15);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILASync30_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,30);
      updateActions(cf);
      }
   }


void MainFrame::on_actionDMILASyncMax_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeAsynchronousMode(true,M_INFINITE);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILCompressNone_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeCompressionType(M_NULL);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILCompressLossy_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeCompressionType(M_JPEG_LOSSY);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILCompressLossless_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeCompressionType(M_JPEG_LOSSLESS);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor60_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(60);
      updateActions(cf);
      }
   }


void MainFrame::on_actionDMILFactor70_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(70);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor75_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(70);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor80_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(80);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor82_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(82);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor85_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(85);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor87_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(87);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor90_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(90);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor92_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(92);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor95_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(95);
      updateActions(cf);
      }
   }

void MainFrame::on_actionDMILFactor99_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeQFactor(99);
      updateActions(cf);
      }
   }

void MainFrame::on_actionOverlayOpacityM_DEFAULT_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(M_DEFAULT);
      updateActions(cf);
      }
   }

void MainFrame::on_actionOverlayOpacityM_DISABLE_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(M_DISABLE);
      updateActions(cf);
      }
   }

void MainFrame::on_actionOverlayOpacity0_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(0);
      updateActions(cf);
      }
   }
void MainFrame::on_actionOverlayOpacity20_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(20);
      updateActions(cf);
      }
   }
void MainFrame::on_actionOverlayOpacity40_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(40);
      updateActions(cf);
      }
   }
void MainFrame::on_actionOverlayOpacity60_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(60);
      updateActions(cf);
      }
   }
void MainFrame::on_actionOverlayOpacity80_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(80);
      updateActions(cf);
      }
   }

void MainFrame::on_actionOverlayOpacity100_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeOverlayOpacity(100);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacityM_DEFAULT_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(M_DEFAULT);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacityM_DISABLE_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(M_DISABLE);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacity0_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(0);
      updateActions(cf);
      }
   }


void MainFrame::on_actionGLOpacity20_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(20);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacity40_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(40);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacity60_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(60);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacity80_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(80);
      updateActions(cf);
      }
   }

void MainFrame::on_actionGLOpacity100_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->ChangeGraphicListOpacity(100);
      updateActions(cf);
      }
   }

void MainFrame::on_actionNewRectangle_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraRectangle();
      updateActions(cf);
      }

   }
void MainFrame::on_actionNewArc_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraCircle();
      updateActions(cf);
      }
   }

void MainFrame::on_actionNewPolygon_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraPolygon();
      updateActions(cf);
      }
   }

void MainFrame::on_actionNewOrientedRect_triggered()
   {
   ChildFrame* cf = activeChild();
   if(cf)
      {
      cf->view()->OnGraOrientedRect();
      updateActions(cf);
      }
   }
void MainFrame::on_actionNewArcThreePoints_triggered()
   {
   ChildFrame* cf = activeChild();
   if(cf)
      {
      cf->view()->OnGraArcThreePoints();
      updateActions(cf);
      }
   }

void MainFrame::on_actionSelectgraphiccolor_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraChooseColor();
      updateActions(cf);
      }
   }

void MainFrame::on_actionCycleDrawDir_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraCycleDrawDir();
      updateActions(cf);
      }
   }

void MainFrame::on_actionToggleLineThickness_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraToggleLineThickness();
      updateActions(cf);
      }
   }

void MainFrame::on_actionFillgraphic_triggered()
   {
   ChildFrame* cf = activeChild();
   if (cf)
      {
      cf->view()->OnGraFill();
      updateActions(cf);
      }
   }

void MainFrame::windowMenuAboutToShow()
   {
   ui->menuHelp->clear();
   ui->menuHelp->addAction(ui->action_Cascade);
   ui->menuHelp->addAction(ui->action_Tile);
   if ( m_MdiArea->subWindowList().isEmpty() )
      {
      ui->action_Cascade->setEnabled(false);
      ui->action_Tile->setEnabled(false);
      }
   else
      {
      ui->action_Cascade->setEnabled(true);
      ui->action_Tile->setEnabled(true);
      }
   ui->menuHelp->addSeparator();

   QList <QMdiSubWindow *> windows = m_MdiArea->subWindowList();
   for ( int i=0; i<windows.size(); i++ )
      {
      QMdiSubWindow* subwindow = windows[i];
      ChildFrame* cf = (ChildFrame*) subwindow->widget();
      QString text;
      if(cf)
         text = tr("%1 %2").arg(i+1).arg(cf->view()->filename());
      else
         text = tr("%1 Image").arg(i+1);

      QAction *action = ui->menuHelp->addAction(text);

      //connect(action,SIGNAL(triggered()), this, SLOT(windowMenuActivated(int)));
      action->setCheckable(true);
      action->setChecked(m_MdiArea->activeSubWindow() == subwindow);
      connect(action, SIGNAL(triggered()), m_WindowMapper, SLOT(map()));
      m_WindowMapper->setMapping(action, windows.at(i));

      }
   }

void MainFrame::windowMenuActivated( QWidget* w )
   {
   if (w)
      {
      w->showNormal();
      }
   w->setFocus();
   }

void MainFrame::windowActivated( QMdiSubWindow* w )
   {
   if(w)
      updateActions((ChildFrame *)w->widget());
   }


void MainFrame::updateActions(ChildFrame *cf)
   {
   m_ViewModeComboBox->setEnabled(cf);
   ui->actionClose->setEnabled(cf);
   ui->actionSave->setEnabled(cf);
   ui->actionSaveAs->setEnabled(cf);
   ui->actionScaleDisplay->setEnabled(cf);
   ui->actionShowHideOverlay->setEnabled(cf);
   ui->actionX11Annotation->setEnabled(cf);
   ui->actionGraphicsAnnotations->setEnabled(cf);
   ui->actionViewDefault->setEnabled(cf);
   ui->actionViewTransparent->setEnabled(cf);
   ui->actionViewAutoScale->setEnabled(cf);
   ui->actionVieewMultiBytes->setEnabled(cf);
   ui->actionViewBitShift2->setEnabled(cf);
   ui->actionViewBitShift4->setEnabled(cf);
   ui->actionViewBitShift8->setEnabled(cf);

   if (cf)
      {
      MdispQtApp* app = (MdispQtApp*) qApp;
      MdispQtView* view = cf->view();

      ui->actionGrabStart->setEnabled( app->m_numberOfDigitizer != 0
                                                                   && !(app->m_pGrabView && app->m_isGrabStarted) );
      ui->actionGrabStop->setEnabled( app->m_pGrabView && app->m_isGrabStarted );

      ui->actionShowHideOverlay->setChecked( cf->view()->IsOverlayEnabled() );
      ui->menuOverlayOpacity->setEnabled( cf->view()->IsOverlayEnabled() );

      ui->actionX11Annotation->setChecked( cf->view()->IsNativeAnnotationsEnabled() );
      ui->actionGraphicsAnnotations->setChecked( cf->view()->IsGraphicsAnnotationsEnabled() );
      ui->actionScaleDisplay->setChecked( cf->view()->IsScaleDisplayEnabled() );

      ui->menuGraphicListOpacity->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionNewArc->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionNewRectangle->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionNewPolygon->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionNewOrientedRect->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionNewArcThreePoints->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionSelectgraphiccolor->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionFillgraphic->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionCycleDrawDir->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());
      ui->actionToggleLineThickness->setEnabled(cf->view()->IsGraphicsAnnotationsEnabled());

      if ( view->IsScaleDisplayEnabled() )
         {
         ui->actionNoZoom->setEnabled(false);
         ui->actionZoomIn->setEnabled(false);
         ui->actionZoomOut->setEnabled(false);
         }
      else
         {
         ui->actionZoomIn->setEnabled( cf->view()->CurrentZoomFactorX() < 16.0 );
         ui->actionZoomOut->setEnabled( cf->view()->CurrentZoomFactorX() > 1.0/16.0 );
         ui->actionNoZoom->setEnabled(true);
         }


      if(cf->view()->IsExclusive())
         {
         ui->actionRestrictedCursor->setEnabled(true);
         ui->actionRestrictedCursor->setChecked(view->CurrentRestrictCursor() == M_ENABLE);
         }
      else
         {
         ui->actionRestrictedCursor->setEnabled(false);
         }

      ui->actionX11Annotation->setEnabled(cf->view()->IsWindowed());

      ui->actionViewDefault->setChecked(view->CurrentViewMode()==M_DEFAULT);
      ui->actionViewTransparent->setChecked(view->CurrentViewMode()==M_TRANSPARENT);
      ui->actionViewAutoScale->setChecked(view->CurrentViewMode()==M_AUTO_SCALE);
      ui->actionVieewMultiBytes->setChecked(view->CurrentViewMode()==M_MULTI_BYTES);
      ui->actionViewBitShift2->setChecked((view->CurrentViewMode()==M_BIT_SHIFT)&& (view->CurrentShiftValue()==2));
      ui->actionViewBitShift4->setChecked((view->CurrentViewMode()==M_BIT_SHIFT)&& (view->CurrentShiftValue()==4));
      ui->actionViewBitShift8->setChecked((view->CurrentViewMode()==M_BIT_SHIFT)&& (view->CurrentShiftValue()==8));
      int ViewValue = VIEW_MODE_DEFAULT;
      switch(view->CurrentViewMode())
         {
         case M_DEFAULT:
            ViewValue = VIEW_MODE_DEFAULT;
            break;
         case M_TRANSPARENT:
            ViewValue = VIEW_MODE_TRANSPARENT;
            break;
         case M_AUTO_SCALE:
            ViewValue = VIEW_MODE_AUTO_SCALE;
            break;
         case M_MULTI_BYTES:
            ViewValue = VIEW_MODE_MULTI_BYTES;
            break;
         case M_BIT_SHIFT:
            {
            if(view->CurrentShiftValue() == 2)
               ViewValue = VIEW_MODE_BIT_SHIFT2;
            else if(view->CurrentShiftValue() == 4)
               ViewValue = VIEW_MODE_BIT_SHIFT4;
            else if(view->CurrentShiftValue() == 8)
               ViewValue = VIEW_MODE_BIT_SHIFT8;
            }
            break;
         }
      m_ViewModeComboBox->setCurrentIndex(ViewValue);

      ui->actionOverlayOpacityM_DEFAULT->setChecked(view->OverlayOpacity()==M_DEFAULT);
      ui->actionOverlayOpacityM_DISABLE->setChecked(view->OverlayOpacity()==M_DISABLE);
      ui->actionOverlayOpacity0->setChecked(view->OverlayOpacity()==0);
      ui->actionOverlayOpacity20->setChecked(view->OverlayOpacity()==20);
      ui->actionOverlayOpacity40->setChecked(view->OverlayOpacity()==40);
      ui->actionOverlayOpacity60->setChecked(view->OverlayOpacity()==60);
      ui->actionOverlayOpacity80->setChecked(view->OverlayOpacity()==80);
      ui->actionOverlayOpacity100->setChecked(view->OverlayOpacity()==100);

      ui->actionGLOpacityM_DEFAULT->setChecked(view->GraphicListOpacity()==M_DEFAULT);
      ui->actionGLOpacityM_DISABLE->setChecked(view->GraphicListOpacity()==M_DISABLE);
      ui->actionGLOpacity0->setChecked(view->GraphicListOpacity()==0);
      ui->actionGLOpacity20->setChecked(view->GraphicListOpacity()==20);
      ui->actionGLOpacity40->setChecked(view->GraphicListOpacity()==40);
      ui->actionGLOpacity60->setChecked(view->GraphicListOpacity()==60);
      ui->actionGLOpacity80->setChecked(view->GraphicListOpacity()==80);
      ui->actionGLOpacity100->setChecked(view->GraphicListOpacity()==100);

      if(view->IsNetworkedSystem())
         {
         ui->menuASynchronous_mode->setEnabled(true);
         ui->menuCompression->setEnabled(true);
         ui->menuQFactor->setEnabled(true);
         if(!view->IsInAsynchronousMode())
            ui->actionDMILASyncDisable->setChecked(true);
         else
            {
            ui->actionDMILASync1->setChecked(view->AsynchronousFrameRate()==1);
            ui->actionDMILASync5->setChecked(view->AsynchronousFrameRate()==5);
            ui->actionDMILASync10->setChecked(view->AsynchronousFrameRate()==10);
            ui->actionDMILASync15->setChecked(view->AsynchronousFrameRate()==15);
            ui->actionDMILASync30->setChecked(view->AsynchronousFrameRate()==30);
            ui->actionDMILASyncMax->setChecked(view->AsynchronousFrameRate()==M_INFINITE);
            }
         ui->actionDMILCompressNone->setChecked(view->CompressionType()==M_NULL);
         ui->actionDMILCompressLossy->setChecked(view->CompressionType()==M_JPEG_LOSSY);
         ui->actionDMILCompressLossless->setChecked(view->CompressionType()==M_JPEG_LOSSLESS);

         ui->actionDMILFactor60->setChecked(view->QFactor()==60);
         ui->actionDMILFactor70->setChecked(view->QFactor()==70);
         ui->actionDMILFactor75->setChecked(view->QFactor()==75);
         ui->actionDMILFactor80->setChecked(view->QFactor()==80);
         ui->actionDMILFactor82->setChecked(view->QFactor()==82);
         ui->actionDMILFactor85->setChecked(view->QFactor()==85);
         ui->actionDMILFactor87->setChecked(view->QFactor()==87);
         ui->actionDMILFactor90->setChecked(view->QFactor()==90);
         ui->actionDMILFactor92->setChecked(view->QFactor()==92);
         ui->actionDMILFactor95->setChecked(view->QFactor()==95);
         ui->actionDMILFactor99->setChecked(view->QFactor()==99);

         }
      else
         {
         ui->menuASynchronous_mode->setEnabled(false);
         ui->menuCompression->setEnabled(false);
         ui->menuQFactor->setEnabled(false);
         }
      }
   else
      {
      ui->actionGrabStart->setEnabled(false);
      ui->actionGrabStop->setEnabled(false);

      ui->actionShowHideOverlay->setChecked(false);
      ui->menuOverlayOpacity->setEnabled(false);
      ui->actionX11Annotation->setChecked(false);
      ui->actionGraphicsAnnotations->setChecked(false);
      ui->actionZoomIn->setEnabled(false);
      ui->actionZoomOut->setEnabled(false);
      ui->actionNoZoom->setEnabled(false);
      ui->actionRestrictedCursor->setEnabled(false);
      ui->actionScaleDisplay->setEnabled(false);
      ui->menuASynchronous_mode->setEnabled(false);
      ui->menuCompression->setEnabled(false);
      ui->menuQFactor->setEnabled(false);
      ui->menuGraphicListOpacity->setEnabled(false);
      ui->actionNewArc->setEnabled(false);
      ui->actionNewRectangle->setEnabled(false);
      ui->actionNewPolygon->setEnabled(false);
      ui->actionNewOrientedRect->setEnabled(false);
      ui->actionNewArcThreePoints->setEnabled(false);
      ui->actionSelectgraphiccolor->setEnabled(false);
      ui->actionFillgraphic->setEnabled(false);
      ui->actionCycleDrawDir->setEnabled(false);
      ui->actionToggleLineThickness->setEnabled(false);
      }
   }

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_mainframe.cpp"
#endif
