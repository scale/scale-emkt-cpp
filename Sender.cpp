/**************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "Sender.h"
#include "unistd.h"
#include "Database.h"
extern "C" {
#include <openssl/md5.h>
}
#include <iostream>
#include <vector>
using namespace std;

void* Sender::Run(void*) {
	Debug debug(1, "Sender");
	Mailer mail(es.DNS);

	setRunning(true);
	active = true;


	while (active) {
		/* Instanciando a classe de email e alimentando-a
		 com o conteudo e as pessoas que devem receber o email */
		try {
			mail.setsubject(es.subject);
			mail.setsender(es.from);
			mail.setid_camp_peca(es.id_camp_peca);

			std::vector<Address>::iterator it;
			for (it = recipients.begin(); it != recipients.end(); ++it) {
				mail.to((*it));
			}

			mail.seterrorsto(peca.from);

			if (recipients.size() > 1) {
				//Para um grupo de pessoas nao tem como colocar o nome de cada
				mail.setBodyText((peca.txt).c_str());
				mail.setBodyHtml((peca.html).c_str());
			} else if (es.to.size() == 1) {
				string html = peca.html;
				string txt = peca.txt;

				Address rcpt = recipients[0];

				//no html
				string::size_type pos = html.find("##NOME##", 0);
				while (pos != string::npos) {
					html.replace(pos, 8, dp.nome); //o 8 eh o numero de caratcteres ##NOME##
					pos = html.find("##NOME##", 0);
				}
				pos = html.find("##EMAIL##", 0);
				while (pos != string::npos) {
					html.replace(pos, 9, dp.email); //o 9 eh o numero de caratcteres ##EMAIL##
					pos = html.find("##EMAIL##", 0);
				}
				pos = html.find("##ID##", 0);
				while (pos != string::npos) {
					html.replace(pos, 6, dp.id); //o 8 eh o numero de caratcteres ##NOME##
					pos = html.find("##ID##", 0);
				}

				//no TXT
				pos = txt.find("##NOME##", 0);
				while (pos != string::npos) {
					txt.replace(pos, 8, dp.nome); //o 8 eh o numero de caratcteres ##NOME##
					pos = txt.find("##NOME##", 0);
				}
				pos = txt.find("##EMAIL##", 0);
				while (pos != string::npos) {
					txt.replace(pos, 9, dp.email); //o 9 eh o numero de caratcteres ##EMAIL##
					pos = txt.find("##EMAIL##", 0);
				}

				pos = html.find("##EMAIL_MD5##", 0);
				while (pos != string::npos) {
					unsigned int counter;
					MD5_CTX ctx;
					char ptr_byte[3];
					unsigned char *ptr;
					unsigned char digest[16];
					unsigned char md5email[32];

					char email[255];
					snprintf(email, sizeof(email), "%s", dp.email.c_str());

					MD5_Init(&ctx);
					MD5_Update(&ctx, email, strlen((const char*) email));
					MD5_Final(digest, &ctx);

					ptr = md5email;
					for (counter = 0; counter < sizeof(digest); counter++, ptr
							+= 2) {
						sprintf(ptr_byte, "%02x", digest[counter] & 0xFF);
						*ptr = ptr_byte[0];
						*(ptr + 1) = ptr_byte[1];
					}
					*ptr = '\0';

					html.replace(pos, 13, (const char*) md5email);
					pos = html.find("##EMAIL_MD5##", 0);

				}

				mail.setBodyText(txt.c_str());
				mail.setBodyHtml(html.c_str());

			}

			mail.send();

		trantandoErros(em, es.id_peca, es.id_campanha);

		} catch (ErrorMessages_t __exception) {
			cerr << "ERR: " << __exception.message_error << " (" << hex << this << ")" << endl;
		    debug.error("SENDER EXCEPTION || %s!", __exception.message_error.c_str());
		    //throw(exception);
		}

		setRunning(false);
		//free(mail);
		if (mail != NULL) delete mail;
		Stop();
		return NULL;

	}


	Sender::Sender(int status){
		mutex.Acquire();
		SetThreadID(status);
		mutex.Release();
	}


	Sender::~Sender(){
		Stop();
	}


	ErrorMessages_t* Sender::getErrorMessages(){
		return &em;

	}

	bool Sender::setEmailSouces(emailSource_t& emailsources){
		Debug debug("Sender");
		mutex.Acquire();
		es = emailsources;
		mutex.Release();

		if(es.to.size() == 0)
		    debug.error("Nao indicou nenhum email para receber");

		if(strlen((es.DNS).c_str()) < 7)
		    debug.error("Nao indicou nenhum DNS");

		if(strlen((es.body_txt).c_str()) < 7)
		    debug.error("Nao tem corpo texto");

		if(strlen((es.subject).c_str()) < 3)
		    debug.error("Nao tem assunto ");

		if(strlen((es.from).c_str()) < 5)
		    debug.error("Nao tem remetente");

		if( es.to.size() == 0 || strlen((es.DNS).c_str()) < 7 ||
			strlen((es.body_txt).c_str()) < 7 ||
			strlen((es.subject).c_str()) < 3 ||
			strlen((es.from).c_str()) < 5 )
			return false;

		return true;
	}


	void* Sender::trantandoErros(ErrorMessages_t em, int id_peca, int id_campanha){
		Debug debug("Sender");
		string emails_validos;
		if(em.id_error < 300 && em.id_error > 0) { //sucessno envio, verificar se todos usuarios eram validos
			for(unsigned int x = 0; x < em.id_email_error.size(); x++){
				QueueManager::statsInsert((em.emails_error[x]).c_str(),em.id_email_error[x], em.message_error, id_peca, id_campanha);
				QueueManager::eraseQueue((em.emails_error[x]).c_str(), id_peca, id_campanha);

			    debug.debug("*** %d - %s (Campanha:%d-Peca:%d) ",em.id_error, (em.emails_error[x]).c_str(), id_campanha, id_peca);
			}

			em.message_error = mail->getErrorMessages().message_error;
			em.id_error = mail->getErrorMessages().id_error;

			tratandoErros(em, es.id_peca, es.id_campanha);

		} catch (ErrorMessages_t __exception) {
			debug.error("SENDER EXCEPTION || %s!",
					__exception.message_error.c_str());
			//throw(exception);
		}

		setRunning(false);

	}
	Stop();
	return NULL;

}

void Sender::add_recipient(const Address& rcpt) {
	this->recipients->push(rcpt);
}


