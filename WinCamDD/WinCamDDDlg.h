
// WinCamDDDlg.h : ヘッダー ファイル
//

#pragma once


// CWinCamDDDlg ダイアログ
class CWinCamDDDlg : public CDialogEx
{
// コンストラクション
public:
	CWinCamDDDlg(CWnd* pParent = nullptr);	// 標準コンストラクター

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WINCAMDD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート


// 実装
protected:
	HICON m_hIcon;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedProc0();
	afx_msg void OnBnClickedProc1();
	afx_msg void OnBnClickedProc2();
	afx_msg void OnBnClickedProc3();
	afx_msg void OnBnClickedProc4();
	afx_msg void OnBnClickedProc5();
	afx_msg void OnBnClickedProc6();
	afx_msg void OnBnClickedShrink2();
	afx_msg void OnBnClickedShrink3();
	afx_msg void OnBnClickedShrink1();
	afx_msg void OnNMCustomdrawDdThresh(NMHDR *pNMHDR, LRESULT *pResult);
	// Parameter of DD threshold
	CSliderCtrl paramThreshold;
	// manual focus adjustment of camera
	CSliderCtrl paramFocus;
	afx_msg void OnNMCustomdrawCameraFocus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCamera0();
	afx_msg void OnBnClickedCamera1();
	afx_msg void OnBnClickedCamera2();
	afx_msg void OnBnClickedFlogtake();
	afx_msg void OnBnClickedUseProfile();
	afx_msg void OnBnClickedQrRead();
};
