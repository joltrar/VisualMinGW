/********************************************************************
*	Module:	project.cpp. This is part of Visual-MinGW.
*
*	Purpose:	Procedures to manage a loaded project.
*
*	Authors:	Manu B.
*
*	License:	Visual-MinGW is covered by GNU General Public License, 
*			Copyright (C) 2001  Manu B.
*			See license.htm for more details.
*
*	Revisions:	
*
********************************************************************/
#include <windows.h>
#include <stdio.h>

#include "project.h"
#include "rsrc.h"

extern CMessageBox MsgBox;

// For dialogs.
extern CWinApp 	winApp;


/********************************************************************
*	Class:	COptionsDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
COptionsDlg::COptionsDlg(){
	hwndTab = 0;
	Pos.left 	= 0;
	Pos.top 	= 0;
	Pos.right 	= 0;
	Pos.bottom	= 0;

	pProject = NULL;
	pMakefile = NULL;
}

COptionsDlg::~COptionsDlg(){
}

LRESULT CALLBACK COptionsDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){
	switch(Message){
		case WM_INITDIALOG:
			return OnInitDialog((HWND) wParam, lParam);
		
		case WM_NOTIFY:
			OnNotify((int) wParam, (LPNMHDR) lParam);
			break;
		
		case WM_COMMAND:
			OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
			break;

		case WM_CLOSE:
			EndDlg(0); 
			break;
	}
return FALSE;
}

BOOL COptionsDlg::OnInitDialog(HWND, LPARAM lInitParam){
	// Set pointers.
	pProject = (CProject *) lInitParam;
	if (pProject == NULL)
		return TRUE;

	// Tab control handle and TCITEM.
	hwndTab = ::GetDlgItem(_hWnd, IDC_OPTION_TABS);
	tcitem.mask = TCIF_TEXT | TCIF_PARAM;

	// Insert General tab.
	HWND hwndChild = GeneralPane.Create(this, IDD_GENERAL_PANE, NULL, (long) pProject);
	tcitem.pszText = "General";
	tcitem.lParam = (long) &GeneralPane;
	::SendMessage(hwndTab, TCM_INSERTITEM, BUILD_OPTIONS_TAB, 
		(LPARAM)&tcitem);
	SetChildPosition(hwndTab, hwndChild, &Pos);
	
	// Insert Compiler tab item.
	CompilerPane.Create(this, IDD_COMPILER, &Pos, (long) pProject);
	tcitem.pszText = "Compiler";
	tcitem.lParam = (long) &CompilerPane;
	::SendMessage(hwndTab, TCM_INSERTITEM, COMPILER_OPTIONS_TAB, 
		(LPARAM)&tcitem);

	// Insert Linker tab.
	LinkerPane.Create(this, IDD_LINKER, &Pos, (long) pProject);
	tcitem.pszText = "Linker";
	tcitem.lParam = (long) &LinkerPane;
	::SendMessage(hwndTab, TCM_INSERTITEM, LINKER_OPTIONS_TAB, 
		(LPARAM)&tcitem);

	// Show the dialog and default pane.
	Show();
	GeneralPane.Show();
	GeneralPane.SetFocus();
return TRUE;
}

bool COptionsDlg::SetChildPosition(HWND hTab, HWND hChild, RECT * lpRect){
	// Get tab's display area.
	RECT area;
	::GetWindowRect(hTab, &area);
	::ScreenToClient(_hWnd, (POINT *) &area.left);
	::ScreenToClient(_hWnd, (POINT *) &area.right);
	::SendMessage(hTab, TCM_ADJUSTRECT, FALSE, (LPARAM) &area);
	::CopyRect(lpRect, &area);

	// Get child dialog's rect.
	RECT child;
	::GetWindowRect(hChild, &child);
	::ScreenToClient(_hWnd, (POINT *) &child.left);
	::ScreenToClient(_hWnd, (POINT *) &child.right);

	// Center child dialog.
	int childWidth = child.right-child.left;
	int childHeight = child.bottom-child.top;
	int hMargin = ((area.right-area.left)-childWidth)/2;
	int vMargin = ((area.bottom-area.top)-childHeight)/2;
	lpRect->left += hMargin;
	lpRect->top += vMargin;
	lpRect->right = childWidth;
	lpRect->bottom = childHeight;

return ::SetWindowPos(hChild, 0, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, 0);
}

BOOL COptionsDlg::OnCommand(WORD wNotifyCode, WORD wID, HWND hwndCtl){
	switch (wID){
		case IDOK:
			GeneralPane.OnCommand(wNotifyCode, wID, hwndCtl);
			CompilerPane.OnCommand(wNotifyCode, wID, hwndCtl);
			LinkerPane.OnCommand(wNotifyCode, wID, hwndCtl);
			pProject->buildMakefile = true;
			pProject->modified = true;
			EndDlg(IDOK);
			return TRUE;

		case IDCANCEL:
			EndDlg(IDCANCEL);
			break;
	}
return 0;
}

void COptionsDlg::OnNotify(int, LPNMHDR notify){
	// Dispatch tab control messages.
	switch (notify->code){
		case TCN_SELCHANGING:
			OnSelChanging(notify);
		break;

		case TCN_SELCHANGE:
			OnSelChange(notify);
		break;
	}
}

void COptionsDlg::OnSelChanging(LPNMHDR notify){
	// Hide child dialog that is deselected.
	if (notify->hwndFrom == hwndTab){
		CWindow * pPaneDlg = (CWindow *) GetParam();
		if (pPaneDlg){
			if (pPaneDlg->_hWnd)
				pPaneDlg->Hide();
		}
	}
}

void COptionsDlg::OnSelChange(LPNMHDR notify){
	// Show child dialog that is selected.
	if (notify->hwndFrom == hwndTab){
		CWindow * pPaneDlg = (CWindow *) GetParam();
		if (pPaneDlg){
			if (pPaneDlg->_hWnd)
				pPaneDlg->Show();
				pPaneDlg->SetFocus();
		}
	}
}

LPARAM COptionsDlg::GetParam(void){
	int iItem = ::SendMessage(hwndTab, TCM_GETCURSEL, 0, 0);

	tcitem.mask = TCIF_PARAM;
	BOOL result = ::SendMessage(hwndTab, TCM_GETITEM, iItem, (long) &tcitem);
	if (result)
		return tcitem.lParam;
return 0;
}

BOOL COptionsDlg::EndDlg(int nResult){
	GeneralPane.EndDlg(0);
	CompilerPane.EndDlg(0);
	LinkerPane.EndDlg(0);
	if (_hWnd){
		BOOL result = ::EndDialog(_hWnd, nResult);
		_hWnd = 0;
		return result;
	}
return false;
}


/********************************************************************
*	Class:	CGeneralDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CGeneralDlg::CGeneralDlg(){
	pProject = NULL;
	pMakefile = NULL;
}

CGeneralDlg::~CGeneralDlg(){
}

LRESULT CALLBACK CGeneralDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){
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

//extern CProject Project;

BOOL CGeneralDlg::OnInitDialog(HWND, LPARAM lInitParam){
	/* Set pointers */
	pProject = (CProject *) lInitParam;
	if (pProject == NULL || &pProject->Makefile == NULL)
		return TRUE;
	pMakefile = &pProject->Makefile;

	/* Get control handles */
	hStatLib 	= GetItem(IDC_STATLIB);
	hDll 		= GetItem(IDC_DLL);
	hConsole 	= GetItem(IDC_CONSOLE);
	hGuiExe 	= GetItem(IDC_GUIEXE);
	hDbgSym 	= GetItem(IDC_DBGSYM);
	hLangC 	= GetItem(IDC_LANGC);
	hLangCpp 	= GetItem(IDC_LANGCPP);
	hMkfName 	= GetItem(IDC_MKF_NAME);
	hMkfDir 	= GetItem(IDC_MKF_DIR);
	hUserMkf 	= GetItem(IDC_USER_MKF);
	hTgtName 	= GetItem(IDC_TGT_NAME);
	hTgtDir 	= GetItem(IDC_TGT_DIR);

	/* Set buttons state */
	if (pMakefile->buildWhat == BUILD_GUIEXE)
		::SendMessage(hGuiExe, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendMessage(hConsole, BM_SETCHECK, BST_CHECKED, 0);

	if (pMakefile->debug)
		::SendMessage(hDbgSym, BM_SETCHECK, BST_CHECKED, 0);

	if (pMakefile->lang == LANGCPP)
		::SendMessage(hLangCpp, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendMessage(hLangC, BM_SETCHECK, BST_CHECKED, 0);

	/* Set text */
	char name[64];
	if (pMakefile->nFileOffset){
		strcpy(name, &pMakefile->szFileName[pMakefile->nFileOffset]);
	}else{
		strcpy(name, "noname");
	}

	SetItemText(hMkfName,	name);
	SetItemText(hMkfDir,		pMakefile->mkfDir);
	SetItemText(hTgtName,	pMakefile->target);
	SetItemText(hTgtDir, 		pMakefile->tgtDir);
return TRUE;
}

BOOL CGeneralDlg::OnCommand(WORD, WORD wID, HWND){
	switch (wID){
		case IDOK:
			/* Get buttons state */
			if (BST_CHECKED == ::SendMessage(hGuiExe, BM_GETCHECK, 0, 0))
				pMakefile->buildWhat = BUILD_GUIEXE;
			else
				pMakefile->buildWhat = BUILD_EXE;

			pMakefile->debug = 
				(BST_CHECKED==::SendMessage(hDbgSym, BM_GETCHECK, 0, 0));

			if (BST_CHECKED == ::SendMessage(hLangCpp, BM_GETCHECK, 0, 0))
				pMakefile->lang = LANGCPP;
			else
				pMakefile->lang = LANGC;

			/* Get text */
			char name[64];
			GetItemText(hMkfName,	name,	64);
			GetItemText(hMkfDir,		pMakefile->mkfDir,	MAX_PATH);
			GetItemText(hTgtName, 	pMakefile->target,	64);
			GetItemText(hTgtDir,		pMakefile->tgtDir,	MAX_PATH);

			pMakefile->GetFullPath(pMakefile->szFileName, pProject->nFileOffset, name);
			//@@TODO check if directories exist.
		return TRUE;

		case IDCANCEL:
		return FALSE;
	}
return FALSE;
}


/********************************************************************
*	Class:	CCompilerDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CCompilerDlg::CCompilerDlg(){
	pProject = NULL;
	pMakefile = NULL;
}

CCompilerDlg::~CCompilerDlg(){
}

LRESULT CALLBACK CCompilerDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){
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

BOOL CCompilerDlg::OnInitDialog(HWND, LPARAM lInitParam){
	// Set pointers.
	pProject = (CProject *) lInitParam;
	if (pProject == NULL || &pProject->Makefile == NULL)
		return TRUE;
	pMakefile = &pProject->Makefile;

	hCppFlags	= GetItem(IDC_CPPFLAGS);
	hWarning	= GetItem(IDC_WARNING);
	hOptimiz	= GetItem(IDC_OPTIMIZ);
	hCFlags	= GetItem(IDC_CFLAGS);
	hIncDirs	= GetItem(IDC_INCDIRS);

	SetItemText(hCppFlags, pMakefile->cppFlags);
	SetItemText(hWarning, pMakefile->warning);
	SetItemText(hOptimiz, 	pMakefile->optimize);
	SetItemText(hCFlags, 	pMakefile->cFlags);
	SetItemText(hIncDirs, 	pMakefile->incDirs);
return TRUE;
}

BOOL CCompilerDlg::OnCommand(WORD, WORD wID, HWND){
	switch (wID){
		case IDOK:
			GetItemText(hCppFlags,	pMakefile->cppFlags,	256);
			GetItemText(hWarning,	pMakefile->warning,	64);
			GetItemText(hOptimiz, 	pMakefile->optimize,	64);
			GetItemText(hCFlags, 	pMakefile->cFlags,	64);
			GetItemText(hIncDirs, 	pMakefile->incDirs, 	256);
		return TRUE;

		case IDCANCEL:
		return FALSE;
	}
return FALSE;
}


/********************************************************************
*	Class:	CLinkerDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CLinkerDlg::CLinkerDlg(){
	pProject = NULL;
	pMakefile = NULL;
}

CLinkerDlg::~CLinkerDlg(){
}

LRESULT CALLBACK CLinkerDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){
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

BOOL CLinkerDlg::OnInitDialog(HWND, LPARAM lInitParam){
	// Set pointers.
	pProject = (CProject *) lInitParam;
	if (pProject == NULL || &pProject->Makefile == NULL)
		return TRUE;
	pMakefile = &pProject->Makefile;

	hLdStrip	= GetItem(IDC_LDSTRIP);
	hLdOpts	= GetItem(IDC_LDOPTS);
	hLdLibs	= GetItem(IDC_LDLIBS);
	hLibsDirs	= GetItem(IDC_LIBDIRS);

	SetItemText(hLdStrip, 	pMakefile->ldStrip);
	SetItemText(hLdOpts, 	pMakefile->ldOpts);
	SetItemText(hLdLibs, 	pMakefile->ldLibs);
	SetItemText(hLibsDirs, 	pMakefile->libDirs);
return TRUE;
}

BOOL CLinkerDlg::OnCommand(WORD, WORD wID, HWND){
	switch (wID){
		case IDOK:
			GetItemText(hLdStrip, 	pMakefile->ldStrip,	32);
			GetItemText(hLdOpts, 	pMakefile->ldOpts,	64);
			GetItemText(hLdLibs, 	pMakefile->ldLibs,	64);
			GetItemText(hLibsDirs,	pMakefile->libDirs,	256);
		return TRUE;

		case IDCANCEL:
		return FALSE;
	}
return FALSE;
}


/********************************************************************
*	Class:	CNewModuleDlg.
*
*	Purpose:
*
*	Revisions:	
*
********************************************************************/
CNewModuleDlg::CNewModuleDlg(){
	pProject = NULL;
}

CNewModuleDlg::~CNewModuleDlg(){
}

LRESULT CALLBACK CNewModuleDlg::CDlgProc(UINT Message, WPARAM wParam, LPARAM lParam){

	char		fileName[64];
	bool		createHeader = true;
	HWND wCreateHeader = GetItem(IDC_HEADER);

	switch(Message){
		case WM_INITDIALOG:
			pProject = (CProject *) lParam;
			if (createHeader)
				::SendMessage(wCreateHeader, BM_SETCHECK, BST_CHECKED, 0);
				strcpy(fileName, "new.cpp");
				::SetDlgItemText(_hWnd, 301, fileName);
			return TRUE;
	
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL){
				fileName[0] = 0;
				EndDlg(IDCANCEL);
				return FALSE;
			}else if(LOWORD(wParam) == IDOK){
				::GetDlgItemText(_hWnd, 301, fileName, 64);
				createHeader = BST_CHECKED ==
						 ::SendMessage(wCreateHeader, BM_GETCHECK, 0, 0);
				pProject->NewModule(fileName, createHeader);
				EndDlg(IDCANCEL);
				return TRUE;
			}
			break; 

		case WM_CLOSE:
			EndDlg(0); 
			break;
	}
return FALSE;
}


