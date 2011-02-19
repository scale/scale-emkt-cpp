/*
 * Message.h
 *
 *  Created on: 14/02/2011
 *      Author: fcmaia
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Peca.h"

class Address {
public:
	std::string id;
	std::string name;    // e.g.   freddy foobar
	std::string email; // e.g.   someone@mail.com

	std::string domain() {
		std::string::size_type pos = email.find('@');

		if( pos == std::string::npos) {
			return email;
		} else {
			return email.substr(pos+1);
		}
	}

	Address & operator =( const Address &other) {
		id = other.id;
		name = other.name;
		email = other.email;
		return *this;
	}
};

class Message {
public:
	Message();
	virtual ~Message();

	struct mx mx;
	Address from;
	Address recipient;
	std::string subject;

	int status;
	std::string result;

	void peca(Peca& peca);
	Peca &peca() { return _peca; }
	const std::string& html();
	const std::string& text();

private:
	Peca _peca;
	std::string _html;
	std::string _text;

	void substitute(std::string contexto, const std::string what, const std::string new_token);

	std::string	md5(const char *keyword);
};

#endif /* MESSAGE_H_ */
