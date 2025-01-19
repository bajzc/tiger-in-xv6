.global	_start
.text
_start:
	mv       t6, fp               # save frame pointer
	mv       fp, sp
	addi     sp, sp, -12
	nop
	sw       t6, -8(fp)           # store spilled reg T160 to stack
	sw       ra, -4(fp)           # store spilled reg T161 to stack
L4:
	li       t5, 1                # MOVE(TEMP(i),CONST(c)) T117 <- 1
	li       t6, 1                # MOVE(TEMP(i),CONST(c)) T148 <- 1
L1:
	li       t4, 10               # load CONST(i) to T150
	ble      t6, t4, L2           # compare e1 e2, true 'L2', false 'L0'
	j        L0                   # jump to false
L0:
	mv       a0, t5               # copy arg to reg
	call     printInt
	j        L3                   # JUMP(LABEL(t)) L3
L2:
	mul      t5, t5, t6           # T155 <- T117 * T148
	addi     t6, t6, 1
	j        L1                   # JUMP(LABEL(t)) L1
L3:
	lw       ra, -4(fp)           # fetch spilled reg T161 from stack to T174
	lw       fp, -8(fp)           # fetch spilled reg T160 from stack to T176
	nop
	addi     sp, sp, 12           # restore stack pointer
	ret


