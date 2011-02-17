/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _THREAD_HEADER
#define _THREAD_HEADER

#include "Mutex.h"
#include <iostream>

#ifdef  __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef  __cplusplus
}
#endif

static void* ThreadFunction(void*);

class Thread {

	friend void* ThreadFunction(void*);

public:
	Thread(void);
	virtual ~Thread(void);
	int Start(void* = NULL);
	void Detach(void);
	void* Wait(void);
	void Stop(void);
	unsigned int GetThreadID(void);
	void SetThreadID(int x);
	static unsigned int GetCurrentThreadID(void);
	static void Sleep(int);
	bool getStatus();

protected:
	virtual void* Run(void*) = 0;
	void setRunning(int x);

private:
	pthread_t ThreadHandle;
	unsigned int ThreadID;
	static Mutex mutex;
	unsigned int GetNextThreadID(void);
	static unsigned int NextThreadID;
	int Started;
	int running;
	int Detached;
	void* Param;

};

#endif
