/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "Pointer.h"


/* Construtor da Classe ponteiro, responsavel por percorrer os dados do result.
 * @param const char* O comando sql que devera ser percorrido
 * @throw DBException
 */
Pointer::Pointer(const char *fmt, ...)
{
	//pegando a query
   	char sql[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(sql, 2047, fmt, args);
	va_end(args);
	
	query(sql);
}

/* Destrutora */
Pointer::~Pointer()
{
	if( database != NULL){
		delete database;
		database = NULL;
	}

}


void
Pointer::query(const char *fmt, ...)
{
	char sql[2048];
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(sql, 2047, fmt, args);
	va_end(args);
	
	Debug debug("Pointer");
	
	try{
	
		if(strlen(sql) < 5){
			DBException dbe;
			dbe.err_description = "Query invalida!";
			dbe.err_id = "500";
			throw dbe;
		}
	
		debug.debug(sql);
		
		database = new Database();
		result = database->select(sql);

		total_record_set = mysql_num_rows(result);
		if( total_record_set > 0 ){
			status = true;
			total_fields = mysql_num_fields(result);
			if ((row = mysql_fetch_row (result)) != NULL)
			{
				mysql_field_seek (result, 0);
				for (int i = 0; i < total_fields; i++)
				{
					mysql_fields = mysql_fetch_field (result);
					fields[mysql_fields->name] = i;
   				}
  			}
		}
		_posicao = 0;
		mysql_data_seek(result, 0);

	} catch(DBException dbe) {
		throw dbe;
	}
}


/* Busca o campo pelo nome
 * @param const char* com o nome do campo
 */
std::string
Pointer::get(const char* field)
{
	if(row != NULL){
		return (row[fields[field]] ? row[fields[field]] : "");
	}

	return "";
}


/* correndo o cursos para a proxima posicao
 * @throw DBException
 */
bool
Pointer::next()
{

	if( ( row = mysql_fetch_row(result)) != NULL ){
		_posicao++;
		return true;
	} else
		return false;

}


int
Pointer::total()
{
	return total_record_set;
}


bool
Pointer::first()
{
	_posicao = 0;
	mysql_data_seek(result, 0);
	if( (row = mysql_fetch_row (result)) != NULL )
		return false;

	return true;
}


bool
Pointer::last()
{
	_posicao = total_record_set - 1;
	mysql_data_seek(result, (total_record_set-1));
	if( (row = mysql_fetch_row (result)) != NULL )
		return false;

	return true;
}

int
Pointer::posicao()
{
	return _posicao;
}


bool Pointer::back()
{
	_posicao--;
	mysql_data_seek(result, _posicao);

	return true;
}
