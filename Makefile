EXOMIZER_PATH = ./thirdparty/exomizer
2CDT_PATH = ./thirdparty/2cdt

VPATH := $(EXOMIZER_PATH)/src $(2CDT_PATH)/src ./src

SHARED_OBJS = getflag.o log.o membuf.o vec.o
RAW_OBJS = match.o search.o optimal.o output.o membuf_io.o \
           chunkpool.o radix.o exo_helper.o exodec.o progress.o exo_util.o crunch.o

2CDT_OBJS = cdt.o tzxfile.o opth.o

DSK2CDT_OBJS = dsk2cdt.o

INCLUDES = -I$(EXOMIZER_PATH)/src -I$(2CDT_PATH)/src -Isrc

CFLAGS = -g -O3
LDFLAGS = --static

all: dsk2cdt

dsk2cdt: $(DSK2CDT_OBJS) $(2CDT_OBJS) $(RAW_OBJS) $(SHARED_OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -o $@ $^

dsk2cdt.o: rsx.bin loader.bin

rsx.bin: rsx.s
	z80-unknown-coff-as $< -o rsx.o
	z80-unknown-coff-objcopy rsx.o -O binary rsx-padded.bin
	dd if=rsx-padded.bin of=$@ bs=36864 skip=1

loader.bin: loader.txt
	unix2dos -n $< $@

%.o:	%.c
	@echo "Compiling $<"
	$(CC) -v -c $(CFLAGS) $(INCLUDES) -o $@ $<

thirdparty:
	unzip -n -d thirdparty thirdparty/2cdt.zip
	unzip -n -d thirdparty/exomizer thirdparty/exomizer-2.0.11.zip
	$(RM) $(EXOMIZER_PATH)/src/*.o $(2CDT_PATH)/src/*.o

clean:
	$(RM) dsk2cdt *.a *.o *.exe *.bin $(EXOMIZER_PATH)/src/*.o $(2CDT_PATH)/src/*.o

.PHONY: clean thirdparty
