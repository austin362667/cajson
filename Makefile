CC = gcc
CFLAGS = -g -I ./src
OBJS = cj

all: $(OBJS)

cj: src/cj.c
	$(CC) $(CFLAGS) $^ -o $@

run: $(OBJS)
	./cj prog/hello.cj
	./cj prog/stmt1.cj
	./cj prog/fn.cj
	./cj prog/sum.cj
	./cj prog/matrix.cj
	./cj prog/array.cj
	./cj prog/mt.cj
	./cj prog/if.cj

clean:
	rm -f $(OBJS)
