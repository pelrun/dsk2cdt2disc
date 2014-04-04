CC = gcc

EXOMIZER_PATH = ./exomizer207
2CDT_PATH = ./2cdt

VPATH := $(EXOMIZER_PATH)/src $(2CDT_PATH)/src ./dsk2cdt

SHARED_OBJS = getflag.o log.o membuf.o
RAW_OBJS = match.o search.o optimal.o output.o membuf_io.o \
           chunkpool.o radix.o exo_helper.o exodec.o progress.o exo_util.o

DSK2CDT_OBJS = dsk2cdt.o crunch.o loader.o

INCLUDES = -I$(EXOMIZER_PATH)/src -Idsk2cdt

dsk2cdt: $(DSK2CDT_OBJS) libexo.a
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -L. -o $@ $(DSK2CDT_OBJS) -lexo

loader.o: loader.txt rsx.bin
	$(LD) -r -b binary -o $@ $^

libexo.a: $(RAW_OBJS) $(SHARED_OBJS)
	@$(AR) r $@ $^

%.o:	%.c
	@echo "Compiling $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f *.a *.o *.exe

.PHONY: clean