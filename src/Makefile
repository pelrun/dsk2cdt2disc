EXOMIZER_PATH = ./exomizer209
2CDT_PATH = ./2cdt

VPATH := $(EXOMIZER_PATH)/src $(2CDT_PATH)/src ./dsk2cdt-src

SHARED_OBJS = getflag.o log.o membuf.o vec.o
RAW_OBJS = match.o search.o optimal.o output.o membuf_io.o \
           chunkpool.o radix.o exo_helper.o exodec.o progress.o exo_util.o crunch.o 

2CDT_OBJS = cdt.o tzxfile.o opth.o

DSK2CDT_OBJS = dsk2cdt.o cpc-code.o

INCLUDES = -I$(EXOMIZER_PATH)/src -I$(2CDT_PATH)/src -Idsk2cdt-src

CFLAGS = -g -O3

dsk2cdt: $(DSK2CDT_OBJS) $(2CDT_OBJS) $(RAW_OBJS) $(SHARED_OBJS)
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

cpc-code.o: cpc-code.s rsx.bin loader.bin
	$(AS) $< $(INCLUDES) -o cpc-code.o

rsx.bin: rsx.s
	z80-unknown-coff-as $< -o rsx.o
	z80-unknown-coff-objcopy rsx.o -O binary $@

loader.bin: loader.txt
	unix2dos -n $< $@

%.o:	%.c
	@echo "Compiling $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	$(RM) *.a *.o *.exe *.bin $(EXOMIZER_PATH)/src/*.o $(2CDT_PATH)/src/*.o

.PHONY: clean
