/********************************************************************
*	Module:	process.cpp. This is part of Visual-MinGW.
*
*	Purpose:	Procedures to invoke MinGW compiler.
*
*	Authors:	Manu B.
*
*	License:	Visual-MinGW is covered by GNU General Public License, 
*			Copyright (C) 2001  Manu B.
*			See license.htm for more details.
*
*	Note:		The following article from MSDN explanes how to handle Callback
*			procedures :
*						Calling All Members: Member Functions as Callbacks.
*						by Dale Rogerson.
*						Microsoft Developer Network Technology Group.
*						April 30, 1992.
*						http://msdn.microsoft.com/archive/default.asp
*
*	Revisions:	
*
********************************************************************/
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include "process.h"
#include "project.h"
#include "main.h"
#include "rsrc.h"

extern CMessageBox MsgBox;

// For winApp.isWinNT and winApp.ListView.SendMsg
extern CWinApp winApp;

// @@ to acces to Project->Makefile->SendToMakefile();
extern CProject Project;


/********************************************************************
*	Class:	CTask.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CTask::CTask(){
	cmdLine 		= new char [MAX_PATH];
	creationFlag 	= 0;
}

CTask::~CTask(){
	delete [] cmdLine;
}


/********************************************************************
*	Class:	CStack.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CStack::CStack(){
	retBuf = NULL;
}

CStack::~CStack(){
	DestroyList();
	if (retBuf)
		delete retBuf;
}

void CStack::DetachCurrent(void){
	// Empty list ?
	if (current != NULL){
		CNode * node = current;

		// Detach node from the list.
		if (node->next != NULL)
			node->next->prev = node->prev;
		if (node->prev != NULL)
			node->prev->next = node->next;
	
		// Set current node.
		if(node->next != NULL)
			current = node->next;
		else
			current = node->prev;

		if (current == NULL){
			// Now, the list is empty.
			first = last = NULL;

		}else if (first == node){
			// Detached node was first.
			first = current;

		}else if (last == node){
			// Detached node was last.
			last = current;
		}
		count--;
	}
}

/********************************************************************
*	Push/Pop/Flush.
********************************************************************/
int CStack::Push(CTask * newTask){
	InsertLast(newTask);
return Length();
}

CTask * CStack::Pop(void){
	// Delete return buffer.
	if (retBuf){
		delete retBuf;
		retBuf = NULL;
	}

	// Get first node. (FIFO stack)
	retBuf  = (CTask*) First();

	// The Stack is empty ?
	if (!retBuf)
		return NULL;

	// Detach current node from the list. Return a pointer to it.
	DetachCurrent();
return retBuf;
}

void CStack::Flush(void){
	DestroyList();
	if (retBuf)
		delete retBuf;
	retBuf = NULL;
}


/********************************************************************
*	Class:	CPipes.
*
*	Purpose:	Creates needed pipes, depending on creationFlag. 
*		Like GNU Make does, we use an Handle array for our pipes.
*		Parent Process Side is stdXXX[0] and Child Process Side is stdXXX[1].
*
*		Ex:	PARENT ->[0]IN_PIPE[1]-> 	CHILD_IO	->[1]OUT_PIPE[0]-> PARENT
*										->[1]ERR_PIPE[0]-> PARENT
*	Revisions:	
*
********************************************************************/
CPipes::CPipes(){
	stdIn[0] 	= NULL;
	stdIn[1] 	= NULL;
	stdOut[0] 	= NULL;
	stdOut[1] 	= NULL;
	stdErr[0] 	= NULL;
	stdErr[1] 	= NULL;
}

CPipes::~CPipes(){
}

bool CPipes::Create(WORD creationFlag, bool winNT){
	// Create needeed pipes
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	if (winNT){
		// Create a security descriptor for Windows NT
		SECURITY_DESCRIPTOR sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
		sa.lpSecurityDescriptor = &sd;
	}
	// Parent side is [0], Child side is [1].
	if ((creationFlag & IN_PIPE) == IN_PIPE){
		CreatePipe(&stdIn[1], 		&stdIn[0], 	&sa, 0);
		SetHandleInformation(stdIn[0], HANDLE_FLAG_INHERIT, 0);
	}

	if ((creationFlag & OUTERR_PIPE) != OUTERR_PIPE){

		if ((creationFlag & ERR_PIPE) == ERR_PIPE){
			CreatePipe(&stdErr[0], 	&stdErr[1], 	&sa, 0);
			SetHandleInformation(stdErr[0], HANDLE_FLAG_INHERIT, 0);
		}
	}

	if ((creationFlag & OUT_PIPE) == OUT_PIPE){
		CreatePipe(&stdOut[0], 	&stdOut[1], 	&sa, 0);
		SetHandleInformation(stdOut[0], HANDLE_FLAG_INHERIT, 0);
	}

return true;
}

