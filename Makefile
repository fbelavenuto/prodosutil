# Makefile

CC = gcc
LD = gcc
CP = cp
RM = rm -f
MD = mkdir

SDIR = src
ODIR = obj
IDIR = inc

CFLAGS = -g -Wall -I$(IDIR)
LDFLAGS = 

_OBJ = prodos.o
OBJS = $(addprefix $(ODIR)/, $(_OBJ))

all: $(ODIR) prodos

prodos: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(ODIR):
	$(MD) $(ODIR)

.PHONY: clean install

clean:
	$(RM) *.exe prodos $(ODIR)/*

install: prodos
	$(CP) $< /usr/local/bin



$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SDIR)/%.a65
	$(AAS) $(AAFLAGS) -o $@ $<
