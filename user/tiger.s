.global	_start
.text
_start:
	mv       t6, fp               # save frame pointer
	mv       fp, sp
	addi     sp, sp, -12
	nop
	sw       t6, -8(fp)           # store spilled reg T150 to stack
	sw       ra, -4(fp)           # store spilled reg T151 to stack
L1:
	li       t6, 1                # MOVE(TEMP(i),CONST(c)) T117 <- 1
	mv       a0, t6               # copy arg to reg
	call     printInt
	j        L0                   # JUMP(LABEL(t)) L0
L0:
	lw       ra, -4(fp)           # fetch spilled reg T151 from stack to T164
	lw       fp, -8(fp)           # fetch spilled reg T150 from stack to T166
	nop
	addi     sp, sp, 12           # restore stack pointer
	ret

