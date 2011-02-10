/* Copyright (c) 2007 Girish Venkatachalam
*
* Permission to use, copy, modify, and distribute
* this software for any
* purpose with or without fee is hereby granted,
* provided that the above copyright notice
* and this permission notice appear
* in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <getopt.h>

#include "MicroDNS.h" 

extern int h_errno;

int 
MicroDNS::mxcomp(int p[],int n)
{
if (p[1] > p[2]) return(1);
else if (p[1] < p[2]) return(0);
else return(random() % n);
}



void 
MicroDNS::sort_mxrecs (struct mx *mxrecs, int nmx)
{

if (nmx < 2) return;

/*
for (a = nmx - 2; a >= 0; --a)
{
for (b = 0; b <= a; ++b)
{
if (mxcomp(mxrecs[b].prio,mxrecs[b+1].prio))
{
memcpy(&t1, &mxrecs[b], sizeof(struct mx));
memcpy(&t2, &mxrecs[b+1], sizeof(struct mx));
memcpy(&mxrecs[b], &t2, sizeof(struct mx));
memcpy(&mxrecs[b+1], &t1, sizeof(struct mx));
}
}
}
*/
}



int 
MicroDNS::getmxip(const char *domain,struct in_addr *ip)
{
union
{
u_char bytes[1024];
HEADER header;
} ans;

int ret;
unsigned char *startptr, *endptr, *ptr;
char expanded_buf[1024];
unsigned short prio, type;
int n = 0;
int qdcount;

struct mx *mxrecs = NULL;
struct hostent *h;
int nmx = 0;

h = (struct hostent *)malloc(sizeof(struct hostent));
ret = res_query (domain, C_IN, T_MX, (unsigned char *)ans.bytes, sizeof(ans));
if (ret < 0)
{
mxrecs = (struct mx *)malloc(sizeof(struct mx));
mxrecs[0].prio = 0;
strcpy(mxrecs[0].host, domain);
h = gethostbyname(domain);

if (!h) {
switch (h_errno) {
case HOST_NOT_FOUND:
case NO_DATA:
case NO_RECOVERY:
	perror("resolve");
	return -1;
}
nmx = 0;
} else {
memcpy(ip,h->h_addr,h->h_length);
nmx = 1;
}
}
else
{
if (ret > sizeof(ans)) ret = sizeof(ans);

startptr = &ans.bytes[0];
endptr = &ans.bytes[ret];
ptr = startptr + HFIXEDSZ; /* skip header */

for (qdcount = ntohs(ans.header.qdcount); qdcount--; ptr += ret + QFIXEDSZ)
{
if ((ret = dn_skipname(ptr, endptr)) < 0) return(0);
}

while(TRUE)
{
memset (expanded_buf, 0, sizeof(expanded_buf));
ret = dn_expand (startptr, endptr, ptr, expanded_buf,
sizeof(expanded_buf));
if (ret < 0) break;
ptr += ret;

GETSHORT (type, ptr);
ptr += INT16SZ + INT32SZ;
GETSHORT (n, ptr);

if (type != T_MX) ptr += n;
else
{
GETSHORT(prio, ptr);
ret = dn_expand(startptr, endptr, ptr, expanded_buf,
sizeof(expanded_buf));
ptr += ret;

++nmx;
if (mxrecs == NULL)
mxrecs = (struct mx *)malloc(sizeof(struct mx));
else
mxrecs = (struct mx *)realloc (mxrecs, (sizeof(struct mx) * nmx));

mxrecs[nmx - 1].prio = prio;
strcpy(mxrecs[nmx - 1].host, expanded_buf);
}
}
}
sort_mxrecs(mxrecs, nmx);

h = gethostbyname(mxrecs[0].host);

if (h) {
memcpy(ip,h->h_addr,h->h_length);
}

{char *ipadd;

ipadd = inet_ntoa(*ip);
}

free(mxrecs);
return(nmx);
}

