%include "io.mac"

.DATA
	int_m_msg	db	"Please enter integer 'm': ",0
	int_n_msg	db	"Please enter integer 'n': ",0
.UDATA
.CODE
	.STARTUP
	PutStr	int_m_msg	;request input 'm'
	GetLInt	EBX			;read 'm'
	PutStr	int_n_msg	;request input 'n'
	GetLInt	ECX			;read 'n'
	push	EBX			;push 'm' and 'n' for calling procedure
	push	ECX
	call	ackermann	;call procedure
	PutLInt	EAX			;display result
	nwln
	.EXIT

;---------------------------------------------------------------
;Procedure ackermann receives two integers via the stack
;The Ackermann of the integers is returned in EAX
;---------------------------------------------------------------

ackermann:
	enter	0,0
	mov		EAX,[EBP+12];move 'm' into EAX
	cmp		EAX,0		;compare 'm' with 0
	jne		case2		;jump to case2 if not equal
	mov		EAX,[EBP+8]
	add		EAX,1		;move 'n+1' into EAX	
	leave
	ret		8			;return and clear parameters
case2:
	mov		EAX,[EBP+8]	;move 'n' into EAX
	cmp		EAX,0		;compare 'n' with 0
	jne		case3		;jump to case3 if not equal
	mov		EAX,[EBP+12]
	sub		EAX,1		;move 'm' into EAX
	push	EAX			;push 'm' and '1' for calling procedure
	push	1
	call	ackermann	;call procedure
	leave
	ret		8			;return and clear parameters
case3:
	mov		EAX,[EBP+12]
	sub		EAX,1		;move 'm' into EAX
	push	EAX			;push 'm' for outer procedure
	mov		EAX,[EBP+12];move 'm+1' into EAX
	push	EAX			;push 'm+1' for inner procedure
	mov		EAX,[EBP+8]
	sub		EAX,1		;move 'n' into EAX
	push	EAX			;push 'n' for inner procedure
	call	ackermann	;call inner procedure
	push	EAX			;push 'A(m+1,n)' for outer procedure
	call	ackermann	;call outer procedure
	leave
	ret		8			;return and clear parameters

