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


QueueManager::QueueManager(Connection_Info_t* s_ConInfo, int peca, int campanha, int total_emails, int tpeca){
	
	s_CI.host = s_ConInfo->host;
	s_CI.user = s_ConInfo->user;
	s_CI.pass = s_ConInfo->pass;
	s_CI.db   = s_ConInfo->db;
	
	Database::host = s_ConInfo->host.c_str();
	Database::user = s_ConInfo->user.c_str();
	Database::pass = s_ConInfo->pass.c_str();
	Database::db   = s_ConInfo->db.c_str();

	database = new Database(s_CI);
	block_size = total_emails;
	
	id_peca = peca;
	id_campanha = campanha;

	tipo_peca = tpeca;

	debug = new Debug("QueueManager");


	
}

QueueManager::~QueueManager(){
	if( database != NULL ){
		delete(database);
		database = NULL;
	}
	if( debug != NULL){
		delete(debug);
		debug = NULL;
	}

}


bool QueueManager::returnQueue(const char* email){
	
	Database* db = new Database();
	try{
		if(strlen(email) > 0) {
			db->executeQuery("update EmktFilaEnvioPeca set stats='2',id_thread=50 where email in ('1'%s)", email);
		}
	} catch(DBException dbe) {
		throw dbe;
	}

	delete(db);
	return true;
}

bool QueueManager::returnQueue(){

	try{
		MYSQL_RES* result;
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
				Pointer p2(s_CI,"select * from EmktFilaEnvioPeca where id_peca='%d' and id_campanha='%d' and email not in (select email from EmktListaNegra) order by email",id_peca,id_campanha);
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


void QueueManager::statsInsert(const char* email, int code){
	Database db;
	try{
		db.executeQuery("replace into EmktStatsEnvio (id_peca,id_campanha,error_code,email,enviado) values ('%d','%d','%d','%s',now())",id_peca,id_campanha,code,email);

	} catch(DBException dbe) {
		throw dbe;
	} 
}

void QueueManager::statsInsert(const char* email, int code, const std::string& mensagem, int id_peca, int id_campanha){
	Database db;
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


void QueueManager::eraseQueue(const char* email){
	Database db(s_CI);
	try{
		db.executeQuery("delete from "
						"EmktFilaEnvioPeca where "
						"id_campanha='%d' and id_peca='%d' and email = '%s'",
						id_campanha,id_peca,email);
	} catch(DBException dbe) {
		throw dbe;
	}
}

void QueueManager::eraseQueue(const char* email, int id_peca, int id_campanha){

	Database db;
	try{
		db.executeQuery("delete from EmktFilaEnvioPeca where id_campanha='%d' and id_peca='%d' and email = '%s'",
					id_campanha,id_peca,email);

	} catch(DBException dbe) {
		throw dbe;
	}
}

vDadosPessoa QueueManager::getEmails(int threadId){

	vDadosPessoa listEmails;

	try{
	
		Pointer pointer(QueueManager::s_CI,
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

