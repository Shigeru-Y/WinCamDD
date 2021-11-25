
// WinCamDDDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "WinCamDD.h"
#include "WinCamDDDlg.h"
#include "afxdialogex.h"

#define CLASS extern
#include "usbCamera.h"
#include "DDconfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWinCamDDDlg ダイアログ



CWinCamDDDlg::CWinCamDDDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WINCAMDD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWinCamDDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DD_THRESH, paramThreshold);
	DDX_Control(pDX, IDC_CAMERA_FOCUS, paramFocus);
}

BEGIN_MESSAGE_MAP(CWinCamDDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PROC0, &CWinCamDDDlg::OnBnClickedProc0)
	ON_BN_CLICKED(IDC_PROC1, &CWinCamDDDlg::OnBnClickedProc1)
	ON_BN_CLICKED(IDC_PROC2, &CWinCamDDDlg::OnBnClickedProc2)
	ON_BN_CLICKED(IDC_PROC3, &CWinCamDDDlg::OnBnClickedProc3)
	ON_BN_CLICKED(IDC_PROC4, &CWinCamDDDlg::OnBnClickedProc4)
	ON_BN_CLICKED(IDC_PROC5, &CWinCamDDDlg::OnBnClickedProc5)
	ON_BN_CLICKED(IDC_PROC6, &CWinCamDDDlg::OnBnClickedProc6)
	ON_BN_CLICKED(IDC_SHRINK2, &CWinCamDDDlg::OnBnClickedShrink2)
	ON_BN_CLICKED(IDC_SHRINK3, &CWinCamDDDlg::OnBnClickedShrink3)
	ON_BN_CLICKED(IDC_SHRINK1, &CWinCamDDDlg::OnBnClickedShrink1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DD_THRESH, &CWinCamDDDlg::OnNMCustomdrawDdThresh)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_CAMERA_FOCUS, &CWinCamDDDlg::OnNMCustomdrawCameraFocus)
END_MESSAGE_MAP()


// CWinCamDDDlg メッセージ ハンドラー

BOOL CWinCamDDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。
	// Start WEBCAM thread.
	CheckDlgButton(IDC_PROC0, BST_CHECKED);
	CheckDlgButton(IDC_SHRINK1, BST_CHECKED);
	shrinkSteps = 1;

	// Setup for threshold.
	paramThreshold.SetRange(0, 100);
	paramThreshold.SetPos(50);

	paramThresh = 50;	// Percent.

	// Setup for camera.
	paramFocus.SetRange(0, 100);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CWinCamDDDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CWinCamDDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CWinCamDDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWinCamDDDlg::OnBnClickedProc0()
{
	procMode = 0;
}


void CWinCamDDDlg::OnBnClickedProc1()
{
	procMode = 1;
}


void CWinCamDDDlg::OnBnClickedProc2()
{
	procMode = 2;
}


void CWinCamDDDlg::OnBnClickedProc3()
{
	procMode = 3;
}


void CWinCamDDDlg::OnBnClickedProc4()
{
	procMode = 4;
}


void CWinCamDDDlg::OnBnClickedProc5()
{
	procMode = 5;
}


void CWinCamDDDlg::OnBnClickedProc6()
{
	procMode = 6;
}


void CWinCamDDDlg::OnBnClickedShrink1()
{
	shrinkSteps = 1;
}

void CWinCamDDDlg::OnBnClickedShrink2()
{
	shrinkSteps = 2;
}

void CWinCamDDDlg::OnBnClickedShrink3()
{
	shrinkSteps = 3;
}


void CWinCamDDDlg::OnNMCustomdrawDdThresh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	paramThresh = paramThreshold.GetPos();
	SetDlgItemInt(IDC_TXT_DD_THESH, paramThresh);

	*pResult = 0;
}


void CWinCamDDDlg::OnNMCustomdrawCameraFocus(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CAM_SetFocus(paramFocus.GetPos());

	*pResult = 0;
}
