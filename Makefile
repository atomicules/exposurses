# exposurses

include config.mk

SRC = exposurses.c
OBJ = ${SRC:.c=.o}

all: options exposurses

options:
	@echo build options:
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c $<

exposurses: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f exposurses
