//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

#include "mdispqtview.h"
#include "mdispqtapp.h"
#include "childframe.h"

#include <QtGui>
#if M_MIL_USE_LINUX
#include <cairo-xlib.h>
#include <QX11Info>
#endif
#include <cmath>
#include <cstdlib>
#if M_MIL_USE_LINUX 
#include <X11/Xlib.h> // For XClearArea, XSendEvent...
#undef Bool
#else
#include <windows.h>
#endif
#include <QColorDialog>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

#define NON_MOUSE_MASK (~(ButtonPressMask|ButtonReleaseMask|PointerMotionMask))

#define IMAGE_FILE   M_IMAGE_PATH MIL_TEXT("BaboonRGB.mim")

MIL_INT MFTYPE MouseFct(MIL_INT /*HookType*/, MIL_ID EventID, void* UserDataPtr)
   {
   MdispQtView* pCurrentView = (MdispQtView *)UserDataPtr;

   if(pCurrentView)
      {
      MOUSEPOSITION MousePosition;
      MdispGetHookInfo(EventID, M_MOUSE_POSITION_X,         &MousePosition.m_DisplayPositionX);
      MdispGetHookInfo(EventID, M_MOUSE_POSITION_Y,         &MousePosition.m_DisplayPositionY);
      MdispGetHookInfo(EventID, M_MOUSE_POSITION_BUFFER_X,  &MousePosition.m_BufferPositionX);
      MdispGetHookInfo(EventID, M_MOUSE_POSITION_BUFFER_Y,  &MousePosition.m_BufferPositionY);

      pCurrentView->SetMousePosition(MousePosition);
      ((MdispQtApp*)qApp)->postEvent( pCurrentView, new MilMouseEvent(MousePosition));
      }
   return 0;
   }

MIL_INT MFTYPE GraphicListModifiedHookFct(MIL_INT /*HookType*/, MIL_ID EventID, void* UserDataPtr)
   {
   MdispQtView *pCurrentView = (MdispQtView *)UserDataPtr;

   if(pCurrentView)
      {
      MIL_INT State = M_NULL;
      MgraGetHookInfo(EventID, M_INTERACTIVE_GRAPHIC_STATE, &State);

      if((State != M_STATE_WAITING_FOR_CREATION) && (State != M_STATE_BEING_CREATED))
         {
         pCurrentView->ResetPrimitiveCreation();
         }
      }
   return 0;
   }

MdispQtView::MdispQtView( QWidget* parent )
   :QWidget(parent)
   , m_Modified(false)
   {
   m_InitDone = false;
   setAttribute(Qt::WA_OpaquePaintEvent, true);
   setAttribute(Qt::WA_PaintOnScreen, true);
   setAttribute(Qt::WA_NoSystemBackground, false);


   m_MilOverlayImage            = M_NULL;   // Overlay image buffer identifier
   m_MilDisplay                 = M_NULL;   // Display identifier.
   m_MilGraphContext            = M_NULL;
   m_MilGraphList               = M_NULL;

   static int viewNumber = 0;
   m_Filename = QString(tr("Image%1.mim")).arg(++viewNumber);
   m_FilenameValid = false;

   m_currentZoomFactorX         = 1.0;
   m_currentZoomFactorY         = 1.0;
   m_isWindowed                 = true;
   m_isExclusive                = false;
   m_isOverlayEnabled           = false;    // Overlay state
   m_isOverlayInitialized       = false;
   m_isScaleDisplayEnabled       = false;
   m_isGraphicsAnnotationsEnabled = false;
   m_isNativeAnnotationsEnabled   = false;
   m_currentViewMode            = M_TRANSPARENT;
   m_currentShiftValue          = M_NULL;
   m_isInAsynchronousMode          = false;
   m_currentCompressionType        = M_NULL;
   m_currentAsynchronousFrameRate  = M_INFINITE;
   m_currentQFactor                = M_DEFAULT;
   m_currentOverlayOpacity         = M_DEFAULT;
   m_currentGraphicListOpacity     = M_DEFAULT;
   m_currentRestrictCursor         = M_ENABLE;
   m_PrimitiveInCreation           = M_NULL;
   m_FrameRateTimer = startTimer(500);
   
#if M_MIL_USE_LINUX
   m_GC                           = M_NULL;
#endif
   }

MdispQtView::~MdispQtView()
   {
   // Halt the grab, deselected the display, free the display and the image buffer
   // only if MbufAlloc was successful
   if (m_MilImage)
      {
      // Make sure display is deselected and grab is halt
      RemoveFromDisplay();

      // Free image buffer [CALL TO MIL]
      MbufFree(m_MilImage);
      }
   
#if M_MIL_USE_LINUX
   if(m_GC)
      XFreeGC(QX11Info::display(),m_GC);
#endif
   }

void MdispQtView::GrabStart()
   {
    // TODO: Add your command handler code here
	
	/////////////////////////////////////////////////////////////////////////
	// MIL: Write code that will be executed on a grab start
	/////////////////////////////////////////////////////////////////////////

   // If there is a grab in a view, halt the grab before starting a new one
   if(((MdispQtApp*)qApp)->m_isGrabStarted)
      ((MdispQtApp*)qApp)->m_pGrabView->GrabStop();

   // Start a continuous grab in this view
   MdigGrabContinuous(((MdispQtApp*)qApp)->m_MilDigitizer, m_MilImage);

   // Update the variable GrabIsStarted
   ((MdispQtApp*)qApp)->m_isGrabStarted = true;

   // GrabInViewPtr is now a pointer to m_pGrabView view
   ((MdispQtApp*)qApp)->m_pGrabView = this;

   // Document has been modified
   m_Modified = true;

	/////////////////////////////////////////////////////////////////////////	
	// MIL: Write code that will be executed on a grab start
	/////////////////////////////////////////////////////////////////////////
   
   }

void MdispQtView::GrabStop()
   {
   // TODO: Add your command handler code here
 
   /////////////////////////////////////////////////////////////////////////
	// MIL: Write code that will be executed on a grab stop 
	/////////////////////////////////////////////////////////////////////////
   // Halt the grab
   MdigHalt(((MdispQtApp*)qApp)->m_MilDigitizer);
   ((MdispQtApp*)qApp)->m_isGrabStarted = false;

   /////////////////////////////////////////////////////////////////////////
	// MIL: Write code that will be executed on a grab stop 
	/////////////////////////////////////////////////////////////////////////
   }

