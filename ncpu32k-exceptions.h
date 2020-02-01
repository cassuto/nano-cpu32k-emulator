#ifndef NCPU32K_EXCEPTIONS_H
#define NCPU32K_EXCEPTIONS_H

#define VECT_ERST           0x00000000
#define VECT_EINSN          0x00000004
#define VECT_EIRQ           0x00000008
#define VECT_ESYSCALL       0x0000000c
#define VECT_EBUS           0x00000010
#define VECT_EIPF           0x00000014
#define VECT_EDPF           0x00000018
#define VECT_EITM           0x0000001c
#define VECT_EDTM           0x00000020
#define VECT_EALGIN         0x00000024
#define VECT_EINT           0x00000028

#define ERSC_DEFAULT		0X0
#define ERSC_TSC_IRQ		0x1

#endif /* NCPU32K_EXCEPTIONS_H */