bool CPipes::CloseChildSide(void){
return Close(1);
}

bool CPipes::CloseParentSide(void){
return Close(0);
}

bool CPipes::Close(int side){

	if (side < 0 || side > 1)
		return false;

	if (stdIn[side]){
		CloseHandle(stdIn[side]);
		stdIn[side] = NULL;
	}

	if (stdOut[side]){
		CloseHandle(stdOut[side]);
		stdOut[side] = NULL;
	}

	if (stdErr[side]){
		CloseHandle(stdErr[side]);
		stdErr[side] = NULL;
	}
return true;
}


/********************************************************************
*	Class:	CProcess.
*
*	Purpose:	
*
*	Revisions:	
*
********************************************************************/
CProcess::CProcess(){
	Running = false;
	exitCode = 0;

	pi.hProcess		= 0; 
	pi.hThread		= 0; 
	pi.dwProcessId	= 0; 
	pi.dwThreadId	= 0; 
}

CProcess::~CProcess(){
}

/********************************************************************
*	Manage Tasks.
********************************************************************/
bool CProcess::isRunning(void){
	if (Running){
		MsgBox.DisplayWarning("A process is already running !");
		return true;
	}
return false;
}

void CProcess::AddTask(char * cmdLine, WORD creationFlag){
	CTask * newTask = new CTask;

	strcpy(newTask->cmdLine, cmdLine);
	newTask->creationFlag = creationFlag;
	Push(newTask);
}

bool CProcess::CmdCat(char * cmdLine){
	CTask * task = (CTask*) GetCurrent();
	if (!task)
		return false;

	strcat(task->cmdLine, cmdLine);
return true;
}

/********************************************************************
*	RunNext/Run/RunProcess.
********************************************************************/
void call_thread(void * ptr){
	/* C++ adapter */
	((CProcess *) ptr)->Run_Thread_Internal();
}

void CProcess::Run(void){
	// Check if something is already running before creating a thread.
	if (!Running){
		// Call Run_Thread_Internal()
		_beginthread(call_thread, 1024 * 1024, (void *) this);
/*		if (_beginthread(call_thread, 1024 * 1024, (void *) this) != -1)
			Running = true;*/
	}
}

void CProcess::Run_Thread_Internal(void){
	exitCode = 0;
	// Execute each task, one after each other.
	for ( ; ; ){
		// If previous task returns an error code, abort.
		if (exitCode != 0)
			break;
	
		// Get one task to execute.
		currTask = Pop();
	
		// Nothing to run.
		if (!currTask)
			break;

		/* Show command lines ?
		winApp.ListView.SendMsg(currTask->cmdLine, 0);*/

		if (!RunProcess(currTask)){
			MsgBox.DisplayString("Failed to Create Process");
			winApp.ListView.SendMsg("visual-mingw: Failed to Create Process", 1);
			winApp.ListView.SendMsg(currTask->cmdLine, 0);
			exitCode = 1;
			break;
		}
	}

	// Successful ?
	if (exitCode == 0)
		winApp.ListView.SendMsg("Performed successfully.", 0);
	else
		winApp.ListView.SendMsg("vm: Abort !", 1);

	Flush();
	Running = false;
return;
}

