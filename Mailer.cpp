/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>   // ostrstream
#include <ctime>     // for localtime
#include "Mailer.h"
#include "QuotedPrintable.h"
#include "MicroDNS.h"

#include <strings.h>
#include <errno.h>

Mailer::Mailer(const std::string& s) :
	server(s), port(25), lookupMXRecord(true) {
	init(); // in win32 init networking, else just does nothin'
}

Mailer::~Mailer() {
}

void Mailer::html(const std::string& html) {

	body_html = html;

	// if the message is less than a 1000 chrarcters we do not need to add newlines
	if (body_html.size() > 1000)
		checklinesarelessthan1000chars();
}

// this breaks a message line up to be less than 1000 chars per line.
// keeps words intact also.
void Mailer::checklinesarelessthan1000chars() {
	int count(1);
	for (std::vector<char>::iterator it = message.begin(); it < message.end(); ++it, ++count) {
		if (*it == '\r') {
			count = 0; // reset for a new line.
			++it; // get past newline
			continue;
		} else if (count >= 998) {
			++it;
			if (*it != ' ') { // we are in a word!!
				// it should never get to message.begin() because we
				// start at least 998 chars into the message!
				// Also, assume a word isn't bigger than 997 chars! (seems reasonable)
				std::vector<char>::iterator pos = it;
				for (int j = 0; j < 997; ++j, --pos) {
					if (*pos == ' ') {
						it = ++pos; // get past the space.
						break;
					}
				}
			}
			if (it < message.end())
				it = message.insert(it, '\r');
			++it;
			if (it < message.end())
				it = message.insert(it, '\n');
			count = 0; // reset for a new line.
		}
	}
}

void Mailer::subject(const std::string& newSubject) {
	if (newSubject.length() > 0)
		_subject = newSubject;
}

void Mailer::to(const std::string& name, const std::string& email) {
	// SMTP only allows 100 recipients max at a time.
	// rfc821
	if (recipients.size() >= 20) // == would be fine, but let's be stupid safe
		return;

	if (email.length()) {
		Address newaddress = parseaddress(email);
		newaddress.name = name;

		recipients.push_back(newaddress);
	}
}

