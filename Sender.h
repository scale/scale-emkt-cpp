/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _SENDER_HEADER
#define _SENDER_HEADER

#include "Mailer.h"
#include "Mutex.h"
#include "Thread.h"
#include "Debug.h"
#include "global.h"
#include "QueueManager.h"


class Sender : public Thread {

public:
	Sender(int status);
	~Sender();

	ErrorMessages_t* getErrorMessages();
	bool setEmailSouces(emailSource_t& emailsources);
	void* tratandoErros(ErrorMessages_t em, int id_peca, int id_campanha);

private:
	virtual void* Run(void*);
	int id;
	Mutex mutex;
	ErrorMessages_t em;

};


#endif

