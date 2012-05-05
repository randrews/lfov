CC = gcc
LUA_DIR = /usr/local/include
FOV_DIR = .
HEADERS = fov.h lfov.h

default: lfov.so

.c.o: ${HEADERS}
	${CC} $? -c -o $@ -I${LUA_DIR} -I${FOV_DIR}

lfov.so: fov.o lfov.o
	${CC} *.o -o lfov.so -shared -undefined dynamic_lookup

test: lfov.so
	lua test.lua

clean:
	rm -f *~
	rm -f lfov.so
	rm -f *.o
