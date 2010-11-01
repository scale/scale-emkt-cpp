/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <mysql/mysql.h>
#include "string.h"
using namespace std;

#include "Pointer.h"
#include "ErrorPop.h"
#include "global.h"
#include "PecaHandler.h"
#include "QueueManager.h"
#include "Mailer.h"
#include "unistd.h"
#include <fstream>


int main() {
	debug.info("Iniciando o PROGRAMA DE ENVIO!");

	std::vector<std:string> ddv;
	DNS::server = DNS;

	_dns.GetMX(ddv, "hotmail.com");

	for_each( ddv.begin(), ddv.end(), print_string);

	return 0;
}

void print_string(const std::string& name) {
	cout << name << endl;
}
