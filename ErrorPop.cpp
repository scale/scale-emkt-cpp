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
#include <string.h>

#include "ErrorPop.h"
#include "Pointer.h"



void* ErrorPop::Run(void* param){

	std::vector<errorEmail_t> vEmails_error;

	//Debug para jogar no log as informacoes
	Debug debug("ErrorPop");
	debug.debug("Iniciando ErrorPop");

	//Esta Thread nunca deve morrer
	while(1){
	    try{
		debug.debug("Iniciando processo do ErrorPop dentro do while");
	  	string id_peca,id_campanha,email,err_code;

		//Pegando os errors que estão no banco, não importa a peça que se refere pois esta estará exposta
		//no email de retorno como X-Emkt x.y (onde x = campanha e y = peca)
		Pointer pointer(this->s_CI,"select * from EmktPop");
		Database database(this->s_CI);

		//Percorrendo os erros que estao no banco
		if (pointer.getTotal() > 0)
			debug.debug("ErrorPop::Run - Total de %d Erros no Banco!",pointer.getTotal());
			
		while(pointer.next()){
			sleep(3);

			debug.debug("ERRPOP-> Posicao %d de um total de %d", pointer.getPosicao(), pointer.getTotal());

			string email_lower = lowerCase(pointer.get("email"));
			debug.debug("ERRPOP-> LowerCase com sucesso!");
			//

			int pos_inicio = 0;
			int pos_final  = 0;
			bool ig = false;
			if(email_lower.find("content-type: message/delivery-status") != string::npos){
				debug.debug("ENCONTROU o cabecalho da parte que diz que retorna o erro!");
				pos_inicio = email_lower.find("content-type: message/delivery-status");
			} else if(email_lower.find("recebeu a sua mensagem. veja a seguir a desc") != string::npos) {
				//Caso serja um email de retorno do ig
				ig = true;
				debug.debug("Email do IG FORA DO PADRAO!");
				pos_inicio = email_lower.find("recebeu a sua mensagem. veja a seguir a desc");
				pos_inicio = email_lower.find("<",pos_inicio) + 1;
				pos_final = email_lower.find(">:",pos_inicio) + 1;

				errorEmail_t eeMailsErrors;
				//caixa postal que excedeu quota
				if(email_lower.find("excedeu o limite de capacidade")  != string::npos ){
					eeMailsErrors.id_error = "522";
				//Conta Inativa
				} else if(email_lower.find("conta inativa") != string::npos ) {
					eeMailsErrors.id_error = "571";
				//Outro erro qualquer
				} else {
					eeMailsErrors.id_error = "550";
				}

				eeMailsErrors.email_error = email_lower.substr(pos_inicio,pos_final-pos_inicio);

				if(eeMailsErrors.id_error.length() > 0 && eeMailsErrors.email_error.length() > 0)
					vEmails_error.push_back(eeMailsErrors);

			} else {
				debug.debug("Nao encontrou o cabecalho da parte que diz que retorna o erro!");
				debug.debug("ERRPOP -> Delete from EmktPop WHERE id='%s'",pointer.get("id"));
				database.executeQuery("Delete from EmktPop WHERE id='%s'",pointer.get("id"));
				continue;
			}

			pos_final = email_lower.find("--",pos_inicio + 1);
			debug.debug("Comeco %d e Fim %d da PART!",pos_inicio,pos_final);
			int pos_inicio_linha = 0, pos_fim_linha = 0, andar = 0;


			string header = email_lower.substr(pos_inicio, pos_final-pos_inicio);

			if(pos_inicio != string::npos && pos_final != string::npos && !ig){

				while(pos_inicio_linha != string::npos){

					debug.debug("ERRPOP -> Loop procurando o email e o erro!! %d ",
							 pointer.getPosicao());

					errorEmail_t eeMailsErrors;

					eeMailsErrors.id_errPop = pointer.get("id_err");


					//procurando o email da pessoa
					pos_inicio_linha = header.find("original-recipient: rfc822;",pos_fim_linha);

					if(pos_inicio_linha == string::npos){
						pos_inicio_linha = header.find("final-recipient: rfc822;",pos_fim_linha);
						andar = 24;
					} else {
						andar = 27;

					}

					if(pos_inicio_linha != string::npos){
						pos_fim_linha = header.find("\n",pos_inicio_linha);
						pos_inicio_linha += andar;
						eeMailsErrors.email_error = header.substr(pos_inicio_linha,
								 pos_fim_linha-pos_inicio_linha);

						//tirando os espacos vazios
						string::size_type pos = eeMailsErrors.email_error.find (" ",0);
						while(pos != string::npos){
							eeMailsErrors.email_error.erase(pos,1);
							pos = eeMailsErrors.email_error.find (" ",0);
						}

						debug.debug("ERRPOP -> email de erro \"%s\" ",
								 eeMailsErrors.email_error.c_str());
						eeMailsErrors.id_error = "599"; //email voltou
						
						pos_fim_linha = header.find("action: ",pos_fim_linha) + 15;

					}

					//pegando do o erro
					Pointer errs(this->s_CI,"select * from EmktPopErr");
					andar = header.find("original-recipient: rfc822;",pos_fim_linha);
					if(andar == string::npos)
						andar = header.find("final-recipient: rfc822;",pos_fim_linha);

					if(pos_inicio_linha != string::npos)
						pos_inicio_linha = 0;

					if(pos_inicio_linha != string::npos){
						int posicao = 0;
						string sErr = header.substr(pos_inicio_linha,andar-pos_inicio_linha);

						while(errs.next()){

							posicao = sErr.find(errs.get("key"));
							debug.debug(" POPERR || errorrr %s", sErr.c_str());
							if(posicao != string::npos){
								eeMailsErrors.id_error = errs.get("value");
								break;

							} 

						}
					}

					if(eeMailsErrors.id_error.length() > 0 && eeMailsErrors.email_error.length() > 0){
						debug.debug(" POPERR || Adicionou o erro!");
						vEmails_error.push_back(eeMailsErrors);
					} else {
						debug.debug(" POPERR || Nao Adicionou o erro! %s %s",
						eeMailsErrors.id_error.c_str(),eeMailsErrors.email_error.c_str());
					}


				}

			}

			//achando os dados da peca
			debug.debug("Indo buscar os dados da peca!");
			pos_inicio = email_lower.find("x-emkt: ");
			pos_final = email_lower.find("\n",pos_inicio);
			if(pos_inicio != string::npos && pos_final != string::npos){
				string xemkt = email_lower.substr(pos_inicio+8, pos_final-(pos_inicio+8));
				id_campanha = xemkt.substr(0,xemkt.find("."));
				id_peca = xemkt.substr(xemkt.find(".")+1);
				debug.debug("Id_campanha='%s' |||| Id_peca='%s'",id_campanha.c_str(),id_peca.c_str());

			}


			for(int x = 0; x < vEmails_error.size(); x++){
				printf("replace into EmktStats (email,error_code,id_campanha,id_peca, enviado) values ('%s','%s','%s','%s',now())\n\n",
					vEmails_error[x].email_error.c_str(),vEmails_error[x].id_error.c_str(),id_campanha.c_str(),id_peca.c_str());
				database.executeQuery("replace into EmktStatsEnvio (email,error_code,id_campanha,id_peca, enviado) values ('%s','%s','%s','%s',now())", vEmails_error[x].email_error.c_str(), vEmails_error[x].id_error.c_str(), id_campanha.c_str(), id_peca.c_str());
				database.executeQuery("Delete from EmktPop WHERE id='%s'",pointer.get("id"));
			}

			vEmails_error.clear();

		}


		//debug.debug("ERRPOP -> EMail retornados Verificados indo dormir para restornar + tarde!");
	 	sleep(60);


	   } catch(DBException dbe) {
		debug.debug("ERRPOP -> %s",dbe.err_description.c_str());

	   }
	}
        return NULL;

}


ErrorPop::ErrorPop(Connection_Info_t* s_ConInfo){

	SetThreadID(0);

	s_CI.host=s_ConInfo->host;
	s_CI.user=s_ConInfo->user;
	s_CI.pass=s_ConInfo->pass;
	s_CI.db=s_ConInfo->db;

}

ErrorPop::~ErrorPop(){


}

// Make a lowercase copy of s:
string ErrorPop::lowerCase(const char *s) {
  char* buf = new char[strlen(s)+10];
  strcpy(buf, s);

  for(int i = 0; i < strlen(s); i++){
    buf[i] = tolower(buf[i]);
  }

  string r = buf;

  delete(buf);

  return r;
}
