/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "PecaHandler.h"
#include "QueueManager.h"
#include "unistd.h"
#include <iostream>
#include <memory>
using namespace std;

PecaHandler::PecaHandler(int peca, int campanha) {
	mutex.Acquire();

	dead = false;
	SetThreadID(peca + campanha * 100);
	id_peca = peca;
	id_campanha = campanha;

	debug.debug("PecaHandler::PecaHandler -- campanha = %d / peca = %d ",
			id_campanha, id_peca);

	mutex.Release();
}

PecaHandler::~PecaHandler() {
	Stop();
	dead = true;
}

void PecaHandler::init() {
	//Se o programa retornou sem ter excluido os emails da fila
	//é porque não foram entregues, então ao reiniciar recolocamos todos
	//na fila e caso seja a primeira, nao ira mudar nada
	database.executeQuery("update EmktFilaEnvioPeca set "
		"stats=0 where id_peca=%d and id_campanha=%d",
		id_peca, id_campanha);

	//Definindo o stats de que esta em andamento
	database.executeQuery("update EmktPeca set stats=1 where "
		"id_peca='%d' and id_campanha='%d'",
		id_peca, id_campanha);
}

void* PecaHandler::Run(void* param) {

	Peca peca;
	Sender* sender[MAX_THREADS_ENVIO];
	map<string, Sender> mapSenders;

	int num_threads = 0; //Numero de threads rodando
	int vez = 0;

	emailSource_t esTemp[MAX_THREADS_ENVIO];

	init();

	for (int i = 0; i < MAX_THREADS_ENVIO; i++)
		sender[i] = NULL;

	int queueId = 0;
	int status_running = MAX_THREADS_ENVIO;

	vector < ErrorMessages_t > vErrorMessages;
	emailSource_t es;

	ErrorMessages_t erro_vez;

	try {
		Pointer pointer(conn,
				"select * from EmktPeca where id_peca=%d and id_campanha=%d ",
				id_peca, id_campanha);

		if (pointer.next()) {

			peca.from = pointer.get("email_resposta");
			peca.subject = pointer.get("subject");
			peca.txt = pointer.get("corpoTXT");
			peca.html = pointer.get("corpoHTML");
			peca.html += " ";

			peca.campanhaId = id_campanha;
			peca.pecaId = pecaId;

			debug.debug("Peca:%d/%d - %s",
					peca.campanhaId, peca.pecaId,
					peca.subject.c_str());

		} else {
			setRunning(false);
			debug.info("Nao ha pecas para campanha %d", id_campanha);
			dead = true;
			return NULL;
		}

		DNS _dns(DNS);
		Pointer pointer(
						conn,
						"select distinct dominio(email) as dominio from EmktFilaEnvioPeca "
						"where id_peca=%d and id_campanha=%d ",
						id_peca, id_campanha);

		while (pointer.next()) {
			vector < string > vdd;
			string domain = pointer.get("dominio");

			_dns.GetMX(ddv);
			debug.debug("Num. MX para %s: %d", domain.c_str(), vdd.size());
			servidoresMX[domain] = vdd;
		}

	} catch (DBException dbe) {
		debug.error("%s", dbe.err_description.c_str());
		dead = true;
		return NULL;
	}

	// Iniciando o procedimentos para comecar o envio!
	int finalizar = 0;

	while (status_running > 0) {

		map<string, vector<string> >::iterator it;
		map<string, auto_ptr<Sender> >::iterator itSender;

		for (it = servidoresMX.begin(); it != servidoresMX.end();) {
			string domain = (*it).first;
			vector < auto_ptr<Sender> > v_senders = (*it).second;

			Pointer
					pointer(
							s_CI,
							"select * from EmktFilaEnvioPeca where id_peca=%d and id_campanha=%d and dominio(email) ='%s' limit 1 ",
							id_peca, id_campanha, domain);

			if (pointer.total_record_set == 0) {
				servidoresMX.erase(it++);
			} else {
				debug.debug("%d emails na fila para: %s",
						pointer.total_record_set, domain);

				if (pointer.next()) {
					for (vector<string>::iterator itv = mxs.begin(); itv
							!= mxs.end(); ++itv) {
						string mx = (*itv).second;

						itSender = mapSenders.find(mx);

						if (isSender == mapSenders.end()) {
							auto_ptr<Sender> ptr(new Sender);
							mapSenders[mx] = ptr;
						}

						if (sender.getStatus() == 0) {
							mxs.erase(itv);
							continue;
						}

sender					.
				}
			}
			++it;
		}

		Sleep(1);

	}

	if (finalizar > 0) {
		debug.debug("PH[%d]::Run -- Finalizando peca", id_peca);
		break;
	}

}//FIM Do WHILE

try {
	Database database(s_CI);
	debug.info("Finalizando Peca: id_campanha='%d', id_peca='%d'",
			id_campanha, id_peca);
	database.executeQuery(
			"update EmktPeca set stats=2 where id_peca='%d' and id_campanha='%d'",
			id_peca, id_campanha);
} catch (DBException dbe) {
	debug.error("PH[%d]::DBException:: %s", id_peca,
			dbe.err_description.c_str());

}

debug.debug("PH[%d]::Run -- Fim", id_peca);
setRunning(false);
return NULL;

}

void* PecaHandler::tratarErros(ResultMessage& em, int id_peca, int id_campanha) {
	string emails_validos;

	for (unsigned int x = 0; x < em.id_email_error.size(); x++) {

		db.executeQuery("replace into EmktStatsEnvio "
			"(id_peca,id_campanha,error_code,email,enviado)"
			" values ('%d','%d','%d','%s',now())", id_peca, id_campanha,
				em.error, em.recipient.email.c_str());

		if (em.error >= 550) {
			db.executeQuery(
					"insert ignore into EmktListaNegra (email,code,mensagem) "
						"values ('%s',%d,'%s') ", em.recipient.email.c_str(),
					em.error, em.message.c_str());
		}

		if (em.error < 300 || em.error >= 500) {
			db.executeQuery("delete from EmktFilaEnvioPeca where "
				"id_campanha='%d' and id_peca='%d' and email = '%s'",
					id_campanha, id_peca, em.recipient.email.c_str());
		}

		debug.debug("Peca (%d/%d): %d - %s ", id_campanha, id_peca, em.error,
				em.recipient.email.c_str());
	}

	return NULL;
}
