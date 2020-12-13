#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "trx.h"
#include "file.h"
#include "buffer.h"


/* APIs for lock table */
#define TRX_NUM 100;

typedef struct lock_t lock_t;
typedef struct keys_t keys_t;

typedef struct node_t node_t;



pthread_mutex_t lock_table_latch;


void my_abort(int trx_id);
int init_lock_table();
int lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode, lock_t* ret_lock);
void lock_wait(lock_t* lock_obj);
int detect(node_t* node, int trx_id);
lock_t* find_same_trx(node_t* node, int trx_id);
void my_abort(int trx_id);
int lock_release(lock_t* lock_obj);
#endif /* __LOCK_TABLE_H__ */

