#ifndef __LOADER_H_
#define __LOADER_H_

#ifdef _WIN32
// symbols exported by loader.o

extern char binary___loader_donotedit_crlf_bin_start[];
extern char binary___loader_donotedit_crlf_bin_end[];
extern char binary___loader_donotedit_crlf_bin_size[];
extern char binary_rsx_bin_start[];
extern size_t binary_rsx_bin_end[];
extern size_t binary_rsx_bin_size[];

// the symbols out of ld are unwieldy, so make them easier to work with.
// gcc doesn't let us do symbol aliasing with external symbols, bah

#define loader_txt_start   binary___loader_donotedit_crlf_bin_start
#define loader_txt_end     binary___loader_donotedit_crlf_bin_end
#define loader_txt_size    binary___loader_donotedit_crlf_bin_size

#define rsx_bin_start      binary_rsx_bin_start
#define rsx_bin_end        binary_rsx_bin_end
#define rsx_bin_size       binary_rsx_bin_size

#else

// symbols exported by loader.o

extern char _binary_loader_donotedit_crlf_bin_start[];
extern char _binary_loader_donotedit_crlf_bin_end[];
extern char _binary_loader_donotedit_crlf_bin_size[];
extern char _binary___dsk2cdt_src_rsx_bin_start[];
extern size_t _binary___dsk2cdt_src_rsx_bin_end[];
extern size_t _binary___dsk2cdt_src_rsx_bin_size[];

// the symbols out of ld are unwieldy, so make them easier to work with.
// gcc doesn't let us do symbol aliasing with external symbols, bah

#define loader_txt_start   _binary_loader_donotedit_crlf_bin_start
#define loader_txt_end     _binary_loader_donotedit_crlf_bin_end
#define loader_txt_size    _binary_loader_donotedit_crlf_bin_size

#define rsx_bin_start      _binary___dsk2cdt_src_rsx_bin_start
#define rsx_bin_end        _binary___dsk2cdt_src_rsx_bin_end
#define rsx_bin_size       _binary___dsk2cdt_src_rsx_bin_size

#endif

#endif // __LOADER_H_
