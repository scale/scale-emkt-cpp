/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "QueueManager.h"

#include <cstdlib>
#include <sstream>

QueueManager::QueueManager() {
	debug = new Debug("QueueManager");
}

QueueManager::~QueueManager() {
	Stop();
	delete debug;
}

void*
QueueManager::Run(void* param) {
	while (1) {
		vector<Message> messages;

		vector<Peca>::iterator it;
		for (it = pecas.begin(); it != pecas.end(); it++) {
			Peca peca = (*it);
			vector<Address> addresses = load_queue(peca);

			vector<Address>::iterator itA;
			for (itA = addresses.begin(); it != addresses.end(); it++) {

				std::auto_ptr<Message> msg(new Message());
				msg->peca(peca);

				msg->recipient = (*itA);

				std::string domain = msg->recipient.domain();

				check_for_best_mx(msg);

				messages.push_back(msg);
			}
		}



		debug->info("Queueing...");

		Sleep(30);
	}
	return this;
}

void QueueManager::include_peca(Peca& peca) {
	Pointer pt = new Pointer("select count(*) as c from EmktFilaEnvioPeca "
		"where id_peca=%d and id_campanha=%d", peca.pecaId, peca.campanhaId);

	int num_emails = 0;
	if (pt.next())
		num_emails = atoi(pt.get("c").c_str());

	if (num_emails > 0) {
		//Se o programa retornou sem ter excluido os emails da fila
		//é porque não foram entregues, então ao reiniciar recolocamos todos
		//na fila e caso seja a primeira, nao ira mudar nada
		database.executeQuery(
				"update EmktFilaEnvioPeca set stats=0, id_thread = %d "
					"where id_peca=%d and id_campanha=%d and id_thread = 0 "
					"limit %d", INSTANCE_NUM, peca.pecaId, peca.campanhaId,
				num_emails);

		pecas.push_back(peca);
	}
}

void QueueManager::cancel_peca(Peca& peca) {
	vector<Peca>::iterator it;

	for (it = pecas.begin(); it != pecas.end(); it++) {
		Peca p = *it;
		if (p == peca) {
			p.ativa = false;
			break;
		}
	}
}

bool QueueManager::return_queue() {
	try {
		result = database.select(
				"select id_peca from EmktPeca where date_add(data_enviar, "
					"INTERVAL 5 DAY) > now() and id_peca='%d' and"
					" id_campanha='%d'", id_peca, id_campanha);
		if (mysql_num_rows(result) > 0) {

			debug->debug("Erros temporarios marcados para nova tentativa");

			database.executeQuery(
					"update EmktFilaEnvioPeca set stats='0' where id_peca='%d' and"
						" id_campanha='%d' and stats='2'", id_peca, id_campanha);

		} else {
			debug->info("Excluindo os que ficaram na fila a mais do que o tempo limite");
			try {
				Pointer
						p2(
								"select * from EmktFilaEnvioPeca where id_peca='%d' and id_campanha='%d' and email not in (select email from EmktListaNegra) order by email",
								id_peca, id_campanha);
				while (p2.next()) {
					string email = p2.get("email");
					statsInsert(email.c_str(), 850);
					eraseQueue(p2.get("email"));

				}

			} catch (DBException dbe) {
				debug->warn("EMail muito tempo de fila err: \n%s\n\n\n",
						dbe.err_description.c_str());
			}

		}

		database->freeResult();
	} catch (DBException dbe) {
		debug->warn("ERR no returnQueue():\n%\n\n", dbe.err_description.c_str());
		throw dbe;
	}

	return true;
}

void QueueManager::statsInsert(const char* email, int code) {
	try {
		database.executeQuery("replace into EmktStatsEnvio "
			"(id_peca,id_campanha,error_code,email,enviado) "
			"values ('%d','%d','%d','%s',now())", id_peca, id_campanha, code,
				email);

	} catch (DBException dbe) {
		throw dbe;
	}
}

void QueueManager::statsInsert(const char* email, int code,
		const std::string& mensagem, int id_peca, int id_campanha) {
	try {
		database.executeQuery("replace into EmktStatsEnvio "
			"(id_peca,id_campanha,error_code,email,enviado)"
			" values ('%d','%d','%d','%s',now())", id_peca, id_campanha, code,
				email);

		if (code >= 550) {
			db.executeQuery(
					"insert ignore into EmktListaNegra values ('%s',%d,'%s') ",
					email, code, mensagem.c_str());
		}

	} catch (DBException dbe) {
		throw dbe;
	}
}

