#ifndef _MICRODNS_HEADER
#define _MICRODNS_HEADER

#include <string>

struct mx {
	int prio;
	std::string host;
	struct in_addr addr;
};

class MicroDNS {
public:
	MicroDNS(void) {}
	~MicroDNS(void) {}

	vector<struct mx> MX(std::string& domain);
	std::string A(std::string& domain);

};

#endif
