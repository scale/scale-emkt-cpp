/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <mysql/mysql.h>
#include "string.h"
using namespace std;

#include "Pointer.h"
#include "ErrorPop.h"
#include "global.h"
#include "PecaHandler.h"
#include "QueueManager.h"
#include "Mailer.h"
#include "unistd.h"
#include "stdlib.h"
#include <fstream>


QueueManager qm;

void leConfiguracao();

int main() {
	Debug debug("main");
	debug.info("Iniciando o PROGRAMA DE ENVIO!");

	PecaHandler* ph[MAX_THREADS_PECA]; //Matriz de Threads que deverao rodar

	leConfiguracao();

	for (int i = 0; i < MAX_THREADS_PECA; i++) {
		ph[i] = NULL;
	}

	try {
		//Para tratar os erros que retornam para a tabela EmktPop pelo programa procmailemkt
		//colocado no MTA para desviar os emails da conta de retorno da peca para o Banco
		//ErrorPop ep(&conn);
		//ep.Start();

		debug.debug("Conectando ao BD...");
		Database database(conn);
		qm.Start();

		//caso o programa tenha caido durante um envio voltar a peca ao status para ser entregue
		database.executeQuery(
				"update EmktPeca P inner join EmkyCampanha C using(id_campanha) "
					"set P.stats = 0 where P.stats = 1 and C.stats = 0");

		while (1) {
			Pointer
					pointer(
							conn,
							"select P.* from EmktPeca P inner join EmktCampanha C using(id_campanha) "
								"where P.data_enviar <= now() and P.stats = 0 and (C.stats = 0 or C.id_campanha = 0) "
								"order by P.id_peca,P.id_campanha");

			if (pointer.getTotal() < 1) {
				sleep(30);
				continue;
			}

			while (pointer.getNext()) {
				int slots_free = 0;

				for (int vez = 0; vez < MAX_THREADS_PECA; vez++) {

					//este tqm.ad deve morrer mas antes passar o "relatorio" do envio
					if (ph[vez] != NULL && ph[vez]->getStatus() == 0) {
						delete (ph[vez]);
						ph[vez] = NULL;
					}

					if (ph[vez] != NULL) continue;

					int id_campanha = 0;
					int id_peca = 0;
					slots_free++;

					id_peca = atoi(pointer.get("id_peca"));
					id_campanha = atoi(pointer.get("id_campanha"));

					debug.info("Iniciando peca %d/%d", id_campanha, id_peca);
					ph[vez] = new PecaHandler(id_peca, id_campanha);

					ph[vez]->Start();

				} // FIM FOR

				if (slots_free == 0) break;
			}

			sleep(30);
		} //FIM DO WHile forever

	} catch (DBException dbe) {
		debug.error("DBException: %s", dbe.err_description.c_str());

	}

	debug.info("SAINDO DO PROGRAMA");

	return 0;
}

void leConfiguracao() {
	ifstream hfile(CONF_DIR);

	debug.info("Configuration file: %s", CONF_DIR);

	string temp;
	while (!hfile.eof()) {
		getline(hfile, temp);

		if (temp.find("database") == 0) {
			conn.db = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("host") == 0) {
			conn.host = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("user") == 0) {
			conn.user = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("pass") == 0) {
			conn.pass = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("DNS") == 0) {
			DNS = temp.substr(temp.find("=") + 1, temp.length());
		}
		if (temp.find("instante_id") == 0) {
			INSTANCE_NUM = atoi( temp.substr(temp.find("=") + 1, temp.length()).c_str() );
		}
	}

	debug.info("Parametros encontrados: %s@%s:/%s - DNS: %s para instancia %d",
			conn.user.c_str(), conn.host.c_str(), conn.db.c_str(), DNS.c_str(), INSTANCE_NUM);

	hfile.close();
}

