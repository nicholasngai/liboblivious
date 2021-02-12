TARGET = liboblivious.so
OBJS = primitives.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD
CFLAGS = -Iinclude -Wall -Wextra
LDFLAGS = -shared

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)
