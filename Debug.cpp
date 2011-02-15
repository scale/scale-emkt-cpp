/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "Debug.h"


Debug::Debug(const char* ident){
	char nome[500];

	snprintf(nome, 500, "emkt:%s", ident);

#ifdef __SYSLOG_H__
	openlog(nome, LOG_NDELAY | LOG_PID, LOG_LOCAL0);
#else
	printf("%s", nome);
#endif
}

Debug::~Debug(){
#ifdef __SYSLOG_H__
        closelog();
#endif
}

bool Debug::Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef __SYSLOG_H__
	vsyslog(LOG_INFO, fmt, args);
#else
	printf(fmt, args);
#endif
	va_end(args);

	return true;
}

void Debug::info(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef __SYSLOG_H__
	vsyslog(LOG_INFO, fmt, args);
#else
	printf(fmt, args);
#endif
	va_end(args);
}



void Debug::debug(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef __SYSLOG_H__
	vsyslog(LOG_DEBUG, fmt, args);
#else
	printf(fmt, args);
#endif
	va_end(args);
}

void Debug::error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef __SYSLOG_H__
	vsyslog(LOG_ERR, fmt, args);
#else
	printf(fmt, args);
#endif
	va_end(args);
}

void Debug::warn(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#ifdef __SYSLOG_H__
	vsyslog(LOG_WARNING, fmt, args);
#else
	printf(fmt, args);
#endif
	va_end(args);
}

