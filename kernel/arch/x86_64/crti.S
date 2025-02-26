.section .init
.global _init
.type _init, @function
.code64
_init:
	pushq %rbp
	movq %rsp, %rbp
	/* gcc will nicely put the contents of crtbegin.o's .init section here. */

.section .fini
.global _fini
.type _fini, @function
.code64
_fini:
	pushq %rbp
	movq %rsp, %rbp
	/* gcc will nicely put the contents of crtbegin.o's .fini section here. */
