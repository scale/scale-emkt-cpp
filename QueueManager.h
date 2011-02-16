/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _QUEUEMANAGER_HEADER
#define _QUEUEMANAGER_HEADER

#include "global.h"
#include "Pointer.h"
#include "Mutex.h"
#include "Thread.h"
#include "Mailer.h"

#include <map>

class QueueManager : public Thread {

public:
	QueueManager();
	~QueueManager();

	vector<Address> getEmails(int threadId);

	void includePeca(Peca& peca);
	void cancelPeca(Peca& peca);

	void pause();
	void resume();
private:
	virtual void* Run(void* param);

	static bool returnQueue(const char* email);
	bool returnQueue();
	void statsInsert(const char* email, int code);
	static void statsInsert(const char* email, int code, const std::string& m, int peca, int campanha);
	void eraseQueue(const char* email);
	static void eraseQueue(const char* email, int id_peca, int id_campanha);

	void* tratarErros(ResultMessage& em, int id_peca, int id_campanha);

	int id;
	Database* database;
	int block_size;
	string domain;
	int id_peca;
	int id_campanha;
	int terminate;

	map<string, Mailer> servidoresMX;
	vector<Peca> pecas;
};


#endif
