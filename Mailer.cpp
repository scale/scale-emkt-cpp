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

#include <errno.h>

Mailer::Mailer(const char* ip) :
	server(getserveraddress(ip)) {
	init(); // in win32 init networking, else just does nothin'
}

Mailer::~Mailer() {
}

void Mailer::html(const std::string& html) {

	message[0] = html;

	// if the message is less than a 1000 chrarcters we do not need to add newlines
	if (message.size() > 1000)
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

void Mailer::setsubject(const std::string& newSubject) {
	if (!newSubject.length())
		return false;

	subject = newSubject;
	return true;
}

void Mailer::from(const std::string& newsender) {
	if (newsender.length() > 0) {
		Address newaddress = parseaddress(newsender);
		fromAddress = newaddress;
	}
}

void Mailer::to(const std::string& name, const std::string& email) {
	// SMTP only allows 100 recipients max at a time.
	// rfc821
	if (recipients.size() >= 100) // == would be fine, but let's be stupid safe
		return;

	if (email.length()) {
		// If there are no recipients yet
		// set the server address for MX queries
		if (!recipients.size()) {
			server = getserveraddress(email);
		}

		Address newaddress = parseaddress(email);
		newaddress.name = name;

		recipients.push_back(newaddress);
	}
}

// this is where we do all the work.
void Mailer::send() {
	Debug debug(1, "Mailer");

	returnstring = ""; // clear out any errors from previous use
	m_ErrorMessages.message_error = "";
	m_ErrorMessages.id_error = 0;

	if (recipients.size() == 0) {
		m_ErrorMessages.message_error = "453 No recipients";
		m_ErrorMessages.id_error = 453;
		returnstring = m_ErrorMessages.message_error;
		throw m_ErrorMessages;
	}

	if (fromAddress.address.length() == 0) {
		m_ErrorMessages.message_error = "454 From email address empty";
		m_ErrorMessages.id_error = 454;
		returnstring = m_ErrorMessages.message_error;
		throw m_ErrorMessages;
	}

	if (nameserver.length() == 0) {
		m_ErrorMessages.message_error = "455 No nameserver";
		m_ErrorMessages.id_error = 455;
		returnstring = m_ErrorMessages.message_error;
		throw m_ErrorMessages;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	if (!adds.size()) { // oops
		m_ErrorMessages.message_error
				= "457 Requested action aborted: No MX records ascertained";
		m_ErrorMessages.id_error = 457;
		returnstring = m_ErrorMessages.message_error;
	}

	const std::string OK("250");
	const std::string smtpheader(makesmtpmessage());
	char buff[1024] = "";

	for (std::vector<sockaddr_in>::const_iterator address = adds.begin(); address
			< adds.end(); ++address) {

		// without the temp variable on RedHat 8.0 we get an error
		//  i.e. this don't work:
		//   connect(s, address, sizeof(*address)); // cannot convert iterator to sockaddr*
		// this should still work on other platforms fine.
		sockaddr temp;
		sockaddr_in *temp_in;
		struct in_addr *temp_addr;

		std::memcpy(&temp, &(*address), sizeof(temp));
		temp_in = (sockaddr_in*) &temp;
		temp_addr = &temp_in->sin_addr;

		// cout << "Conectando a " << inet_ntoa(*temp_addr) << " ... ";
		if (connect(s, &temp, sizeof(temp)) != 0) {
			m_ErrorMessages.message_error = "500 Transaction failed";
			m_ErrorMessages.id_error = 500;
			returnstring = m_ErrorMessages.message_error;
			if (address == (adds.end() - 1)) {
				for (recipient_const_iter recip = recipients.begin(); recip
						< recipients.end(); ++recip) {
					m_ErrorMessages.id_email_error.push_back(451);
					m_ErrorMessages.emails_error.push_back(
							(*recip).first.address);
				}

				throw m_ErrorMessages;
			}
			continue;
		}

		// 220 the server line returned here
		int len1 = recv(s, buff, 1024, 0);

		std::string commandline(std::string("EHLO ") + getDomainEmail(
				fromAddress.address) + std::string("\r\n"));
		// say hello to the server
		len1 = ::send(s, commandline.c_str(), commandline.length(),
				MSG_DONTROUTE | MSG_NOSIGNAL);
		sleep(1);

		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;
		//debug.debug("EHLO : %s",returnstring.c_str());

		if (returnstring.substr(0, 3) != OK) {
			debug.error("%s", returnstring.c_str());
			// maybe we only do non extended smtp
			// send HELO instead.
			commandline[0] = 'H';
			commandline[1] = 'E';
			len1 = ::send(s, commandline.c_str(), commandline.length(),
					MSG_DONTROUTE | MSG_NOSIGNAL);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			if (returnstring.substr(0, 3) != OK) {
				// we must issue a quit even on an error.
				// in keeping with the rfc's
				len1 = ::send(s, "QUIT\r\n", 6, MSG_DONTROUTE | MSG_NOSIGNAL);
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
		if (errors_toAddress.length() > 0)
			commandline = "MAIL FROM:<" + errors_toAddress + ">\r\n";
		else
			commandline = "MAIL FROM:<" + fromAddress.address + ">\r\n";

		len1 = ::send(s, commandline.c_str(), commandline.length(),
				MSG_DONTROUTE | MSG_NOSIGNAL);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;
		//debug.debug("FROM : %s",returnstring.c_str());

		if (returnstring.substr(0, 3) != OK) {
			debug.error("%s", returnstring.c_str());
			// we must issue a quit even on an error.
			// in keeping with the rfc's
			len1 = ::send(s, "QUIT\r\n", 6, MSG_DONTROUTE | MSG_NOSIGNAL);
			len1 = recv(s, buff, 1024, 0);

			m_ErrorMessages.message_error = returnstring;
			m_ErrorMessages.id_error = 555;
			Closesocket(s);
			continue;
		}

		for (recipient_const_iter recip = recipients.begin(); recip
				< recipients.end(); ++recip) {
			// RCPT
			// S: RCPT TO:<Jones@Beta.ARPA>
			// R: 250 OK
			commandline = "RCPT TO: <" + (*recip).first.address + ">\r\n";
			// S: RCPT TO:<Green@Beta.ARPA>
			// R: 550 No such user here
			//
			// S: RCPT TO:<Brown@Beta.ARPA>
			// R: 250 OK

			len1 = ::send(s, commandline.c_str(), commandline.length(),
					MSG_DONTROUTE | MSG_NOSIGNAL);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			debug.debug("RCPT: %s", returnstring.c_str());

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
				m_ErrorMessages.emails_error.push_back((*recip).first.address);

				continue;
			}

			//SUCESSO ao adicionar o destinatario
			m_ErrorMessages.id_email_error.push_back(255);
			m_ErrorMessages.emails_error.push_back((*recip).first.address);
		}

		// DATA
		// S: DATA
		// R: 354 Start mail input; end with <CRLF>.<CRLF>
		// S: Blah blah blah...
		// S: ...etc. etc. etc.
		// S: <CRLF>.<CRLF>
		// R: 250 OK
		len1 = ::send(s, "DATA\r\n", 6, MSG_DONTROUTE);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		if (returnstring.substr(0, 3) != "354") {
			debug.error("%s", returnstring.c_str());
			// we must issue a quit even on an error.
			// in keeping with the rfc's
			len1 = ::send(s, "QUIT\r\n", 6, MSG_DONTROUTE | MSG_NOSIGNAL);
			len1 = recv(s, buff, 1024, 0);
			buff[len1] = '\0';
			returnstring = buff;
			debug.debug("Problemas no comando DATA: %s", returnstring.c_str());

			m_ErrorMessages.message_error = returnstring;
			m_ErrorMessages.id_error = 555;
			Closesocket(s);
			continue;
		}

		// Sending the email
		len1 = ::send(s, smtpheader.c_str(), smtpheader.length(), MSG_DONTROUTE
				| MSG_NOSIGNAL);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		if (returnstring.substr(0, 3) != OK) {
			debug.error("%s", returnstring.c_str());
			// we must issue a quit even on an error.
			// in keeping with the rfc's
			len1 = ::send(s, "QUIT\r\n", 6, MSG_DONTROUTE);
			len1 = recv(s, buff, 1024, 0);
			Closesocket(s);
			continue;
		}

		// hang up the connection
		len1 = ::send(s, "QUIT\r\n", 6, MSG_DONTROUTE);
		len1 = recv(s, buff, 1024, 0);
		buff[len1] = '\0';
		returnstring = buff;

		if (returnstring.substr(0, 3) != "221")
			Closesocket(s);

		Closesocket(s);

		// don't continue as we have delivered the mail
		// at this point just leave
		m_ErrorMessages.message_error
				= "250 Requested mail action okay, completed";
		m_ErrorMessages.id_error = 250;

		//Verificando se foi tudo bem sucedido caso nao seja alimentar os dados
		//no messageErrors para poder trata cadaum posteriormente
		if (m_ErrorMessages.id_email_error.empty()) {
			int code_error = 0;
			if (m_ErrorMessages.id_error > 500) {
				code_error = 533;
			} else if (m_ErrorMessages.id_error > 400) {
				code_error = 450;
			} else {
				code_error = 250;
			}

			for (recipient_const_iter recip = recipients.begin(); recip
					< recipients.end(); ++recip) {
				m_ErrorMessages.id_email_error.push_back(code_error);
				m_ErrorMessages.emails_error.push_back((*recip).first.address);
			}

		}

		//all done
		returnstring = m_ErrorMessages.message_error;
		break;
	}
}

std::string Mailer::makesmtpmessage() const {
	std::string sender(fromAddress.address);
	if (sender.length()) {
		std::string::size_type pos(sender.find("@"));
		if (pos != std::string::npos) { //found the server beginning
			sender = sender.substr(0, pos);
		}
	}
	std::string smtpheader;
	if (fromAddress.name.length())
		smtpheader = "From: " + fromAddress.address + " (" + fromAddress.name
				+ ") \r\n"
					"Reply-To: " + fromAddress.address + "\r\n";
	else if (errors_toAddress.length())
		smtpheader = "From: " + errors_toAddress + " \r\n"
			"Reply-To: " + fromAddress.address + "\r\n";
	else
		smtpheader = "From: " + fromAddress.address + " \r\n"
			"Reply-To: " + fromAddress.address + "\r\n";

	if (errors_toAddress.length())
		smtpheader += "Errors-to: " + errors_toAddress + " \r\n";

	if (id_camp_peca.length()) {
		smtpheader += "X-Emkt: " + id_camp_peca + " \r\n";
	}

	// add the recipients to the header
	std::vector<std::string> to, cc, bcc;
	for (recipient_const_iter recip = recipients.begin(); recip
			< recipients.end(); ++recip) {
		if (recip->second == TO) {
			to.push_back(recip->first.address);
		} else if (recip->second == Cc) {
			cc.push_back(recip->first.address);
		} else if (recip->second == Bcc) {
			bcc.push_back(recip->first.address);
		}
	}
	vec_str_const_iter it; // instead of making three on the stack, just one (stops VC whining too)
	// next section adds To: Cc: Bcc: lines to the header
	int count = to.size();
	if (count)
		smtpheader += "To: ";
	for (it = to.begin(); it < to.end(); ++it) {
		smtpheader += *it;
		if (count > 1 && ((it + 1) < to.end()))
			smtpheader += ",\r\n "; // must add a space after the comma
		else
			smtpheader += "\r\n";
	}
	count = cc.size();
	if (count)
		smtpheader += "Cc: ";
	for (it = cc.begin(); it < cc.end(); ++it) {
		smtpheader += *it;
		if (count > 1 && ((it + 1) < cc.end()))
			smtpheader += ",\r\n "; // must add a space after the comma
		else
			smtpheader += "\r\n";
	}
	// end adding To: Cc: Bcc: lines to the header

	std::string boundary("----Scale_NextP_0056wi_0_8_ty789432_tp");
	bool MIME(true);

	smtpheader += "MIME-Version: 1.0\r\n"
		"Content-Type: multipart/alternative;"
		"boundary=\"" + boundary + "\"\r\n";

	///////////////////////////////////////////////////////////////////////////
	// add the current time.
	// format is
	//     05 Jan 93 21:22:07
	//     05 Jan 93 21:22:07 PST
	time_t t;
	time(&t);
	tm* ptm = localtime(&t);
	if (ptm) {
		smtpheader += "Date: ";
		std::ostringstream str;
		if (ptm->tm_mday < 10) // add a trailing zero if sigle digit
			str << "0";
		str << ptm->tm_mday << " ";
		switch (ptm->tm_mon) {
		case 0:
			str << "Jan ";
			break;
		case 1:
			str << "Feb ";
			break;
		case 2:
			str << "Mar ";
			break;
		case 3:
			str << "Apr ";
			break;
		case 4:
			str << "May ";
			break;
		case 5:
			str << "Jun ";
			break;
		case 6:
			str << "Jul ";
			break;
		case 7:
			str << "Aug ";
			break;
		case 8:
			str << "Sep ";
			break;
		case 9:
			str << "Oct ";
			break;
		case 10:
			str << "Nov ";
			break;
		case 11:
			str << "Dec ";
			break;
		default:
			str << "Jan "; // be safe
		}

		std::ostringstream year;
		year << ptm->tm_year;
		str << year.str().substr(year.str().length() - 2, 2) << " ";
		str << ptm->tm_hour << ":" << ptm->tm_min << ":" << ptm->tm_sec
				<< "\r\n";

		smtpheader += str.str(); // add the date to the headers
	}
	///////////////////////////////////////////////////////////////////////////

	// add the subject
	QuotedPrintable qp;
	smtpheader += "Subject: =?iso-8859-1?Q?" + qp.Encode(subject)
			+ "?=\r\n\r\n";
	// everything else added is the body of the email message.

	smtpheader += "This is a MIME encapsulated message\r\n\r\n";
	smtpheader += "--" + boundary + "\r\n";
	smtpheader += "Content-type: text/plain; charset=iso-8859-1\r\n"
		"Content-transfer-encoding: 8BIT\r\n\r\n";

	smtpheader += "\r\n\r\n--" + boundary + "\r\n";
	smtpheader += "Content-type: text/plain; charset=iso-8859-1\r\n"
		"Content-transfer-encoding: quoted-printable\r\n\r\n";
	smtpheader += body_text;

	if (body_html.length() > 1) {
		smtpheader += "\r\n\r\n--" + boundary + "\r\n";
		smtpheader += "Content-type: text/html; charset=iso-8859-1\r\n"
			"Content-transfer-encoding: quoted-printable\r\n\r\n";
		smtpheader += body_html;
	}

	smtpheader += "\r\n\r\n--" + boundary + "--\r\n";

	// end the data in the message.
	smtpheader += "\r\n.\r\n";

	return smtpheader;
}

bool Mailer::attach(const std::string& filename) {
	if (!filename.length()) // do silly checks.
		return false;

	std::ifstream file(filename.c_str(), std::ios::binary | std::ios::in);
	if (!file)
		return false;

	std::vector<char> filedata;
	char c = file.get();
	for (; file.good(); c = file.get()) {
		if (file.bad())
			break;
		filedata.push_back(c);
	}

	//        filedata = base64encode(filedata);

	std::string fn(filename);
	std::string::size_type p = fn.find_last_of('/');
	if (p == std::string::npos)
		p = fn.find_last_of('\\');
	if (p != std::string::npos) {
		p += 1; // get past folder delimeter
		fn = fn.substr(p, fn.length() - p);
	}

	attachments.push_back(std::make_pair(filedata, fn));

	return true;
}

bool Mailer::removeattachment(const std::string& filename) {
	if (!filename.length()) // do silly checks.
		return false;

	if (!attachments.size())
		return false;

	std::string fn(filename);
	std::string::size_type p = fn.find_last_of('/');
	if (p == std::string::npos)
		p = fn.find_last_of('\\');
	if (p != std::string::npos) {
		p += 1; // get past folder delimeter
		fn = fn.substr(p, fn.length() - p);
	}
	// fn is now what is stored in the attachments pair as the second parameter
	// i.e.  it->second == fn
	std::vector<std::pair<std::vector<char>, std::string> >::iterator it;
	for (it = attachments.begin(); it < attachments.end(); ++it) {
		if ((*it).second == fn) {
			attachments.erase(it);
			return true;
		}
	}
	return false;
}

// returns everything after the '@' synbol in an email address
// if there is no '@' symbol returns the empty string.
std::string Mailer::getserveraddress(const std::string& toaddress) const {
	if (toaddress.length()) {
		std::string::size_type pos(toaddress.find("@"));
		if (pos != std::string::npos) { //found the server beginning
			if (++pos < toaddress.length())
				return toaddress.substr(pos, toaddress.length() - pos);
		}
	}
	return "";
}

// this function has to get an MX record for 'server'
// and return its address. Correct form for smtp.
// as the domain 'server' may not be the mail server for the server domain!!
bool Mailer::gethostaddresses(std::vector<sockaddr_in>& adds) {
	adds.clear(); // be safe in case of my utter stupidity

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(DNS_PORT); // default dns port

	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
	hostent* host = 0;

#ifdef WIN32
	addr.sin_addr.S_un.S_addr = inet_addr(nameserver.c_str());
	if(addr.sin_addr.S_un.S_addr != INADDR_NONE) {
#else
	if (inet_aton(nameserver.c_str(), &addr.sin_addr)) {
#endif
		host = gethostbyaddr((char*) &addr.sin_addr, sizeof(addr.sin_addr),
				AF_INET);
	} else
		host = gethostbyname(nameserver.c_str());
	if (!host) { // couldn't get to dns, try to connect directly to 'server' instead.
		////////////////////////////////////////////////////////////////////////////////
		// just try to deliver mail directly to "server"
		// as we didn't get an MX record.
		addr.sin_family = AF_INET;
		addr.sin_port = port; // smtp port!! 25
#ifdef WIN32
		addr.sin_addr.S_un.S_addr = inet_addr(server.c_str());
		if(addr.sin_addr.S_un.S_addr != INADDR_NONE) {
#else
		if (inet_aton(server.c_str(), &addr.sin_addr)) {
#endif
			host = gethostbyaddr((char*) &addr.sin_addr, sizeof(addr.sin_addr),
					AF_INET);
		} else
			host = gethostbyname(server.c_str());

		if (!host) {
			m_ErrorMessages.message_error
					= "550 Requested action not taken: mailbox unavailable";
			m_ErrorMessages.id_error = 550;
			returnstring = m_ErrorMessages.message_error;
			//std::cout << returnstring << std::endl;
			return false; // error!!!
		}

		memcpy((char*) &addr.sin_addr, host->h_addr, host->h_length);

		adds.push_back(addr);
		//std::cout << "sucesso " << host->h_addr << std::endl;
		return true;
	} else
		memcpy((char*) &addr.sin_addr, host->h_addr, host->h_length);

	bzero(&(addr.sin_zero), 8);
#ifdef WIN32
	if(connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#else
	if (connect(s, (sockaddr*) &addr, sizeof(addr))) {
#endif
		returnstring = "451 Requested action aborted: socket error: ";
		std::cerr << returnstring << get_socket_error(errno) << std::endl;
		return false; // dns connection unavailable
	}

	// dnsheader info         id    flags   num queries
	unsigned char dns[512] = { 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 };
	int dnspos = 12; // end of dns header
	std::string::size_type stringpos(0);
	std::string::size_type next(server.find("."));
	if (next != std::string::npos) { // multipart name e.g. "aserver.somewhere.net"
		while (stringpos < server.length()) {
			std::string part(server.substr(stringpos, next - stringpos));
			dns[dnspos] = part.length();
			++dnspos;
			for (std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
				dns[dnspos] = part[i];
			}

			stringpos = ++next;
			next = server.find(".", stringpos);
			if (next == std::string::npos) {
				part = server.substr(stringpos, server.length() - stringpos);
				dns[dnspos] = part.length();
				++dnspos;
				for (std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
					dns[dnspos] = part[i];
				}
				break;
			}
		}
	} else { // just a single part name. e.g. "aserver"
		dns[dnspos] = server.length();
		++dnspos;
		for (std::string::size_type i = 0; i < server.length(); ++i, ++dnspos) {
			dns[dnspos] = server[i];
		}
	}
	// in case the server string has a "." on the end
	if (server[server.length() - 1] == '.')
		dns[dnspos] = 0;
	else
		dns[dnspos++] = 0;

	// add the class & type
	dns[dnspos++] = 0;
	dns[dnspos++] = 15; // MX record.
	dns[dnspos++] = 0;
	dns[dnspos++] = 1;

	dnspos = ::send(s, (char*) dns, dnspos, MSG_DONTROUTE);
	dnspos = recv(s, (char*) dns, 512, 0);

	Closesocket(s);
#ifdef WIN32
	if(dnspos != SOCKET_ERROR) {
#else
	if (dnspos > -1) {
#endif
		// now parse the data sent back from the dns for MX records
		if (dnspos > 12) { // we got more than a dns header back
			unsigned short numsitenames = ((unsigned short) dns[4] << 8)
					| dns[5];
			unsigned short numanswerRR = ((unsigned short) dns[6] << 8)
					| dns[7];
			unsigned short numauthorityRR = ((unsigned short) dns[8] << 8)
					| dns[9];
			unsigned short numadditionalRR = ((unsigned short) dns[10] << 8)
					| dns[11];

			if (!(dns[3] & 0x0F)) { // check for an error
				// int auth((dns[2] & 0x04)); // AA bit. the nameserver has given authoritive answer.
				int pos = 12; // start after the header.

				std::string questionname;
				if (numsitenames) {
					parsename(pos, dns, questionname);
					pos += 4; // move to the next RR
				}

				// This gives this warning in VC.
				// bloody annoying, there is a way round it according to microsoft.
				// The debugger basically cannot browse anything with a name
				// longer than 256 characters, "get with the template program MS".
				// #pragma warning( disable : 4786 )
				// #pragma warning( default : 4786 )
				std::vector < std::string > names;
				in_addr address;
				std::string name;
				// VC++ incompatability scoping
				// num should be able to be declared in every for loop here
				// not in VC
				int num = 0;
				for (; num < numanswerRR; ++num) {
					name = "";
					parseRR(pos, dns, name, address);
					if (name.length())
						names.push_back(name);
				}
				for (num = 0; num < numauthorityRR; ++num) {
					name = "";
					parseRR(pos, dns, name, address);
					if (name.length())
						names.push_back(name);
				}
				for (num = 0; num < numadditionalRR; ++num) {
					name = "";
					parseRR(pos, dns, name, address);
					if (name.length())
						names.push_back(name);
				}

				// now get all the MX records IP addresess
				addr.sin_family = AF_INET;
				addr.sin_port = port; // smtp port!! 25
				hostent* host = 0;
				for (vec_str_const_iter it = names.begin(); it < names.end(); ++it) {
					host = gethostbyname(it->c_str());
					if (!host) {
#ifdef WIN32
						addr.sin_addr.S_un.S_addr = 0;
#else
						addr.sin_addr.s_addr = 0;
#endif
						continue; // just skip it!!!
					}
					memcpy((char*) &addr.sin_addr, host->h_addr, host->h_length);
					adds.push_back(addr);
				}
				// got the addresses
				return true;
			}
		}
	}
	// what are we doing here!!
	m_ErrorMessages.message_error = "460 Dominio inexistente: " + server;
	m_ErrorMessages.id_error = 460;
	return false;
}

// This does nothing on unix.
// for windoze only, to initialise networking, snore
void Mailer::init() const {
#ifdef WIN32
	class socks
	{
	public:
		bool init() {

			WORD wVersionRequested;
			WSADATA wsaData;

			wVersionRequested = MAKEWORD( 2, 0 );
			int ret = WSAStartup( wVersionRequested, &wsaData);
			if(ret)
			return false;
			initialised = true;
			return true;
		}
		bool IsInitialised() const {return initialised;}
		socks():initialised(false) {init();}
		~socks()
		{
			if(initialised)
			shutdown();
		}
	private:
		void shutdown() {WSACleanup();}
		bool initialised;
	};
	static socks s;
#endif
}

const std::string& Mailer::response() const {
	return returnstring;
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

Mailer::Address Mailer::parseaddress(const std::string& addresstoparse) {
	Address newaddress; // return value

	// do some silly checks
	if (!addresstoparse.length())
		return newaddress; // its empty, oops (this should fail at the server.)

	if (!addresstoparse.find("@") == std::string::npos) {
		// no '@' symbol (could be a local address, e.g. root)
		// so just assume this. The SMTP server should just deny delivery if its messed up!
		newaddress.address = addresstoparse;
		return newaddress;
	}

	// we have one angle bracket but not the other
	// (this isn't strictly needed, just thought i'd throw it in)
	if (((addresstoparse.find('<') != std::string::npos)
			&& (addresstoparse.find('>') == std::string::npos))
			|| ((addresstoparse.find('>') != std::string::npos)
					&& (addresstoparse.find('<') == std::string::npos))) {
		return newaddress; // its empty, oops (this should fail at the server.)
	}

	// we have angle bracketed delimitered address
	// like this maybe:
	//        "foo@bar.com"
	// or     "foo bar <foo@bar.com>"
	// or     "<foo@bar.com> foo bar"
	if ((addresstoparse.find('<') != std::string::npos)
			&& (addresstoparse.find('>') != std::string::npos)) {
		std::string::size_type sta = addresstoparse.find('<');
		std::string::size_type end = addresstoparse.find('>');

		newaddress.address = addresstoparse.substr(sta + 1, end - sta - 1);

		if (sta > 0) { // name at the beginning
			end = sta - 1;
			//if(addresstoparse.length() < sta) { // no name to get
			//      return newaddress;
			//}
			newaddress.name = addresstoparse.substr(0, end);
			return newaddress;
		} else { // name at the end
			// no name to get
			if (end >= addresstoparse.length() - 1)
				return newaddress;

			end += 2;
			if (end >= addresstoparse.length())
				return newaddress;

			newaddress.name = addresstoparse.substr(end,
					addresstoparse.length() - end);
			// remove whitespace from end if need be
			if (newaddress.name[newaddress.name.length() - 1] == ' ')
				newaddress.name = newaddress.name.substr(0,
						newaddress.name.length() - 1);
			return newaddress;
		}
	}

	// if we get here assume an address of the form: foo@bar.com
	// and just save it.
	newaddress.address = addresstoparse;

	return newaddress;
}

void Mailer::text(const std::string& text) {
	body_text = text;

	return true;
}

void Mailer::html(const std::string& html) {
	QuotedPrintable enc;
	body_html = enc.Encode(html);
	return true;
}

char * Mailer::qpEncode(const char *sfrom, int fromlen, int *tolen) {
	unsigned char *from = (unsigned char *) sfrom;
	unsigned char hex[] = "0123456789ABCDEF";

	// Allocate output buffer: max size is 3 bytes per input byte + 3
	// bytes of soft line break every 75 output bytes.
	int m_tolen = 3 * fromlen + 3 * ((3 * fromlen) / 75) + 250;
	unsigned char *to_base = (unsigned char *) malloc(m_tolen);
	if (to_base == 0)
		return 0;

	unsigned char *to = to_base;
	unsigned char *line_base = to;

	fromlen = strlen(sfrom);

	unsigned char c = *from;
	int bla = 0;
	//while (fromlen-- || bla < fromlen) {
	while (1) {
		bla++;
		// Process next char
		unsigned char c = *from++;

		if (c == 0) {
			break;

		}
		// Hard Line break ?
		if (c == '\n' || (c == '\r' && fromlen && *from == '\n')) {
			*to++ = '\r';
			*to++ = '\n';
			if (*from == '\n') {
				//fromlen--;
				*from++;
			}
			line_base = to;
			continue;
		}
		// Need quoting ?
		int needquote = 1;
		if ((33 <= c && c <= 60) || (62 <= c && c <= 126) || (c == 32
				&& fromlen && *from != '\r' && *from != '\n'))
			needquote = 0;

		// Need soft line break ?
		if ((needquote && (to - line_base >= 73)) || (!needquote && (to
				- line_base >= 75))) {
			memcpy(to, "=\r\n", 3);
			to += 3;
			line_base = to;
		}

		if (c == '=' || c == 61) {
			needquote = false;
			*to++ = '=';
			*to++ = '3';
			*to++ = 'D';
			continue;
		}

		if (needquote) {
			if (c == 32)
				continue;

			int i = 'C';
			if (c > 150)
				i = 'E';
			*to++ = '=';
			*to++ = i;//hex[(c >> 4)]+i;
			*to++ = hex[c & 0xF];
			char teste = hex[c & 0xF];

			fromlen--;
		} else {
			*to++ = c;
		}
	}
	*to++ = 0;

	return (char *) to_base;
}

void Mailer::substitute(const std::string& name, const std::string value) {
	macros[name] = value;
}

ErrorMessages_t Mailer::getErrorMessages() {

	return m_ErrorMessages;

}
