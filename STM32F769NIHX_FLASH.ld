/* GCC port: keep 64 KiB heap and 24 KiB stack from the Keil startup. */
ENTRY(Reset_Handler)
MEMORY {
 FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
 RAM (xrw) : ORIGIN = 0x20020000, LENGTH = 368K
 DMA_RAM (xrw) : ORIGIN = 0x2007C000, LENGTH = 16K
 ROM_RAM (xrw) : ORIGIN = 0xC0400000, LENGTH = 1024K
 DTCM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
}
_estack = ORIGIN(DTCM) + LENGTH(DTCM);
_Min_Stack_Size = 0x6000;
_Min_Heap_Size = 0x10000;
SECTIONS {
 .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } >FLASH
 .text : { . = ALIGN(4); *(.text .text.*) *(.glue_7*) *(.eh_frame*) KEEP(*(.init)) KEEP(*(.fini)) . = ALIGN(4); _etext = .; } >FLASH
 .rodata : { . = ALIGN(4); *(.rodata .rodata.*) . = ALIGN(4); } >FLASH
 .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
 .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } >FLASH
 .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } >FLASH
 .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array)) PROVIDE_HIDDEN(__init_array_end = .); } >FLASH
 .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array)) PROVIDE_HIDDEN(__fini_array_end = .); } >FLASH
 _sidata = LOADADDR(.data);
 .data : { . = ALIGN(4); _sdata = .; *(.data .data.*) . = ALIGN(4); _edata = .; } >RAM AT>FLASH
 .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; __bss_start__ = .; *(.bss .bss.*) *(COMMON) . = ALIGN(4); _ebss = .; __bss_end__ = .; } >RAM
 .heap (NOLOAD) : { . = ALIGN(8); end = .; _end = .; __heap_start__ = .; . += _Min_Heap_Size; __heap_end__ = .; } >DTCM
 .stack _estack - _Min_Stack_Size (NOLOAD) : { __stack_limit__ = .; . += _Min_Stack_Size; } >DTCM
 .nes_rom (NOLOAD) : { . = ALIGN(32); *(.nes_rom) . = ALIGN(32); } >ROM_RAM
 .ARM.attributes 0 : { *(.ARM.attributes) }
 .dma_audio (NOLOAD) : { . = ALIGN(32); *(.dma_audio) . = ALIGN(32); } >DMA_RAM
 ASSERT(_ebss <= ORIGIN(RAM) + LENGTH(RAM), "SRAM1 overflow")
 ASSERT(__heap_end__ <= __stack_limit__, "DTCM heap/stack overlap")
 ASSERT(SIZEOF(.dma_audio) <= LENGTH(DMA_RAM), "Audio DMA buffer overflow")
}
