#ifndef DEBUG_H_
#define DEBUG_H_

#define VERBOSE 1

#if VERBOSE > 1
#define verbose_print(...) printf(__VA_ARGS__)
#else
#define verbose_print(...) ((void)0)
#endif
#if VERBOSE == 1
#define verbose_print_1(...) printf(__VA_ARGS__)
#else
#define verbose_print_1(...) ((void)0)
#endif

void debug_putc(uint8_t ch);

#endif /* DEBUG_H_ */