# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
#INCS = -I. -I/usr/include
LIBS=-lmenu -lcurses

# flags
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