void MdispQtView::Overlay( bool on )
   {
   // Enable overlay
   if (on && !m_isOverlayEnabled)
      {
      MdispControl(m_MilDisplay, M_OVERLAY, M_ENABLE);

      //If overlay buffer as not been initialized yet, do it now.
      if(!m_isOverlayInitialized)
         InitializeOverlay();

      m_isOverlayEnabled = true;
      }

   // Disable overlay
   else if (!on && m_isOverlayEnabled)
      {
      // Disable the overlay display. [CALL TO MIL]
      MdispControl(m_MilDisplay, M_OVERLAY, M_DISABLE);

      m_isOverlayInitialized = false;
      m_isOverlayEnabled     = false;
      }

   /////////////////////////////////////////////////////////////////////////
   // MIL: Write code that will be executed when 'add overlay' is selected
   /////////////////////////////////////////////////////////////////////////

   }


void MdispQtView::Initialize()
   {
   // Allocate a display [CALL TO MIL]
   MdispAlloc(((MdispQtApp*)qApp)->m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &m_MilDisplay);

   if(m_MilDisplay)
      {
      MIL_INT DisplayType = MdispInquire(m_MilDisplay, M_DISPLAY_TYPE, M_NULL);
      
      // Check display type [CALL TO MIL]
      if((DisplayType&(M_WINDOWED|M_EXCLUSIVE)) !=M_WINDOWED)
         m_isWindowed = false;

      if(DisplayType&(M_EXCLUSIVE))
         m_isExclusive = true;
 
      ChangeViewMode(M_DEFAULT);
		
      if(IsNetworkedSystem())
         {
         // Check compression type [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_COMPRESSION_TYPE, &m_currentCompressionType);
         
         // Check asynchronous mode [CALL TO MIL]
         m_isInAsynchronousMode = (MdispInquire(m_MilDisplay, M_ASYNC_UPDATE, M_NULL) == M_ENABLE);
         
         // Check asynchronous frame rate [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_UPDATE_RATE_MAX, &m_currentAsynchronousFrameRate);
         
         // Check Q factor [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_Q_FACTOR, &m_currentQFactor);

         // Check Overlay Opacity [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_OVERLAY_OPACITY, &m_currentOverlayOpacity);

         // Check Graphic List Opacity [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_GRAPHIC_LIST_OPACITY, &m_currentGraphicListOpacity);

         }

      if(m_isExclusive)
         {
         setAttribute(Qt::WA_PaintOnScreen, false);
         MdispInquire(m_MilDisplay, M_RESTRICT_CURSOR,    &m_currentRestrictCursor);
         
         }

		// Allow panning and zooming with the mouse [CALL TO MIL]
		MdispControl(m_MilDisplay, M_MOUSE_USE, M_ENABLE);

      // Tell Mil display we are using Qt.SDK.
      MdispControl(m_MilDisplay, M_QT_MODE, M_ENABLE);

      // Allow mouse cursor handling [CALL TO MIL]
      MdispControl(m_MilDisplay, M_MOUSE_CURSOR_CHANGE, M_ENABLE);

      
      // Hook a function to mouse-movement event, to update cursor position in status bar.
      MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE, MouseFct, (void*)this);
      }
   m_InitDone = true;
   /////////////////////////////////////////////////////////////////////////
   // MIL: Code that will be executed when a view is first attached to the document
   /////////////////////////////////////////////////////////////////////////
   }



void MdispQtView::RemoveFromDisplay()

   {
   //Halt grab if in process in THIS view
   if ((((MdispQtApp*)qApp)->m_pGrabView == this) &&
        ((MdispQtApp*)qApp)->m_isGrabStarted)
      {
      //Ask the digitizer to halt the grab [CALL TO MIL]
      MdigHalt(((MdispQtApp*)qApp)->m_MilDigitizer);


      ((MdispQtApp*)qApp)->m_isGrabStarted = false;
      }

   if (m_MilImage && m_MilDisplay)
      {
      //Deselect the buffer from it's display object and given window [CALL TO MIL]
      MdispSelect(m_MilDisplay, M_NULL);

      // Hook from mouse-movement event.
      MdispHookFunction(m_MilDisplay, M_MOUSE_MOVE+M_UNHOOK, MouseFct, (void*)this);

      //Free GraphicList [CALL TO MIL]
      if(m_MilGraphList)
      {
         MgraFree(m_MilGraphList);
         m_MilGraphList = M_NULL;
      }
      if(m_MilGraphContext)
      {
         MgraFree(m_MilGraphContext);
         m_MilGraphContext = M_NULL;
      }

      //Free the display [CALL TO MIL]
      MdispFree(m_MilDisplay);
      m_MilDisplay = M_NULL;
      }
   }

#if M_MIL_USE_WINDOWS
void MdispQtView::resizeEvent(QResizeEvent*)
   {
    if(m_MilDisplay)
       {
       MdispControl(m_MilDisplay, M_UPDATE, M_NOW);
       }
   }
#endif

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
QPaintEngine* MdispQtView::paintEngine()const 
   {
   if (!m_InitDone)
      return NULL;
   else if (m_MilDisplay && m_isWindowed)
      return NULL;
   else
      return QWidget::paintEngine();
   }
#endif


