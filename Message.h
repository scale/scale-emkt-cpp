/*
 * Message.h
 *
 *  Created on: 14/02/2011
 *      Author: fcmaia
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Peca.h"

class Message {
public:
	Message();
	virtual ~Message();

	Address from;
	Address recipient;
	std::string subject;

	int status;
	std::string result;

	void peca(Peca& peca);
	Peca &peca() { return _peca; }
	const std::string& html() const;
	const std::string& text() const;

private:
	Peca _peca;
	std::string _html;
	std::string _text;

	void substitute(std::string contexto, const std::string what, const std::string new_token);
};

#endif /* MESSAGE_H_ */
