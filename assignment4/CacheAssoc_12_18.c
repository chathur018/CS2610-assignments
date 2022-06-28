#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"
#include "emmintrin.h"

#define SIZE 65536 //Array sized more than 32KB so that there will be a miss in the set we access

int main(){
	char arr[SIZE];


	srand(time(0));

	for (int i = 0; i < SIZE; i++) {
		arr[i] = rand() % 26 + 'A';
	}
		
	char k;
	int timings;
	
	
	unsigned long flags;
	uint64_t start, end;
	unsigned cycles_low, cycles_high, cycles_low1, cycles_high1;
	
	for(int i = 0; i < SIZE/32; i++)
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
		
	for(int i=0; i<=8; i++){
		k = arr[i*4096]; //4096 is cache block size (64B) * number of sets (64)
		
		
		for(int j = 0; j <= i*4096; j+=4096) { //To confirm associativity is 8, we'll need at least 9 accesses
		
			asm volatile ("CPUID\n\t"
			"RDTSC\n\t"
			"mov %%edx, %0\n\t""mov %%eax, %1\n\t": "=r" (cycles_high), "=r"
			(cycles_low):: "%rax", "%rbx", "%rcx", "%rdx");
			
			
			k = arr[j];

			
			
			asm volatile("RDTSCP\n\t"
			"mov %%edx, %0\n\t"
			"mov %%eax, %1\n\t"
			"CPUID\n\t": "=r" (cycles_high1), "=r"
			(cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");
			


			start = ( ((uint64_t)cycles_high << 32) | cycles_low );

			end = ( ((uint64_t)cycles_high1 << 32) | cycles_low1 );
			
			timings = (end - start);
			printf("%d %d \t %u\n", i, j/4096, timings);
		}
	}
	
	return 0;
}
