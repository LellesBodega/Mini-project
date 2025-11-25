# timetemplate.S
# Written 2015 by F Lundevall
# Ported 2024/06 by W Szczerek (from MIPS to RISC-V)
# Copyright abandonded - this file is in the public domain.

#############################################################
# Choose the macro syntax for RARS or DTEK-V board.         #
# The syntax for RARS is probably due to its MIPS heritage. #
#############################################################
###################
# Macros for RARS #
###################
#.macro	PUSH (%reg)
#	addi	sp,sp,-4
#	sw	%reg,0(sp) 
#.end_macro
#
#.macro	POP (%reg)
#	lw	%reg,0(sp)
#	addi	sp,sp,4
#.end_macro
###################
# Macros for DTEK #
###################
.macro	PUSH reg
	addi sp,sp,-4
	sw \reg,0(sp) 
.endm

.macro	POP reg
	lw	\reg,0(sp)
	addi	sp,sp,4
.endm
#############################################################

	.data
	.align 2
mytime:	.word 	0x5957
timstr:	.asciz 	"text more text lots of text\0"
	.text
	.global timetemplate, tick, time2string, delay, display_string

# Function for displaying a string with a newline at the end	
display_string:	
	li a7,4
	ecall
	li a0, 10
	li a7,11
	ecall
	jr ra
	
timetemplate:
	la	a0, timstr
	jal     display_string
	
	# wait a little
	li	a0, 5000		# ms
	jal	delay
	
	# call tick
	la 	a0, mytime
	jal	tick
	
	# call your function time2string
	la	a0, timstr
	la	t0, mytime
	lw	a1, 0(t0)
	jal	time2string
	
	# go back and do it all again
	j	timetemplate

	
# tick: update time pointed to by $a0
tick:	lw	t0, 0(a0)	# get time
	addi	t0, t0, 1	# increase
	andi	t1, t0, 0xf	# check lowest digit
	sltiu	t2, t1, 0xa	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x6	# adjust lowest digit
	
	andi	t1, t0, 0xf0	# check next digit
	sltiu	t2, t1, 0x60	# if digit < 6, okay
	bnez	t2, tiend
	addi	t0, t0, 0xa0	# adjust digit
	
	li	t3, 0xF
	slli	t3, t3, 0x8
	and	t1, t0, t3	# check minute digit
	addi	t3, x0, 0xA
	slli	t3, t3, 0x8
	slt	t2, t1, t3	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x600	# adjust digit - this one's okay, it's lower than 0x7FF 
	
	li	t3, 0xF
	slli	t3, t3, 0xC
	and	t1, t0, t3	# check last digit
	addi	t3, x0, 0x6
	slli	t3, t3, 0xC
	slt	t2, t1, t3	# if digit < 6, okay
	bnez	t2, tiend
	
	li	t3, 0xA
	slli	t3, t3, 0xC
	add	t0, t0, t3	# adjust last digit
tiend:	sw	t0,0(a0)	# save updated result
	jr	ra		# return

#########################################################
# Place for your functions: time2string, hex2asc, delay.#
#########################################################

time2string:
	# Input: a0 (address), a1 (time/info 16 lsb)
	# Output: None
	
	# Required action, (6 char by a0)
		# ASCII digits -> input parameter minutes
		# Colon character (ASCII = 0x3A)
		# ASCII digits -> input parameter seconds 
		# Null byte (0x00)
	
	# push
	addi sp, sp, -12
	sw ra, 0(sp) 		# Save return address
	sw s0, 4(sp)
	sw s1, 8(sp)
	
	# copy
	add s0, a0,zero		# s0 = base pointer
	add s1, a1, zero	# s1 = time info NBCD-digits
	
	#SURPRISE
	srli t0, s1, 0
	andi t0, t0, 0xFF
	srli t1, s1, 8
	andi t1, t1, 0xFF
	add t0, t0, t1
	beq t0, zero, hourbranch
	
	# minutes tens & ones 
	srli t0, s1, 12		# shift 12 bits
	andi t0, t0, 0xF	# isolate 1 nibble (minute tens)
	add a0, t0, zero	# copy to a0

	jal hexasc		# call hexasc - convert to ASCII (for printing)
	sb a0, 0(s0)		# store a0 value to pointer t0, offset memory[t0]
	addi s0, s0, 1		# increment pointer to next free bit 
	
	srli t0, s1, 8		# shift 8 bits
	andi t0, t0, 0xF	# isolate 1 nibble (minute ones)
	add a0, t0, zero	

	jal hexasc		
	sb a0, 0(s0)		
	addi s0, s0, 1
	
	# add colon 
	li a0, 0x3A		# add ":"
	sb a0, 0(s0)
	addi s0, s0, 1
	
	# seconds tens & ones
	srli t0, s1, 4		#00:x0
	andi t0, t0, 0xF
	add a0, t0, zero
	
	jal hexasc
	sb a0, 0(s0)
	addi s0, s0, 1
	
	srli t0, s1, 0		# shift 0 bits 00:0x
	andi t0, t0, 0xF
	add a0, t0, zero
	
	jal hexasc
	sb a0, 0(s0)
	addi s0, s0, 1
	
	# Null operator
	li a0, 0x00
	sb a0, 0(s0)
	
	j pop
	
hourbranch:
	li a0, 0x48
	sb a0, 0(s0)
	addi s0, s0, 1
	
	li a0, 0x4F
	sb a0, 0(s0)
	addi s0, s0, 1

	li a0, 0x55
	sb a0, 0(s0)
	addi s0, s0, 1
	
	li a0, 0x52
	sb a0, 0(s0)
	addi s0, s0, 1
	
	#Null operator
	li a0, 0x00
	sb a0, 0(s0)
	
	j pop
	
pop:
	# pop 
	lw ra, 0(sp)
	lw s0, 4(sp)
	lw s1, 8(sp)
	addi sp, sp, 12
	
	jr ra

delay: 
	# input = a0 (ms)
	# output = none 
	
	#push 
	addi sp, sp, -12
	sw ra, 0(sp) #return address
	sw s0, 4(sp) # ms
	sw s1, 8(sp) # i
	
	add s0, a0, zero	#save ms - add not and
	addi s1, zero, 0	#save i
	
	whileloop:
		ble s0, zero, exit    #branch is less or equal to 0 to forloop
		addi s0, s0, -1

	forloop:
		li t0, 1
		bge s1, t0, whileloop
		addi s1, s1, 1
		j forloop
		
	exit:
		# pop 
		lw ra, 0(sp)
		lw s0, 4(sp)
		lw s1, 8(sp)
		addi sp, sp, 12
		jr ra

# Copied from file
hex2asc:
	li	a0, 10		# test number (from 0 to 15)
	
	addi    sp,sp,-4
	sw      ra,0(sp)
	
	jal	hexasc		# call hexasc
	
	li	a7, 11	# write a0 to stdout
	ecall

	lw      ra,0(sp)
	addi    sp,sp,4
	jr      ra	

  # You can write your own code for hexasc here
hexasc:	
	# Input a0: 0-15 (4 lsb)
	# Output a0: ASCII (30 to 39) & (41 to  46) (7 lsb)
	
	andi a0, a0, 0xF 	# use the lowest 4 bits
	
	li t0, 10		# load to t-register
	blt a0, t0, digit	# branch if less than 10 
	
	addi a0, a0, 55		# If A-F (10+55=65)
	andi a0, a0, 0x7F	# use the 7 lsb
	jr ra
	
digit: 
	addi a0, a0, 48
	andi a0, a0, 0x7F
	jr ra