void MdispQtView::paintEvent( QPaintEvent* /*ev*/)
   {
   if(!m_MilDisplay)
      {
      QPainter p( this);
      QFont font;

      font.setStyleStrategy( QFont::NoAntialias );
      font.setBold(true);
      p.setFont(font);
      p.setPen( QColor(255,0,0) );
      p.drawText( 0, 0, width() , p.fontMetrics().height(), Qt::AlignCenter,
                  tr("Display Allocation Failed!") );
      }

   else if (m_isWindowed)
      {
       if (m_isNativeAnnotationsEnabled)
          {
#if M_MIL_USE_LINUX
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
          QPainter p( this);
          QFont font;
          p.setPen( QColor(255,0,255) );
          p.drawText( 0, 0, contentsRect().width(), p.fontMetrics().height(), Qt::AlignCenter,
                      tr("Window Annotations") );
#else
          // PaintEngine is disabled use Xlib functions
          cairo_t *cr;
          cairo_text_extents_t extents;
          cairo_surface_t *surface = cairo_xlib_surface_create (QX11Info::display(),
                                                                winId(),
                                                                DefaultVisual(QX11Info::display(),0),
                                                                contentsRect().width(),
                                                                contentsRect().height());
          cr = cairo_create (surface);
          cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
          cairo_set_font_size (cr, 12.0);
          cairo_set_source_rgb (cr, 1.0, 0.0, 1.0);
          cairo_text_extents(cr, "Window Annotation", &extents);
          cairo_move_to (cr, (m_isScaleDisplayEnabled)?(contentsRect().width()/2 - extents.width/2):(m_imageSizeX/2 - extents.width/2),20.0);
          cairo_show_text (cr, "Window Annotation");
          cairo_destroy(cr);
          XFlush(QX11Info::display());
#endif
#elif M_MIL_USE_WINDOWS
          // PaintEngine is disabled use GDi functions
          RECT rectangle;
          rectangle.top =  0;
          rectangle.left =  0;
          rectangle.right =  rect().width();
          rectangle.bottom = rect().height();
          
          HDC hdc = GetDC((HWND)winId());
          SetTextColor(hdc,RGB(255,0,255));
          SetBkMode(hdc, TRANSPARENT);
          DrawText(hdc,L"Window Annotation", 17,&rectangle, DT_CENTER);
          ReleaseDC((HWND)winId(), hdc);          
#endif
          }
      }
   else
      {
      // In external mode, write message in window
      QPainter p( this);
      QFont font;

      font.setStyleStrategy( QFont::NoAntialias );
      font.setBold(true);
      p.setFont(font);
      p.setPen( QColor(0,0,0) );
      p.drawText( 0, 0, m_isScaleDisplayEnabled ? width() :
                  contentsRect().width(), p.fontMetrics().height(), Qt::AlignLeft,
                  tr("Image Displayed on external screen") );
      }
   }


void MdispQtView::timerEvent( QTimerEvent* e )
   {
   if (m_MilDisplay)
      {
      if (e->timerId() == m_FrameRateTimer)
         {
         MIL_DOUBLE CurrentFrameRate = M_NULL;
         MdispInquire(m_MilDisplay, M_UPDATE_RATE, &CurrentFrameRate);
         emit frameRateChanged(CurrentFrameRate);

         MIL_DOUBLE ZoomX = 1.0, ZoomY = 1.0;
         MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_X, &ZoomX);
         MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_Y, &ZoomY);
         emit zoomFactorChanged(ZoomX, ZoomY);
         }
      }
   }

void MdispQtView::ZoomIn()
   {
	 if(m_MilDisplay)
		{
			 MIL_DOUBLE ZoomX =1.0, ZoomY =1.0;
			 MdispInquire(m_MilDisplay,M_ZOOM_FACTOR_X, &ZoomX);
			 MdispInquire(m_MilDisplay,M_ZOOM_FACTOR_Y, &ZoomY);

			 if(ZoomX <=8.0 && ZoomY<=8.0)
				 {
					 ZoomX*=2.0;
					 ZoomY*=2.0;
				 }
			 //Perform zooming with MIL (using MdispZoom)
			 Zoom( ZoomX, ZoomY);
		 }
 }

void MdispQtView::ZoomOut()
   {
	if(m_MilDisplay)
		{
			MIL_DOUBLE ZoomX =1.0, ZoomY =1.0;
			MdispInquire(m_MilDisplay,M_ZOOM_FACTOR_X, &ZoomX);
			MdispInquire(m_MilDisplay,M_ZOOM_FACTOR_Y, &ZoomY);
			if(ZoomX >=0.125 && ZoomY>=0.125)
				{
					ZoomX/=2.0;
					ZoomY/=2.0;
				}
			//Perform zooming with MIL (using MdispZoom)
			Zoom( ZoomX, ZoomY );
		}
	}

void MdispQtView::NoZoom()
   {
	if(m_MilDisplay)
		{
			//Perform zooming with MIL
			Zoom(1.0, 1.0);
         MdispPan(m_MilDisplay, M_NULL,M_NULL);
		}
   }

void MdispQtView::Zoom( MIL_DOUBLE ZoomFactorToApplyX, MIL_DOUBLE ZoomFactorToApplyY )
{
	if( m_MilDisplay)
	 {
		 //Apply zoom  [CALL TO MIL]
		 MdispZoom(m_MilDisplay, ZoomFactorToApplyX, ZoomFactorToApplyY);
		 m_currentZoomFactorX = ZoomFactorToApplyX;
		 m_currentZoomFactorY = ZoomFactorToApplyY;
		 emit zoomFactorChanged(m_currentZoomFactorX, m_currentZoomFactorY);
	}
}

void MdispQtView::ScaleDisplay( bool on )
   {
   if(m_MilDisplay)
      {
#if M_MIL_USE_LINUX      
      if(!on)
         {
         XClearWindow(QX11Info::display(), winId());
         XFlush(QX11Info::display());
         XSync(QX11Info::display(),False);
         }
#endif
      //Using MIL, enable/disable Scale Display Mode [CALL TO MIL]
		MdispControl(m_MilDisplay, M_SCALE_DISPLAY, on ? M_ENABLE : M_DISABLE);

		m_isScaleDisplayEnabled = on;


      // clear contents
      repaint();

      }
   }


void MdispQtView::OnGraRectangle()
   {
   if(m_MilDisplay)
      {
      if(m_isGraphicsAnnotationsEnabled)
         {
         MgraColor(m_MilGraphContext, M_COLOR_WHITE);
         MgraInteractive(m_MilGraphContext, m_MilGraphList, M_GRAPHIC_TYPE_RECT, M_DEFAULT, M_AXIS_ALIGNED_RECT);
         m_PrimitiveInCreation = M_AXIS_ALIGNED_RECT;
         }
      }
   }
void MdispQtView::OnGraCircle()
   {
   if(m_MilDisplay)
      {
      if(m_isGraphicsAnnotationsEnabled)
         {
         MgraColor(m_MilGraphContext, M_COLOR_YELLOW);
         MgraInteractive(m_MilGraphContext, m_MilGraphList, M_GRAPHIC_TYPE_ARC, M_DEFAULT, M_CIRCLE);
         m_PrimitiveInCreation = M_CIRCLE;
         }
      }
   }
