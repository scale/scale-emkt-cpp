#$Id: Makefile,v 1.18 1998/05/12 21:09:14 srb Exp $

BASENAME= ${ROOT_DIR}/opt

O=o
CC=g++
INCLUDES= -I. -I/usr/include -I/usr/include/mysql -I/usr/include/openssl

BINDIR=$(BASENAME)/emkt
INSTALL=install -o root -m
BINPERM=02755 -s -g root

CFLAGS	= -g -W -Wall -O2 ${INCLUDES} -D'CONF_DIR="${BINDIR}/database.conf"'
CXXFLAGS = -g -W -Wall -O2 ${INCLUDES}  -D'CONF_DIR="${BINDIR}/database.conf"'
LDFLAGS = -lmysqlclient -lm -lz -lpthread -L. -L/usr/lib/mysql -lssl -lcrypto

OBJS=Base64.${O} Database.$(O) Date.$(O) Debug.$(O) Encoder.$(O) \
     Mailer.$(O) Mutex.$(O) PecaHandler.$(O) Pointer.$(O) QueueManager.$(O) DNS.$(O) \
     QuotedPrintable.$(O) Sender.$(O) Thread.$(O) binaryblock.$(O) \
     socket_err.$(O) 

SRCS=$(OBJS,.o=.cpp)

BINS=emkt.exe test.exe



all: $(BINS)


emkt.exe: $(OBJS) emkt.$(O)
	$(CC) -o $@ $(CFLAGS) $(OBJS) emkt.$(O) $(LDFLAGS) $(LIBS)

test.exe: $(OBJS) test.$(O)
	$(CC) -o $@ $(CFLAGS) $(OBJS) test.$(O) $(LDFLAGS) $(LIBS)

.o.c:
	@echo "  CC $@" ; ${CC} $(CFLAGS)  -c $<

${OBJS}: ${SRCS}



install: $(BINS)
	strip $(BINS)
	$(INSTALL) $(BINPERM) $(BINS) $(BINDIR)

deinstall:
	cd $(BINDIR) && $(RM) $(BINS)

clean:
	rm -f $(OBJS) $(BINS) *core*
