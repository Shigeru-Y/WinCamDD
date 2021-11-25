// usbcam_tebure.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "ewclib.h"

#define CLASS
#include "usbCamera.h"
#include "DDconfig.h"

extern CWnd* controlWnd;
extern double actualFPS;

//
// Main loop of USB CAMERA
//
DWORD WINAPI setup_usbcam( LPVOID data )
{
	int i, status;
	clock_t sTime, eTime;

	xReso = 1280;
	yReso = 720;
//	xReso = 1280;
//	yReso = 1024;
	cameraNumber = 0;
	nextCamera = 0;
	trapezoidCompensation = 0;
	fEnableImageProc = FALSE;		// Disable image processing.

	// Definition of Main display window.
	WNDCLASSEX	wc;					// New window class
	memset( &wc, 0, sizeof(WNDCLASSEX) );
	wc.cbSize        = sizeof(WNDCLASSEX);	
	wc.lpfnWndProc   = grProc;				// Window procedure
	// For x64 correspondence.
	// wc.hInstance     = (HINSTANCE)GetWindowLong( HWND_DESKTOP, GWL_HINSTANCE ); 
	wc.hInstance = (HINSTANCE)GetWindowLongPtr(HWND_DESKTOP, GWLP_HINSTANCE);
	wc.lpszClassName = _T("GRC0");				// Name of window
	wc.cbWndExtra    = 10;					// allocate pointer area for related structure.
	RegisterClassEx( &wc );			// Register window class
	
	// Start capture
	// Create display window
	DWORD tid;

	img00.hwnd = NULL;
	img00.bih.biSize = 40;
	img00.bih.biWidth = xReso;
	img00.bih.biHeight = yReso;
	img00.bih.biBitCount = 24;
	img00.bih.biPlanes = 1;
	img00.bih.biSizeImage = xReso * yReso * 3;

	// 2021/Nov/20 S.Yamamoto
	// DD process shows original image.
	procMode = 0;
//	procMode = 5;		// Show trapezoid compensation.
//	procMode = MAX_PROC_IMAGES;

//	cursorType = CURSOR_NON;	// Default no cursor.
	cursorType = CURSOR_TYPE1;	// Default type 1.
	key = 0;					// Reset key input.

	imageStill = FALSE;			// Image is moving.
	snapTimer  = 0;				// Stop timer.
	printingFlag = FALSE;		// Not printing.

	// Allocate image buffer.
	img00.lpBmpData = (BYTE *)malloc( xReso * yReso * 3 );
	for( i = 0 ; i < MAX_PROC_IMAGES * 2 ; i++ ){
		proc[i] = img00;		// Copy contents of structure.
		proc[i].lpBmpData = (BYTE *)malloc( xReso * yReso * 3 );
		memset( proc[i].lpBmpData, 0, xReso * yReso * 3 );
	}

	if( f_simulate == 0 ){
		// No simulation mode. Start camera.
		if( EWC_Open(cameraNumber, xReso, yReso, fps, -1, MEDIASUBTYPE_RGB24) != 0 ){
			// Open error of cameraNumber==0.
			// Try to open second camera.
			cameraNumber = 1;
			nextCamera = 1;
			if (EWC_Open(cameraNumber, xReso, yReso, fps, -1, MEDIASUBTYPE_RGB24) != 0) {
				// Open error. Might be caused by no USB camera implemented.
				free(img00.lpBmpData);
				for (i = 0; i < MAX_PROC_IMAGES * 2; i++) {
					free(proc[i].lpBmpData);
				}
				return FALSE;
			}
		}
		EWC_RawMode(cameraNumber, 1);	// Set up/down
	}
	// Succeed to open. Set up necessary parameters.

//	USBcameraParamSet( cameraNumber );

	CreateThread( NULL, 0,
				 (LPTHREAD_START_ROUTINE)th_Proc,
				 NULL, 0, &tid );
											// Start thread for message loop.
//	while( !(img00.hwnd) );					// Wait for window display.

	fMessage = FALSE;		// Output message.
//	fImageAvarage = FALSE;
	fImageAvarage = TRUE;
	fDynamicColor = TRUE;

	// Test. Background image.
//	JPEGRead( "C:\\Users\\yamamo-s\\Pictures\\Sight\\DoctorYellow.jpg",
//		&proc[9] );
	
	//// Set window transparent on RGB(0,0,0)
	//LONG exStyle = GetWindowLong( img00.hwnd, GWL_EXSTYLE );
 //   SetWindowLong( img00.hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
	//SetLayeredWindowAttributes( img00.hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	sTime = clock();	// Start time measurement for FPS calculation.

	while(1){
		// Infinite loop.
		//if( kbhit() ){
		//	key = getch();
		if( key ){
			if( key == 'a' ){
				fImageAvarage = ~fImageAvarage;
			} else
			if( key == 'q' ){
				break;		// Quit loop.
			} else if( key == 'p' ){
				// Change process.
				if( ++procMode > MAX_PROC_IMAGES ){
					procMode = 0;
				}
			} else if( key == 'c' ){
				// Camera control
				status = EWC_PropertyPage( cameraNumber );
			} else if( key == 't' ){
				// Trapezoid compensation
				if( ++trapezoidCompensation > MAX_TRAPEZOID ){
					trapezoidCompensation = 0;
				}
				printf( "Trapezoid compensation = %d\n", trapezoidCompensation );
			} else if( key == '-' ){
				// Trapezoid compensation
				if( --trapezoidCompensation < 0 ){
					trapezoidCompensation = MAX_TRAPEZOID - 1;
				}
				printf( "Trapezoid compensation = %d\n", trapezoidCompensation );
			} else if( key == 'v' ){
				// Unti-vibration
				fVibration = ~fVibration;

			} else if( key == 'm' ){
				// Message switch.
				fMessage = ~fMessage;
				if( fMessage ){
					printf("Message ON\n" );
				} else {
					printf("Message OFF\n" );
				}
			}
			key = 0;
		}

		// Check changing of camera number.
		if (cameraNumber != nextCamera) {
			// Switch camera.
			EWC_Close(cameraNumber);
			cameraNumber = nextCamera;
			EWC_Open(cameraNumber, xReso, yReso, fps, -1, MEDIASUBTYPE_RGB24);
			EWC_RawMode(cameraNumber, 1);	// Set up/down
		}
		if( f_simulate == 0 ){
			// No simulation mode.
			if( EWC_GetImage(cameraNumber, img00.lpBmpData) != 0 ){
				// No image captured.
				Sleep(1);
	//			continue;
			} else {
				// Image captured.
				// Image processing.
//				if( imageStill == FALSE ){
//					ImagePreProc();
//				}
				ImageProc();

				// Message
				//if( defect[0].fDefect ){
				//	printf(" Find Blue target at (%d,%d)-(%d,%d)\n",  defect[0].dfMinX, defect[0].dfMaxX, defect[0].dfMinY, defect[0].dfMaxY );
				//}
				//if( defect[1].fDefect ){
				//	printf(" Find Green target at (%d,%d)-(%d,%d)\n",  defect[1].dfMinX, defect[1].dfMaxX, defect[1].dfMinY, defect[1].dfMaxY );
				//}
				//if( defect[2].fDefect ){
				//	printf(" Find Red target at (%d,%d)-(%d,%d)\n",  defect[2].dfMinX, defect[2].dfMaxX, defect[2].dfMinY, defect[2].dfMaxY );
				//}

				// Measure FPS
				if (++frameCounter >= 10) {
					// Processed 100 frames.
					eTime = clock();
					actualFPS = 10. / ((double)(eTime - sTime) / CLOCKS_PER_SEC);
					sTime = clock();
					frameCounter = 0;
				}

				// Refresh drawing.
				InvalidateRect( img00.hwnd, NULL, FALSE);

			}		
		} else {
			// Camera Simulation mode.
			Sleep(30);
			// Refresh.
			InvalidateRect( img00.hwnd, NULL, FALSE);
		}
	}

	// Finish up image processes.
	fApsRunning = FALSE;		// Indicate application stops.
	Sleep( 100 );

	// Clean up
	free( img00.lpBmpData );
	for( i = 0 ; i < MAX_PROC_IMAGES * 2 ; i++ ){
		free( proc[i].lpBmpData );
	}

	// Close camera
	if( f_simulate == 0 ){
		EWC_Close(cameraNumber);
	}

	return (DWORD)0;
}

