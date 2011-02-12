/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef _POINTER_H
#define _POINTER_H

#include "Database.h"
#include "global.h"
#include <map>

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

typedef map<const char*,int, ltstr> CharIntMap;



class Pointer{

private:
	Database* database;
	MYSQL_RES* result;
	MYSQL_RES* field;
	MYSQL_FIELD* mysql_fields;
	MYSQL_ROW row;
	Connection_Info_t s_CI;
	CharIntMap fields;
	int total_fields;
	int total_record_set;
	int posicao;
	bool status;

public:

	Pointer(const Connection_Info_t &conInfo, const char *fmt, ...);
    ~Pointer();

    const char* get(const char* field);
    bool next();
    bool getNext();
    int getTotal();
    bool firstRecord();
    bool lastRecord();
	int getPosicao();
	bool setPosicao(const int* pos);
	bool backRecord();
	void query(const char *fmt, ...);


};


#endif // _PONTEIRO_H
