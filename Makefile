TARGET_SO = liboblivious.so
TARGET_AR = liboblivious.a
OBJS = algorithms.o opagedmem.o oram.o primitives.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD -Iinclude
CFLAGS = -O3 -Wall -Wextra
LDFLAGS = -shared

all: $(TARGET_SO) $(TARGET_AR) FORCE

$(TARGET_SO): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TARGET_AR): $(OBJS)
	$(AR) rcs $@ $^

clean: FORCE
	rm -rf $(TARGET_SO) $(TARGET_AR) $(OBJS) $(DEPS)

FORCE:

-include $(DEPS)
