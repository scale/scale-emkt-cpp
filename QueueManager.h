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

typedef struct dadosPessoa {

string email;
string nome;
string id;

} dadosPessoa_t;

typedef vector<dadosPessoa_t> vDadosPessoa;

class QueueManager{

public:
	QueueManager(Connection_Info_t* s_ConInfo, int peca, int campanha, int total_emails = 1, int tpeca = 0 );
	~QueueManager();
	static bool returnQueue(const char* email);
	bool returnQueue();
	void statsInsert(const char* email, int code);
	static void statsInsert(const char* email, int code, int peca, int campanha);
	void eraseQueue(const char* email);
	static void eraseQueue(const char* email, int id_peca, int id_campanha);
	vDadosPessoa getEmails(int threadId);
	static Connection_Info_t static_CI;
	
private:
	Debug* debug;
	int id;
	Database* database;
	int block_size;
	string domain;
	int id_peca;
	int id_campanha;
	int terminate;
	Connection_Info_t s_CI;
	int tipo_peca;

};


#endif
