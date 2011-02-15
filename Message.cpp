/*
 * Message.cpp
 *
 *  Created on: 14/02/2011
 *      Author: fcmaia
 */

#include "Message.h"
extern "C" {
#include <openssl/md5.h>
}

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
	substitute(_html, "##NOME##", recipient.name);
	substitute(_html, "##EMAIL##", recipient.email);
	substitute(_html, "##ID##", recipient.id);
	substitute(_html, "##EMAIL_MD5##", md5(recipient.email.c_str()) );

	return _html;
}

const std::string &
Message::text()
{
	substitute(_text, "##NOME##", recipient.name);
	substitute(_text, "##EMAIL##", recipient.email);
	substitute(_text, "##ID##", recipient.id);
	substitute(_text, "##EMAIL_MD5##", md5(recipient.email.c_str()) );

	return _text;
}

void
Message::substitute(std::string contexto, const std::string what, const std::string new_token)
{
	std::string::size_type pos = contexto.find(what);

	while (pos != contexto.end())
	{
		contexto.replace(pos, pos + new_token.size(), new_token);
		pos = contexto.find(what, it);
	}
}

std::string
Message::md5(const char *keyword)
{
	unsigned int counter;
	MD5_CTX ctx;
	char ptr_byte[3];
	unsigned char *ptr;
	unsigned char digest[16];
	unsigned char md5email[32];

	MD5_Init(&ctx);
	MD5_Update(&ctx, keyword, strlen(keyword));
	MD5_Final(digest, &ctx);

	ptr = md5email;
	for (counter = 0; counter < sizeof(digest); counter++, ptr
			+= 2) {
		sprintf(ptr_byte, "%02x", digest[counter] & 0xFF);
		*ptr = ptr_byte[0];
		*(ptr + 1) = ptr_byte[1];
	}
	*ptr = '\0';

	return (const char*) md5email;
}
