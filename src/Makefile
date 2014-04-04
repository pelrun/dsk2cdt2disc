CC = gcc

VPATH := ./exomizer207/src ./2cdt/src ./dsk2cdt

SHARED_OBJS = getflag.o log.o membuf.o
RAW_OBJS = match.o search.o optimal.o output.o membuf_io.o \
           chunkpool.o radix.o exo_helper.o exodec.o progress.o exo_util.o

DSK2CDT_OBJS = dsk2cdt.o crunch.o

INCLUDES = -Iexomizer207/src -Idsk2cdt

dsk2cdt: $(DSK2CDT_OBJS) libexo.a
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -L. -o $@ $(DSK2CDT_OBJS) -lexo
		   
libexo.a: $(RAW_OBJS) $(SHARED_OBJS)
	@$(AR) r $@ $^

%.o:	%.c
	@echo "Compiling $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
	
clean:
	rm -f *.a *.o *.exe

.PHONY: clean