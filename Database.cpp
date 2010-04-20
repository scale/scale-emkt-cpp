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



std::string Database::host = "";
std::string Database::user = "";
std::string Database::pass = "";
std::string Database::db   = "";


/** Constructor which does nothing.
*/
Database::Database(void) {
	connection = NULL;
	conn_status = false;

	int dados_conexao = 0;

	if(host.length() < 1 &&
		pass.length() < 1 &&
		db.length() < 1 &&
		user.length() < 1 ){
	        std::ifstream hfile(CONF_DIR);
	
		int dados_conexao = 0;
		std::string temp;
        	while ( !hfile.eof() ) {
	    	getline(hfile, temp);
	
	    	if(temp.find("database") == 0){
                	connectionInfo.db = temp.substr( temp.find("=")+1, temp.length() );
	                dados_conexao++;
            	}
	
	    	if(temp.find("host") == 0){
	        	connectionInfo.host = temp.substr( temp.find("=")+1, temp.length() );
			dados_conexao++;
            	}
	
	    	if(temp.find("user") == 0){
	        	connectionInfo.user = temp.substr( temp.find("=")+1, temp.length() );
			dados_conexao++;
	    	}
	
	    	if(temp.find("pass") == 0){
	        	connectionInfo.pass = temp.substr( temp.find("=")+1, temp.length() );
			dados_conexao++;
		    }
        	}
	} else {
		//cout << "Aproveitou os dadods de conexão!" << endl;
		connectionInfo.db   = db;
		connectionInfo.user = user;
		connectionInfo.pass = pass;
		connectionInfo.host = host;
		dados_conexao = 4;
	}


	result = NULL;

	if(dados_conexao > 0 ){	
	
		try{
		
			connect();


		} catch (DBException dbe){
			throw dbe;
		}

	} else { 
		DBException dbe;
        	dbe.err_description = "Dados insuficiente para conectar!";
		dbe.err_id = "CONEXAO";

        	throw dbe;// Erro ao executar o comando

	}

}

/**
 * The real constructor for this class.
 * @param ci_t Dados para conexao
*/
Database::Database(Connection_Info_t &ci_t) {
	conn_status = false;
	/*connectionInfo.host=ci_t.host;
	connectionInfo.user=ci_t.user;
	connectionInfo.pass=ci_t.pass;
	connectionInfo.db=ci_t.db;
	*/
	connectionInfo = ci_t;
	
	result = NULL;
	
	try{
		
		connect();

	} catch (DBException dbe){
		throw dbe;
	}
}

/**
 * Destrutora fecha a conexao e o resultset
*/
Database::~Database(){

	if( result != NULL && result > 0 ){
		mysql_free_result(result);
		result = NULL;
	}
	if( connection != NULL){
		mysql_close(connection);
		connection = NULL;
	}

}

//Limpa o ponteiro com o banco da var membra result
void Database::freeResult(){
	if( result != NULL && result > 0 ){
		mysql_free_result(result);
		result = NULL;
	}

}


/**
 * Definindo os parametros para conexao
 * @param ci_t Dados para conexao
*/
void Database::setConnectionInfo(Connection_Info_t &ci_t){

	connectionInfo = ci_t;

}



/*
 * rotina para conectar-se a uma Database
 * @return bool retorna se houve sucesso ou nao;
 * @throw DBExcption
 */
void Database::connect(){

	if(strlen((connectionInfo.host).c_str()) < 1 ){
		DBException dbe;
		dbe.err_description = "O host nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	if(strlen((connectionInfo.user).c_str()) < 1 ){
		DBException dbe;
		dbe.err_description = "O user nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	if(strlen((connectionInfo.pass).c_str()) < 1 ){
		DBException dbe;
		dbe.err_description = "O pass nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}

	if(strlen((connectionInfo.db).c_str()) < 1 ){
		DBException dbe;
		dbe.err_description = "O db nao foi indicado corretamente";
		dbe.err_id = "DADOS";
		throw dbe;

	}



	mysql_init(&mysql);

	mysql_real_connect(&mysql,connectionInfo.host.c_str(),connectionInfo.user.c_str(),connectionInfo.pass.c_str(),connectionInfo.db.c_str(),0,NULL,0);
	connection = &mysql;
	
/*	if(mysql_real_connect(&mysql,connectionInfo.host.c_str(),connectionInfo.user.c_str(),connectionInfo.pass.c_str(),connectionInfo.db.c_str(),0,NULL,0)){
		DBException dbe;
        	//dbe.err_description = mysql_error(connection);
        	dbe.err_description = (connectionInfo.host+" | "+connectionInfo.user+" | "+connectionInfo.pass+" | "+connectionInfo.db).c_str();
                dbe.err_id = "DB";

                throw dbe;// Erro ao executar o comando
	}
*/		
	//connection = mysql_connect(&mysql,connectionInfo.host.c_str(),connectionInfo.user.c_str(),connectionInfo.pass.c_str());

	if( connection == NULL ) {
		DBException dbe;
        	dbe.err_description = "O Mysql nao aceitou, ou nao esta aceitando a conexao!";
		dbe.err_id = "CONEXAO";

        	throw dbe;// Erro ao executar o comando
    	}

        if (mysql_select_db(&mysql,connectionInfo.db.c_str())){
		DBException dbe;
        	dbe.err_description = mysql_error(connection);
		dbe.err_id = "DB";

        	throw dbe;// Erro ao executar o comando
	}


	conn_status = true;

}


/* retorna se ha algum erro caso contrario retorna "" */
const char* Database::getError(){
	return mysql_error(connection);


}


void Database::executeQuery(const char *fmt, ...){
    	char *sql;
	int count;
	va_list args;

	if(!fmt){
		DBException dbe;
		dbe.err_description = "nao foi passado o comando sql a ser executado";
		throw dbe;

	}
        va_start(args, fmt);
	
	count = vsnprintf(NULL, 0, fmt, args);

  	sql = (char *)malloc(count+1);

	va_start(args,fmt);

	vsnprintf(sql,count+1,fmt,args);

  	va_end(args);


	
	if( conn_status == false){ // Caso nao esteja conectado tenta conectar
		try{
			connect();

		} catch (DBException dbe){

			throw dbe;
		}
	}

	if( connection != NULL && mysql_query(connection,sql) != 0) {
	
		DBException dbe;
        	dbe.err_description = mysql_error(connection);
		dbe.err_id = "QUERY";
		free(sql);
        	throw dbe;// Erro ao executar o comando

    	} 

	free(sql);
	
}


MYSQL_RES* Database::select(const char *fmt, ...){

    	char sql[2048];
	va_list args;

	if(!fmt)
		return NULL;

        va_start(args, fmt);

        vsnprintf(sql, 2047, fmt, args);
	
       va_end(args);
	
	if(conn_status == false){ // Caso nao esteja conectado tenta conectar
		try{
			connect();

		} catch (DBException dbe){
			throw dbe;
		}
	}

	if( connection != NULL && mysql_query(connection,sql) != 0 ) {
		DBException dbe;
        	dbe.err_description = mysql_error(connection);
		dbe.err_id = "QUERY";

        	throw dbe;// Erro ao executar o comando
    	}

    	result = mysql_store_result(connection);
	return result;

}

DBException Database::getLastError(){

		DBException dbe;
        	dbe.err_description = mysql_error(connection);
		dbe.err_id = "POINTER";

        	return dbe;// Erro ao executar o comando


}


