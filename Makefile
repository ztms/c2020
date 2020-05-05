CFLAGS=-std=c11 -g -static

cmz: cmz.c

test: cmz
	./test.sh

clean:
	rm -f cmz *.o *~ tmp*

.PHONY: test clean
