TARGET = test
OBJS = test.o algorithms.o common.o opagedmem.o oram.o
DEPS = $(OBJS:.o=.d)

LIB = ../liboblivious.a

CPPFLAGS = -MMD -I../include
CFLAGS = -Wall -Wextra -g
LDFLAGS =
LDLIBS = \
	$(LIB)

all: $(TARGET)

$(TARGET): $(LIB) $(OBJS)

$(LIB): FORCE
	$(MAKE) -C ..

clean: FORCE
	rm -rf $(TARGET) $(OBJS) $(DEPS)
	$(MAKE) -C .. clean

FORCE:

-include $(DEPS)
