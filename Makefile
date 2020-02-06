
CC = gcc
CFLAGS = -std=gnu99 -g -Wall -Wno-unused-function $(INCS) $(DEFS)
LDFLAGS = -g

OBJS = main-loop.o \
		exec.o \
		memory-mmio.o \
		debug.o \
		msr.o \
		i-mmu.o \
		d-mmu.o \
		tsc.o \
		irqc.o \
		parse-symtable.o
    
DEPS := $(OBJS:.o=.d)

.PHONY:all clean distclean

all: emu
clean:
	-rm $(OBJS)
distclean:
	-rm $(OBJS) $(DEPS)
	
emu: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
  
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.c
	$(CC) $(CFLAGS) -MM -MF $@ $<
	-@sed -i 's,\($(notdir $*)\)\.o[ :]*,$(basename $@).o: ,g' '$@'


$(OBJS): $(DEPS)

-include $(DEPS)
