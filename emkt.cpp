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
#include <fstream>


int main()
{
	Debug debug(1,"emkt");
	debug.info("Iniciando o PROGRAMA DE ENVIO!");
        // variaveis 
        int num_threads = 0; //Numero de threads rodando
        PecaHandler* ph[MAX_THREADS_PECA]; //Matriz de Threads que deverao rodar
        Connection_Info_t s_CI; //Struct com os dados de conexao;

        ifstream hfile(CONF_DIR);

	debug.info("Configuration file: %s", CONF_DIR);
	string DNS = "";

	int dados_conexao = 0;
	string temp;
        while ( !hfile.eof() ) {
	    getline(hfile, temp);

	    if(temp.find("database") == 0){
                s_CI.db = temp.substr( temp.find("=")+1, temp.length() );
                dados_conexao++;
            }

	    if(temp.find("host") == 0){
	        s_CI.host = temp.substr( temp.find("=")+1, temp.length() );
		dados_conexao++;
            }

	    if(temp.find("user") == 0){
	        s_CI.user = temp.substr( temp.find("=")+1, temp.length() );
		dados_conexao++;
	    }

	    if(temp.find("pass") == 0){
	        s_CI.pass = temp.substr( temp.find("=")+1, temp.length() );
		dados_conexao++;
	    }

	    if(temp.find("DNS") == 0){
	        DNS = temp.substr( temp.find("=")+1, temp.length() );
		
	    }
        }
	
	debug.info("Parametros encontrados: %s@%s:/%s - DNS: %s ", s_CI.user.c_str(), s_CI.host.c_str(), s_CI.db.c_str(), DNS.c_str());
	
	for(int i = 0; i < MAX_THREADS_PECA; i++)
		ph[i] = NULL;
			
	try{
	  //Para tratar os erros que retornam para a tabela EmktPop pelo programa procmailemkt
	  //colocado no MTA para desviar os emails da conta de retorno da peca para o Banco
	  ErrorPop ep(&s_CI);
	  ep.Start();

	  debug.debug("Conectando ao BD...");	  
	  Database database(s_CI);
	  
	  //caso o programa tenha caido durante um envio voltar a peca ao status para ser entregue
	  database.executeQuery("update EmktPeca set stats='0' where stats='1'");

	  while(1){
	    Pointer pointer(s_CI,"select * from EmktPeca where data_enviar <= now() and stats = 0");
		
	    if(pointer.getTotal() < 1){
			sleep(10);
		continue;
	    }
	    for(int vez = 0; vez < MAX_THREADS_PECA; vez++){
		
		//Se so tiver uma campanha
		if(vez >= pointer.getTotal())
			break;
		
		//PecaHandler(int id, Connection_Info_t ci, int peca, int campanha, int total_emails = 1);
	    	//este tqm.ad dece morrer mas antes passar o "relatorio" do envio
		if( ph[vez] != NULL && ph[vez]->getStatus() == 0){
                    delete(ph[vez]);
                    ph[vez] = NULL;
		} 
		    		
		if(pointer.getNext() && ph[vez] == NULL){

		        int id_campanha = 0;
			int id_peca = 0;

	    		id_peca = atoi(pointer.get("id_peca"));
			id_campanha = atoi(pointer.get("id_campanha"));
			
			ph[vez] = new PecaHandler(vez,s_CI, id_peca, id_campanha,TOTAL_EMAIL);

			//colocando o DNS que veio do arquivo
			ph[vez]->setDNS(DNS);

			//Se o programa retornou sem ter exluido os emails da fila 
			//é porque não foram entregues, então ao reiniciar recolocamos todos
			//na fila e caso seja a primeira, nao ira mudar nada
			database.executeQuery("update EmktFilaEnvioPeca set "
						"stats='0' where id_peca='%d' "
						"and id_campanha='%d'",id_peca, id_campanha);
			
			//Definindo o stats de que esta em andamento
			debug.debug("Atualizando peca %d!",id_peca);
			database.executeQuery("update EmktPeca set stats=1 where "
				"id_peca='%d' and id_campanha='%d'", id_peca, id_campanha);
			
			debug.info("Iniciando peca %d!",id_peca);
			ph[vez]->Start();
			

			
		}

		sleep(1);
	    } // FIM FOR

		sleep(10);
	  } //FIM DO WHile

	} catch(DBException dbe) {
		debug.error("EMKT::DBException:: %s", dbe.err_description.c_str());

	} catch(ErrorMessages_t ___exception){
		debug.error("EMKT::ErrorMessages_t:: " ,___exception.message_error.c_str());
	} 
	
	debug.info("SAINDO DO PROGRAMA");
	
        return 0;
}


