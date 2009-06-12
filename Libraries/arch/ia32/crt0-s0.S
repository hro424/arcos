	.text
	.global _start
	
_start:
0:	leal	_sigma0_stack_top, %esp
        call    _init
	pushl	$__return_from_main
	jmp	main
	
__return_from_main:
	int	$3
	jmp	1f
	.ascii	"Sigma0 stopped."
1:	jmp	__return_from_main

	.bss
	
	.align	16
_sigma0_stack:
	.space	1024
_sigma0_stack_top:
