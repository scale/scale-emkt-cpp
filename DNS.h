/*
 * DNS.h
 *
 *  Created on: 21/10/2010
 *      Author: Fabiana
 */

#ifndef DNS_H_
#define DNS_H_

class DNS {
public:
	DNS(const std::string& dns_server);
	virtual ~DNS();

	static char* GetDomain(const std::string& email);
	bool GetMX(std::vector<sockaddr_in>& adds);

private:
	std::string server;
};

#endif /* DNS_H_ */
