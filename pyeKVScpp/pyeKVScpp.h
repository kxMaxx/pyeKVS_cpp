
// pyeKVScpp.h: Hauptheaderdatei für die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'pch.h' vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"		// Hauptsymbole


// CpyeKVScppApp:
// Siehe pyeKVScpp.cpp für die Implementierung dieser Klasse
//

class CpyeKVScppApp : public CWinApp
{
public:
	CpyeKVScppApp();

// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CpyeKVScppApp theApp;