//
// Start thread for USB camera handling
//
BOOL USBCAMstart( void )
{
	//// Start automatic operation
	//hUSBCAMhalfClockerThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)jai_vga_usbcam_halfclocker, (LPVOID)NULL, 0, &thUSBCAMhalfClockerId );
	// Start USB camera thread.
	hUSBCAMthread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)setup_usbcam, (LPVOID)NULL, 0, &thUSBCAMid );
	if( hUSBCAMthread == NULL ){
		return FALSE;
	} else {
		return TRUE;
	}
}

// Handle for second monitor
HMONITOR hSecondMonitor;
RECT     rcSecondMonitor;
int      captWindowXsize, captWindowYsize;

// Second monitor enumlation
BOOL CALLBACK secondMonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	// HPEN hOldPen;

	// Get parameter of second display
	hSecondMonitor = hMonitor;
	rcSecondMonitor = *lprcMonitor;
	captWindowXsize = rcSecondMonitor.right - rcSecondMonitor.left;
	captWindowYsize = rcSecondMonitor.bottom - rcSecondMonitor.top;

	// Move capture window to fit for second monitor
//	m_pCaptWnd->MoveWindow( rcSecondMonitor.left, rcSecondMonitor.top, captWindowXsize, captWindowYsize );

	// Test draw
	//hOldPen = (HPEN)SelectObject( hdcMonitor, GetStockObject( BLACK_PEN ));
	//MoveToEx( hdcMonitor, 0, 0, NULL );
	//LineTo( hdcMonitor, 1000, 1000 );
	//SelectObject( hdcMonitor, hOldPen );

	return TRUE;
}


