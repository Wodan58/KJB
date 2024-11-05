#
#   module  : makefile
#   version : 1.2
#   date    : 10/18/24
#
.POSIX:
.SUFFIXES:
.SUFFIXES: .c .o

CC     = gcc
CFLAGS = -DNCHECK -O3 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
HDRS   = globals.h
OBJS   = main.o start.o bible.o utils.o schidx.o

bible:	$(OBJS)
	$(CC) -o$@ $(OBJS) -lgc

$(OBJS): $(HDRS)

.c.o:
	$(CC) -o$@ $(CFLAGS) -c $<

clean:
	rm -f *.o
