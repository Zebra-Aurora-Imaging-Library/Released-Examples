//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef PAINTAREA_H
#define PAINTAREA_H
#include <mil.h>
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include <QtGui>
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
#if M_MIL_USE_LINUX
#include <X11/Xlib.h> // For XClearArea, XSendEvent...
#undef Bool
#endif
typedef struct _MOUSEPOSITION
   {
   void Set(MIL_INT DisplayPositionX, MIL_INT DisplayPositionY, MIL_DOUBLE BufferPositionX, MIL_DOUBLE BufferPositionY)
      {
      m_DisplayPositionX = DisplayPositionX;
      m_DisplayPositionY = DisplayPositionY;
      m_BufferPositionX = BufferPositionX;
      m_BufferPositionY = BufferPositionY;
      }
   _MOUSEPOSITION()
      {
      Set(M_INVALID, M_INVALID, M_INVALID, M_INVALID);
      }
   _MOUSEPOSITION& operator=(const _MOUSEPOSITION& MousePosition)
      {
      Set(MousePosition.m_DisplayPositionX,
         MousePosition.m_DisplayPositionY,
         MousePosition.m_BufferPositionX,
         MousePosition.m_BufferPositionY);

      return *this;
      }
      
   _MOUSEPOSITION(const _MOUSEPOSITION&)=default;   
   _MOUSEPOSITION(_MOUSEPOSITION&&) = default;
   _MOUSEPOSITION& operator=(_MOUSEPOSITION&&)= default;
      
   MIL_INT     m_DisplayPositionX;
   MIL_INT     m_DisplayPositionY;
   MIL_DOUBLE  m_BufferPositionX;
   MIL_DOUBLE  m_BufferPositionY;
   }MOUSEPOSITION;


// Events sent when Mouse Position changed
class MilMouseEvent : public QEvent
{
public:
   MilMouseEvent( MOUSEPOSITION Pos)
      : QEvent(QEvent::Type(TYPE))
      , m_MousePosition(Pos)
   {
   }

   inline MOUSEPOSITION MousePostion() const { return m_MousePosition;}
   static const int TYPE = QEvent::User + 8;

private:
   MOUSEPOSITION m_MousePosition;

};



#include <QWidget>
#include <mil.h>


class ChildFrame;

