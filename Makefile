SCHRIFT = libschrift

OBJS = \
	bdf.o \
	fntconv.o \
	buildfnt.o \
	dumpfna.o \
	dumpfont.o \
	dumpgrx.o

CFLAGS  = -Wall -I. -O2 #-O0 -g3 -DDEBUG
LDFLAGS = -L$(SCHRIFT)
LIBS    = -lschrift -lm

all: libschrift fntconv

libschrift: $(SCHRIFT)/libschrift.a

$(SCHRIFT)/libschrift.a:
	make -C $(SCHRIFT)/ libschrift.a

%o: %o Makefile
	$(CC) $(CFLAGS) -c $< -o $@

fntconv: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f fntconv $(OBJS)
	make -C libschrift/ clean
