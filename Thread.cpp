/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include "Thread.h"


unsigned int Thread::NextThreadID = 0;
Mutex Thread::mutex;

//m�odo respons�el por chamar o run
static void* ThreadFunction(void* object)
{
	Thread* thread = (Thread*)object;
        return thread->Run(thread->Param);
}


//Construtora
Thread::Thread( void )
{
        Started = false;
        Detached = false;
	running = false;

}

//Destrutora
Thread::~Thread( void )
{
	running = false;
        Stop();
}

//Cria uma nova thread, e completa
int Thread::Start( void* param )
{
        if(!Started)
        {
		running = true;
                pthread_attr_t attributes;
                pthread_attr_init(&attributes);
                if(Detached)
                        pthread_attr_setdetachstate(&attributes,PTHREAD_CREATE_DETACHED);

                Param = param;
                //ThreadID = GetNextThreadID();
                if(pthread_create(&ThreadHandle,&attributes, ThreadFunction,this) == 0 )
                        Started = true;

                pthread_attr_destroy(&attributes);

        }

        return Started;
}


void Thread::Detach(void)
{
        if(Started && !Detached)
                pthread_detach(ThreadHandle);

        Detached = true;
}

void* Thread::Wait(void)
{
        void* status = NULL;
        if(Started && !Detached)
        {
                pthread_join(ThreadHandle,&status);
                Detached = true;
        }

        return status;

}

void Thread::Stop(void)
{
        if(Started && !Detached)
        {
                pthread_cancel(ThreadHandle);
                pthread_detach(ThreadHandle);
                Detached = true;
        }

}


unsigned int Thread::GetThreadID(void)
{
        return ThreadID;
}

unsigned int Thread::GetCurrentThreadID(void)
{
        return NextThreadID;
}


void Thread::SetThreadID(int x)
{
	mutex.Acquire();
        ThreadID = x;
	mutex.Release();
}

void Thread::Sleep(int delay)
{
        timeval timeout = {( delay / 1000 ),( (delay*1000) % 1000000 )};
        select( 0, (fd_set*)NULL, (fd_set*)NULL, (fd_set*)NULL, &timeout);
}


void* Thread::Run(void* param)
{
        return NULL;

}

unsigned int Thread::GetNextThreadID(void){
	mutex.Acquire();
	unsigned int thread_id = ++NextThreadID;
	mutex.Release();
	return thread_id;

}

void Thread::setRunning(int x){
	mutex.Acquire();
	running = x;
	mutex.Release();
}



bool Thread::getStatus(){

	return running;

}