/********************************************************************
*	Class:	CProjectView.
*
*	Purpose:	Project TreeView.
*
*	Revisions:	
*
********************************************************************/
CProjectView::CProjectView(){
	_hItem = NULL;
}

CProjectView::~CProjectView(){
}

HWND CProjectView::Create(CWindow * pParent, CImageList * imgList){
	// Create TreeView.
	CreateEx(
		pParent, 
		WS_EX_CLIENTEDGE,
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS 
		| TVS_EDITLABELS | TVS_HASLINES | TVS_HASBUTTONS);

	// Assign the image list to the treeview control.
	SendMessage(TVM_SETIMAGELIST, (long)TVSIL_NORMAL, (long) imgList->GetId());
return _hWnd;	
}

void CProjectView::Open(char * name){

	_pParent->SendMessage(TCM_SETCURFOCUS, 1, 0);

	// Create Root Item.
	_hItem = CreateItem(TVI_ROOT, TVI_LAST, PROJECT, name, 0);
	SendMessage(TVM_EXPAND, (long) TVE_EXPAND, (long) _hItem);
}

HTREEITEM CProjectView::CreateDirItem(HTREEITEM hParent, char * dir){

return CreateItem(hParent, TVI_LAST, DIR, dir, 0);
}

bool CProjectView::AddFileSorted(CFileItem * newFile){
	if (!newFile)
		return false;

	// Initialize _hDirItem and get a pointer to current file.
	newFile->_hDirItem = _hItem;
	CFileItem * currentFile = (CFileItem *) GetCurrent();

	// Try to insert the new file to the list.
	if(!InsertSorted(newFile)){
		MsgBox.DisplayWarning("Can't add %s\nOperation cancelled !", newFile->szFileName);
		delete newFile;
		return false;
	}

	// Some files were in the list ?
	if (!currentFile){
		// No files. Parse path string and create tree view items.
		char dir[MAX_PATH];
		WORD len = 2;
		WORD dirlen = 0;
		while (len < newFile->nFileOffset){
			while (newFile->szFileName[len] != '\\'){
				// End of string reached ?
				if (newFile->szFileName[len] == '\0'){
					delete newFile;
					return false;
				}
				// Copy each char.
				dir[dirlen] = newFile->szFileName[len];
				len++;
				dirlen++;
			}
			// We have a valid "dir\".
			dir[dirlen] = '\0';
			newFile->_hDirItem = CreateDirItem(newFile->_hDirItem, dir);
			len++;
			dirlen = 0;
		}	// Continue to parse.

	}else{
		// A currentFile.
		if (newFile->nFileOffset == currentFile->nFileOffset){
			// Same length, compare names.
			if (0 == strnicmp(newFile->szFileName, currentFile->szFileName, currentFile->nFileOffset)){
				// Same dir, same _hDirItem;
				newFile->_hDirItem = currentFile->_hDirItem;
			}else{
				// Find directory names.
				newFile->_hDirItem = _hItem;
			}
		}
	}

	// Now that we have a DirItem, create the file item with *newFile attached to it.
	newFile->_hItem = CreateItem(
		newFile->_hDirItem, 
		TVI_SORT, 
		newFile->type, 
		&newFile->szFileName[newFile->nFileOffset],
		(LPARAM) newFile);

	newFile->pTreeView = this;

	// Create an editor view ?
	if (newFile->show){
		winApp.CreateChild(newFile->szFileName, newFile);
	}
return true;
}