void MdispQtView::OnGraPolygon()
   {
   if(m_MilDisplay)
      {
      if(m_isGraphicsAnnotationsEnabled)
         {
         MgraColor(m_MilGraphContext, M_COLOR_RED);
         MgraInteractive(m_MilGraphContext, m_MilGraphList, M_GRAPHIC_TYPE_POLYGON, M_DEFAULT, M_DEFAULT);
         m_PrimitiveInCreation = M_GRAPHIC_TYPE_POLYGON;
         }
      }
   }

void MdispQtView::OnGraOrientedRect()
   {
   if(m_MilDisplay)
      {
      if(m_isGraphicsAnnotationsEnabled)
         {
         MgraColor(m_MilGraphContext, M_COLOR_BLUE);
         MgraInteractive(m_MilGraphContext, m_MilGraphList, M_GRAPHIC_TYPE_RECT, M_DEFAULT, M_ORIENTED_RECT);
         m_PrimitiveInCreation = M_ORIENTED_RECT;
         }
      }
   }
void MdispQtView::OnGraArcThreePoints()
   {
   if(m_MilDisplay)
      {
      if(m_isGraphicsAnnotationsEnabled)
         {
         MgraColor(m_MilGraphContext, M_COLOR_GREEN);
         MgraInteractive(m_MilGraphContext, m_MilGraphList, M_GRAPHIC_TYPE_ARC, M_DEFAULT, M_ARC_THREE_POINTS);
         m_PrimitiveInCreation = M_ARC_THREE_POINTS;
         }
      }
   }

void MdispQtView::OnGraChooseColor()
   {
   if(m_MilDisplay && m_MilGraphList)
      {
      QColor c = QColorDialog::getColor(Qt::white,this);
      if(c.isValid())
          {
          MIL_INT NewColor = M_RGB888(c.red(),c.green(), c.blue());
          MgraControlList(m_MilGraphList, M_ALL_SELECTED, M_DEFAULT, M_COLOR, (MIL_INT)NewColor);
          MgraControlList(m_MilGraphList, M_ALL, M_DEFAULT, M_GRAPHIC_SELECTED, M_FALSE);
          }
      }
   }

void MdispQtView::OnGraCycleDrawDir()
   {
   if(m_MilDisplay && m_MilGraphList)
      {
      const MIL_INT NbValues = 4;
      MIL_INT DrawDirValues[NbValues] = 
         {
         M_NONE,
         M_PRIMARY_DIRECTION,      
         M_SECONDARY_DIRECTION,
         M_PRIMARY_DIRECTION + M_SECONDARY_DIRECTION
         };

      MIL_INT NbGrph = 0;
      MgraInquireList(m_MilGraphList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS, &NbGrph);
      MIL_INT DrawDirCurValueIdx = 0;
      MIL_INT NbSelectedGrph = 0;
      for(MIL_INT g = 0; g < NbGrph; g++) // Finds the highest draw direction among selected graphics
         {
         if(MgraInquireList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_GRAPHIC_SELECTED, M_NULL) == M_TRUE)
            {
            MIL_INT GrphDrawDir =
               MgraInquireList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_DRAW_DIRECTION, M_NULL);
            if(GrphDrawDir == M_DEFAULT)
               { GrphDrawDir = M_NONE; }

            MIL_INT FoundIdx = std::find(&DrawDirValues[0], &DrawDirValues[NbValues], GrphDrawDir) - &DrawDirValues[0];
            if(FoundIdx < NbValues)
               { DrawDirCurValueIdx = std::max(DrawDirCurValueIdx, FoundIdx); }
            ++NbSelectedGrph;
            }
         }

      if(NbSelectedGrph > 0)
         {
         // toggle current value
         DrawDirCurValueIdx = (DrawDirCurValueIdx + 1) % NbValues;
         MgraControlList(m_MilGraphList, M_ALL_SELECTED, M_DEFAULT, M_DRAW_DIRECTION, DrawDirValues[DrawDirCurValueIdx]);
         }
      }
   }

void MdispQtView::OnGraToggleLineThickness()
   {
   if(m_MilDisplay && m_MilGraphList)
      {
      MIL_INT NbGrph = 0;
      MgraInquireList(m_MilGraphList, M_LIST, M_DEFAULT, M_NUMBER_OF_GRAPHICS, &NbGrph);

      for(MIL_INT g = 0; g < NbGrph; g++)
         {
         if(MgraInquireList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_GRAPHIC_SELECTED, M_NULL) == M_TRUE)
            {
            MIL_INT CurLineThickness =
               MgraInquireList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_LINE_THICKNESS, M_NULL);

            // Toggle thickness
            if(CurLineThickness > 1)
               { MgraControlList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_LINE_THICKNESS, 1); }
            else
               { MgraControlList(m_MilGraphList, M_GRAPHIC_INDEX(g), M_DEFAULT, M_LINE_THICKNESS, 3); }
            }
         }
      }
   }

void MdispQtView::OnGraFill()
   {
   if(m_MilDisplay && m_MilGraphList)
      {
      MgraControlList(m_MilGraphList, M_ALL_SELECTED, M_DEFAULT, M_FILLED, M_TRUE);
      MgraControlList(m_MilGraphList, M_ALL, M_DEFAULT, M_GRAPHIC_SELECTED, M_FALSE);
      }
   }

void MdispQtView::ChangeGraphicListOpacity(MIL_INT Opacity)
   {
   if(m_MilDisplay && m_MilGraphList)
      {
      // Apply Opacity to display [CALL TO MIL]
      MdispControl(m_MilDisplay, M_GRAPHIC_LIST_OPACITY, Opacity);

      // Check if control worked correctly before considering it successful [CALL TO MIL]
      if((Opacity == M_DEFAULT) || (MdispInquire(m_MilDisplay, M_GRAPHIC_LIST_OPACITY, M_NULL) == Opacity))
         {
         m_currentGraphicListOpacity = Opacity;
         }
      }
   }

void MdispQtView::X11Annotations( bool on ) 
   { 
   m_isNativeAnnotationsEnabled = on;
   
#if !M_MIL_USE_LINUX
  if(on)
      {
      MdispControl(m_MilDisplay, M_WINDOW_ANNOTATIONS, M_ENABLE);
      }
   else
      {
      MdispControl(m_MilDisplay,M_WINDOW_ANNOTATIONS,M_DISABLE);
      }
#endif
  repaint();
   }
 
