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

	// helper function.
	// returns the part of the string toaddress after the @ symbol.
	// i.e. the 'toaddress' is an email address eg. someone@somewhere.com
	// this function returns 'somewhere.com'
	std::string getserveraddress(const std::string& toaddress) const;

	// Does the work of getting MX records for the server returned by 'getserveraddress'
	// will use the dns server passed to this's constructor in 'nameserver'
	// or if MXlookup is false in the constuctor, will return an address
	// for the server that 'getserveraddress' returns.
	// returns false on failure, true on success
	bool gethostaddresses(std::vector<sockaddr_in>& adds);

	// split an address into its relevant parts i.e.
	// name and actual address and return it in Address.
	// this may be usefull out of the class maybe
	// it should be a static function or a global? thinking about it.
	Address parseaddress(const std::string& addresstoparse);

	std::string server;
};

#endif /* DNS_H_ */
