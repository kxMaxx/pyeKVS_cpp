
// pyeKVScppDlg.cpp: Implementierungsdatei
//

//#include "pch.h"
#include "framework.h"
#include "pyeKVScpp.h"
#include "pyeKVScppDlg.h"
#include "afxdialogex.h"

#include <vector>
#include <string>
#include <map>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include "pyeKVS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CpyeKVScppDlg-Dialogfeld

const std::string txtAbout = "pyeKVS is a binary key - value - storage format\r\nThis project is a C++/MFC implementation unit and test environment.\r\npyeKVS: Michael Koch (kxMaxx)\r\nC++/MFC implementation: Dr. Sylvio Schneider\r\n(c)2021; MIT license";

// working pyeKVS document
PyeDocument pyeDoc;
std::vector<unsigned char> buffer;

// paths for file input
// char const* filenameIn1 = "C:\\Users\\syvon Delphi in lvio\\Desktop\\workspace\\pyeKVS\\Testbench\\Output\\pyeKVS1.pye";
char const* filenameIn1 = "./pyeKVS1.pye";
// char const* filenameIn1 = "C:\\Users\\Schneider\\Desktop\\Baustellen\\workspace\\pyeKVS_Delphi\\Testbench\\Output\\pyeKVS1.pye";
// char const* filenameIn2 = "C:\\Users\\Schneider\\Desktop\\Baustellen\\workspace\\pyeKVS_Delphi\\Testbench\\Output\\pyeKVS1.pye";
// char const* filenameIn3 = "C:\\Users\\Schneider\\Desktop\\Baustellen\\workspace\\pyeKVS_Delphi\\Testbench\\Output\\pyeKVS1.pye";


// paths for file output
char const* filenameOut1 = filenameIn1;
//char const* filenameOut2 = filenameIn2;
//char const* filenameOut3 = filenameOut3;

 CEdit* mEditCtrl;
 HWND hwndEdit;

void writeBufferToFile(const char* filename, std::vector<unsigned char>& fileBytes) {
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	std::copy(fileBytes.cbegin(), fileBytes.cend(), std::ostream_iterator<unsigned char>(file));
}

CpyeKVScppDlg::CpyeKVScppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PYEKVSCPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CpyeKVScppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CpyeKVScppDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CpyeKVScppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON14, &CpyeKVScppDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_DlgButtonMemoryHandmade, &CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryhandmade)
	ON_BN_CLICKED(IDC_DlgButtonMemoryRandom, &CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryrandom)
	ON_BN_CLICKED(IDC_DlgButtonMemorySaveToFile, &CpyeKVScppDlg::OnBnClickedDlgbuttonmemorysavetofile)
	ON_BN_CLICKED(IDC_BUTTON13, &CpyeKVScppDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_RADIO2, &CpyeKVScppDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO1, &CpyeKVScppDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_DlgButtonMemorySimple, &CpyeKVScppDlg::OnBnClickedDlgbuttonmemorysimple)
	ON_BN_CLICKED(IDC_DlgButtonMemoryReadFromFile, &CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryreadfromfile)
	ON_BN_CLICKED(IDC_DlgButtonClear, &CpyeKVScppDlg::OnBnClickedDlgbuttonclear)

END_MESSAGE_MAP()


// CpyeKVScppDlg-Meldungshandler

