#include "lock_table.h"




node_t* hash_table;

int
init_lock_table()
{	
	hash_table = NULL;
	pthread_mutex_init(&lock_table_latch, 0);
	/* DO IMPLEMENT YOUR ART !!!!! */

	return 0;
}

void print_node(node_t* node){/*
	printf("\n\n\n\n");
	printf("-----------------------------------------\n");
	printf("노드정보) key:%ld, head ID: %d, tail ID: %d\n", node->key.record_id, node->head->trx_id, node->tail->trx_id );
	printf("\n");
	lock_t* lock = node->head;
	while(lock != NULL){
	printf("lock정보: trx_id: %d, lock_mode: %d \n", lock->trx_id, lock->lock_mode);
	lock = lock->prev;
	printf("\t\t\tV\t\t\n");
	}
	printf("-----------------------------------\n\n\n\n");*/
}

void my_abort(int trx_id){
	printf("my_abort()\n");
	trx_t* trx;
	HASH_FIND_INT(get_trx_table(), &trx_id, trx);
	lock_t* lock = trx->lock;
	val_t* val_str;
	node_t* node;
	page_t page;
	while(lock != NULL){
		node = lock->sent;
		if(lock->next == NULL && lock->lock_mode == 1){
		//	printf("바꾼거 되돌리기\n");
			val_str = trx->old_val;
			buf_read_frame(val_str->tid, val_str->pnum, &page);
			strcpy(page.leaf_page.records[val_str->index].value, val_str->val);
			my_unlock(val_str->index);
			trx->old_val = val_str->next;
		}

		if(lock-> next != NULL && lock->prev == NULL){
			lock->next->prev = NULL;
			node->tail = lock->next;
		}
		else if(lock->next != NULL && lock->prev !=NULL){
			lock->prev->next = lock->next;
			lock->next->prev = lock->prev;
		}
		else if(lock->next == NULL && lock->prev != NULL){
			node->head = lock->prev;
			lock->prev->next = NULL;
		}
		lock = lock->trx_next;		
	}
	//printf("abort done\n");
}

