
#include "openpx64k-emu.h"
#include <stdio.h>


static char enable_debug = 1;

void debug_putc(uint8_t ch)
{
  if(enable_debug)
    {
      putchar(ch);
      fflush(stdout);
    }
}

