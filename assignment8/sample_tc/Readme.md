# Compiling the assembly program into ICache.txt

The script compile.sh will translate basic assembly instructions into ICache.txt file.
Note that all immediate values are given as single hex digits
Also the L1 of JMP and BEQZ must be given as two separate hex digits
Eg: if L1 is 0x9, it should be given as JMP 0 9
Eg: if L1 is 0x1e, it should be given as JMP 1 e

# Things to note
- Please strictly follow the output format as given in the output/DCache.txt and output/Output.txt files.
- Your program should read the input from the DCache.txt, ICache.txt and RF.txt files present in the current working directory and write the output data cache to ODCache.txt and statistics to Output.txt file (in these current working directory).
  Please follow these file name conventions.
- Submit a Readme file with instructions on how to compile and execute your program.
- HALT instruction is present in its own instruction class
- We continue counting the cycles until the HLT instruction passes the writeback stage
- The program starts at the beginning of ICache.txt so initially PC should be 0.
- The offset in BEQZ and JMP instruction should be in number of instructions (not number of bytes).
  Also the offset is from the instruction after the branch instruction, i.e. the 
  PC will point to the next instruction by the time the branch reaches execute stage
- The Instruction cache is read only and the address used is different for Icache and Dcache.
  i.e. address 0x2 of ICache and address 0x2 of DCache need not have same value
- All values in .txt files (except Output.txt) will be given as one byte per line, in hexadecimal (without 0x)
