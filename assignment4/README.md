This folder contains the present README file, two C programs (CacheAssoc_12_18.c, CacheBlockSize_12_18.c), a text file (Assoc_opvalues_12_18.txt) containing outputs for the second (associativity) program and a report (Report_A4_12_18.pdf).

For best results, use isolcpus while booting Linux. This can be confirmed using the command $cat /proc/cmdline

Compile either C file with gcc. In order to run it, if isolcpus was used, run the following command: $taskset -c <x> <executable>
where <x> is the isolated CPU, for example if isolcpus=1 was used, <x> is 1
and <executable> is the executable for the C program, such as ./a.out
In case isolcpus was not used, the executable may be run directly.

The output is printed to the terminal. The program to find cache block size also writes to a file in order to make a plot using python, but these lines (9, 76, 78) may be commented out if not wanted.