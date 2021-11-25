// usbCamera.h : Definition of usb camera
//
//

// System flags.
CLASS int f_simulate;

// For thread creation
CLASS HANDLE  hUSBCAMthread;
CLASS DWORD   thUSBCAMid;
CLASS int     fUSBCAMquit;
CLASS int     fUSBCAMend;


// グラフィック用ウィンドウのデータをまとめた構造体
typedef struct{				
	HINSTANCE			hi;
	HWND				hwnd;		// 自分のウィンドウハンドル
	BYTE				*lpBmpData; // BMPのデータ部分
	BITMAPINFOHEADER	bih;
} IMG0;

#define MAX_PROC_IMAGES 10

CLASS IMG0	img00;
CLASS IMG0	proc[MAX_PROC_IMAGES * 2];
CLASS int	procMode;
CLASS int	imageZoom;

CLASS int xReso, yReso;
CLASS int cameraNumber, nextCamera;
CLASS int trapezoidCompensation;
CLASS double fps, actualFPS;
CLASS int frameCounter;
CLASS int key;

#define CURSOR_NON     0
#define CURSOR_TYPE1   1
#define CURSOR_TYPE2   2
#define CURSOR_END     3

CLASS int cursorType;

#define MAX_DEFECTS 10
#define CLUSTER_SIZE 100
#define MIN_DEFECT_PIXELS 10
#define BIG_DEFECT   160

#define MAX_TRAPEZOID  50

//
// Unti vibration
//
#define EDGE_DIST  4
#define EDGE_MARGIN 10
#define CORR_AREA  320
#define CORR_SIZE  100
#define CORR_DIST   10

CLASS int xOffset, yOffset;
CLASS BOOL fVibration;
CLASS BOOL fMessage;

CLASS BOOL fApsRunning;		// Application Running Flag

CLASS int imageStill;		// Image still.
CLASS int snapTimer;		// Snap shot timer.
CLASS int snapSubTimer;		// sub timer per frame.
CLASS int printingFlag;
CLASS int processingFlag;
CLASS BOOL fEnableImageProc;	// Enable flag.
CLASS BOOL fImageAvarage;		// Do averaging.
CLASS BOOL fDynamicColor;

CLASS double ratioGR, ratioGB;
CLASS double minRatioGR, maxRatioGR, minRatioGB, maxRatioGB;

#define FRAMES_PER_SEC 20

DWORD th_Proc( void );
LRESULT CALLBACK grProc( HWND, UINT, WPARAM, LPARAM );
		// Image processing
BOOL ImagePreProc( void );
BOOL ImageProc( void );
BOOL GeneratePrintImage( int nImages, char *cfile );
BOOL GeneratePrintImageToDefaultPrinter( int nImages, char *cfile );
LPDEVMODE GetDevMode_LandscapeA4(LPTSTR pPrinterName);
BOOL DragSetColor( int r, int g, int b );
BOOL DragErase( void );
BOOL DragOnImage( int xs, int ys, int xe, int ye );
BOOL ConvertProcTo128Button( IMG0 *src, HWND hButtonWnd, HBITMAP hBitmap );
BOOL IMP_StretchImage( IMG0 *src, IMG0 *dest, int sizeX, int sizeY );
BOOL IMP_ImagePaint( IMG0 *dest, int r, int g, int b );
BOOL IMP_ImageShrinkCopy1_16x( IMG0 *src, IMG0 *dest, int ofsX, int ofsY );
BOOL PrintToDefaultPrinter( IMG0 *src, TCHAR *message = NULL, int mesX = 0, int mesY = 0 );

// JPEG relating.
int JPEGRead( char *jpegname, IMG0 *dest_image );
int JPEGWrite( char *jpegname, IMG0 *src_image, int quality );

// Camera control
BOOL CAM_SelectCamera(int newCam);
BOOL CAM_SetFocus(int pos);

// QR code related.
int IMP_readQR(IMG0* src, char *attr);
