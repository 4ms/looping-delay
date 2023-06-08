OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_Reset)

#include "../src/mp153-drivers/ld.h"

MEMORY
{
	ROM (rx)			: ORIGIN = A7_CODE,			LENGTH = A7_CODE_SZ
	RAM (rw)			: ORIGIN = A7_RAM,			LENGTH = A7_RAM_SZ
	HEAP (rw)			: ORIGIN = A7_HEAP,			LENGTH = A7_HEAP_SZ
	DMABUFFER (rw)		: ORIGIN = DMABUF,			LENGTH = DMABUF_SZ

	SHAREDMEMINDEX (rw) : ORIGIN = A7_SHARED_LIST,	LENGTH = SHARED_LIST_SZ
	SYSRAM (rw)			: ORIGIN = 0x2FFC0000,		LENGTH = 256K
}

/* A7 stacks are at end of RAM */

_shared_list = A7_SHARED_LIST;

SECTIONS
{
	.text : {
		*(.vector_table)
		*(.resethandler)
		*(.irqhandler)
		*(.text)
		*(.text*)
					/* Todo: check if we need the next 5 lines */
		*(.glue_7)         /* glue arm to thumb code */
		*(.glue_7t)        /* glue thumb to arm code */
		*(.eh_frame)
		KEEP (*(.init))
		KEEP (*(.fini))

		. = ALIGN(8);
	 } > ROM

	.rodata :
	{
		. = ALIGN(8);
		*(.rodata)         /* .rodata sections (constants, strings, etc.) */
		*(.rodata*) 
		. = ALIGN(8);
	} > ROM

	.ARM.extab	 : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ROM
	.ARM : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } >ROM

	.preinit_array :
	{
		. = ALIGN(8);
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array*))
		PROVIDE_HIDDEN (__preinit_array_end = .);
		. = ALIGN(8);
	} > ROM

	.init_array :
	{
		. = ALIGN(8);
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array*))
		PROVIDE_HIDDEN (__init_array_end = .);
		. = ALIGN(8);
	} > ROM

	.fini_array :
	{
		. = ALIGN(8);
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array*))
		PROVIDE_HIDDEN (__fini_array_end = .);
		. = ALIGN(8);
	} > ROM


	_text_end = .;

	.data : AT(_text_end)
	{
		. = ALIGN(8);
		_data_start = .;
		*(.data)
		*(.data*)
		. = ALIGN(8);
		_data_end = .;
	} > RAM

	.bss : {
		_bss_start = .;
		*(.bss)
		*(.bss*)   /* required for some malloc calls */
		*(COMMON)  /* required for libc, such as __lock___atexit_recursive_mutex */
		. = ALIGN(8);
		_bss_end = .;
		. = ALIGN(256);
		_ram_aligned_end = .;
	} > RAM

	.heap (NOLOAD):
	{
		. = ALIGN(8);
		_sheap = .;
		. += A7_HEAP_SZ;
		_eheap = .;
	} > HEAP

	.ddma (NOLOAD):
	{
		. = ALIGN(8);
		*(.ddma)
	} > DMABUFFER

	.sysram (NOLOAD):
	{
		. = ALIGN(8);
		*(.sysram)
		*(.noncachable)
		. = ALIGN(8);
		_end_of_sysram = .;
	} > SYSRAM


	_user_stack_start = _ram_aligned_end;
	_user_stack_end = _user_stack_start + 0x100000; /* 1M */

	_svc_stack_start = _user_stack_end;
	_svc_stack_end = _svc_stack_start + 0x6000; /* 24k for ISRs*/

	_irq_stack_start = _svc_stack_end;
	_irq_stack_end = _irq_stack_start + 0x100; 

	_fiq_stack_start = _irq_stack_end;
	_fiq_stack_end = _fiq_stack_start + 0x100;

	_auxcore_user_stack_start = _fiq_stack_end;
	_auxcore_user_stack_end = _auxcore_user_stack_start + 0x1000;


	.sharedmemindex (NOLOAD) :
	{
		. = ALIGN(8);
		*(.sharedmemindex)
		*(.sharedmemindex*)
		. = ALIGN(8);
	} > SHAREDMEMINDEX
	

  /* Remove information from the compiler libraries */
  /DISCARD/ :
  {
	libc.a ( * )
	libm.a ( * )
	libgcc.a ( * )
  }

	.ARM.attributes 0 : { *(.ARM.attributes) }
}
