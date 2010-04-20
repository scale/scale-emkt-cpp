/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _MUTEX_HEADER
#define _MUTEX_HEADER


#ifdef  __cplusplus
extern "C" {
#endif

#include "pthread.h"

#ifdef  __cplusplus
}
#endif

class Mutex 
{

public:
	Mutex(void);
	virtual ~Mutex(void);
	virtual void Acquire(void);
	virtual int Acquired(void);
	virtual void Release(void);
private:
	pthread_mutex_t m_Mutex;

};

#endif // fim do _MUTEX_HEADER