void CProjectView::Close(void){
	SendMessage(TVM_DELETEITEM, 0, (long) _hItem);		
	_hItem = 0;

	_pParent->SendMessage(TCM_SETCURFOCUS, 0, 0);
	DestroyList();
}


WORD AppendPath(char * dirBuffer, WORD offset, char absolutePath[]){
	WORD len = 0;
	if (absolutePath[0] == '.'){
		if (absolutePath[1] == '\\' && absolutePath[2] != '\0'){
			dirBuffer[offset-1] = '\\';
			len = strcpylen(&dirBuffer[offset], &absolutePath[2]);
			len++;
		}
	}
return len;
}

/********************************************************************
*	Class:	CProject.
*
*	Purpose:	Project management.
*
*	Revisions:	
*
********************************************************************/
//@@TODO close project Dlg's before closing Project.
CProject::CProject(){
	prjVer = 40;
}

CProject::~CProject(){
}

void CProject::Reset(){
	szFileName[MAX_PATH - 1] = '\0';	// security.
	szFileName[0] 		= '\0';
	szDirBuffer[0] 		= '\0';
	nFileOffset			= 0;
	nFileExtension		= 0;

	numFiles 			= 0;
	loaded			= false;
	modified			= false;
	buildMakefile		= true;

	compilerName[0]		= '\0';

	Makefile.Init();
}

