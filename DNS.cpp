/*
 * DNS.cpp
 *
 *  Created on: 21/10/2010
 *      Author: Fabiana
 */

#include "DNS.h"

DNS::DNS(const std::string& dns_server) {
	server = dns_server;
}

DNS::~DNS() {
}

char* DNS::GetDomain(const std::string& email) {

	char* domain;
	int pos = 0;
        // do some silly checks
        if( email.length() > 0 ){
		pos = email.find("@") + 1;
		if(pos > 0){
			domain = (char*) malloc (email.length() + 1 - pos);
			strcpy(domain, email.c_str()+pos);

		}
	}

	return domain;

}


bool DNS::GetMX(std::vector<sockaddr_in>& adds) {
        adds.clear(); // be safe in case of my utter stupidity

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(53); // default dns port

        SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
        hostent* host = 0;

#ifdef WIN32
        addr.sin_addr.S_un.S_addr = inet_addr(nameserver.c_str());
        if(addr.sin_addr.S_un.S_addr != INADDR_NONE) {
#else
        if(inet_aton(server.c_str(), &addr.sin_addr)) {
#endif
                host = gethostbyaddr((char*)&addr.sin_addr, sizeof(addr.sin_addr), AF_INET);
        }
        else
                host = gethostbyname(server.c_str());
        if(!host) { // couldn't get to dns, try to connect directly to 'server' instead.
                ////////////////////////////////////////////////////////////////////////////////
                // just try to deliver mail directly to "server"
                // as we didn't get an MX record.
                addr.sin_family = AF_INET;
                addr.sin_port = 25; // smtp port!! 25
#ifdef WIN32
                addr.sin_addr.S_un.S_addr = inet_addr(server.c_str());
                if(addr.sin_addr.S_un.S_addr != INADDR_NONE) {
#else
                if(inet_aton(server.c_str(), &addr.sin_addr)) {
#endif
                        host = gethostbyaddr((char*)&addr.sin_addr, sizeof(addr.sin_addr), AF_INET);
                }
                else
                        host = gethostbyname(server.c_str());

                if(!host) {
                        return false; // error!!!
                }

                memcpy((char*)&addr.sin_addr, host->h_addr, host->h_length);

                adds.push_back(addr);
		//std::cout << "sucesso " << host->h_addr << std::endl;
                return true;
        }
        else
                memcpy((char*)&addr.sin_addr, host->h_addr, host->h_length);


	bzero(&(addr.sin_zero), 8);
#ifdef WIN32
        if(connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#else
        if(connect(s, (sockaddr*)&addr, sizeof(addr))) {
#endif
                return false; // dns connection unavailable
        }

        // dnsheader info         id    flags   num queries
        unsigned char dns[512] = {1,1,   1,0,      0,1,      0,0, 0,0, 0,0};
        int dnspos = 12; // end of dns header
        std::string::size_type stringpos(0);
        std::string::size_type next(server.find("."));
        if(next != std::string::npos) { // multipart name e.g. "aserver.somewhere.net"
                while(stringpos < server.length()) {
                        std::string part(server.substr(stringpos, next-stringpos));
                        dns[dnspos] = part.length();
                        ++dnspos;
                        for(std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
                                dns[dnspos] = part[i];
                        }

                        stringpos = ++next;
                        next = server.find(".", stringpos);
                        if(next == std::string::npos) {
                                part = server.substr(stringpos, server.length() - stringpos);
                                dns[dnspos] = part.length();
                                ++dnspos;
                                for(std::string::size_type i = 0; i < part.length(); ++i, ++dnspos) {
                                        dns[dnspos] = part[i];
                                }
                                break;
                        }
                }
        }
        else { // just a single part name. e.g. "aserver"
                dns[dnspos] = server.length();
                ++dnspos;
                for(std::string::size_type i = 0; i < server.length(); ++i, ++dnspos) {
                        dns[dnspos] = server[i];
                }
        }
        // in case the server string has a "." on the end
        if(server[server.length()-1] == '.')
                dns[dnspos] = 0;
        else
                dns[dnspos++] = 0;

        // add the class & type
        dns[dnspos++] = 0;
        dns[dnspos++] = 15;  // MX record.
        dns[dnspos++] = 0;
        dns[dnspos++] = 1;

        dnspos = ::send(s, (char*)dns, dnspos, MSG_DONTROUTE);
        dnspos = recv(s, (char*)dns, 512, 0);

#ifdef WIN32
        closesocket(s);
#else
        close(s);
#endif

#ifdef WIN32
        if(dnspos != SOCKET_ERROR) {
#else
        if(dnspos > -1) {
#endif
                // now parse the data sent back from the dns for MX records
                if(dnspos > 12) { // we got more than a dns header back
                        unsigned short numsitenames = ((unsigned short)dns[4]<<8) | dns[5];
                        unsigned short numanswerRR = ((unsigned short)dns[6]<<8) | dns[7];
                        unsigned short numauthorityRR = ((unsigned short)dns[8]<<8) | dns[9];
                        unsigned short numadditionalRR = ((unsigned short)dns[10]<<8) | dns[11];

                        if(!(dns[3] & 0x0F)) { // check for an error
                                // int auth((dns[2] & 0x04)); // AA bit. the nameserver has given authoritive answer.
                                int pos = 12; // start after the header.

                                std::string questionname;
                                if(numsitenames) {
                                        parsename(pos, dns, questionname);
                                        pos += 4; // move to the next RR
                                }

                                // This gives this warning in VC.
                                // bloody annoying, there is a way round it according to microsoft.
                                // The debugger basically cannot browse anything with a name
                                // longer than 256 characters, "get with the template program MS".
                                // #pragma warning( disable : 4786 )
                                // #pragma warning( default : 4786 )
                                std::vector<std::string> names;
                                in_addr address;
                                std::string name;
                                // VC++ incompatability scoping
                                // num should be able to be declared in every for loop here
                                // not in VC
                                int num = 0;
                                for(; num < numanswerRR; ++num) {
                                        name = "";
                                        parseRR(pos, dns, name);
                                        if(name.length())
                                                names.push_back(name);
                                }
                                for(num = 0; num < numauthorityRR; ++num) {
                                        name = "";
                                        parseRR(pos, dns, name);
                                        if(name.length())
                                                names.push_back(name);
                                }
                                for(num = 0; num < numadditionalRR; ++num) {
                                        name = "";
                                        parseRR(pos, dns, name);
                                        if(name.length())
                                                names.push_back(name);
                                }

                                // now get all the MX records IP addresess
                                addr.sin_family = AF_INET;
                                addr.sin_port = 25; // smtp port!! 25
                                hostent* host = 0;
                                for(std::vector<std::string>::const_iterator it = names.begin(); it < names.end(); ++it) {
                                        host = gethostbyname(it->c_str());
                                        if(!host) {
#ifdef WIN32
                                                addr.sin_addr.S_un.S_addr = 0;
#else
                                                addr.sin_addr.s_addr = 0;
#endif
                                                continue; // just skip it!!!
                                        }
                                        memcpy((char*)&addr.sin_addr, host->h_addr, host->h_length);
                                        adds.push_back(addr);
                                }
                                // got the addresses
                                return true;
                        }
                }
        }
        return false;
}

void DNS::parsename(int& pos, const unsigned char dns[], std::string& name) {
        int len = dns[pos];
        if(len >= 192) {
                int pos1 = ++pos;
                ++pos;
                parsename(pos1, dns, name);
        }
        else {
                for(int i = 0; i < len; ++i)
                        name += dns[++pos];
                len = dns[++pos];
                if(len != 0)
                        name += ".";
                if(len >= 192) {
                        int pos1 = dns[++pos];
                        ++pos;
                        parsename(pos1, dns, name);
                }
                else if(len > 0) {
                        parsename(pos, dns, name);
                        //++pos;
                }
                else if(len == 0)
                        ++pos;
        }
}

bool DNS::parseRR(int& pos, const unsigned char dns[], std::string& name) {
        if(pos < 12) // didn,t get more than a header.
                return false;
        if(pos > 512) // oops.
                return false;

        int len = dns[pos];
        if(len >= 192) { // pointer
                int pos1 = dns[++pos];
                len = dns[pos1];
        }
        else { // not a pointer.
                parsename(pos, dns, name);
        }
        // If I do not seperate getting the short values to different
        // lines of code, the optimizer in VC++ only increments pos once!!!
        unsigned short a = ((unsigned short)dns[++pos]<<8);
        unsigned short b = dns[++pos];
        unsigned short Type = a | b;
        a = ((unsigned short)dns[++pos]<<8);
        b = dns[++pos];
        // unsigned short Class = a | b;
        pos += 4; // ttl
        a = ((unsigned short)dns[++pos]<<8);
        b = dns[++pos];
        unsigned short Datalen = a | b;
        if(Type == 15) { // MX record
                // first two bytes the precedence of the MX server
                a = ((unsigned short)dns[++pos]<<8);
                b = dns[++pos];
                // unsigned short order = a | b; // we don't use this here
                len = dns[++pos];
                if(len >= 192) {
                        int pos1 = dns[++pos];
                        parsename(pos1, dns, name);
                }
                else
                        parsename(pos, dns, name);
        }
        else if(Type == 12) { // pointer record
                pos += Datalen+1;
        }
        else if(Type == 2) { // nameserver
                pos += Datalen+1;
        }
        else if(Type == 1) { // IP address, Datalen should be 4.
                pos += Datalen+1;
        }
        else {
                pos += Datalen+1;
        }
        return true;
}
