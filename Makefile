# Makefile

# Installation directory
PREFIX=/usr/local

# Use Xft for the menu
USE_XFT=yes

VERSION=0.10

BINDIR=$(PREFIX)/bin
MANPREFIX=$(PREFIX)/share/man
MANDIR=$(MANPREFIX)/man1
PROG=wmbluecpu
MANUAL=$(PROG).1
OBJS=main.o menu.o
CFLAGS=-O2 -Wall -DVERSION=\"$(VERSION)\"

OS=$(shell uname -s)
#For Linux:
ifeq ($(OS),Linux)
LIBS=-L/usr/X11R6/lib -lX11 -lXext -lXpm
OBJS += cpu_linux.o
endif
#For FreeBSD:
ifeq ($(OS),FreeBSD)
LIBS=-L/usr/X11R6/lib -lX11 -lXext -lXpm -lkvm
OBJS += cpu_freebsd.o
endif

ifeq ($(USE_XFT),yes)
CFLAGS += -DUSE_XFT $(shell pkg-config xft --cflags)
LIBS += $(shell pkg-config xft --libs)
endif

CC=gcc
RM=rm -rf
INST=install

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS)
	strip $(PROG)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	$(RM) *.o $(PROG) *~ *.bak *.BAK .xvpics
install: $(PROG) $(MANUAL)
	$(INST) -m 755 $(PROG) $(BINDIR)
	$(INST) -m 644 $(MANUAL) $(MANDIR)
uninstall:
	$(RM) $(BINDIR)/$(PROG)
	$(RM) $(MANDIR)/$(MANUAL)

cpu_freebsd.o: cpu_freebsd.c
cpu_linux.o: cpu_linux.c
main.o: main.c cpu.h mwm.h menu.h pixmap.xpm icon.xpm common.c
menu.o: menu.c menu.h
