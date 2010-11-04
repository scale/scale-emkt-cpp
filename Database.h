#ifndef _DATABASE_H
#define _DATABASE_H

#include "global.h"

#ifdef  __cplusplus
extern "C" {
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//typedef int SOCKET; // get round windows definitions.

#include <mysql/mysql.h>

#ifdef  __cplusplus
}
#endif

#include "Debug.h"

/**
 * Connection pool struct.
 * Database connection handle.
*/

typedef struct Connection_Info
{
	string host;
	string user;
	string pass;
	string db;

	struct Connection_Info& operator=(struct Connection_Info& ci2){
            host = ci2.host;
            user = ci2.user;
            pass = ci2.pass;
            db = ci2.db;
	    return *this;
	}

} Connection_Info_t;

Connection_Info_t conn; //Struct com os dados de conexao;

class DBException{

public:
	string err_description;
	string err_id;

};


class Database {
public:
        Database(void);
        Database(Connection_Info_t &ci_t);
        ~Database(void);
	void setConnectionInfo(Connection_Info_t &ci_t);
	void executeQuery(const char *fmt, ...);
	MYSQL_RES* select(const char *fmt, ...);
	const char* getError();
	void connect();
	DBException getLastError();
	void freeResult();

	static string host;
	static string user;
	static string pass;
	static string db;

private:
        Connection_Info_t connectionInfo; //Dados para conexao
	int conn_status;
	MYSQL *connection, mysql;
	MYSQL_RES *result;
	Debug* debug;

};



#endif // _DATABASE_H