//
// Enumlate second monitor and move window to the second.
//
BOOL MoveWindowToTheSecondMonitor( HWND hWnd )
{
	HDC hdcVertualMonitor;
	RECT rcClip;
	// RECT rcControl;
	int imageWindowPosX, imageWindowPosY;
	BOOL rStatus;

	hSecondMonitor = NULL;
	hdcVertualMonitor = GetDC( NULL );

	// Set clipping area to left of main screen.
	rcClip.top = -10000;
	rcClip.bottom = 10000;
	rcClip.left = -10000;
	rcClip.right = 0;

	// Invoke call back proc to enumlate monitor screen
	EnumDisplayMonitors( hdcVertualMonitor, &rcClip, secondMonitorEnumProc, 0 );

	// Check if second monitor found
	if( hSecondMonitor == NULL ){
		// No monitor at left of main monitor. Check right.
		rcClip.left = GetSystemMetrics( SM_CXFULLSCREEN );
		rcClip.right = GetSystemMetrics( SM_CXFULLSCREEN ) + 10000;
		// Invoke call back proc to enumlate monitor screen
		EnumDisplayMonitors( hdcVertualMonitor, &rcClip, secondMonitorEnumProc, 0 );
	}
	ReleaseDC( NULL, hdcVertualMonitor );

	// Post process
	if( hSecondMonitor != NULL ){
		imageWindowPosX = rcSecondMonitor.left;
		imageWindowPosY = rcSecondMonitor.top;
		rStatus = TRUE;
	} else {
		// Move to top left.
		imageWindowPosX = 0;
		imageWindowPosY = 0;
		rStatus = FALSE;
	}

	MoveWindow( hWnd, imageWindowPosX, imageWindowPosY, img00.bih.biWidth, img00.bih.biHeight, TRUE );
//	if( hSecondMonitor != NULL ){
//		// Second monitor found. Move control dialog to the second monitor.
		//controlWnd->GetWindowRect( &rcControl );
		//controlWnd->MoveWindow( imageWindowPosX, imageWindowPosY + img00.bih.biHeight + 5, 
		//	rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, TRUE );
//	}
	return rStatus;
}

// Double buffering in order to avoid text flushing.
HBITMAP hBitmap;
HDC     hdcMem;

