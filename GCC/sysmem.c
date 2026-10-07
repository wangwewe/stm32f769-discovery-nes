/* Bounded newlib heap, matching the original Keil allocation. */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "stm32f7xx.h"
extern char __heap_start__, __heap_end__;
void *_sbrk(ptrdiff_t increment)
{
    static uintptr_t current;
    uintptr_t start = (uintptr_t)&__heap_start__;
    uintptr_t limit = (uintptr_t)&__heap_end__;
    if (!current) {
        current = start;
    }
    uintptr_t previous = current;
    if (increment >= 0) {
        if ((uintptr_t)increment > limit - current) { errno = ENOMEM; return (void *)-1; }
        current += (uintptr_t)increment;
    } else {
        uintptr_t amount = (uintptr_t)(-(increment + 1)) + 1;
        if (amount > current - start) { errno = ENOMEM; return (void *)-1; }
        current -= amount;
    }
    return (void *)previous;
}
/* -nostartfiles: startup calls __libc_init_array itself. */
void _init(void) {}
void _fini(void) {}
