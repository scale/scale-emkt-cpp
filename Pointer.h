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
#include <string>
#include <iostream>

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
	CharIntMap fields;
	int total_fields;
	int total_record_set;
	int _posicao;
	bool status;

public:

	Pointer(const char *fmt, ...);
    ~Pointer();

    std::string get(const char* field);
    bool next();
    int total();
    bool first();
    bool last();
	int posicao();
	bool posicao(const int pos);
	bool back();

	void query(const char *fmt, ...);
};


#endif // _PONTEIRO_H
