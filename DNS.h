<<<<<<< Updated upstream
/*
 * DNS.h
 *
 *  Created on: 21/10/2010
 *      Author: Flavio
 */

#ifndef DNS_H_
#define DNS_H_

#include "global.h"
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET; // get round windows definitions.

#include <string>
using namespace std;

class DNS {
public:
	DNS(const std::string& dns_server);
	virtual ~DNS();

	static char* GetDomain(const std::string& email);
	bool GetMX(std::vector<string>& adds);

private:
    void parsename(int& pos, const unsigned char dns[], std::string& name);
    bool parseRR(int& pos, const unsigned char dns[], std::string& name);

	std::string server;
};

#endif /* DNS_H_ */
