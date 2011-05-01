CC = gcc
CFLAGS = -g -O0 -pedantic -Wall -W -std=gnu99
LDFLAGS = -lpthread

SOURCES = $(shell find . -name "*.c")
OBJECTS = $(SOURCES:.c=.o)

TESTER = ~comp2129/assignment3/mark-sample
BINARIES = counter

.PHONY: all clean depends test

all: $(BINARIES)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(BINARIES)

depends:
	$(CC) $(CFLAGS) -MM $(SOURCES) > Makefile.depends

test: test_counter

counter: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

test_counter: counter
	$(TESTER) question1 ./counter

-include Makefile.depends
