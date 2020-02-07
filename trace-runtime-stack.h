#ifndef TRACE_RUNTIME_STACK_H_
#define TRACE_RUNTIME_STACK_H_

#define TRACE_CALL_STACK
/* #undef TRACE_CALL_STACK */
//#define TRACE_STACK_POINTER
/* #undef TRACE_STACK_POINTER */
//#define TRACE_EXCEPTION
/* #undef TRACE_EXCEPTION */

/* trace_* funcs are called by CPU only. */
#ifdef TRACE_CALL_STACK
extern void trace_call_stack_jmp(vm_addr_t insn_pc, vm_addr_t lnk_pc, vm_addr_t target_pc);
extern void trace_call_stack_return(vm_addr_t insn_pc, vm_addr_t target_pc);
#else
# define trace_call_stack_jmp(insn_pc,lnk_pc,target_pc)
# define trace_call_stack_return(insn_pc,target_pc)
#endif

#ifdef TRACE_STACK_POINTER
extern void trace_stack_pointer_ld(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp);
extern void trace_stack_pointer_mov(vm_addr_t pc, cpu_word_t sp);
extern void trace_stack_pointer_st(vm_addr_t pc, vm_addr_t addr, cpu_word_t sp);
#else
# define trace_stack_pointer_ld(pc, addr, sp)
# define trace_stack_pointer_mov(pc, sp)
# define trace_stack_pointer_st(pc, addr, sp)
#endif

extern void trace_print_frames();

#endif /* TRACE_RUNTIME_STACK_H_ */
