// useful assembler macros

#ifndef _MAMIGO_ASM_HELPER_H
#define _MAMIGO_ASM_HELPER_H

#define ASM_HEADER .text;\
		.align 4;


#define FUNC_DEF(f) .global f;\
		.type f, STT_FUNC;

#define LABEL(x) x:
#define FUNC_START(x) LABEL(x)
#if defined(__GNUC__)
#define CAT(x,y) x.y
#else
// VDSP preprocessor does not like the above
#define CAT(x,y) x##.##y
#endif
#define FUNC_END(x) LABEL(CAT(x,END))


#endif
