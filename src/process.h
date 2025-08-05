/********************************************************************
*	Module:	process.h. This is part of Visual-MinGW.
*
*	License:	Visual-MinGW is covered by GNU General Public License, 
*			Copyright (C) 2001  Manu B.
*			See license.htm for more details.
*
********************************************************************/
#ifndef PROCESS_H
#define PROCESS_H

#include "CList.h"

/********************************************************
WORD 
(CTask)->creationFlag
					80	40	20	10	8	4	2	1
					-----------------------------------------
(list view out + no pipes)			0	0
		MKF_OUT				1	
		ENABLE_PIPES				1	0	0	0	0
		IN_PIPE					1	1	0	0	0
		OUT_PIPE					1	0	1	0	0
		ERR_PIPE					1	0	0	1	0
		OUTERR_PIPE				1	0	1	0	1

*********************************************************/
#define MKF_OUT		0x0020
#define ENABLE_PIPES		0x0010
#define IN_PIPE			0x0018
#define OUT_PIPE		0x0014
#define ERR_PIPE			0x0012
#define OUTERR_PIPE		0x0015

class CTask : public CNode
{
	public:
	CTask();
	~CTask();

	char * 	cmdLine;
	WORD	creationFlag;

	protected:

	private:

};

class CStack : public CList
{
	public:
	CStack();
	~CStack();

	int Push(CTask * newTask);
	CTask * Pop(void);
	void Flush(void);

	protected:

	private:   
	CTask * retBuf;
	void DetachCurrent(void);
};

class CPipes
{
	public:
	CPipes();
	~CPipes();

	HANDLE stdIn[2];
	HANDLE stdOut[2];
	HANDLE stdErr[2];

	bool Create(WORD creationFlag, bool winNT);
	bool CloseChildSide(void);
	bool CloseParentSide(void);
	protected:

	private:
	bool Close(int side);
};

class CProcess : public CStack
{
	public:
	CProcess();
	~CProcess();

	bool isRunning(void);
	void AddTask(char * cmdLine, WORD creationFlag);
	bool CmdCat(char * cmdLine);
	void Run(void);
	void Run_Thread_Internal(void);

	protected:

	private:
	PROCESS_INFORMATION pi;
	bool 		Running;
	DWORD 	exitCode;
	CTask * 	currTask;
	CPipes	Pipes;

	int 	nextChar;
	char 	inBuf[1024];
	char 	outBuf[1024];
	char 	errBuf[1024];

	bool	RunProcess(CTask * task);

	void	WriteStdIn(HANDLE hPipe, WORD creationFlag);
	void	ReadStdOut(HANDLE hPipe, WORD creationFlag);
	void	ReadStdErr(HANDLE hPipe, WORD creationFlag);

	int	ReadChar(HANDLE hPipe, char * chrin);
	bool	SendToOutput(char * line, WORD creationFlag);

};

#endif
