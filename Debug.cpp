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


Debug::Debug(int log, const char* ident){
	write = log;
#ifdef LINUX	
	openlog(ident, LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_LOCAL0);
#endif
}

Debug::~Debug(){
#ifdef LINUX	
        closelog();
#endif
}

bool Debug::Log(const char *fmt, ...)
{
    	char log_text[2048];
	va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

#ifdef LINUX	
        syslog(LOG_LOCAL0|LOG_INFO, log_text);
#endif

	return true;
}

bool Debug::append(const char *fmt, ...) 
{

	char log_text[4096];
        va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

    	ofstream myFile("emkt.log",ios::app);
        // Creates an ofstream object named myFile

    	if (! myFile) // Always test file open
    	{
        	cerr << "Error opening output file" << endl;
        	return false;
    	}

    	myFile << log_text << endl;

    	myFile.close();

	return true;
}

void Debug::Writing(int log)
{
	write = log;
}


void Debug::info(const char *fmt, ...)
{
    	char log_text[2048];
	va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

#ifdef LINUX	
        syslog(LOG_LOCAL0|LOG_INFO, log_text);
#endif

}



void Debug::debug(const char *fmt, ...){
	char log_text[2048];
	va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

#ifdef LINUX	
        syslog(LOG_DEBUG|LOG_LOCAL0, log_text);
#endif

}

void Debug::error(const char *fmt, ...){
	char log_text[2048];
	va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

#ifdef LINUX	
        syslog(LOG_ERR|LOG_LOCAL0, log_text);
#endif

}

void Debug::warn(const char *fmt, ...){
	char log_text[2048];
	va_list args;
        va_start(args, fmt);
        vsnprintf(log_text, 2047, fmt, args);
        va_end(args);

#ifdef LINUX	
        syslog(LOG_WARNING|LOG_LOCAL0, log_text);
#endif

}

