CC = g++
CFLAGS = -g -Wall -std=c++11 -I../include
LDFLAGS = -pthread

SOURCES = slave.cpp ../include/Log.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = slave

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)