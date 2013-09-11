# exposurses

include config.mk

SRC = exposurses.c
OBJ = ${SRC:.c=.o}

all: options exposurses

options:
	@echo build options:
	@echo "CC       = ${CC}"

exposurses: ${OBJ}
	@echo CC -o $@
	@${CC} -s ${LIBS} ${SRC} -o $@ 

debug: 
	@echo "Building with debug symbols"	
	@${CC} -g ${LIBS} ${SRC} -o exposurses

clean:
	@echo cleaning
	@rm -f exposurses ${OBJ}
