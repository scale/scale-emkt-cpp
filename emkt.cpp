/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#ifdef  __cplusplus
}
#endif

#include "global.h"
#include "Pointer.h"
#include "PecaHandler.h"
#include "QueueManager.h"
#include "Mailer.h"
#include <fstream>
#include <iostream>

using namespace std;

QueueManager queue;
PecaHandler peca_handler;

void leConfiguracao();

int main() {
	Debug debug("main");
	debug.info("Iniciando o PROGRAMA DE ENVIO!");

	leConfiguracao();

	peca_handler.Start();
	queue.Start();

	while (1) {

		Thread::Sleep(30);
	} //FIM DO While forever

	debug.info("Exiting...");

	return 0;
}

void leConfiguracao() {
	ifstream hfile(CONF_DIR);

	debug.info("Configuration file: %s", CONF_DIR);

	string temp;
	while (!hfile.eof()) {
		getline(hfile, temp);

		if (temp.find("database") == 0) {
			Database::db = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("host") == 0) {
			Database::host = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("user") == 0) {
			Database::user = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("pass") == 0) {
			Database::pass = temp.substr(temp.find("=") + 1, temp.length());
		}

		if (temp.find("DNS") == 0) {
			DNS = temp.substr(temp.find("=") + 1, temp.length());
		}
		if (temp.find("instante_id") == 0) {
			INSTANCE_NUM = atoi(
					temp.substr(temp.find("=") + 1, temp.length()).c_str());
		}
	}

	debug.info("Parametros encontrados: %s@%s:/%s - DNS: %s para instancia %d",
			Database::user.c_str(), Database::host.c_str(),
			Database::db.c_str(), DNS.c_str(), INSTANCE_NUM);

	hfile.close();
}

