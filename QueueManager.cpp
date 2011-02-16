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


QueueManager::QueueManager(int peca, int campanha, int total_emails, int tpeca)
{
	database = new Database();
	
	block_size = total_emails;
	
	id_peca = peca;
	id_campanha = campanha;

	tipo_peca = tpeca;

	debug = new Debug("QueueManager");
}


QueueManager::~QueueManager() {
	Stop();
}

void*
QueueManager::Run(void* param)
{
	while (1) {
		debug.info("Queueing...");




		sleep(30);
	}
}

void QueueManager::includePeca(Peca& peca)
{
	Pointer pt = new Pointer("select count(*) as c from EmktFilaEnvioPeca "
							 "where id_peca=%d and id_campanha=%d", peca.pecaId, peca.campanhaId);

	int num_emails = 0;
	if (pt.next()) num_emails = atoi(pt.get("c"));

	if (num_emails > 0)
	{
		//Se o programa retornou sem ter excluido os emails da fila
		//� porque n�o foram entregues, ent�o ao reiniciar recolocamos todos
		//na fila e caso seja a primeira, nao ira mudar nada
		database.executeQuery("update EmktFilaEnvioPeca set stats=0, id_thread = %d "
						  "where id_peca=%d and id_campanha=%d and id_thread = 0 "
						  "limit %d",
						  INSTANCE_NUM,
						  peca.pecaId,
						  peca.campanhaId,
						  num_emails);

		pecas.push_back(peca);
	}
}

void QueueManager::cancelPeca(Peca& peca)
{
	vector<Peca>::iterator it;

	for (it = pecas.begin(); it != pecas.end(); it++)
	{
		Peca p = *it;
		if (p == peca)
		{
			p.ativa = false;
			break;
		}
	}
}

bool
QueueManager::returnQueue()
{
	try{
		result = database->select("select id_peca from EmktPeca where date_add(data_enviar, "
						"INTERVAL 5 DAY) > now() and id_peca='%d' and"
						" id_campanha='%d'",id_peca,id_campanha);
		if(mysql_num_rows(result) > 0){
			
			debug->debug("Erros temporarios marcados para nova tentativa");

			database->executeQuery("update EmktFilaEnvioPeca set stats='0' where id_peca='%d' and"
						" id_campanha='%d' and stats='2'",id_peca,id_campanha);

		} else {
			ErrorMessages_t em2;
			em2.message_error = "Muitos dias na fila";
			em2.id_error = 850;
			debug->debug("Excluindo os que ficaram na fila a mais do que o tempo limite");
		    try{
				Pointer p2(conn,"select * from EmktFilaEnvioPeca where id_peca='%d' and id_campanha='%d' and email not in (select email from EmktListaNegra) order by email",id_peca,id_campanha);
				while(p2.next()){
					string email = p2.get("email");
					statsInsert(email.c_str(),850);
					eraseQueue(p2.get("email"));
	
				}
			
			debug->debug("MUITOS DIAS DE FILA: Camp. %d / Peca %d",id_campanha,id_peca);
		    } catch(DBException dbe){
		    	debug->warn("QM :: EMail muito tempo de fila err: \n%s\n\n\n",dbe.err_description.c_str());
		    }
			
		}
	
		database->freeResult();
	} catch(DBException dbe) {
		debug->warn("ERR no returnQueue():\n%\n\n",dbe.err_description.c_str());
		throw dbe;
	}

	return true;
}


void
QueueManager::statsInsert(const char* email, int code)
{
	try{
		db.executeQuery("replace into EmktStatsEnvio "
				"(id_peca,id_campanha,error_code,email,enviado) "
				"values ('%d','%d','%d','%s',now())",id_peca,id_campanha,code,email);

	} catch(DBException dbe) {
		throw dbe;
	} 
}

void
QueueManager::statsInsert(const char* email, int code, const std::string& mensagem, int id_peca, int id_campanha)
{
	try{
		db.executeQuery("replace into EmktStatsEnvio "
				"(id_peca,id_campanha,error_code,email,enviado)" 
				" values ('%d','%d','%d','%s',now())",
				id_peca,id_campanha,code,email);

		if (code >= 550) {
			db.executeQuery("insert ignore into EmktListaNegra values ('%s',%d,'%s') ",
				email,
				code,
				mensagem.c_str() );
		}

	} catch(DBException dbe) {
		throw dbe;
	} 
}


