CC = g++
CFLAGS = -MMD -MP -std=c++20 -c -g3 -O2 -Wall `pkg-config --cflags gtk4`
OBJS = main.o application.o
DEPS = Makefile application.h
LIBS = -lepoxy `pkg-config --libs gtk4`
PROJNAME = Foldscape

all: $(PROJNAME)

$(PROJNAME): $(OBJS) $(DEPS)
	$(CC) $(OBJS) $(LIBS) -o $(PROJNAME)

%.o: %.cpp
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o *.d $(PROJNAME)