void QueueManager::eraseQueue(const char* email) {
	try {
		database.executeQuery("delete from "
			"EmktFilaEnvioPeca where "
			"id_campanha='%d' and id_peca='%d' and email = '%s'", id_campanha,
				id_peca, email);
	} catch (DBException dbe) {
		throw dbe;
	}
}

void QueueManager::eraseQueue(const char* email, int id_peca, int id_campanha) {
	try {
		database.executeQuery(
				"delete from EmktFilaEnvioPeca where id_campanha='%d' and id_peca='%d' and email = '%s'",
				id_campanha, id_peca, email);

	} catch (DBException dbe) {
		throw dbe;
	}
}

vector<Address> QueueManager::load_queue(Peca &peca) {
	vector<Address> listEmails;

	try {

		Pointer pointer("select * from EmktFilaEnvioPeca where "
				"id_peca='%d' and id_campanha='%d' and thread_id=%d "
				"order by email limit 0,%d",
				peca.pecaId,
				peca.campanhaId,
				INSTANCE_NUM,
				1000);

		while (pointer.next()) {
			random_number = lowest + (unsigned long) ((1.0 * range * rand()) / (RAND_MAX + 1.0));

			stringstream ss;
			ss << random_number;

			string s = pointer.get("domain");

			//Domain to lower
			char *buf = new char[s.length()];
			s.copy(buf, s.length());
			for (int f = 0; f < (int) s.length(); f++) {
				buf[f] = tolower(buf[f]);
			}
			std::string r(buf, s.length());
			delete buf;

			if (i == 0) {
				domain = pointer.get("domain");
				dadosPessoa_t dp;
				dp.email = pointer.get("email");
				dp.nome = pointer.get("nome");
				ss >> dp.id;
				listEmails.push_back(dp);
				database->executeQuery("update EmktFilaEnvioPeca "
						"set stats='1',id_thread='%d' where id_peca='%d' and "
						"id_campanha='%d' and email='%s'", threadId, id_peca,
						id_campanha, pointer.get("email"));

			} else if (strcmp(r.c_str(), domain.c_str()) != 0) {
				pointer.backRecord();
				break;

			} else {
				dadosPessoa_t dp;
				dp.email = pointer.get("email");
				dp.nome = pointer.get("nome");
				ss >> dp.id;
				listEmails.push_back(dp);
				database->executeQuery("update EmktFilaEnvioPeca set "
						"stats='1',id_thread='%d' where id_peca='%d' and "
						"id_campanha='%d' and email='%s'", threadId, id_peca,
						id_campanha, pointer.get("email"));

			}

		}

	} catch (DBException dbe) {
		debug->error("QUEUE MGR. EXCEPTION || %s!", dbe.err_description.c_str());
		throw dbe;
	}

	return listEmails;

}

void QueueManager::process_results() {
	if (em.id_error < 300 && em.id_error > 0) { //sucesso no envio, verificar se todos usuarios eram validos
		for (unsigned int x = 0; x < em.id_email_error.size(); x++) {
			QueueManager::statsInsert((em.emails_error[x]).c_str(),
					em.id_email_error[x], em.message_error, id_peca,
					id_campanha);
			QueueManager::eraseQueue((em.emails_error[x]).c_str(), id_peca,
					id_campanha);

			debug.debug("*** %d - %s (Campanha:%d-Peca:%d) ", em.id_error,
					(em.emails_error[x]).c_str(), id_campanha, id_peca);
		}

		em.message_error = mail->getErrorMessages().message_error;
		em.id_error = mail->getErrorMessages().id_error;

		tratandoErros(em, es.id_peca, es.id_campanha);

	}
}

void QueueManager::build_mx_servers() {
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
}

void QueueManager::trata_retorno() {
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
}

const Mailer* QueueManager::find_mailer(const std::string &domain)
{
	Mailer * mailer;
	multimap<string, struct mx>::iterator pos;

	if (servidoresMX.find(domain) != servidoresMX.end()) {
		for (pos = servidoresMX.lower_bound(domain); pos
		!= servidoresMX.upper_bound(domain); ++pos) {
			multimap<string, *Mailer>::iterator posMailer = mailers.find(
					pos->second.host);

			if	(posMailer == mailers.end()) {

				mailer = new Mailer();
				mailer->
				mailers.insert(make_pair(domain, mailer));
			} else {

			}
		}
	} else {

		servidoresMX.insert( make_pair(domain,x) );
	}
}