void MdispQtView::GraphicsAnnotations( bool on )
{
if(m_MilDisplay)
   {
   m_isGraphicsAnnotationsEnabled = on;

    if(m_isGraphicsAnnotationsEnabled)
      {
      if(!m_MilGraphContext && !m_MilGraphList)
         {
         MIL_INT BufSizeX  = 0, BufSizeY = 0;
         MIL_INT Offset    = 15;

         MgraAlloc(((MdispQtApp*)qApp)->m_MilSystem, &m_MilGraphContext);
         MgraAllocList(((MdispQtApp*)qApp)->m_MilSystem, M_DEFAULT, &m_MilGraphList);
         MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, m_MilGraphList);



         MdispControl(m_MilDisplay, M_UPDATE_GRAPHIC_LIST, M_DISABLE);
         MbufInquire(m_MilImage, M_SIZE_X, &BufSizeX);
         MbufInquire(m_MilImage, M_SIZE_Y, &BufSizeY);

         MgraClear(m_MilGraphContext, m_MilGraphList);

         MgraColor(m_MilGraphContext, M_COLOR_LIGHT_BLUE);
         MgraRect(m_MilGraphContext, m_MilGraphList, Offset, Offset, BufSizeX - Offset, BufSizeY - Offset);

         MgraColor(m_MilGraphContext, M_COLOR_GREEN);
         MgraControl(m_MilGraphContext, M_BACKGROUND_MODE, M_TRANSPARENT);
         MgraControl(m_MilGraphContext, M_TEXT_ALIGN_HORIZONTAL, M_CENTER);
         MgraControl(m_MilGraphContext, M_TEXT_ALIGN_VERTICAL, M_CENTER);
         MgraControl(m_MilGraphContext, M_FONT_SIZE, 24);
         MgraFont(m_MilGraphContext, M_FONT_DEFAULT_TTF);
         MgraText(m_MilGraphContext, m_MilGraphList, BufSizeX/2, Offset, MIL_TEXT("Interactive Graphic Annotations"));

         //Initialize graphic list
         MdispControl(m_MilDisplay, M_UPDATE_GRAPHIC_LIST, M_ENABLE);
         MdispControl(m_MilDisplay, M_GRAPHIC_LIST_INTERACTIVE, M_ENABLE);

         MgraHookFunction(m_MilGraphList, M_INTERACTIVE_GRAPHIC_STATE_MODIFIED, GraphicListModifiedHookFct, (void*)this);
         }
      }
   else
      {
      MgraHookFunction(m_MilGraphList, M_INTERACTIVE_GRAPHIC_STATE_MODIFIED+M_UNHOOK, GraphicListModifiedHookFct, (void*)this);
      MdispControl(m_MilDisplay, M_ASSOCIATED_GRAPHIC_LIST_ID, M_NULL);

      if(m_MilGraphList)
         {
         MgraFree(m_MilGraphList);
         m_MilGraphList = M_NULL;
         }
      if(m_MilGraphContext)
         {
         MgraFree(m_MilGraphContext);
         m_MilGraphContext = M_NULL;
         }
      }
   }
}


void MdispQtView::ChangeViewMode(long ViewMode,long ShiftValue)
   {
   if(m_MilDisplay)
      {
      //Apply view mode on display [CALL TO MIL]
      MdispControl(m_MilDisplay, M_VIEW_MODE, ViewMode);

      if(ViewMode == M_BIT_SHIFT)
         MdispControl(m_MilDisplay, M_VIEW_BIT_SHIFT, ShiftValue);

      //Check if control worked correctly before considering it as successful [CALL TO MIL]
      if(MdispInquire(m_MilDisplay, M_VIEW_MODE,M_NULL)==ViewMode)
         {
         //Make sure ViewMode Mode combo box shows current view mode
         m_currentViewMode   = ViewMode;
         m_currentShiftValue = ShiftValue;
         }
      }
   }

void MdispQtView::ChangeCompressionType(MIL_INT CompressionType)
   {
   if(m_MilDisplay)
      {
      // Apply compression type to display [CALL TO MIL]
      MdispControl(m_MilDisplay, M_COMPRESSION_TYPE, CompressionType);
   
      // Check if control worked correctly before considering it successful [CALL TO MIL]
      if(MdispInquire(m_MilDisplay, M_COMPRESSION_TYPE, M_NULL) == CompressionType)
         {
         m_currentCompressionType = CompressionType;
         }
      }
   }

void MdispQtView::ChangeAsynchronousMode(bool Enabled, MIL_INT FrameRate)
   {
   if(Enabled && (FrameRate != m_currentAsynchronousFrameRate))
      {
      if(m_MilDisplay)
         {
         // Apply asynchronous frame rate to display [CALL TO MIL]
         MdispControl(m_MilDisplay, M_UPDATE_RATE_MAX, FrameRate);
      
         // Check if control worked correctly before considering it successful [CALL TO MIL]
         if(MdispInquire(m_MilDisplay, M_UPDATE_RATE_MAX, M_NULL) == FrameRate)
            {
            m_currentAsynchronousFrameRate = FrameRate;
            }
         }
      }

   if((Enabled && !m_isInAsynchronousMode) ||
      (!Enabled && m_isInAsynchronousMode))
      {
      if(m_MilDisplay)
         {
         // Apply asynchronous update to display [CALL TO MIL]
         MdispControl(m_MilDisplay, M_ASYNC_UPDATE, (Enabled ? M_ENABLE : M_DISABLE));
      
         // Check if control worked correctly before considering it successful [CALL TO MIL]
         if(MdispInquire(m_MilDisplay, M_ASYNC_UPDATE, M_NULL) == (Enabled ? M_ENABLE : M_DISABLE))
            {
            m_isInAsynchronousMode = Enabled;
            }
         }
      }
   }

void MdispQtView::ChangeQFactor(MIL_INT QFactor)
   {
   if(m_MilDisplay)
      {
      // Apply Q factor to display [CALL TO MIL]
      MdispControl(m_MilDisplay, M_Q_FACTOR, QFactor);
   
      // Check if control worked correctly before considering it successful [CALL TO MIL]
      if(MdispInquire(m_MilDisplay, M_Q_FACTOR, M_NULL) == QFactor)
         {
         m_currentQFactor = QFactor;
         }
      }
   }

