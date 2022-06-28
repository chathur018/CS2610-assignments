if [ $# -ne 2 ]
then
	echo "Usage: bash compile.sh file.asm output_filename"
	exit
fi

# Note that the values of immediate operands must be in hexadecimal

# Also the L1 of JMP and BEQZ must be given as two separate hex digits
# Eg: if L1 is 0x9, it should be given as JMP 0 9
# Eg: if L1 is 0x1e, it should be given as JMP 1 e

cat $1 | \
	sed '
	s/\/\/.*//;
	/^[ \t]*$/d;
	s/,/ /g;
	s/[ \t]\+/ /g;
	s/R10/a/g;
	s/R11/b/g;
	s/R12/c/g;
	s/R13/d/g;
	s/R14/e/g;
	s/R15/f/g;
	s/R0/0/g;
	s/R1/1/g;
	s/R2/2/g;
	s/R3/3/g;
	s/R4/4/g;
	s/R5/5/g;
	s/R6/6/g;
	s/R7/7/g;
	s/R8/8/g;
	s/R9/9/g;
	s/\(INC .\)/\1 0 0/g;
	s/\(NOT . .\)/\1 0/g;
	s/\(JMP . .\)/\1 0/g;
	s/\(HLT\)/\1 0 0 0/g;
	s/STORE/9/g;
	s/ADD/0/g;
	s/SUB/1/g;
	s/MUL/2/g;
	s/INC/3/g;
	s/AND/4/g;
	s/OR/5/g;
	s/NOT/6/g;
	s/XOR/7/g;
	s/LOAD/8/g;
	s/JMP/a/g;
	s/BEQZ/b/g;
	s/HLT/f/g;
	s/\(.\) \(.\) \(.\) \(.\) \?/\1\2\n\3\4/
	' > $2

lines=`wc -l $2 | cut -d" " -f1`
while [ $lines -lt 256 ]
do
	echo 00 >> $2
	lines=$(($lines+1))
done
