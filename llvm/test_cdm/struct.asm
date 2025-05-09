### SECTIONN: .text
rsect _src_struct_c_16807



macro movens/2
push $1
pop $2
mend

print>                                  # -- Begin function print
# %bb.0:                                # %entry
	push	fp
	ldsp	fp
	addsp	-8
	ssw	r4, -2                          # 2-byte Folded Spill
	movens	r1, r2
	movens	r0, r3
	shr	r3, r4, 8
	ldi	r0, -4
	add	r0, fp, r0
	ldi	r1, 1
	stb	r0, r1, r4
	ssb	r3, -4
	ssw	r2, -6
	ldb	r0, r1, r0
	shl	r0, r0, 8
	lsb	r1, -4
	or r0, r1, r0
	ldw	r0, r0
	ssw	r0, -8
	ldi	r0, 0
	lsw	r4, -2                          # 2-byte Folded Reload
	addsp	8
	pop	fp
	rts
__Lfunc_end0:
                                        # -- End function
main>                                   # -- Begin function main
# %bb.0:                                # %entry
	push	fp
	ldsp	fp
	addsp	-14
	ldi	r0, __L__const.main.s+12
	ldw	r0, r1
	ldi	r0, -14
	add	r0, fp, r0
	ldi	r2, 12
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s+10
	ldw	r1, r1
	ldi	r2, 10
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s+8
	ldw	r1, r1
	ldi	r2, 8
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s+6
	ldw	r1, r1
	ldi	r2, 6
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s+4
	ldw	r1, r1
	ldi	r2, 4
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s+2
	ldw	r1, r1
	ldi	r2, 2
	stw	r0, r2, r1
	ldi	r1, __L__const.main.s
	ldw	r1, r1
	ssw	r1, -14
	ldi	r1, 14
	jsr	print
	ldi	r0, 0
	addsp	14
	pop	fp
	rts
__Lfunc_end1:
                                        # -- End function
### SECTIONN: .rodata
__L__const.main.s:                      # @__const.main.s
	dc	1337                            # 0x539
	db	70                              # 0x46
	db	115
	db	97
	db	114
	db	101
	db	103
	db	0
	db	0
	db	0
	db	0
	db	0
	ds	1

end.
