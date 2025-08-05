/********************************************************************
*	Module:	main.cpp. This is part of Visual-MinGW.
*
*	Purpose:	Main module.
*
*	Authors:	Manu B.
*
*	License:	Visual-MinGW is a C/C++ Integrated Development Environment.
*			Copyright (C) 2001  Manu.
*
*			This program is free software; you can redistribute it and/or modify
*			it under the terms of the GNU General Public License as published by
*			the Free Software Foundation; either version 2 of the License, or
*			(at your option) any later version.
*
*			This program is distributed in the hope that it will be useful,
*			but WITHOUT ANY WARRANTY; without even the implied warranty of
*			MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*			GNU General Public License for more details.
*
*			You should have received a copy of the GNU General Public License
*			along with this program; if not, write to the Free Software
*			Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
*			USA.
*
*			(See license.htm for more details.)
*
*	Revisions:	
*			Manu B. 12/15/01	CFileList created.
*
********************************************************************/
#include <windows.h>
#include <stdio.h>
#include "process.h"
#include "project.h"
#include "main.h"
#include "rsrc.h"

#define MSG_MODIFIED "Modified"

extern CMessageBox MsgBox;
CFindReplaceDlg EditorDlg;
void Main_CmdTest(HWND hwnd);

/* Globals */

CWinApp 		winApp;
CProject 		Project;
CChrono		Chrono;

// Global font
CFont		defFont;

char 			appVersion[] = "0.40a";

// File filters & flags.
DWORD singleFileOpen = OFN_EXPLORER | OFN_FILEMUSTEXIST;
DWORD multipleFileOpen = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;
DWORD fileSave = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
				OFN_OVERWRITEPROMPT;

char defFilter[] = "All Sources\0*.c;*.cpp;*.cxx;*.h;*.hpp;*.hxx;*.rc;*.mingw;*.prj\0"
			"C Files\0*.c;*.cpp;*.cxx;*.rc\0"
			"Headers\0*.h;*.hpp;*.hxx\0"
			"Ini file\0*.ini\0"
			"All Files (*.*)\0*.*\0\0";

char prjFilter[] = "Project Files (*.prj)\0*.prj\0All Files (*.*)\0*.*\0\0";
char prjDefExt[] = "prj";

char srcFilter[] = "All Sources\0*.c;*.cpp;*.cxx;*.h;*.hpp;*.hxx;*.rc\0"
			"C Files\0*.c;*.cpp;*.cxx;*.rc\0"
			"Headers\0*.h;*.hpp;*.hxx\0"
			"All Files (*.*)\0*.*\0\0";
char srcDefExt[] = "cpp";

/* For tests */
void Main_CmdTest(HWND){
	//putenv("PATH=%PATH%;c:\\bla_bla");
	char * env = getenv("PATH");
	if (env)
		MsgBox.DisplayString(env);
return;
	char outBuff[MAX_PATH];
	strcpy(outBuff, "C:\\Temp");
	//Project.RelativeToAbsolute(".\\src\\file.cpp");
	WORD offset = AppendPath(outBuff, 8, ".\\src");
	MsgBox.DisplayString(outBuff);
	MsgBox.DisplayLong((long) offset);

	char directory[MAX_PATH];
	winApp.ShellDlg.BrowseForFolder(&winApp, directory, "Browse", 
		BIF_RETURNONLYFSDIRS);
MsgBox.DisplayString(directory);
return;
}


/********************************************************************
*	Class:	CPreferencesDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CPreferencesDlg::CPreferencesDlg(){
}

CPreferencesDlg::~CPreferencesDlg(){
}

LRESULT CALLBACK CPreferencesDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){
	switch(Message){
		case WM_INITDIALOG:
			return OnInitDialog((HWND) wParam, lParam);
		
		case WM_COMMAND:
			OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
			break;

		case WM_CLOSE:
			EndDlg(0); 
			break;
	}
return FALSE;
}

BOOL CPreferencesDlg::OnInitDialog(HWND, LPARAM){
	hCcIncDir	= GetItem(IDC_CC_INCDIR);

	SetItemText(hCcIncDir, winApp.includeDir);
return TRUE;
}

BOOL CPreferencesDlg::OnCommand(WORD, WORD wID, HWND){
	switch (wID){
		case IDOK:
			GetItemText(hCcIncDir, winApp.includeDir,	MAX_PATH);
			EndDlg(IDOK);
		return TRUE;

		case IDCANCEL:
			EndDlg(IDCANCEL);
		return FALSE;
	}
return FALSE;
}


/********************************************************************
*	Functions:	WinMain procedure.
*
*	Purpose:	Runs the application.
*
*	Revisions:	
*
********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
return winApp.Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

	
/********************************************************************
*	Class:	CWinApp.
*
*	Purpose:	Manages the all application.
*
*	Revisions:	
*
********************************************************************/
CWinApp::CWinApp(){
	*iniFileName = '\0';
	hmod	= NULL;
	*projectDir = '\0';
	*includeDir = '\0';
	firstRun = false;
	// Child windows dimensions.
	deltaY 	= 0;
	tbarHeight 	= 26;
	sbarHeight 	= 20;
	tvWidth 	= 140;
	lvHeight 	= 120;

	hSplitter 	= 4;
	vSplitter 	= 4;
}

CWinApp::~CWinApp(){
}