// this is where we do all the work.
void Mailer::send() {
	Debug debug("Mailer");

	const std::string OK("250");
	std::string smtpheader;
	char buff[1024] = "";

#ifdef LINUX
		int sock_flags = MSG_OOB | MSG_NOSIGNAL;
#else
		int sock_flags = MSG_OOB;
#endif

	returnstring = ""; // clear out any errors from previous use
	m_ErrorMessages.message_error = "";
	m_ErrorMessages.id_error = 0;

	std::vector<sockaddr_in> adds;

	//TODO
	MicroDNS mdns;
	struct mx _mx = mdns.mx(recipients.front().domain());

	struct sockaddr_in mx_host;
	mx_host.sin_addr = _mx.addr;
	mx_host.sin_port = htons(25);
	mx_host.sin_family = AF_INET;

	adds.push_back(mx_host);
	/*
	 if(lookupMXRecord) {
	 if(!gethostaddresses(adds)) {
	 // error!! we are dead.
	 if( m_ErrorMessages.id_error < 500){
	 m_ErrorMessages.message_error = "456 Requested action aborted: No MX records ascertained";
	 m_ErrorMessages.id_error = 456;
	 }
	 returnstring = m_ErrorMessages.message_error;
	 //	if (fromAddress.email.length() == 0) {
	 //		ResultMessage msg;
	 //		msg.error = 1;
	 //		msg.message = "'From' email address empty";

	 mensagens.push_back(msg);

	 throw msg;
	 }
	 */

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	for (std::vector<sockaddr_in>::const_iterator address = adds.begin(); address
			< adds.end(); ++address) {

		// without the temp variable on RedHat 8.0 we get an error
		//  i.e. this don't work:
		//   connect(s, address, sizeof(*address)); // cannot convert iterator to sockaddr*
		// this should still work on other platforms fine.
		sockaddr temp;
		sockaddr_in *temp_in;
		struct in_addr *temp_addr;

		//		addr.sin_family = AF_INET;
		//		addr.sin_port = 25; // smtp port!! 25

		::memcpy(&temp, &(*address), sizeof(temp));

		temp_in = (sockaddr_in*) &temp;
		temp_addr = &temp_in->sin_addr;

		// cout << "Conectando a " << inet_ntoa(*temp_addr) << " ... ";
		if (connect(s, &temp, sizeof(temp)) != 0) {
			for (vector<Address>::iterator itr = recipients.begin(); itr
					!= recipients.end(); ++itr) {
				ResultMessage msg;

				msg.error = 400;
				msg.message = "Could not connect.";
				msg.recipient = (*itr);

				mensagens.push_back(msg);
			}
		}

		// 220 the server line returned here
		int len1 = recv(s, buff, 1024, 0);

		std::ostringstream cmd;

		cmd << "EHLO " << fromAddress.domain() << std::endl;

		// say hello to the server
		len1 = ::send(s, cmd.str().c_str(), cmd.str().size(), sock_flags);
		//		sleep(1);

		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;
		//debug.debug("EHLO : %s",returnstring.c_str());

		if (returnstring.substr(0, 3) != OK) {
			std::ostringstream cmd;

			debug.error("%s", returnstring.c_str());

			cmd << "HELO " << fromAddress.domain() << std::endl;

			len1 = ::send(s, cmd.str().c_str(), cmd.str().size(), sock_flags);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;

			if (returnstring.substr(0, 3) != OK) {
				// we must issue a quit even on an error.
				// in keeping with the rfc's
				len1 = ::send(s, "QUIT\r\n", 6, sock_flags);
				len1 = recv(s, buff, 1024, 0);
				Closesocket(s);
				// don't know what went wrong here if we are connected!!
				// we continue because maybe we have more than 1 serevr to connect to.
				continue;
			}
		}

		// MAIL
		// S: MAIL FROM:<Smith@Alpha.ARPA>
		// R: 250 OK
		// e.g. "MAIL FROM:<someone@somewhere.com>\r\n"
		// or   "MAIL FROM: John Wiggins <someone@somewhere.com>"
		cmd.str("");

		cmd << "MAIL FROM:<" << fromAddress.email << ">" << std::endl;

		len1 = ::send(s, cmd.str().c_str(), cmd.str().size(), sock_flags);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		if (returnstring.substr(0, 3) != OK) {
			debug.error("%s", returnstring.c_str());
			// we must issue a quit even on an error.
			// in keeping with the rfc's
			len1 = ::send(s, "QUIT\r\n", 6, sock_flags);
			len1 = recv(s, buff, 1024, 0);

			for (vector<Address>::iterator itr = recipients.begin(); itr
					!= recipients.end(); ++itr) {
				ResultMessage msg;

				msg.error = atoi(returnstring.substr(0, 3).c_str());
				msg.message = returnstring;
				msg.recipient = (*itr);

				mensagens.push_back(msg);
			}

			Closesocket(s);
		}

		bool algumOK = false;

		for (vector<Address>::iterator itr = recipients.begin(); itr
				!= recipients.end(); ++itr) {
			// RCPT
			// S: RCPT TO:<Jones@Beta.ARPA>
			// R: 250 OK
			cmd.str("");
			cmd << "RCPT TO: <" << (*itr).email << ">" << std::endl;
			// S: RCPT TO:<Green@Beta.ARPA>
			// R: 550 No such user here
			//
			// S: RCPT TO:<Brown@Beta.ARPA>
			// R: 250 OK

			len1 = ::send(s, cmd.str().c_str(), cmd.str().size(), sock_flags);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			//debug.debug("RCPT: %s",returnstring.c_str());

			if (returnstring.substr(0, 3) != OK) {
				debug.error("%s", returnstring.c_str());
				// This particular recipient does not exist!
				// not strictly an error as we may have more than one recipient
				// we should have an error vector e.g.
				// vector<pair<string address, string error> > errs;
				// errs.push_back(make_pair(recip->first, returnstring));
				//
				// we then need a function to return this vector.
				// e.g. const vector<pair<string address, string error> >& getrecipienterrors();

				if ((returnstring.substr(0, 3)).c_str() != NULL)
					m_ErrorMessages.id_email_error.push_back(atoi(
							(returnstring.substr(0, 3)).c_str()));
				else
					m_ErrorMessages.id_email_error.push_back(511);

				m_ErrorMessages.emails_error.push_back((*itr));
				continue;
			}

			//SUCESSO ao adicionar o destinatario
			m_ErrorMessages.id_email_error.push_back(255);
			m_ErrorMessages.emails_error.push_back((*itr));

			debug.debug("RCPT %s: %s", (*itr).email.c_str(),
					returnstring.c_str());

			ResultMessage msg;

			msg.error = atoi(returnstring.substr(0, 3).c_str());
			msg.message = returnstring;
			msg.recipient = (*itr);

			mensagens.push_back(msg);

			if (returnstring.substr(0, 3) != OK) {
				debug.error("%s", returnstring.c_str());
				// This particular recipient does not exist!
				// not strictly an error as we may have more than one recipient
				// we should have an error vector e.g.
				// vector<pair<string address, string error> > errs;
				// errs.push_back(make_pair(recip->first, returnstring));
				//
				// we then need a function to return this vector.
				// e.g. const vector<pair<string address, string error> >& getrecipienterrors();
				continue;
			} else {
				algumOK = true;
			}
		}

		if (!algumOK) {
			len1 = ::send(s, "QUIT\r\n", 6, sock_flags);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			debug.debug("Nenhum destinatario valido");

			Closesocket(s);
			return;
		}

		// DATA
		// S: DATA
		// R: 354 Start mail input; end with <CRLF>.<CRLF>
		// S: Blah blah blah...
		// S: ...etc. etc. etc.
		// S: <CRLF>.<CRLF>
		// R: 250 OK
		len1 = ::send(s, "DATA\r\n", 6, sock_flags);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		if (returnstring.substr(0, 3) != "354") {
			debug.error("%s", returnstring.c_str());
			// we must issue a quit even on an error.
			// in keeping with the rfc's
			len1 = ::send(s, "QUIT\r\n", 6, sock_flags);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			debug.debug("Problemas no comando DATA: %s", returnstring.c_str());

			//			ResultMessage msg;
			//			msg.error = atoi( returnstring.substr(0, 3).c_str() );
			//			msg.message = returnstring;
			//			msg.recipient = *itr;
			//
			//			mensagens.push_back(msg);

			Closesocket(s);
			break;
		}

		smtpheader = makesmtpmessage();
		// Sending the email
		len1 = ::send(s, smtpheader.c_str(), smtpheader.length(), sock_flags);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		// hang up the connection
		len1 = ::send(s, "QUIT\r\n", 6, sock_flags);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		Closesocket(s);

		break;
	}
}

