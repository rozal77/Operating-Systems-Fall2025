/**
 * @file mmu.c
 * @brief Memory Management Unit Simulator
 * * This program simulates memory allocation (First Fit, Best Fit, Worst Fit),
 * deallocation, and coalescing (defragmentation) of memory blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "list.h"
#include "util.h"

/**
 * @brief Helper function to convert a string to uppercase in-place.
 */
void TOUPPER(char * arr){
  
    for(int i=0;i<strlen(arr);i++){
        arr[i] = toupper(arr[i]);
    }
}

/**
 * @brief Parses command line arguments and populates simulation parameters.
 * * @param args Command line arguments (File path, Policy)
 * @param input Array to store parsed input instructions
 * @param n Pointer to store total number of instructions
 * @param size Pointer to store partition size
 * @param policy Pointer to store selected policy (1=FIFO, 2=Best, 3=Worst)
 */
void get_input(char *args[], int input[][2], int *n, int *size, int *policy) 
{
    FILE *input_file = fopen(args[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Invalid filepath\n");
        fflush(stdout);
        exit(0);
    }

    parse_file(input_file, input, n, size);
  
    fclose(input_file);
  
    TOUPPER(args[2]);
  
    if((strcmp(args[2],"-F") == 0) || (strcmp(args[2],"-FIFO") == 0))
        *policy = 1;
    else if((strcmp(args[2],"-B") == 0) || (strcmp(args[2],"-BESTFIT") == 0))
        *policy = 2;
    else if((strcmp(args[2],"-W") == 0) || (strcmp(args[2],"-WORSTFIT") == 0))
        *policy = 3;
    else {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
       exit(1);
    }
        
}

/**
 * @brief Allocates memory for a specific PID.
 * * Finds a suitable block in the free list, assigns it to the PID, moves it to 
 * the allocated list, and handles fragmentation by returning unused memory 
 * back to the free list based on the policy.
 */
void allocate_memory(list_t * freelist, list_t * alloclist, int pid, int blocksize, int policy) {
  // 1. Check if a block of sufficient size exists
  if(!list_is_in_by_size(freelist, blocksize)){
    printf("Error: Not Enough Memory\n");
    return;
  }

  // 2. Find and remove the block from the Free List
  int index = list_get_index_of_by_Size(freelist, blocksize);
  block_t *blk = list_remove_at_index(freelist, index);

  // 3. Update block metadata
  blk->pid = pid;
  int original_end = blk->end;
  blk->end = blk->start + blocksize - 1;

  // 4. Add to Allocated List (sorted by address)
  list_add_ascending_by_address(alloclist, blk);

  // 5. Handle Fragmentation: If block is larger than needed, create a fragment
  if(blk->end < original_end){
        block_t *fragment = malloc(sizeof(block_t));
        fragment->pid = 0; // Free block
        fragment->start = blk->end + 1;
        fragment->end = original_end;

        // Add the fragment back to the Free List based on the policy
        if (policy == 1) {
            list_add_to_back(freelist, fragment); // First Fit (FIFO)
        } else if (policy == 2) {
            list_add_ascending_by_blocksize(freelist, fragment); // Best Fit
        } else if (policy == 3) {
            list_add_descending_by_blocksize(freelist, fragment); // Worst Fit
        }
  }
}

/**
 * @brief Deallocates memory for a specific PID.
 * * Removes the block from the allocated list and returns it to the free list
 * according to the active memory management policy.
 */
void deallocate_memory(list_t * alloclist, list_t * freelist, int pid, int policy) { 
    // 1. Check if the PID exists in the allocated list
    if (!list_is_in_by_pid(alloclist, pid)) {
        printf("Error: Can't locate Memory Used by PID: %d\n", pid);
        return;
    }
    
    // 2. Remove the block from the Allocated List
    int index = list_get_index_of_by_Pid(alloclist, pid);
    block_t *blk = list_remove_at_index(alloclist, index);

    // 3. Reset PID to 0 (Free)
    blk->pid = 0;

    // 4. Return block to the Free List based on policy
    if (policy == 1) {
        list_add_to_back(freelist, blk); // First Fit (FIFO)
    } else if (policy == 2) {
        list_add_ascending_by_blocksize(freelist, blk); // Best Fit
    } else if (policy == 3) {
        list_add_descending_by_blocksize(freelist, blk); // Worst Fit
    }
}

/**
 * @brief Coalesces (Defragments) the free memory list.
 * * Sorts the free list by physical address and then merges adjacent blocks.
 * Returns a pointer to the new, coalesced list.
 */
list_t* coalese_memory(list_t * list){
  list_t *temp_list = list_alloc();
  block_t *blk;
  
  // Step 1: Sort the list in ascending order by address
  while((blk = list_remove_from_front(list)) != NULL) {  
        list_add_ascending_by_address(temp_list, blk);
  }
  
  // Step 2: Combine physically adjacent blocks
  list_coalese_nodes(temp_list);
        
  return temp_list;
}

/**
 * @brief Utility function to print the contents of a memory list.
 */
void print_list(list_t * list, char * message){
    node_t *current = list->head;
    block_t *blk;
    int i = 0;
  
    printf("%s:\n", message);
  
    while(current != NULL){
        blk = current->blk;
        printf("Block %d:\t START: %d\t END: %d", i, blk->start, blk->end);
      
        if(blk->pid != 0)
            printf("\t PID: %d\n", blk->pid);
        else  
            printf("\n");
      
        current = current->next;
        i += 1;
    }
}

/* DO NOT MODIFY */
int main(int argc, char *argv[]) 
{
   int PARTITION_SIZE, inputdata[200][2], N = 0, Memory_Mgt_Policy;
  
   list_t *FREE_LIST = list_alloc();   // list that holds all free blocks (PID is always zero)
   list_t *ALLOC_LIST = list_alloc();  // list that holds all allocated blocks
   int i;
  
   if(argc != 3) {
       printf("usage: ./mmu <input file> -{F | B | W }  \n(F=FIFO | B=BESTFIT | W-WORSTFIT)\n");
       exit(1);
   }
  
   get_input(argv, inputdata, &N, &PARTITION_SIZE, &Memory_Mgt_Policy);
  
   // Allocated the initial partition of size PARTITION_SIZE
   
   block_t * partition = malloc(sizeof(block_t));   // create the partition meta data
   partition->start = 0;
   partition->end = PARTITION_SIZE + partition->start - 1;
                                   
   list_add_to_front(FREE_LIST, partition);          // add partition to free list
                                   
   for(i = 0; i < N; i++) // loop through all the input data and simulate a memory management policy
   {
       printf("************************\n");
       if(inputdata[i][0] != -99999 && inputdata[i][0] > 0) {
             printf("ALLOCATE: %d FROM PID: %d\n", inputdata[i][1], inputdata[i][0]);
             allocate_memory(FREE_LIST, ALLOC_LIST, inputdata[i][0], inputdata[i][1], Memory_Mgt_Policy);
       }
       else if (inputdata[i][0] != -99999 && inputdata[i][0] < 0) {
             printf("DEALLOCATE MEM: PID %d\n", abs(inputdata[i][0]));
             deallocate_memory(ALLOC_LIST, FREE_LIST, abs(inputdata[i][0]), Memory_Mgt_Policy);
       }
       else {
             printf("COALESCE/COMPACT\n");
             FREE_LIST = coalese_memory(FREE_LIST);
       }   
     
       printf("************************\n");
       print_list(FREE_LIST, "Free Memory");
       print_list(ALLOC_LIST,"\nAllocated Memory");
       printf("\n\n");
   }
  
   list_free(FREE_LIST);
   list_free(ALLOC_LIST);
  
   return 0;
}