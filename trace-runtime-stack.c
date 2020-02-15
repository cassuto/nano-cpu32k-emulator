/**@file Simple signal-thread call stack tracing
 * NOTE! This never works on a NON call/return-normal-form programs
 * and multitask systems.
 */
#include "cpu.h"
#include "debug.h"
#include "parse-symtable.h"
#include "trace-runtime-stack.h"

#define MAX_STACK_TRACE_DEPTH 8192

struct stack_info
{
  vm_addr_t insn_pc;
  vm_addr_t lnk_pc;
  vm_addr_t target_pc;
};

static int stack_depth;
static char stack_damaged;
static struct stack_info stack_traces[MAX_STACK_TRACE_DEPTH];

#ifdef TRACE_CALL_STACK
inline void
trace_call_stack_jmp(vm_addr_t insn_pc, vm_addr_t lnk_pc, vm_addr_t target_pc)
{
  if (stack_depth >= MAX_STACK_TRACE_DEPTH)
    {
      fprintf(stderr,"Stack Trace: out of MAX_STACK_TRACE_DEPTH.\n");
      panic(1);
    }
  stack_traces[stack_depth].insn_pc = insn_pc;
  stack_traces[stack_depth].lnk_pc = lnk_pc;
  stack_traces[stack_depth].target_pc = target_pc;
  ++stack_depth;
}

inline void
trace_call_stack_return(vm_addr_t insn_pc, vm_addr_t target_pc)
{
  --stack_depth;
  if (stack_depth < 0)
    {
      stack_damaged = 1;
      stack_depth = 0;
    }
}
#endif

/**
 * @brief Print the call stack
 */
void
trace_print_frames()
{
#ifdef TRACE_CALL_STACK
  if (stack_damaged)
    {
      verbose_print_1("Warning: Call stack damaged.\n");
    }
  for(int frame=0;frame<stack_depth;++frame)
    {
      const struct sym_node *sym = find_symbol(stack_traces[frame].target_pc);
      if(!sym)
        verbose_print_1("#%d (%#x): call %x, ret=%x\n", frame,
                        stack_traces[frame].insn_pc,
                        stack_traces[frame].target_pc,
                        stack_traces[frame].lnk_pc);
      else
        verbose_print_1("#%d (%#x): call %s(%x), ret=%x\n", frame,
                        stack_traces[frame].insn_pc,
                        sym->symbol,
                        stack_traces[frame].target_pc,
                        stack_traces[frame].lnk_pc);
    }
#else
  verbose_print_1("config TRACE_CALL_STACK is not enabled.");
#endif
}

#ifdef TRACE_STACK_POINTER
inline void
trace_stack_pointer_ld(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- &%#x(%#x)\n", pc, addr, sp);
}

inline void
trace_stack_pointer_mov(vm_addr_t pc, cpu_word_t sp)
{
  verbose_print_1("(%#x): sp <- %#x\n", pc, sp);
}

inline void
trace_stack_pointer_st(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp)
{
  verbose_print_1("(%#x): %#x <- sp(%#x)\n", pc, addr, sp);
}
#endif