void CWinApp::FirstRunTest(void){
	MsgBox.DisplayWarning("Visual-MinGW first run. Installation checking.\n"
					"Try to launch rm.exe, gcc.exe");
	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Testing for first run...", 0);

	// Put the command line and the run flag in the command stack.
	winApp.Process.AddTask("gcc -v", OUTERR_PIPE);
	winApp.Process.AddTask("rm --version", OUTERR_PIPE);
	winApp.Process.Run();

return;
}

bool CWinApp::ReadIniFile(char * fileName){
	ParseCmdLine(iniFileName);
	strcat(iniFileName, fileName);
	
	if (!IniFile.Load(iniFileName)){
		firstRun = true;
		MsgBox.DisplayWarning("Visual-MinGW first run. Please report bugs to:\n"
			"http://sourceforge.net/projects/visual-mingw/");
		FILE * file = fopen(iniFileName, "wb");
		if (!file)
			return false;
		SaveIniFile(file);
		fclose(file);
		return false;
	}
	if (!IniFile.Load(iniFileName)){
		MsgBox.DisplayString("Can't load ini file");
		return false;
	}
	// [General] section
	IniFile.GetString(Manager.FilesView.openFilesDir, 	"FilesDirectory", 		"General"	);
	IniFile.GetString(projectDir, 		"ProjectDirectory"		);
	// [Compiler] section
	IniFile.GetString(includeDir, 	"IncludeDir", 		"Compiler"	);
return true;
}

void CWinApp::SaveIniFile(FILE * file){
	// [General]
	fprintf (file, "[General]\nSignature = 40");
	fprintf (file, "\nFilesDirectory = %s",	Manager.FilesView.openFilesDir);
	fprintf (file, "\nProjectDirectory = %s",	projectDir);
	fprintf (file, "\nTvWidth = %d",	tvWidth);
	fprintf (file, "\nLvHeight = %d",	lvHeight);
	// [General]
	fprintf (file, "\n\n[Compiler]\nIncludeDir = %s",	includeDir);
}

bool CWinApp::WriteIniFile(void){
	if (*iniFileName == '\0')
		return false;
	FILE * file = fopen(iniFileName, "wb");
	if (!file)
		return false;
	SaveIniFile(file);
	fclose(file);
return true;
}

bool CWinApp::CustomInit(void){
	SetName("Visual MinGW", appVersion);
	MsgBox.SetCaption("Visual MinGW");
	IsWinNT();
	ReadIniFile("visual-mingw.ini");

	defFont.Create(8,	0, 0, 0,	FW_NORMAL,	0, 0, 0, 0, 0, 0, 0, 0,
		"Ms Sans Serif");

	hmod = LoadLibrary("SciLexer.DLL");
	if (!hmod){
		MsgBox.DisplayFatal("Unable to load SciLexer.DLL");
		return false;
	}
return true;
}

bool CWinApp::Release(void){
	WriteIniFile();
	if (hmod)
		FreeLibrary(hmod);
return true;
}


/********************************************************************
*	CWinApp: Create each application's window.
********************************************************************/
bool CWinApp::CreateUI(void){

	InitCommonControls();

	// Custom values.
	wc.style			= 0;
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor			= NULL;
	wc.hbrBackground	= //NULL;
					   (HBRUSH)(COLOR_INACTIVEBORDER + 1);
	wc.lpszMenuName	= MAKEINTRESOURCE(ID_MENU);
	wc.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	if(!MainRegisterEx("main_class")) {
		MsgBox.DisplayFatal("Can't Register Main Window");
		return false;
	}

	// Custom values.
	wc.hbrBackground	= NULL;
	wc.lpszMenuName	= 0;

	if(!ChildRegisterEx("child_class")) {
		MsgBox.DisplayFatal("Can't Register MDI Class");
		return false;
	}

	// Use a CreateWindowEx like procedure.
	HWND hwnd = CreateEx(
		this,	// Owner class.
		0, 
		mainClass, 
		appName, 
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		0, 
		NULL);

	if(!hwnd) {
		MsgBox.DisplayFatal("Can't create Main window");
		return false;
	}
	MsgBox.SetParent(hwnd);

	// SW_SHOWMAXIMIZED.
	::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	::UpdateWindow(hwnd);
	if (firstRun)
		FirstRunTest();
	firstRun = false;
return true;
}

void CWinApp::CreateToolbar(void){
	Toolbar.CreateEx(
		this, 
		0,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
		|TBSTYLE_FLAT | TBSTYLE_TOOLTIPS
		| CCS_NORESIZE);
	
	Toolbar.AddBitmap(IDB_TOOLBAR, 15);

	TBBUTTON tbButtons [] = 
	{	{ 0, 0, 		TBSTATE_ENABLED, TBSTYLE_SEP, 	{0, 0}, 0, 0},
		{ 0, IDM_NEW, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 1, IDM_OPEN, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 2, IDM_SAVE, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 0, 0, 		TBSTATE_ENABLED, TBSTYLE_SEP, 	{0, 0}, 0, 0},
		{ 3, IDM_CUT, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 4, IDM_COPY, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 5, IDM_PASTE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 0, 0, 		TBSTATE_ENABLED, TBSTYLE_SEP, 	{0, 0}, 0, 0},
		{ 6, IDM_UNDO, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0},
		{ 7, IDM_REDO, 	TBSTATE_ENABLED, TBSTYLE_BUTTON, 	{0, 0}, 0, 0}		};

	int numbutton = sizeof tbButtons/sizeof tbButtons[0];

	Toolbar.AddButtons(&tbButtons[0], numbutton);
} 
 
