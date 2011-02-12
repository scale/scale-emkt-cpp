/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _SENDER_HEADER
#define _SENDER_HEADER

#include "Mailer.h"
#include "Mutex.h"
#include "Thread.h"
#include "Debug.h"
#include "global.h"
#include "QueueManager.h"
#include "Peca.h"

#include <string.h>
#include <queue>

class Sender: public Thread {

public:
	Sender(const std::string& server, const Peca& peca) : mailer(server) {
		this->peca = peca;
		this->server = server;
		setRunning(false);
	}

	~Sender() {
		Stop();
	}

	ResultMessage getErrorMessages() {
		return em;
	}

	void add_recipient(const Address& rcpt);
	void substitute(const std::string& name, const std::string value) {};


private:
	virtual void* Run(void*);

	int id;
	Peca peca;
	std::queue<Address> recipients;
	std::string server;
	Mutex mutex;
	Mailer mailer;
	ResultMessage em;

	std::map<std::string, std::string> macros;


	bool active;
};

#endif

