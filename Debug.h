/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H

#include <iostream>
#include <fstream>

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef LINUX
#include <syslog.h>
#endif

#ifdef  __cplusplus
}
#endif

#include "global.h"

class Debug {

public:
	Debug(int log, const char* ident);
	~Debug();
	bool Log(const char *fmt, ...);
	bool append(const char *fmt, ...);
	void Writing(int log);
	static void info(const char *fmt, ...);
	static void debug(const char *fmt, ...);
	static void error(const char *fmt, ...);
	static void warn(const char *fmt, ...);

private:
	int write;

};

Debug debug(1, "emkt");


#endif // _DEBUG_H