void CWinApp::CreateMDI(void){
	MdiClient.Init(3, ID_FIRSTCHILD);
	MdiClient.CreateEx(
		this, 
		WS_EX_CLIENTEDGE,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
			| WS_VSCROLL | WS_HSCROLL
		);
}

HWND CWinApp::CreateChild(char * caption, LPVOID lParam){

	CChildView	* mdiChild = new CChildView;

	HWND hwnd = mdiChild->CreateEx(
		&MdiClient,	// MUST be an MdiClient *.
		WS_EX_MDICHILD, 				
		MDIS_ALLCHILDSTYLES | WS_CHILD | WS_SYSMENU | WS_CAPTION 
		| WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
		| WS_MAXIMIZE,
		caption,
		0,
		lParam);

	if (!hwnd)
		delete mdiChild;
return hwnd;
}

void CWinApp::CreateLview(void){
	ListView.CreateEx(
		this, 
		WS_EX_CLIENTEDGE,
		WS_VISIBLE | WS_CHILD| WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
		| WS_BORDER | LVS_REPORT);

	ListView.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 
						LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
	ListView.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,
						LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	// Insert columns.
	LVCOLUMN	lvc;	
	lvc.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt		= LVCFMT_LEFT;
	
	lvc.iSubItem	= 0;
	lvc.cx		= 35;
	lvc.pszText		= "Line";
	ListView.SendMessage(LVM_INSERTCOLUMN, 0, (LPARAM) &lvc);

	lvc.iSubItem	= 1;
	lvc.cx		= 70;
	lvc.pszText		= "Unit";
	ListView.SendMessage(LVM_INSERTCOLUMN, 1, (LPARAM) &lvc);

	lvc.iSubItem	= 2;
	lvc.cx		= 670;
	lvc.pszText		= "Message";
	ListView.SendMessage(LVM_INSERTCOLUMN, 2, (LPARAM) &lvc);
}

void CWinApp::CreateSplitter(void){
	Splitter.Init(&VertSplitter, &ListView, SPLSTYLE_HORZ, lvHeight, SPLMODE_2);
	VertSplitter.Init(&Manager, &MdiClient, SPLSTYLE_VERT, tvWidth, SPLMODE_1);

	// File Manager.
	Manager.Create(this);
	// MDI client.
	CreateMDI();
	// ListView.
	CreateLview();
}

void CWinApp::CreateStatusBar(void){
	Sbar.CreateEx(
		this, 
		0,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);

	int sbWidths[] = {60, 120, -1};

	int numparts = sizeof sbWidths/sizeof sbWidths[0];

	Sbar.SetParts(numparts, &sbWidths[0]);
}

void CWinApp::SendCaretPos(int caretPos) {
	// To display the "Line : xxxx" message, we use our standard msgBuf[256].
	sprintf(msgBuf, "Line : %d", caretPos);
	Sbar.SendMessage(SB_SETTEXT, 0, (LPARAM) msgBuf);
return;
}


/********************************************************************
*	CWinApp: Message handling procedures.
********************************************************************/
LRESULT CALLBACK CWinApp::CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam){
	switch(Message){
		case WM_CREATE:
			return OnCreate((LPCREATESTRUCT) lParam);
		
		case WM_PAINT:
			return OnPaint((HDC) wParam);

		case WM_SIZE:
			return OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		
		case WM_DESTROY:
			return OnDestroy();

		case WM_COMMAND:
			return OnCommand(wParam, lParam);
		
		case WM_CLOSE:
			return OnClose();

		case WM_NOTIFY:
			Manager.OnNotify((int) wParam, (LPNMHDR) lParam);
			break;
		
		case WM_LBUTTONDOWN:
			return OnLButtonDown((short) LOWORD(lParam), (short) HIWORD(lParam), wParam);
	
		case WM_MOUSEMOVE:
			return OnMouseMove((short) LOWORD(lParam), (short) HIWORD(lParam), wParam);
	
		case WM_LBUTTONUP:
			return OnLButtonUp((short) LOWORD(lParam), (short) HIWORD(lParam), wParam);
	
		case WM_SETCURSOR:
			OnSetCursor((HWND) wParam, (UINT) LOWORD(lParam), (UINT) HIWORD(lParam));
			return DefWindowProc(_hWnd, Message, wParam, lParam);

		default:
			return DefFrameProc(_hWnd, MdiClient.GetId(), Message, wParam, lParam);
	}
return 0;
}

BOOL CWinApp::OnCreate(LPCREATESTRUCT){
	// Toolbar.
	CreateToolbar();
	// Splitter.
	CreateSplitter();
	// Statusbar.
	CreateStatusBar();
return TRUE;
} 

BOOL CWinApp::OnPaint(HDC){
	PAINTSTRUCT ps;
	BeginPaint(_hWnd, &ps);
	EndPaint(_hWnd, &ps);
return 0;
}

BOOL CWinApp::OnSize(UINT, int width, int height){
	// TreeView and MDI Client delta-height.
	deltaY =	height-sbarHeight-lvHeight-vSplitter-tbarHeight;

	if (deltaY>3){
		Toolbar.SetPosition(0,
			0, 					0, 
			width, 				tbarHeight,
			0);

		Splitter.SetPosition(0,	
			0, 					tbarHeight, 
			width, 				height-tbarHeight-sbarHeight,
			0);
	
		Sbar.SetPosition(0,
			0, 					height-sbarHeight, 
			width, 				sbarHeight,
			0);
	}
	InvalidateRect(_hWnd, NULL, false);
return 0;
} 

