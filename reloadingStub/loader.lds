OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

MEMORY {
	stub (rx) : org = 0x80001800, len = 0x1800
	mem1 : org = 0x80004000, len = 0x017fc000
	mem2 : org = 0x90000000, len = 0x03400000
}

SECTIONS {
	.init : {
		KEEP(*crt0.o(*.init))
		KEEP(*(.init))
	}

	.text : { *(.text) }
	.rodata : { *(.rodata .rodata.*) }

	.data : {
		__data_start = .;
		*(.data)
		__data_end = .;
	} > mem2 AT > stub
	__data_init = LOADADDR(.data);

	.bss : {
		__bss_start = .;
		*(.sbss) *(.bss) *(COMMON);
		. = ALIGN(0x40);
		__bss_end = .;
	} > mem2

	.stack : {
		. += 0x8000;
		__stack_top = .;
	} > mem2

	__code_buffer = __stack_top + 0x40;
}