bool CProcess::RunProcess(CTask * task){
	bool created;
	nextChar = -1;	// Reset buffer state.
	outBuf[0] = 0;

	/* 1. Process creation using pipes */

	if ((task->creationFlag & ENABLE_PIPES) == ENABLE_PIPES){
		// Create needed pipes depending on creation flags.
		Pipes.Create(task->creationFlag, winApp.isWinNT);
	
		// Startup Info.
		STARTUPINFO si	= {sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0};
		si.dwFlags 		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow	= SW_HIDE;

		// A pipe for standard input ?
		if (Pipes.stdIn[1] != NULL)
			si.hStdInput 	= Pipes.stdIn[1];
		else
			si.hStdInput 	= ::GetStdHandle(STD_INPUT_HANDLE);
		
		// For std output, std error ?
		if ((task->creationFlag & OUTERR_PIPE) != OUTERR_PIPE){
			if (Pipes.stdOut[1] != NULL)
				si.hStdOutput 	= Pipes.stdOut[1];
			else
				si.hStdOutput 	= ::GetStdHandle(STD_OUTPUT_HANDLE);
			
			if (Pipes.stdErr[1] != NULL)
				si.hStdError 	= Pipes.stdErr[1];
			else
				si.hStdError 	= ::GetStdHandle(STD_ERROR_HANDLE);

		// Only for std output ?
		}else{
			if (Pipes.stdOut[1] != NULL){
				si.hStdOutput 	= Pipes.stdOut[1];
				si.hStdError 	= Pipes.stdOut[1];
			}else{
				si.hStdOutput 	= ::GetStdHandle(STD_OUTPUT_HANDLE);
				si.hStdError 	= ::GetStdHandle(STD_ERROR_HANDLE);
			}
		}

		// Piped process creation.
		pi.hProcess		= 0; 
		pi.hThread		= 0; 
		pi.dwProcessId	= 0; 
		pi.dwThreadId	= 0; 
	
		created = ::CreateProcess(NULL, task->cmdLine, NULL, NULL, TRUE, 0, NULL,
					/*startDir[0] ? startDir :*/ NULL, &si, &pi);

		// Before anything, close unneeded handles.
		Pipes.CloseChildSide();
	
		if(!created){
			// Creation failed. Close parent side handles and return.
			Pipes.CloseParentSide();
			return false;
		}

	/* 2. Process creation without pipes */

	}else{
		STARTUPINFO si = {sizeof(STARTUPINFO), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
						0, 0, 0, 0, 0};
		
		created = ::CreateProcess(NULL, task->cmdLine, NULL, NULL, FALSE, 0, NULL,
					/*startDir[0] ? startDir : */NULL, &si, &pi);
		if(!created)
			return false;
	}

	/* 3. Our process is running, perform I/O until terminated */
	Running = true;

	while(::WaitForSingleObject(pi.hProcess, (DWORD) 0) != WAIT_OBJECT_0){
		// Give some time to the child process.
		Sleep(100L);
		// Read/write if using pipes.
		WriteStdIn(	Pipes.stdIn[0], 	task->creationFlag);
		ReadStdOut(	Pipes.stdOut[0], 	task->creationFlag);
		ReadStdErr(	Pipes.stdErr[0], 	task->creationFlag);
	}

	/* 4. Process terminated */

	// Get exit code.
	::GetExitCodeProcess(pi.hProcess, &exitCode);

	// Close handles.
	::CloseHandle(pi.hProcess);
	if (pi.hThread != 0){
		::CloseHandle(pi.hThread);
		pi.hThread = 0;
	}
	Pipes.CloseParentSide();

return created;
}

/********************************************************************
*	Pipes input/output.
********************************************************************/
void CProcess::WriteStdIn(HANDLE hPipe, WORD){
	if (!hPipe)
		return;

return;
}

void CProcess::ReadStdOut(HANDLE hPipe, WORD creationFlag){
	if (!hPipe)
		return;

	char chrin[1]  = {0};

	for ( ; ; ){		
		// Read one char
		if (!ReadChar(hPipe, chrin))		// nothing to read!
			break;

		// Format output lines
		if (*chrin == '\r')			// ignore '\r'
			continue;

		nextChar++;			// There is something to read

		if (*chrin != '\n'){
			outBuf[nextChar] = *chrin;

			// @@TODO Manage Overflow.
		}
				
		// Default read char is End of line
		if (*chrin =='\n'){
			outBuf[nextChar] = 0;
			SendToOutput(outBuf, creationFlag);
			nextChar = -1;	// Reset buffer
		}
	}
}	

void CProcess::ReadStdErr(HANDLE hPipe, WORD){
	if (!hPipe)
		return;

return;
}

int CProcess::ReadChar(HANDLE hPipe, char * chrin){
	DWORD bytesRead = 0;
	DWORD bytesAvail = 0;

	if (!PeekNamedPipe(hPipe, chrin, (DWORD)1, &bytesRead, &bytesAvail, NULL))
		return 0;

	if (bytesAvail == 0)
		return 0;

	if (!ReadFile(hPipe, chrin, (DWORD)1, &bytesRead, NULL))
		return 0;

return bytesRead;
}

bool CProcess::SendToOutput (char * line, WORD creationFlag){

	// Output to makefile.
	if ((creationFlag & MKF_OUT) == MKF_OUT){
		Project.Makefile.SendToMakefile(line);
	}else{
	// Output error lines to List View.
		if (strstr (line, ": "))
			winApp.ListView.SendMsg(line, 1);
/*		else
			winApp.ListView.SendMsg(line, 0);*/
	}

return true;
}