void MdispQtView::ChangeOverlayOpacity(MIL_INT Opacity)
   {
   if(m_MilDisplay)
      {
      // Apply Opacity to display [CALL TO MIL]
      MdispControl(m_MilDisplay, M_OVERLAY_OPACITY, Opacity);

      // Check if control worked correctly before considering it successful [CALL TO MIL]
      if((Opacity == M_DEFAULT) || (MdispInquire(m_MilDisplay, M_OVERLAY_OPACITY, M_NULL) == Opacity))
         {
         m_currentOverlayOpacity = Opacity;
         }
      }
   }


bool MdispQtView::IsNetworkedSystem()
   {
   bool NetworkedSystem = false;
   MIL_ID SystemId = ((MdispQtApp*)qApp)->m_MilSystem;

   // Check if system is networked (DistributedMIL) [CALL TO MIL]
   if(SystemId)
      NetworkedSystem = (MsysInquire(SystemId, M_LOCATION, M_NULL) == M_REMOTE);

   return NetworkedSystem;
   }


void MdispQtView::InitializeOverlay()
   {
   // Initialize overlay if not already done
   if ((!m_isOverlayInitialized) && (m_MilDisplay))
      {
      //Only do it on a valid windowed display [CALL TO MIL]
      if (m_MilImage && m_MilDisplay )
         {
         // Prepare overlay buffer //
         ////////////////////////////

         // Enable display overlay annotations.
         MdispControl(m_MilDisplay, M_OVERLAY, M_ENABLE);

         // Inquire the Overlay buffer associated with the displayed buffer [CALL TO MIL]
         MdispInquire(m_MilDisplay, M_OVERLAY_ID, &m_MilOverlayImage);

         // Clear the overlay to transparent.
         MdispControl(m_MilDisplay, M_OVERLAY_CLEAR, M_DEFAULT);
         
         // Disable the overlay display update to accelerate annotations.
         MdispControl(m_MilDisplay, M_OVERLAY_SHOW, M_DISABLE);


         // Draw MIL monochrome overlay annotation *
         //*****************************************

         // Inquire MilOverlayImage size x and y [CALL TO MIL]
         long imageWidth  = MbufInquire(m_MilOverlayImage,M_SIZE_X,M_NULL);
         long imageHeight = MbufInquire(m_MilOverlayImage,M_SIZE_Y,M_NULL);

         // Set graphic text to transparent background. [CALL TO MIL]
         MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

         // Set drawing color to white. [CALL TO MIL]
         MgraColor(M_DEFAULT, M_COLOR_WHITE);

         // Print a string in the overlay image buffer. [CALL TO MIL]
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth/9, imageHeight/5,    MIL_TEXT(" -------------------- "));
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth/9, imageHeight/5+25, MIL_TEXT(" - MIL Overlay Text - "));
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth/9, imageHeight/5+50, MIL_TEXT(" -------------------- "));

         // Print a green string in the green component overlay image buffer. [CALL TO MIL]
         MgraColor(M_DEFAULT, M_COLOR_GREEN);
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth*11/18, imageHeight/5,    MIL_TEXT(" -------------------- "));
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth*11/18, imageHeight/5+25, MIL_TEXT(" - MIL Overlay Text - "));
         MgraText(M_DEFAULT, m_MilOverlayImage, imageWidth*11/18, imageHeight/5+50, MIL_TEXT(" -------------------- "));

         // Draw GDI color overlay annotation *
         //************************************

         // Disable hook to MIL error because control might not be supported
         MappControl(M_DEFAULT, M_ERROR_HOOKS, M_DISABLE);

#if M_MIL_USE_LINUX

         // Create a device context to draw in the overlay buffer with Cairo.  [CALL TO MIL]
         MbufControl(m_MilOverlayImage, M_SURFACE_ALLOC, M_COMPENSATION_ENABLE);

         // Reenable hook to MIL error
         MappControl(M_DEFAULT, M_ERROR_HOOKS, M_ENABLE);

         // Retrieve the cairo surface of the overlay [CALL TO MIL]
         cairo_surface_t *surface = (cairo_surface_t *)MbufInquire(m_MilOverlayImage, M_SURFACE_HANDLE, M_NULL);
         if (surface)
            {
            cairo_t *cr;
            MIL_TEXT_CHAR chText[80];
            cr = cairo_create (surface);
         
            cairo_set_source_rgb (cr, 0, 0, 1);
            // Draw a blue cross in the overlay buffer.
            cairo_move_to (cr, 0, imageHeight/2);
            cairo_line_to (cr, imageWidth, imageHeight/2);
            cairo_stroke (cr);
            cairo_move_to (cr, imageWidth/2, 0);
            cairo_line_to (cr, imageWidth/2, imageHeight);
            cairo_stroke (cr);
         
            // Write Red text in the overlay buffer.
            MosStrcpy(chText, 80, "X Overlay Text "); 
            cairo_set_source_rgb (cr, 1, 0, 0);
            cairo_set_font_size(cr, 13);
            cairo_move_to(cr, imageWidth*3/18, imageHeight*4/6);
            cairo_show_text(cr, chText);

            // Write Yellow text in the overlay buffer.
            cairo_set_source_rgb (cr, 1, 1, 0);
            cairo_set_font_size(cr, 13);
            cairo_move_to(cr,  imageWidth*12/18,imageHeight*4/6);
            cairo_show_text(cr, chText);
            cairo_destroy(cr);
            XFlush(QX11Info::display());
            XSync(QX11Info::display(),False);

            // Delete created cairo surface.  [CALL TO MIL]
            MbufControl(m_MilOverlayImage, M_SURFACE_FREE, M_DEFAULT);
            
            // Signal MIL that the overlay buffer was modified. [CALL TO MIL]
            MbufControl(m_MilOverlayImage, M_MODIFIED, M_DEFAULT);

            }