int 
MicroDNS::pipe_file_to_server(int s,char *filename,char *sender, char *sname,char *recipient,char *subject) 
{
int fd,b,bs;

char buf[8192],envelope[8192];

#define MSG "Please terminate mail with "

/*
* SMTP envelope format from rfc 2821
* : Received: from bar.com by foo.com ; Thu, 21 May 1998
* : 05:33:29 -0700
* : Date: Thu, 21 May 1998 05:33:22 -0700
* : From: John Q. Public

* : Subject: The Next Meeting of the
* board
* : To: Jones@xyz.com
* :
* : Bill:
*
*/
envelope[0] = 0; /* Initialize the buffer */
snprintf(buf,sizeof(buf),"From: %s <%s>rn",sname,sender);
strncat(envelope,buf,strlen(buf));
snprintf(buf,sizeof(buf),"To: <%s>rn",recipient);
strncat(envelope,buf,strlen(buf));
snprintf(buf,sizeof(buf),"Subject: %srn",subject);
strncat(envelope,buf,strlen(buf));
bs = send(s,envelope,strlen(envelope),0);

if( bs <= 0) {
perror("send()");
}

/* Blank line between envelope & body */
snprintf(buf,sizeof(buf),"rn");
bs = send(s,buf,strlen(buf),0);

if( bs <= 0) {
perror("send()");
}

if(NULL == filename) {
write(1,MSG,strlen(MSG));
fd = 0;
}
else {
fd = open(filename,O_RDONLY);
if (-1 == fd ) {
printf("Couldn't open mail text file [%s]",filename);
}
}

while((b = read(fd,buf,sizeof(buf))) > 0) {

bs = send(s,buf,b,0);
if( bs < 0) {
perror("send()");
}
}

if(fd != 0)
close(fd);

}

int send_line(int s, char *buf,int n) {

int bs,r;
char rbuf[512];
int status;

bs = send(s,buf,n,0);

if (-1 == bs) {
perror("send()");

}

r = recv(s,rbuf,sizeof(rbuf),0);

if(-1 == r) {
perror("recv()");
}
rbuf[r] = 0;


status = strtoll(rbuf,NULL,10);

if(421 == status) {
printf("We are being greylisted");
}

if(status > 400) {
printf("Trouble for us, let us get out of the game..Exiting");
}
return 0;


}

int 
MicroDNS::smtp_auth(int s,struct in_addr *ip, char *userid, char *password) {

/* Fill in later*/
return 0;
}

int 
MicroDNS::send_mail(struct in_addr *ip,char *sender, char *sname, char *recipient,char *subject, char *userid,char *password,char *file) 
{

char sbuf[512],rcvbuf[512];
int s,rl,ret;
struct sockaddr_in mailhost;
char *localdomain = "susmita.org";
socklen_t l;
int status;

mailhost.sin_addr = *ip;
/* This could be 587 also... */
mailhost.sin_port = htons(25);
mailhost.sin_family = AF_INET;

l = sizeof(struct sockaddr_in);

s = socket(PF_INET,SOCK_STREAM,0);

if( s < 0) {
perror("socket");
}

ret = connect(s, (struct sockaddr *)&mailhost,l);

if(-1 == ret) {
perror("connect");
}


rl = recv(s,rcvbuf,sizeof(rcvbuf),0);

if(-1 == rl) {
perror("recv()");
}
puts(rcvbuf);
status = strtoll(rcvbuf,NULL,10);

if(421 == status) {
printf("We are being greylisted");
}

if(status > 400) {
printf("Trouble for us, let us get out of the game..Exiting");
}

smtp_auth(s,ip,userid,password);

snprintf(sbuf,sizeof(sbuf),"HELO %srn",localdomain);
send_line(s,sbuf,strlen(sbuf));
snprintf(sbuf,sizeof(sbuf),"MAIL FROM:<%s>rn",sender);
send_line(s,sbuf,strlen(sbuf));
snprintf(sbuf,sizeof(sbuf),"RCPT TO:<%s>rn",recipient);
send_line(s,sbuf,strlen(sbuf));
snprintf(sbuf,sizeof(sbuf),"DATArn",recipient);
send_line(s,sbuf,strlen(sbuf));

pipe_file_to_server(s,file,sender,sname,recipient,subject);
snprintf(sbuf,sizeof(sbuf),"rn.rn",subject);
send_line(s,sbuf,strlen(sbuf));

return 0;


}

struct mx &
MicroDNS::mx( std::string domain )
{

char buf[1024],prompt[512];
int op,status;
struct in_addr ip;

_mx.prio = 0;

if (getmxip(domain.c_str(), &ip) < 0) {
	struct hostent *h = gethostbyname(domain.c_str());

	if (h) {
		memcpy(&_mx.addr, h->h_addr,h->h_length);
	} else {
		bzero(&_mx, sizeof(struct mx) );
	}
} else {

strncpy(_mx.host, inet_ntoa(ip), 1024);
_mx.addr = ip;
}
return _mx;

}


