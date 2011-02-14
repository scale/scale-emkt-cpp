/*
 * Message.cpp
 *
 *  Created on: 14/02/2011
 *      Author: fcmaia
 */

#include "Message.h"

Message::Message() : status(0) {

}

Message::~Message() {

}

void
Message::peca(Peca & peca) {
	subject = peca.subject;
	from = peca.from;

	_html = peca.html;
}

const std::string &
Message::html()
{
	substitute(_html, "#NOME#", recipient.name);
	substitute(_html, "#EMAIL#", recipient.email);
	substitute(_html, "#ID#", recipient.id);

	return _html;
}

const std::string &
Message::text()
{
	substitute(_text, "#NOME#", recipient.name);
	substitute(_text, "#EMAIL#", recipient.email);
	substitute(_text, "#ID#", recipient.id);

	return _text;
}

void
Message::substitute(std::string contexto, const std::string what, const std::string new_token)
{
	std::string::iterator<std::string> it = contexto.find(what);

	while (it != contexto.end())
	{
		contexto.replace(it, it + new_token.size(), new_token);
		it = contexto.find(what, it);
	}
}
