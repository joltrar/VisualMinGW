/********************************************************************
*	Module:	main.h. This is part of Visual-MinGW.
*
*	License:	Visual-MinGW is covered by GNU General Public License, 
*			Copyright (C) 2001  Manu B.
*			See license.htm for more details.
*
********************************************************************/
#ifndef MAIN_H
#define MAIN_H

#include "CList.h"
#include "winui.h"
#include "editor.h"
#include "process.h"

//#define IDSILENT		20
#define IDASK		21

#define WORKSPACE	0
#define PROJECT	1
#define DIR		2

#define PRJ_FILE		0
#define SRC_FILE		1
#define ADD_SRC_FILE	2

#define FILES_TAB 	0
#define PROJECT_TAB 	1

class CChildView : public CMDIChild
{
	public:
	CChildView();
	virtual ~CChildView();

	bool	modified;

	bool OnCreate(LPCREATESTRUCT lParam);
	bool OnSize(UINT wParam, int width, int height);
	BOOL OnClose(void);
	BOOL OnDestroy(void);

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(int idCtrl, LPNMHDR notify);
	BOOL OnSetFocus(HWND hwndLoseFocus);

	void CmdSave(void);
	void CmdSaveAs(void);
	CEditor Editor;

	protected:

	private:   
};

class CFileList : public CList
{
	public:
	CFileList();
	~CFileList();

	int	SaveAll(int decision);

	protected:
	virtual int Compare(CNode *node1, CNode *node2);

	private:
};

class CFilesView : public CTreeView, public CFileList
{
	public:
	CFilesView();
	~CFilesView();

	HWND Create(CWindow * pParent, CImageList * imgList);
	void	New (void);
	void	Open(void);
	bool	OpenFile(CFileItem * file);
	void	CloseFile(CFileItem * file);

	char			openFilesDir[MAX_PATH];
	protected:

	private:   
	HTREEITEM hRoot;
};

class CFileManager : public CTabCtrl
{
	public:
	CFileManager();
	~CFileManager();

	void	Create(CWindow * pParent);
	bool	SetPosition(HWND hInsertAfter, int x, int y, int width, int height, UINT uFlags);

	int SaveAll(int silent);

	void	OnNotify(int idCtrl, LPNMHDR notify);
	void	Tv_OnDeleteItem(LPNMTREEVIEW notify);
	void	Tv_OnSelchanged(LPNMTREEVIEW notify);
	void	OnSelChanging(LPNMHDR notify);
	void	OnSelChange(LPNMHDR notify);

	CImageList	ImgList;
	CFilesView	FilesView;

	protected:

	private:
	void CreateImageList(void);

};

class CReport : public CListView
{
	public:
	CReport();
	~CReport();

	bool SendMsg(char * line, int outputFlag);

	protected:

	private:   
	char	msgbuf[3][512];

	void SplitErrorLine(char * line);
};

class CFileDlg : public CFileDlgBase
{
	public:
	CFileDlg();
	~CFileDlg();

	bool	Open(CWindow * pWindow, char * pszFileName, DWORD nMaxFile, int fileflag);
	bool	Save(CWindow * pWindow, char * pszFileName, DWORD nMaxFile, int fileflag);

	protected:

	private:   
};

class CPreferencesDlg : public CDlgBase
{
	public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();

	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	OnInitDialog(HWND hwndFocus, LPARAM lInitParam);
	BOOL OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);

	protected:

	private:   
	HWND hCcIncDir;
};

class CWinApp : public CMDIBase
{
	public:
	CWinApp();
	~CWinApp();

	void FirstRunTest(void);
	bool	ReadIniFile(char * iniFile);
	void SaveIniFile(FILE * file);
	bool	WriteIniFile(void);
	bool	CustomInit(void);
	bool	Release(void);

	bool	CreateUI(void);
	void	CreateToolbar(void);
	void	CreateSplitter(void);
	void	CreateMDI(void);
	HWND CreateChild(char * caption, LPVOID lParam);
	void	CreateLview(void);
	void	CreateStatusBar(void);

	void	SendCaretPos(int caretPos);

	// Main window.
	LRESULT CALLBACK CMainWndProc(UINT Message, WPARAM wParam, LPARAM lParam);

	BOOL	OnCreate(LPCREATESTRUCT lParam);
	BOOL	OnPaint(HDC wParam);
	BOOL	OnSize(UINT wParam, int width, int height);
	BOOL	OnDestroy(void);
	BOOL	OnClose (void);

	BOOL	OnLButtonDown(short xPos, short yPos, UINT fwKeys);
	BOOL	OnMouseMove(short xPos, short yPos, UINT fwKeys);
	BOOL	OnLButtonUp(short xPos, short yPos, UINT fwKeys);
	BOOL	OnSetCursor(HWND hwnd, UINT nHittest, UINT wMouseMsg);

	BOOL	OnCommand(WPARAM wParam, LPARAM lParam);

	// Child window.
	LRESULT CALLBACK CChildWndProc(CWindow * pWnd, UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	Child_OnActivate(CWindow * pWnd, HWND hwndChildDeact, HWND hwndChildAct);

	HMODULE 		hmod;
	char			iniFileName[MAX_PATH];
	CIniFile 		IniFile;
	CPreferencesDlg	PreferencesDlg;
	CShellDlg		ShellDlg;
	CFileDlg 		FileDlg;
	CProcess		Process;

	CToolBar		Toolbar;
	CSplitter 		Splitter;
	CSplitter 		VertSplitter;
	CFileManager	Manager;
	CReport		ListView;
	CStatusBar		Sbar;

	char			projectDir[MAX_PATH];
	char			includeDir[MAX_PATH];

	protected:

	private:
	bool firstRun;
	// Child windows dimensions.
	int deltaY;
	int tbarHeight;
	int sbarHeight;
	int tvWidth;
	int lvHeight;
	
	int hSplitter;
	int vSplitter;
};

#endif
