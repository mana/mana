CC=g++
CFLAGS=-g -ggdb -O0 -Wall -c `pkg-config --cflags libxml-2.0`
LDFLAGS=`pkg-config --libs libxml-2.0`
SOURCES_UTILS=base64.cpp map.cpp xmlutils.cpp zlibutils.cpp
OBJECTS_UTILS=$(SOURCES_UTILS:.cpp=.o)
EXECUTABLES=tmxcopy tmx_random_fill

all: $(SOURCES_UTILS) $(EXECUTABLES)
	make clean

tmxcopy: tmxcopy.o $(OBJECTS_UTILS)
	$(CC) $(LDFLAGS) tmxcopy.o $(OBJECTS_UTILS) -o $@

tmx_random_fill: tmx_random_fill.o $(OBJECTS_UTILS)
	$(CC) $(LDFLAGS) tmx_random_fill.o $(OBJECTS_UTILS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
