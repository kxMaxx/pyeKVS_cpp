
// pyeKVScppDlg.h: Headerdatei
//

#pragma once


// CpyeKVScppDlg-Dialogfeld
class CpyeKVScppDlg : public CDialogEx
{
// Konstruktion
public:
	CpyeKVScppDlg(CWnd* pParent = nullptr);	// Standardkonstruktor

// Dialogfelddaten
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PYEKVSCPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung

private:
	void writePyeDocToEdit();

// Implementierung
protected:
	HICON m_hIcon;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDlgbuttonmemoryhandmade();
	afx_msg void OnBnClickedDlgbuttonmemorysavetofile();
	afx_msg void OnBnClickedDlgbuttonmemorysimple();
	afx_msg void OnBnClickedDlgbuttonmemoryreadfromfile();
	afx_msg void OnBnClickedDlgbuttonclear();
	afx_msg void OnBnClickedDlgbuttonmemoryrandom();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
};
