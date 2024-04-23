//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef MDISPWINDOWQT_H
#define MDISPWINDOWQT_H
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QMainWindow>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

class PaintArea : public QWidget
   {
   Q_OBJECT

   public:
      PaintArea(QWidget* parent = 0);

      void startMil();

      virtual QSize sizeHint() const;

      WId UserWindowHandle() const { return m_UserWindowHandle;}

      QPaintEngine* paintEngine() const { return 0; }

   protected:

      virtual bool event(QEvent* e);

   private:
      WId    m_UserWindowHandle;

   };

class MilWindow : public QMainWindow
   {
   Q_OBJECT

   public:
      MilWindow();
      QToolBar* ToolBar() const { return m_Tools;}

   public slots:
      void start();

   private:
      PaintArea* m_PaintArea;
      QToolBar *m_Tools;
   };

#endif // MDISPWINDOWQT_H
