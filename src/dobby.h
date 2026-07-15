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

// This defines the missing type the compiler was complaining about!
typedef void (*dobby_dummy_func_t)(void);

// The main hooking function mapping
int DobbyHook(void *function_address, dobby_dummy_func_t replace_call, dobby_dummy_func_t *origin_call);

#ifdef __cplusplus
}
#endif

#endif
