#include <stdio.h>
#include "utlist.h"
#include "utils.h"

#include "memory_controller.h"
#include "params.h"

extern long long int CYCLE_VAL;

/* A data structure to see if a bank is a candidate for precharge. */
int recent_colacc[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS];

/* Keeping track of how many preemptive precharges are performed. */
long long int num_aggr_precharge = 0;

short int counter;
long long int num_counter_changes = 0;
long long int close_to_open = 0; //Number of times policy switches from close page to open page
long long int open_to_close = 0; //Number of times policy switches from open page to close page
short int LO_TH, HI_TH;
int is_open; //Current policy
int last_closed[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS]; //Banks which were last precharged (to check page hit in close page policy)
long long int prev_row[MAX_NUM_CHANNELS][MAX_NUM_RANKS][MAX_NUM_BANKS]; //Row which was previously activated in open page, to check for page miss

void init_scheduler_vars()
{
	// initialize all scheduler variables here
	int i, j, k;
	for (i=0; i<MAX_NUM_CHANNELS; i++) {
	  for (j=0; j<MAX_NUM_RANKS; j++) {
	    for (k=0; k<MAX_NUM_BANKS; k++) {
	      recent_colacc[i][j][k] = 0;
	      last_closed[i][j][k] = 0;
	      prev_row[i][j][k] = -1; //No row active initially
	    }
	  }
	}
	
	counter = 7;
	LO_TH = 5;
	HI_TH = 10;
	is_open = 1; //Start with open-page policy

	return;
}

#define HI_WM 40

#define LO_WM 20

int drain_writes[MAX_NUM_CHANNELS];

