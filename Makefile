CC=gcc

.PHONY=all
all: bfdb

bfdb: bfdb.c
	${CC} -o bfdb bfdb.c
