
#include "ncpu32k-emu.h"
#include "ncpu32k-msr.h"

void wmsr(msr_index_t index, cpu_word_t val)
{
  switch(index)
  {
    case MSR_DBGR_NUMPORT:
      {
        char buff[64], *p = buff;
        snprintf(buff, sizeof(buff), "DEBUG NUM PORT - %#x\n", val);
        while(*p)
          debug_putc(*p++);
        break;
      }
      
    case MSR_DBGR_MSGPORT:
      debug_putc(val);
      break;
  }
    
}

cpu_word_t rmsr(msr_index_t index)
{
  return 0;
}
