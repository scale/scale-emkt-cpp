/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef LINUX
#include <asm/errno.h>
#else
#include <errno.h>
#endif


#ifdef  __cplusplus
}
#endif

#ifndef CONF_DIR
#define CONF_DIR "/usr/local/emkt/database.conf"
#endif

#include <string>
#include <vector>

using namespace std;

#define MAX_THREADS_PECA 1
#define MAX_THREADS_ENVIO 200
#define TOTAL_EMAIL 1

typedef vector<string> vString;

#ifndef ADDRESS_H_
#define ADDRESS_H_

class Address {
public:
	std::string id;
	std::string name; // e.g.   freddy foobar
	std::string email; // e.g.   someone@mail.com

	Address & operator =( const Address &other) {
		id = other.id;
		name = other.name;
		email = other.email;
		return *this;
	}
};

#endif // _ADDRESS_H

class ResultMessage {
public:
	Address recipient;
	std::string message;
	int error;
};

std::string DNS = "";
int INSTANCE_NUM = 1;

const char * get_socket_error(int err_no);

#endif

