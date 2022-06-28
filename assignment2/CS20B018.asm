%include "io.mac"

.DATA
	input_msg		db		"Please enter a string: ", 0
	terminate_msg	db		"Do you want to terminate? ", 0
.UDATA
	input_string	resb	21
.CODE
	.STARTUP
repeat:
	PutStr	input_msg			;diplay input request message
	GetStr	input_string,21		;read input string
	mov		EBX,input_string	;EBX = address of the string
encrypt:
	mov		AL, [EBX]			;move character to AL
	cmp		AL, 0				;check if it is NULL
	je		done				;end of string reached
	cmp		AL, 64				;to check if character is a number
	jg		next				;if char, jump to end
	and		AL, 0FH				;convert to integer
	cmp		AL, 4				;compare with 4
	jg		case2				;go to case2 if greater than
	add		AL, AL				;multipy by 2
	add		AL, 5				;add 5
	jmp		modulo				;jump to modulo part
case2:
	add		AL, AL				;multipy by 2
	add		AL, 6				;add 6
modulo:
	cmp		AL, 9				;compare with 9
	jle		replace				;go to replace if less than or equal
	sub		AL, 10				;subtract 10
	jmp		modulo
replace:
	add		AL, 48				;convert back to ASCII
	mov		[EBX], AL			;replace in the string
next:
	inc		EBX					;update EBX to point to next char
	jmp		encrypt				;encrypt next char
done:
	PutStr	input_string		;display the encrypted string
	nwln
	PutStr	terminate_msg		;display termination message
	GetCh	AL					;read the input char
	cmp		AL, 89				;check for 'Y'
	je		end
	cmp		AL, 121				;check for 'y'
	je		end
	jmp		repeat
end:
	.EXIT