lock_t*
lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode)
{
	//lock occurs
	//printf("lock얻을준비\n");
	pthread_mutex_lock(&lock_table_latch);
	//printf("lock을 이어보자\n");
	//node pointer points hash table entry
	node_t* node;

	//makes lock
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
	lock->trx_next = NULL;
	//make temp_key to find
	keys_t* temp_key = (keys_t*)malloc(sizeof(keys_t));

	memset(temp_key, 0, sizeof(keys_t));
	temp_key->table_id = table_id;
	temp_key->record_id = key;

	//find table_id, record_id in hash table
	HASH_FIND(hh, hash_table, temp_key, sizeof(keys_t), node);
	
	//if there's no pred, return new lock_t	
	if(node == NULL){
		//printf("key:%ld -> trx_id: %d:, lock_mode: %d 내가 1빠!", key, trx_id, lock_mode);
		node = (node_t*)malloc(sizeof(node_t));
		memset(node, 0, sizeof(node_t));

		node->key.table_id = table_id;		
		node->key.record_id = key;
		
		lock-> prev = NULL;
		lock-> next = NULL;
		//printf("lock next NULL\n");
		lock-> sent = node;

		lock->trx_id = trx_id;
		lock->lock_mode = lock_mode;

		trx_t* trx;
		HASH_FIND_INT(get_trx_table(), &trx_id, trx);
		//printf("trx id: %d\n", trx->trx_id);
		lock_t* trx_lock = trx->lock;
		
		if(trx_lock == NULL){
			trx->lock = lock;
			//printf("trx의 lock이 처음 이어짐\n");
		}
		else{
			while(trx_lock->trx_next != NULL){
				trx_lock = trx_lock->trx_next;
			}
			trx_lock->trx_next = lock;
		}

		node->tail = lock;
		node->head = lock;
		//printf("node 의 주소: %p\n", node);
		HASH_ADD(hh, hash_table, key, sizeof(keys_t), node);
		//unlock before returning
		//printf("lock을 다아아아아이었따!\n");	
		pthread_mutex_unlock(&lock_table_latch);
		
		return lock;
	}

	//if exists
	lock->prev = NULL;	
	lock->sent = node;
	
	//printf("노드의 헤드의 trxid: %d, 내 trx_id: %d\n", node->head->trx_id, trx_id);
	//if there's pred, sleep until pred wakes up.
	if(node->head != NULL){
		//printf("진행\n");
		print_node(node);
		//printf("key:%ld -> trx_id: %d:, lock_mode: %d 뒤에라도 서야지\n", key, trx_id, lock_mode);
		/*case: if I read or write into same record*/

		lock_t* prelock;
		//만약 달려있는 lock이 내가 매단거고 유일하면 그 lock을 return
		if(node->head->trx_id == node->tail->trx_id && node->head->trx_id == trx_id){
			pthread_mutex_unlock(&lock_table_latch);
			//printf("lock을 다아아아아이었따!\n");	
			return node->head;
		}
		//if there exists lock which has same trx-> abort or continue
		else if((prelock = find_same_trx(node, trx_id)) != NULL){
			//if prelock is S and I am X
			if(prelock->lock_mode == 0 && lock_mode == 1){
				
				//printf("앞에 내가 있어! 모드도 나는 쓰는데 앞은 읽어!\n");
				//printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
				my_abort(trx_id);
				pthread_mutex_unlock(&lock_table_latch);
				return NULL;	//abort 발생!!
			}
			else
			{//prelock is already working, I also work
				pthread_mutex_unlock(&lock_table_latch);
				//printf("lock을 다아아아아이었따!\n");	
				return prelock;
			} 
		}


		/*case: first time to read or write*/
		
		//before connection, deadlock이 일어날지 check
		int exist = detect(node, trx_id);
		if(exist == -1){
			//printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
			my_abort(trx_id);
			pthread_mutex_unlock(&lock_table_latch);
			return NULL; //dedlock detected->abort 발생!
		}
		
		//printf("내가없으니 그냥 연결해볼까?\n");
		lock->trx_id = trx_id;
		lock->lock_mode = lock_mode;

		trx_t* trx;
		HASH_FIND_INT(get_trx_table(), &trx_id, trx);
		lock_t* trx_lock = trx->lock;
		if(trx_lock == NULL) trx->lock = lock;
		else{
			while(trx_lock->trx_next != NULL){
				trx_lock = trx_lock->trx_next;
			}
			trx_lock->trx_next = lock;
		}
		
		pthread_cond_init(&(lock->cond), NULL);
		
		lock->next = node->tail;
		lock->prev = NULL;
		node->tail->prev = lock;
		node->tail = lock;
		//만약 내 앞에 shared 밖에없고 내가 shared면
		if(lock_mode == 0){
			//printf("일단 나는 s 모드\n");
			lock_t* prelock = lock->next;
			while(prelock->lock_mode == 0 ){
				//printf("prelock 의 trxid: %d", prelock->trx_id);
				prelock = prelock->next;
				if(prelock == NULL){
					//printf("null 까지 찾았다!\n");
					break;
				} 
			}
			if(prelock == NULL){
				//printf("key:%ld -> trx_id: %d:, lock_mode: %d 인데 내앞 싹다 shared\n", key, trx_id, lock_mode);	
				print_node(node);
				//printf("lock을 다아아아아이었따!\n");
				pthread_mutex_unlock(&lock_table_latch);
				return lock;
			} 
		}	
		print_node(node);
		//printf("일단 자자..zzzzzzzzzzzzzzzzzzzzzzzzz\n");
		pthread_cond_wait(&(lock->cond), &lock_table_latch);
	}
	else{
		node->tail = lock;
		node->head = lock;
	}
	//printf("진행\n");
	//내가 S고 내 뒤가 S면, 깨우자일단
	if(lock->prev != NULL && lock->prev->lock_mode == 0 && lock_mode == 0){
		pthread_cond_signal(&(lock->prev->cond));
	}
	print_node(node);
	pthread_mutex_unlock(&lock_table_latch);
	//printf("lock을 다아아아아이었따!\n");	
	return lock;
}



