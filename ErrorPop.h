/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _ERRORPOP
#define _ERRORPOP

#include "Thread.h"
#include "Database.h"

typedef struct errorEmail {
	    std::string email_error; // email que retornou
	    std::string id_error;    // id do erro que voltou
	    std::string id_errPop;   // id da mensagem na tabela pop para ser excluida no futuro
	} errorEmail_t ;

class ErrorPop : public Thread {
	
public:
	ErrorPop(Connection_Info_t* s_ConInfo);
	~ErrorPop();

private:
	virtual void* Run(void*);
	Connection_Info_t s_CI;
	std::string lowerCase(const char *s);
};

#endif
