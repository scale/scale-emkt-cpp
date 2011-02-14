#ifndef _DATABASE_H
#define _DATABASE_H

#include "global.h"
#include "Debug.h"

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

class DBException{

public:
	string err_description;
	string err_id;

};


class Database {
public:
	Database(void);
	~Database(void);

	void executeQuery(const char *fmt, ...);
	MYSQL_RES* select(const char *fmt, ...);
	std::string getError();
	DBException getLastError();
	void freeResult();

	static string host;
	static string user;
	static string pass;
	static string db;

private:
	void connect();

	int conn_status;
	MYSQL *connection, mysql;
	MYSQL_RES *result;

	Debug *debug;
};



#endif // _DATABASE_H
