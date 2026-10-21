TARGET = chan_sccp.so
OBJECTS = sccp.o sccp_debug.o sccp_config.o sccp_device.o sccp_device_registry.o \
	sccp_msg.o sccp_queue.o sccp_session.o sccp_server.o sccp_task.o sccp_utils.o
HEADERS = sccp.h sccp_debug.h sccp_config.h sccp_device.h sccp_device_registry.h \
	sccp_msg.h sccp_queue.h sccp_session.h sccp_server.h sccp_task.h \
	sccp_utils.h device/sccp_channel_tech.h device/sccp_rtp_glue.h
CFLAGS = -Wall -Wextra -Wno-unused-parameter -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Winit-self -Wmissing-format-attribute -Wformat=2 -g -fPIC \
	-D'_GNU_SOURCE' -D'AST_MODULE="chan_sccp"' -D'AST_MODULE_SELF_SYM=__internal_chan_sccp_self'
LDFLAGS = -Wall -shared

ifdef VERSION
	CFLAGS += -D'VERSION="$(VERSION)"'
endif

TESTS = tests/test_sccp_msg

.PHONY: install clean check

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

tests/test_sccp_msg: tests/test_sccp_msg.c sccp_msg.o $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $< sccp_msg.o

check: $(TESTS)
	@for t in $(TESTS); do echo "$$t"; ./$$t || exit 1; done

install: $(TARGET)
	mkdir -p $(DESTDIR)/usr/lib/asterisk/modules
	install -m 644 $(TARGET) $(DESTDIR)/usr/lib/asterisk/modules/

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET)
	rm -f $(TESTS)