BOOL CWinApp::OnDestroy(void){
	PostQuitMessage(0);
return 0;
}

BOOL CWinApp::OnClose(void){
	if (IDCANCEL == Manager.SaveAll(IDASK)) // Ask to save.
		return TRUE; // Cancelled by user.
	::DestroyWindow(_hWnd);
return 0;
}

BOOL CWinApp::OnLButtonDown(short xPos, short yPos, UINT){
	Splitter.OnLButtonDown(_hWnd, xPos, yPos);
	VertSplitter.OnLButtonDown(_hWnd, xPos, yPos);
return 0;
}

BOOL CWinApp::OnMouseMove(short xPos, short yPos, UINT){
	Splitter.OnMouseMove(_hWnd, xPos, yPos);
	VertSplitter.OnMouseMove(_hWnd, xPos, yPos);
return 0;
}

BOOL CWinApp::OnLButtonUp(short xPos, short yPos, UINT){
	Splitter.OnLButtonUp(_hWnd, xPos, yPos);
	VertSplitter.OnLButtonUp(_hWnd, xPos, yPos);
return 0;
}

BOOL CWinApp::OnSetCursor(HWND, UINT nHittest, UINT){
	if (nHittest == HTCLIENT) {
		if (Splitter.OnSetCursor(_hWnd, 0)){
			return 0;
		}else if (VertSplitter.OnSetCursor(_hWnd, 0)){
			return 0;
		}else{
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		}
	}
return 0;
}


/********************************************************************
*	CWinApp: Dispatch command messages.
********************************************************************/
BOOL CWinApp::OnCommand(WPARAM wParam, LPARAM lParam){
	int wID = LOWORD(wParam);

	switch (wID){
		// File Menu.
		case IDM_NEW:
			Manager.FilesView.New();
			break;
	
		case IDM_OPEN:
			Manager.FilesView.Open();
			break;
		
		case IDM_NEW_PROJECT:
			Project.New();
			break;

		case IDM_OPEN_PROJECT:
			Project.Open();
			break;
		
		case IDM_SAVE_PROJECT:
			Project.Save();
			break;
		
		case IDM_CLOSE_PROJECT:
			Project.Close();
			break;

		case IDM_PREFERENCES:
			PreferencesDlg.CreateModal(&winApp, IDD_PREFERENCES, (LPARAM) this);
			break;

		case IDM_QUIT:
			PostMessage(_hWnd, WM_CLOSE, 0, 0);
			break;

		// Window Menu.
		case IDM_CASCADE:
			PostMessage(MdiClient.GetId(), WM_MDICASCADE, 0, 0);
			break;
		case IDM_TILEHORZ:
			PostMessage(MdiClient.GetId(), WM_MDITILE, MDITILE_HORIZONTAL, 0);
			break;
		case IDM_TILEVERT:
			PostMessage(MdiClient.GetId(), WM_MDITILE, MDITILE_VERTICAL, 0);
			break;
		case IDM_ARRANGE:
			PostMessage(MdiClient.GetId(), WM_MDIICONARRANGE, 0, 0);
			break;
	
		// Project Menu.
		case IDM_NEW_MODULE:
			Project.NewModuleDlg();
			break;

		case IDM_ADD:
			Project.AddFiles();
			break;

		case IDM_REMOVE_FILE:
			Project.RemoveFile(false);
			break;

		case IDM_REMOVE_MODULE:
			Project.RemoveFile(true);
			break;

		case IDM_OPTION:
			Project.OptionsDlg();
			break;

		case IDM_ZIP_SRCS:
			Project.ZipSrcs();
			break;

		case IDM_EXPLORE:
			Project.Explore(_hWnd);
			break;

		// Build Menu.
		case IDM_BUILD:
			Project.Build();
			break;

		case IDM_REBUILDALL:
			Project.RebuildAll();
			break;

		case IDM_RUN_TARGET:
			Project.RunTarget();
			break;

		case IDM_MKCLEAN:
			Project.MakeClean();
			break;

		case IDM_MKF_BUILD:
			Project.BuildMakefile();
			break;

		case IDM_TEST:
			Main_CmdTest(_hWnd);
			break;

		default:{
			if (wID >= ID_FIRSTCHILD){
				DefFrameProc(_hWnd, MdiClient.GetId(), WM_COMMAND, wParam, lParam);
			}else{
				HWND hChildWindow = (HWND) MdiClient.SendMessage(WM_MDIGETACTIVE);

				if (hChildWindow)
					::SendMessage(hChildWindow, WM_COMMAND, wParam, lParam);
			}
		}
	}
return TRUE;
}


