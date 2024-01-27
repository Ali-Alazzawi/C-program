CC       = gcc
LINKER   = gcc
LFLAGS   = -Wall
SRCDIR   = src
BINDIR   = bin

SOURCES   := $(wildcard $(SRCDIR)/*.c)
INCLUDES  := $(wildcard $(SRCDIR)/*.h)
BINS     := $(patsubst src/%.c,bin/%,$(SOURCES))
rm       = rm -f

all: bin/main bin/rbc bin/train_father bin/train_process

$(BINDIR)/main: $(SRCDIR)/main.c
	$(CC) $(LFLAGS) -o $@ $<

$(BINDIR)/rbc: $(SRCDIR)/rbc.c
	$(CC) $(LFLAGS) -o $@ $<

$(BINDIR)/train_father: $(SRCDIR)/train_father.c
	$(CC) $(LFLAGS) -o $@ $<

$(BINDIR)/train_process: $(SRCDIR)/train_process.c $(SRCDIR)/train_process_utils.c
	$(CC) $(LFLAGS) -o $@ $(SRCDIR)/train_process.c $(SRCDIR)/train_process_utils.c


.PHONY: clean
clean:
	@$(rm) $(BINS) logs/*.log MAx/*
	@echo "Cleanup complete!"