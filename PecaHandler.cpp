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

#include "unistd.h"
#include <iostream>
#include <memory>
using namespace std;

PecaHandler::PecaHandler(int peca, int campanha, int total_emails = 1) {
	//	mutex.Acquire();

	debug.debug("PecaHandler::PecaHandler -- campanha = %d / peca = %d ",
			id_campanha, id_peca);

	//	mutex.Release();
}

PecaHandler::~PecaHandler() {

}

void PecaHandler::resetEnvio(Peca &peca) {
	//Definindo o stats de que esta em andamento
	database.executeQuery("update EmktPeca set stats=1 where "
		"id_peca='%d' and id_campanha='%d'", peca.pecaId, peca.campanhaId);
}

void*
PecaHandler::Run(void* param) {

	Peca peca;

	resetEnvio();

	int status_running = MAX_THREADS_ENVIO;

	try {
		Pointer pointer(
				"select * from EmktPeca P inner join EmktCampanha C using(id_campanha, id_peca) "
				"where P.data_enviar = date_add(now(), interval -14 day) and C.stats = 0 and P.stats != 2");

		while (pointer.next())
		{

			peca.from = pointer.get("email_resposta");
			peca.subject = pointer.get("subject");
			peca.txt = pointer.get("corpoTXT");
			peca.html = pointer.get("corpoHTML");
			peca.html += " ";

			peca.campanhaId = atoi(pointer.get("id_campanha"));
			peca.pecaId = atoi(pointer.get("id_peca"));

			debug.debug("Peca:%d/%d - %s", peca.campanhaId, peca.pecaId,
					peca.subject.c_str());

		}

		DNS _dns(DNS);
		Pointer pointer(
				"select distinct dominio(email) as dominio from EmktFilaEnvioPeca "
					"where id_peca=%d and id_campanha=%d and id_thread = %d",
				id_peca, id_campanha, INSTANCE_NUM);

		while (pointer.next()) {
			vector<string> vdd;
			string domain = pointer.get("dominio");

			_dns.GetMX(ddv);
			debug.debug("Num. MX para %s: %d", domain.c_str(), vdd.size());
			servidoresMX[domain] = vdd;
		}

	} catch (DBException dbe) {
		debug.error("%s", dbe.err_description.c_str());
		_dead = true;
		return NULL;
	}

	// Iniciando o procedimentos para comecar o envio!
	int finalizar = 0;

	while (status_running > 0) {

		map<string, vector<string> >::iterator it;
		map<string, auto_ptr<Sender> >::iterator itSender;

		for (it = servidoresMX.begin(); it != servidoresMX.end();) {
			string domain = (*it).first;
			vector<auto_ptr<Sender> > v_senders = (*it).second;

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

						//sender					.
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

DNS = "";
debug = new Debug("PecaHandler");

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