/********************************************************************
*	CWinApp: Handles child messages.
********************************************************************/
LRESULT CALLBACK CWinApp::CChildWndProc(CWindow * pWnd, UINT Message, WPARAM wParam, LPARAM lParam){

	CChildView * childView = (CChildView *) pWnd;

	switch(Message){
		case WM_CREATE:
			childView->OnCreate((LPCREATESTRUCT) lParam);
			break;
		
		case WM_SIZE:
			childView->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
			break;
		
		case WM_COMMAND:
			childView->OnCommand(wParam, lParam);
			break;
		
		case WM_DESTROY:
			childView->OnDestroy();
			break;
		
		case WM_CLOSE:
			if (childView->OnClose()){
				MdiClient.SendMessage(WM_MDIDESTROY,(WPARAM) pWnd->_hWnd, 0);
			}
			return true;

		case WM_NOTIFY:
			childView->OnNotify((int) wParam, (LPNMHDR) lParam);
			break;
		
		case WM_SETFOCUS:
			childView->OnSetFocus((HWND) wParam);
			break;
		
		case WM_MDIACTIVATE:
			Child_OnActivate(pWnd, (HWND) wParam, (HWND) lParam);
			break;
	}
return DefMDIChildProc(pWnd->_hWnd, Message, wParam, lParam);
}

