TARGET = test
OBJS = test.o algorithms.o common.o opagedmem.o oram.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD
CFLAGS = -g -I../include

$(TARGET): $(OBJS) ../liboblivious.a

../liboblivious.a: FORCE
	$(MAKE) -C .. liboblivious.a

clean: FORCE
	rm -rf $(TARGET) $(OBJS) $(DEPS)

FORCE:

-include $(DEPS)
