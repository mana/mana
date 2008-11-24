CC=g++
CFLAGS=-c `pkg-config --cflags libxml-2.0`
LDFLAGS=`pkg-config --libs libxml-2.0`
SOURCES=base64.cpp main.cpp map.cpp xmlutils.cpp zlibutils.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=tmxcopy

all: $(SOURCES) $(EXECUTABLE)
	make clean

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o