void schedule(int channel)
{
	if(counter > HI_TH) //If counter rises past high threshold, switch to close page policy
	{
		is_open = 0;
		open_to_close++;
	}
	if(counter < LO_TH) //If counter dips below low threshold, switch to open page policy
	{
		is_open = 1;
		close_to_open++;
	}
	if(is_open) //Open-page policy
	{
		request_t * rd_ptr = NULL;
		request_t * wr_ptr = NULL;


		// if in write drain mode, keep draining writes until the
		// write queue occupancy drops to LO_WM
		if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
		  drain_writes[channel] = 1; // Keep draining.
		}
		else {
		  drain_writes[channel] = 0; // No need to drain.
		}

		// initiate write drain if either the write queue occupancy
		// has reached the HI_WM , OR, if there are no pending read
		// requests
		if(write_queue_length[channel] > HI_WM)
		{
			drain_writes[channel] = 1;
		}
		else {
		  if (!read_queue_length[channel])
		    drain_writes[channel] = 1;
		}


		// If in write drain mode, look through all the write queue
		// elements (already arranged in the order of arrival), and
		// issue the command for the first request that is ready
		if(drain_writes[channel])
		{

			LL_FOREACH(write_queue_head[channel], wr_ptr)
			{
				if(wr_ptr->command_issuable)
				{
					int channel = wr_ptr->dram_addr.channel;
					int rank = wr_ptr->dram_addr.rank;
					int bank = wr_ptr->dram_addr.bank;
					long long int row = wr_ptr->dram_addr.row;
					command_t cmd = wr_ptr->next_command;
					if(cmd == COL_WRITE_CMD)
					{
						if(prev_row[channel][rank][bank] != row) //Page miss
						{
							counter++;
							num_counter_changes++;
						}
						prev_row[channel][rank][bank] = row;
					}
					issue_request_command(wr_ptr);
					break;
				}
			}

		}

		// Draining Reads
		// look through the queue and find the first request whose
		// command can be issued in this cycle and issue it 
		// Simple FCFS 
		if(!drain_writes[channel])
		{
			LL_FOREACH(read_queue_head[channel],rd_ptr)
			{
				if(rd_ptr->command_issuable)
				{
					int channel = rd_ptr->dram_addr.channel;
					int rank = rd_ptr->dram_addr.rank;
					int bank = rd_ptr->dram_addr.bank;
					long long int row = rd_ptr->dram_addr.row;
					command_t cmd = rd_ptr->next_command;
					if(cmd == COL_WRITE_CMD)
					{
						if(prev_row[channel][rank][bank] != row) //Page miss
						{
							counter++;
							num_counter_changes++;
						}
						prev_row[channel][rank][bank] = row;
					}
					issue_request_command(rd_ptr);
					break;
				}
			}
		}
		if(counter > 15) //Saturation
			counter = 15;
	}
	
	else //Close-page policy
	{
		request_t * rd_ptr = NULL;
		request_t * wr_ptr = NULL;
		int i, j;


		// if in write drain mode, keep draining writes until the
		// write queue occupancy drops to LO_WM
		if (drain_writes[channel] && (write_queue_length[channel] > LO_WM)) {
		  drain_writes[channel] = 1; // Keep draining.
		}
		else {
		  drain_writes[channel] = 0; // No need to drain.
		}

		// initiate write drain if either the write queue occupancy
		// has reached the HI_WM , OR, if there are no pending read
		// requests
		if(write_queue_length[channel] > HI_WM)
		{
			drain_writes[channel] = 1;
		}
		else {
		  if (!read_queue_length[channel])
		    drain_writes[channel] = 1;
		}


		// If in write drain mode, look through all the write queue
		// elements (already arranged in the order of arrival), and
		// issue the command for the first request that is ready
		if(drain_writes[channel])
		{

			LL_FOREACH(write_queue_head[channel], wr_ptr)
			{
				if(wr_ptr->command_issuable)
				{
					/* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
					   If the bank just did an activate or precharge, it is not a candidate for closure. */
					if (wr_ptr->next_command == COL_WRITE_CMD) {
					  recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 1;
					  if(last_closed[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank]) //Recently closed, hit
					  {
					  	counter--;
					  	num_counter_changes++;
					  	last_closed[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
					  }					  
					}
					if (wr_ptr->next_command == ACT_CMD) {
					  recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
					}
					if (wr_ptr->next_command == PRE_CMD) {
					  recent_colacc[channel][wr_ptr->dram_addr.rank][wr_ptr->dram_addr.bank] = 0;
					}
					issue_request_command(wr_ptr);
					break;
				}
			}
		}

		// Draining Reads
		// look through the queue and find the first request whose
		// command can be issued in this cycle and issue it 
		// Simple FCFS 
		if(!drain_writes[channel])
		{
			LL_FOREACH(read_queue_head[channel],rd_ptr)
			{
				if(rd_ptr->command_issuable)
				{
					/* Before issuing the command, see if this bank is now a candidate for closure (if it just did a column-rd/wr).
					   If the bank just did an activate or precharge, it is not a candidate for closure. */
					if (rd_ptr->next_command == COL_READ_CMD) {
					  recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 1;
					  if(last_closed[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank]) //Recently closed, hit
					  {
					  	counter--;
					  	num_counter_changes++;
					  	last_closed[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
					  }	
					}
					if (rd_ptr->next_command == ACT_CMD) {
					  recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
					}
					if (rd_ptr->next_command == PRE_CMD) {
					  recent_colacc[channel][rd_ptr->dram_addr.rank][rd_ptr->dram_addr.bank] = 0;
					}
					issue_request_command(rd_ptr);
					break;
				}
			}
		}

		/* If a command hasn't yet been issued to this channel in this cycle, issue a precharge. */
		if (!command_issued_current_cycle[channel]) {
		  for (i=0; i<NUM_RANKS; i++) {
		    for (j=0; j<NUM_BANKS; j++) {  /* For all banks on the channel.. */
		      last_closed[channel][i][j] = recent_colacc[channel][i][j]; //Copy recent_colacc into last_closed since last_closed, before precharging, contains exactly the information we need
		      if (recent_colacc[channel][i][j]) {  /* See if this bank is a candidate. */
			if (is_precharge_allowed(channel,i,j)) {  /* See if precharge is doable. */
			  if (issue_precharge_command(channel,i,j)) {
			    num_aggr_precharge++;
			    recent_colacc[channel][i][j] = 0;
			  }
			}
		      }
		    }
		  }
		}
		if(counter < 0)
			counter = 0;
	}
}

void scheduler_stats()
{
  /* Nothing to print for now. */
  printf("Number of aggressive precharges: %lld\n", num_aggr_precharge);
  printf("Number of counter changes: %lld\n", num_counter_changes);
  printf("Open to close changes: %lld\n", open_to_close);
  printf("Close to open changes: %lld\n", close_to_open);
}

