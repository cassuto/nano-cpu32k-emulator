
#include "cpu.h"
#include "debug.h"
#include <stdio.h>

static const char enable_debug = 1;

/**
 * @brief Put a character to debug console
 * @param ch Target char
 */
void debug_putc(uint8_t ch)
{
  if(enable_debug)
    {
      putchar(ch);
      fflush(stdout);
    }
}

void
memory_breakpoint(vm_addr_t addr, uint32_t val)
{
  verbose_print_1("memory BP (%#x): addr = %#x, val = %#x\n", cpu_pc, addr, val);
}
