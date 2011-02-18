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

#ifdef LINUX
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET; // get round windows definitions.
#endif

#include <unistd.h>

#include <vector>
#include <string>

#include "Debug.h"
#include "Message.h"

// email address wrapper struct

class ResultMessage {
public:
	Address recipient;
	std::string message;
	int error;
};


class Mailer {

public:
	Mailer(const std::string& server = "127.0.0.1");
	~Mailer();

	void send();

	// Set a new Subject for the mail (replacing the old)
	// will return false if newSubject is empty.
	void subject(const std::string& newSubject);
	const std::string& subject() { return _subject; } ;

	// sets the senders address (fromAddress variable)
	void from(const std::string& from) { fromAddress = parseaddress(from); };
	void from(const Address& addr) { fromAddress = addr; };

	void to(const Address& addr) { recipients.push_back(addr); };
	void to(const std::string& name, const std::string& email);


	// returns the return code sent by the smtp server or a local error.
	// this is the only way to find if there is an error in processing.
	// if the mail is sent correctly this string will begin with 250
	// see smtp RFC 821 section 4.2.2 for response codes.
	const std::string& response() const { return returnstring; };

	//Para inserir o conteudo TEXT
	void text(const std::string& text) { body_text = text; };
	//Para inserir o conteudo HTML
	void html(const std::string& html);

	//Para devolver o struct que conetem os erros elvantados durante a entrea do email
	std::vector<ResultMessage>& getErrorMessages() { return mensagens; };

private:
        // create a header with current message and attachments.
        std::string makesmtpmessage() const;

        // make sure the message body has lines less than 1000 characters
        // add line breaks if necessary.
        // rfc821
        void checklinesarelessthan1000chars();

        // helper function.
        // returns the part of the string toaddress after the @ symbol.
        // i.e. the 'toaddress' is an email address eg. someone@somewhere.com
        // this function returns 'somewhere.com'
        std::string getserveraddress(const std::string& toaddress) const;

        // Does the work of getting MX records for the server returned by 'getserveraddress'
        // will use the dns server passed to this's constructor in 'nameserver'
        // or if MXlookup is false in the constuctor, will return an address
        // for the server that 'getserveraddress' returns.
        // returns false on failure, true on success
        bool gethostaddresses(std::vector<sockaddr_in>& adds);

        // Parses a dns Resource Record (see TCP/IP illustrated, STEVENS, page 194)
        bool parseRR(int& pos, const unsigned char dns[], std::string& name, in_addr& address);

        // Parses a dns name returned in a dns query (see TCP/IP illustrated, STEVENS, page 192)
        void parsename(int& pos, const unsigned char dns[], std::string& name);

        // initialises winsock in win32, does nothing in unix (a definate snore function)
        void init() const;

        // wrapper for closesocket...windows & close...unix
        void Closesocket(const SOCKET& s);

        // less typing later, these are definately abominations!
        typedef std::vector<std::pair<std::vector<char>, std::string> >::const_iterator vec_pair_char_str_const_iter;
        typedef std::vector<std::pair<Address, short> >::const_iterator recipient_const_iter;
        typedef std::vector<std::pair<Address, short> >::iterator recipient_iter;
        typedef std::vector<std::string>::const_iterator vec_str_const_iter;

        // split an address into its relevant parts i.e.
        // name and actual address and return it in Address.
        // this may be usefull out of the class maybe
        // it should be a static function or a global? thinking about it.
        Address parseaddress(const std::string& addresstoparse);

        // The addresses to send the mail to
        std::vector<Address> recipients;
        // The address the mail is from.
        Address fromAddress;

        string errors_toAddress;

        std::vector<ResultMessage> mensagens;

        // Subject of the mail
        std::string _subject;
        // Corpo em texto
        std::string body_text;
	// Corpo em HTML
        std::string body_html;
        // The contents of the mail message
        std::vector<char> message;
        // attachments: the file as a stream of char's and the name of the file.
        std::vector<std::pair<std::vector<char>, std::string> > attachments;
        // This will be filled in from the toAddress by getserveraddress
        std::string server;
        // Name of a nameserver to query
        std::string nameserver;
        // Name of a nameserver to query
        std::string id_camp_peca;
        // The port to mail to on the smtp server.
        const unsigned short port;
        // use dns to query for MX records
        const bool lookupMXRecord;
	// quoted-printable funcao para converter
	char * qpEncode(const char *sfrom, int fromlen, int *tolen = NULL);

	ErrorMessages_t m_ErrorMessages;

	// filled in with server return strings
	std::string returnstring;
/*
	void checklinesarelessthan1000chars();

	// initialises winsock in win32, does nothing in unix (a definate snore function)
	void init() const;
	// wrapper for closesocket...windows & close...unix
	void closesocket(const SOCKET& s);

	// This will be filled in from the toAddress by getserveraddress
	std::string server;

	std::vector<ResultMessage> mensagens;

	// filled in with server return strings
	std::string returnstring;
*/
};

#endif // !ifndef __Mailer_H__

