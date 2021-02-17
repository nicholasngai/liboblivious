TARGET = test
OBJS = test.o algorithms.o common.o opagedmem.o oram.o
DEPS = $(OBJS:.o=.d)

CPPFLAGS = -MMD
CFLAGS = -g -I../include
LDLIBS = -L.. -l:liboblivious.a

all: $(TARGET)

$(TARGET): .. $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(TARGET)

..: FORCE
	$(MAKE) -C ..

clean: FORCE
	rm -rf $(TARGET) $(OBJS) $(DEPS)

FORCE:

-include $(DEPS)
