#ifndef dobby_h
#define dobby_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef uintptr_t addr_t;
typedef void *asm_func_t;

typedef enum {
    kInstructionSoft,
    kInstructionHard
} InstructionType;

// The main hooking function we call in main.cpp
int DobbyHook(void *function_address, void *replace_call, void **origin_call);

// Helper to resolve dynamic symbols in the ELF library
void *DobbySymbolResolver(const char *image_name, const char *symbol_name);

#ifdef __cplusplus
}
#endif

#endif
