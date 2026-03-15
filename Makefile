CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
LDFLAGS ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man
MAN1DIR ?= $(MANDIR)/man1

TARGET := simplecall
SRCS := simplecall.c third_party/hexdump/hexdump.c
MAN := simplecall.1

CFLAGS += -Ithird_party

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -vf $(TARGET)

install: $(TARGET) $(MAN)
	install -vd $(DESTDIR)$(BINDIR) $(DESTDIR)$(MAN1DIR)
	install -vm 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -vm 0644 $(MAN) $(DESTDIR)$(MAN1DIR)/$(MAN)

uninstall:
	rm -vf $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -vf $(DESTDIR)$(MAN1DIR)/$(MAN)
