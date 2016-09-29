PROG=ipv4-heatmap

all: $(PROG)

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
uname_O := $(shell sh -c 'uname -o 2>/dev/null || echo not')
uname_R := $(shell sh -c 'uname -r 2>/dev/null || echo not')
uname_P := $(shell sh -c 'uname -p 2>/dev/null || echo not')
uname_V := $(shell sh -c 'uname -v 2>/dev/null || echo not')

ifndef CFLAGS
  CFLAGS = -Wall -O3 -I/usr/local/include
endif

ifndef LDFLAGS
  LDFLAGS = -lgd -lm -L/usr/local/lib
endif

ifndef INSTALLDIR
  INSTALLDIR = /usr/local/bin
endif

ifndef MANDIR
  MANDIR = /usr/local/man
endif

ifdef CC
  LD = $(CC)
else 
  CC = gcc
  LD = gcc
  ifeq ($(uname_S),Darwin)
    CC = clang
    LD = clang
  endif
endif

OBJS=\
	ipv4-heatmap.o \
	xy_from_ip.o \
	hilbert.o \
	morton.o \
	annotate.o \
	shade.o \
	legend.o \
	bbox.o \
	text.o \
	cidr.o

ipv4-heatmap: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS}

clean:
	rm -f ${OBJS}
	rm -f ipv4-heatmap
	rm -rf bin

install: ipv4-heatmap
	install -C -m 755 ipv4-heatmap $(INSTALLDIR)
	install -C -m 755 man/ipv4-heatmap.1 $(MANDIR)/man1

test: ${OBJS}
	@mkdir -p bin
	@${CC} ${LDFLAGS} -o bin/ipv4-heatmap ${OBJS}
	@rm -rf bin