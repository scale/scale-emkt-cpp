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


typedef struct ErrorMessages {
	    std::vector<std::string> emails_error;
	    std::vector<int> id_email_error;
	    std::string message_error;
	    int id_error;
	} ErrorMessages_t ;


class Mailer
{
public:
        // if MXLookup is true:
        //    'server' is a nameserver to lookup an MX record by.
        // if MXLookup is false.
        //    'server' is an SMTP server which will be attempted directly for mailing
        // if an IP address is not found, either MX record or direct to SMTP server,
        // an attempt will be made to send mail directly to the server in the mail address.
        // e.g. mail to fred@somewhere.com will have a connection attempt made directly to:
        //      somewhere.com
        Mailer(const char* TOaddress, const char* FROMaddress,
                   const char* Subject, const std::vector<char>& Message,
                   const char* server = "127.0.0.1"/*default to localhost*/,
                   unsigned short Port = SMTP_PORT, // default SMTP port
                   bool MXLookup = true);

        Mailer(const char* TOaddress, const char* FROMaddress,
                   const char* Subject, const char* Message,
                   const char* server = "127.0.0.1"/*default to localhost*/,
                   unsigned short Port = SMTP_PORT, // default SMTP port
                   bool MXLookup = true);

	Mailer(std::string = "127.0.0.1"/*default to localhost*/,
                   unsigned short Port = SMTP_PORT, // default SMTP port
                   bool MXLookup = true);

        ~Mailer();

        // call this operator to have the mail mailed.
        // This is to facilitate using multiple threads
        // i.e. using boost::thread.     (see http://www.boost.org)

        //
        // e.g.
        //    Mailer mail(args...);
        //    boost::thread thrd(mail); // operator()() implicitly called.
        //    thrd.join(); // if needed.
        //
        // or:
        //    Mailer mail(args...);
        //    mail.operator()();
        void send();

        // attach a file to the mail. (MIME 1.0)
        // returns false if !filename.length() or
        // the file could not be opened for reading...etc.
        bool attach(const std::string& filename);

        // remove an attachment from the list of attachments.
        // returns false if !filename.length() or
        // the file is not attached or there are no attachments.
        bool removeattachment(const std::string& filename);

        // Set a new message (replacing the old)block_size
        // will return false and not change the message if newmessage is empty.
        bool setmessage(const std::string& newmessage);
        bool setmessage(const std::vector<char>& newmessage);

        // Set a new Subject for the mail (replacing the old)
        // will return false if newSubject is empty.
        bool setsubject(const std::string& newSubject);

        // sets the nameserver or smtp server to connect to
        // dependant on the constructor call, i.e. whether
        // 'lookupMXRecord' was set to false or true.
        // (see constructor comment for details)
        bool setserver(const std::string& nameserver_or_smtpserver);

        // sets the senders address (fromAddress variable)
        bool setsender(const std::string& newsender);

        bool seterrorsto(const std::string& errors_to);

        // add a recipient to the recipient list. (maximum allowed recipients 100).
        // returns true if the address could be added to the
        // recipient list, otherwise false.
        // recipient_type must be in the range Mailer::TO -> Mailer::BCC if
        // not recipient_type defaults to BCC (blind copy), see const enum below.
        bool addrecipient(const std::string& newrecipient, short recipient_type = Bcc /*TO, CC, BCC*/);

        // remove a recipient from the recipient list.
        // returns true if the address could be removed from the
        // recipient list, otherwise false.
        bool removerecipient(const std::string& recipient);

        // clear all recipients from the recipient list.
        void clearrecipients();

        // clear all attachments from the mail.
        void clearattachments();

        // clear all recipients, message, attachments, errors.
        // does not reset the name/smtp server (use setserver for this)
        // does not set the senders address (use setsender for this)
        void reset();

        // returns the return code sent by the smtp server or a local error.
        // this is the only way to find if there is an error in processing.
        // if the mail is sent correctly this string will begin with 250
        // see smtp RFC 821 section 4.2.2 for response codes.
        const std::string& response() const;

        // Constants
        // in unix we have to have a named object, hence the name "consts".
        const static enum {TO, Cc, Bcc, SMTP_PORT = 25, DNS_PORT = 53} consts;


	//Para retornar o dominio a ser dado como EHLO, pegara o dominio do email que esta
	// enviando o email
	char* getDomainEmail(const std::string& email);


	//Para inserir o conteudo TEXT
	bool setBodyText(const char* text);
	//Para inserir o conteudo HTML
	bool setBodyHtml(const char* html);

	//Para devolver o struct que conetem os erros elvantados durante a entrea do email
	ErrorMessages_t getErrorMessages();

	bool setid_camp_peca(const std::string& id);


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

        // email address wrapper struct
        struct Address {
                std::string name;    // e.g.   freddy foobar
                std::string address; // e.g.   someone@mail.com
        };

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
        std::vector<std::pair<Address, short> > recipients;
        // The address the mail is from.
        Address fromAddress;

	string errors_toAddress;

        // Subject of the mail
        std::string subject;
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
};

#endif // !ifndef __Mailer_H__


