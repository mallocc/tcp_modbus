CC = g++
CFLAGS = -g -Wall -std=c++11 -I../include 
LDFLAGS = -pthread

SOURCES = master.cpp ../include/Log.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = master

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)