BOOL CpyeKVScppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(TEXT("pyeKVS (C++) Testbench"));

	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden

	mEditCtrl = (CEdit*) this->GetDlgItem(IDC_EDIT1);

	CButton* m_check = (CButton*) this->GetDlgItem(IDC_RADIO1);
	m_check->SetCheck(TRUE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen.  Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CpyeKVScppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CpyeKVScppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CpyeKVScppDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonmemorysimple()
{
	pyeDoc = PyeDocument();

	unsigned __int16 headerPrefix = pyeDoc.getHeaderPrefix();
	unsigned __int16 headerVersionH = pyeDoc.getHeaderVersionH();
	unsigned __int16 headerVersionL = pyeDoc.getHeaderVersionL();
	unsigned __int16 headerSize = pyeDoc.getHeaderSize();

	unsigned __int16 rootSize = pyeDoc.getRoot().getSize();
	unsigned __int16 rootCount = pyeDoc.getRoot().getCount();

	// prepare keys and values for test
	std::string keyInt16 = "MyValue1";
	std::string keyStrS = "MyString1";

	__int16 valInt16 = 256;
	std::string valStrS = "Hello PYES.";

	pyeDoc.getRoot().putInt16(valInt16, keyInt16);
	pyeDoc.getRoot().putStringS(valStrS, keyStrS);

	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryrandom()
{
	pyeDoc = PyeDocument();

	unsigned __int16 headerPrefix = pyeDoc.getHeaderPrefix();
	unsigned __int16 headerVersionH = pyeDoc.getHeaderVersionH();
	unsigned __int16 headerVersionL = pyeDoc.getHeaderVersionL();
	unsigned __int16 headerSize = pyeDoc.getHeaderSize();

	unsigned __int16 rootSize = pyeDoc.getRoot().getSize();
	unsigned __int16 rootCount = pyeDoc.getRoot().getCount();

	// prepare keys and values for test
	std::string keyInt16 = "MyValue1";
	std::string keyStrS = "MyString1";

	__int16 valInt16 = 256;
	std::string valStrS = "Hello PYES.";

	pyeDoc.getRoot().putInt16(valInt16, keyInt16);
	pyeDoc.getRoot().putStringS(valStrS, keyStrS);

	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryhandmade()
{
	pyeDoc = PyeDocument();

	/*
	* Test 1: get header data of document
	*/

	unsigned __int16 headerPrefix = pyeDoc.getHeaderPrefix();
	unsigned __int16 headerVersionH = pyeDoc.getHeaderVersionH();
	unsigned __int16 headerVersionL = pyeDoc.getHeaderVersionL();
	unsigned __int16 headerSize = pyeDoc.getHeaderSize();

	unsigned __int16 rootSize = pyeDoc.getRoot().getSize();
	unsigned __int16 rootCount = pyeDoc.getRoot().getCount();

	/*
	* Test 2: put and get fundamental values to document in 1 level and safe to file, test readability/validate with delphi testbench
	*/

	// prepare keys and values for test
	std::string keyint8 = "EinInt8WertL1";
	std::string keyint16 = "EinInt16WertL1";
	std::string keyint32 = "EinInt32WertL1";
	std::string keyint64 = "EinInt64WertL1";

	std::string keyuint8 = "EinUInt8WertL1";
	std::string keyuint16 = "EinUInt16WertL1";
	std::string keyuint32 = "EinUInt32WertL1";
	std::string keyuint64 = "EinUInt64WertL1";

	std::string keyfloat = "EinFloatWertL1";
	std::string keydouble = "EinDoubleWertL1";
	std::string keystrS = "EinStringSWertL1";
	std::string keystrL = "EinStringLWertL1";
	std::string keymem = "EinMemoryWertL1";

	__int8 valint8 = 8;
	__int16 valint16 = 16;
	__int32 valint32 = 32;
	__int64 valint64 = 64;

	unsigned __int8 valuint8 = 88;
	unsigned __int16 valuint16 = 1616;
	unsigned __int32 valuint32 = 3232;
	unsigned __int64 valuint64 = 6464;

	float valfloat = 32.3232f;
	double valdouble = 64.646464;
	std::string valstrS = "ein kurzer String";
	std::string valstrL = "ein langer String";
	std::vector<unsigned char> valmem = { 0,34,83,94,27,32 };

	// put keys and values to document
	unsigned __int32 docSize = pyeDoc.getHeaderSize();
	unsigned __int32 listSize = pyeDoc.getRoot().getSize();
	unsigned __int32 listCount = pyeDoc.getRoot().getCount();

	pyeDoc.getRoot().putInt8(valint8, keyint8);
	pyeDoc.getRoot().putInt16(valint16, keyint16);
	pyeDoc.getRoot().putInt32(valint32, keyint32);
	pyeDoc.getRoot().putInt64(valint64, keyint64);
	pyeDoc.getRoot().putUInt8(valuint8, keyuint8);
	pyeDoc.getRoot().putUInt16(valuint16, keyuint16);
	pyeDoc.getRoot().putUInt32(valuint32, keyuint32);
	pyeDoc.getRoot().putUInt64(valuint64, keyuint64);
	pyeDoc.getRoot().putFloat(valfloat, keyfloat);
	pyeDoc.getRoot().putDouble(valdouble, keydouble);
	pyeDoc.getRoot().putStringS(valstrS, keystrS);
	pyeDoc.getRoot().putStringL(valstrL, keystrL);

	docSize = pyeDoc.getHeaderSize();
	listSize = pyeDoc.getRoot().getSize();
	listCount = pyeDoc.getRoot().getCount();

	// read back the values
	__int8 valint8back = pyeDoc.getRoot().getInt8(keyint8);
	__int16 valint16back = pyeDoc.getRoot().getInt16(keyint16);
	__int32 valint32back = pyeDoc.getRoot().getInt32(keyint32);
	__int64 valint64back = pyeDoc.getRoot().getInt64(keyint64);

	unsigned __int8 valuint8back = pyeDoc.getRoot().getUInt8(keyuint8);
	unsigned __int16 valuint16back = pyeDoc.getRoot().getUInt16(keyuint16);
	unsigned __int32 valuint32back = pyeDoc.getRoot().getUInt32(keyuint32);
	unsigned __int64 valuint64back = pyeDoc.getRoot().getUInt64(keyuint64);

	float valfloatback = pyeDoc.getRoot().getFloat(keyfloat);
	double valdoubleback = pyeDoc.getRoot().getDouble(keydouble);
	std::string valstrSback = pyeDoc.getRoot().getStringS(keystrS);
	std::string valstrLback = pyeDoc.getRoot().getStringL(keystrL);
//	std::vector<unsigned char> valmemback = pyeDoc.getRoot().getMemory(keymem);


	/*
	* Test 3: put and read keys and values to document in multiple levels and save to file, test readability/validate with delphi testbench
	*/

	// prepare keys and values for test
	std::string keyint8L2 = "EinInt8WertL2";
	std::string keyint16L2 = "EinInt16WertL2";
	std::string keyint32L2 = "EinInt32WertL2";
	std::string keyint64L2 = "EinInt64WertL2";

	std::string keyuint8L2 = "EinUInt8WertL2";
	std::string keyuint16L2 = "EinUInt16WertL2";
	std::string keyuint32L2 = "EinUInt32WertL2";
	std::string keyuint64L2 = "EinUInt64WertL2";

	std::string keyfloatL2 = "EinFloatWertL2";
	std::string keydoubleL2 = "EinDoubleWertL2";
	std::string keystrSL2 = "EinStringSWertL2";
	std::string keystrLL2 = "EinStringLWertL2";
	std::string keymemL2 = "EinMemoryWertL2";

	PyeList& pyeList = pyeDoc.getRoot().putList("List1.Level2");
	int bufferSize = (*pyeDoc.getBuffer()).size();
	docSize = pyeDoc.getHeaderSize();
	listSize = pyeDoc.getRoot().getSize();
	listCount = pyeDoc.getRoot().getCount();

	pyeList.putInt8(valint8, keyint8L2);

	int putted = 1/*key length*/ + keyint8L2.length() + 1/*pyeValueType*/ + sizeof(valint8);
	int diff = docSize + putted - pyeDoc.getHeaderSize();
	int diff2 = (*pyeDoc.getBuffer()).size() - pyeDoc.getHeaderSize();

	bufferSize = (*pyeDoc.getBuffer()).size();
	docSize = pyeDoc.getHeaderSize();
	listSize = pyeDoc.getRoot().getSize();
	listCount = pyeDoc.getRoot().getCount();

	pyeList.putInt16(valint16, keyint16L2);
	pyeList.putInt32(valint32, keyint32L2);
	pyeList.putInt64(valint64, keyint64L2);
	pyeList.putUInt8(valuint8, keyuint8L2);
	pyeList.putUInt16(valuint16, keyuint16L2);
	pyeList.putUInt32(valuint32, keyuint32L2);
	pyeList.putUInt64(valuint64, keyuint64L2);
	pyeList.putFloat(valfloat, keyfloatL2);
	pyeList.putDouble(valdouble, keydoubleL2);
	pyeList.putStringS(valstrS, keystrSL2);
	pyeList.putStringL(valstrL, keystrLL2);

	PyeList& pyeList2 = pyeList.putList("List1.Level3");
	pyeList2.putInt8(valint8, keyint8L2);
	pyeList2.putInt16(valint16, keyint16L2);
	pyeList2.putInt32(valint32, keyint32L2);
	pyeList2.putInt64(valint64, keyint64L2);
	pyeList2.putUInt8(valuint8, keyuint8L2);
	pyeList2.putUInt16(valuint16, keyuint16L2);
	pyeList2.putUInt32(valuint32, keyuint32L2);
	pyeList2.putUInt64(valuint64, keyuint64L2);
	pyeList2.putFloat(valfloat, keyfloatL2);
	pyeList2.putDouble(valdouble, keydoubleL2);
	pyeList2.putStringS(valstrS, keystrSL2);
	pyeList2.putStringL(valstrL, keystrLL2);

	PyeArray& pyeArray1 = pyeList.putArray("Array1", pyeValueType::pyeInt8);
	pyeArray1.putInt8(1);
	pyeArray1.putInt8(2);
	pyeArray1.putInt8(3);
	pyeArray1.putInt8(4);
	pyeArray1.putInt8(5);


	PyeArray& pyeArray2 = pyeDoc.getRoot().putArray("Array2", pyeValueType::pyeStringUTF8S);
	pyeArray2.putStringS("Str 1");
	pyeArray2.putStringS("String 2");
	pyeArray2.putStringS("String 3 Test");
	pyeArray2.putStringS("String 4.0");
	pyeArray2.putStringS("String 5555");

	std::vector<pyeValueType> mapType1{pyeValueType::pyeInt16, pyeValueType::pyeFloat64, pyeValueType::pyeUInt32};

	PyeArrayMap& pyeArrayMap1 = pyeDoc.getRoot().putArrayMap("ArrayMap1", mapType1);
	pyeArrayMap1.putInt16((__int16)16);
	pyeArrayMap1.putDouble((double)64.6464);
	pyeArrayMap1.putUInt32((unsigned __int32)32);

	pyeArrayMap1.putInt16((__int16)17);
	pyeArrayMap1.putDouble((double)65.6565);
	pyeArrayMap1.putUInt32((unsigned __int32)33);

	pyeArrayMap1.putInt16((__int16)18);
	pyeArrayMap1.putDouble((double)66.6666);
	pyeArrayMap1.putUInt32((unsigned __int32)34);

	__int16 int16_1 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getInt16(0, 0);
	double double_1 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getDouble(0, 1);
	unsigned __int32 uint32_1 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getUInt32(0, 2);

	__int16 int16_2 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getInt16(1, 0);
	double double_2 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getDouble(1, 1);
	unsigned __int32 uint32_2 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getUInt32(1, 2);

	__int16 int16_3 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getInt16(2, 0);
	double double_3 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getDouble(2, 1);
	unsigned __int32 uint32_3 = pyeDoc.getRoot().getArrayMap("ArrayMap1").getUInt32(2, 2);
	
	// finish pye document with handmade data

	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonmemorysavetofile()
{
	writeBufferToFile(filenameOut1, *pyeDoc.getBuffer());
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonmemoryreadfromfile()
{
	pyeDoc = PyeDocument(filenameIn1);

	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedDlgbuttonclear()
{
	pyeDoc = PyeDocument();
	mEditCtrl->SetWindowText(_T(""));
}

void CpyeKVScppDlg::writePyeDocToEdit () {
	std::string strJson;
	if (IsDlgButtonChecked(IDC_RADIO1)) {
		strJson = pyeDoc.toStringSimple("\r\n", "\t");
	}
	else {
		strJson = pyeDoc.toStringJSON("\r\n", "\t");
	}

	CString cStrJSON (strJson.c_str());
	mEditCtrl->SetWindowText(cStrJSON);
}

void CpyeKVScppDlg::OnBnClickedRadio1()
{
	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedRadio2()
{
	writePyeDocToEdit();
}

void CpyeKVScppDlg::OnBnClickedButton13()
{
	CString cStr(txtAbout.c_str());
	mEditCtrl->SetWindowText(cStr);
}


#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>

int __cdecl sendReveiveData()
{
	WSADATA              wsaData;
	SOCKET               SendingSocket;

	// Server/receiver address
	SOCKADDR_IN          ServerAddr, ThisSenderInfo;

	// Server/receiver port to connect to
	unsigned int         Port = 9999;

	int  RetCode;
	// Be careful with the array bound, provide some checking mechanism...
	char sendbuf[1024] = "This is a test string from sender";
	int BytesSent, nlen;

	// Initialize Winsock version 2.2

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Create a new socket to make a client connection.
	// AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol

	SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SendingSocket == INVALID_SOCKET) {
		printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());

		// Do the clean up
		WSACleanup();

		// Exit with error
		return -1;
	}
	else {
		printf("Client: socket() is OK!\n");
	}

	// Set up a SOCKADDR_IN structure that will be used to connect
	// to a listening server on port 5150. For demonstration
	// purposes, let's assume our server's IP address is 127.0.0.1 or localhost

	// IPv4
	ServerAddr.sin_family = AF_INET;

	// Port no.
	ServerAddr.sin_port = htons(Port);

	// The IP address
//	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	InetPton(AF_INET, _T("127.0.0.1"), &ServerAddr.sin_addr.s_addr);

	// Make a connection to the server with socket SendingSocket.
	RetCode = connect(SendingSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));

	if (RetCode != 0) {
		printf("Client: connect() failed! Error code: %ld\n", WSAGetLastError());

		// Close the socket
		closesocket(SendingSocket);

		// Do the clean up
		WSACleanup();

		// Exit with error
		return -1;
	}

	else {
		printf("Client: connect() is OK, got connected...\n");
		printf("Client: Ready for sending and/or receiving data...\n");
	}

	// At this point you can start sending or receiving data on
	// the socket SendingSocket.

	// Some info on the receiver side...
	getsockname(SendingSocket, (SOCKADDR*)&ServerAddr, (int*)sizeof(ServerAddr));

//	printf("Client: Receiver IP(s) used: %s\n", inet_ntoa(ServerAddr.sin_addr));
//	printf("Client: Receiver port used: %d\n", htons(ServerAddr.sin_port));

	// Sends some data to server/receiver...
	BytesSent = send(SendingSocket, sendbuf, strlen(sendbuf), 0);

	if (BytesSent == SOCKET_ERROR) {
		printf("Client: send() error %ld.\n", WSAGetLastError());
	}
	else {
		printf("Client: send() is OK - bytes sent: %ld\n", BytesSent);

		// Some info on this sender side...
		// Allocate the required resources
		memset(&ThisSenderInfo, 0, sizeof(ThisSenderInfo));
		nlen = sizeof(ThisSenderInfo);

		getsockname(SendingSocket, (SOCKADDR*)&ThisSenderInfo, &nlen);

//		printf("Client: Sender IP(s) used: %s\n", inet_ntoa(ThisSenderInfo.sin_addr));
		printf("Client: Sender port used: %d\n", htons(ThisSenderInfo.sin_port));
		printf("Client: Those bytes represent: \"%s\"\n", sendbuf);

	}

	if (shutdown(SendingSocket, SD_SEND) != 0) {
		printf("Client: Well, there is something wrong with the shutdown(). The error code : % ld\n", WSAGetLastError());
	}
	else {
		printf("Client: shutdown() looks OK...\n");
	}

	// When you are finished sending and receiving data on socket SendingSocket,
	// you should close the socket using the closesocket API. We will
	// describe socket closure later in the chapter.

	if (closesocket(SendingSocket) != 0) {
		printf("Client: Cannot close \"SendingSocket\" socket. Error code: %ld\n", WSAGetLastError());
	}
	else {
		printf("Client: Closing \"SendingSocket\" socket...\n");
	}

	// When your application is finished handling the connection, call WSACleanup.

	if (WSACleanup() != 0) {
		printf("Client: WSACleanup() failed!...\n");
	}
	else {
		printf("Client: WSACleanup() is OK...\n");
	}

	return 0;

}

void CpyeKVScppDlg::OnBnClickedButton14()
{
	sendReveiveData();
}
