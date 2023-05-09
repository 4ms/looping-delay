#pragma once
#include <stdint.h>
// TODO: refactor this:
//  #define A7_CODE 0xC2000040, etc in a header (with ONLY that)
//  pre-process linker script to use these symbols
//  no 'extern uint32_t LD_XXX[];' needed!

// Defined in linker scripts:

extern uint32_t LD_UNUSED0[];
extern uint32_t LD_UNUSED0_SZ[];
extern uint32_t LD_UNUSED0_END[];

extern uint32_t LD_TTB[];
extern uint32_t LD_TTB_SZ[];
extern uint32_t LD_TTB_END[];

// N
extern uint32_t LD_A7_CODE[];
extern uint32_t LD_A7_CODE_SZ[];
extern uint32_t LD_A7_CODE_END[];

// N-RW
extern uint32_t LD_A7_RAM[];
extern uint32_t LD_A7_RAM_SZ[];
extern uint32_t LD_A7_RAM_END[];

// N-RW
extern uint32_t LD_A7_HEAP[];
extern uint32_t LD_A7_HEAP_SZ[];
extern uint32_t LD_A7_HEAP_END[];

// SO
extern uint32_t LD_DMABUF[];
extern uint32_t LD_DMABUF_SZ[];
extern uint32_t LD_DMABUF_END[];

// SRAM1-3: Device-RW
extern uint32_t LD_M4_CODE[];
extern uint32_t LD_M4_CODE_SZ[];
extern uint32_t LD_M4_CODE_END[];

// SRAM4: Device-RW
extern uint32_t LD_M4_STACK[];
extern uint32_t LD_M4_STACK_SZ[];
extern uint32_t LD_M4_STACK_END[];

// SRAM4: Device-RW
extern uint32_t LD_M4_SHARED_LIST[];
extern uint32_t LD_A7_SHARED_LIST[];
extern uint32_t LD_SHARED_LIST_SZ[];
extern uint32_t LD_M4_SHARED_LIST_END[];

// Not listed:
// SYSRAM is SO

//// as uint32_t's:

// static uint32_t UNUSED0 = (uint32_t)LD_UNUSED0;
// static uint32_t UNUSED0_SZ = (uint32_t)LD_UNUSED0_SZ;
// static uint32_t UNUSED0_END = (uint32_t)LD_UNUSED0_END;

// static uint32_t TTB = (uint32_t)LD_TTB;
// static uint32_t TTB_SZ = (uint32_t)LD_TTB_SZ;
// static uint32_t TTB_END = (uint32_t)LD_TTB_END;

static uint32_t A7_CODE = (uint32_t)LD_A7_CODE;
// static uint32_t A7_CODE_SZ = (uint32_t)LD_A7_CODE_SZ;
static uint32_t A7_CODE_END = (uint32_t)LD_A7_CODE_END;

static uint32_t A7_RAM = (uint32_t)LD_A7_RAM;
static uint32_t A7_RAM_SZ = (uint32_t)LD_A7_RAM_SZ;
// static uint32_t A7_RAM_END = (uint32_t)LD_A7_RAM_END;

static uint32_t A7_HEAP = (uint32_t)LD_A7_HEAP;
static uint32_t A7_HEAP_SZ = (uint32_t)LD_A7_HEAP_SZ;
// static uint32_t A7_HEAP_END = (uint32_t)LD_A7_HEAP_END;

static uint32_t DMABUF = (uint32_t)LD_DMABUF;
static uint32_t DMABUF_SZ = (uint32_t)LD_DMABUF_SZ;
// static uint32_t DMABUF_END = (uint32_t)LD_DMABUF_END;

// static uint32_t M4_CODE = (uint32_t)LD_M4_CODE;
// static uint32_t M4_CODE_SZ = (uint32_t)LD_M4_CODE_SZ;
// static uint32_t M4_CODE_END = (uint32_t)LD_M4_CODE_END;

// static uint32_t M4_STACK = (uint32_t)LD_M4_STACK;
// static uint32_t M4_STACK_SZ = (uint32_t)LD_M4_STACK_SZ;
// static uint32_t M4_STACK_END = (uint32_t)LD_M4_STACK_END;

// static uint32_t M4_SHARED_LIST = (uint32_t)LD_M4_SHARED_LIST;
// static uint32_t A7_SHARED_LIST = (uint32_t)LD_A7_SHARED_LIST;
// static uint32_t SHARED_LIST_SZ = (uint32_t)LD_SHARED_LIST_SZ;
// static uint32_t M4_SHARED_LIST_END = (uint32_t)LD_M4_SHARED_LIST_END;
