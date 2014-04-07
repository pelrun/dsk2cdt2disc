EXOMIZER_PATH = ./exomizer207
2CDT_PATH = ./2cdt

VPATH := $(EXOMIZER_PATH)/src $(2CDT_PATH)/src ./dsk2cdt-src

SHARED_OBJS = getflag.o log.o membuf.o
RAW_OBJS = match.o search.o optimal.o output.o membuf_io.o \
           chunkpool.o radix.o exo_helper.o exodec.o progress.o exo_util.o crunch.o 

2CDT_OBJS = cdt.o tzxfile.o

DSK2CDT_OBJS = dsk2cdt.o loader.o

INCLUDES = -I$(EXOMIZER_PATH)/src -I$(2CDT_PATH)/src -Idsk2cdt-src

CFLAGS = -g -O3

dsk2cdt: $(DSK2CDT_OBJS) libexo.a lib2cdt.a
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -L. -o $@ $(DSK2CDT_OBJS) -lexo -l2cdt

loader.o: loader.txt rsx.bin
	$(LD) -r -b binary -o $@ $^

rsx.bin: rsx.s
	z80-unknown-coff-as.exe $< -o rsx.o
	z80-unknown-coff-objcopy.exe rsx.o -O binary rsx-padded.bin
	dd if=rsx-padded.bin bs=4k skip=9 of=rsx.bin

libexo.a: $(RAW_OBJS) $(SHARED_OBJS)
	@$(AR) r $@ $^

# 2cdt.c has a main function we don't want, so weaken the symbols
lib2cdt.a: $(2CDT_OBJS)
	@$(AR) r $@ $^
	@objcopy --weaken $@

%.o:	%.c
	@echo "Compiling $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	$(RM) *.a *.o *.exe *.bin $(EXOMIZER_PATH)/src/*.o $(2CDT_PATH)/src/*.o

.PHONY: clean