BOOL CWinApp::Child_OnActivate(CWindow * pWnd, HWND, HWND hwndChildAct){
	HMENU hMenu;
	HMENU hFileMenu;
	BOOL EnableFlag;

	hMenu = GetMenu(_hWnd);

	if(pWnd->_hWnd == hwndChildAct){
		EnableFlag = TRUE;    //being activated
	}else{
		EnableFlag = FALSE;   //being de-activated
	}
	// Menu items.
	EnableMenuItem(hMenu, 1, MF_BYPOSITION | (EnableFlag ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, 2, MF_BYPOSITION | (EnableFlag ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hMenu, 3, MF_BYPOSITION | (EnableFlag ? MF_ENABLED : MF_GRAYED));
	// "File" sub-menu items.
	hFileMenu = GetSubMenu(hMenu, 0);
	EnableMenuItem(hFileMenu, IDM_SAVE, MF_BYCOMMAND | (EnableFlag ? MF_ENABLED : MF_GRAYED));
	EnableMenuItem(hFileMenu, IDM_SAVEAS, MF_BYCOMMAND | (EnableFlag ? MF_ENABLED : MF_GRAYED));
	DrawMenuBar(_hWnd); 
return 0;
}


/********************************************************************
*	Class:	CChildView.
*
*	Purpose:	MDI child window class.
*
*	Revisions:	
*
********************************************************************/
CChildView::CChildView(){
	modified = false;
}

CChildView::~CChildView(){
}

bool CChildView::OnCreate(LPCREATESTRUCT){
	CFileItem * file = (CFileItem *) GetLong(GWLP_USERDATA);

	// Create Scintilla Editor Control.
	HWND hwnd = Editor.CreateEx(
		this, 
		0,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
		| WS_VSCROLL | WS_HSCROLL);

	if (!hwnd)
		return false; // @@TODO need to manage creation failure.

	// Set window handles
	file->pMdiChild = this;

	// Load a file if there is one to load.
	Editor.LoadFile(file);
return true;
}

bool CChildView::OnSize(UINT wParam, int width, int height){
	if(wParam != SIZE_MINIMIZED){
		Editor.SetPosition(0, 
					0, 
					0, 
					width, 
					height,
		0);
	}
return true;
}

BOOL CChildView::OnClose(void){
	if (modified){
		int decision = MsgBox.AskToSave(true);
		switch (decision){
			case IDCANCEL:
			return FALSE;

			case IDYES:
			CmdSave();
			break;
		}
	}
return TRUE;
}

BOOL CChildView::OnDestroy(void){
	CFileItem * file = (CFileItem*) GetLong(GWLP_USERDATA);

	if (file){
		if (!file->isInProject){
			// A simple file.
			winApp.Manager.FilesView.CloseFile(file);
		}else{
			// A project one.
			file->pMdiChild 	= NULL;
			//modified 		= false;
		}
	}
return 0;
}

BOOL CChildView::OnNotify(int, LPNMHDR notify){
	SCNotification * notification = (SCNotification *) notify;

	// Notify Message from Current Editor Control.
	if (notify->hwndFrom == Editor._hWnd){
		switch (notify->code){
			case SCN_UPDATEUI:
			Editor.GetCurrentPos();
			winApp.SendCaretPos(Editor.caretPos);
			break;
	
			case SCN_SAVEPOINTREACHED:
			modified = false;
			winApp.Sbar.WriteString("", 1);
			break;
		
			case SCN_SAVEPOINTLEFT:
			modified = true;
			winApp.Sbar.WriteString(MSG_MODIFIED, 1);
			break;

			case SCN_MARGINCLICK:
			if (notification->margin == 2)
				Editor.MarginClick(notification->position, notification->modifiers);
			break;
		}
	}
return 0;
}

BOOL CChildView::OnSetFocus(HWND){
	CFileItem * file = (CFileItem*) GetLong(GWLP_USERDATA);

	// Select corresponding TreeView item.
	CTreeView * pTreeView = file->pTreeView;

	if(!pTreeView)
		return false;

	pTreeView->SendMessage(TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)file->_hItem);

	// Set Focus on Editor Control.
	Editor.SetFocus();

	// Display "Modified" message or nothing in the Status Bar.
	winApp.SendCaretPos(Editor.caretPos);

	if(modified)
		winApp.Sbar.SendMessage(SB_SETTEXT, 1, (LPARAM) MSG_MODIFIED);
	else
		winApp.Sbar.SendMessage(SB_SETTEXT, 1, (LPARAM) "");

	int selectedTab = winApp.Manager.SendMessage(TCM_GETCURSEL);

	if (file->isInProject == true && selectedTab != PROJECT_TAB){
		winApp.Manager.SendMessage(TCM_SETCURFOCUS, PROJECT_TAB);
	}else if (file->isInProject == false && selectedTab != FILES_TAB){
		winApp.Manager.SendMessage(TCM_SETCURFOCUS, FILES_TAB);
	}
return 0;
}

void CChildView::CmdSave(void){
	CFileItem * file = (CFileItem*) GetLong(GWLP_USERDATA);

	/* Untitled file ? */
	if (file->nFileOffset == 0){
		CmdSaveAs();
		return;
	}

	if (!file->isInProject){
		// A simple file.
		Editor.SaveFile(file->szFileName);
	}else{
		// A project one.
		Project.szDirBuffer[Project.nFileOffset - 1] = '\\';
		strcpy(&Project.szDirBuffer[Project.nFileOffset], file->szFileName);
		Editor.SaveFile(Project.szDirBuffer);
		Project.szDirBuffer[Project.nFileOffset - 1] = '\0';
	}
}

void CChildView::CmdSaveAs(void){
	CFileItem * file = (CFileItem*) GetLong(GWLP_USERDATA);

	char fileName[MAX_PATH];
	if (!winApp.FileDlg.Save(&winApp, fileName, MAX_PATH, SRC_FILE)) //@@ 
		     return;  // canceled by user

	::SetWindowText(_hWnd, fileName);	
	strcpy(file->szFileName, fileName);

	Editor.SaveFile(file->szFileName);
	//@@ TODO we need to check for errors
}

BOOL CChildView::OnCommand(WPARAM wParam, LPARAM){
	CFileItem * file = (CFileItem*) GetLong(GWLP_USERDATA);

	if(!file)
		return false;

	switch (LOWORD(wParam)){
		case IDM_SAVE:
			CmdSave();
			break;
		
		case IDM_SAVEAS:
			CmdSaveAs();
			break;

		case IDM_SAVEALL:
			winApp.Manager.SaveAll(IDYES); // Silent.
			break;

/*		case IDM_CLOSE:
			PostMessage(pWnd, WM_CLOSE, 0, 0);
			break;
*/
		// To Scintilla control.
		case IDM_FIND:
			EditorDlg.Find(&Editor);
			break;

		case IDM_REPLACE:
			EditorDlg.Replace(&Editor);
			break;

		case IDM_CUT:
			Editor.SendMessage(SCI_CUT);
			break;
		case IDM_COPY:
			Editor.SendMessage(SCI_COPY);
			break;
		case IDM_PASTE:
			Editor.SendMessage(SCI_PASTE);
			break;
		case IDM_UNDO:
			Editor.SendMessage(SCI_UNDO);
			break;
		case IDM_REDO:
			Editor.SendMessage(SCI_REDO);
			break;
		case IDM_SELECTALL:
			Editor.SendMessage(SCI_SELECTALL);
			break;
	}
return TRUE;
}


/********************************************************************
*	Class:	CFileManager.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CFileManager::CFileManager(){
}

CFileManager::~CFileManager(){
}
	
void CFileManager::CreateImageList(void){ 
	// Create an empty image list.
	ImgList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 8, 1);
	
	// Load treeview bmp and add it to the image list.
	CBitmap	tvBitmap;
	tvBitmap.Load(this, IDB_TREEVIEW);
	ImgList.AddMasked(&tvBitmap, RGB(255,0,255));
	
	// We no longer need treeview bmp.
	tvBitmap.Destroy();
}

void CFileManager::Create(CWindow * pParent){ 
	// Create the Tab Control.
	CreateEx(
		pParent, 
		WS_EX_CLIENTEDGE,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
			| TCS_BOTTOM | TCS_FOCUSNEVER);

	// Give it a smart font, create tabs, select first one and show the control.
	SetFont(&defFont);

	InsertItem(FILES_TAB, TCIF_TEXT, 0, 0, "Files", 16, 0, 0);
	InsertItem(PROJECT_TAB, TCIF_TEXT, 0, 0, "Project", 16, 0, 0);
	Show();

	// Create an Image list and then the Project TreeView.
	CreateImageList();
	Project.Tree.Create(this, &ImgList);
	SetItem_Param(PROJECT_TAB, (long) &Project.Tree);
	FilesView.Create(this, &ImgList);
//	FilesView.SetLong(GWL_USERDATA, 52);
	SetItem_Param(FILES_TAB, (long) &FilesView);
}

bool CFileManager::SetPosition(HWND, int x, int y, int width, int height, UINT){
	/* Overwrites CTabCtrl::SetPosition() so that all child windows are also resized */

	// Tab Control.
	::SetWindowPos(_hWnd, 0, 
				x, 
				y, 
				width, 
				height,
	0);
	// Child windows.
	RECT Rect;
	::GetClientRect(_hWnd, &Rect);
	Project.Tree.SetPosition(0, Rect.top +5, Rect.left +5, 
		Rect.right-10, Rect.bottom-30, 0);

	FilesView.SetPosition(0, Rect.top +5, Rect.left +5, 
		Rect.right-10, Rect.bottom-30, 0);
return true;
}

int CFileManager::SaveAll(int decision){
	/* Save open files ? */
	decision = FilesView.SaveAll(decision);
	/* Save project files ? */
	decision = Project.Save(decision);
return decision;
}
	