bool CProject::NoProject(void){
	if (!loaded){
		MsgBox.DisplayWarning("No project loaded");
		// Or directly show open project dlg.
		return true;
	}
return false;
}

bool 	CProject::NoClose(void){
	if (loaded){
		if (IDNO==MsgBox.Ask("Close current project ?", false))
			return true;
		else
			Close();
	}
return false;
}

bool CProject::SwitchCurrentDir(void){
	// Switch to Project Directory
	szDirBuffer[nFileOffset-1] = '\0';
return ::SetCurrentDirectory(szDirBuffer);
}

bool CProject::RelativeToAbsolute(char * relativePath){
	if (*szDirBuffer && nFileOffset){
		if (relativePath[0] == '.' && relativePath[1] == '\\' 
									&& relativePath[2] != '\0'){
			szDirBuffer[nFileOffset-1] = '\\';
			strcpy(&szDirBuffer[nFileOffset], &relativePath[2]);
			return true;
		}
	}
return false;
}


/********************************************************************
*	New Project.
********************************************************************/
bool CProject::New(void){

	if(NoClose())
		return false;

	// Load default values.
	Reset();

	if (!winApp.FileDlg.Save(&winApp, szFileName, MAX_PATH, PRJ_FILE)){
		return false;  // canceled by user
	}
	// Copy prj file's directory.
	nFileOffset = winApp.FileDlg.GetFileOffset();
	strncpy(szDirBuffer, szFileName, (nFileOffset - 1));
	szDirBuffer[nFileOffset-1] = '\0';
	strcpy(winApp.projectDir, szDirBuffer);

	// Makefile: Get target name.
	nFileExtension = winApp.FileDlg.GetFileExtension();
	int len = nFileExtension-nFileOffset;
	strncpy(Makefile.target, &szFileName[nFileOffset], len);
	strcpy(&Makefile.target[len], "exe");

	Tree.Open("Project");
	loaded = true;

	AddFiles();
	
	Makefile.GetFullPath(szFileName, nFileOffset, "makefile");
	MsgBox.DisplayString(Makefile.szFileName);

	buildMakefile = true;
	//modified = true;
	OptionsDlg();
	SavePrjFile(IDYES);
return true;
}

/********************************************************************
*	Open Project.
********************************************************************/
bool CProject::Open(void){
	if(NoClose())
		return false;

	// Load default values.
	Reset();
	winApp.FileDlg.Reset();
	winApp.FileDlg.SetInitialDir(winApp.projectDir);

	if (!winApp.FileDlg.Open(&winApp, szFileName, MAX_PATH, PRJ_FILE)){
		return false;  // canceled by user
	}
	// Copy project file's directory.
	nFileOffset = winApp.FileDlg.GetFileOffset();
	strncpy(szDirBuffer, szFileName, (nFileOffset - 1));
	szDirBuffer[nFileOffset-1] = '\0';
	strcpy(winApp.projectDir, szDirBuffer);

	// Initialize project tree view.
	Tree.Open(&szFileName[nFileOffset]);

	// Load project file in a buffer.
	if (!Load(szFileName)){
		MsgBox.DisplayFatal("Can't load project file !");
		//@@ should close inifile ?
		Tree.Close();
		return false;
	}

	char name[64];
	*name = '\0';
	// [Project] section
	int signature = GetInt(			"Signature", 		"Project"	);
	if (signature != prjVer){
		MsgBox.DisplayFatal("Bad signature in the project file !");
		Tree.Close();
		return false;
	}

	numFiles = GetInt(			"NumFiles"	);
	GetString(compilerName, 		"Compiler"	);
	buildMakefile = GetInt(			"BuildMakefile"	);

	// [Makefile] section
	GetString(Makefile.make, 		"Make", 		"Makefile"	);
	GetString(Makefile.cc, 			"CC"		);
	GetString(Makefile.wres, 		"WRES"	);
	GetString(Makefile.test, 		"TEST"	);
	GetString(name, 				"Makefile"	);
	GetString(Makefile.mkfDir, 		"MakefileDir");
	GetString(Makefile.target, 		"Target"	);
	GetString(Makefile.tgtDir, 		"TargetDir"	);
	Makefile.buildWhat = GetInt( 	"Build"	);
	Makefile.debug = GetInt( 		"Debug"	);
	Makefile.lang = GetInt( 		"Lang"	);
	GetString(Makefile.cppFlags,		"CppFlags"	);
	GetString(Makefile.warning, 		"CcWarning");
	GetString(Makefile.optimize, 		"CcOptimize");
	GetString(Makefile.cFlags, 		"CcFlags"	);
	GetString(Makefile.incDirs,		"IncDirs"	);
	GetString(Makefile.ldStrip, 		"LdStrip"	);
	GetString(Makefile.ldOpts, 		"LdOptions"	);
	GetString(Makefile.ldLibs, 		"LdLibraries");
	GetString(Makefile.libDirs, 		"LdLibDirs"	);

	Makefile.GetFullPath(szFileName, nFileOffset, name);

	if (numFiles){
		CFileItem * srcFile;
		// [FileXX] section
		char fileNumber [8];
		char fileSection [16];
	
		for (int n=1; n<=numFiles; n++){
			itoa(n, fileNumber, 10);
			strcpy(fileSection, "File");
			strcat(fileSection, fileNumber);
	
			// SrcFile
			srcFile = new CFileItem;
			GetString(srcFile->szFileName,	"Name", 	fileSection);
			CheckFile(srcFile);
			::GetFileType(srcFile);
			srcFile->show 	= GetInt(	"Show"	);
	
			if(!Tree.AddFileSorted(srcFile)){
				Tree.Close();
				return false;
			}
			srcFile->isInProject = true;
		}
	}
	loaded = true;
return true;
}

