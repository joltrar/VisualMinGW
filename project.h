/********************************************************************
*	Module:	project.h. This is part of Visual-MinGW.
*
*	License:	Visual-MinGW is covered by GNU General Public License, 
*			Copyright (C) 2001  Manu B.
*			See license.htm for more details.
*
********************************************************************/
#ifndef PROJECT_H
#define PROJECT_H

#include "winui.h"
#include "main.h"
#include "process.h"

#define BUILD_STATLIB		0
#define BUILD_DLL		1
#define BUILD_EXE		2
#define BUILD_GUIEXE		3
#define LANGC	0
#define LANGCPP	1

#define BUILD_OPTIONS_TAB		0
#define COMPILER_OPTIONS_TAB	1
#define LINKER_OPTIONS_TAB	2

class CProject;
class CMakefile;

class CGeneralDlg : public CDlgBase
{
	public:
	CGeneralDlg();
	virtual ~CGeneralDlg();

	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	OnInitDialog(HWND hwndFocus, LPARAM lInitParam);
	BOOL OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);

	protected:

	private:   
	CProject *pProject;
	CMakefile *pMakefile;
	HWND hStatLib;
	HWND hDll;
	HWND hConsole;
	HWND hGuiExe;
	HWND hDbgSym;
	HWND hLangC;
	HWND hLangCpp;
	HWND hMkfName;
	HWND hMkfDir;
	HWND hUserMkf;
	HWND hTgtName;
	HWND hTgtDir;
};

class CCompilerDlg : public CDlgBase
{
	public:
	CCompilerDlg();
	virtual ~CCompilerDlg();

	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	OnInitDialog(HWND hwndFocus, LPARAM lInitParam);
	BOOL OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);

	protected:

	private:   
	CProject *pProject;
	CMakefile *pMakefile;
	HWND hCppFlags;
	HWND hWarning;
	HWND hOptimiz;
	HWND hCFlags;
	HWND hIncDirs;
};

class CLinkerDlg : public CDlgBase
{
	public:
	CLinkerDlg();
	virtual ~CLinkerDlg();

	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	OnInitDialog(HWND hwndFocus, LPARAM lInitParam);
	BOOL OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);

	protected:

	private:   
	CProject *pProject;
	CMakefile *pMakefile;
	HWND hLdStrip;
	HWND hLdOpts;
	HWND hLdLibs;
	HWND hLibsDirs;
};

class COptionsDlg : public CDlgBase
{
	public:
	COptionsDlg();
	virtual ~COptionsDlg();

	BOOL EndDlg(int nResult);
	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);
	BOOL	OnInitDialog(HWND hwndFocus, LPARAM lInitParam);
	BOOL OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl);
	void	OnNotify(int idCtrl, LPNMHDR notify);
	void	OnSelChanging(LPNMHDR notify);
	void	OnSelChange(LPNMHDR notify);
	bool SetChildPosition(HWND hTab, HWND hChild, RECT * lpRect);

	protected:

	private:   
	LPARAM GetParam(void);

	HWND hwndTab;
	CGeneralDlg	GeneralPane;
	CCompilerDlg	CompilerPane;
	CLinkerDlg		LinkerPane;
	RECT Pos;
	TCITEM tcitem;
	CProject *pProject;
	CMakefile *pMakefile;
};

class CNewModuleDlg : public CDlgBase
{
	public:
	CNewModuleDlg();
	virtual ~CNewModuleDlg();

	virtual LRESULT CALLBACK CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam);

	protected:

	private:   
	CProject *pProject;
};

class CProjectView : public CTreeView, public CFileList
{
	public:
	CProjectView();
	~CProjectView();

	HWND Create(CWindow * pParent, CImageList * imgList);
	HTREEITEM CreateDirItem(HTREEITEM hParent, char * dir);
	void Open(char * name);
	bool AddFileSorted(CFileItem * fileInfo);
	void Close(void);

	protected:

	private:   
	HTREEITEM _hItem;
};

class CCompiler : public CIniFile
{
	public:
	CCompiler();
	~CCompiler();

	bool LoadData(char * fullpath);

	char make[64];

	char cc[16];
	char cFlags[64];
	char ldFlags[64];
	char wres[16];

	char debug[16];
	char test[16];

	protected:

	private:
};

class CMakefile
{
	public:
	CMakefile();
	~CMakefile();

	void Init(void);
	bool SwitchCurrentDir(void);
	void GetFullPath(char * prjFileName, WORD offset, char * name);
	void Build(CProjectView * Tree, CProcess* Process);
	void SrcList2Buffers(CProjectView * Tree);
	void Write(void);
	bool SendToMakefile(char * line);

	// Compiler dependent.
	char make[64];
	char wres[16];
	char test[16];

	// Data.
	char szFileName[MAX_PATH];
	WORD nFileOffset;
	char mkfDir[MAX_PATH];

	char target[64];
	char tgtDir[MAX_PATH];
	UINT buildWhat;
	bool debug;
	UINT lang;

	char cc[16];
	// Compiler data.
	char cppFlags[256];
	char warning[64];
	char optimize[64];
	char cFlags[64];
	char incDirs[256];

	// Linker data.
	char ldStrip[32];
	char ldOpts[64];
	char ldLibs[64];
	char libDirs[256];

	protected:

	private:
	// Buffers.
	char objFile[64];
	char srcBuf [1024];
	char objBuf [1024];
	char resBuf [1024];
	char depBuf [256];
};

WORD AppendPath(char * dirBuffer, WORD offset, char * absolutePath);
class CProject : public CIniFile
{
	public:
	CProject();
	~CProject();

	bool RelativeToAbsolute(char * relativePath);
	bool	New(void);
	bool	AddFiles(void);

	bool	Open(void);
	bool	Close(void);
	int	Save(int decision=IDYES);

	bool OptionsDlg(void);
	bool NewModuleDlg(void);
	CFileItem * NewFile(char * name);
	bool NewModule(char * srcFile, bool createHeader);
	void	RemoveFile(bool allModule);

	void	ZipSrcs(void);
	void	Explore(HWND hwnd);

	void	Build(void);
	void	RebuildAll(void);
	void	RunTarget(void);
	void	MakeClean(void);
	void	BuildMakefile(void);

	bool SwitchCurrentDir(void);

	CProjectView	Tree;
	CMakefile		Makefile;

	bool 		loaded;
	int 		numFiles;
	bool 		modified;
	bool		buildMakefile;

	char		szFileName[MAX_PATH];
	WORD 	nFileOffset;
	WORD	nFileExtension;
	char		szDirBuffer[MAX_PATH];

	char		compilerName[64];

	protected:

	private:   
	void Reset();
	bool	NoProject(void);
	bool 	NoClose(void);

	bool CheckFile(CFileItem * newFile);

	int CloseFile(CChildView * pMdiChild, int decision=IDASK);

	int SavePrjFile(int decision);

	COptionsDlg	_OptionsDlg;
	CNewModuleDlg	_NewModuleDlg;

	int	prjVer;
};

#endif
