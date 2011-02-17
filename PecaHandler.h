/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _PECAHANDLER_HEADER
#define _PECAHANDLER_HEADER

#include "Mutex.h"
#include "Thread.h"
#include "Database.h"
#include "Peca.h"
#include "Pointer.h"

#include <map>

class PecaHandler : public Thread {

public:
	PecaHandler();
	~PecaHandler();

	void finalizar_peca(const Peca &p);
private:
	virtual void* Run(void*);

	void lePecasAtivas();
	void resetEnvio(Peca &peca);

	std::vector<Peca> pecas;
	Database database;
};


#endif
