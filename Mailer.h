/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef __Mailer_H__
#define __Mailer_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET; // get round windows definitions.

#include <vector>
#include <string>
#include "Debug.h"

class Mailer {

public:
	Mailer(const std::string& server = "127.0.0.1");
	~Mailer();

	void send();

	// Set a new Subject for the mail (replacing the old)
	// will return false if newSubject is empty.
	void subject(const std::string& newSubject) { subject = newSubject; } ;
	const std::string subject() { return _subject; } ;

	// sets the senders address (fromAddress variable)
	void from(const std::string& from) { fromAddress.email = from; fromAddress.name=""; };
	void from(const Address& addr) { fromAddress = addr; };

	void to(const Address& addr) { recipients.push_back(addr); };
	void to(const std::string& name, const std::string& email) {
		Address addr;
		addr.name = name;
		addr.email = email;
		recipients.push_back(addr);
	};


	// returns the return code sent by the smtp server or a local error.
	// this is the only way to find if there is an error in processing.
	// if the mail is sent correctly this string will begin with 250
	// see smtp RFC 821 section 4.2.2 for response codes.
	const std::string& response() const { return returnstring; };

	//Para inserir o conteudo TEXT
	void text(const char* text) { body_text = text; };
	//Para inserir o conteudo HTML
	void html(const char* html) { body_html = html; };

	//Para devolver o struct que conetem os erros elvantados durante a entrea do email
	std::vector<ResultMessage> getErrorMessages() { return mensagens; };

private:


	// create a header with current message and attachments.
	std::string makesmtpmessage() const;

	// make sure the message body has lines less than 1000 characters
	// add line breaks if necessary.
	// rfc821
	void checklinesarelessthan1000chars();

	// initialises winsock in win32, does nothing in unix (a definate snore function)
	void init() const;
	// wrapper for closesocket...windows & close...unix
	void closesocket(const SOCKET& s);

	// The addresses to send the mail to
	std::vector<Address> recipients;
	// The address the mail is from.
	Address fromAddress;

	std::string _subject;
	std::string body_text;
	std::string body_html;

	// This will be filled in from the toAddress by getserveraddress
	std::string server;

	std::vector<ResultMessage> mensagens;

	// filled in with server return strings
	std::string returnstring;
};

#endif // !ifndef __Mailer_H__

