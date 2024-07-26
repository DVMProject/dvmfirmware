/*
 *   Copyright (C) 2017 by Andy Uribe CA6JAU
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Required amount of heap and stack */
_min_heap_size = 0x1000;
_min_stack_size = 0x0800;

/* The entry point in the interrupt vector table */
ENTRY(Reset_Handler)

/* Memory areas */
MEMORY
{
	ROM (rx)      : ORIGIN = 0x08000000, LENGTH = 2048K    /* FLASH */
	RAM (xrw)     : ORIGIN = 0x20000000, LENGTH = 512K     /* Main RAM */
}

/* Stack start address (end of 512K RAM) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
	.text : 
	{ 
		/* The interrupt vector table */
		. = ALIGN(4);
		KEEP(*(.isr_vector .isr_vector.*))
		
		/* The program code */
		. = ALIGN(4);
		*(.text .text*)
		*(.rodata .rodata*)
		
		/* ARM-Thumb code */
		*(.glue_7) *(.glue_7t)              
		
		. = ALIGN(4);
		KEEP(*(.init))
		KEEP(*(.fini))
		
		/* EABI C++ global constructors support */
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))		
		__preinit_array_end = .;
		
		/* EABI C++ global constructors support */
		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
		
		/* EABI C++ global constructors support */
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;

	} > ROM
 
 	/* ARM sections containing exception unwinding information */
	.ARM.extab : {
		__extab_start = .;
		*(.ARM.extab* .gnu.linkonce.armextab.*) 
		__extab_end = .;
	} > ROM
	
	/* ARM index entries for section unwinding */
	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} > ROM
    
    /* Start address for the initialization values of the .data section */
	_sidata = .;

	/* The .data section (initialized data) */
	.data : AT ( _sidata )
	{
		. = ALIGN(4);
		_sdata = . ;       /* Start address for the .data section */
		*(.data .data*)
		
		. = ALIGN(4);
		_edata = . ;       /* End address for the .data section */
	} > RAM

	/* The .bss section (uninitialized data) */
	.bss :
	{
		. = ALIGN(4);
		_sbss = .;         /* Start address for the .bss section */
		__bss_start__ = _sbss;
		*(.bss)
		*(.bss*)
		*(COMMON)
		
		. = ALIGN(4);
		_ebss = . ;        /* End address for the .bss section */
    	__bss_end__ = _ebss;
	} > RAM

	/* Space for heap and stack */
	.heap_stack :
	{
		end = . ;		  		 /* 'end' symbol defines heap location */
		_end = end ;
		. = . + _min_heap_size;  /* Additional space for heap and stack */
		. = . + _min_stack_size;
	} > RAM

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) } > ROM
    .stabstr       0 : { *(.stabstr) } > ROM
    .stab.excl     0 : { *(.stab.excl) } > ROM
    .stab.exclstr  0 : { *(.stab.exclstr) } > ROM
    .stab.index    0 : { *(.stab.index) } > ROM
    .stab.indexstr 0 : { *(.stab.indexstr) } > ROM
    .comment       0 : { *(.comment) } > ROM 
    .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) } > ROM
    /* DWARF debug sections.
        Symbols in the DWARF debugging sections are relative to the beginning
        of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) } > ROM
    .line           0 : { *(.line) } > ROM
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) } > ROM
    .debug_sfnames  0 : { *(.debug_sfnames) } > ROM
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) } > ROM
    .debug_pubnames 0 : { *(.debug_pubnames) } > ROM
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) } > ROM
    .debug_abbrev   0 : { *(.debug_abbrev) } > ROM
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) } > ROM
    .debug_frame    0 : { *(.debug_frame) } > ROM
    .debug_str      0 : { *(.debug_str) } > ROM
    .debug_loc      0 : { *(.debug_loc) } > ROM
    .debug_macinfo  0 : { *(.debug_macinfo) } > ROM

	/* Remove information from the standard libraries */
	/DISCARD/ :
	{
		libc.a ( * )
		libm.a ( * )
		libgcc.a ( * )
	}
}
