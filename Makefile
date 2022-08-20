CC=gcc
CCFLAGS=-Wall -Wextra

.PHONY: all debug

all: bfdb

debug: bfdb.c
	${CC} ${CCFLAGS} -o bfdb bfdb.c -ggdb

bfdb: bfdb.c
	${CC} ${CCFLAGS} -o bfdb bfdb.c
