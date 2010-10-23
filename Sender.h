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
#include "global.h"
#include "QueueManager.h"


typedef struct emailSource {
vDadosPessoa to;
string from;
string subject;
string body_txt;
string body_html;
string DNS;
string errors_to;
string id_camp_peca;
int id_peca;
int id_campanha;

	bool isValido() {
		Debug debug(1,"Inicio");
		if( strlen(DNS.c_str()) < 7 ) {
			debug.error("ERRO: DNS invalido? {%s}", DNS.c_str()); return false;
		}
		if( strlen(body_txt.c_str()) < 7 ) {
			debug.error("ERRO: Body TXT invalido."); return false;
		}
		if( strlen(body_html.c_str()) < 10 ) {
			debug.error("ERRO: Body HTML invalido."); return false;
		}
		if( strlen(subject.c_str()) < 3 ) {
			debug.error("ERRO: Subject invalido? {%s}", subject.c_str()); return false;
		}
	}
} emailSource_t;

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
	emailSource_t es;

};


#endif

