CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wno-unterminated-string-initialization -O2 -g
LDFLAGS ?=
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man
MAN1DIR ?= $(MANDIR)/man1

TARGET := simplecall
SRCS := simplecall.c third_party/hexdump/hexdump.c
MAN := simplecall.1
MANSRC := simplecall.sc
SCDOC ?= scdoc

.PHONY: all clean install uninstall

all: $(TARGET) $(MAN)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -Ithird_party -o $@ $^ $(LDFLAGS)

$(MAN): $(MANSRC)
	$(SCDOC) < $< > $@

clean:
	rm -f $(TARGET) $(MAN)

install: $(TARGET) $(MAN)
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(MAN1DIR)
	install -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -m 0644 $(MAN) $(DESTDIR)$(MAN1DIR)/$(MAN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(MAN1DIR)/$(MAN)
