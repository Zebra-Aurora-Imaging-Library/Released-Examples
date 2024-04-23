//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef CHILDFRAME_H
#define CHILDFRAME_H
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QMainWindow>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

class MdispQtView;
class QLabel;
class QMdiArea;

class ChildFrame : public QMainWindow
   {
   Q_OBJECT

   public:
      ChildFrame( QWidget* parent = 0 );
      MdispQtView* view();
      QSize sizeHint() const;
      void MdiArea(QMdiArea* wa) { m_MdiArea = wa;}

   protected:
      virtual void closeEvent( QCloseEvent* e );
      virtual bool eventFilter(QObject *object, QEvent *e);
      
   private slots:
      void UpdateStatusBarWithFrameRate(double CurrentRate);
      void UpdateStatusBarWithScale(double CurrentScaleX, double CurrentScaleY);
      void UpdateStatusBarWithMousePosition(long DispX, long DispY, double BufX, double BufY);
      void UpdateContentSize(long SizeX, long SizeY);

   signals:
      void childClosedSignal();

   private:
      QMdiArea *m_MdiArea;

      MdispQtView* m_View;
      QLabel* m_FramerateIndicator;
      QLabel* m_ScaleIndicator;
      QLabel* m_MouseIndicator;

      bool m_Ready;
   };

#endif