void
QueueManager::eraseQueue(const char* email)
{
	try{
		db.executeQuery("delete from "
						"EmktFilaEnvioPeca where "
						"id_campanha='%d' and id_peca='%d' and email = '%s'",
						id_campanha,id_peca,email);
	} catch(DBException dbe) {
		throw dbe;
	}
}

void
QueueManager::eraseQueue(const char* email, int id_peca, int id_campanha)
{
	Database db;
	try{
		db.executeQuery("delete from EmktFilaEnvioPeca where id_campanha='%d' and id_peca='%d' and email = '%s'",
					id_campanha,id_peca,email);

	} catch(DBException dbe) {
		throw dbe;
	}
}

vDadosPessoa
QueueManager::getEmails(int threadId)
{
	vDadosPessoa listEmails;

	try{
	
		Pointer pointer(
			"select *,dominio(email) as domain "
			"from EmktFilaEnvioPeca where id_peca='%d' "
			"and id_campanha='%d' and (stats='0') "
			"order by email limit 0,%d",
			id_peca,
			id_campanha,
			block_size);


		if(pointer.getTotal() < 1 ) {
			returnQueue();

			pointer.query(
				"select "
				"*, " 
				"dominio(email) as domain "
				"from EmktFilaEnvioPeca "
				"where "
				"id_peca='%d' "
				"and id_campanha='%d' "
				"and (stats='0' or stats='2') "
				"order by "
				"dominio(email) "
				"limit 0,%d",
				id_peca,id_campanha,block_size);

		}

		if( pointer.getTotal() > pointer.getPosicao() ) {
			srand((unsigned)time(0));
			unsigned long random_number;
    		unsigned long lowest=100, highest=99999999;
    		unsigned long range=(highest-lowest)+1;
			
			for(int i = 0; i < block_size; i++){
				if(pointer.getNext()){
					random_number = lowest + (unsigned long)((1.0*range*rand())/(RAND_MAX + 1.0));
					
					stringstream ss;
					ss << random_number;
					
					string s = pointer.get("domain");

					//Domain to lower
					char *buf = new char[s.length ()];
					s.copy (buf, s.length ());
					for (int f = 0; f < (int) s.length (); f++) {
						buf[f] = tolower (buf[f]);
					}
					std::string r (buf, s.length ());
					delete buf;

					if(i == 0){
						domain = pointer.get("domain");
						dadosPessoa_t dp;
						dp.email = pointer.get("email");
						dp.nome = pointer.get("nome");
						ss >> dp.id;
						listEmails.push_back(dp);
						database->executeQuery("update EmktFilaEnvioPeca "
							"set stats='1',id_thread='%d' where id_peca='%d' and "
							"id_campanha='%d' and email='%s'",threadId,id_peca,
							id_campanha,pointer.get("email"));

					} else if(strcmp(r.c_str(),domain.c_str()) != 0 ){
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
							"id_campanha='%d' and email='%s'",
							threadId,id_peca,
							id_campanha,
							pointer.get("email")
							);
					
					}
				 
				}
				
				
			}
		}
	
	} catch(DBException dbe) {
		debug->error("QUEUE MGR. EXCEPTION || %s!", dbe.err_description.c_str());
		throw dbe;
	}

	return listEmails;

}

void
QueueManager::process_results()
{
	if(em.id_error < 300 && em.id_error > 0)
	{ //sucesso no envio, verificar se todos usuarios eram validos
		for(unsigned int x = 0; x < em.id_email_error.size(); x++)
		{
			QueueManager::statsInsert((em.emails_error[x]).c_str(),em.id_email_error[x], em.message_error, id_peca, id_campanha);
			QueueManager::eraseQueue((em.emails_error[x]).c_str(), id_peca, id_campanha);

			debug.debug("*** %d - %s (Campanha:%d-Peca:%d) ",em.id_error, (em.emails_error[x]).c_str(), id_campanha, id_peca);
		}

		em.message_error = mail->getErrorMessages().message_error;
		em.id_error = mail->getErrorMessages().id_error;

		tratandoErros(em, es.id_peca, es.id_campanha);

	}
}
