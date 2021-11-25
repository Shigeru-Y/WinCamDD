
// WinCamDD.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// メイン シンボル


// CWinCamDDApp:
// このクラスの実装については、WinCamDD.cpp を参照してください
//

class CWinCamDDApp : public CWinApp
{
public:
	CWinCamDDApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CWinCamDDApp theApp;
