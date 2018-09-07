
CFLAGS += -Wall -g
LDLIBS += -l m

TARGET := mempipe
SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)


all:$(OBJS)
	gcc $(CFLAGS)  -o $(TARGET) $(OBJS) $(LDLIBS) $(INCLUDEFLAGS) 
$(OBJS):%.o:%.c
	gcc $(CFLAGS)  -c $< -o $@ $(LDFLAGS) $(INCLUDEFLAGS) 

%.d:$(SRCS)
	@set -e; \
	rm -f $@; \
	$(CC) -MM $< $(INCLUDEFLAGS) > $@.depends; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.depends > $@; \
	rm -f $@.depends
	

include $(OBJS:.o=.d)

.PHONY:clean
clean:
	echo $(OBJS)
	rm -f $(TARGET) $(OBJS) *.d *.d.*
