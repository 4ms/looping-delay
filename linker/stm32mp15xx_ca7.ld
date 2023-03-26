OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_Reset)

/* INCLUDE ./memory.ld */

/* Presume 256MB RAM */
_A7_CODE 		= 0xC2000040;
_A7_CODE_SZ  	= 16M - 0x40;/*0x00FFFFD8*/
_A7_CODE_END 	= 0xC3000000;

_A7_RAM 		= 0xC3000000;
_A7_RAM_SZ 		= 144M;		/*0x09000000*/
_A7_RAM_END 	= 0xCC000000;

_A7_HEAP		= 0xCC000000;
_A7_HEAP_SZ 	= 63M; 	/*0x03F00000*/
_A7_HEAP_END	= 0xCFF00000;

_DMABUF			= 0xCFF00000;
_DMABUF_SZ		= 1M;		/*0x00100000*/
_DMABUF_END		= 0xD0000000;

_M4_SHARED_LIST 	= 0x1005FF00;
_A7_SHARED_LIST 	= 0x3005FF00;
_SHARED_LIST_SZ 	= 256;
_M4_SHARED_LIST_END = 0x10060000;

MEMORY
{
    ROM (rx)			: ORIGIN = _A7_CODE,		LENGTH = _A7_CODE_SZ
    RAM (rw)			: ORIGIN = _A7_RAM,			LENGTH = _A7_RAM_SZ
	HEAP (rw)			: ORIGIN = _A7_HEAP,		LENGTH = _A7_HEAP_SZ
	DMABUF (rw)			: ORIGIN = _DMABUF,			LENGTH = _DMABUF_SZ 

	SHAREDMEMINDEX (rw) : ORIGIN = _A7_SHARED_LIST, LENGTH = _SHARED_LIST_SZ
	SYSRAM (rw)			: ORIGIN = 0x2FFC0000,		LENGTH = 256K
}

/* A7 stacks are at end of RAM */

_shared_list = _A7_SHARED_LIST;

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
		. += _A7_HEAP_SZ;
		_eheap = .;
	} > HEAP

	.ddma (NOLOAD):
	{
		. = ALIGN(8);
        *(.ddma)
	} > DMABUF

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
