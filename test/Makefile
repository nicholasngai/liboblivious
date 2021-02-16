TARGET = test
OBJS = test.o algorithms.o common.o opagetable.o oram.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD
CFLAGS = -g -I../include

$(TARGET): $(OBJS) ../liboblivious.a

../liboblivious.a: FORCE
	$(MAKE) -C .. liboblivious.a

.PHONY: FORCE
FORCE:

-include $(DEPS)