#else
         // Create a device context to draw in the overlay buffer with GDI.  [CALL TO MIL]
         MbufControl(m_MilOverlayImage, M_DC_ALLOC, M_DEFAULT);

         // Reenable hook to MIL error
         MappControl(M_DEFAULT, M_ERROR_HOOKS, M_ENABLE);

         // Retrieve the HDC of the overlay [CALL TO MIL]
         HDC OverlayDC = (HDC)MbufInquire(m_MilOverlayImage, M_DC_HANDLE, M_NULL);

         if(OverlayDC != M_NULL)
         {
             HGDIOBJ        hpen, hpenOld;
             POINT Hor[2];
             POINT Ver[2];
             SIZE  TxtSz;
             RECT  Txt;
             MIL_TEXT_CHAR  chText[80];
             int            Count;

             /* Draw a blue cross. */
             hpen=CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
             hpenOld = SelectObject(OverlayDC, hpen);

             Hor[0].x = (MIL_INT32)0;
             Hor[0].y = (MIL_INT32)(imageHeight/2);
             Hor[1].x = (MIL_INT32)imageWidth;
             Hor[1].y = (MIL_INT32)(imageHeight/2);
             Polyline(OverlayDC, Hor,2);

             Ver[0].x = (MIL_INT32)(imageWidth/2);
             Ver[0].y = (MIL_INT32)0;
             Ver[1].x = (MIL_INT32)(imageWidth/2);
             Ver[1].y = (MIL_INT32)imageHeight;
             Polyline(OverlayDC, Ver,2);

             SelectObject(OverlayDC, hpenOld);
             DeleteObject(hpen);

             /* Prepare transparent text annotations. */
             SetBkMode(OverlayDC, TRANSPARENT);
             MosStrcpy(chText, 80, MIL_TEXT("GDI Overlay Text"));
             Count = (int) MosStrlen(chText);
             GetTextExtentPoint(OverlayDC, chText, Count, &TxtSz);


             /* Write red text. */
             Txt.left = (MIL_INT32)(imageWidth*3/18);
             Txt.top  = (MIL_INT32)(imageHeight*17/24);
             Txt.right  = (MIL_INT32)(Txt.left + TxtSz.cx);
             Txt.bottom = (MIL_INT32)(Txt.top  + TxtSz.cy);
             SetTextColor(OverlayDC,RGB(255, 0, 0));
             DrawText(OverlayDC, chText, Count, &Txt, DT_RIGHT);

             /* Write yellow text. */
             Txt.left = (MIL_INT32)imageWidth*12/18;
             Txt.top  = (MIL_INT32)imageHeight*17/24;
             Txt.right  = (MIL_INT32)(Txt.left + TxtSz.cx);
             Txt.bottom = (MIL_INT32)(Txt.top  + TxtSz.cy);
             SetTextColor(OverlayDC, RGB(255, 255, 0));
             DrawText(OverlayDC, chText, Count, &Txt, DT_RIGHT);

             // Delete created device context.  [CALL TO MIL]
             MbufControl(m_MilOverlayImage, M_DC_FREE, M_DEFAULT);

             // Signal to MIL that the overlay buffer was modified. [CALL TO MIL]
             MbufControl(m_MilOverlayImage, M_MODIFIED, M_DEFAULT);
         }
#endif
         // Now that overlay buffer is correctly prepared, we can show it [CALL TO MIL]
         MdispControl(m_MilDisplay, M_OVERLAY_SHOW, M_ENABLE);

         // Overlay is now initialized
         m_isOverlayInitialized = true;
         }
      }
   }


void MdispQtView::RestrictCursor(bool on)
   {
   /////////////////////////////////////////////////////////////////////////
   // MIL: Write code that will be executed when 'Restrict Cursor' menu item is clicked
   /////////////////////////////////////////////////////////////////////////

   if(m_MilDisplay)
      {
      MdispControl(m_MilDisplay, M_RESTRICT_CURSOR,on?M_ENABLE:M_DISABLE);

      // Check if control worked correctly before considering it successful [CALL TO MIL]
      MdispInquire(m_MilDisplay, M_RESTRICT_CURSOR, &m_currentRestrictCursor);

      }
   }

bool MdispQtView::newDoc()
   {
   // Set buffer attributes
   if(((MdispQtApp*)qApp)->m_numberOfDigitizer)
   {
      m_bufferAttributes=M_IMAGE+M_DISP+M_GRAB+M_PROC;

      m_bufferAttributes=M_IMAGE+M_DISP+M_GRAB+M_PROC;
      m_imageSizeX = ((MdispQtApp*)qApp)->m_digitizerSizeX;
      m_imageSizeY = ((MdispQtApp*)qApp)->m_digitizerSizeY;
      m_NbBands    = ((MdispQtApp*)qApp)->m_digitizerNbBands;

      // Allocate a buffer [CALL TO MIL]
      MbufAllocColor(((MdispQtApp*)qApp)->m_MilSystem,
                     m_NbBands,
                     m_imageSizeX,
                     m_imageSizeY,
                     8+M_UNSIGNED,
                     m_bufferAttributes,
                     &m_MilImage);

      // Clear the buffer [CALL TO MIL]
      MbufClear(m_MilImage,M_COLOR_BLACK);
   }
   else
   {
      MbufImport(IMAGE_FILE,M_DEFAULT,M_RESTORE,((MdispQtApp*)qApp)->m_MilSystem,&m_MilImage);

      // Set SizeX and SizeY variable to the size of the buffer [CALL TO MIL]
      if (m_MilImage)
      {
         m_imageSizeX   = MbufInquire(m_MilImage, M_SIZE_X, M_NULL);
         m_imageSizeY   = MbufInquire(m_MilImage, M_SIZE_Y, M_NULL);
         m_NbBands      = MbufInquire(m_MilImage, M_SIZE_BAND, M_NULL);
      }
   }


   UpdateContentSize();

   // If not able to allocate a buffer, do not create a new document
   if(!m_MilImage)
      return false;

   Initialize();

   return true;
}

bool MdispQtView::load( const QString& fn )
   {
   //Import image in buffer [CALL TO MIL]

   MIL_TEXT_CHAR *txt = new MIL_TEXT_CHAR[ fn.length() + 1 ];
#if M_MIL_USE_LINUX
   QByteArray ba = fn.toLocal8Bit();
   const char *tmp = ba.data();
   strncpy( txt, tmp, fn.length() );
#else
   fn.toWCharArray(txt);
#endif
   txt[ fn.length() ] = 0;

   MbufImport(txt,M_DEFAULT,M_RESTORE,((MdispQtApp*)qApp)->m_MilSystem,&m_MilImage);
   delete[] txt;

   // Set SizeX and SizeY variable to the size of the buffer [CALL TO MIL]
   if (m_MilImage)
      {
      Initialize();
      m_imageSizeX = MbufInquire(m_MilImage,M_SIZE_X,M_NULL);
      m_imageSizeY = MbufInquire(m_MilImage,M_SIZE_Y,M_NULL);
      UpdateContentSize();

      m_Filename = QFileInfo(fn).fileName();
      m_FilenameValid = true;
      emit filenameChanged(m_Filename);
      return true;
      }
   else
      {
      return false;
      }
   }


