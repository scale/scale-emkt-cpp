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
using namespace std; 


void* PecaHandler::Run(void* param){

	debug->debug("PecaHandler[%d]::Run", id_peca);
	Sender* sender[MAX_THREADS_ENVIO]; 
	QueueManager qm(&s_CI, id_peca, id_campanha, TOTAL_EMAIL);
	
	int num_threads = 0; //Numero de threads rodando
	int vez = 0;

	emailSource_t esTemp[MAX_THREADS_ENVIO];
        
	for(int i = 0; i < MAX_THREADS_ENVIO; i++) sender[i] = NULL;
		
	int queueId = 0 ;
	int status_running = MAX_THREADS_ENVIO;
	
	vector<ErrorMessages_t> vErrorMessages;
	emailSource_t es;

	ErrorMessages_t erro_vez;
	
	try{
		Pointer pointer(s_CI,"select * from EmktPeca where id_peca=%d and id_campanha=%d ", id_peca,id_campanha);
		if(pointer.next()){
			es.from = pointer.get("email_resposta");
			es.subject = pointer.get("subject");
			es.body_txt = pointer.get("corpoTXT");
			es.body_html = pointer.get("corpoHTML");
			es.body_html += " ";
			es.errors_to = pointer.get("errors_to");

			
			if(DNS.size() > 3)
				es.DNS = DNS;
			else 
				es.DNS = "127.0.0.1";
			
			
			es.id_camp_peca = pointer.get("id_campanha");
			es.id_camp_peca += ".";
			es.id_camp_peca += pointer.get("id_peca");

			es.id_peca = id_peca;
			es.id_campanha = id_campanha;

			debug->debug("PH[%d]::Run --Peca:%s  Email Resposta: %s / Subject: %s", id_peca, es.id_camp_peca.c_str(), es.from.c_str(), es.subject.c_str() );
			
		} else {
			Sender* running = 0;
			setRunning(false);
			debug->info("Nao ha pecas para campanha %d!", id_campanha);
			return NULL;
		
		}	
	} catch(DBException dbe) {
		debug->error("%s", dbe.err_description.c_str());
		throw dbe;
	}
	// Iniciando o procedimentos para comecar o envio!
	int finalizar = 0;
	while(status_running > 0){
	
	  for(vez = 0; vez < MAX_THREADS_ENVIO; vez++){
	    //este qm.ad deve morrer mas antes passar o "relatorio" do envio
		if( sender[vez] != NULL && sender[vez]->getStatus() == 0){
			//debug->Syslog("PH :: ACABOU A THREAD_ID %d",vez);
			delete(sender[vez]);
			sender[vez] = NULL;
			
		}
		
		if( sender[vez] == NULL) {
			//debug->Syslog("PH :: CRIANDO A THREAD_ID %d",vez);
			esTemp[vez].to = qm.getEmails(vez);
			
			if(esTemp[vez].to.size() < 1){
				//debug->debug("PH[%d]::Run -- Hora de finalizar a peca nao ha mais emails na fila!",id_peca);
				finalizar = 1;
				break;
			}

			mutex.Acquire();
			if(status_running){
				if(esTemp[vez].to.size() > 0) {
					
					esTemp[vez].from        = es.from;
					esTemp[vez].subject     = es.subject;
					esTemp[vez].body_txt    = es.body_txt;
					esTemp[vez].body_html   = es.body_html;
					esTemp[vez].errors_to   = es.errors_to;
					esTemp[vez].DNS         = es.DNS;
					esTemp[vez].id_peca     = id_peca;
					esTemp[vez].id_campanha = id_campanha;

					sender[vez] = new Sender(vez);
					if(!sender[vez]->setEmailSouces(esTemp[vez])){
						//debug->debug("PH[%d]::Run -- problemas aoadcionar os sources do email", id_peca);					
					}
						
					sender[vez]->Start();
				}
			}
			mutex.Release();

		}

		sleep(3);

	  } // FIM FOR

		if(finalizar > 0){
			debug->debug("PH[%d]::Run -- Finalizando peca",id_peca);
			break;
		}

	}//FIM Do WHILE	
	
	try{
	  	Database database(s_CI);
		debug->info("Finalizando Peca: id_campanha='%d', id_peca='%d'", id_campanha,id_peca);
		database.executeQuery("update EmktPeca set stats=2 where id_peca='%d' and id_campanha='%d'", id_peca, id_campanha);
	} catch(DBException dbe) {
		debug->error("PH[%d]::DBException:: %s", id_peca, dbe.err_description.c_str());

	}	  
	  
	debug->debug("PH[%d]::Run -- Fim",id_peca);
	setRunning(false);
	return NULL;

}


PecaHandler::PecaHandler(int id, Connection_Info_t ci, int peca, int campanha, int total_emails){
	mutex.Acquire();
	SetThreadID(id);
	s_CI = ci;
	id_peca = peca;
	id_campanha = campanha;
	this->total_emails = total_emails;

	DNS = "";
	debug = new Debug(1,"PecaHandler");

	debug->debug("PH[%d]::Run -- Id_campanha = %d / Id_peca = %d ",id_peca,id_campanha,id_peca);

	mutex.Release();
}


PecaHandler::~PecaHandler(){
	if(debug != NULL){
		delete(debug);
		debug = NULL;

	}

	Stop();

}

void PecaHandler::setDNS(string dns){
	DNS = dns;
}