lock_t* find_same_trx(node_t* node, int trx_id){
	lock_t* lock = node->tail;
	while(lock != NULL){
		if(lock->trx_id == trx_id) return lock;
		lock = lock->next;
	}
	return NULL; 
}


int detect(node_t* node, int trx_id){
	/*lock_t* lock;
	//printf("검사한번ㄱㄱㄱ?\n");
	trx_t* trx;
	lock = node->tail;
	int i = 0;
	while(lock != NULL){
		//printf("진행\n");
		//printf("%d번째 실행 ", i);
		i++;
		
		lock_t* trx_lock;
		
		HASH_FIND_INT(get_trx_table(), &(lock->trx_id), trx);	
		
		trx_lock = trx->lock;
		
		while(trx_lock != NULL){
			printf("번째 실행 \n");
			lock_t* check_lock = trx_lock-> next;
			lock_t* save = check_lock;
			while(check_lock != NULL){
				if(check_lock->trx_id == trx_id ){
					while(check_lock != save){
						if(check_lock->lock_mode == 1){
							printf("데드락있을수도..\n");
							return -1; //deadlock
						} 
						check_lock = check_lock->prev;
					}
				} 
				check_lock = check_lock->next;
			}
			trx_lock = trx_lock->trx_next;
		}
		lock = lock-> next;
	}
	print_node(node);
	return 0; //not exist*/
}


int
lock_release(lock_t* lock_obj)
{
	//pthread_mutex_lock(&lock_table_latch);
	//printf("\n\n\n\n\ntrx_id:%d의 release 진행\n", lock_obj->trx_id);
	node_t* node = lock_obj->sent;
	//앞에 뭐가있고 뒤에없으면, 누군가는 아직 일하고있는중임.
	if(lock_obj->next != NULL && lock_obj->prev == NULL){
	//	printf("case1\n");
		lock_obj->next->prev = NULL;
		node->tail = lock_obj->next;
	}
	//앞에 뭐가있고 뒤에도 있으면 잎에는 일하고있으니까 뒤가 누구든지 깨우면 안됨.
	else if(lock_obj->next != NULL && lock_obj->prev !=NULL){
		//printf("case2\n");
		//printf("node 의 주소:%p", node);
		//printf("lock next id: %d lock prev id: %d\n", lock_obj->next->trx_id, lock_obj->prev->trx_id);
		lock_obj->prev->next = lock_obj->next;
		lock_obj->next->prev = lock_obj->prev;
	}
	//앞에 뭐가없고 뒤에 있으면 뒤에는 일하고있으면 깨워도 아무일없고 일안하고있으면 내가 마지막이므로 깨워야댐
	else if(lock_obj->next == NULL && lock_obj->prev != NULL){	
		//printf("case3:뒤에 꺠움\n");
		node->head = lock_obj->prev;
		lock_obj->prev->next = NULL;
		pthread_cond_signal(&(lock_obj->prev->cond));
	}
	//앞뒤 둘다없으면 해당 노드를 해시테이블에서 삭제
	else{
		//printf("case4:n");
		//printf("node 의 주소:%p", node);
		HASH_DEL(hash_table, node);
		free(node);
	}
	//printf("free해보자\n");
	free(lock_obj);
	//printf("trx_id: %d의 release done\n", lock_obj->trx_id);
	//pthread_mutex_unlock(&lock_table_latch);

	return 0;
	/* GOOD LUCK !!! */
	
}
