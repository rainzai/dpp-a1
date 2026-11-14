PROGNAME = assign1_3
SRCFILES = assign1_3.c queue.c
TARNAME = assign1_3.tgz

# number of primes to print
RUNARGS = 100

CC = gcc

WARNFLAGS = -Wall -Werror-implicit-function-declaration -Wshadow \
		  -Wstrict-prototypes -pedantic-errors
CFLAGS = -std=c99 -ggdb -O2 $(WARNFLAGS) -D_POSIX_C_SOURCE=200112
LFLAGS = -lpthread

OBJFILES = $(patsubst %.c,%.o,$(SRCFILES))

.PHONY: all run runlocal clean dist

all: $(PROGNAME) assign1

$(PROGNAME): $(OBJFILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

# Copy named assign1, so that "./assign1 3 <number of primes>" also works.
assign1: $(PROGNAME)
	cp $(PROGNAME) assign1

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

run: $(PROGNAME)
	prun -v -np 1 $(PROGNAME) $(RUNARGS)

runlocal: $(PROGNAME)
	./$(PROGNAME) $(RUNARGS)

dist:
	tar cvzf $(TARNAME) Makefile *.c *.h

clean:
	rm -fv $(PROGNAME) assign1 $(OBJFILES) $(TARNAME)
