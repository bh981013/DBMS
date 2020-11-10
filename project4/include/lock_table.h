#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include <stdint.h>
#include <pthread.h>
#include "uthash.h"


/* APIs for lock table */

typedef struct lock_t lock_t;

typedef struct keys_t{
	int table_id;
	int64_t record_id;
}keys_t;

typedef struct node_t{
	keys_t key;
	lock_t* tail;
	lock_t* head;
	UT_hash_handle hh;	//uthash
}node_t;

struct lock_t{
	lock_t* prev;
	lock_t* next;
	node_t* sent;
	pthread_cond_t cond;
	/* NO PAIN, NO GAIN. */
};

int init_lock_table();
lock_t* lock_acquire(int table_id, int64_t key);
int lock_release(lock_t* lock_obj);
#endif /* __LOCK_TABLE_H__ */