class MdispQtView : public QWidget
   {
   Q_OBJECT

   public:
      MdispQtView( QWidget* parent );
      ~MdispQtView();
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
      QPaintEngine* paintEngine() const;
#endif
      bool newDoc();
      bool load( const QString& filename );
      bool save();
      bool saveAs();

      const QString& filename() const;

      void GrabStart();
      void GrabStop();

      void Overlay( bool on );
      void GraphicsAnnotations( bool on );
      void X11Annotations( bool on ); 
      void ZoomIn();
      void ZoomOut();
      void NoZoom();
      void ScaleDisplay( bool on );
      void ChangeViewMode(long ViewMode, long ShiftValue=M_NULL);
      void ChangeAsynchronousMode(bool Enabled, MIL_INT FrameRate);
      void ChangeCompressionType(MIL_INT CompressionType);
      void ChangeQFactor(MIL_INT QFactor);
      void ChangeOverlayOpacity(MIL_INT Opacity);
      void ChangeGraphicListOpacity(MIL_INT Opacity);
      bool IsNetworkedSystem();
      void RestrictCursor(bool on);
      void SelectWindow();
      void UpdateMousePosition();
      void OnGraRectangle();
      void OnGraCircle();
      void OnGraPolygon();
      void OnGraOrientedRect();
      void OnGraArcThreePoints();
      void OnGraChooseColor();
      void OnGraFill();
      void OnGraCycleDrawDir();
      void OnGraToggleLineThickness();
      void ResetPrimitiveCreation(){m_PrimitiveInCreation = M_NULL;}

      inline MIL_ID MilDisplay() const {return m_MilDisplay;}
      inline bool IsWindowed() const { return m_isWindowed ;}
      inline bool IsExclusive() const { return m_isExclusive ;}
      inline long CurrentShiftValue() const { return m_currentShiftValue; }
      inline long CurrentViewMode() const { return m_currentViewMode;}
      inline long CurrentRestrictCursor() const { return m_currentRestrictCursor;}
      bool IsGraphicsAnnotationsEnabled() { return m_isGraphicsAnnotationsEnabled;}
      bool IsNativeAnnotationsEnabled() { return m_isNativeAnnotationsEnabled;} 
      inline bool IsScaleDisplayEnabled() const { return m_isScaleDisplayEnabled;}
      inline MIL_DOUBLE CurrentZoomFactorX() const {return m_currentZoomFactorX; }
      inline MIL_DOUBLE CurrentZoomFactorY() const {return m_currentZoomFactorY; }
      inline bool IsOverlayEnabled() const { return m_isOverlayEnabled; }
      inline bool IsModified() const {return m_Modified;}
      inline MIL_INT CompressionType() const {return  m_currentCompressionType;}      
      inline bool IsInAsynchronousMode() const {return  m_isInAsynchronousMode;}      
      inline MIL_INT AsynchronousFrameRate() const {return  m_currentAsynchronousFrameRate;}      
      inline MIL_INT QFactor() const {return  m_currentQFactor;}      
      inline MIL_INT OverlayOpacity() const {return  m_currentOverlayOpacity;}
      inline MIL_INT GraphicListOpacity() const {return  m_currentGraphicListOpacity;}
      inline MIL_INT ImageSizeX() const { return m_imageSizeX;}
      inline MIL_INT ImageSizeY() const { return m_imageSizeY;}
      inline void SetMousePosition(const MOUSEPOSITION& MousePosition){m_LastMousePosition = MousePosition;}
      inline QString strippedName(const QString& fullPath) const { return QFileInfo(fullPath).fileName();}
      QSize sizeHint() const;

      friend long MFTYPE FrameEndHookHandler(long HookType, MIL_ID EventId, void* UserDataPtr);

   signals:
      void zoomFactorChanged( double zoomFactorX, double zoonFactorY );
      void frameRateChanged( double frameRate );
      void filenameChanged( const QString& filename );
      void mousePositionChanged(long, long, double, double);
      void sizeChanged(long, long);
   protected:
      virtual void closeEvent( QCloseEvent* e );
      virtual void timerEvent( QTimerEvent* e );
      virtual void paintEvent(QPaintEvent *event);
      virtual void customEvent(QEvent* e);
#if M_MIL_USE_WINDOWS
      virtual void resizeEvent(QResizeEvent *);
#endif
   private:
      void Initialize();
      void InitializeOverlay();
      void RemoveFromDisplay();
      void UpdateContentSize();

      void Zoom( MIL_DOUBLE ZoomFactorToApplyX, MIL_DOUBLE ZoomFactorToApplyY );

      bool m_Modified;
      QString m_Filename;
      bool m_FilenameValid;

      MIL_ID   m_MilImage;          // Image buffer identifier.

      long     m_imageSizeX;        // Buffer Size X
      long     m_imageSizeY;        // Buffer Size Y
      long     m_NbBands;

      int m_FrameRateTimer;

      //Attributes
      MIL_ID   m_MilOverlayImage;               //Overlay image buffer identifier
      MIL_ID   m_MilDisplay;                    //Display identifier.
      MIL_ID   m_MilGraphContext;
      MIL_ID    m_MilGraphList;

      MIL_DOUBLE   m_currentZoomFactorX;            //Current zoom factor
      MIL_DOUBLE   m_currentZoomFactorY;            //Current zoom factor
      MIL_INT      m_currentViewMode;               //Current view mode (M_VIEW_MODE dispControl)
      MIL_INT      m_currentShiftValue;             //Current bit-shift value(M_VIEW_BIT_SHIFT dispControl)
      MIL_INT      m_currentCompressionType;        //Current compression type (M_COMPRESSION_TYPE dispControl)
      bool         m_isInAsynchronousMode;          //Current asynchronous mode (M_ASYNC_UPDATE dispControl)
      MIL_INT      m_currentAsynchronousFrameRate;  //Current asynchronous frame rate (M_UPDATE_RATE_MAX dispControl)
      MIL_INT      m_currentQFactor;                //Current Q factor (M_Q_FACTOR dispControl)
      MIL_INT      m_currentOverlayOpacity;         //Current Overlay Opacity (M_OVERLAY_OPACITY dispControl)
      MIL_INT      m_currentGraphicListOpacity;     //Current Graphic List Opacity (M_GRAPHIC_LIST_OPACITY dispControl)
      MIL_INT      m_currentRestrictCursor;         //Current cursor restriction (M_RESTRICT_CURSOR dispControl)
      bool     m_isScaleDisplayEnabled;          //Scale Display state(M_SCALE_DISPLAY dispControl)
      bool     m_isGraphicsAnnotationsEnabled;    //Graphics Annotations state
      bool     m_isNativeAnnotationsEnabled;         //X11/Win32 Annotations state 
      bool     m_isOverlayEnabled;              //Overlay show state
      bool     m_isOverlayInitialized;          //Overlay initialization state

      bool     m_isWindowed;
      bool     m_isExclusive;
      bool     m_InitDone;

      MIL_INT64   m_bufferAttributes;  // Buffer attributes that will be allocated

      MOUSEPOSITION m_LastMousePosition;
      MIL_INT       m_PrimitiveInCreation;
      
#if M_MIL_USE_LINUX
      GC            m_GC;             // X11 graphics context
#endif
      
   };

#endif
