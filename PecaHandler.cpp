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

PecaHandler::PecaHandler() {
	//	mutex.Acquire();
	//	mutex.Release();
	debug = new Debug("PecaHandler");
}

PecaHandler::~PecaHandler() {
	delete debug;
}

void PecaHandler::resetEnvio(Peca &peca) {
	//Definindo o stats de que esta em andamento
	database.executeQuery("update EmktPeca set stats=1 where "
		"id_peca='%d' and id_campanha='%d'", peca.pecaId, peca.campanhaId);
}

void PecaHandler::lePecasAtivas() {

	vector<Peca> v;

	copy(pecas.begin(), pecas.end(), v);
	try {
		Pointer
				pointer("select * from "
						"EmktPeca P inner join EmktCampanha C using(id_campanha, id_peca) "
						"where P.data_enviar <= now() and C.stats = 0 and P.stats != 2");

		while (pointer.next()) {

			Peca peca;
			peca.from = pointer.get("email_resposta");
			peca.subject = pointer.get("subject");
			peca.txt = pointer.get("corpoTXT");
			peca.html = pointer.get("corpoHTML");
			peca.html += " ";

			peca.campanhaId = atoi(pointer.get("id_campanha").c_str());
			peca.pecaId = atoi(pointer.get("id_peca").c_str());

			debug->debug("Peca:%d/%d - %s", peca.campanhaId, peca.pecaId,
					peca.subject.c_str());

			vector<Peca>::iterator it;
			for (it = v.begin(); it != v.end(); it++) {
				if ((*it) == peca) {
					(*it).ativa = true;

					//excluir aqui
					v.erase(it);
					continue;
				}
			}
			peca.ativa = true;
			pecas.push_back(peca);
		}

		// desativar as remanescentes
		vector<Peca>::iterator it;
		for (it = v.begin(); it != v.end(); it++) {
			(*it).ativa = false;
		}

	} catch (DBException dbe) {
		debug->error("%s", dbe.err_description.c_str());
	}
}

void*
PecaHandler::Run(void* param) {

	Peca peca;

	lePecasAtivas();

	vector<Peca>::iterator it;
	for (it = pecas.begin(); it != pecas.end(); it++) {
		resetEnvio(*it);
	}

	while (1) {
		lePecasAtivas();
		Sleep(15);
	}

	return this;
}

void
PecaHandler::finalizar_peca(const Peca &p) {
	try {
		Database database;
		debug->info("Finalizando Peca: id_campanha='%d', id_peca='%d'",
				p.campanhaId, p.pecaId);
		database.executeQuery(
				"update EmktPeca set stats=2 where id_campanha='%d' and id_peca='%d'",
				p.campanhaId, p.pecaId);

		p.ativa = false;
	} catch (DBException dbe) {
		debug->error("Peca %d - ERRO: %s", p.pecaId, dbe.err_description.c_str());
	}
}

