%include "io.mac"

.DATA
	file_name	db		"input.txt",0
	get_num		db		"Enter the number of integers",0
	get_ints	db		"Enter the integers", 0

.UDATA
	num_int		resd	1
	in_val		resd	1
	fd_in		resd	1
	map_add		resd	1
	arr_int		resd	1024
	
.CODE
	.STARTUP
	mov		EAX,8						;Create and open the file
	mov		EBX,file_name	
	mov		ECX,0o777		
	int		0x80			
	mov		[fd_in],EAX					;Save the file descriptor
	PutStr	get_num
	nwln
	GetLInt	[num_int]					;The number of integers to write to the file
	mov		ECX,[num_int]	
	cmp		ECX,0			
	je		read_done		
	PutStr	get_ints
	nwln
repeat1:
	GetLInt	[in_val]					;Read the integer
	pusha
	pushf
	mov		EAX,4						;Write to the file
	mov		EBX,[fd_in]		
	mov		ECX,in_val		
	mov		EDX,4			
	int		0x80			
	popf
	popa
	loop	repeat1
read_done:
	mov		EAX,6			
	mov		EBX,[fd_in]		
	int		0x80			

	mov		EAX,5			
	mov		EBX,file_name	
	mov		ECX,0			
	mov		EDX,0o777		
	int		0x80			
	mov		[fd_in],EAX		
	
	mov		EBX,0		    			;address
    mov		EDX,0x1		    			;protection
    mov		ESI,0x2		    			;flags
    mov		EDI,[fd_in]					;file descriptor
    mov		ECX,4096      				;size
    mov		EBP,0		    			;offset
    mov		EAX,192		    			;mmap2
    int		0x80		    
    mov		[map_add],EAX	
    
	mov		EAX,2						;fork
	int		0x80			
	cmp		EAX,0						;EAX is zero for the child process
	je		sum				
    
    mov		EBX,arr_int					;parent process (insertion sort) begins
    mov		ECX,0						;the outer loop iterator for the insertion sort (array index where we are traversing)
get_val:
	mov		EAX,ECX						;EAX is the inner loop iterator for insertion sort (insertion position)
find_pos:
	mov		EDX,[map_add]				;Get the integer from the mmap-ed file
	mov		EDX,[EDX + 4*ECX]
	cmp		EAX,0						;Our current value is the smallest as it has reached index 0, so insert there
	je		insert
	cmp		EDX,[EBX + 4*(EAX - 1)]		;Insertion condition
	jge		insert					
	mov		EDX,[EBX + 4*(EAX - 1)]		;Shift the element forward to make space for the element we need to insert
	mov		[EBX + 4*EAX], EDX
	dec		EAX							;We move backward trying to find our position
	jmp		find_pos
insert:
	mov		[EBX + 4*EAX],EDX			;Place the element in the correct position
	inc		ECX
	cmp		ECX,[num_int]				;We repeat this for each index in the array
	jl		get_val
	pusha
	pushf
	mov		EAX, 7						;System call wait PID
	mov		EBX, -1						;Waits for all child processes to complete
	mov		ECX, 0
	mov		EDX, 0
	int 	0x80
	popf
	popa
	mov		EAX, 0
print_arr:
	PutLInt [EBX + 4*EAX]
    nwln
	inc		EAX
	loop	print_arr
	jmp		exit
    
sum:
	mov		EAX,0			
	mov		EBX,[map_add]	
	mov		ECX,[num_int]	
repeat2:
	add		EAX,[EBX]					;Get the value, add, and move to the next position in the mapped file
	add		EBX,4			
	loop	repeat2			
	PutLInt	EAX				
	nwln

exit:	
	.EXIT
