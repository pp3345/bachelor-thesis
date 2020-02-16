#include <asm/unwind_hints.h>
#include <asm/smap.h>
#include <asm/irqflags.h>
#include <linux/linkage.h>

.code64
.section .text, "ax"

// roughly matches .macro idtentry from arch/x86/entry/entry_64.S
// requires modified kernel with EXPORT_SYMBOL(error_entry) and EXPORT_SYMBOL(error_exit) in entry_64.S
ENTRY(reclocking_analysis_interrupt)
	UNWIND_HINT_IRET_REGS offset=0
	ASM_CLAC
	pushq $-1
	call error_entry
	UNWIND_HINT_REGS
	movq %rsp, %rdi
	xorl %esi, %esi
	call do_reclocking_analysis_interrupt
	jmp error_exit
	_ASM_NOKPROBE(reclocking_analysis_interrupt)
END(reclocking_analysis_interrupt)

