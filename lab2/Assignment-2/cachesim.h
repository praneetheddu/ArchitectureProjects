#ifndef __CACHESIM_H
#define __CACHESIM_H
typedef unsigned long long addr_t;
typedef unsigned long long counter_t;

void cachesim_init(int block_size, int cache_size, int ways);
void cachesim_access(addr_t physical_add, int write);

void cachesim_print_stats(void);

//Linked List Implementation
// typedef struct _LinkedList LinkedList;
// LinkedList* create_list();
// addr_t return_LRU(LinkedList* list);
// void add_to_tail(LinkedList* list, int data);
#endif
