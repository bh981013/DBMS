#include "lock_table.h"
#include <stdio.h>

pthread_mutex_t lock_table_latch;

node_t* hash_table;

int
init_lock_table()
{	
	hash_table = NULL;
	pthread_mutex_init(&lock_table_latch, 0);
	/* DO IMPLEMENT YOUR ART !!!!! */

	return 0;
}

lock_t*
lock_acquire(int table_id, int64_t key)
{
	//lock occurs
	pthread_mutex_lock(&lock_table_latch);

	//node pointer points hash table entry
	node_t* node;

	//makes lock
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
	if(lock == NULL) return NULL;
	//make temp_key to find
	keys_t* temp_key = (keys_t*)malloc(sizeof(keys_t));
	if(temp_key == NULL) return NULL;

	memset(temp_key, 0, sizeof(keys_t));
	temp_key->table_id = table_id;
	temp_key->record_id = key;

	//find table_id, record_id in hash table
	HASH_FIND(hh, hash_table, temp_key, sizeof(keys_t), node);
	
	//if there's no pred, return new lock_t	
	if(node == NULL){
		node = (node_t*)malloc(sizeof(node_t));
		if(node == NULL) return NULL;
		memset(node, 0, sizeof(node_t));

		node->key.table_id = table_id;		
		node->key.record_id = key;
		
		lock-> prev = NULL;
		lock-> next = NULL;
		lock-> sent = node;
		node->tail = lock;
		node->head = lock;

		HASH_ADD(hh, hash_table, key, sizeof(keys_t), node);
		//unlock before returning		
		pthread_mutex_unlock(&lock_table_latch);
		
		return lock;
	}


	//if exists
	lock->prev = NULL;	
	lock->sent = node;
	
	
	//if there's pred, sleep until pred wakes up.
	if(node->head != NULL){
		
		pthread_cond_init(&(lock->cond), NULL);
		
		lock->next = node->tail;
		node->tail->prev = lock;
		node->tail = lock;
		
		pthread_cond_wait(&(lock->cond), &lock_table_latch);
	}
	else node->tail = lock;
	lock->next = NULL;
	node->head = lock;
	
	pthread_mutex_unlock(&lock_table_latch);
	
	return lock;

	/* ENJOY CODING !!!! */
}

int
lock_release(lock_t* lock_obj)
{
	pthread_mutex_lock(&lock_table_latch);

	if(lock_obj->prev != NULL){	
		pthread_cond_signal(&(lock_obj->prev->cond));
	}
	else{
		lock_obj->sent->head = NULL;
		lock_obj->sent->tail = NULL;
	}
	
	free(lock_obj);

	pthread_mutex_unlock(&lock_table_latch);

	return 0;
	/* GOOD LUCK !!! */
	
}