int CProject::SavePrjFile(int decision){
	if (!loaded || !modified)
		return decision;
	if (decision == IDNO || decision == IDCANCEL)
		return decision;

	/* Ask ? */
	if (decision == IDASK){
		decision = MsgBox.AskToSave(true); // Cancel button.
		if (decision != IDYES)
			return decision; // IDNO or IDCANCEL.
	}

	FILE * file;
	CFileItem * srcFile;
	int count = 0;

	numFiles = Tree.Length();

	file = fopen(szFileName, "w");
	if (!file){
		MsgBox.DisplayFatal("Can't save project file !");
		return decision;
	}

	// [Project]
	fprintf (file, "[Project]\nSignature = %d"		, prjVer			);
	fprintf (file, "\nNumFiles = %d"			, numFiles			);
	fprintf (file, "\nCompiler = %s"			, compilerName		);
	fprintf (file, "\nBuildMakefile = %d"			, buildMakefile		);

	// [Makefile]
	fprintf (file, "\n\n[Makefile]\nMAKE = %s"		, Makefile.make		);
	fprintf (file, "\nCC = %s"				, Makefile.cc		);
	fprintf (file, "\nWRES = %s"				, Makefile.wres		);
	fprintf (file, "\nTEST = %s"				, Makefile.test		);
	fprintf (file, "\nMakefile = %s"				, &Makefile.szFileName[Makefile.nFileOffset]);
	fprintf (file, "\nMakefileDir = %s"			, Makefile.mkfDir		);
	fprintf (file, "\nTarget = %s"				, Makefile.target		);
	fprintf (file, "\nTargetDir = %s"			, Makefile.tgtDir		);
	fprintf (file, "\nBuild = %d"				, Makefile.buildWhat	);
	fprintf (file, "\nDebug = %d"				, Makefile.debug		);
	fprintf (file, "\nLang = %d"				, Makefile.lang		);
	fprintf (file, "\nCppFlags = %s"			, Makefile.cppFlags	);
	fprintf (file, "\nCcWarning = %s"			, Makefile.warning	);
	fprintf (file, "\nCcOptimize = %s"			, Makefile.optimize	);
	fprintf (file, "\nCcFlags = %s"				, Makefile.cFlags		);
	fprintf (file, "\nIncDirs = %s"				, Makefile.incDirs		);
	fprintf (file, "\nLdStrip = %s"				, Makefile.ldStrip		);
	fprintf (file, "\nLdOptions = %s"			, Makefile.ldOpts		);
	fprintf (file, "\nLdLibraries = %s"			, Makefile.ldLibs		);
	fprintf (file, "\nLdLibDirs = %s"			, Makefile.libDirs		);

	/* [Filexx] */
	srcFile = (CFileItem *) Tree.First();
	while (srcFile){
		count++;
		fprintf (file, "\n\n[File%d"			, count			);
		fprintf (file, "]\nName = %s"			, srcFile->szFileName	);
		fprintf (file, "\nShow = %d" 			, 0				);
		srcFile = (CFileItem *) Tree.Next();
	}
	fprintf (file, "\n");
	fclose(file);
	modified = false;
return decision;
}

bool CProject::CheckFile(CFileItem * newFile){
	WORD offset = 2;
	WORD len = 2;
	// Valid relative path ?
	if (newFile && newFile->szFileName[0] == '.' && newFile->szFileName[1] == '\\'
									&& newFile->szFileName[2] != '\0'){
		// Find backslashes & dots.
		while (newFile->szFileName[len] != '\0'){
			switch(newFile->szFileName[len]){
				case '\\':
				offset = len+1;
				break;

				case '.':
				newFile->nFileExtension = len+1;
				break;
			}
			len++;
		}
		// Last char can't be '\\'.
		if (newFile->szFileName[offset] != '\0'){
			newFile->nFileOffset = offset;
			return true;
		}
	}
return false;
}

CFileItem * CProject::NewFile(char * name){
	CFileItem * current = (CFileItem *) Tree.GetCurrent();
	CFileItem * srcFile = new CFileItem;

	// Copy directory name.
	if (current){
		strncpy(srcFile->szFileName, current->szFileName, current->nFileOffset);
		srcFile->nFileOffset = current->nFileOffset;
	}else{
		// No files in the project yet, use makefile directory.
		int len = strcpylen(srcFile->szFileName, Makefile.mkfDir);
		srcFile->nFileOffset = len+1;
	}

	srcFile->szFileName[srcFile->nFileOffset - 1] = '\\';

	// Append file name.
	strcpy(&srcFile->szFileName[srcFile->nFileOffset], name);
	MsgBox.DisplayString(srcFile->szFileName);
//	delete srcFile;
//	return NULL;

	if (CheckFile(srcFile)){
		::GetFileType(srcFile);
		srcFile->isInProject = true;
		HANDLE hFile;      
		hFile = ::CreateFile(srcFile->szFileName, 
					0, 
					0, 
					NULL,
					CREATE_NEW, 
					FILE_ATTRIBUTE_ARCHIVE,
					NULL);
	
		if (hFile != INVALID_HANDLE_VALUE){
			CloseHandle(hFile);
			// Try to add new file to the project.
			if(Tree.AddFileSorted(srcFile))
				return srcFile;
			else
				return NULL;
		}
	}
	delete srcFile;
return NULL;
}

bool CProject::NewModule(char * name, bool createHeader){
	if (NoProject())
		return false;

	SwitchCurrentDir();

	CFileItem * srcFile = NewFile(name);
	if (!srcFile){
		MsgBox.DisplayWarning("Can't create file : %s", name);
		return false;
	}
	if (createHeader && srcFile->type != H_FILE){
		char header[64];
		strcpy(header, name);
		char ext[] = "h";
		ChangeFileExt(header, ext);

		if (!NewFile(header)){
			MsgBox.DisplayWarning("Can't create file : %s", header);
			return false;
		}
	}
return true;
}

bool CProject::AddFiles(void){
	if (NoProject())
		return false;

	CFileItem * srcFile;
	char srcFiles[2048];
	srcFiles [0] = 0;
	WORD fileOffset;

	// Show Open dialog.
	if (!winApp.FileDlg.Open(&winApp, srcFiles, 2048, ADD_SRC_FILE))
		return false;  // canceled by user

	// Check if srcFiles path includes projectDir.
	int n = 0;
	int maxlen = nFileOffset - 1;
	while (n<maxlen){
		// @@ shouldn't be case sensitive.
		if (srcFiles[n] != szFileName[n])
			break;
		n++;
	}

	if (srcFiles[n] == '\\' || srcFiles[n] == '\0'){
		// We are in the project, copy directory name.
		n++;
		char relativePath[MAX_PATH];
		relativePath[0] = '.';
		relativePath[1] = '\\';
		int nn = 2;

		fileOffset = winApp.FileDlg.GetFileOffset();
		maxlen = fileOffset - 1;
		while (n<maxlen){
			relativePath[nn] = srcFiles[n];
			n++;
			nn++;
		}
		if (nn > 2){
			relativePath[nn] = '\\';
			nn++;
		}
		relativePath[nn] = '\0';

		// Append each file name.
		fileOffset = winApp.FileDlg.GetFileOffset();

		while (fileOffset){
			// Try to add each file to the project.
			srcFile = new CFileItem;
			strcpy(srcFile->szFileName, relativePath);

			strcat(srcFile->szFileName, &srcFiles[fileOffset]);

			CheckFile(srcFile);
			::GetFileType(srcFile);
			srcFile->show 	= 0;
			srcFile->isInProject = true;
	
			if(!Tree.AddFileSorted(srcFile))
				return false;

			fileOffset = winApp.FileDlg.GetNextFileOffset();
		}
		modified = true;
		buildMakefile = true;
		return true;
	}
	MsgBox.DisplayString("Out of the project");
return false;
}

