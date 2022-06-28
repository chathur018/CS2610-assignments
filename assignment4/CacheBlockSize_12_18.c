#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"
#include "emmintrin.h"

int main(){
	char arr[1024];
	FILE *fp = fopen("output.txt", "w"); //Store output into the file to make a plot

	
	srand(time(0));

	for (int i = 0; i < 1024; i++) {
		arr[i] = rand() % 26 + 'A';
	}
		
	char k;
	int timings[1024]; //Array to store output
	
	unsigned long flags;
	uint64_t start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;

	
	for(int i = 0; i < 32; i++) //Clear the cache line pointed to after every 32 bytes, assuming that each cache block size is at least 32B 
		_mm_clflush(arr + 32*i);
	

	
	asm volatile ("CPUID\n\t"
	"RDTSC\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
	"%rax", "%rbx", "%rcx", "%rdx");
	asm volatile("RDTSCP\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t"
	"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: "%rax",
	"%rbx", "%rcx", "%rdx");
	asm volatile ("CPUID\n\t"
	"RDTSC\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
	"%rax", "%rbx", "%rcx", "%rdx");
	asm volatile("RDTSCP\n\t"
	"mov %%edx, %0\n\t"
	"mov %%eax, %1\n\t"
	"CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1):: "%rax",
	"%rbx", "%rcx", "%rdx");
		
	for(int i=0; i<1024; i++){

		
		
		asm volatile ("CPUID\n\t"
		"RDTSC\n\t"
		"mov %%edx, %0\n\t""mov %%eax, %1\n\t": "=r" (cycles_high), "=r"
		(cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");
		
		k = arr[i];
		
		asm volatile("RDTSCP\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t"
		"CPUID\n\t": "=r" (cycles_high1), "=r"
		(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");


		start = ( ((uint64_t)cycles_high << 32) | cycles_low );

		end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );
		
		timings[i] = (end - start);
		printf("%d \t %u\n", i, timings[i]);
		fprintf(fp, "%u\n", timings[i]);
	}
	fclose(fp);
	
	return 0;
}
