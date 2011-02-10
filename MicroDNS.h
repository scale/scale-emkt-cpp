
#ifndef _MICRODNS_HEADER
#define _MICRODNS_HEADER

#include <string>

#define TRUE 1

struct mx
{
  int prio;
  char host[1024];
  struct in_addr addr;
};

#ifndef HFIXEDSZ
# define HFIXEDSZ 12
#endif
#ifndef INT16SZ
# define INT16SZ sizeof(cit_int16_t)
#endif
#ifndef INT32SZ
# define INT32SZ sizeof(cit_int32_t)
#endif


class MicroDNS
{
public:
	MicroDNS(void) { };
	~MicroDNS(void) {};
	struct mx& mx( std::string domain );

private:
	int mxcomp(int p[],int n);
	void sort_mxrecs (struct mx *mxrecs, int nmx);
	int getmxip(const char *domain,struct in_addr *ip);
	int pipe_file_to_server(int s,char *filename,char *sender, char *sname,char *recipient,char *subject);
	int smtp_auth(int s,struct in_addr *ip, char *userid, char *password);
	int send_mail(struct in_addr *ip,char *sender, char *sname, char *recipient,char *subject, char *userid,char *password,char *file);

	struct mx _mx;
};


#endif
