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
#include <stdlib.h>
#include "unistd.h"

#include "global.h"
#include "Pointer.h"
#include "PecaHandler.h"
#include "DNS.h"
#include "Mailer.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

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
