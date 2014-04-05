
// symbols exported by loader.o

extern char binary___dsk2cdt_src_loader_txt_start[];
extern char binary___dsk2cdt_src_loader_txt_end[];
extern char binary___dsk2cdt_src_loader_txt_size[];
extern char binary___dsk2cdt_src_rsx_bin_start[];
extern size_t binary___dsk2cdt_src_rsx_bin_end[];
extern size_t binary___dsk2cdt_src_rsx_bin_size[];

// the symbols out of ld are unwieldy, so make them easier to work with.
// gcc doesn't let us do symbol aliasing with external symbols, bah

#define loader_txt_start   binary___dsk2cdt_src_loader_txt_start
#define loader_txt_end     binary___dsk2cdt_src_loader_txt_end
#define loader_txt_size    binary___dsk2cdt_src_loader_txt_size

#define rsx_bin_start      binary___dsk2cdt_src_rsx_bin_start
#define rsx_bin_end        binary___dsk2cdt_src_rsx_bin_end
#define rsx_bin_size       binary___dsk2cdt_src_rsx_bin_size
