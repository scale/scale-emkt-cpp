/***************************************************************************
 *   Copyright (C) 2003 by eddiedu                                         *
 *   eddiedu@scale.com.br                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "Database.h"
#include <iostream>
#include <string.h>

/** Constructor which does nothing.
 */
Database::Database() {
	connection = NULL;
	conn_status = false;

	int dados_conexao = 0;
	debug = new Debug("Database");

	if (host.length() < 1 && pass.length() < 1 && db.length() < 1
			&& user.length() < 1) {
		std::ifstream hfile(CONF_DIR);

		int dados_conexao = 0;
		std::string temp;
		while (!hfile.eof()) {
			getline(hfile, temp);

			if (temp.find("database") == 0) {
				db = temp.substr(temp.find("=") + 1, temp.length());
				dados_conexao++;
			}

			if (temp.find("host") == 0) {
				host = temp.substr(temp.find("=") + 1, temp.length());
				dados_conexao++;
			}

			if (temp.find("user") == 0) {
				user = temp.substr(temp.find("=") + 1, temp.length());
				dados_conexao++;
			}

			if (temp.find("pass") == 0) {
				pass = temp.substr(temp.find("=") + 1, temp.length());
				dados_conexao++;
			}
		}
	}

	result = NULL;

	if (dados_conexao > 0) {
		try {
			connect();
		} catch (DBException dbe) {
			throw dbe;
		}

	} else {
		DBException dbe;
		dbe.err_description = "Dados insuficientes para conectar!";
		dbe.err_id = "CONEXAO";
		throw dbe;// Erro ao executar o comando
	}
}

/**
 * Destrutora fecha a conexao e o resultset
 */
Database::~Database() {

	if (result != NULL && result > 0) {
		mysql_free_result(result);
		result = NULL;
	}
	if (connection != NULL) {
		mysql_close(connection);
		connection = NULL;
	}

	if (debug != NULL)
		delete debug;
}

//Limpa o ponteiro com o banco da var membra result
void Database::freeResult() {
	if (result != NULL && result > 0) {
		mysql_free_result(result);
		result = NULL;
	}

}

/*
 * rotina para conectar-se a uma Database
 * @return bool retorna se houve sucesso ou nao;
 * @throw DBExcption
 */
void Database::connect() {

	if (strlen(host.c_str()) < 1) {
		DBException dbe;
		dbe.err_description = "O host nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;
	}

	if (strlen(user.c_str()) < 1) {
		DBException dbe;
		dbe.err_description = "O user nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	if (strlen(pass.c_str()) < 1) {
		DBException dbe;
		dbe.err_description = "O pass nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	if (strlen(db.c_str()) < 1) {
		DBException dbe;
		dbe.err_description = "O db nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	mysql_init(&mysql);

	mysql_real_connect(&mysql, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), 0, NULL, 0);
	connection = &mysql;

	if (connection == NULL) {
		DBException dbe;
		dbe.err_description
				= "O Mysql nao aceitou, ou nao esta aceitando a conexao!";
		dbe.err_id = "CONEXAO";

		throw dbe;// Erro ao executar o comando
	}

	if (mysql_select_db(&mysql, db.c_str())) {
		DBException dbe;
		dbe.err_description = mysql_error(connection);
		dbe.err_id = "DB";

		throw dbe;// Erro ao executar o comando
	}

	conn_status = true;

}

/* retorna se ha algum erro caso contrario retorna "" */
std::string Database::getError() {
	return mysql_error(connection);

}

void Database::executeQuery(const char *fmt, ...) {
	char *sql;
	int count;
	va_list args;

	if (!fmt) return;

	va_start(args, fmt);
	count = vsnprintf(NULL, 0, fmt, args);
	sql = (char *) malloc(count + 1);

	va_start(args, fmt);
	vsnprintf(sql, count + 1, fmt, args);
	va_end(args);

	if (conn_status == false) { // Caso nao esteja conectado tenta conectar
		try {
			connect();
		} catch (DBException dbe) {
			throw dbe;
		}
	}

	debug->debug("%s", sql);

	if (connection != NULL && mysql_query(connection, sql) != 0) {

		DBException dbe;
		dbe.err_description = mysql_error(connection);
		dbe.err_id = "QUERY";
		free(sql);
		throw dbe;// Erro ao executar o comando

	}

	free(sql);
}

MYSQL_RES* Database::select(const char *fmt, ...) {

	char sql[2048];
	va_list args;

	if (!fmt) return NULL;

	va_start(args, fmt);
	vsnprintf(sql, 2047, fmt, args);
	va_end(args);

	if (conn_status == false) { // Caso nao esteja conectado tenta conectar
		try {
			connect();
		} catch (DBException dbe) {
			throw dbe;
		}
	}

	if (connection != NULL && mysql_query(connection, sql) != 0) {
		DBException dbe;
		dbe.err_description = mysql_error(connection);
		dbe.err_id = "QUERY";

		throw dbe;// Erro ao executar o comando
	}

	result = mysql_store_result(connection);
	return result;

}

DBException Database::getLastError() {

	DBException dbe;
	dbe.err_description = mysql_error(connection);
	dbe.err_id = "POINTER";

	return dbe;// Erro ao executar o comando


}

