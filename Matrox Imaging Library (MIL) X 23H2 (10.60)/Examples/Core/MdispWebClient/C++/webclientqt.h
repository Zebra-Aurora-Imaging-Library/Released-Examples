//
// Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef WEBCLIENTQT_H
#define WEBCLIENTQT_H
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QApplication>
#include <QtPlugin>
#include <QtCore>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QLabel>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
#include "milweb.h"
static QMutex gMutex;
class MilWebWidget: public QLabel
   {
      Q_OBJECT
   public:
      MilWebWidget(MIL_ID ObjectId, MIL_UINT64 ObjectType, QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags()):QLabel(parent, f)
         {
         m_ObjectId   = ObjectId;
         m_ObjectType = ObjectType;
         Init();
         }

      MilWebWidget(MIL_ID ObjectId, MIL_UINT64 ObjectType, const QString & text, QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags()):QLabel(text, parent, f)
         {
         m_ObjectId   = ObjectId;
         m_ObjectType = ObjectType;
         Init();
         }

      void Init()
         {
         m_sizeX      = 0;
         m_sizeY      = 0;
         m_IsMapped   = false;
         setText("");
         connect(this, SIGNAL(ResizeSignal(int, int)), this, SLOT(onResizeImage(int, int)));
         connect(this, SIGNAL(ImageSignal(const QImage&)), this, SLOT(onImageDraw(const QImage&)),Qt::BlockingQueuedConnection);
         connect(this, SIGNAL(TextSignal(const QString&)), this, SLOT(onTextDraw(const QString&)),Qt::BlockingQueuedConnection);
         }
      
      inline void RedrawImage(const QImage& image)
         {
         emit ImageSignal(image);
         }
      inline void ResizeImage(int sizex , int sizey)
         {
         emit ResizeSignal(sizex, sizey);
         }

      inline void RedrawText(const QString& text)
         {
         emit TextSignal(text);
         }

      inline int SizeX() const { return m_sizeX;}
      inline int SizeY() const { return m_sizeY;}
      inline bool IsVisible() const { return m_IsMapped;}
      
      
   signals:
      void ImageSignal(const QImage& image);
      void ResizeSignal(int sizex, int sizey);
      void TextSignal(const QString& text);

   public slots:
      void onResizeImage(int SizeX, int SizeY)
         {
         if (SizeX != m_sizeX || SizeY != m_sizeY)
            {
            m_sizeX = SizeX;
            m_sizeY = SizeY;
            resize(SizeX,SizeY);
            }
         }
      void onImageDraw(const QImage & image)
         {
         setPixmap(QPixmap::fromImage(image.copy()));
         }
      
      void onTextDraw(const QString & text)
         {
         setText(text);
         }

   protected:
      virtual bool event(QEvent *e)
         {
         if((m_ObjectId && m_ObjectType == M_DISPLAY) &&
            (e->type() == QEvent::MouseButtonPress   ||
             e->type() == QEvent::MouseButtonRelease ||
             e->type() == QEvent::Leave              ||
             e->type() == QEvent::Wheel              ||
             e->type() == QEvent::MouseMove          ||
             e->type() == QEvent::KeyPress           ||
             e->type() == QEvent::KeyRelease ))
            {
            switch(e->type())
               {
               case QEvent::MouseButtonRelease:
                  {
                  QMouseEvent* Me = (QMouseEvent *)e;
                  MIL_INT EventType = M_NULL;
                  MIL_INT CombinationKeys = M_NULL;

                  switch(Me->button())
                     {
                     case Qt::LeftButton:
                        EventType = M_MOUSE_LEFT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_LEFT_BUTTON;
                        break;

                     case Qt::RightButton:
                        EventType = M_MOUSE_RIGHT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_RIGHT_BUTTON;
                        break;

                     case Qt::MiddleButton:
                        EventType = M_MOUSE_RIGHT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_MIDDLE_BUTTON;
                        break;

                     default:
                        break;
                     }

                  if(Me->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Me->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Me->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Me->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
                     MilWeb::MdispMessage(m_ObjectId,
                                       EventType,
                                       Me->x(),
                                       Me->y(),
                                       M_NULL,
                                       CombinationKeys,
                                       M_NULL);
                     }

                  }
                  break;

               case QEvent::MouseButtonPress:
                  {
                  QMouseEvent* Me = (QMouseEvent *)e;
                  MIL_INT EventType = M_NULL;
                  MIL_INT CombinationKeys = M_NULL;

                  switch(Me->button())
                     {
                     case Qt::LeftButton:
                        EventType = M_MOUSE_LEFT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_LEFT_BUTTON;
                        break;

                     case Qt::RightButton:
                        EventType = M_MOUSE_RIGHT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_RIGHT_BUTTON;
                        break;

                     case Qt::MiddleButton:
                        EventType = M_MOUSE_RIGHT_BUTTON_DOWN;
                        CombinationKeys = M_MOUSE_MIDDLE_BUTTON;
                        break;

                     default:
                        break;
                     }

                  if(Me->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Me->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Me->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Me->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
                     MilWeb::MdispMessage(m_ObjectId,
                                       EventType,
                                       Me->x(),
                                       Me->y(),
                                       M_NULL,
                                       CombinationKeys,
                                       M_NULL);
                     }

                  }
                  break;

               case QEvent::Leave:
                  {
                  QMouseEvent* Me = (QMouseEvent *)e;
                  MIL_INT EventType = M_MOUSE_LEAVE;
                  MIL_INT CombinationKeys = M_NULL;

                  switch(Me->button())
                     {
                     case Qt::LeftButton:
                        CombinationKeys = M_MOUSE_LEFT_BUTTON;
                        break;

                     case Qt::RightButton:
                        CombinationKeys = M_MOUSE_RIGHT_BUTTON;
                        break;

                     case Qt::MiddleButton:
                        CombinationKeys = M_MOUSE_MIDDLE_BUTTON;
                        break;

                     default:
                        break;
                     }

                  if(Me->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Me->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Me->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Me->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          Me->x(),
                                          Me->y(),
                                          M_NULL,
                                          CombinationKeys,
                                          M_NULL);
                     }

                  }
                  break;

               case QEvent::MouseMove:
                  {
                  QMouseEvent* Me = (QMouseEvent *)e;
                  MIL_INT EventType = M_MOUSE_MOVE;
                  MIL_INT CombinationKeys = M_NULL;

                  switch(Me->buttons())
                     {
                     case Qt::LeftButton:
                        CombinationKeys = M_MOUSE_LEFT_BUTTON;
                        break;

                     case Qt::RightButton:
                        CombinationKeys = M_MOUSE_RIGHT_BUTTON;
                        break;

                     case Qt::MiddleButton:
                        CombinationKeys = M_MOUSE_MIDDLE_BUTTON;
                        break;

                     default:
                        break;
                     }

                  if(Me->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Me->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Me->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Me->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          Me->x(),
                                          Me->y(),
                                          M_NULL,
                                          CombinationKeys,
                                          M_NULL);

                  }
                  break;

               case QEvent::Wheel:
                  {
                  QWheelEvent* We = (QWheelEvent*)e;
      #if QT_VERSION >= 0x050000
                  QPoint numDegrees = We->angleDelta() / 8;
                  int delta = numDegrees.y()/15;
      #else
                  int numDegrees = We->delta() / 8;
                  int delta = numDegrees/15;
      #endif
                  MIL_INT EventType = M_MOUSE_WHEEL;
                  MIL_INT CombinationKeys = M_NULL;

                  if(We->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(We->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(We->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(We->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
#if  QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          We->x(),
                                          We->y(),
                                          delta,
                                          CombinationKeys,
                                          M_NULL);
#else
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          We->position().x(),
                                          We->position().y(),
                                          delta,
                                          CombinationKeys,
                                          M_NULL);
#endif
                     }

                  }
                  break;

              case QEvent::KeyPress:
                  {
                  QKeyEvent* Ke = (QKeyEvent *) e;
                  MIL_INT EventType = M_KEY_DOWN;
                  MIL_INT CombinationKeys = M_NULL;

                  if(Ke->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Ke->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Ke->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Ke->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          M_DEFAULT,
                                          M_DEFAULT,
                                          Ke->nativeVirtualKey(),
                                          CombinationKeys,
                                          M_NULL);
                     }

                  }
                  break;

               case QEvent::KeyRelease:
                  {
                  QKeyEvent* Ke = (QKeyEvent *) e;
                  MIL_INT EventType = M_KEY_UP;
                  MIL_INT CombinationKeys = M_NULL;

                  if(Ke->modifiers()&Qt::ShiftModifier)
                     CombinationKeys |= M_KEY_SHIFT;

                  if(Ke->modifiers()&Qt::ControlModifier)
                     CombinationKeys |= M_KEY_CTRL;

                  if(Ke->modifiers()&Qt::AltModifier)
                     CombinationKeys |= M_KEY_ALT;

                  if(Ke->modifiers()&Qt::MetaModifier)
                     CombinationKeys |= M_KEY_WIN;

                  if(EventType)
                     {
                     MilWeb::MdispMessage(m_ObjectId,
                                          EventType,
                                          M_DEFAULT,
                                          M_DEFAULT,
                                          Ke->nativeVirtualKey(),
                                          CombinationKeys,
                                          M_NULL);
                     }

                  }
                  break;
               default:
                  qDebug()<<"Unhandled event";
                  break;
               }
            }
         if (e->type() == QEvent::Paint)
            m_IsMapped = true;
         
         return QLabel::event(e);
         }

   private:
      MIL_ID m_ObjectId;
      MIL_UINT64 m_ObjectType;
      bool m_IsMapped;
      int m_sizeX;
      int m_sizeY;
   };

#endif // WEBCLIENTQT_H
