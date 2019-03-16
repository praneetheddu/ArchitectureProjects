//Praneeth Eddu
//Cache Simulator
//ECE 3056

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachesim.h"


//define struct for cache charecteristics
typedef struct {
    int valid, dirty, tag, LRU;
} _bits;

//define struct for number of sets accessing
typedef struct {
    _bits* arr_sets;
} _n_set;

//counters
counter_t accesses = 0, hits = 0, misses = 0, writebacks = 0;

//global defines
int ind; //index bits
int offset; //offset bits
int tag; //tag bits
int  num_sets; //number of sets
int nway; // global ways
int i;
int j;
_n_set* n_set;


void cachesim_init(int blocksize, int cachesize, int ways) {
    nway = ways;
    int temp = blocksize;

    while (temp!= 1) {
        temp/=2;
        offset++; //offset bits calculation
    }
    temp = (cachesize)/ ((2 << (offset -1)) * nway);

    while (temp != 1) {
        temp/=2;
        ind++; //index bits calculation
    }

    tag = 32 - ind - offset; //remaining bits are the tag bits
    num_sets = (2 << (ind-1)); //num sets is 2 ^ index

    //Allocating array of structs
    n_set = (_n_set*)malloc(sizeof(_n_set) * num_sets);  //allocate using number of sets
    for (i = 0; i < num_sets; i++) {
        n_set[i].arr_sets = (_bits*)malloc(sizeof(_bits) * ways); //allocate using associativity
        for (j = 0; j < ways; j++) {
            //instatiations
            n_set[i].arr_sets[j].valid = 0;
            n_set[i].arr_sets[j].dirty = 0;
            n_set[i].arr_sets[j].LRU = 0;
            n_set[i].arr_sets[j].tag = 0;
        }
    }
}

void cachesim_access(addr_t physical_addr, int write) {
    ++accesses;
     int check = 0;

    int indx, c_tag = 0;

    //Calculating tag mask
    int tagMask = ((1 << (tag))-1) << (offset + ind);
    int indexMask = ((1 << (ind))-1) << (offset);

    //Getting the right order of bits from physical address
    indx = indexMask & physical_addr;
    c_tag = tagMask & physical_addr;

    //Shifting to get the final index and tag
    indx = indx >> (offset);
    c_tag = c_tag >> (offset + ind);


    for (i = 0; i < nway; i++) {
         n_set[indx].arr_sets[i].LRU++; //LRU counter increments on every access
    }


    int max_LRU = n_set[indx].arr_sets[0].LRU;
    int max_LRU_ind = 0;
    for (i = 0; i < nway; i++) {
        //Check to see if tag matches, if match, hits increment and LRU for that index is set to 0
        if (n_set[indx].arr_sets[i].tag == c_tag && n_set[indx].arr_sets[i].valid == 1) {
            hits++;
            n_set[indx].arr_sets[i].LRU = 0;
            check = 2;
            break;
            //if valid is low, misses increment and LRU set to zero
        } else if (n_set[indx].arr_sets[i].valid == 0) {
            if (write == 1) {
                n_set[indx].arr_sets[i].dirty = 1; //Set dirty to high if its a write
            }
            misses++;
            n_set[indx].arr_sets[i].valid = 1;
            n_set[indx].arr_sets[i].LRU = 0;
            n_set[indx].arr_sets[i].tag = c_tag;
            check = 1;
            break;
        }
     }

     //If cache is full
     if (check == 0) {
        misses++;
        for (i = 0; i < nway; i++) {
            if (max_LRU < n_set[indx].arr_sets[i].LRU) {
                max_LRU = n_set[indx].arr_sets[i].LRU; //get the LRU element
                max_LRU_ind = i; //LRU index
            }
         }
        for (i = 0; i < nway; i++) {
            n_set[indx].arr_sets[i].LRU++; //LRU increment
        }

        if (n_set[indx].arr_sets[max_LRU_ind].dirty == 1) {
            writebacks++; //writeback if dirty is high
        }

        if (write == 1) {
            n_set[indx].arr_sets[max_LRU_ind].dirty = 1; //if write miss access, dirty is high
        }

        //new overriden tag and defaults
        n_set[indx].arr_sets[max_LRU_ind].tag = c_tag;
        n_set[indx].arr_sets[max_LRU_ind].LRU = 0;
        n_set[indx].arr_sets[max_LRU_ind].valid = 1;
     }
}

void cachesim_print_stats() {
  printf("%llu, %llu, %llu, %llu\n, %d , %d\n", accesses, hits, misses, writebacks);
}
