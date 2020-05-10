CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cmz: $(OBJS)
	$(CC) -o cmz $(OBJS) $(LDFLAGS)

$(OBJS): cmz.h

test: cmz
	./test.sh

clean:
	rm -f cmz *.o *~ tmp*

.PHONY: test clean