/********************************************************************
*	Remove Module.
********************************************************************/
void CProject::RemoveFile(bool allModule){
	if (NoProject())
		return;

	CFileItem * srcFile;
	CFileItem * otherFile;
	
	// get lParam of current tree node 
	Tree._TvItem.hItem  = (HTREEITEM) Tree.SendMessage(TVM_GETNEXTITEM, 
						TVGN_CARET, 0);
	Tree._TvItem.mask   = TVIF_PARAM;
	Tree._TvItem.lParam = 0; 
	Tree.SendMessage(TVM_GETITEM, 0, (long) &Tree._TvItem); 

	srcFile = (CFileItem *) Tree._TvItem.lParam;
	
	if (srcFile){
		if (allModule){
			if (srcFile->prev){
				otherFile = (CFileItem *) srcFile->prev;
				if (otherFile->nFileExtension != 0){
					if (0 == strnicmp(srcFile->szFileName, otherFile->szFileName, otherFile->nFileExtension)){
						if (otherFile->pMdiChild)
							CloseFile((CChildView *) otherFile->pMdiChild);
						Tree.SendMessage(TVM_DELETEITEM, 0, (long) otherFile->_hItem);		
						Tree.Destroy(otherFile);
					}
				}
			}
			if (srcFile->next){
				otherFile = (CFileItem *) srcFile->next;
				if (otherFile->nFileExtension != 0){
					if (0 == strnicmp(srcFile->szFileName, otherFile->szFileName, otherFile->nFileExtension)){
						if (otherFile->pMdiChild)
							CloseFile((CChildView *) otherFile->pMdiChild);
						Tree.SendMessage(TVM_DELETEITEM, 0, (long) otherFile->_hItem);		
						Tree.Destroy(otherFile);
					}
				}
			}
		}
		if (srcFile->pMdiChild)
			CloseFile((CChildView *) srcFile->pMdiChild);
		Tree.SendMessage(TVM_DELETEITEM, 0, (long) srcFile->_hItem);		
		Tree.Destroy(srcFile);

		// we need to save prj file before exit.
		modified = true;
		buildMakefile = true;
	}else{
		MsgBox.DisplayWarning("No project file selected");
	}
}

/********************************************************************
*	Save Project.
********************************************************************/
int CProject::Save(int decision){
	if (!loaded)
		return 0;
	decision = Tree.SaveAll(decision);
	if (modified)
		return SavePrjFile(decision);
return decision;
}

/********************************************************************
*	Close Project.
********************************************************************/
bool CProject::Close(void){
	if (NoProject())
		return false;

	int decision = IDASK;
	decision = Save(decision);
	if (decision == IDCANCEL)
		return false;

	// Parse the list while there's a next node.
	CFileItem * srcFile = (CFileItem *) Tree.First();
	while(srcFile){
		CloseFile((CChildView *) srcFile->pMdiChild, decision);
		srcFile = (CFileItem *) Tree.Next();
	}
	Tree.Close();

	loaded = false;
	winApp.ListView.Clear();
return true;
}

int CProject::CloseFile(CChildView * pMdiChild, int decision){
	if (!pMdiChild)
		return 0;

	if (pMdiChild->modified && decision != IDNO){
		// Ask ?
		if (decision == IDASK){
			decision = MsgBox.AskToSave(true); // (Cancel button)
			if (decision == IDCANCEL) 
				return decision;
		}
		pMdiChild->CmdSave();
	}

	if (pMdiChild->_hWnd)	// have an editor window, so destroy it.
		winApp.MdiClient.SendMessage(WM_MDIDESTROY, (WPARAM)pMdiChild->_hWnd, 0);
return decision;
}


/********************************************************************
*	Dialogs.
********************************************************************/
bool CProject::OptionsDlg(void){
/**/	if (NoProject())
		return false;

	_OptionsDlg.CreateModal(&winApp, IDD_OPTION, (LPARAM) this);
return true;
}

bool CProject::NewModuleDlg(void){
	if (NoProject())
		return false;

	_NewModuleDlg.CreateModal(&winApp, IDD_NEW_MODULE, (LPARAM) this);
return true;
}

/********************************************************************
*	Project Commands.
********************************************************************/
void CProject::ZipSrcs(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	// Switch to Project Directory
	SwitchCurrentDir();

	winApp.ListView.Clear();

	char zipDir[] =".\\zip\\";
	char fileName[64];
	char date[16];
	date[0] = 0;

	// Archive name = zipDir\month+day+year+src.zip
	GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, "MMddyyyy", 
				date, 16);

	strcpy (fileName, zipDir);
	strcat (fileName, date);
	strcat (fileName, "src");
	strcat (fileName, ".zip");

	char msgBuf[128];
	sprintf(msgBuf, "Create archive > %s", fileName);
	winApp.ListView.SendMsg(msgBuf, 0);

	// Put the command line and the run flag in the command stack.
	winApp.Process.AddTask(
		"zip ", 
		OUTERR_PIPE);

	winApp.Process.CmdCat(fileName);
	winApp.Process.CmdCat(" *.prj .\\bin\\*.dll .\\src\\*.* .\\include\\*.h -x *.o");
	
	winApp.Process.Run();
}

void CProject::Explore(HWND hwnd){
	if (NoProject())
		return;

	if(ShellExecute(hwnd, "explore", szDirBuffer, NULL, NULL, SW_SHOWMAXIMIZED) 
			< (HINSTANCE) 32){
		MsgBox.DisplayString("Can't launch Explorer");
		return;
	}
}

