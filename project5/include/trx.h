#pragma once

#include "uthash.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef uint64_t pagenum_t;
//int global_trx_id;

typedef struct val_t{ 
    char val[120];
    struct val_t* next;
    pagenum_t pnum;
    int tid;
    int index;
}val_t;
typedef struct keys_t{
	int table_id;
	int64_t record_id;
}keys_t;
typedef struct lock_t lock_t;
typedef struct node_t{
	keys_t key;
	char record[120];
	lock_t* tail;
	lock_t* head;
	UT_hash_handle hh;	//uthash
}node_t;


struct lock_t{
	lock_t* prev;
	lock_t* next;
	node_t* sent;
	pthread_cond_t cond;

	int lock_mode;
	lock_t* trx_next;
	int trx_id;
	/* NO PAIN, NO GAIN. */
};


typedef struct trx_t{
    int trx_id;
    lock_t* lock;
    struct val_t* old_val;
    UT_hash_handle hh;
}trx_t;



trx_t* get_trx_table();
int db_begin();
int db_commit(int trx_id);