DWORD th_Proc( void )
{
	MSG msg;

	int sm0 = GetSystemMetrics( SM_CYCAPTION );
	int sm1 = GetSystemMetrics( SM_CXDLGFRAME );	// WS_OVRELAPPの場合、枠の太さは
	int sm2 = GetSystemMetrics( SM_CYDLGFRAME );	// SM_C?DLGFRAMEになる
	// 必ずスレッドの中でウィンドウを作る
	//img00.hwnd = CreateWindow( _T("GRC0"),	// クラスの名前
	//			_T("..."),
	//			WS_OVERLAPPED | WS_VISIBLE,			// ウィンドウの属性
	//			100, 100,		// 表示位置
	//			img00.bih.biWidth  + sm1 * 2, 	// 描画サイズから大きさを計算
	//			img00.bih.biHeight + sm0 + sm2 * 2,
	//			HWND_DESKTOP,						// 親はディスクトップ
	//			NULL, img00.hi, NULL 	     );					


	img00.hwnd = CreateWindowEx( WS_EX_COMPOSITED,
		_T("GRC0"),	// クラスの名前
		_T("..."),
		WS_OVERLAPPED | WS_VISIBLE,			// ウィンドウの属性
		100, 100,		// 表示位置
		img00.bih.biWidth + sm1 * 2, 	// 描画サイズから大きさを計算
		img00.bih.biHeight + sm0 + sm2 * 2,
		HWND_DESKTOP,						// 親はディスクトップ
		NULL, img00.hi, NULL);

	HDC hdc;
	RECT rc;
	hdc = GetDC(img00.hwnd);                      	// ウインドウのDCを取得
	GetClientRect(GetDesktopWindow(), &rc);  	// デスクトップのサイズを取得
	hBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	hdcMem = CreateCompatibleDC(NULL);		// カレントスクリーン互換
	SelectObject(hdcMem, hBitmap);		// MDCにビットマップを割り付け

	// Enumlate second monitor and move this window to the second monitor.
	MoveWindowToTheSecondMonitor( img00.hwnd );

	while( GetMessage( &msg, NULL, 0, 0 ) && fApsRunning ){
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	return 0;
}

// Prepare font
HFONT SetMyFont(HDC hdc, LPCTSTR face, int h, int angle)
{
    HFONT hFont;
    hFont = CreateFont(h,    //フォント高さ
        0,                    //文字幅
        angle,                    //テキストの角度
        0,                    //ベースラインとｘ軸との角度
        FW_REGULAR,            //フォントの重さ（太さ）
        FALSE,                //イタリック体
        FALSE,                //アンダーライン
        FALSE,                //打ち消し線
        SHIFTJIS_CHARSET,    //文字セット
        OUT_DEFAULT_PRECIS,    //出力精度
        CLIP_DEFAULT_PRECIS,//クリッピング精度
        PROOF_QUALITY,        //出力品質
        FIXED_PITCH | FF_MODERN,//ピッチとファミリー
        face);    //書体名
    return hFont;
}

// To report graylevel.
int grayReportX = 0;
int grayReportY = 0;

//　ウィンドウプロシージャ，再描画のみを行う
LRESULT CALLBACK grProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	PAINTSTRUCT	ps;
	HPEN rPen, gPen, oldPen;
	HBRUSH oldBrush;
	COLORREF rgb;
	int r, g, b;
	int i;
	TCHAR buf[256];

	int xPos, yPos;
	static int mouseDrag = FALSE;
	static int xPosLast, yPosLast;

    switch (msg) {

	case WM_LBUTTONDOWN:
		xPos = GET_X_LPARAM(lp); 
		yPos = GET_Y_LPARAM(lp); 
		mouseDrag = TRUE;
		xPosLast = xPos;
		yPosLast = yPos;
		break;

	case WM_LBUTTONUP:
		mouseDrag = FALSE;
		if( procMode != 5 ){
			// Get image gray level and report.
			grayReportX = GET_X_LPARAM(lp); 
			grayReportY = GET_Y_LPARAM(lp); 
		}
		break;

	case WM_MOUSEMOVE:
		//if( mouseDrag ){
		//	xPos = GET_X_LPARAM(lp); 
		//	yPos = GET_Y_LPARAM(lp); 
		//	DragOnImage( xPosLast, img00.bih.biHeight - yPosLast - 1, xPos, img00.bih.biHeight - yPos - 1 );
		//	xPosLast = xPos;
		//	yPosLast = yPos;
		//}
		break;

	case WM_PAINT:								// ビットマップの描画
		// BeginPaint( hwnd, &ps );
		if( procMode == 0 ){
			SetDIBitsToDevice(hdcMem, 0, 0,			// コピー先x,y座標
					img00.bih.biWidth,			// DIBの幅
					img00.bih.biHeight,			// DIBの高さ
					0, 0,							// DIBの座標 
					0,								// 走査線
					img00.bih.biHeight,			// 走査線数
					img00.lpBmpData,
					(BITMAPINFO *)&( img00.bih),	// BITMAPINFOにキャスト
					DIB_RGB_COLORS );
			swprintf_s( buf, 256, _T("Org") );
			TextOut(hdcMem, 10, 10, buf, (int)wcslen(buf));
		} else {
			SetDIBitsToDevice(hdcMem, 0, 0,			// コピー先x,y座標
					img00.bih.biWidth,			// DIBの幅
					img00.bih.biHeight,			// DIBの高さ
					0, 0,							// DIBの座標 
					0,								// 走査線
					img00.bih.biHeight,			// 走査線数
					proc[procMode-1].lpBmpData,
					(BITMAPINFO *)&( img00.bih),	// BITMAPINFOにキャスト
					DIB_RGB_COLORS );
			
			if( procMode == 15 ){		// If mirror image.
				swprintf_s( buf, 256, _T("ミラー") );
				TextOut(hdcMem, 600, 50, buf, (int)wcslen(buf));

			} else 
			if( procMode != 5 ){
				// Image for debug
				swprintf_s( buf, 256, _T("Proc %d"), procMode );
				TextOut(hdcMem, 10, 10, buf, (int)wcslen(buf));

				// Gray level report
				if( grayReportX != 0 || grayReportY != 0 ){
					rgb = GetPixel( hdcMem, grayReportX, grayReportY );
					r = (rgb & 0x0000ff);
					g = ((rgb >> 8) & 0x0000ff);
					b = ((rgb >> 16) & 0x0000ff);
					swprintf_s( buf, 256, _T("R=%d G=%d B=%d"), r, g, b );
					TextOut(hdcMem, 10, 30, buf, (int)wcslen(buf));
					swprintf_s( buf, 256, _T("GR=%d     GB=%d"), g * 100 / (r+1), g * 100 / (b+1) );
					TextOut(hdcMem, 10, 50, buf, (int)wcslen(buf));
				}

			}

			// Snap shot timer.
			if( snapTimer ){
				HFONT hFont, hFontOld;
				
				// Decrement tiemr.
				if( --snapSubTimer == 0 ){
					snapSubTimer = FRAMES_PER_SEC;		// Reset sub-timer.
					if( --snapTimer == 0 ){
						imageStill = TRUE;				// Still image.
						if( procMode == 15 ){		// if mirror image.
							procMode = 5;			// Switch back to normal display.
						}
					}
				}

				SetBkMode(hdcMem, TRANSPARENT);
				SetTextColor(hdcMem, RGB(128, 255, 128));

				hFont = SetMyFont(hdcMem, (LPCTSTR)"ＭＳ ゴシック", 80, 0);
				hFontOld = (HFONT)SelectObject(hdcMem, hFont);
				swprintf_s( buf, 256, _T("%d"), snapTimer );
				TextOut(hdcMem, 600, 100, buf, (int)wcslen(buf));

				SelectObject(hdcMem, hFontOld);
				DeleteObject( hFont );
			}

			// Printing timer.
			if( printingFlag ){
				HFONT hFont, hFontOld;
				
				// Decrement timer.
				if( --snapSubTimer < 0 ){
					snapSubTimer = FRAMES_PER_SEC;		// Reset sub-timer.
				} else if( snapSubTimer > FRAMES_PER_SEC / 3 ){
					SetBkMode(hdcMem, TRANSPARENT);
					SetTextColor(hdcMem, RGB(255, 128, 255));

					hFont = SetMyFont(hdcMem, (LPCTSTR)"ＭＳ ゴシック", 80, 0);
					hFontOld = (HFONT)SelectObject(hdcMem, hFont);
					swprintf_s( buf, 256, _T("プリント中") );
					TextOut(hdcMem, 500, 100, buf, (int)wcslen(buf));

					SelectObject(hdcMem, hFontOld);
					DeleteObject( hFont );
				}
			}

		}

		// For DD
		// Draw detected defects.
		if (procMode < 2) {
			// Output FPS
			swprintf_s(buf, 256, _T("%4.1f FPS"), actualFPS);
			TextOut(hdcMem, xReso - 200, 10, buf, wcslen(buf));

			// Draw defect
			rPen = CreatePen(PS_DASH, 1, RGB(255, 0, 0));
			gPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
			oldPen = (HPEN)SelectObject(hdcMem, rPen);
			oldBrush = (HBRUSH)SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
			for (i = 0; i < defectBottom; i++) {
				SelectObject(hdcMem, rPen);
				if (defects[i].fDefect) {
					// Defect found.
					swprintf_s(buf, 256, _T("*** Defect Found (%d,%d)-(%d,%d) ***"),
						defects[i].dfMinX - 1, defects[i].dfMinY - 1, defects[i].dfMaxX + 1, defects[i].dfMaxY + 1);
					TextOut(hdcMem, 10, 10 + i * 30, buf, wcslen(buf));
					// Draw rect
					Rectangle(hdcMem, defects[i].dfMinX - 4, defects[i].dfMinY - 4, defects[i].dfMaxX + 4, defects[i].dfMaxY + 4);
					Rectangle(hdcMem, defects[i].dfMinX - 5, defects[i].dfMinY - 5, defects[i].dfMaxX + 5, defects[i].dfMaxY + 5);
					//					Rectangle( ps.hdc, defects[i].dfMinX, defects[i].dfMinY, defects[i].dfMaxX, defects[i].dfMaxY );
					//					Rectangle( ps.hdc, defects[i].dfMinX, defects[i].dfMinY, defects[i].dfMaxX, defects[i].dfMaxY );
				}
				else {
					swprintf_s(buf, 256, _T("!!! No Defect !!!"));
					TextOut(hdcMem, 10, 10, buf, wcslen(buf));
				}

				// Draw defect size.
				SelectObject(hdcMem, gPen);
				buf[0] = 0;		// Clear message
				size_t ret;

				if (defects[i].attribute[0] != 0) {
					// Some attribute set.
					mbstowcs_s(&ret, buf, 100, defects[i].attribute, _TRUNCATE);
					// swprintf_s(buf, 256, defects[i].attribute[0]);
				}
				else {
					if (defects[i].pixels > BIG_DEFECT) {
						wcscat_s(buf, 256, _T("大きくて"));
					}
					if (defects[i].positive >= defects[i].pixels / 2) {
						// Positive defect
						wcscat_s(buf, 256, _T("白い"));
					}
					else {
						wcscat_s(buf, 256, _T("黒い"));
					}
				}
				TextOut(hdcMem, defects[i].dfMaxX + 20, defects[i].dfMaxY + 20, buf, wcslen(buf));

			}

			// Draw distance.
			SelectObject(hdcMem, gPen);
			// Right vertical line.
			MoveToEx(hdcMem, xReso - 100, 30, NULL);
			LineTo(hdcMem, xReso - 100, 40);
			// Arrow head.
			MoveToEx(hdcMem, xReso - 130, 30, NULL);
			LineTo(hdcMem, xReso - 100, 35);
			LineTo(hdcMem, xReso - 130, 40);
			// Horizontal line.
			MoveToEx(hdcMem, xReso - 100, 35, NULL);
			LineTo(hdcMem, xReso - 100 - distMode, 35);
			// Vertical line.
			MoveToEx(hdcMem, xReso - 100 - distMode, 30, NULL);
			LineTo(hdcMem, xReso - 100 - distMode, 40);

			// Restore drawing objects.
			SelectObject(hdcMem, oldPen);
			SelectObject(hdcMem, oldBrush);
			DeleteObject(gPen);
			DeleteObject(rPen);
		}
		else {
			swprintf_s(buf, 256, _T("Proc %d"), procMode);
			TextOut(hdcMem, 10, 10, buf, wcslen(buf));

			if (procMode == 2) {
				// Display binarize
				swprintf_s(buf, 256, _T("xRep %d"), distMode);
				TextOut(hdcMem, 10, 30, buf, wcslen(buf));
			}
		}

		// Copy from memory DC.
		BeginPaint(hwnd, &ps);
		BitBlt(ps.hdc, 0, 0, img00.bih.biWidth,
			img00.bih.biHeight, hdcMem, 0, 0, SRCCOPY);
        EndPaint( hwnd, &ps );
		return 0;

	case WM_DESTROY:							// スレッドの
		PostQuitMessage( 0 );					// メッセージループを終了させる
		break;			

	default:
		return DefWindowProc( hwnd, msg, wp, lp );
	}
	return 0;
}

//
// Utility for camera control.
//

// Select camera number.
// Close current camera then open new camera.
BOOL CAM_SelectCamera(int newCam)
{
	if (newCam != cameraNumber) {
		//EWC_Close(cameraNumber);
		//cameraNumber = newCam;
		//EWC_Open(cameraNumber, xReso, yReso, fps, -1, MEDIASUBTYPE_RGB24);

		nextCamera = newCam;	// Switch camera in processing loop.
	}
	return TRUE;
}

// Set camera focus.
// input pos - Percent of focus positino. 0-100 
int lastFocus = 0;
BOOL CAM_SetFocus(int pos)
{
	double focusValue = (double)pos;

	if (lastFocus != pos) {
		//		EWC_SetManual(cameraNumber, EWC_FOCUS);
		EWC_SetValue(cameraNumber, EWC_FOCUS, focusValue);
		lastFocus = pos;
	}
	return TRUE;
}
