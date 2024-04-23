/***************************************************************************************/
/*
 * File name: WebClientWin.cpp
 *
 * Synopsis:  This program shows how to use web publishing.
 *
 *
 *
 * Copyright Â© Matrox Electronic Systems Ltd., 1992-2023.
 * All Rights Reserved
 */
#include "resource.h"
#define MAX_LOADSTRING 100
#include "webclient.h"

MilWebWindow gMainWindow = { NULL, NULL, NULL, NULL };
HINSTANCE hInst;								// current instance

/* Get Message object data */
void DisplayMessage(MIL_UINT8 *MsgData, MIL_INT MsgLength, MIL_INT64 MsgTag, void *UserData)
   {
   MilWebWindow *MainWindow = (MilWebWindow *)UserData;
   if (MainWindow && MsgData && MsgLength > 0)
      {
      /* do whatever with data*/
      }
   }

/* Get Display image  object data */
/* display it in the window       */
void DisplayImage(MIL_UINT8 *Data, MIL_INT SizeByte, MIL_INT SizeX, MIL_INT SizeY, MIL_INT PitchByte, void *UserData)
   {
   MilWebWindow *MainWindow = (MilWebWindow *)UserData;
   if (MainWindow && Data && SizeByte > 0 && SizeX > 0 && SizeY > 0)
      {
      if (!MainWindow->image_data)
         {
         MainWindow->image_data = new MIL_UINT8[SizeX *SizeY * 4];
         MainWindow->MemDC = CreateCompatibleDC(NULL);
         SetWindowPos(MainWindow->Window, 0, 0, 0, (int)SizeX, (int)SizeY, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        
         BITMAPINFOHEADER   bmih;
         memset(&bmih, 0, sizeof(BITMAPINFOHEADER));

         bmih.biWidth = (LONG)SizeX;
         bmih.biHeight = (LONG)-SizeY;
         bmih.biBitCount = 32;
         bmih.biCompression = BI_RGB;
         bmih.biSize = sizeof(BITMAPINFOHEADER);
         bmih.biPlanes = 1;

         BITMAPINFO* bmi = (BITMAPINFO*)&bmih;

        MainWindow->hbmp = CreateDIBitmap(GetDC(MainWindow->Window), &bmih, CBM_INIT, (BYTE*)MainWindow->image_data, bmi, DIB_RGB_COLORS);
         }

      MIL_UINT8 const* Src = Data;
      MIL_UINT8* Dst = MainWindow->image_data;
      memcpy(Dst, Src, SizeByte);
      SetBitmapBits(MainWindow->hbmp, (DWORD)SizeByte, MainWindow->image_data);
      InvalidateRect(MainWindow->Window, NULL, false);
     
      }
   }

// Global Variables:

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(
					  __in HINSTANCE hInstance,
                     __in_opt HINSTANCE hPrevInstance,
                     __in LPTSTR    lpCmdLine,
                     __in int       nCmdShow
					 )
{
   MIL_ID AppId = M_NULL;
   MIL_TEXT_PTR Url = MILWEB_URL;
   if (__argc > 1)
      Url = __targv[1];
   AppId = StartConnection(&gMainWindow, Url);
   UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WEBCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	

	// Main message loop:
   /* Get and dispatch messages until a WM_QUIT message is received. */
   while (GetMessage(&msg, (HWND)NULL, (UINT)NULL, (UINT)NULL))
      {
      TranslateMessage(&msg);   /* Translates virtual key codes. */
      DispatchMessage(&msg);    /* Dispatches message to window. */
      }
   EndConnection(AppId);
   if (gMainWindow.image_data)
      delete[] gMainWindow.image_data;

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WEBCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WEBCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   gMainWindow.Window = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
   case WM_PAINT:
         {
         if (gMainWindow.hbmp)
            {
            hdc = BeginPaint(hWnd, &ps);
            BITMAP 			bitmap;
            HDC hdcMem = CreateCompatibleDC(hdc);
            HGDIOBJ oldBitmap = SelectObject(hdcMem, gMainWindow.hbmp);
            GetObject(gMainWindow.hbmp, sizeof(bitmap), &bitmap);
            BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, oldBitmap);
            DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
            }
         else
            {
            hdc = BeginPaint(hWnd, &ps);
            TextOut(hdc, 10, 50, MIL_TEXT("Not connected."), 14);
            EndPaint(hWnd, &ps);
            }
         }
		break;
	case WM_DESTROY:
      DeleteObject(gMainWindow.MemDC);
      DeleteObject(gMainWindow.hbmp);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