std::string Mailer::makesmtpmessage() const {
	std::ostringstream os;
	QuotedPrintable qp;

	std::string sender(fromAddress.email);

	if (sender.length()) {
		std::string::size_type pos(sender.find("@"));
		if (pos != std::string::npos) { //found the server beginning
			sender = sender.substr(0, pos);
		}
	}
	std::string smtpheader;
	if (fromAddress.name.length()) {
		os << "From: \"" << fromAddress.name << "\" <" + fromAddress.email
				<< ">" << std::endl;
	} else {
		os << "From: <" + fromAddress.email << ">" << std::endl;
	}

	// add the recipients to the header
	if (recipients.size() == 1) {
		Address r = recipients[0];
		os << "To: \"" << r.name << "\" <" << r.email << ">" << std::endl;
	} else {
		os << "To: \"" << fromAddress.name << "\" <" << fromAddress.email
				<< ">" << std::endl;
	}

	std::string boundary("----=_NextPart_AUTHORIZED_EMAIL");

	os << "MIME-Version: 1.0" << std::endl;

	//Message-ID: <2010103122258.16.fcmaia@scale.com.br>
	//Date: Sun, 31 Oct 2010 2:22:58 -0300
	char buff[200];
	time_t t;
	time(&t);
	tm* ptm = localtime(&t);

	strftime(buff, 200, "%s", ptm);
	os << "Message-ID: <" << buff << "." << fromAddress.email << ">"
			<< std::endl;

	memset(buff, sizeof(char), 200);
	strftime(buff, 200, "%a, %d %b %Y %H:%M:%S %z", ptm);
	os << "Date: " << buff << std::endl;

	// add the subject
	os << "Subject: =?iso-8859-1?Q?" << qp.Encode(_subject) << "?="
			<< std::endl;

	os << "Content-Type: multipart/alternative; boundary=\"" << boundary
			<< "\"" << std::endl;

	if (id_camp_peca.length()) {
		os << "X-Campanha_Peca: " << id_camp_peca << std::endl;
	}

	// everything else added is the body of the email message.

	os << std::endl;

	os << "This is a MIME encapsulated message" << std::endl << std::endl;

	os << "--" << boundary << std::endl;
	os << "Content-type: text/plain; charset=iso-8859-1" << std::endl;
	os << "Content-transfer-encoding: 8BIT" << std::endl;

	os << std::endl;
	os << std::endl;

	os << "--" << boundary << std::endl;
	os << "Content-type: text/plain; charset=iso-8859-1" << std::endl;
	os << "Content-transfer-encoding: quoted-printable" << std::endl
			<< std::endl;

	os << body_text << std::endl;

	os << std::endl;
	os << std::endl;

	os << "--" << boundary << std::endl;
	os << "Content-type: text/html; charset=iso-8859-1" << std::endl;
	os << "Content-transfer-encoding: quoted-printable" << std::endl
			<< std::endl;

	os << qp.Encode(body_html) << std::endl;

	os << std::endl;
	os << std::endl;

	os << "--" << boundary << "--" << std::endl;

	os << std::endl;
	os << "." << std::endl;

	return os.str().c_str();
}

// This does nothing on unix.
// for windoze only, to initialise networking, snore
void Mailer::init() const {
#ifdef WIN32
	class socks {
	public:
		bool init() {

			WORD wVersionRequested;
			WSADATA wsaData;

			wVersionRequested = MAKEWORD( 2, 0 );
			int ret = WSAStartup(wVersionRequested, &wsaData);
			if (ret)
				return false;
			initialised = true;
			return true;
		}
		bool IsInitialised() const {
			return initialised;
		}
		socks() :
			initialised(false) {
			init();
		}
		~socks() {
			if (initialised)
				shutdown();
		}
	private:
		void shutdown() {
			WSACleanup();
		}
		bool initialised;
	};
	static socks s;
#endif
}

// just wrap the call to shutdown the connection on a socket
// this way I don't have to call it this way everywhere.
void Mailer::Closesocket(const SOCKET& s) {
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
}