/********************************************************************
*	Compiler Commands.
********************************************************************/
void CProject::Build(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Invoking compiler...", 0);

	// Save modified files
	Tree.SaveAll(IDYES); // Silent.

	// Switch to Makefile Directory
	Makefile.SwitchCurrentDir();

	/* Build makefile ? */
	if (buildMakefile){
		winApp.ListView.SendMsg("Building makefile...", 0);

		// Fill buffers and initialize a new process.
		Makefile.Build(&Tree, &winApp.Process);
		buildMakefile = false;
		modified = true;
	}

	// Put the command line and the run flag in the command stack.
	winApp.Process.AddTask(
		Makefile.make, 
		OUTERR_PIPE);

	winApp.Process.CmdCat(" -f ");
	winApp.Process.CmdCat(&Makefile.szFileName[Makefile.nFileOffset]);
	if (Makefile.debug)
		winApp.Process.CmdCat(" debug");

	winApp.Process.Run();
}

void CProject::RebuildAll(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	// Switch to Makefile Directory
	Makefile.SwitchCurrentDir();

	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Invoking compiler...", 0);

	// Save modified files
	Tree.SaveAll(IDYES); // Silent.

	// Build makefile.
	Makefile.Build(&Tree, &winApp.Process);
	buildMakefile = false;
	modified = true;

	// Make clean.
	winApp.Process.AddTask(
		Makefile.make, 
		OUTERR_PIPE);

	winApp.Process.CmdCat(" -f ");
	winApp.Process.CmdCat(&Makefile.szFileName[Makefile.nFileOffset]);
	winApp.Process.CmdCat(" clean");
	
	// Build.
	winApp.Process.AddTask(
		Makefile.make, 
		OUTERR_PIPE);

	winApp.Process.CmdCat(" -f ");
	winApp.Process.CmdCat(&Makefile.szFileName[Makefile.nFileOffset]);
	if (Makefile.debug)
		winApp.Process.CmdCat(" debug");

	winApp.Process.Run();
}

void CProject::RunTarget(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Run target...", 0);

	// Put the command line and the run flag in the command stack.
	winApp.Process.AddTask(szDirBuffer, 0);
	winApp.Process.CmdCat("\\");
	if (Makefile.tgtDir[0] == '.'){
		if (Makefile.tgtDir[1] == '.' && Makefile.tgtDir[2] == '\\' 
									&& Makefile.tgtDir[3] != '\0'){
			winApp.Process.CmdCat(&Makefile.tgtDir[3]);
		}else{
			// Invalid dir, try ".\target".
			winApp.Process.CmdCat(".");
		}
	}
	winApp.Process.CmdCat("\\");
	winApp.Process.CmdCat(Makefile.target);
	
	winApp.Process.Run();
}

void CProject::MakeClean(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	// Switch to Makefile Directory
	Makefile.SwitchCurrentDir();

	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Deleting objects...", 0);

	// Put the command line and the output flag in the command stack.
	winApp.Process.AddTask(
		Makefile.make, 
		OUTERR_PIPE);

	winApp.Process.CmdCat(" -f ");
	winApp.Process.CmdCat(&Makefile.szFileName[Makefile.nFileOffset]);
	winApp.Process.CmdCat(" clean");
	
	winApp.Process.Run();
}

void CProject::BuildMakefile(void){
	if (NoProject() || winApp.Process.isRunning())
		return;

	// Switch to Project Directory
	Makefile.SwitchCurrentDir();

	winApp.ListView.Clear();
	winApp.ListView.SendMsg("Building makefile...", 0);

	// Fill buffers and initialize a new process.
	Makefile.Build(&Tree, &winApp.Process);
	buildMakefile = false;
	modified = true;
	// Run the process.
	winApp.Process.Run();
}


/********************************************************************
*	Class:	CMakefile.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CMakefile::CMakefile(){
	Init();
}

CMakefile::~CMakefile(){
}

void CMakefile::Init(void){
	szFileName[MAX_PATH - 1] = '\0';	// security.
	szFileName[0] 		= '\0';
	nFileOffset			= 0;

	debug			= false;
	lang				= LANGC;
	buildWhat			= BUILD_EXE;

	target[0] 			= '\0';
	strcpy	(make, 	"make");
	strcpy	(cc, 		"g++");

	cppFlags[0]		= '\0';
	strcpy	(warning, 	"-W -Wall -pedantic");
	strcpy	(optimize, 	"-O2");
	cFlags[0]			= '\0';
	incDirs[0] 			= '\0';

	strcpy	(ldStrip, 	"-s");
	ldOpts[0]			= '\0';
	ldLibs[0]			= '\0';
	libDirs[0] 			= '\0';

	strcpy	(wres, 	"windres");
	strcpy	(test, 	"gcc -v");
	// Directories
	strcpy	(mkfDir,	".");
	strcpy	(tgtDir,	".");
}

void CMakefile::GetFullPath(char * prjFileName, WORD offset, char * name){
	// Copy project directory and append makefile relative dir.
	strncpy(szFileName, prjFileName, offset);
	WORD len = AppendPath(szFileName, offset, mkfDir);
	// Increment file offset.
	if (len){
		offset += len;
	}else{
		strcpy(mkfDir, ".");
	}
	// Append makefile name.
	szFileName[offset-1] = '\\';
	if (*name){
		strcpy(&szFileName[offset], name);
	}else{
		strcpy(name, "makefile");
	}
	nFileOffset = offset;
}

bool CMakefile::SwitchCurrentDir(void){
	// Switch to Makefile Directory.
	if (nFileOffset < 2)
		return false;
	szFileName[nFileOffset-1] = '\0';
	bool result = SetCurrentDirectory(szFileName);
	szFileName[nFileOffset-1] = '\\';
return result;
}

void CMakefile::Build(CProjectView * Tree, CProcess* /*Process*/){
	SrcList2Buffers(Tree);
	// Write the first part of the Makefile.
	Write();
	// Invokes compiler to get dependencies with something like: 
	// 	"g++ -MM file1.cpp file2.cpp ..."

	// 3. Put the command line and the run flag in the command stack.
	winApp.Process.AddTask(
		depBuf, 
		OUTERR_PIPE | MKF_OUT);
}

