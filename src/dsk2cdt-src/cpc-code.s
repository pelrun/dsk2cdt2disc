.section .rodata

.global loader
.align 4

loader:
.incbin "loader.bin"

.global loader_size
.align 4

loader_size:
.int loader_size - loader

.global rsx
.align 4

rsx:
.incbin "rsx.bin",0x9000

.global rsx_size
.align 4

rsx_size:
.int rsx_size - rsx
