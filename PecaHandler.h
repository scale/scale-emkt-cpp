/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _PECAHANDLER_HEADER
#define _PECAHANDLER_HEADER

#include "Mutex.h"
#include "Thread.h"
#include "Sender.h"
#include "Mailer.h"
#include "Database.h"

class PecaHandler : public Thread {

public:
	PecaHandler(int id, Connection_Info_t ci, int peca, int campanha, int total_emails = 1);
	void setDNS(string dns);
	~PecaHandler();

private:
	virtual void* Run(void*);
	int id;
	Mutex mutex;
	int id_peca;
	int id_campanha;
	int total_emails;
	Connection_Info_t s_CI;
	Debug* debug;
	string DNS;

};


#endif
