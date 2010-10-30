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
#include "Peca.h"

#include <map>

class PecaHandler : public Thread {

public:
	PecaHandler(int peca, int campanha, int total_emails = 1);
	~PecaHandler();

	bool dead() { return dead; }

private:
	virtual void* Run(void*);
	void* tratarErros(ResultMessage& em, int id_peca, int id_campanha)
	void init();

	int id;
	Mutex mutex;
	int id_peca;
	int id_campanha;
	int total_emails;

	bool dead;

	Database database(conn);

	map<string, vector< auto_ptr<Sender> > > servidoresMX;
};


#endif