void CMakefile::SrcList2Buffers(CProjectView * Tree){

	// 1. Begin to fill each buffer.
	strcpy(depBuf, cc);
	if (*cppFlags != '\0'){
		strcat(depBuf, " ");
		strcat(depBuf, cppFlags);
	}
	strcat(depBuf, " -MM ");
	if (*incDirs != '\0'){
		strcat(depBuf, incDirs);
		strcat(depBuf, " ");
	}

	strcpy(srcBuf, "\nSRCS\t=\\\n");
	strcpy(objBuf, "\nOBJS\t=\\\n");
	resBuf [0] = 0;

	// 2. Parse the module list and retrieve sources files names.
	CFileItem* srcFile;

	if(!Tree->First())
		return; // The list is empty, nothing to search.

	do {	srcFile = (CFileItem *) Tree->GetCurrent();

		if (srcFile->type == C_FILE || srcFile->type == RC_FILE){
			// Source files and objects buffers.
			strcat (srcBuf, "\t");
			strcat (srcBuf, &srcFile->szFileName[srcFile->nFileOffset]);
			strcat (srcBuf, "\\\n");

			// Change file extension.
			char ext[] = "o";
			strcpy(objFile, &srcFile->szFileName[srcFile->nFileOffset]);
			ChangeFileExt(objFile, ext);
			strcat (objBuf, "\t");
			strcat (objBuf, objFile);
			strcat (objBuf, "\\\n");

			if (srcFile->type == C_FILE){
				// Dependencies buffer.
				strcat(depBuf, &srcFile->szFileName[srcFile->nFileOffset]);
				strcat(depBuf, " ");
			}else if (srcFile->type == RC_FILE){
				// Resource buffer.
				strcat (resBuf, objFile);
				strcat (resBuf, ": ");
				strcat (resBuf, &srcFile->szFileName[srcFile->nFileOffset]);
				strcat (resBuf, "\n\n");
			}
		}
	} while (Tree->Next());

	int len = strlen(srcBuf);
	srcBuf[len-2] = '\n';
	srcBuf[len-1] = 0;

	len = strlen(objBuf);
	objBuf[len-2] = '\n';
	objBuf[len-1] = 0;
}

void CMakefile::Write(void){
	FILE * file;

	file = fopen(szFileName, "w");
	if (!file){
		MsgBox.DisplayString("Can't open file :\r\n%s", szFileName);
		return;
	}

	/* Signature */
	fprintf (file, "# Generated automatically by Visual-MinGW.\n");
	fprintf (file, "# http://visual-mingw.sourceforge.net/\n");

	/* Standard defines */
	fprintf (file, "\nCC = gcc\nWRES = %s", 			wres		);
	fprintf (file, "\nCPPFLAGS = %s", 				cppFlags	);

	if (buildWhat == BUILD_GUIEXE)
		fprintf (file, "\nLDBASEFLAGS = -mwindows %s %s", 	ldOpts, ldLibs	);
	else
		fprintf (file, "\nLDBASEFLAGS = %s %s", 		ldOpts, ldLibs	);
	fprintf (file, "\nINCDIRS = %s",				incDirs	);
	fprintf (file, "\nOPTIMIZ = %s",				optimize	);
	fprintf (file, "\nSTRIP = %s",					ldStrip	);
	/* Debug symbols ? Language ? */
	fprintf (file, "\n\nifeq ($(MAKECMDGOALS),debug)");
	if (lang == LANGCPP){
		fprintf (file, "\nCXXFLAGS = %s $(INCDIRS) -g %s", warning, cFlags);
		fprintf (file, "\nLDFLAGS = $(LDBASEFLAGS)");
		fprintf (file, "\nelse");
		fprintf (file, "\nCXXFLAGS = %s $(INCDIRS) $(OPTIMIZ) %s", warning, cFlags);
		fprintf (file, "\nLDFLAGS = $(STRIP) $(LDBASEFLAGS)");
		fprintf (file, "\nendif");
	}else{
		fprintf (file, "\nCFLAGS = %s $(INCDIRS) -g %s", warning, cFlags);
		fprintf (file, "\nLDFLAGS = $(LDBASEFLAGS)");
		fprintf (file, "\nelse");
		fprintf (file, "\nCFLAGS = %s $(INCDIRS) $(OPTIMIZ) %s", warning, cFlags);
		fprintf (file, "\nLDFLAGS = $(STRIP) $(LDBASEFLAGS)");
		fprintf (file, "\nendif");
	}
	/* Directories */
	fprintf (file, "\n\nSRCDIR = %s", 							mkfDir	);
	fprintf (file, "\nBINDIR = %s", 							tgtDir		);
	fprintf (file, "\nLIBDIRS = %s",							libDirs	);
	/* Rule to compile rc files */
	fprintf (file, "\n\n%c.o : %c.rc\n\t$(WRES) $< $@", 	'%', '%'				);
	/* List of objects */
	fprintf (file, "\n%s", 									objBuf	);
	/* Target */
	fprintf (file, "\nTARGET =\t$(BINDIR)\\%s" , 					target	);
	/* all, alldebug */
	fprintf (file, "\n\n# Targets\n"									);
	fprintf (file, "all:\t$(TARGET)\n\ndebug:\t$(TARGET)\n\n"									);
	/* clean */
	fprintf (file, "cleanobjs:\n\trm -f $(OBJS)\n\n"							);
	fprintf (file, "cleanbin:\n\trm -f $(TARGET)\n\n"							);
	fprintf (file, "clean:\tcleanobjs cleanbin\n\n"							);
	/* Dependencies */
	fprintf (file, "# Dependency rules\n"									);
	fprintf (file, "$(BINDIR)\\%s", 								target	);
	/* Language */
	if (lang == LANGCPP)
		fprintf (file, ": $(OBJS)\n\t$(CXX) -o $(BINDIR)\\%s", 		target	);
	else
		fprintf (file, ": $(OBJS)\n\t$(CC) -o $(BINDIR)\\%s", 			target	);
	fprintf (file, " $(OBJS) $(INCDIRS) $(LIBDIRS) $(LDFLAGS)\n\n%s", 	resBuf	);
	fclose(file);
}

bool CMakefile::SendToMakefile(char * line){
	// @@ TODO revise this function
	FILE * file;

	file = fopen(szFileName, "a");
	if (!file)
		return false;

	fprintf(file, "%s\n", line);
	fclose(file);
return true;
}


/********************************************************************
*	Class:	CCompiler.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CCompiler::CCompiler(){
	make[0] 		= '\0';
	cc[0] 		= '\0';
	cFlags[0] 		= '\0';
	ldFlags[0]		= '\0';
	wres[0] 		= '\0';
	debug[0] 		= '\0';
	test[0] 		= '\0';
}

CCompiler::~CCompiler(){
}

bool CCompiler::LoadData(char * fullpath){

	Load(fullpath);
	// [Common] section
	GetString(make, 		"MAKE", 		"Common");
	GetString(cc,		"CC"	 		);
	GetString(cFlags, 	"CFLAGS" 		);
	GetString(ldFlags, 	"LDFLAGS" 		);
	GetString(wres, 		"WRES" 		);
	GetString(debug, 	"DEBUG" 		);
	GetString(test, 		"TEST" 		);

return ShowError();
}

