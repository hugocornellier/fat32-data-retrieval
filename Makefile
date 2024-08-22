CC = gcc
CFLAGS = -Wall -g -std=gnu99

LDLIBS =

SRCDIR = src
OBJDIR = obj
BINDIR = bin
INCDIR = headers

OBJS = $(OBJDIR)/fat32.o $(OBJDIR)/main.o $(OBJDIR)/shell.o

EXE = $(BINDIR)/fat32

all: $(EXE)

$(EXE): $(OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

$(OBJDIR)/shell.o: $(SRCDIR)/shell.c $(INCDIR)/shell.h $(INCDIR)/fat32.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/shell.c -o $@

$(OBJDIR)/fat32.o: $(SRCDIR)/fat32.c $(INCDIR)/fat32.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/fat32.c -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.c $(INCDIR)/shell.h
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $@

clean:
	rm -f $(OBJDIR)/*.o
	rm -f *~
	rm -f $(EXE)
