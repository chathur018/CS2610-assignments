%include "io.mac"

.DATA
.UDATA
	number		resd	1		;will store the no. of integers
.CODE
	.STARTUP
	GetLInt		[number]		;get no. of integers
	mov			ECX,[number]	;move it into ECX register
	sub			EDX,EDX			;clear EDX to store the sum
repeat1:
	GetLInt		EAX				;get next integer
	add			EDX,EAX			;add it to current sum
	loop		repeat1

	PutLInt		[number]		;display no. of integers
	nwln
	PutLInt		EDX				;display sum of integers
	nwln
	.EXIT
