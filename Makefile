SCHRIFT = libschrift

OBJS = \
	fntconv.o \
	buildfnt.o \
	dumpfna.o \
	dumpfont.o \
	dumpgrx.o

CFLAGS  = -Wall -O2 -I. #-DDEBUG
LDFLAGS = -L$(SCHRIFT)
LIBS    = -lX11 -lschrift -lm

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