bool MdispQtView::save()
   {
   if ( !m_FilenameValid )
      {
      return saveAs();
      }

   bool SaveStatus;

   // Halt the grab if the current view has it [CALL TO MIL]
   if((((MdispQtApp*)qApp)->m_pGrabView == this) &&
      (((MdispQtApp*)qApp)->m_isGrabStarted == true))
      MdigHalt(((MdispQtApp*)qApp)->m_MilDigitizer);

   // Save the current buffer [CALL TO MIL]
#if !UNICODE
   QByteArray ba = m_Filename.toLocal8Bit();
   const char *tmp = ba.data();
#else
   const wchar_t *tmp = (const wchar_t *) m_Filename.utf16();
#endif
   MbufExport(tmp, M_USE_EXTENSION, m_MilImage);

   // Verify if save operation was successful [CALL TO MIL]
   SaveStatus = (MappGetError(M_DEFAULT, M_CURRENT,M_NULL) == M_NULL_ERROR);

   // Document has been saved
   if (!((((MdispQtApp*)qApp)->m_pGrabView == this) &&
         (((MdispQtApp*)qApp)->m_isGrabStarted == true)))
      m_Modified = false;

   // Restart the grab if the current view had it [CALL TO MIL]
   if((((MdispQtApp*)qApp)->m_pGrabView == this) &&
     (((MdispQtApp*)qApp)->m_isGrabStarted == true))
     MdigGrabContinuous(((MdispQtApp*)qApp)->m_MilDigitizer, m_MilImage);

   return SaveStatus;
   }

bool MdispQtView::saveAs()
   {
   QString showName = strippedName(m_Filename);
   QString fn = QFileDialog::getSaveFileName(this, tr("Save File"),
                                             tr("%1").arg(showName),
                                             tr("Image Files (*.mim *.bmp *.tif *.jpg *.jp2 *.raw *.png)"));

   if ( !fn.isEmpty() )
      {
      m_Filename = fn;
      if (!m_Filename.contains('.'))
         {
         m_Filename+=".mim";
         }
      m_FilenameValid = true;
      emit filenameChanged(strippedName(m_Filename));
      return save();
      }
   else
      {
      return false;
     }
   }

const QString& MdispQtView::filename() const
   {
   return m_Filename;
   }


void MdispQtView::closeEvent( QCloseEvent* e )
   {
   if ( IsModified() )
      {
      switch ( QMessageBox::warning( this, tr("MdispQt Message"),
               tr("Save changes to %1?").arg(m_Filename),
               QMessageBox::Yes | QMessageBox::Default,
               QMessageBox::No,
               QMessageBox::Cancel | QMessageBox::Escape ) )
         {
         case QMessageBox::Yes:
            {
            if ( save() )
               {
               RemoveFromDisplay();
               e->accept();
               }
            else
               e->ignore();
            }
            break;
         case QMessageBox::No:
            {
            RemoveFromDisplay();
            e->accept();
            }
            break;
         default:
            e->ignore();
            break;
         }
      }
   else
      {
      RemoveFromDisplay();
      e->accept();
      }
   }

QSize MdispQtView::sizeHint() const
   {
   return QSize(width(), height());
   }

void MdispQtView::UpdateContentSize()
   {
   int sizeX, sizeY;
	if(m_MilDisplay)
		{
      MIL_DOUBLE ZoomX =1.0, ZoomY =1.0;
      MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_X, &ZoomX);
      MdispInquire(m_MilDisplay, M_ZOOM_FACTOR_Y, &ZoomY);
      if(ZoomX || ZoomY)
         {
         m_currentZoomFactorX = ZoomX;
         m_currentZoomFactorY = ZoomY;
         }
      }
   
   sizeX = int( m_imageSizeX * m_currentZoomFactorX );
   sizeY = int( m_imageSizeY * m_currentZoomFactorY );


   resize( sizeX, sizeY );
   emit sizeChanged((long)(sizeX) , (long)(sizeY));
   }


void MdispQtView::UpdateMousePosition()
   {
   emit mousePositionChanged(m_LastMousePosition.m_DisplayPositionX,
                             m_LastMousePosition.m_DisplayPositionY,
                             m_LastMousePosition.m_BufferPositionX,
                             m_LastMousePosition.m_BufferPositionY);
   // Reset mouse position
   m_LastMousePosition.Set(M_INVALID, M_INVALID, M_INVALID, M_INVALID);
   }

void MdispQtView::customEvent(QEvent* e)
   {
	if (e->type() == QEvent::User +8)
   {

      MilMouseEvent* re = (MilMouseEvent*)e;
      MOUSEPOSITION Pos = re->MousePostion();
      emit mousePositionChanged(Pos.m_DisplayPositionX,
                                Pos.m_DisplayPositionY,
                                Pos.m_BufferPositionX,
                                Pos.m_BufferPositionY);
      // Reset mouse position
      m_LastMousePosition.Set(M_INVALID, M_INVALID, M_INVALID, M_INVALID);
   }
}

void MdispQtView::SelectWindow()
   {
   //Select the buffer from its display object and given window [CALL TO MIL]
   if(m_MilDisplay && m_MilImage)
      {
#if M_MIL_USE_LINUX
      XColor xcolor,exact;
      if (getenv("QT_XCB_NO_XI2_MOUSE"))
         {
         XWindowAttributes attr;
         XGetWindowAttributes(QX11Info::display(),winId(),&attr);
         // Do not select mouse event, it will be done by the MIL Display
         XSelectInput(QX11Info::display(),winId(), attr.your_event_mask & NON_MOUSE_MASK);
         }
      XSetWindowBackground(QX11Info::display(), winId(), WhitePixel(QX11Info::display(),0));
      m_GC = XCreateGC(QX11Info::display(), winId(), 0, 0);
      XAllocNamedColor(QX11Info::display(),DefaultColormap(QX11Info::display(),0),"magenta", &xcolor,&exact);
      XSetForeground(QX11Info::display(),m_GC, xcolor.pixel);
      XFlush(QX11Info::display());
      XSync(QX11Info::display(),False);
#endif  

      MdispSelectWindow(m_MilDisplay, m_MilImage, (MIL_WINDOW_HANDLE)(m_isWindowed?winId():0));   
      }
   }
#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_mdispqtview.cpp"
#endif