void CFileManager::OnNotify(int, LPNMHDR notify){
	// Dispatch messages.
	switch (notify->code){
		// Tab Control.
		case TCN_SELCHANGING:
			OnSelChanging(notify);
		break;

		case TCN_SELCHANGE:
			OnSelChange(notify);
		break;

		// TreeView.
		case TVN_SELCHANGED:
			Tv_OnSelchanged((LPNMTREEVIEW) notify);
		break;
	}
}

void CFileManager::OnSelChanging(LPNMHDR notify){
	if (_hWnd == notify->hwndFrom){
		CTreeView * pTreeView = (CTreeView *) GetItem_Param(GetCurSel());
		if (pTreeView){
			pTreeView->Hide();
/*			long val = pTreeView->GetLong(GWL_USERDATA);
			if (val)
				MsgBox.DisplayLong(val);*/
		}
	}
}

void CFileManager::OnSelChange(LPNMHDR notify){
	if (_hWnd == notify->hwndFrom){
		CTreeView * pTreeView = (CTreeView *) GetItem_Param(GetCurSel());
		if (pTreeView){
			pTreeView->Show();
		}
	}
}

void CFileManager::Tv_OnSelchanged(LPNMTREEVIEW notify){
	// Get lParam of current tree item.
	CFileItem * file = (CFileItem *) notify->itemNew.lParam;

	if (file){
		CChildView * pMdiChild = (CChildView *) file->pMdiChild;

		if(pMdiChild){
			// An editor, focus it.
			::SetFocus((HWND) pMdiChild->_hWnd);
		}else{
			// No editor, the item is part of a project.
			Project.SwitchCurrentDir();
			winApp.CreateChild(file->szFileName, file);
		}
	}
} 
 

/********************************************************************
*	Class:	CFilesView.
*
*	Purpose:	Open files TreeView.
*
*	Revisions:	
*
********************************************************************/
CFilesView::CFilesView(){
	hRoot = NULL;
	*openFilesDir ='\0';
}

CFilesView::~CFilesView(){
}

HWND CFilesView::Create(CWindow * pParent, CImageList * imgList){
	// Create TreeView.
	CreateEx(
		pParent, 
		WS_EX_CLIENTEDGE,
		WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
			| TVS_EDITLABELS | TVS_HASLINES | TVS_HASBUTTONS,
		0,
		(void*) 12);

	// Assign the image list to the treeview control.
	SendMessage(TVM_SETIMAGELIST, (long)TVSIL_NORMAL, (long) imgList->GetId());
	hRoot = CreateItem(TVI_ROOT, TVI_LAST, WORKSPACE, "Workspace", 0);
return _hWnd;
}

void CFilesView::New(void){
	CFileItem * file = new CFileItem;
	if(!OpenFile(file)){
		delete file;
		MsgBox.DisplayString("Untitled file already exist.");
	}
}

void CFilesView::Open(void){
	CFileItem * file = new CFileItem;

	winApp.FileDlg.Reset();
	winApp.FileDlg.SetInitialDir(openFilesDir);

	if(!winApp.FileDlg.Open(&winApp, file->szFileName, MAX_PATH, SRC_FILE)){
		delete file;
		return;  // canceled by user
	}

	file->nFileExtension = winApp.FileDlg.GetFileExtension();
	file->nFileOffset = winApp.FileDlg.GetFileOffset();
	GetFileType(file);

	// Copy file's directory.
	strncpy(openFilesDir, file->szFileName, (file->nFileOffset - 1));
	openFilesDir[file->nFileOffset-1] = '\0';

	if(!OpenFile(file)){
		delete file;
		MsgBox.DisplayString("This file is already opened.");
	}
}

bool CFilesView::OpenFile(CFileItem * file){
	// Untitled file ?
	if (file->nFileOffset == 0){
		//@@TODO add a counter to get Untitled01, 02, etc...
		strcpy(file->szFileName, "Untitled");
	}
	
	// Add file to the open files CList.
	if (!InsertSorted(file))
		return false;

	HTREEITEM hTreeItem = CreateItem(
		hRoot, 
		TVI_SORT, 
		file->type, 
		&file->szFileName[file->nFileOffset], 
		(LPARAM) file);

	file->_hItem = hTreeItem;
	file->pTreeView = this;

	if (!winApp.CreateChild(file->szFileName, file)){
		MsgBox.DisplayFatal("Can't create child window");
		return false;
	}
return true;
}

void CFilesView::CloseFile(CFileItem * file){
	SendMessage(TVM_DELETEITEM, 0, (LPARAM) file->_hItem);		
	Destroy(file);
}


/********************************************************************
*	Class:	CFileItem.
*
*	Purpose:	Linked List Node for file parameters.
*
*	Revisions:	
*
********************************************************************/
CFileItem::CFileItem(){
	type 			= U_FILE;

	szFileName[MAX_PATH - 1] = '\0';	// security.
	szFileName[0] 	= '\0'; 
	nFileOffset		= 0;
	nFileExtension	= 0;

	pTreeView		= NULL; 
	_hDirItem		= 0;
	_hItem 		= 0;

	pMdiChild		= NULL;
	show 		= 0;
	isInProject		= false;
}

CFileItem::~CFileItem(){
}


/********************************************************************
*	Class:	CFileList.
*
*	Purpose:	A CList with a dedicated Compare() procedure.
*
*	Revisions:	
*
********************************************************************/
CFileList::CFileList(){
}

CFileList::~CFileList(){
}

