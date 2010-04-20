/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

 #include "Mutex.h"

Mutex::Mutex(void){
	pthread_mutex_init(&m_Mutex,(pthread_mutexattr_t*)NULL);
}

Mutex::~Mutex(void){
	pthread_mutex_destroy(&m_Mutex);
}

void Mutex::Acquire(void){
	pthread_mutex_lock(&m_Mutex);
}

int Mutex::Acquired(void){
	return (pthread_mutex_trylock(&m_Mutex) == 0);
}

void Mutex::Release(void){
	pthread_mutex_unlock(&m_Mutex);
}