int CFileList::SaveAll(int decision){
	if (decision == IDNO || decision == IDCANCEL)
		return decision;
	CFileItem * file = (CFileItem*) First();
	while (file){	
		if (file->pMdiChild){
			CChildView * childView = (CChildView *) file->pMdiChild;
			/* Modified ? */
			if (childView->modified){
				/* Ask ? */
				if (decision == IDASK){
					decision = MsgBox.AskToSave(true); // Cancel button.
					if (decision != IDYES)
						return decision; // IDNO or IDCANCEL.
				}
				childView->CmdSave();
			}
		}
		file = (CFileItem*) Next();
	}
return decision;
}

int CFileList::Compare(CNode *node1, CNode *node2){
return stricmp(((CFileItem *)node1)->szFileName, ((CFileItem *)node2)->szFileName);
}


/********************************************************************
*	Class:	CFileDlg.
*
*	Purpose:	A CFileDlgBase for Open/Save dlg boxes.
*
*	Revisions:	
*
********************************************************************/
CFileDlg::CFileDlg(){
}

CFileDlg::~CFileDlg(){
}

bool CFileDlg::Open(CWindow * pWindow, char * pszFileName, DWORD nMaxFile, int fileflag){

	switch(fileflag){
		// Project file.
		case PRJ_FILE:
		SetData(prjFilter, prjDefExt, singleFileOpen);
		break;

		// Add multiple files to project.
		case ADD_SRC_FILE:
		Reset();
		SetTitle("Add files to project");
		nMaxFile = 2048;
		SetFilterIndex(1);
		SetData(srcFilter, srcDefExt, multipleFileOpen);
		break;

		default: // SRC_FILE
		SetData(defFilter, srcDefExt, singleFileOpen);
		SetFilterIndex(1);
		break;
	}
return OpenFileName(pWindow, pszFileName, nMaxFile);
}

bool CFileDlg::Save(CWindow * pWindow, char * pszFileName, DWORD nMaxFile, int fileflag){
	Reset();

	switch(fileflag){
		case SRC_FILE:
		SetData(defFilter, srcDefExt, fileSave);
		SetFilterIndex(1);
		break;

		default: // PRJ_FILE
		SetData(prjFilter, prjDefExt, fileSave);
		break;
	}
return SaveFileName(pWindow, pszFileName, nMaxFile);
}


/********************************************************************
*	Class:	CReport.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CReport::CReport(){
}

CReport::~CReport(){
}

bool CReport::SendMsg(char * line, int outputFlag){
	int	row;

	if (outputFlag == 1){
		SplitErrorLine(line);
	
	}else if (outputFlag == 0){
		msgbuf[0] [0] = 0;
		msgbuf[1] [0] = 0;
		strcpy (msgbuf[2], line); 
	}else{
		msgbuf[0] [0] = 0;
		msgbuf[1] [0] = 0;
		strcpy (msgbuf[2], "Unrecognized outputFlag"); 

	}

	// Fill in List View columns, first is column 0.
	LV_ITEM		lvi;
	lvi.mask		= LVIF_TEXT; // | LVIF_PARAM;
	lvi.iItem		= 0x7FFF;
	lvi.iSubItem		= 0;
	lvi.pszText		= msgbuf[0];
	lvi.cchTextMax	= strlen(lvi.pszText)+1;
	lvi.lParam		= 0;

	row = SendMessage(LVM_INSERTITEM, 0, (LPARAM) &lvi);

	// Continue with column 1.
	lvi.iSubItem=1;
	lvi.pszText=msgbuf[1];
	SendMessage(LVM_SETITEMTEXT, (WPARAM)row, (LPARAM)&lvi);

	// Continue with column 2.
	lvi.iSubItem=2;
	lvi.pszText=msgbuf[2];
	SendMessage(LVM_SETITEMTEXT, (WPARAM)row, (LPARAM)&lvi);

	// Save last row position
	lastRow = row+1;

return true;
}

void CReport::SplitErrorLine(char * line){
	int	col = 1;
	int	nChar = -1;
	int	len = strlen(line)+2;

	for (int n = 0; n < len; n++){	
	
		nChar++;
		msgbuf[col][nChar] = line[n];

		// don't want first char to be a space
		if(nChar == 0 && line[n] == 0x20){
			nChar = -1;
			continue;
		}

		// error line of make 	=> [unit]:[line_n°]: [error message]
		// or				=> [unit]: [error message]
		//				unit 				=> column 1
		//				line_n°			=> column 0
		//				error message 		=> column 2
		if (line[n] == ':'){
			switch (col){
				case 1:// column 1 => Unit	
					if (line[n+1] > 0x2F && line[n+1] < 0x3A){
						// next char is a number, so next column will be 0.
						col = 0;
						msgbuf[1][nChar] = 0;
						nChar = -1;
					}else if (line[n+1] == 0x20){
						// next char is a space, so next column will be 2.
						col = 2;
						msgbuf[1][nChar] = 0;
						nChar = -1;
						// nothing to put in col 0.
						msgbuf[0][0] = 0;
					}
				break;

				case 0: // column 0 => Line
					// next col will be 2.
					msgbuf[0][nChar] = 0;
					col = 2;
					nChar = -1;
				break;
			}

		}else if (line[n] == 0){
			// end of line is reached.
			msgbuf[2][nChar] = 0;
			// exit the "for" loop
			break;
		}
	}
}	


